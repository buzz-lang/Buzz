#include "buzz_controller.h"
#include "buzzasm.h"
#include <cstdlib>
#include <fstream>
#include <argos3/core/utility/logging/argos_log.h>

/****************************************/
/****************************************/

void dump(buzzvm_t vm) {
   DEBUG("============================================================\n");
   DEBUG("state: %d\terror: %d\n", vm->state, vm->error);
   DEBUG("code size: %u\tpc: %d\n", vm->bcode_size, vm->pc);
   DEBUG("stacks: %lld\tcur elem: %lld (size %lld)\n", buzzdarray_size(vm->stacks), buzzvm_stack_top(vm), buzzvm_stack_top(vm));
   for(int64_t i = buzzvm_stack_top(vm)-1; i >= 0; --i) {
      DEBUG("\t%lld\t", i);
      buzzobj_t o = buzzdarray_get(vm->stack, i, buzzobj_t);
      switch(o->o.type) {
         case BUZZTYPE_NIL:
            fprintf(stderr, "[nil]\n");
            break;
         case BUZZTYPE_INT:
            fprintf(stderr, "[int] %d\n", o->i.value);
            break;
         case BUZZTYPE_FLOAT:
            fprintf(stderr, "[float] %f\n", o->f.value);
            break;
         case BUZZTYPE_TABLE:
            fprintf(stderr, "[table] %d elements\n", buzzdict_size(o->t.value));
            break;
         case BUZZTYPE_ARRAY:
            fprintf(stderr, "[array] %lld\n", buzzdarray_size(o->a.value));
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative) {
               fprintf(stderr, "[n-closure] %d\n", o->c.value.ref);
            }
            else {
               fprintf(stderr, "[c-closure] %d\n", o->c.value.ref);
            }
            break;
         case BUZZTYPE_STRING:
            fprintf(stderr, "[string] %d:'%s'\n", o->s.value.sid, o->s.value.str);
            break;
         default:
            fprintf(stderr, "[TODO] type = %d\n", o->o.type);
      }
   }
   DEBUG("============================================================\n\n");
}

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
            LOG << "[table " << (buzzdict_size(o->t.value)) << " elems]";
            break;
         case BUZZTYPE_ARRAY:
            LOG << "[array " << (buzzdict_size(o->a.value)) << " elems]";
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative)
               LOG << "[n-closure " << o->c.value.ref << "]";
            else
               LOG << "[c-closure " << o->c.value.ref << "]";
            break;
         case BUZZTYPE_STRING:
            LOG << o->s.value.str;
            break;
         case BUZZTYPE_USERDATA:
            LOG << "[userdata " << o->u.value << "]";
            break;
         default:
            break;
      }
   }
   LOG << std::endl;
   LOG.Flush();
   buzzvm_ret0(vm);
   return BUZZVM_STATE_READY;
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
   try {
      /* Get pointers to devices */
      m_pcRABA = GetActuator<CCI_RangeAndBearingActuator>("range_and_bearing");
      m_pcRABS = GetSensor  <CCI_RangeAndBearingSensor  >("range_and_bearing");
      /* Get the script name */
      std::string strFName;
      GetNodeAttribute(t_node, "bytecode_file", strFName);
      /* Initialize the rest */
      m_tBuzzVM = NULL;
      m_unRobotId = FromString<UInt32>(GetId().substr(2));
      SetBytecode(strFName);
   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED("Error initializing the Buzz controller", ex);
   }
}

/****************************************/
/****************************************/

void CBuzzController::Reset() {
   SetBytecode(m_strBytecodeFName);
}

/****************************************/
/****************************************/

void CBuzzController::ControlStep() {
   ProcessInMsgs();
   UpdateSensors();
   buzzswarm_members_print(m_tBuzzVM->swarmmembers, m_tBuzzVM->robot);
   buzzvm_function_call(m_tBuzzVM, "step", 0);
   UpdateActuators();
   ProcessOutMsgs();
}

/****************************************/
/****************************************/

void CBuzzController::Destroy() {
   /* Get rid of the VM */
   if(m_tBuzzVM) {
      buzzvm_function_call(m_tBuzzVM, "destroy", 0);
      buzzvm_destroy(&m_tBuzzVM);
   }
}

/****************************************/
/****************************************/

