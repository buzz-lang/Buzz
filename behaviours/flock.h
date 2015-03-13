 /* AUTHOR: Adam Lee-Brown <adamlb@live.com.au> */

  #ifndef FLOCK_H
  #define FLOCK_H

/* Headers */
/* Definition of the CCI_Controller class. */
#include <argos3/core/control_interface/ci_controller.h>
/* Definition of the range-and-bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
/* Vector2 definitions */
#include <argos3/core/utility/math/vector2.h>

using namespace argos;

class Flock {
    
public:
    /* Class constructor. */
    Flock() {
        /* Pointer to the range-and-bearing sensor */
        CCI_RangeAndBearingSensor* m_pcRABSens;

        /* Target robot-robot distance in cm */
        Real rTargetDistance = 100;
        /* Gain of the Lennard-Jones potential */
        Real rGain = 2.5;
        /* Exponent of the Lennard-Jones potential */
        Real rExponent = 1.5;
        /* Max length for the resulting interaction force vector */
        Real rMaxInteraction = 0.025;
        /* Range of the "range and bearing" device */
        Real RAB = 6;
    }
    /* Init */
    virtual void Init();
    
    /* Control step */
    virtual void ControlStep();
    
    /* Destroy */
    virtual void Destroy() {}
    
private:
    /* Calculate the flocking interaction vector */
    CVector2 FlockingVector();
    
    /* Lennard-Jones: */
    Real GeneralizedLennardJones(Real TargetDistance, Real f_distance);
    
};

#endif

