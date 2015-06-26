#ifndef BUZZ_CONTROLLER_FOOTBOT_H
#define BUZZ_CONTROLLER_FOOTBOT_H

#include "buzz_controller.h"
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>

using namespace argos;

class CBuzzControllerFootBot : public CBuzzController {

public:

   struct SWheelTurningParams {
      /*
       * The turning mechanism.
       * The robot can be in three different turning states.
       */
      enum ETurningMechanism
      {
         NO_TURN = 0, // go straight
         SOFT_TURN,   // both wheels are turning forwards, but at different speeds
         HARD_TURN    // wheels are turning with opposite speeds
      } TurningMechanism;
      /*
       * Angular thresholds to change turning state.
       */
      CRadians HardTurnOnAngleThreshold;
      CRadians SoftTurnOnAngleThreshold;
      CRadians NoTurnAngleThreshold;
      /* Maximum wheel speed */
      Real MaxSpeed;

      void Init(TConfigurationNode& t_tree);
   };

public:

   CBuzzControllerFootBot();
   virtual ~CBuzzControllerFootBot();

   virtual void Init(TConfigurationNode& t_node);

   void SetWheelSpeedsFromVector(const CVector2& c_heading);
   void SetLEDs(const CColor& c_color);

private:

   virtual buzzvm_state RegisterFunctions();

protected:

   /* Pointer to the differential steering actuator */
   CCI_DifferentialSteeringActuator* m_pcWheels;
   /* Pointer to the LEDs actuator */
   CCI_LEDsActuator* m_pcLEDs;

   /* The turning parameters. */
   SWheelTurningParams m_sWheelTurningParams;

};

#endif
