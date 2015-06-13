#ifndef BUZZ_CONTROLLER_SPIRI_H
#define BUZZ_CONTROLLER_SPIRI_H

#include <buzz_controller.h>
#include <argos3/plugins/robots/generic/control_interface/ci_quadrotor_position_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>

using namespace argos;

class CBuzzControllerSpiri : public CBuzzController {

public:

   CBuzzControllerSpiri();
   virtual ~CBuzzControllerSpiri();

   virtual void Init(TConfigurationNode& t_node);

   bool TakeOff();
   bool Land();
   void SetDirection(const CVector3& c_heading);

private:

   virtual int RegisterFunctions();

protected:

   /* Pointer to the position actuator */
   CCI_QuadRotorPositionActuator* m_pcPropellers;
   /* Pointer to the position sensor */
   CCI_PositioningSensor* m_pcPosition;

};

#endif
