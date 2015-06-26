#ifndef BUZZ_CONTROLLER_H
#define BUZZ_CONTROLLER_H

#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
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

   virtual void SetBytecode(const std::string& str_fname);

protected:

   virtual buzzvm_state RegisterFunctions();

   virtual void ProcessInMsgs();
   virtual void ProcessOutMsgs();

   virtual void UpdateSensors();

   buzzvm_state Register(const std::string& str_key,
                         buzzobj_t t_obj);

   buzzvm_state Register(const std::string& str_key,
                         SInt32 n_value);

   buzzvm_state Register(const std::string& str_key,
                         Real f_value);

   buzzvm_state Register(const std::string& str_key,
                         const CRadians& c_angle);

   buzzvm_state Register(const std::string& str_key,
                         const CVector3& c_vec);
   
   buzzvm_state Register(const std::string& str_key,
                         const CQuaternion& c_quat);
   
   buzzvm_state Register(const std::string& str_key,
                         const CColor& c_color);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         buzzobj_t t_obj);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         SInt32 n_value);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         Real f_value);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         const CRadians& c_angle);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         const CVector3& c_vec);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         const CQuaternion& c_quat);

   buzzvm_state TablePut(buzzobj_t t_table,
                         const std::string& str_key,
                         const CColor& c_color);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         buzzobj_t t_obj);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         SInt32 n_value);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         Real f_value);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         const CRadians& c_angle);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         const CVector3& c_vec);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         const CQuaternion& c_quat);

   buzzvm_state TablePut(buzzobj_t t_table,
                         SInt32 n_idx,
                         const CColor& c_color);

protected:

   /* Pointer to the range and bearing actuator */
   CCI_RangeAndBearingActuator*  m_pcRABA;
   /* Pointer to the range and bearing sensor */
   CCI_RangeAndBearingSensor* m_pcRABS;

   /* The robot numeric id */
   UInt16 m_unRobotId;
   /* Buzz VM state */
   buzzvm_t m_tBuzzVM;
   /* Name of the bytecode file */
   std::string m_strBytecodeFName;
   /* The actual bytecode */
   CByteArray m_cBytecode;

};

#endif
