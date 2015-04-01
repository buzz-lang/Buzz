 /* AUTHOR: Adam Lee-Brown <adamlb@live.com.au> */

  #ifndef MOVE_H
  #define MOVE_H

/* Definition of the CCI_Controller class. */
#include <argos3/core/control_interface/ci_controller.h>
/* Definition of the quadrotor positioning actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_quadrotor_position_actuator.h>
/* Definition of the range-and-bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
/* Definition of the positioning sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
/* Vector2 definitions */
#include <argos3/core/utility/math/vector2.h>

using namespace argos;

class CMove {
    
public:
    CMove(CCI_QuadRotorPositionActuator& m_pcPosAct,
           CCI_RangeAndBearingSensor& m_pcRABSens,
           CCI_PositioningSensor& m_pcPosSens,
           Real m_rTargetDistanceKin,
           Real m_rTargetDistanceNotKin,
           Real m_rGain,
           Real m_rExponent,
           Real m_rMaxInteraction,
           Real m_rRAB,
           CVector3& m_cTargetPos);
    
    void TakeOff();
    
    void Move(uint32_t m_unGroup);
    
    bool ArrivalCheck();
    
    /* Calculates the vector to the given target position */
    CVector2 VectorToPoint();
    
    /* Calculate the flocking interaction vector */
    CVector2 FlockingVector(uint32_t m_unGroup);
    
    /* Lennard-Jones: */
    Real GeneralizedLennardJones(Real TargetDistance, Real f_distance);
    
private:
    CCI_QuadRotorPositionActuator& m_pcPosAct;
    CCI_RangeAndBearingSensor& m_pcRABSens;
    CCI_PositioningSensor& m_pcPosSens;
    
    Real m_rTargetDistanceKin;
    Real m_rTargetDistanceNotKin;
    Real m_rGain;
    Real m_rExponent;
    Real m_rMaxInteraction;
    Real m_rRAB;
    
    CVector3& m_cTargetPos;
};

#endif

