# What is Buzz?

Buzz is a novel programming language for heterogeneous robots swarms.

Buzz advocates a compositional approach, by offering primitives to define swarm behaviors both in a bottom-up and in a top-down fashion.

Bottom-up primitives include robot-wise commands and manipulation of neighborhood data through mapping/reducing/filtering operations.

Top-down primitives allow for the dynamic management of robot teams, and for sharing information globally across the swarm.

Self-organization results from the fact that the Buzz run-time platform is purely distributed.

The language can be extended to add new primitives (thus supporting heterogeneous robot swarms) and can be laid on top of other frameworks, such as ROS.

A simulation demonstration is available [here](https://www.youtube.com/watch?v=WAlS7a7_BaM)

The whitepaper on Buzz Design and Implementation is available [here](https://arxiv.org/abs/1507.05946).


# Documentation Contents
The documentation is structured as follows:
1. [Installation](doc/installation.md)
2. [ARGoS Integration](doc/argos-integration.md)
3. Buzz Concepts\
3.1 [Basics](doc/concepts/basics.md)\
3.2 [Virtual Machine](doc/concepts/vm.md)
4. [Buzz API](doc/api.org)
5. [Extending Buzz](doc/integration.md)
6. Examples\
6.1 [Distance Gradient](doc/examples/distance_gradient.md)\
6.2 [Pattern Formation](doc/examples/pattern_formation.md)\
6.3 [Message API](doc/examples/message_api.md)
7. Technical Specifications\
7.1 [Assembly Language](doc/technical-specifications/assembler.md)\
7.2 [Backus-Naur Form Syntax](doc/technical-specifications/syntax.md)
8. Robot Integration\
8.1 [Khepera IV](robot-integration/kheperaiv.md)

# Who Made Buzz?
Buzz is a research project conducted at [NESTLab](https://www.nestlab.net), [Worcester Polytechnic Institute](https://www.wpi.edu), and [MIST](https://www.mistlab.ca), [École Polytechnique de Montréal](https://www.polymtl.ca). It is funded by the [Natural Sciences and Engineering Research Council of Canada](https://www.nserc-crsng.gc.ca).

The main developer and maintainer is [Carlo Pinciroli](https://carlo.pinciroli.net).

# Buzz News

## Scientific Publications

* C. Pinciroli, G. Beltrame. 2016. **Swarm-Oriented Programming of Distributed Robot Networks**. *IEEE Computer*. In press.
* C. Pinciroli, G. Beltrame. 2016. **Buzz: An Extensible Programming Language for Heterogeneous Swarm Robotics**. *Proceedings of the IEEE/RSJ International Conference on Intelligent Robots and Systems (IROS 2016)*. In press.
* C. Pinciroli and G. Beltrame. 2016. **Buzz: A Programming Language for Robot Swarms**. *IEEE Software, volume 33, number 4, pages 97-100*. IEEE Press.
* C. Pinciroli, A. Lee-Brown, G. Beltrame. 2015. **A Tuple Space for Data Sharing in Robot Swarms**. *9th EAI International Conference on Bio-inspired Information and Communications Technologies (BICT 2015), pages 287-294*. ACM Digital Library.

## Media Coverage

* [2015/08/11] Heise: [short version](http://www.heise.de/newsticker/meldung/Roboterschwaerme-bekommen-eigene-Programmiersprache-2775563.html) [full version](http://www.heise.de/tr/artikel/Programmiersprache-fuer-Roboterschwaerme-2775561.html)
* [2015/08/10] [RoboHub](http://robohub.org/buzz-a-novel-programming-language-for-heterogeneous-robot-swarms/)
* [2015/08/06] [École Polytechnique de Montréal](http://www.polymtl.ca/carrefour/article.php?no=4692)
* [2015/08/03] [Communications of the ACM](http://cacm.acm.org/news/190313-a-programming-language-for-robot-swarms/fulltext)
* [2015/08/03] [Fusion](http://fusion.net/story/175797/robot-swarms-just-got-a-little-scarier/)
* [2015/07/30] [SD Times](http://sdtimes.com/researchers-develop-buzz-a-programming-language-for-robot-swarms/)
* [2015/07/29] [MIT Technology Review](http://www.technologyreview.com/view/539761/a-programming-language-for-robot-swarms/)

## Talks, Tutorials, Demos

* [2017/01/04] The [ICRA2017 tutorial page](http://the.swarming.buzz/ICRA2017/) is online!
* [2016/11/16] Tutorial proposal on Buzz accepted at [ICRA2017](http://www.icra2017.org/).
* [2016/10/29-30] Demo at [NERC 2016](http://northeastrobotics.org/NERC2016.html), the 5th Northeast Robotics Colloquium.
* [2016/10/12] Oral presentation at [IROS 2016](http://www.iros2016.org/)
* [2016/06/18] Tutorial at the [Swarmathon workshop](http://nasaswarmathon.com/rssworkshop) at [RSS2016](http://www.roboticsconference.org)
* [2016/01/29] Colloquium at [Worcester Polytechnic Institute](https://www.wpi.edu): *Software for Complex Robot Swarms* (Carlo Pinciroli)
* [2016/01/15] Colloquium at [Université de Montreal](https://www.umontreal.ca): *Software for Complex Robot Swarms* (Carlo Pinciroli)
* [2015/11/19] Guest lecture for the course of [Evolutionary Swarm Robotics](https://sites.google.com/site/esrcs591/) ([Prof. Moses](https://www.cs.unm.edu/~melaniem/Home.html)) at [University of New Mexico](http://www.unm.edu): *Programming Complex Robot Swarms* (Carlo Pinciroli)
* [2015/11/18] Colloquium at [University of New Mexico](http://www.unm.edu): *Software for Complex Swarm Robotics* (Carlo Pinciroli)

## Other

* [2016/03/02] Buzz to be used in the [NASA Swarmathon](http://nasaswarmathon.com/)
* [2015/08/10] [The Hello World Collection](http://helloworldcollection.de/) includes an example for a Buzz 'Hello world!' program.
