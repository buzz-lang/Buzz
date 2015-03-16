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

class CFlock {
    
public:
   
   /* Public class constructor. */
   CFlock(CCI_RangeAndBearingSensor* pc_rab_sens,
          Real f_target_distance,
          Real f_gain,
          Real f_exponent,
          Real f_max_interaction,
          Real f_range);
    
   void SetFlockParams(Real f_target_distance,
                       Real f_gain,
                       Real f_exponent,
                       Real f_max_interaction,
                       Real f_range);
    
   /* Control step */
   void Do();
    
   /* Calculate the flocking interaction vector */
   CVector2 FlockingVector();
    
private:
   
   /* Lennard-Jones: */
   Real GeneralizedLennardJones(Real f_distance);
    
private:

   /* Pointer to the range-and-bearing sensor */
   CCI_RangeAndBearingSensor* m_pcRABSens;
   
   Real m_fTargetDistance;
   Real m_fGain;
   Real m_fExponent;
   Real m_fMaxInteraction;
   Real m_fRange;

};

#endif

