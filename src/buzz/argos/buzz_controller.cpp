#include "buzz_controller.h"
#include <buzz/buzzasm.h>
#include <buzz/buzzdebug.h>
#include <cstdlib>
#include <fstream>
#include <cerrno>
#include <argos3/core/utility/logging/argos_log.h>

/****************************************/
/****************************************/

pthread_mutex_t CBuzzController::TRAJECTORY_MUTEX;
CSet<CBuzzController*> CBuzzController::TRAJECTORY_CONTROLLERS;

/*
 * A class used to trick the linker to initialize the trajectory mutex
 * during static initialization.
 */
class CBuzzControllerMutexInitializer {
public:
   CBuzzControllerMutexInitializer() {
      pthread_mutex_init(&CBuzzController::TRAJECTORY_MUTEX, NULL);
   }
} __cBuzzControllerMutexInitializer;

/****************************************/
/****************************************/

int BuzzLOG (buzzvm_t vm) {
   LOG << "BUZZ: ";
   for(UInt32 i = 1; i < buzzdarray_size(vm->lsyms->syms); ++i) {
      buzzvm_lload(vm, i);
      buzzobj_t o = buzzvm_stack_at(vm, 1);
      buzzvm_pop(vm);
      switch(o->o.type) {
         case BUZZTYPE_NIL:
            LOG << "[nil]";
            break;
         case BUZZTYPE_INT:
            LOG << o->i.value;
            break;
         case BUZZTYPE_FLOAT:
            LOG << o->f.value;
            break;
         case BUZZTYPE_TABLE:
            LOG << "[table with " << (buzzdict_size(o->t.value)) << " elems]";
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative)
               LOG << "[n-closure @" << o->c.value.ref << "]";
            else
               LOG << "[c-closure @" << o->c.value.ref << "]";
            break;
         case BUZZTYPE_STRING:
            LOG << o->s.value.str;
            break;
         case BUZZTYPE_USERDATA:
            LOG << "[userdata @" << o->u.value << "]";
            break;
         default:
            break;
      }
   }
   LOG << std::endl;
   LOG.Flush();
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int BuzzDebugPrint(buzzvm_t vm) {
   /* Get pointer to controller user data */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_USERDATA);
   CBuzzController& cContr = *reinterpret_cast<CBuzzController*>(buzzvm_stack_at(vm, 1)->u.value);
   /* Fill message */
   std::ostringstream oss;
   for(UInt32 i = 1; i < buzzdarray_size(vm->lsyms->syms); ++i) {
      buzzvm_lload(vm, i);
      buzzobj_t o = buzzvm_stack_at(vm, 1);
      buzzvm_pop(vm);
      switch(o->o.type) {
         case BUZZTYPE_NIL:
            oss << "[nil]";
            break;
         case BUZZTYPE_INT:
            oss << o->i.value;
            break;
         case BUZZTYPE_FLOAT:
            oss << o->f.value;
            break;
         case BUZZTYPE_TABLE:
            oss << "[table with " << (buzzdict_size(o->t.value)) << " elems]";
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative)
               oss << "[n-closure @" << o->c.value.ref << "]";
            else
               oss << "[c-closure @" << o->c.value.ref << "]";
            break;
         case BUZZTYPE_STRING:
            oss << o->s.value.str;
            break;
         case BUZZTYPE_USERDATA:
            oss << "[userdata @" << o->u.value << "]";
            break;
         default:
            break;
      }
   }
   cContr.GetARGoSDebugInfo().Msg = oss.str();
   return buzzvm_ret0(vm);
}

