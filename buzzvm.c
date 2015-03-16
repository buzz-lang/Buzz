#include "buzzvm.h"
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

buzzvm_t buzzvm_new(uint32_t stack_size,
                    uint32_t flist_size,
                    uint32_t gidlist_size) {
   /* Create VM state. calloc() takes care of zeroing everything */
   buzzvm_t vms = (buzzvm_t)calloc(1, sizeof(struct buzzvm_s));
   /* Create stack. calloc() takes care of zeroing everything */
   vms->stack_size = stack_size;
   vms->stack = calloc(stack_size, sizeof(buzzvm_stack_elem));
   /* Create function list. calloc() takes care of zeroing everything */
   vms->flist_size = flist_size;
   vms->flist = calloc(flist_size, sizeof(buzzvm_funp));
   /* Create group id list. calloc() takes care of zeroing everything */
   vms->gidlist_size = gidlist_size;
   vms->gidlist = calloc(gidlist_size, sizeof(buzzvm_gid));
   /* Return new vm */
   return vms;
}

/****************************************/
/****************************************/

void buzzvm_reset(buzzvm_t vm) {
   memset(vm->stack, 0, sizeof(buzzvm_stack_elem) * vm->stack_size);
   memset(vm->gidlist, 0, sizeof(buzzvm_gid) * vm->stack_size);
   vm->stack_top = 0;
   vm->gidlist_entries = 0;
   vm->pc = 0;
   if(vm->bcode) vm->state = BUZZVM_STATE_READY;
   else vm->state = BUZZVM_STATE_NOCODE;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   free((*vm)->stack);
   free((*vm)->flist);
   free((*vm)->gidlist);
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

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(&(vm->bcode[vm->pc]))); vm->pc += sizeof(TYPE);

buzzvm_state buzzvm_step(buzzvm_t vm) {
   /* Can't execute if not ready */
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
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
         vm->pc = buzzvm_at(vm, 0).i;
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
      case BUZZVM_INSTR_QGID: {
         buzzvm_qgid(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_SGID: {
         buzzvm_sgid(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_CGID: {
         buzzvm_cgid(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_PUSH: {
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
         buzzvm_assert_stack(vm, 1);
         if(buzzvm_at(vm, 1).i == 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPNZ: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_assert_stack(vm, 1);
         if(buzzvm_at(vm, 1).i != 0) {
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
   }
   return vm->state;
}

/****************************************/
/****************************************/

int64_t buzzvm_register_function(buzzvm_t vm,
                                  buzzvm_funp funp) {
   if(vm->flist_entries >= vm->flist_size) {
      return -1;
   }
   vm->flist[vm->flist_entries] = funp;
   ++vm->flist_entries;
   return vm->flist_entries - 1;
}

/****************************************/
/****************************************/
