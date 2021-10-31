# Buzz on the Khepera IV

The [Khepera IV](https://www.k-team.com/khepera-iv) is a commercial robot produced by [K-Team](https://www.k-team.com/).

A Buzz port for the Khepera IV is available at https://github.com/MISTLab/BuzzKH4. An ARGoS plugin to simulate the robot is available at https://github.com/ilpincy/argos3-kheperaiv.

## Supported Devices

### Actuators

  * **Wheels**

    * `set_wheels(lws, rws)` sets the speed of the wheels to `lws` (left wheel speed) and `rws` (right wheel speed). The speed is expressed in cm/sec. The maximum speed of the real Khepera is about 20 cm/sec. `lws, rws` must be floating point values.

    * `gotop(ls, as)` sets the speed of the center of mass of the robot. The speed is expressed as a vector in polar coordinates, where `ls` is the linear speed (i.e., the forward speed, in cm/sec) and `as` is the angular speed (i.e., in rad/sec). The vector is internally transformed into wheel actuation. `ls, as` must be floating point values.

    * `gotoc(sx, sy)` sets the speed of the center of mass of the robot. The speed is expressed as a vector in cartesian coordinates, where `sx` is the linear speed along the local x axis of the robot (i.e., the forward speed, in cm/sec) and `sy` is the linear speed along the local y axis of the robot (a vector pointing left, according to the right-hand rule). The vector is internally transformed into wheel actuation. `sx, sy` must be floating point values.

    * To activate this actuator in ARGoS, use the `differential_drive` actuator.

  * **LEDs**

    * `set_leds(r, g, b)` sets the color of the three LEDs on top of the Khepera IV. The color is expressed with 0-255 integer values for each of the channels (red, green, blue).

    * To activate this actuator in ARGoS, use the `leds` actuator.

### Sensors

  * **Proximity sensors**
    * The Khepera IV proximity sensor is a set of 8 infrared emitter/receiver pairs distributed regularly in a ring around the robot. The sensors are numbered 0 to 7. Sensor 0 looks straight ahead, and the numbers increase counterclockwise when looking at the robot from above.
    * `proximity` is the Buzz table that contains the proximity readings. Each element of this table (e.g., `proximity[0]`, `proximity[1]`, ...) is in turn a table that contains two elements:
      * `angle`, which corresponds to the angle (expressed in radians) at which the sensor is located on the body of the robot.
      * `value`, which is the actual reading. Each individual sensor is saturated (value `1.0`) by objects closer than 4 cm; the maximum range of the sensor is 12 cm. Between 4 and 12 cm, the readings follow an exponential law with the distance: `4.14 * exp(-33.0 * distance) - 0.085`.
      * To activate this sensor in ARGoS, use the `kheperaiv_proximity` sensor.

  * **Light sensors**
    * The Khepera IV light sensor is a set of 8 infrared emitter/receiver pairs distributed regularly in a ring around the robot. The sensors are numbered 0 to 7. Sensor 0 looks straight ahead, and the numbers increase counterclockwise when looking at the robot from above.
    * `light` is the Buzz table that contains the light readings. Each element of this table (e.g., `light[0]`, `light[1]`, ...) is in turn a table that contains two elements:
      * `angle`, which corresponds to the angle (expressed in radians) at which the sensor is located on the body of the robot
      * `value`, which is the actual reading. The sensors all return a value
between 0 and 1, where 0 means nothing within range and 1 means the perceived
light saturates the sensor. Values between 0 and 1 depend on the distance of the perceived light. In ARGoS, each reading `R` is calculated with `R = (I / x)^2`, where `x` is the distance between a sensor and the light, and `I` is the reference intensity of the perceived light. The reference intensity corresponds to the minimum distance at which the light saturates a sensor.
      * To activate this sensor in ARGoS, use the `kheperaiv_light` sensor.

  * **Ultrasound sensors**
    * The Khepera IV ultrasound sensor is a set of 5 emitter/receiver pairs distributed regularly in an arc in front of the robot. The sensors are numbered 0 to 4. Sensor 0 looks straight ahead, and the numbers increase counterclockwise when looking at the robot from above.
    * `ultrasound` is the Buzz table that contains the ultrasound readings. Each element of this table (e.g., `ultrasound[0]`, `ultrasound[1]`, ...) is in turn a table that contains two elements:
      * `angle`, which corresponds to the angle (expressed in radians) at which the sensor is located on the body of the robot.
      * `value`, which is the actual distance of the detected obstacle in cm. Each individual sensor is saturated by objects closer than 25 cm; the maximum range of the sensor is 200 cm.
      * To activate this sensor in ARGoS, use the `kheperaiv_ultrasound` sensor.

  * **LIDAR sensor**
    * The Khepera IV LIDAR sensor returns 682 readings in a 270 degrees arc (if not configured to return less readings in the `.argos` file).
    * `lidar` is the Buzz table that contains the LIDAR readings. Each element of this table (e.g., `lidar[0]`, `lidar[1]`, ...) is the distance of the object detected by a specific laser beam.
      * To activate this sensor in ARGoS, use the `kheperaiv_lidar` sensor.

  * **Battery sensor**
    * This sensor returns the state of the battery.
    * `battery` is a table that contains two elements:
      * `available_charge`, a value between 0.0 and 1.0.
      * `time_left`, the number of control steps before battery depletion.
      * To activate this sensor in ARGoS, use the `battery` sensor.

  * **Positioning sensor** (ARGoS only)
    * This sensor is a sort of GPS, only available in ARGoS. The positioning sensor returns the current pose of the robot.
    * `pose` is a table that contains two elements:
      * `position`, a 3D array `{x, y, z}`.
      * `orientation`, an array `{yaw, pitch, roll}`.
      * To activate this sensor in ARGoS, use the `positioning` sensor.
