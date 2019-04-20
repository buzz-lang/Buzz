#ifndef BUZZ_CONTROLLER_FOOTBOT_H
#define BUZZ_CONTROLLER_FOOTBOT_H

#include <buzz/argos/buzz_controller.h>
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_colored_blob_omnidirectional_camera_sensor.h>
#include <argos3/plugins/robots/foot-bot/control_interface/ci_footbot_proximity_sensor.h>
#include <argos3/plugins/robots/foot-bot/control_interface/ci_footbot_light_sensor.h>
#include <argos3/plugins/robots/foot-bot/control_interface/ci_footbot_gripper_actuator.h>
#include <argos3/plugins/robots/foot-bot/control_interface/ci_footbot_turret_actuator.h>

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

      SWheelTurningParams();
      void Init(TConfigurationNode& t_tree);
   };

public:

   CBuzzControllerFootBot();
   virtual ~CBuzzControllerFootBot();

   virtual void Init(TConfigurationNode& t_node);

   virtual void UpdateSensors();

   void SetWheels(Real f_left_speed, Real f_right_speed);
   void SetWheelSpeedsFromVector(const CVector2& c_heading);
   void SetLEDs(const CColor& c_color);
   void SetLED(UInt32 un_idx, const CColor& c_color);
   void CameraEnable();
   void CameraDisable();
   void GripperLock();
   void GripperUnlock();
   void TurretEnable();
   void TurretDisable();
   void TurretSet(Real f_rotation);

private:

   virtual buzzvm_state RegisterFunctions();

protected:

   /* Pointer to the differential steering actuator */
   CCI_DifferentialSteeringActuator* m_pcWheels;
   /* Pointer to the LEDs actuator */
   CCI_LEDsActuator* m_pcLEDs;
   /* Pointer to the foot-bot gripper actuator */
   CCI_FootBotGripperActuator* m_pcGripper;
   /* Pointer to the foot-bot turret actuator */
   CCI_FootBotTurretActuator* m_pcTurretA;
   /* Pointer to the proximity sensor */
   CCI_FootBotProximitySensor* m_pcProximity;
   /* Pointer to the light sensor */
   CCI_FootBotLightSensor* m_pcLight;
   /* Pointer to the camera sensor */
   CCI_ColoredBlobOmnidirectionalCameraSensor* m_pcCamera;

   /* The turning parameters. */
   SWheelTurningParams m_sWheelTurningParams;

};

#endif