void CBuzzController::SetBytecode(const std::string& str_fname) {
   /* Reset the BuzzVM */
   if(m_tBuzzVM) buzzvm_destroy(&m_tBuzzVM);
   m_tBuzzVM = buzzvm_new(m_unRobotId);
   /* Save the bytecode filename */
   m_strBytecodeFName = str_fname;
   /* Load the bytecode */
   std::ifstream cBCodeFile(str_fname.c_str(), std::ios::binary | std::ios::ate);
   std::ifstream::pos_type unFileSize = cBCodeFile.tellg();
   m_cBytecode.Clear();
   m_cBytecode.Resize(unFileSize);
   cBCodeFile.seekg(0, std::ios::beg);
   cBCodeFile.read(reinterpret_cast<char*>(m_cBytecode.ToCArray()), unFileSize);
   /* Load the script */
   buzzvm_set_bcode(m_tBuzzVM, m_cBytecode.ToCArray(), m_cBytecode.Size());
   /* Register basic function */
   if(RegisterFunctions() != BUZZVM_STATE_READY) {
      THROW_ARGOSEXCEPTION("Error while registering functions");
   }
   /* Call the Init() function */
   buzzvm_function_call(m_tBuzzVM, "init", 0);
}

/****************************************/
/****************************************/

int CBuzzController::RegisterFunctions() {
   /* BuzzLOG */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "log"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzLOG));
   buzzvm_gstore(m_tBuzzVM);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

void CBuzzController::ProcessInMsgs() {
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
            buzzinmsg_queue_append(m_tBuzzVM->inmsgs,
                                   buzzmsg_payload_frombuffer(cData.ToCArray(), unMsgSize));
            /* Get rid of the data read */
            for(size_t i = 0; i < unMsgSize; ++i) cData.PopFront<UInt8>();
         }
      }
      while(cData.Size() > sizeof(UInt16) && unMsgSize > 0);
   }
}

/****************************************/
/****************************************/

void CBuzzController::ProcessOutMsgs() {
   LOGERR << "CNTRL: "
          << GetId()
          << ": At start msg queue has "
          << buzzoutmsg_queue_size(m_tBuzzVM->outmsgs)
          << " elements"
          << std::endl;
   /* Send messages from FIFO */
   CByteArray cData;
   do {
      /* Are there more messages? */
      if(buzzoutmsg_queue_isempty(m_tBuzzVM->outmsgs)) break;
      /* Get first message */
      buzzmsg_payload_t m = buzzoutmsg_queue_first(m_tBuzzVM->outmsgs);
      /* Make sure the next message fits the data buffer */
      if(cData.Size() + buzzmsg_payload_size(m) + sizeof(UInt16)
         >
         m_pcRABA->GetSize()) {
         // LOGERR << "CNTRL: "
         //        << GetId()
         //        << ": Not sending "
         //        << (buzzmsg_size(buzzmsg_queue_get(m_tBuzzVM->outmsgs, 0)) + sizeof(UInt16))
         //        << " bytes"
         //        << std::endl;
         break;
      }
      // LOGERR << "CNTRL: "
      //        << GetId()
      //        << ": Sending "
      //        << (buzzmsg_size(buzzmsg_queue_get(m_tBuzzVM->outmsgs, 0)) + sizeof(UInt16))
      //        << " bytes - "
      //        << cData.Size()
      //        << " bytes sent so far"
      //        << std::endl;
      /* Add message length to data buffer */
      cData << static_cast<UInt16>(buzzmsg_payload_size(m));
      /* Add payload to data buffer */
      cData.AddBuffer(reinterpret_cast<UInt8*>(m->data), buzzmsg_payload_size(m));
      /* Get rid of message */
      buzzoutmsg_queue_next(m_tBuzzVM->outmsgs);
      buzzmsg_payload_destroy(&m);
   } while(1);
   /* Pad the rest of the data with zeroes */
   // LOGERR << "CNTRL: Total msg size sent: "
   //        << cData.Size()
   //        << std::endl;
   while(cData.Size() < m_pcRABA->GetSize()) cData << static_cast<UInt8>(0);
   // LOGERR << "CNTRL: Total msg size sent: "
   //        << cData.Size()
   //        << std::endl;
   /* Send message */
   m_pcRABA->SetData(cData);
   // LOGERR << "CNTRL: "
   //        << std::endl;
   LOGERR << "CNTRL: "
          << GetId()
          << ": At end msg queue has "
          << buzzoutmsg_queue_size(m_tBuzzVM->outmsgs)
          << " elements"
          << std::endl;
}

/****************************************/
/****************************************/

void CBuzzController::UpdateSensors() {
}

/****************************************/
/****************************************/

void CBuzzController::UpdateActuators() {
}

/****************************************/
/****************************************/

REGISTER_CONTROLLER(CBuzzController, "buzz_controller");
