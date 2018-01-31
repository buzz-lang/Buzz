#ifndef BUZZ_CONTROLLER_H
#define BUZZ_CONTROLLER_H

#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
#include <argos3/plugins/robots/generic/control_interface/ci_battery_sensor.h>
#include <argos3/core/utility/math/ray3.h>
#include <argos3/core/utility/datatypes/set.h>
#include <buzz/buzzvm.h>
#include <buzz/buzzdebug.h>
#include <string>
#include <list>

using namespace argos;

class CBuzzController : public CCI_Controller {

public:

   struct SDebug {
      /* A ray and its color */
      struct SRay {
         CRay3 Ray;
         CColor Color;
         SRay(const CColor& c_color,
              const CVector3& c_start,
              const CVector3& c_end);
      };
      /* Trajectory-related data */
      struct {
         /* Whether trajectory tracking is on or off */
         bool Tracking;
         /* Max number of trajectory points to store */
         SInt32 MaxPoints;
         /* Trajectory data */
         std::list<CVector3> Data;
         /* Drawing color */
         CColor Color;
      } Trajectory;
      /* Message */
      std::string Msg;
      /* Rays to draw */
      std::vector<SRay*> Rays;
      /* Constructor */
      SDebug();
      /* Destructor */
      ~SDebug();
      /* Data clearing at each step */
      void Clear();
      /* Trajectory toggling */
      void TrajectoryEnable(SInt32 n_size);
      void TrajectoryDisable();
      void TrajectoryAdd(const CVector3& c_pos);
      void TrajectoryClear();
      /* Ray functions */
      void RayAdd(const CColor& c_color,
                  const CVector3& c_start,
                  const CVector3& c_end);
      void RayClear();
   };

public:

   CBuzzController();
   virtual ~CBuzzController();

   virtual void Init(TConfigurationNode& t_node);
   virtual void Reset();
   virtual void ControlStep();
   virtual void Destroy();

   inline const std::string& GetBytecodeFName() const {
      return m_strBytecodeFName;
   }

   inline const std::string& GetDbgInfoFName() const {
      return m_strDbgInfoFName;
   }

   virtual void SetBytecode(const std::string& str_bc_fname,
                            const std::string& str_dbg_fname);

   inline const buzzvm_t GetBuzzVM() const {
      return m_tBuzzVM;
   }

   inline buzzvm_t GetBuzzVM() {
      return m_tBuzzVM;
   }

   inline SDebug& GetARGoSDebugInfo() {
      return m_sDebug;
   }

   inline const SDebug& GetARGoSDebugInfo() const {
      return m_sDebug;
   }

   inline buzzdebug_t& GetBuzzDbgInfo() {
      return m_tBuzzDbgInfo;
   }

   inline const buzzdebug_t& GetBuzzDbgInfo() const {
      return m_tBuzzDbgInfo;
   }

   std::string ErrorInfo();

   typedef std::map<size_t, bool> TBuzzRobots;
   static TBuzzRobots& BUZZ_ROBOTS() {
      static TBuzzRobots tBuzzRobots;
      return tBuzzRobots;
   }

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

   virtual buzzvm_state RegisterFunctions();

   virtual void ProcessInMsgs();
   virtual void ProcessOutMsgs();

   virtual void UpdateSensors();

protected:

   /* Pointer to the range and bearing actuator */
   CCI_RangeAndBearingActuator*  m_pcRABA;
   /* Pointer to the range and bearing sensor */
   CCI_RangeAndBearingSensor* m_pcRABS;
   /* Pointer to the positioning sensor */
   CCI_PositioningSensor* m_pcPos;
   /* Pointer to the battery sensor */
   CCI_BatterySensor* m_pcBattery;

   /* The robot numeric id */
   UInt16 m_unRobotId;
   /* Buzz VM state */
   buzzvm_t m_tBuzzVM;
   /* Buzz debug info */
   buzzdebug_t m_tBuzzDbgInfo;
   /* Name of the bytecode file */
   std::string m_strBytecodeFName;
   /* Name of the debug info file */
   std::string m_strDbgInfoFName;
   /* The actual bytecode */
   CByteArray m_cBytecode;
   /* Debugging information */
   SDebug m_sDebug;

public:
   
   /* Mutex for trajectory tracking */
   static pthread_mutex_t TRAJECTORY_MUTEX;
   /* List of controllers with trajectory tracking enabled */
   static CSet<CBuzzController*> TRAJECTORY_CONTROLLERS;
   /* Enables trajectory tracking in controllers */
   static void DebugTrajectoryEnable(CBuzzController* pc_cntr,
                                SInt32 n_max_points) {
      pc_cntr->GetARGoSDebugInfo().TrajectoryEnable(n_max_points);
      pthread_mutex_lock(&TRAJECTORY_MUTEX);
      TRAJECTORY_CONTROLLERS.insert(pc_cntr);
      pthread_mutex_unlock(&TRAJECTORY_MUTEX);
   }
   /* Disables trajectory tracking in controllers */
   static void DebugTrajectoryDisable(CBuzzController* pc_cntr) {
      pc_cntr->GetARGoSDebugInfo().TrajectoryDisable();
      pthread_mutex_lock(&TRAJECTORY_MUTEX);
      TRAJECTORY_CONTROLLERS.erase(pc_cntr);
      pthread_mutex_unlock(&TRAJECTORY_MUTEX);
   }
};

#include <argos3/core/utility/plugins/vtable.h>

#define REGISTER_BUZZ_ROBOT(ROBOT_TYPE)                                 \
   class C ## ROBOT_TYPE ## BuzzController ## Proxy {                   \
   public:                                                              \
   C ## ROBOT_TYPE ## BuzzController ## Proxy() {                       \
      CBuzzController::BUZZ_ROBOTS()[GetTag<ROBOT_TYPE,CEntity>()] = true; \
   }                                                                    \
   };                                                                   \
   C ## ROBOT_TYPE ## BuzzController ## Proxy ROBOT_TYPE ## BuzzController ## _p;

#endif
