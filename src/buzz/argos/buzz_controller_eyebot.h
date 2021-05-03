#ifndef BUZZ_CONTROLLER_EYEBOT_H
#define BUZZ_CONTROLLER_EYEBOT_H

#include <buzz/argos/buzz_controller.h>
#include <argos3/plugins/robots/generic/control_interface/ci_quadrotor_position_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_colored_blob_perspective_camera_sensor.h>

using namespace argos;

class CBuzzControllerEyeBot : public CBuzzController {

public:

   CBuzzControllerEyeBot();
   virtual ~CBuzzControllerEyeBot();

   virtual void Init(TConfigurationNode& t_node);
   virtual void UpdateSensors();

   bool TakeOff();
   bool Land();
   void SetDirection(const CVector3& c_heading);
   void SetYaw(const CRadians& c_yaw);
   void SetLEDs(const CColor& c_color);
   void SetLED(UInt32 un_idx, const CColor& c_color);
   void CameraEnable();
   void CameraDisable();

protected:

   virtual buzzvm_state RegisterFunctions();

protected:

   /* Pointer to the position actuator */
   CCI_QuadRotorPositionActuator* m_pcPropellers;
   /* Pointer to the LEDs actuator */
   CCI_LEDsActuator* m_pcLEDs;
   /* Pointer to the camera sensor */
   CCI_ColoredBlobPerspectiveCameraSensor* m_pcCamera;

};

#endif
