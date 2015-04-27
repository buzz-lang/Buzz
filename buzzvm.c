#include "buzzvm.h"
#include "buzzvstig.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/****************************************/
/****************************************/

void buzzvm_msg_destroy(uint32_t pos, void* data, void* param) {
   buzzmsg_t* m = (buzzmsg_t*)data;
   buzzmsg_destroy(m);
}

void buzzvm_process_inmsgs(buzzvm_t vm) {
   /* Go through the messages */
   while(!buzzmsg_queue_isempty(vm->inmsgs)) {
      /* Extract the message data */
      buzzmsg_t msg = buzzmsg_queue_extract(vm->inmsgs);
      /* Dispatch the message wrt its type in msg->payload[0] */
      switch(buzzmsg_get(msg, 0)) {
         case BUZZMSG_USER: {
            fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
            break;
         }
         case BUZZMSG_VSTIG_PUT: {
            fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
            break;
         }
         case BUZZMSG_VSTIG_QUERY: {
            fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
            break;
         }
      }
      /* Get rid of the message */
      buzzmsg_destroy(&msg);
   }
}

/****************************************/
/****************************************/

void buzzvm_vstig_destroy(const void* key, void* data, void* params) {
   free((void*)key);
   buzzvstig_t* x = (buzzvstig_t*)data;
   buzzvstig_destroy(x);
   free(x);
}

/****************************************/
/****************************************/

buzzvm_t buzzvm_new() {
   /* Create VM state. calloc() takes care of zeroing everything */
   buzzvm_t vm = (buzzvm_t)calloc(1, sizeof(struct buzzvm_s));
   /* Create stack. */
   vm->stack = buzzdarray_new(20, sizeof(buzzvar_t), NULL);
   /* Create function list. calloc() takes care of zeroing everything */
   vm->flist = buzzdarray_new(20, sizeof(buzzvm_funp), NULL);
   /* Create message queues */
   vm->inmsgs = buzzmsg_queue_new(20);
   vm->outmsgs = buzzmsg_queue_new(20);
   /* Create virtual stigmergy. */
   vm->vstigs = buzzdict_new(1,
                             sizeof(int32_t),
                             sizeof(buzzvstig_t),
                             buzzdict_intkeyhash,
                             buzzdict_intkeycmp,
                             buzzvm_vstig_destroy);
   /* Return new vm */
   return vm;
}

/****************************************/
/****************************************/

void buzzvm_reset(buzzvm_t vm) {
   /* Clear stigmergy structures */
   buzzdict_destroy(&(vm->vstigs));
   vm->vstigs = buzzdict_new(1,
                             sizeof(int32_t),
                             sizeof(buzzvstig_t),
                             buzzdict_intkeyhash,
                             buzzdict_intkeycmp,
                             NULL);
   /* Reset message queues */
   buzzdarray_foreach(vm->inmsgs, buzzvm_msg_destroy, NULL);
   buzzdarray_clear(vm->inmsgs, 20);
   buzzdarray_foreach(vm->outmsgs, buzzvm_msg_destroy, NULL);
   buzzdarray_clear(vm->outmsgs, 20);
   /* Clear stack */
   buzzdarray_clear(vm->stack, 20);
   /* Reset program counter */
   vm->pc = 0;
   /* Reset VM state */
   if(vm->bcode) vm->state = BUZZVM_STATE_READY;
   else vm->state = BUZZVM_STATE_NOCODE;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   /* Get rid of the stack */
   buzzdarray_destroy(&(*vm)->stack);
   /* Get rid of the function list */
   buzzdarray_destroy(&(*vm)->flist);
   /* Get rid of the message queues */
   buzzdarray_foreach((*vm)->inmsgs, buzzvm_msg_destroy, NULL);
   buzzdarray_destroy(&(*vm)->inmsgs);
   buzzdarray_foreach((*vm)->outmsgs, buzzvm_msg_destroy, NULL);
   buzzdarray_destroy(&(*vm)->outmsgs);
   /* Get rid of the virtual stigmergy structures */
   buzzdict_destroy(&(*vm)->vstigs);
   free(*vm);
   *vm = 0;
}

/****************************************/
/****************************************/

