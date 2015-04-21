#include "buzz_controller.h"
#include "buzzasm.h"
#include <cstdlib>
#include <fstream>

/****************************************/
/****************************************/

void dump(buzzvm_t vm, const char* prefix) {
   fprintf(stderr, "%s============================================================\n", prefix);
   fprintf(stderr, "%sstate: %d\terror: %d\n", prefix, vm->state, vm->error);
   fprintf(stderr, "%scode size: %u\tpc: %d\n", prefix, vm->bcode_size, vm->pc);
   fprintf(stderr, "%sstack max: %lld\tcur: %lld\n", prefix, buzzvm_stack_top(vm), buzzvm_stack_top(vm));
   for(int64_t i = buzzvm_stack_top(vm)-1; i >= 0; --i) {
      fprintf(stderr, "%s\t%lld\t%u\t%f\n", prefix, i, buzzdarray_get(vm->stack, i, buzzvm_var_t)->i.value, buzzdarray_get(vm->stack, i, buzzvm_var_t)->f.value);
   }
   fprintf(stderr, "%s============================================================\n\n", prefix);
}

/****************************************/
/****************************************/

CBuzzController::CBuzzController() :
   m_pcRABA(NULL),
   m_pcRABS(NULL) {
}

/****************************************/
/****************************************/

CBuzzController::~CBuzzController() {
}

/****************************************/
/****************************************/

void CBuzzController::Init(TConfigurationNode& t_node) {
   /* Get pointers to devices */
   m_pcRABA = GetActuator<CCI_RangeAndBearingActuator>("range_and_bearing");
   m_pcRABS = GetSensor  <CCI_RangeAndBearingSensor  >("range_and_bearing");
   /* Create a new BuzzVM */
   m_tBuzzVM = buzzvm_new();
   /* Get the bytecode filename */
   std::string strFName;
   GetNodeAttribute(t_node, "script", strFName);
   /* Load the bytecode */
   std::ifstream cBCodeFile(strFName.c_str(), std::ios::binary | std::ios::ate);
   std::ifstream::pos_type unFileSize = cBCodeFile.tellg();
   CByteArray cBCode(unFileSize);
   cBCodeFile.seekg(0, std::ios::beg);
   cBCodeFile.read(reinterpret_cast<char*>(cBCode.ToCArray()), unFileSize);
   /* Set the script */
   SetBytecode(cBCode);
}

/****************************************/
/****************************************/

void CBuzzController::Reset() {
   /* Reset the VM */
   buzzvm_reset(m_tBuzzVM);
}

/****************************************/
/****************************************/

void CBuzzController::ControlStep() {
   /* Go through RAB messages and add them to the FIFO */
   // m_pcRABS
   /* Process messages */
   // TODO
   /* Step the VM */
   buzzvm_step(m_tBuzzVM);
   dump(m_tBuzzVM, "[DEBUG] ");
   /* Apply actuation */
   // TODO
   /* Send messages from FIFO */
   // m_pcRABA
}

/****************************************/
/****************************************/

void CBuzzController::Destroy() {
   /* Get rid of the VM */
   buzzvm_destroy(&m_tBuzzVM);
}

/****************************************/
/****************************************/

void CBuzzController::SetBytecode(const CByteArray& c_bcode) {
   /* Copy the bytecode */
   m_cBytecode = c_bcode;
   /* Load the script */
   buzzvm_set_bcode(m_tBuzzVM, m_cBytecode.ToCArray(), m_cBytecode.Size());
}

/****************************************/
/****************************************/

REGISTER_CONTROLLER(CBuzzController, "buzz_controller");
