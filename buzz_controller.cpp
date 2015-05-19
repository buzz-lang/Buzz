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
      fprintf(stderr, "%s\t%lld\t%u\t%f\n", prefix, i, buzzdarray_get(vm->stack, i, buzzobj_t)->i.value, buzzdarray_get(vm->stack, i, buzzobj_t)->f.value);
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
   m_tBuzzVM = buzzvm_new(1); // TODO
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
   // TODO
}

/****************************************/
/****************************************/

void CBuzzController::ControlStep() {
   /* Go through RAB messages and add them to the FIFO */
   const CCI_RangeAndBearingSensor::TReadings& tPackets = m_pcRABS->GetReadings();
   for(size_t i = 0; i < tPackets.size(); ++i) {
      /* Copy packet into temporary buffer */
      CByteArray cData = tPackets[i].Data;
      /* Go through the messages until there's nothing else to read */
      UInt16 unMsgSize;
      do {
         /* Get payload size */
         unMsgSize = cData.PopFront<UInt16>();
         /* Append message to the Buzz input message queue */
         if(unMsgSize > 0 && cData.Size() >= unMsgSize) {
            buzzmsg_queue_append(m_tBuzzVM->inmsgs,
                                 buzzmsg_frombuffer(cData.ToCArray(), unMsgSize));
            /* Get rid of the data read */
            for(size_t i = 0; i < unMsgSize; ++i) cData.PopFront<UInt8>();
         }
      }
      while(cData.Size() > sizeof(UInt16) && unMsgSize > 0);
   }
   /* Step the VM */
   buzzvm_step(m_tBuzzVM);
   dump(m_tBuzzVM, "[DEBUG] ");
   /* Apply actuation */
   // TODO
   /* Send messages from FIFO */
   CByteArray cData;
   do {
      /* Are there more messages? */
      if(buzzmsg_queue_isempty(m_tBuzzVM->outmsgs)) break;
      /* Make sure the next message fits the data buffer */
      if(buzzmsg_size(buzzmsg_queue_get(m_tBuzzVM->outmsgs, 0)) +
         sizeof(UInt16) > m_pcRABA->GetSize()) break;
      /* Extract message */
      buzzmsg_t m = buzzmsg_queue_extract(m_tBuzzVM->outmsgs);
      /* Add message length to data buffer */
      cData << static_cast<UInt16>(buzzmsg_size(m));
      /* Add payload to data buffer */
      cData.AddBuffer(reinterpret_cast<UInt8*>(m->data), buzzmsg_size(m));
      /* Get rid of message */
      buzzmsg_destroy(&m);
   } while(1);
   /* Pad the rest of the data with zeroes */
   while(cData.Size() < m_pcRABA->GetSize()) cData << static_cast<UInt8>(0);
   /* Send message */
   m_pcRABA->SetData(cData);
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