void buzzvm_set_bcode(buzzvm_t vm,
                      const uint8_t* bcode,
                      uint32_t bcode_size) {
   vm->bcode_size = bcode_size;
   vm->bcode = bcode;
   vm->pc = 0;
   vm->state = BUZZVM_STATE_READY;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { vm->state = BUZZVM_STATE_ERROR; vm->error = BUZZVM_ERROR_PC; return vm->state; }

#define inc_pc() ++vm->pc; assert_pc(vm->pc);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(vm->bcode + vm->pc)); vm->pc += sizeof(TYPE);

buzzvm_state buzzvm_step(buzzvm_t vm) {
   /* Can't execute if not ready */
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Process messages */
   buzzvm_process_inmsgs(vm);
   /* Fetch instruction and (potential) argument */
   uint8_t instr = vm->bcode[vm->pc];
   /* Execute instruction */
   switch(instr) {
      case BUZZVM_INSTR_NOP: {
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_DONE: {
         buzzvm_done(vm);
         break;
      }
      case BUZZVM_INSTR_POP: {
         buzzvm_pop(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_RET: {
         buzzvm_pop(vm);
         vm->pc = buzzvm_stack_at(vm, 0).i.value;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_ADD: {
         buzzvm_add(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_SUB: {
         buzzvm_sub(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_MUL: {
         buzzvm_mul(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_DIV: {
         buzzvm_div(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_AND: {
         buzzvm_and(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_OR: {
         buzzvm_or(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_NOT: {
         buzzvm_not(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_EQ: {
         buzzvm_eq(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_GT: {
         buzzvm_gt(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_GTE: {
         buzzvm_gte(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_LT: {
         buzzvm_lt(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_LTE: {
         buzzvm_lte(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_SHOUT: {
         /* Get variable from stack(#1) */
         buzzvm_stack_assert(vm, 1);
         buzzvar_t var = buzzvm_stack_at(vm, 1);
         /* Serialize the message */
         buzzdarray_t buf = buzzdarray_new(16, sizeof(uint8_t), NULL);
         buzzmsg_serialize_u8(buf, BUZZMSG_USER);
         buzzvar_serialize(buf, var);
         /* Append it to the out message queue */
         buzzmsg_queue_append(vm->outmsgs, buf);
         /* Next instruction */
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_VSCREATE: {
         /* Get integer id from stack(#1) */
         buzzvm_stack_assert(vm, 1);
         int32_t id = buzzvm_stack_at(vm, 1).i.value;
         /* Create virtual stigmergy if not present already */
         if(!buzzdict_get(vm->vstigs, &id, buzzdict_t)) {
            buzzvstig_t vs = buzzvstig_new();
            buzzdict_set(vm->vstigs, &id, &vs);
         }
         /* Pop operands */
         buzzvm_pop(vm);
         /* Next instruction */
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_VSPUT: {
         buzzvm_stack_assert(vm, 3);
         /* Get integer id from stack(#3) */
         int32_t id = buzzvm_stack_at(vm, 3).i.value;
         /* Get integer key from stack(#2) */
         int32_t k = buzzvm_stack_at(vm, 2).i.value;
         /* Get value from stack(#1) */
         buzzvar_t v = buzzvm_stack_at(vm, 1);
         /* Look for virtual stigmergy */
         buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
         if(vs) {
            /* Found, update it */
            buzzvstig_elem_t x = buzzvstig_fetch(*vs, k);
            if(x) {
               x->data = v;
               ++(x->timestamp);
               // TODO set robot id
               buzzvstig_store(*vs, k, x);
            }
            else {
               struct buzzvstig_elem_s y;
               y.data = v;
               y.timestamp = 1;
               y.robot = 0; // TODO set robot id
               buzzvstig_store(*vs, k, &y);
            }
         }
         else {
            /* Ignore commands for virtual stigmergy that is not there */
            fprintf(stderr, "[WARNING] No virtual stigmergy with id %d\n", id);
         }
         /* Pop operands */
         buzzvm_pop(vm);
         buzzvm_pop(vm);
         buzzvm_pop(vm);
         /* Next instruction */
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_VSGET: {
         buzzvm_stack_assert(vm, 2);
         /* Get integer id from stack(#2) */
         int32_t id = buzzvm_stack_at(vm, 2).i.value;
         /* Get integer key from stack(#1) */
         int32_t k = buzzvm_stack_at(vm, 1).i.value;
         /* Pop operands */
         buzzvm_pop(vm);
         buzzvm_pop(vm);
         /* Look for virtual stigmergy */
         buzzdict_t* vs = buzzdict_get(vm->vstigs, &id, buzzdict_t);
         if(vs) {
            /* Virtual stigmergy found, look for key and push result */
            buzzvstig_elem_t e = buzzvstig_fetch(*vs, k);
            if(e) buzzvm_push(vm, &(e->data));
            else buzzvm_pushi(vm, 0);
         }
         else {
            /* No virtual stigmergy found, push false */
            buzzvm_pushi(vm, 0);
         }
         /* Next instruction */
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_PUSHI: {
         inc_pc();
         get_arg(int32_t);
         buzzvm_pushi(vm, arg);
         break;
      }
      case BUZZVM_INSTR_PUSHF: {
         inc_pc();
         get_arg(float);
         buzzvm_pushf(vm, arg);
         break;
      }
      case BUZZVM_INSTR_DUP: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_dup(vm, arg);
         break;
      }
      case BUZZVM_INSTR_JUMP: {
         inc_pc();
         get_arg(uint32_t);
         vm->pc = arg;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_JUMPZ: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_stack_assert(vm, 1);
         if(buzzvm_stack_at(vm, 1).i.value == 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPNZ: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_stack_assert(vm, 1);
         if(buzzvm_stack_at(vm, 1).i.value != 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPSUB: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_pushi(vm, arg);
         vm->pc = arg;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_CALL: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_call(vm, arg);
         break;
      }
      default:
         vm->state = BUZZVM_STATE_ERROR;
         vm->error = BUZZVM_ERROR_INSTR;
         break;
   }
   return vm->state;
}

/****************************************/
/****************************************/

int64_t buzzvm_register_function(buzzvm_t vm,
                                  buzzvm_funp funp) {
   buzzdarray_push(vm->flist, funp);
   return buzzdarray_size(vm->flist) - 1;
}

/****************************************/
/****************************************/