int BuzzDebugTrajectoryEnable(buzzvm_t vm) {
   /*
    * Possible signatures
    * debug.trajectory.enable(maxpoints,r,g,b)
    *    enable trajectory tracking setting how many points should be stored and the drawing color
    * debug.trajectory.enable(maxpoints)
    *    enable trajectory tracking setting how many points should be stored
    * debug.trajectory.enable(r,g,b)
    *    enable trajectory tracking keeping maxpoints' last value and setting the drawing color
    * debug.trajectory.enable()
    *    enable trajectory tracking keeping maxpoints' last value (default is 30)
    */
   /* Get pointer to controller user data */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_USERDATA);
   CBuzzController* pcContr = reinterpret_cast<CBuzzController*>(buzzvm_stack_at(vm, 1)->u.value);
   /* Get last known value for max points */
   SInt32 nMaxPoints = pcContr->GetARGoSDebugInfo().Trajectory.MaxPoints;
   /* Parse arguments */
   if(buzzvm_lnum(vm) == 4) {
      /* Max points */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      nMaxPoints = buzzvm_stack_at(vm, 1)->i.value;
      /* RGB drawing color */
      buzzvm_lload(vm, 2);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 3);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 4);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      pcContr->GetARGoSDebugInfo().Trajectory.Color.Set(
         buzzvm_stack_at(vm, 3)->i.value,
         buzzvm_stack_at(vm, 2)->i.value,
         buzzvm_stack_at(vm, 1)->i.value);
   }
   else if(buzzvm_lnum(vm) == 3) {
      /* RGB drawing color */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 2);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 3);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      pcContr->GetARGoSDebugInfo().Trajectory.Color.Set(
         buzzvm_stack_at(vm, 3)->i.value,
         buzzvm_stack_at(vm, 2)->i.value,
         buzzvm_stack_at(vm, 1)->i.value);
   }
   else if(buzzvm_lnum(vm) == 1) {
      /* Max points */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      nMaxPoints = buzzvm_stack_at(vm, 1)->i.value;
   }
   else if(buzzvm_lnum(vm) != 0) {
      /* Bomb out */
      buzzvm_seterror(vm, BUZZVM_ERROR_LNUM, "expected 4, 3, or 1 arguments, but %" PRId64 " were passed", buzzvm_lnum(vm));
   }
   /* Call method */
   CBuzzController::DebugTrajectoryEnable(pcContr, nMaxPoints);
   return buzzvm_ret0(vm);
}

int BuzzDebugTrajectoryDisable(buzzvm_t vm) {
   /*
    * Possible signatures
    * debug.trajectory.disable()
    *    disables trajectory tracking
    */
   /* Get pointer to controller user data */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_USERDATA);
   CBuzzController* pcContr = reinterpret_cast<CBuzzController*>(buzzvm_stack_at(vm, 1)->u.value);
   /* Call method */
   CBuzzController::DebugTrajectoryDisable(pcContr);
   return buzzvm_ret0(vm);
}

int BuzzDebugTrajectoryClear(buzzvm_t vm) {
   /*
    * Possible signatures
    * debug.trajectory.clear()
    *    deletes all the trajectory points
    */
   /* Get pointer to controller user data */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_USERDATA);
   CBuzzController& cContr = *reinterpret_cast<CBuzzController*>(buzzvm_stack_at(vm, 1)->u.value);
   cContr.GetARGoSDebugInfo().TrajectoryClear();
   return buzzvm_ret0(vm);
}

int BuzzDebugRayAdd(buzzvm_t vm) {
   /*
    * Possible signatures
    * debug.rays.add(r,g,b, x,y,z)
    *    draws a ray from the reference point of the robot to (x,y,z).
    *    (x,y,z) is expressed wrt the robot reference frame
    * debug.rays.add(r,g,b, x0,y0,z0, x1,y1,z1)
    *    draws a ray from (x0,y0,z0) to (x1,y1,z1)
    *    (x0,y0,z0) and (x1,y1,z1) are expressed wrt the robot reference frame
   */
   CColor cColor;
   CVector3 cStart, cEnd;
   /* Parse arguments */
   int64_t argn = buzzvm_lnum(vm);
   if(argn == 6) {
      /* Parse color */
      buzzvm_lload(vm, 1); /* red */
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 2); /* green */
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 3); /* blue */
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      cColor.Set(buzzvm_stack_at(vm, 3)->i.value,
                 buzzvm_stack_at(vm, 2)->i.value,
                 buzzvm_stack_at(vm, 1)->i.value);
      /* Parse end vector */
      buzzvm_lload(vm, 4); /* x */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      buzzvm_lload(vm, 5); /* y */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      buzzvm_lload(vm, 6); /* z */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      cEnd.Set(buzzvm_stack_at(vm, 3)->f.value,
               buzzvm_stack_at(vm, 2)->f.value,
               buzzvm_stack_at(vm, 1)->f.value);
   }
   else if(argn == 9) {
      /* Parse color */
      buzzvm_lload(vm, 1); /* red */
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 2); /* green */
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      buzzvm_lload(vm, 3); /* blue */
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      cColor.Set(buzzvm_stack_at(vm, 3)->i.value,
                 buzzvm_stack_at(vm, 2)->i.value,
                 buzzvm_stack_at(vm, 1)->i.value);
      /* Parse start vector */
      buzzvm_lload(vm, 4); /* x */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      buzzvm_lload(vm, 5); /* y */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      buzzvm_lload(vm, 6); /* z */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      cStart.Set(buzzvm_stack_at(vm, 3)->f.value,
                 buzzvm_stack_at(vm, 2)->f.value,
                 buzzvm_stack_at(vm, 1)->f.value);
      /* Parse end vector */
      buzzvm_lload(vm, 7); /* x */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      buzzvm_lload(vm, 8); /* y */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      buzzvm_lload(vm, 9); /* z */
      buzzvm_type_assert(vm, 1, BUZZTYPE_FLOAT);
      cEnd.Set(buzzvm_stack_at(vm, 3)->f.value,
               buzzvm_stack_at(vm, 2)->f.value,
               buzzvm_stack_at(vm, 1)->f.value);
   }
   else {
      /* Bomb out */
      buzzvm_seterror(vm, BUZZVM_ERROR_LNUM, "expected 6 or 9 arguments, but %" PRId64 " were passed", buzzvm_lnum(vm));
   }
   /* Get pointer to controller user data */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_USERDATA);
   CBuzzController& cContr = *reinterpret_cast<CBuzzController*>(buzzvm_stack_at(vm, 1)->u.value);
   /* Call method */
   cContr.GetARGoSDebugInfo().RayAdd(cColor, cStart, cEnd);
   return buzzvm_ret0(vm);
}

