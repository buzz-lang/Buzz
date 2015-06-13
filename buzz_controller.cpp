#include "buzz_controller.h"
#include "buzzasm.h"
#include <cstdlib>
#include <fstream>
#include <argos3/core/utility/logging/argos_log.h>

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
         case BUZZTYPE_ARRAY:
            LOG << "[array with " << (buzzdict_size(o->a.value)) << " elems]";
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
      m_pcRABA   = GetActuator<CCI_RangeAndBearingActuator     >("range_and_bearing");
      m_pcRABS   = GetSensor  <CCI_RangeAndBearingSensor       >("range_and_bearing");
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
   if(buzzvm_function_call(m_tBuzzVM, "step", 0) != BUZZVM_STATE_READY) {
      fprintf(stderr, "%s: execution terminated abnormally: %s\n\n",
              m_strBytecodeFName.c_str(),
              buzzvm_error_desc[m_tBuzzVM->error]);
   }
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
   /* Execute the global part of the script */
   buzzvm_execute_script(m_tBuzzVM);
   /* Call the Init() function */
   buzzvm_function_call(m_tBuzzVM, "init", 0);
}

/****************************************/
/****************************************/

int CBuzzController::RegisterFunctions() {
   /* Pointer to this controller */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "controller"));
   buzzvm_pushuserdata(m_tBuzzVM, this);
   buzzvm_gstore(m_tBuzzVM);
   /* BuzzLOG */
   buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "log"));
   buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzLOG));
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
   /* Send robot id */
   CByteArray cData;
   cData << m_tBuzzVM->robot;
   /* Send messages from FIFO */
   do {
      /* Are there more messages? */
      if(buzzoutmsg_queue_isempty(m_tBuzzVM->outmsgs)) break;
      /* Get first message */
      buzzmsg_payload_t m = buzzoutmsg_queue_first(m_tBuzzVM->outmsgs);
      /* Make sure the next message fits the data buffer */
      if(cData.Size() + buzzmsg_payload_size(m) + sizeof(UInt16)
         >
         m_pcRABA->GetSize()) {
         break;
      }
      /* Add message length to data buffer */
      cData << static_cast<UInt16>(buzzmsg_payload_size(m));
      /* Add payload to data buffer */
      cData.AddBuffer(reinterpret_cast<UInt8*>(m->data), buzzmsg_payload_size(m));
      /* Get rid of message */
      buzzoutmsg_queue_next(m_tBuzzVM->outmsgs);
      buzzmsg_payload_destroy(&m);
   } while(1);
   /* Pad the rest of the data with zeroes */
   while(cData.Size() < m_pcRABA->GetSize()) cData << static_cast<UInt8>(0);
   /* Send message */
   m_pcRABA->SetData(cData);
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
