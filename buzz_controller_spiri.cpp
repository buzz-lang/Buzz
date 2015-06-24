#include "buzz_controller_spiri.h"

/****************************************/
/****************************************/

static int BuzzTakeOff(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller"));
   buzzvm_gload(vm);
   /* Call function */
   int cont = reinterpret_cast<CBuzzControllerSpiri*>(buzzvm_stack_at(vm, 1)->u.value)->TakeOff();
   buzzvm_pushi(vm, cont);
   return buzzvm_ret1(vm);
}

static int BuzzLand(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller"));
   buzzvm_gload(vm);
   /* Call function */
   int cont = reinterpret_cast<CBuzzControllerSpiri*>(buzzvm_stack_at(vm, 1)->u.value)->Land();
   buzzvm_pushi(vm, cont);
   return buzzvm_ret1(vm);
}

static int BuzzGoTo(buzzvm_t vm) {
   /* Push the vector components */
   buzzvm_lload(vm, 1);
   buzzvm_lload(vm, 2);
   /* Create a new vector with that */
   CVector3 cDir(buzzvm_stack_at(vm, 2)->f.value,
                 buzzvm_stack_at(vm, 1)->f.value,
                 0.0f);
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller"));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerSpiri*>(buzzvm_stack_at(vm, 1)->u.value)->SetDirection(cDir);
   return buzzvm_ret0(vm);
}

static int BuzzCameraEnable(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller"));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerSpiri*>(buzzvm_stack_at(vm, 1)->u.value)->CameraEnable();
   return buzzvm_ret0(vm);
}

static int BuzzCameraDisable(buzzvm_t vm) {
   /* Get pointer to the controller */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "controller"));
   buzzvm_gload(vm);
   /* Call function */
   reinterpret_cast<CBuzzControllerSpiri*>(buzzvm_stack_at(vm, 1)->u.value)->CameraDisable();
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

CBuzzControllerSpiri::CBuzzControllerSpiri() :
   m_pcPropellers(NULL) {
}

/****************************************/
/****************************************/

CBuzzControllerSpiri::~CBuzzControllerSpiri() {
}

/****************************************/
/****************************************/

void CBuzzControllerSpiri::Init(TConfigurationNode& t_node) {
   try {
      /* Get pointers to devices */
      m_pcPropellers = GetActuator<CCI_QuadRotorPositionActuator>         ("quadrotor_position");
      m_pcCamera     = GetSensor  <CCI_ColoredBlobPerspectiveCameraSensor>("colored_blob_perspective_camera");
      m_pcPosition   = GetSensor  <CCI_PositioningSensor>                 ("positioning");
      /* Initialize the rest */
      CBuzzController::Init(t_node);
   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED("Error initializing the Buzz controller for the spiri", ex);
   }
}

/****************************************/
/****************************************/

void CBuzzControllerSpiri::UpdateSensors() {
   /* Positioning */
   Register("position", m_pcPosition->GetReading().Position);
   Register("orientation", m_pcPosition->GetReading().Orientation);
   /* Camera */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "blobs"));
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

/****************************************/
/****************************************/

bool CBuzzControllerSpiri::TakeOff() {
   CVector3 cPos = m_pcPosition->GetReading().Position;
   if(Abs(cPos.GetZ() - 2.0f) < 0.01f) return false;
   cPos.SetZ(2.0f);
   m_pcPropellers->SetAbsolutePosition(cPos);
   return true;
}

/****************************************/
/****************************************/

bool CBuzzControllerSpiri::Land() {
   CVector3 cPos = m_pcPosition->GetReading().Position;
   if(Abs(cPos.GetZ()) < 0.01f) return false;
   cPos.SetZ(0.0f);
   m_pcPropellers->SetAbsolutePosition(cPos);
   return true;
}

/****************************************/
/****************************************/

void CBuzzControllerSpiri::SetDirection(const CVector3& c_heading) {
   CVector3 cDir = c_heading;
   if(cDir.SquareLength() > 0.01f) {
      cDir.Normalize();
      cDir *= 0.01;
   }
   m_pcPropellers->SetRelativePosition(cDir);
}

/****************************************/
/****************************************/

void CBuzzControllerSpiri::CameraEnable() {
   m_pcCamera->Enable();
}

/****************************************/
/****************************************/

void CBuzzControllerSpiri::CameraDisable() {
   m_pcCamera->Disable();
}

/****************************************/
/****************************************/

buzzvm_state CBuzzControllerSpiri::RegisterFunctions() {
   /* Register base functions */
   CBuzzController::RegisterFunctions();
   /* BuzzTakeOff */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "takeoff"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzTakeOff));
   buzzvm_gstore(m_tBuzzVM);
   /* BuzzLand */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "land"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzLand));
   buzzvm_gstore(m_tBuzzVM);
   /* BuzzGoTo */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "goto"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzGoTo));
   buzzvm_gstore(m_tBuzzVM);
   /* BuzzCameraEnable */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "camera_enable"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzCameraEnable));
   buzzvm_gstore(m_tBuzzVM);
   /* BuzzCameraDisable */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "camera_disable"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzCameraDisable));
   buzzvm_gstore(m_tBuzzVM);
   return m_tBuzzVM->state;
}

/****************************************/
/****************************************/

REGISTER_CONTROLLER(CBuzzControllerSpiri, "buzz_controller_spiri");
