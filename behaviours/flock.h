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
    /* Public class constructor. */
    Flock(Real rTargetDistance,
          Real rGain,
          Real rExponent,
          Real rMaxInteraction,
          Real rRAB);
    
    void SetFlockParams(Real rTargetDistance,
                        Real rGain,
                        Real rExponent,
                        Real rMaxInteraction,
                        Real rRAB);
    
    /* Control step */
    virtual void ControlStep();
    
private:
    /* Private default constructor */
    Flock() {}
    
    /* Calculate the flocking interaction vector */
    CVector2 FlockingVector();
    
    /* Lennard-Jones: */
    Real GeneralizedLennardJones(Real TargetDistance, Real f_distance);
    
private:
    Real m_rTargetDistance;
    Real m_rGain;
    Real m_rExponent;
    Real m_rMaxInteraction;
    Real m_rRAB;
};

#endif