int BuzzDebugRayClear(buzzvm_t vm) {
   /*
    * Possible signatures
    * debug.rays.clear()
    *    deletes all the rays
    */
   /* Get pointer to controller user data */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   buzzvm_type_assert(vm, 1, BUZZTYPE_USERDATA);
   CBuzzController& cContr = *reinterpret_cast<CBuzzController*>(buzzvm_stack_at(vm, 1)->u.value);
   /* Call method */
   cContr.GetARGoSDebugInfo().RayClear();
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

CBuzzController::SDebug::SRay::SRay(const CColor& c_color,
                                    const CVector3& c_start,
                                    const CVector3& c_end) :
   Ray(c_start, c_end),
   Color(c_color) {}

/****************************************/
/****************************************/

CBuzzController::SDebug::SDebug() {
   Trajectory.Tracking = false;
   Trajectory.MaxPoints = 50;
}

/****************************************/
/****************************************/

CBuzzController::SDebug::~SDebug() {
   TrajectoryClear();
   RayClear();
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::Clear() {
   Msg = "";
   RayClear();
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::TrajectoryEnable(SInt32 n_size) {
   Trajectory.Tracking = true;
   Trajectory.MaxPoints = n_size;
   while(Trajectory.Data.size() > Trajectory.MaxPoints)
      Trajectory.Data.pop_back();
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::TrajectoryDisable() {
   Trajectory.Tracking = false;
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::TrajectoryAdd(const CVector3& c_pos) {
   Trajectory.Data.push_front(c_pos);
   while(Trajectory.Data.size() > Trajectory.MaxPoints) {
      Trajectory.Data.pop_back();
   }
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::TrajectoryClear() {
   while(!Trajectory.Data.empty()) {
      Trajectory.Data.pop_back();
   }
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::RayAdd(const CColor& c_color,
                                     const CVector3& c_start,
                                     const CVector3& c_end) {
   Rays.push_back(new SRay(c_color, c_start, c_end));
}

/****************************************/
/****************************************/

void CBuzzController::SDebug::RayClear() {
   while(!Rays.empty()) {
      delete Rays.back();
      Rays.pop_back();
   }
}

/****************************************/
/****************************************/

CBuzzController::CBuzzController() :
   m_pcRABA(NULL),
   m_pcRABS(NULL),
   m_pcPos(NULL),
   m_pcBattery(NULL),
   m_tBuzzVM(NULL),
   m_tBuzzDbgInfo(NULL) {}

/****************************************/
/****************************************/

CBuzzController::~CBuzzController() {
}

/****************************************/
/****************************************/

void CBuzzController::Init(TConfigurationNode& t_node) {
   try {
      /* Get pointers to devices */
      m_pcRABA   = GetActuator<CCI_RangeAndBearingActuator>("range_and_bearing");
      m_pcRABS   = GetSensor  <CCI_RangeAndBearingSensor  >("range_and_bearing");
      try {
         m_pcPos = GetSensor  <CCI_PositioningSensor>("positioning");
      }
      catch(CARGoSException& ex) {}
      try {
         m_pcBattery = GetSensor<CCI_BatterySensor>("battery");
      }
      catch(CARGoSException& ex) {}
      /* Get the script name */
      std::string strBCFName;
      GetNodeAttributeOrDefault(t_node, "bytecode_file", strBCFName, strBCFName);
      /* Get the script name */
      std::string strDbgFName;
      GetNodeAttributeOrDefault(t_node, "debug_file", strDbgFName, strDbgFName);
      /* Initialize the rest */
      bool bIDSuccess = false;
      m_unRobotId = 0;
      /* Find Buzz ID */
      size_t tStartPos = GetId().find_last_of("_");
      if(tStartPos != std::string::npos){
         /* Checks for ID after last "_" ie. footbot_group3_10 -> 10 */
         m_unRobotId = FromString<UInt16>(GetId().substr(tStartPos+1));
         bIDSuccess = true;
      }
      /* FromString() returns 0 if passed an invalid string */
      if(!m_unRobotId || !bIDSuccess){
         /* Checks for ID after first number footbot_simulated10 -> 10 */
         tStartPos = GetId().find_first_of("0123456789");
         if(tStartPos != std::string::npos){
            m_unRobotId = FromString<UInt16>(GetId().substr(tStartPos));
            bIDSuccess = true;
         }
      }
      if(!bIDSuccess) {
            THROW_ARGOSEXCEPTION("Error in finding Buzz ID from name \"" << GetId() << "\"");
      }
      if(strBCFName != "" && strDbgFName != "")
         SetBytecode(strBCFName, strDbgFName);
      else
         m_tBuzzVM = buzzvm_new(m_unRobotId);
      UpdateSensors();
      /* Set initial robot message (id and then all zeros) */
      CByteArray cData;
      cData << m_tBuzzVM->robot;
      while(cData.Size() < m_pcRABA->GetSize()) cData << static_cast<UInt8>(0);
      m_pcRABA->SetData(cData);
   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED("Error initializing the Buzz controller", ex);
   }
}

/****************************************/
/****************************************/

void CBuzzController::Reset() {
   buzzvm_function_call(m_tBuzzVM, "reset", 0);
   if(buzzvm_function_call(m_tBuzzVM, "step", 0) != BUZZVM_STATE_READY) {
      fprintf(stderr, "[ROBOT %u] %s: execution terminated abnormally: %s\n\n",
              m_tBuzzVM->robot,
              m_strBytecodeFName.c_str(),
              ErrorInfo().c_str());
      for(UInt32 i = 1; i <= buzzdarray_size(m_tBuzzVM->stacks); ++i) {
         buzzdebug_stack_dump(m_tBuzzVM, i, stdout);
      }
      return;
   }
   /* Reset debug information */
   m_sDebug.Clear();
   m_sDebug.TrajectoryClear();
   m_sDebug.TrajectoryDisable();
   m_sDebug.RayClear();
   try {
      /* Set the bytecode again */
      if(m_strBytecodeFName != "" && m_strDbgInfoFName != "")
         SetBytecode(m_strBytecodeFName, m_strDbgInfoFName);
      UpdateSensors();
   }
   catch(CARGoSException& ex) {
      LOGERR << ex.what();
   }
}

/****************************************/
/****************************************/

void CBuzzController::ControlStep() {
   /* Update debugging information */
   m_sDebug.Clear();
   if(m_sDebug.Trajectory.Tracking) {
      const CCI_PositioningSensor::SReading& sPosRead = m_pcPos->GetReading();
      m_sDebug.TrajectoryAdd(sPosRead.Position);
   }
   /* Take care of the rest */
   if(m_tBuzzVM && m_tBuzzVM->state == BUZZVM_STATE_READY) {
      ProcessInMsgs();
      UpdateSensors();
      if(buzzvm_function_call(m_tBuzzVM, "step", 0) != BUZZVM_STATE_READY) {
         fprintf(stderr, "[ROBOT %u] %s: execution terminated abnormally: %s\n\n",
                 m_tBuzzVM->robot,
                 m_strBytecodeFName.c_str(),
                 ErrorInfo().c_str());
         for(UInt32 i = 1; i <= buzzdarray_size(m_tBuzzVM->stacks); ++i) {
            buzzdebug_stack_dump(m_tBuzzVM, i, stdout);
         }
         return;
      }
      /* Remove useless return value from stack */
      buzzvm_pop(m_tBuzzVM);
      ProcessOutMsgs();
   }
   else {
      fprintf(stderr, "[ROBOT %s] Robot is not ready to execute Buzz script.\n\n",
              GetId().c_str());
   }
}

/****************************************/
/****************************************/

void CBuzzController::Destroy() {
   /* Get rid of the VM */
   if(m_tBuzzVM) {
      buzzvm_function_call(m_tBuzzVM, "destroy", 0);
      buzzvm_destroy(&m_tBuzzVM);
      if(m_tBuzzDbgInfo) buzzdebug_destroy(&m_tBuzzDbgInfo);
   }
}

/****************************************/
/****************************************/

void CBuzzController::SetBytecode(const std::string& str_bc_fname,
                                  const std::string& str_dbg_fname) {
   /* Reset the BuzzVM */
   if(m_tBuzzVM) buzzvm_destroy(&m_tBuzzVM);
   m_tBuzzVM = buzzvm_new(m_unRobotId);
   /* Get rid of debug info */
   if(m_tBuzzDbgInfo) buzzdebug_destroy(&m_tBuzzDbgInfo);
   m_tBuzzDbgInfo = buzzdebug_new();
   /* Save the filenames */
   m_strBytecodeFName = str_bc_fname;
   m_strDbgInfoFName = str_dbg_fname;
   /* Load the bytecode */
   std::ifstream cBCodeFile(str_bc_fname.c_str(), std::ios::binary | std::ios::ate);
   if(cBCodeFile.fail()) {
      THROW_ARGOSEXCEPTION("Can't open file \"" << str_bc_fname << "\": " << strerror(errno));
   }
   std::ifstream::pos_type unFileSize = cBCodeFile.tellg();
   m_cBytecode.Clear();
   m_cBytecode.Resize(unFileSize);
   cBCodeFile.seekg(0, std::ios::beg);
   cBCodeFile.read(reinterpret_cast<char*>(m_cBytecode.ToCArray()), unFileSize);
   /* Load the debug symbols */
   if(!buzzdebug_fromfile(m_tBuzzDbgInfo, m_strDbgInfoFName.c_str())) {
      THROW_ARGOSEXCEPTION("Can't open file \"" << str_dbg_fname << "\": " << strerror(errno));
   }
   /* Load the script */
   if(buzzvm_set_bcode(m_tBuzzVM, m_cBytecode.ToCArray(), m_cBytecode.Size()) != BUZZVM_STATE_READY) {
      THROW_ARGOSEXCEPTION("Error loading Buzz script \"" << str_bc_fname << "\": " << ErrorInfo());
   }
   /* Register basic function */
   if(RegisterFunctions() != BUZZVM_STATE_READY) {
      THROW_ARGOSEXCEPTION("Error while registering functions: " << ErrorInfo());
   }
   /* Execute the global part of the script */
   buzzvm_execute_script(m_tBuzzVM);
   /* Call the Init() function */
   if(buzzvm_function_call(m_tBuzzVM, "init", 0) != BUZZVM_STATE_READY) {
      fprintf(stderr, "[ROBOT %u] %s: execution terminated abnormally: %s\n\n",
              m_tBuzzVM->robot,
              m_strBytecodeFName.c_str(),
              ErrorInfo().c_str());
      for(UInt32 i = 1; i <= buzzdarray_size(m_tBuzzVM->stacks); ++i) {
         buzzdebug_stack_dump(m_tBuzzVM, i, stdout);
      }
      return;
   }
   /* Remove useless return value from stack */
   buzzvm_pop(m_tBuzzVM);
}

/****************************************/
/****************************************/

std::string CBuzzController::ErrorInfo() {
   if(m_tBuzzDbgInfo) {
      const buzzdebug_entry_t* ptInfo = buzzdebug_info_get_fromoffset(m_tBuzzDbgInfo, &m_tBuzzVM->oldpc);
      std::ostringstream ossErrMsg;
      if(ptInfo) {
         ossErrMsg << (*ptInfo)->fname
                   << ":"
                   << (*ptInfo)->line
                   << ":"
                   << (*ptInfo)->col;
      }
      else {
         ossErrMsg << "At bytecode offset "
                   << m_tBuzzVM->oldpc;
      }
      if(m_tBuzzVM->errormsg)
         ossErrMsg << ": "
                   << m_tBuzzVM->errormsg;
      else
         ossErrMsg << ": "
                   << buzzvm_error_desc[m_tBuzzVM->error];
      return ossErrMsg.str();
   }
   else {
      return "Script not loaded!";
   }
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::RegisterFunctions() {
   /*
    * Pointer to this controller
    */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "controller", 1));
   buzzvm_pushu(m_tBuzzVM, this);
   buzzvm_gstore(m_tBuzzVM);
   /*
    * BuzzLOG
    */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "log", 1));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzLOG));
   buzzvm_gstore(m_tBuzzVM);
   /*
    * Buzz debug facilities
    */
   /* Initialize debug table */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "debug", 1));
   buzzvm_pusht(m_tBuzzVM);
   /* debug.print() */
   buzzvm_dup(m_tBuzzVM);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "print", 1));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzDebugPrint));
   buzzvm_tput(m_tBuzzVM);
   /* Initialize debug.rays table */
   buzzvm_dup(m_tBuzzVM);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "rays", 1));
   buzzvm_pusht(m_tBuzzVM);
   /* debug.rays.add() */
   buzzvm_dup(m_tBuzzVM);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "add", 1));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzDebugRayAdd));
   buzzvm_tput(m_tBuzzVM);
   /* debug.rays.clear() */
   buzzvm_dup(m_tBuzzVM);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "clear", 1));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzDebugRayClear));
   buzzvm_tput(m_tBuzzVM);
   /* Finalize debug.rays table */
   buzzvm_tput(m_tBuzzVM);
   if(m_pcPos != NULL) {
      /* Initialize debug.trajectory table */
      buzzvm_dup(m_tBuzzVM);
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "trajectory", 1));
      buzzvm_pusht(m_tBuzzVM);
      /* debug.trajectory.enable() */
      buzzvm_dup(m_tBuzzVM);
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "enable", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzDebugTrajectoryEnable));
      buzzvm_tput(m_tBuzzVM);
      /* debug.trajectory.disable() */
      buzzvm_dup(m_tBuzzVM);
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "disable", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzDebugTrajectoryDisable));
      buzzvm_tput(m_tBuzzVM);
      /* debug.trajectory.clear() */
      buzzvm_dup(m_tBuzzVM);
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "clear", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzDebugTrajectoryClear));
      buzzvm_tput(m_tBuzzVM);
      /* Finalize debug.trajectory table */
      buzzvm_tput(m_tBuzzVM);
   }
   /* Finalize debug table */
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

