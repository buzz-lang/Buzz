#include "buzz_controller_eyebot.h"

/****************************************/
/****************************************/

static int BuzzTakeOff(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   int cont = reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->TakeOff();
   buzzvm_pushi(vm, cont);
   return buzzvm_ret1(vm);
}

static int BuzzLand(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   int cont = reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->Land();
   buzzvm_pushi(vm, cont);
   return buzzvm_ret1(vm);
}

static int BuzzGoToC(buzzvm_t vm) {
   /* Push the vector components */
   buzzvm_lload(vm, 1);
   buzzvm_lload(vm, 2);
   /* Create a new vector with that */
   CVector3 cDir;
   buzzobj_t tX = buzzvm_stack_at(vm, 2);
   buzzobj_t tY = buzzvm_stack_at(vm, 1);
   if(tX->o.type == BUZZTYPE_INT) cDir.SetX(tX->i.value);
   else if(tX->o.type == BUZZTYPE_FLOAT) cDir.SetX(tX->f.value);
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "gotoc(x,y): expected %s, got %s in first argument",
                      buzztype_desc[BUZZTYPE_FLOAT],
                      buzztype_desc[tX->o.type]
         );
      return vm->state;
   }      
   if(tY->o.type == BUZZTYPE_INT) cDir.SetY(tY->i.value);
   else if(tY->o.type == BUZZTYPE_FLOAT) cDir.SetY(tY->f.value);
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "gotoc(x,y): expected %s, got %s in second argument",
                      buzztype_desc[BUZZTYPE_FLOAT],
                      buzztype_desc[tY->o.type]
         );
      return vm->state;
   }
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->SetDirection(cDir);
   return buzzvm_ret0(vm);
}

static int BuzzGoToP(buzzvm_t vm) {
   /* Push the vector components */
   buzzvm_lload(vm, 1);
   buzzvm_lload(vm, 2);
   /* Create a new vector with that */
   buzzobj_t tLinSpeed = buzzvm_stack_at(vm, 2);
   buzzobj_t tAngSpeed = buzzvm_stack_at(vm, 1);
   Real fLinSpeed = 0.0, fAngSpeed = 0.0;
   if(tLinSpeed->o.type == BUZZTYPE_INT) fLinSpeed = tLinSpeed->i.value;
   else if(tLinSpeed->o.type == BUZZTYPE_FLOAT) fLinSpeed = tLinSpeed->f.value;
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "goto(x,y): expected %s, got %s in first argument",
                      buzztype_desc[BUZZTYPE_FLOAT],
                      buzztype_desc[tLinSpeed->o.type]
         );
      return vm->state;
   }      
   if(tAngSpeed->o.type == BUZZTYPE_INT) fAngSpeed = tAngSpeed->i.value;
   else if(tAngSpeed->o.type == BUZZTYPE_FLOAT) fAngSpeed = tAngSpeed->f.value;
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "goto(x,y): expected %s, got %s in second argument",
                      buzztype_desc[BUZZTYPE_FLOAT],
                      buzztype_desc[tAngSpeed->o.type]
         );
      return vm->state;
   }
   CVector3 cDir(fLinSpeed, CRadians(fAngSpeed), CRadians::ZERO);
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->SetDirection(cDir);
   return buzzvm_ret0(vm);
}

static int BuzzRotate(buzzvm_t vm) {
   /* Push the yaw angle */
   buzzvm_lload(vm, 1);
   /* Create a new angle with that */
   CRadians cYaw(buzzvm_stack_at(vm, 1)->f.value);
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->SetYaw(cYaw);
   return buzzvm_ret0(vm);
}

static int BuzzSetLEDs(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 3);
   /* Push the color components */
   buzzvm_lload(vm, 1);
   buzzvm_lload(vm, 2);
   buzzvm_lload(vm, 3);
   buzzvm_type_assert_number(vm, 3);
   buzzvm_type_assert_number(vm, 2);
   buzzvm_type_assert_number(vm, 1);
   /* Create a new color with that */
   CColor cColor(buzzvm_stack_number_to_int(vm, 3),
                 buzzvm_stack_number_to_int(vm, 2),
                 buzzvm_stack_number_to_int(vm, 1));
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->SetLEDs(cColor);
   return buzzvm_ret0(vm);
}

static int BuzzSetLED(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 4);
   /* Push the color components */
   buzzvm_lload(vm, 1); // idx
   buzzvm_lload(vm, 2); // red
   buzzvm_lload(vm, 3); // green
   buzzvm_lload(vm, 4); // blue
   buzzvm_type_assert_number(vm, 4);
   buzzvm_type_assert_number(vm, 3);
   buzzvm_type_assert_number(vm, 2);
   buzzvm_type_assert_number(vm, 1);
   /* Create a new color with that */
   UInt32 unIdx = buzzvm_stack_number_to_int(vm, 4);
   CColor cColor(buzzvm_stack_number_to_int(vm, 3),
                 buzzvm_stack_number_to_int(vm, 2),
                 buzzvm_stack_number_to_int(vm, 1));
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->
      SetLED(unIdx, cColor);
   return buzzvm_ret0(vm);
}

static int BuzzCameraEnable(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->CameraEnable();
   return buzzvm_ret0(vm);
}

