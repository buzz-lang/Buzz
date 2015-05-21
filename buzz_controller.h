#ifndef BUZZ_CONTROLLER_H
#define BUZZ_CONTROLLER_H

#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>

#include "buzzvm.h"

#include <string>

using namespace argos;

class CBuzzController : public CCI_Controller {

public:

   CBuzzController();
   virtual ~CBuzzController();

   virtual void Init(TConfigurationNode& t_node);
   virtual void Reset();
   virtual void ControlStep();
   virtual void Destroy();

   void SetBytecode(const std::string& str_fname);

private:

   int RegisterFunctions();

   void ProcessInMsgs();
   void ProcessOutMsgs();

   void UpdateSensors();
   void UpdateActuators();

private:

   /* Pointer to the range and bearing actuator */
   CCI_RangeAndBearingActuator*  m_pcRABA;
   /* Pointer to the range and bearing sensor */
   CCI_RangeAndBearingSensor* m_pcRABS;

   /* The robot numeric id */
   UInt32 m_unRobotId;

   /* Buzz VM state */
   buzzvm_t m_tBuzzVM;
   /* Name of the bytecode file */
   std::string m_strBytecodeFName;
   /* The actual bytecode */
   CByteArray m_cBytecode;

};

#endif
