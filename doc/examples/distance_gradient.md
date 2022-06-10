# Calculation of a Distance Gradient

The aim of this code is to have a group of robots form a distance gradient from a source.

There is one robot that acts as the source; for simplicity here it is the robot with id 0. Every robot, including the source, emits its estimated distance from the source, and listens to other robots.
* The robots that can see the source directly emit the distance they sense;
* The robots that cannot see the source are in two categories:
  * Those who don't know any distance yet: these robots emit `nil`
  * Those who received a distance broadcast from one or more neighbors: these robots calculate their distance as the minimum among the received distances

This algorithm keeps running in the `step()` function, so it can adjust the distance gradient if the robots move around.

```ruby
function init() {
  if (id == 0) {
    # Source robot
    my_dist = 0.0
  } else {
    # Other robots
    my_dist = 1000.0
    
    # Listen to other robots' distances
    neighbors.listen("dist_to_source",
      function(value_id, value, robot_id) {
        my_dist = math.min(my_dist, neighbors.get(robot_id).distance + value)
      })
  }
}

function step() {
  neighbors.broadcast("dist_to_source", my_dist)
}

function destroy() {
}
```