void CBuzzController::ProcessInMsgs() {
   /* Reset neighbor information */
   buzzneighbors_reset(m_tBuzzVM);
   /* Go through RAB messages and add them to the FIFO */
   const CCI_RangeAndBearingSensor::TReadings& tPackets = m_pcRABS->GetReadings();
   for(size_t i = 0; i < tPackets.size(); ++i) {
      /* Copy packet into temporary buffer */
      CByteArray cData = tPackets[i].Data;
      /* Get robot id and update neighbor information */
      UInt16 unRobotId = cData.PopFront<UInt16>();
      buzzneighbors_add(m_tBuzzVM,
                        unRobotId,
                        tPackets[i].Range,
                        tPackets[i].HorizontalBearing.GetValue(),
                        tPackets[i].VerticalBearing.GetValue());
      /* Go through the messages until there's nothing else to read */
      UInt16 unMsgSize;
      do {
         /* Get payload size */
         unMsgSize = cData.PopFront<UInt16>();
         /* Append message to the Buzz input message queue */
         if(unMsgSize > 0 && cData.Size() >= unMsgSize) {
            buzzinmsg_queue_append(m_tBuzzVM,
                                   unRobotId,
                                   buzzmsg_payload_frombuffer(cData.ToCArray(), unMsgSize));
            /* Get rid of the data read */
            for(size_t i = 0; i < unMsgSize; ++i) cData.PopFront<UInt8>();
         }
      }
      while(cData.Size() > sizeof(UInt16) && unMsgSize > 0);
   }
   /* Process messages */
   buzzvm_process_inmsgs(m_tBuzzVM);
}

