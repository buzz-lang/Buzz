#include "buzz_controller.h"
#include "buzzasm.h"
#include <cstdlib>

/****************************************/
/****************************************/

void dump(buzzvm_t vm, const char* prefix) {
   fprintf(stderr, "%s============================================================\n", prefix);
   fprintf(stderr, "%sstate: %d\terror: %d\n", prefix, vm->state, vm->error);
   fprintf(stderr, "%scode size: %u\tpc: %lld\n", prefix, vm->bcode_size, vm->pc);
   fprintf(stderr, "%sstack max: %u\tcur: %lld\n", prefix, vm->stack_size, vm->stack_top);
   for(int64_t i = vm->stack_top-1; i >= 0; --i) {
      fprintf(stderr, "%s\t%lld\t%u\t%f\n", prefix, i, vm->stack[i].i, vm->stack[i].f);
   }
   fprintf(stderr, "%s============================================================\n\n", prefix);
}

/****************************************/
/****************************************/

CBuzzController::CBuzzController() :
   m_punBCode(NULL) {
}

/****************************************/
/****************************************/

CBuzzController::~CBuzzController() {
}

/****************************************/
/****************************************/

void CBuzzController::Init(TConfigurationNode& t_node) {
   /* Get pointers to devices */
   // TODO
   /* Create a new BuzzVM */
   m_tBuzzVM = buzzvm_new(100, 5, 100);
   /* Get the script filename */
   std::string strFName;
   GetNodeAttribute(t_node, "script", strFName);
   /* Set the script */
   SetScript(strFName);
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
   /* Step the VM */
   buzzvm_step(m_tBuzzVM);
   dump(m_tBuzzVM, "[DEBUG] ");
}

/****************************************/
/****************************************/

void CBuzzController::Destroy() {
   /* Get rid of the VM */
   buzzvm_destroy(&m_tBuzzVM);
   /* Get rid of the bytecode */
   free(m_punBCode);
}

/****************************************/
/****************************************/

void CBuzzController::SetScript(const std::string& str_fname) {
   /* Compile the script */
   buzz_asm(str_fname.c_str(), &m_punBCode, &m_unBCodeSize);
   /* Load the script */
   buzzvm_set_bcode(m_tBuzzVM, m_punBCode, m_unBCodeSize);
}

/****************************************/
/****************************************/

REGISTER_CONTROLLER(CBuzzController, "buzz_controller");
