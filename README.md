What is Buzz?
=============

Buzz is a novel programming language for heterogeneous robots swarms.

Buzz advocates a compositional approach, by offering primitives to define swarm behaviors both in a bottom-up and in a top-down fashion.

Bottom-up primitives include robot-wise commands and manipulation of neighborhood data through mapping/reducing/filtering operations.

Top-down primitives allow for the dynamic management of robot teams, and for sharing information globally across the swarm.

Self-organization results from the fact that the Buzz run-time platform is purely distributed.

The language can be extended to add new primitives (thus supporting heterogeneous robot swarms) and can be laid on top of other frameworks, such as ROS.

More information is available at http://the.swarming.buzz/wiki/doku.php?id=start.

Downloading Buzz
================

You can download the development sources through git:

    $ git clone https://github.com/MISTLab/Buzz.git buzz

Compiling Buzz
==============

Requirements
------------

You need the following packages:

* A UNIX system (Linux or MacOSX; Microsoft Windows is not supported)
* _g++_ >= 4.3 (on Linux) or _clang_ >= 3.1 (on MacOSX)
* _cmake_ >= 2.8.12

Optionally, you can also install [ARGoS](http://www.argos-sim.info/).

Compilation
-----------

To compile Buzz, execute the following:

    $ cd buzz
    $ mkdir build
    $ cd build
    $ cmake ../src
    $ make

Installation
============

Execute these commands:

    $ cd buzz/build
    $ sudo make install

On Linux, run this command too:
    $ sudo ldconfig
