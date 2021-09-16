# What is Buzz?

Buzz is a novel programming language for heterogeneous robots swarms.

Buzz advocates a compositional approach, by offering primitives to define swarm behaviors both in a bottom-up and in a top-down fashion.

Bottom-up primitives include robot-wise commands and manipulation of neighborhood data through mapping/reducing/filtering operations.

Top-down primitives allow for the dynamic management of robot teams, and for sharing information globally across the swarm.

Self-organization results from the fact that the Buzz run-time platform is purely distributed.

The language can be extended to add new primitives (thus supporting heterogeneous robot swarms) and can be laid on top of other frameworks, such as ROS.

More information is available at http://the.swarming.buzz/wiki/doku.php?id=start.

# Documentation Contents
The documentation is structured as follows:
1. [Installation](doc/installation.md)
2. [ARGoS Integration](doc/argos-integration.md)
3. Buzz Concepts\
3.1 [Basics](doc/concepts/basics.md)\
3.2 [Virtual Machine](doc/concepts/vm.md)
4. [Buzz API](doc/api.org)
5. [Extending Buzz](doc/integration.org)