/****************************************/
/****************************************/

void CBuzzController::ProcessOutMsgs() {
   /* Process outgoing messages */
   buzzvm_process_outmsgs(m_tBuzzVM);
   /* Send robot id */
   CByteArray cData;
   cData << m_tBuzzVM->robot;
   /* Send messages from FIFO */
   do {
      /* Are there more messages? */
      if(buzzoutmsg_queue_isempty(m_tBuzzVM)) break;
      /* Get first message */
      buzzmsg_payload_t m = buzzoutmsg_queue_first(m_tBuzzVM);
      /* Make sure the message is smaller than the data buffer
       * Without this check, large messages would clog the queue forever
       */
      size_t unMsgSize = buzzmsg_payload_size(m) + sizeof(UInt16);
      if(unMsgSize < m_pcRABA->GetSize() - sizeof(UInt16)) {
         /* Make sure the next message fits the data buffer */
         if(cData.Size() + unMsgSize > m_pcRABA->GetSize()) {
            buzzmsg_payload_destroy(&m);
            break;
         }
         /* Add message length to data buffer */
         cData << static_cast<UInt16>(buzzmsg_payload_size(m));
         /* Add payload to data buffer */
         cData.AddBuffer(reinterpret_cast<UInt8*>(m->data), buzzmsg_payload_size(m));
      }
      else {
         RLOGERR << "Discarded oversize message ("
                 << unMsgSize
                 << " bytes). Max size is "
                 << m_pcRABA->GetSize() - sizeof(UInt16)
                 << " bytes."
                 << std::endl;
      }
      /* Get rid of message */
      buzzoutmsg_queue_next(m_tBuzzVM);
      buzzmsg_payload_destroy(&m);
   } while(1);
   /* Pad the rest of the data with zeroes */
   while(cData.Size() < m_pcRABA->GetSize()) cData << static_cast<UInt8>(0);
   /* Send message */
   m_pcRABA->SetData(cData);
   /*
    * Update debug.msgqueue information
    */
   /* Get debug table */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "debug", 1));
   buzzvm_gload(m_tBuzzVM);
   /* Create new debug.msgqueue table */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "msgqueue", 1));
   buzzobj_t tMsgQueue = buzzheap_newobj(m_tBuzzVM, BUZZTYPE_TABLE);
   /* Set debug.msgqueue.total */
   TablePut(tMsgQueue,
            "total",
            static_cast<SInt32>(buzzoutmsg_queue_size(m_tBuzzVM)));
   /* Set debug.msgqueue.broadcast */
   TablePut(tMsgQueue,
            "broadcast",
            static_cast<SInt32>(buzzdarray_size(m_tBuzzVM->outmsgs->queues[BUZZMSG_BROADCAST])));
   /* Set debug.msgqueue.vstig */
   TablePut(tMsgQueue,
            "vstig",
            static_cast<SInt32>(buzzdarray_size(m_tBuzzVM->outmsgs->queues[BUZZMSG_VSTIG_PUT])) + static_cast<SInt32>(buzzdarray_size(m_tBuzzVM->outmsgs->queues[BUZZMSG_VSTIG_QUERY])));
   /* Set debug.msgqueue.swarm */
   TablePut(tMsgQueue,
            "swarm",
            static_cast<SInt32>(buzzdarray_size(m_tBuzzVM->outmsgs->queues[BUZZMSG_SWARM_JOIN])) + static_cast<SInt32>(buzzdarray_size(m_tBuzzVM->outmsgs->queues[BUZZMSG_SWARM_LEAVE])));
   /* Save table */
   buzzvm_push(m_tBuzzVM, tMsgQueue);
   buzzvm_tput(m_tBuzzVM);
}