static int BuzzCameraDisable(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerEyeBot*>(buzzvm_stack_at(vm, 1)->u.value)->CameraDisable();
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

CBuzzControllerEyeBot::CBuzzControllerEyeBot() :
   m_pcPropellers(NULL),
   m_pcLEDs(NULL),
   m_pcCamera(NULL) {
}

/****************************************/
/****************************************/

CBuzzControllerEyeBot::~CBuzzControllerEyeBot() {
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::Init(TConfigurationNode& t_node) {
   /* Get pointers to devices */
   try { m_pcPropellers = GetActuator<CCI_QuadRotorPositionActuator>("quadrotor_position"); }
   catch(CARGoSException& ex) {}
   try { m_pcLEDs = GetActuator<CCI_LEDsActuator>("leds"); }
   catch(CARGoSException& ex) {}
   try { m_pcCamera = GetSensor<CCI_ColoredBlobPerspectiveCameraSensor>("colored_blob_perspective_camera"); }
   catch(CARGoSException& ex) {}
   /* Initialize the rest */
   try {
      CBuzzController::Init(t_node);
   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED("Error initializing the Buzz controller for the eye-bot", ex);
   }
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::UpdateSensors() {
   /*
    * Update generic sensors
    */
   CBuzzController::UpdateSensors();
   /*
    * Camera
    */
   if(m_pcCamera) {
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "blobs", 1));
      buzzvm_pusht(m_tBuzzVM);
      buzzobj_t tBlobs = buzzvm_stack_at(m_tBuzzVM, 1);
      buzzvm_gstore(m_tBuzzVM);
      const CCI_ColoredBlobPerspectiveCameraSensor::SReadings& sBlobs = m_pcCamera->GetReadings();
      for(size_t i = 0; i < sBlobs.BlobList.size(); ++i) {
         buzzvm_pusht(m_tBuzzVM);
         buzzobj_t tEntry = buzzvm_stack_at(m_tBuzzVM, 1);
         buzzvm_pop(m_tBuzzVM);
         TablePut(tBlobs, i, tEntry);
         TablePut(tEntry, "px",    sBlobs.BlobList[i]->X);
         TablePut(tEntry, "py",    sBlobs.BlobList[i]->Y);
         TablePut(tEntry, "color", sBlobs.BlobList[i]->Color);
      }
   }
}

/****************************************/
/****************************************/

bool CBuzzControllerEyeBot::TakeOff() {
   CVector3 cPos = m_pcPos->GetReading().Position;
   if(Abs(cPos.GetZ() - 2.0f) < 0.01f) return false;
   cPos.SetZ(2.0f);
   m_pcPropellers->SetAbsolutePosition(cPos);
   return true;
}

/****************************************/
/****************************************/

bool CBuzzControllerEyeBot::Land() {
   CVector3 cPos = m_pcPos->GetReading().Position;
   if(Abs(cPos.GetZ()) < 0.01f) return false;
   cPos.SetZ(0.0f);
   m_pcPropellers->SetAbsolutePosition(cPos);
   return true;
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::SetDirection(const CVector3& c_heading) {
   CVector3 cDir = c_heading;
   if(cDir.SquareLength() > 1.0) {
      cDir.Normalize();
   }
   m_pcPropellers->SetRelativePosition(cDir);
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::SetYaw(const CRadians& c_yaw) {
   m_pcPropellers->SetRelativeYaw(c_yaw);
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::SetLEDs(const CColor& c_color) {
   m_pcLEDs->SetAllColors(c_color);
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::SetLED(UInt32 un_idx,
                                   const CColor& c_color) {
   m_pcLEDs->SetSingleColor(un_idx, c_color);
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::CameraEnable() {
   m_pcCamera->Enable();
}

/****************************************/
/****************************************/

void CBuzzControllerEyeBot::CameraDisable() {
   m_pcCamera->Disable();
}

/****************************************/
/****************************************/

buzzvm_state CBuzzControllerEyeBot::RegisterFunctions() {
   /* Register base functions */
   if(CBuzzController::RegisterFunctions() != BUZZVM_STATE_READY)
      return m_tBuzzVM->state;
   /* BuzzTakeOff */
   if(m_pcPropellers && m_pcPos) {
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "takeoff", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzTakeOff));
      buzzvm_gstore(m_tBuzzVM);
   }
   /* BuzzLand */
   if(m_pcPropellers && m_pcPos) {
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "land", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzLand));
      buzzvm_gstore(m_tBuzzVM);
   }
   /* BuzzGoTo */
   if(m_pcPropellers) {
      /* With cartesian coordinates */
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "goto", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzGoToC));
      buzzvm_gstore(m_tBuzzVM);
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "gotoc", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzGoToC));
      buzzvm_gstore(m_tBuzzVM);
      /* With polar coordinates */
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "gotop", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzGoToP));
      buzzvm_gstore(m_tBuzzVM);
   }
   /* BuzzRotate */
   if(m_pcPropellers) {
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "rotate", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzRotate));
      buzzvm_gstore(m_tBuzzVM);
   }
   if(m_pcLEDs) {
      /* BuzzSetLEDs */
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "set_leds", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzSetLEDs));
      buzzvm_gstore(m_tBuzzVM);
      /* BuzzSetLED */
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "set_led", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzSetLED));
      buzzvm_gstore(m_tBuzzVM);
   }
   /* BuzzCameraEnable */
   if(m_pcCamera) {
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "camera_enable", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzCameraEnable));
      buzzvm_gstore(m_tBuzzVM);
   }
   /* BuzzCameraDisable */
   if(m_pcCamera) {
      buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "camera_disable", 1));
      buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzCameraDisable));
      buzzvm_gstore(m_tBuzzVM);
   }
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

REGISTER_CONTROLLER(CBuzzControllerEyeBot, "buzz_controller_eyebot");

#include <argos3/plugins/robots/eye-bot/simulator/eyebot_entity.h>
REGISTER_BUZZ_ROBOT(CEyeBotEntity);
