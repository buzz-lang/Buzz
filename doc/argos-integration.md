# Buzz and ARGoS Integration
To use Buzz in the [ARGoS simulator](http://www.argos-sim.info/), it is absolutely necessary to **INSTALL ARGoS FIRST**, otherwise the necessary Buzz plugins won't be compiled.

## Installing ARGoS

[ARGoS](http://www.argos-sim.info) is a fast multi-robot simulator that can interoperate with Buzz. There are two ways to install ARGoS: from binaries or from source.

### Install from binaries
Go to http://www.argos-sim.info/core.php and install a binary package. 

### Install from source
To install ARGoS from source, follow the [instructions](https://github.com/ilpincy/argos3) from the offical repository.
In short, for a basic installation, these steps are are the following:

```bash
# Install dependencies
sudo apt-get install cmake libfreeimage-dev libfreeimageplus-dev \
  qt5-default freeglut3-dev libxi-dev libxmu-dev liblua5.3-dev \
  lua5.3 doxygen graphviz libgraphviz-dev asciidoc

# Download ARGoS
git clone https://github.com/ilpincy/argos3.git

# Build and install ARGoS
cd argos3
mkdir build_simulator && cd build_simulator
cmake ../src
make
make doc
sudo make install
```

### ARGoS Usage Examples
To get started with ARGoS, refer to the [examples](https://github.com/ilpincy/argos3-examples).

## Installing Buzz

Compile and install Buzz following these instructions (the same as in the [README](../README.md)). Again, make sure to compile Buzz **after** having installed ARGoS, so the compilation scripts will also compile the ARGoS integration code.

```bash
cd buzz
mkdir build && cd build
cmake ../src
make
sudo make install
```

On Linux, run this command too:
```bash
sudo ldconfig
```

## Library Configuration

The Buzz integration library for ARGoS is composed of two elements:

  - A set of ARGoS controllers. At the moment, available controllers include one for the [foot-bot](http://www.swarmanoid.org/swarmanoid_hardware.php.html), one for the Khepera IV robot (both wheeled robots), and one for the [Spiri](https://github.com/Pleiades-Spiri) (a commercial quad-rotor). More can be added easily by subclassing `CBuzzController`, defined in `$PREFIX/include/buzz/argos/buzz_controller.h`. The variable `$PREFIX` depends on your system and is usually `/usr` or `/usr/local`.

  - A special definition of ARGoS' QtOpenGL user functions, which allow Buzz scripts to draw in the OpenGL visualization of ARGoS.

To have ARGoS find the Buzz integration library in case you installed it in a non-default location, set the environment variable `ARGOS_PLUGIN_PATH`. This variable is a colon-separated list of directories in which ARGoS looks for libraries before launching an experiment. For instance:

```bash
export ARGOS_PLUGIN_PATH=/opt/lib/buzz
```

If you installed Buzz without specifying a custom installation prefix (e.g., using only `cmake -DCMAKE_BUILD_TYPE=Release ../src; make; make install`), you don't need to set `ARGOS_PLUGIN_PATH`.

# ARGoS Experiments

## Defining an ARGoS experiment file

To use ARGoS and Buzz together, define your `.argos` experiment file as usual.

However, instead of a custom controller, in the `<controllers>` section use `<buzz_controller_footbot>` or `<buzz_controller_spiri>` (or both!), depending on the robots you intend to use. For example, if you want to use both controllers, write something similar to this:

```xml
<!-- 
For a full example of an ARGoS configuration file, refer to

https://github.com/ilpincy/argos3-examples/blob/master/experiments/diffusion_1.argos

and the other examples at

https://github.com/ilpincy/argos3-examples/blob/master/experiments/ -->

<controllers>

  <!-- Include this if you're using foot-bots -->
  <buzz_controller_footbot id="bcf">
    <actuators>
      <differential_steering implementation="default" />
      <leds implementation="default" medium="leds" />
      <range_and_bearing implementation="default" />
    </actuators>
    <sensors>
      <range_and_bearing implementation="medium" medium="rab" show_rays="true" noise_std_dev="0" />
    </sensors>
    <params />
  </buzz_controller_footbot>

  <!-- Include this if you're using Khepera IV -->
  <buzz_controller_kheperaiv id="bckiv">
    <actuators>
      <differential_steering implementation="default" />
      <leds implementation="default" medium="leds" />
      <range_and_bearing implementation="default" />
    </actuators>
    <sensors>
      <range_and_bearing implementation="medium" medium="rab" show_rays="true" noise_std_dev="0" />
    </sensors>
    <params />
  </buzz_controller_kheperaiv>

  <!-- Include this if you're using Spiri -->
  <buzz_controller_spiri id="bcs">
    <actuators>
      <quadrotor_position implementation="default" />
      <range_and_bearing implementation="default" />
    </actuators>
    <sensors>
      <range_and_bearing implementation="medium" medium="rab" show_rays="false" />
      <positioning implementation="default" />
    </sensors>
    <params />
  </buzz_controller_spiri>

</controllers>

<arena ...>

   <!-- Place a foot-bot in the arena -->
   <foot-bot id="fb0" rab_data_size="100">
      ...
      <controller config="bcf" />
   </foot-bot>

   <!-- Place a Khepera IV in the arena -->
   <kheperaiv id="fb0" rab_data_size="100">
      ...
      <controller config="bcf" />
   </kheperaiv>

   <!-- Place a Spiri in the arena -->
   <spiri id="fb0" rab_data_size="100">
      ...
      <controller config="bcs" />
   </spiri>
</arena>
```

Always make sure the parameters `arena/<robot>/rab_data_size` are set to relatively large values, such as 100 bytes as reported in the above examples.

If you want ARGoS to start with a Buzz script already loaded, you can specify that in the `<params />` tag of the robot controller. The above example for the foot-bot, for example, becomes:

```xml
  ...

  <buzz_controller_footbot id="bcf">
    <actuators>
      <differential_steering implementation="default" />
      <leds implementation="default" medium="leds" />
      <range_and_bearing implementation="default" />
    </actuators>
    <sensors>
      <range_and_bearing implementation="medium" medium="rab" show_rays="true" noise_std_dev="0" />
    </sensors>
    
    <!-- This loads the specified files at startup -->
    <params bytecode_file="myscript.bo" debug_file="myscript.bdb" />
    
  </buzz_controller_footbot>
  
  ...
```

To activate the Buzz editor and support debugging, use `buzz_qt` to indicate that you want to use the Buzz QtOpenGL user functions:

```xml
<!-- For a full example of an ARGoS configuration file, refer to
     https://github.com/ilpincy/argos3-examples/blob/master/experiments/diffusion_1.argos
-->

<visualization>
  <qt-opengl>
    <user_functions label="buzz_qt" />
  </qt-opengl>
</visualization>
```

You can launch ARGoS as usual, with the command:

```bash
argos3 -c myexperiment.argos
```

# Debugging Buzz Programs

## Inspecting a Robot's State

To understand what's happening on a specific robot, shift-click on it in the ARGoS visualization to select it. This opens a tree widget in the Buzz editor that reports the value of all the variables, and the list of available functions on the robot.

## Debugging Information

The Buzz integration library offers a data structure, called `debug`, that allows the developer to perform several operations.

### Writing Text on Top of a Robot

  * `debug.print(message)`
    * prints a message on top of the robot, after the robot id
    * `message` can be a combination of text and variables, such as `"x = ", x, " cm"`

### Drawing the Trajectory of a Robot

  * `debug.trajectory.enable(maxpoints, r, g, b)`
    * enable trajectory tracking setting how many points should be stored and the drawing color
    * `(r, g, b)` is the drawing color of the trajectory (0-255 for each value)
  * `debug.trajectory.enable(maxpoints)`
    * enable trajectory tracking setting how many points should be stored
  * `debug.trajectory.enable(r, g, b)`
    * enable trajectory tracking keeping maxpoints' last value and setting the drawing color
    * `(r, g, b)` is the drawing color of the trajectory (0-255 for each value)
  * `debug.trajectory.enable()`
    * enable trajectory tracking keeping maxpoints' last value (default is 30)
  * `debug.trajectory.disable()`
    * disable trajectory tracking
  * `debug.trajectory.clear()`
    * delete all the trajectory points

### Drawing Vectors ###

  * `debug.rays.add(r, g, b, x, y, z)`
    * draw a ray from the reference point of the robot to `(x, y, z)`.
    * `(x, y, z)` is expressed w.r.t. the robot reference frame
    * `(r, g, b)` is the color of the vector (0-255 for each value)
  * `debug.rays.add(r, g, b, x0, y0, z0, x1, y1, z1)`
    * draw a ray from `(x0,y0,z0)` to `(x1,y1,z1)`
    * `(x0, y0, z0)` and `(x1, y1, z1)` are expressed wrt the robot reference frame
    * `(r, g, b)` is the color of the vector (0-255 for each value)
  * `debug.rays.clear()`
    * delete all the rays