/****************************************/
/****************************************/

void CBuzzController::UpdateSensors() {
   /*
    * Update positioning sensor
    */
   if(m_pcPos != NULL) {
      /* Get positioning readings */
      const CCI_PositioningSensor::SReading& sPosRead = m_pcPos->GetReading();
      /* Create empty positioning data table */
      buzzobj_t tPose = buzzheap_newobj(m_tBuzzVM, BUZZTYPE_TABLE);
      /* Store position data */
      TablePut(tPose, "position", sPosRead.Position);
      /* Store orientation data */
      TablePut(tPose, "orientation", sPosRead.Orientation);
      /* Register positioning data table as global symbol */
      Register("pose", tPose);
   }
   /*
    * Update the battery sensor
    */
   if(m_pcBattery != NULL) {
      /* Get battery readings */
      const CCI_BatterySensor::SReading& sBatRead = m_pcBattery->GetReading();
      /* Create empty battery data table */
      buzzobj_t tBattery = buzzheap_newobj(m_tBuzzVM, BUZZTYPE_TABLE);
      /* Store charge data */
      TablePut(tBattery, "available_charge", sBatRead.AvailableCharge);
      /* Store time data */
      TablePut(tBattery, "time_left", sBatRead.TimeLeft);
      /* Register battery data table as global symbol */
      Register("battery", tBattery);
   }
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       buzzobj_t t_obj) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_push(m_tBuzzVM, t_obj);
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       SInt32 n_value) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushi(m_tBuzzVM, n_value);
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       Real f_value) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushf(m_tBuzzVM, f_value);
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       const CRadians& c_angle) {
   return Register(str_key, c_angle.GetValue());
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       const CVector3& c_vec) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tVecTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_gstore(m_tBuzzVM);
   TablePut(tVecTable, "x", c_vec.GetX());
   TablePut(tVecTable, "y", c_vec.GetY());
   TablePut(tVecTable, "z", c_vec.GetZ());
   return m_tBuzzVM->state;
}
   
