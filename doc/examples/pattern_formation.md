# Pattern Formation

## Hexagonal Pattern Formation

Hexagonal patterns can be formed in a simple way by mimicking particle interaction. A simple model of particle interaction is the [Lennard-Jones potential](https://en.wikipedia.org/wiki/Lennard-Jones_potential), which we use in the following code in a slightly modified way. Instead of the big exponents (12 and 6), we use the exponents 4 and 2, which give us smaller but more manageable numbers.

The idea in the code is that every robot can use the `neighbors` structure to sense the distance and angle of every direct neighbor. Using the distance, we calculate the magnitude of the "virtual force" (attraction or repulsion) due to a neighbor (function `lj_magnitude()`). We then use the force magnitude and the angle to make an interaction vector (function `lj_vector()`), and proceed to sum all of these contributions together into an accumulator vector (functions `lj_sum()` and `neighbors.reduce()`). Finally, we scale the accumulator and feed it to the `goto()` function, which transforms a 2D vector into motion.

```ruby
# We need this for 2D vectors
# Make sure you pass the correct include path to "bzzc -I <path1:path2> ..."
include "include/vec2.bzz"

# Lennard-Jones parameters
TARGET     = 283.0
EPSILON    = 150.0

# Lennard-Jones interaction magnitude
function lj_magnitude(dist, target, epsilon) {
    return -(epsilon / dist) * ((target / dist)^4 - (target / dist)^2)
}

# Neighbor data to LJ interaction vector
function lj_vector(rid, data) {
    return math.vec2.newp(lj_magnitude(data.distance, TARGET, EPSILON), data.azimuth)
}

# Accumulator of neighbor LJ interactions
function lj_sum(rid, data, accum) {
    return math.vec2.add(data, accum)
}

# Calculates and actuates the flocking interaction
function hexagon() {
    # Calculate accumulator
    var accum = neighbors.map(lj_vector).reduce(lj_sum, math.vec2.new(0.0, 0.0))
    if (neighbors.count() > 0) {
        math.vec2.scale(accum, 1.0 / neighbors.count())
    }

    # Move according to vector
    goto(accum.x, accum.y)
}

# Executed at init time
function init() {
}

# Executed every time step
function step() {
  hexagon()
}

# Execute at exit
function destroy() {
}
```

### Square Pattern Formation

To form square lattice, we can build upon the previous example. The insight is to notice that, in a square lattice, we can color the nodes forming the lattice with two shades, e.g., red and blue, and then mimic the [crystal structure of kitchen salt](http://www.metafysica.nl/turing/nacl_complex_motif_4.gif). In this structure, if two nodes have different colors, they stay at a distance `D`; if they have the same color, they stay at a distance `D * sqrt(2)`.

With this idea in mind, the following script divides the robots in two swarms: those with an even id and those with an odd id. Then, using `neighbors.kin()` and `neighbors.nonkin()`, the robots can distinguish which distance to pick and calculate the correct interaction vector.

```ruby
# We need this for 2D vectors
# Make sure you pass the correct include path to "bzzc -I <path1:path2> ..."
include "include/vec2.bzz"

# Lennard-Jones parameters
TARGET_KIN     = 283.0
EPSILON_KIN    = 150.0
TARGET_NONKIN  = 200.0
EPSILON_NONKIN = 100.0

# Lennard-Jones interaction magnitude
function lj_magnitude(dist, target, epsilon) {
    return -(epsilon / dist) * ((target / dist)^4 - (target / dist)^2)
}

# Neighbor data to LJ interaction vector
function lj_vector_kin(rid, data) {
    return math.vec2.newp(lj_magnitude(data.distance, TARGET_KIN, EPSILON_KIN), data.azimuth)
}

# Neighbor data to LJ interaction vector
function lj_vector_nonkin(rid, data) {
    return math.vec2.newp(lj_magnitude(data.distance, TARGET_NONKIN, EPSILON_NONKIN), data.azimuth)
}

# Accumulator of neighbor LJ interactions
function lj_sum(rid, data, accum) {
    return math.vec2.add(data, accum)
}

# Calculates and actuates the flocking interaction
function square() {
    # Calculate accumulator
    var accum = neighbors.kin().map(lj_vector_kin).reduce(lj_sum, math.vec2.new(0.0, 0.0))
    accum = neighbors.nonkin().map(lj_vector_nonkin).reduce(lj_sum, accum)
    
    if (neighbors.count() > 0) {
        math.vec2.scale(accum, 1.0 / neighbors.count())
    }

    # Move according to vector
    goto(accum.x, accum.y)
}

# Executed at init time
function init() {
    # Divide the swarm in two sub-swarms
    s1 = swarm.create(1)
    s1.select(id % 2 == 0)
    s2 = s1.others(2)
}

# Executed every time step
function step() {
    s1.exec(square)
    s2.exec(square)
}

# Execute at exit
function destroy() {
}
```