/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       const CQuaternion& c_quat) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tQuatTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_gstore(m_tBuzzVM);
   CRadians cYaw, cPitch, cRoll;
   c_quat.ToEulerAngles(cYaw, cPitch, cRoll);
   TablePut(tQuatTable, "yaw", cYaw);
   TablePut(tQuatTable, "pitch", cPitch);
   TablePut(tQuatTable, "roll", cRoll);
   return m_tBuzzVM->state;
}
   
/****************************************/
/****************************************/

buzzvm_state CBuzzController::Register(const std::string& str_key,
                                       const CColor& c_color) {
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tColorTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_gstore(m_tBuzzVM);
   TablePut(tColorTable, "red", c_color.GetRed());
   TablePut(tColorTable, "green", c_color.GetGreen());
   TablePut(tColorTable, "blue", c_color.GetBlue());
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       buzzobj_t t_obj) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_push(m_tBuzzVM, t_obj);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       SInt32 n_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushi(m_tBuzzVM, n_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       Real f_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pushf(m_tBuzzVM, f_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       const CRadians& c_angle) {
   return TablePut(t_table, str_key, c_angle.GetValue());
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       const CVector3& c_vec) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tVecTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_tput(m_tBuzzVM);
   TablePut(tVecTable, "x", c_vec.GetX());
   TablePut(tVecTable, "y", c_vec.GetY());
   TablePut(tVecTable, "z", c_vec.GetZ());
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       const CQuaternion& c_quat) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tQuatTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_tput(m_tBuzzVM);
   CRadians cYaw, cPitch, cRoll;
   c_quat.ToEulerAngles(cYaw, cPitch, cRoll);
   TablePut(tQuatTable, "yaw", cYaw);
   TablePut(tQuatTable, "pitch", cPitch);
   TablePut(tQuatTable, "roll", cRoll);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       const std::string& str_key,
                                       const CColor& c_color) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, str_key.c_str(), 1));
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tColorTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_tput(m_tBuzzVM);
   TablePut(tColorTable, "red", c_color.GetRed());
   TablePut(tColorTable, "green", c_color.GetGreen());
   TablePut(tColorTable, "blue", c_color.GetBlue());
   return m_tBuzzVM->state;   
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       buzzobj_t t_obj) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_push(m_tBuzzVM, t_obj);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       SInt32 n_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pushi(m_tBuzzVM, n_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       Real f_value) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pushf(m_tBuzzVM, f_value);
   buzzvm_tput(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       const CRadians& c_angle) {
   return TablePut(t_table, n_idx, c_angle.GetValue());
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       const CVector3& c_vec) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tVecTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_tput(m_tBuzzVM);
   TablePut(tVecTable, "x", c_vec.GetX());
   TablePut(tVecTable, "y", c_vec.GetY());
   TablePut(tVecTable, "z", c_vec.GetZ());
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       const CQuaternion& c_quat) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tQuatTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_tput(m_tBuzzVM);
   CRadians cYaw, cPitch, cRoll;
   c_quat.ToEulerAngles(cYaw, cPitch, cRoll);
   TablePut(tQuatTable, "yaw", cYaw);
   TablePut(tQuatTable, "pitch", cPitch);
   TablePut(tQuatTable, "roll", cRoll);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

buzzvm_state CBuzzController::TablePut(buzzobj_t t_table,
                                       SInt32 n_idx,
                                       const CColor& c_color) {
   buzzvm_push(m_tBuzzVM, t_table);
   buzzvm_pushi(m_tBuzzVM, n_idx);
   buzzvm_pusht(m_tBuzzVM);
   buzzobj_t tColorTable = buzzvm_stack_at(m_tBuzzVM, 1);
   buzzvm_tput(m_tBuzzVM);
   TablePut(tColorTable, "red", c_color.GetRed());
   TablePut(tColorTable, "green", c_color.GetGreen());
   TablePut(tColorTable, "blue", c_color.GetBlue());
   return m_tBuzzVM->state;   
}

/****************************************/
/****************************************/
