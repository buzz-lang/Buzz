#include "buzzvm.h"
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

buzzvm_t buzzvm_new(uint32_t stack_size,
                    uint32_t flist_size) {
   /* Create VM state. calloc() takes care of zeroing everything */
   buzzvm_t vms = (buzzvm_t)calloc(1, sizeof(struct buzzvm_s));
   /* Create stack. calloc() takes care of zeroing everything */
   vms->stack_size = stack_size;
   vms->stack = calloc(stack_size, sizeof(buzzvm_stack_elem));
   /* Create function list. calloc() takes care of zeroing everything */
   vms->flist_size = flist_size;
   vms->flist = calloc(flist_size, sizeof(buzzvm_funp));
   /* Return new vm */
   return vms;
}

/****************************************/
/****************************************/

void buzzvm_reset(buzzvm_t vm) {
   memset(vm->stack, 0, vm->stack_size);
   vm->stack_top = 0;
   vm->pc = 0;
   if(vm->bcode) vm->state = BUZZVM_STATE_READY;
   else vm->state = BUZZVM_STATE_NOCODE;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   free((*vm)->stack);
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

#define assert_stack(IDX) if((IDX) < 0 || (IDX) >= vm->stack_size) { vm->state = BUZZVM_STATE_ERROR; vm->error = BUZZVM_ERROR_STACK; return vm->state; }

#define inc_pc() ++vm->pc; assert_pc(vm->pc);

#define inc_stack() ++vm->stack_top; assert_stack(vm->stack_top);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(&(vm->bcode[vm->pc+1]))); vm->pc += sizeof(TYPE) + 1;

#define binary_op_f(OP) assert_stack(vm->stack_top-2); --vm->stack_top; vm->stack[vm->stack_top-1].f = vm->stack[vm->stack_top].f OP vm->stack[vm->stack_top-1].f; inc_pc();

#define binary_op_i(OP) assert_stack(vm->stack_top-2); --vm->stack_top; vm->stack[vm->stack_top-1].i = vm->stack[vm->stack_top].i OP vm->stack[vm->stack_top-1].i; inc_pc();

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
         vm->state = BUZZVM_STATE_DONE;
         break;
      }
      case BUZZVM_INSTR_PUSH: {
         get_arg(float);
         vm->stack[vm->stack_top].f = arg;
         inc_stack();
         break;
      }
      case BUZZVM_INSTR_AT: {
         get_arg(uint32_t);
         assert_stack(arg);
         vm->stack[vm->stack_top].f = vm->stack[arg].f;
         inc_stack();
         break;
      }
      case BUZZVM_INSTR_POP: {
         --vm->stack_top;
         assert_stack(vm->stack_top);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_ADD: {
         binary_op_f(+);
         break;
      }
      case BUZZVM_INSTR_SUB: {
         binary_op_f(-);
         break;
      }
      case BUZZVM_INSTR_MUL: {
         binary_op_f(*);
         break;
      }
      case BUZZVM_INSTR_DIV: {
         binary_op_f(/);
         break;
      }
      case BUZZVM_INSTR_AND: {
         binary_op_i(&);
         break;
      }
      case BUZZVM_INSTR_OR: {
         binary_op_i(|);
         break;
      }
      case BUZZVM_INSTR_NOT: {
         vm->stack[vm->stack_top-1].i = !vm->stack[vm->stack_top-1].i;
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_EQ: {
         binary_op_f(==);
         break;
      }
      case BUZZVM_INSTR_GT: {
         binary_op_f(>);
         break;
      }
      case BUZZVM_INSTR_GTE: {
         binary_op_f(>=);
         break;
      }
      case BUZZVM_INSTR_LT: {
         binary_op_f(>);
         break;
      }
      case BUZZVM_INSTR_LTE: {
         binary_op_f(>=);
         break;
      }
      case BUZZVM_INSTR_JUMP: {
         get_arg(uint32_t);
         vm->pc = arg;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_JUMPZ: {
         assert_stack(vm->stack_top - 1);
         get_arg(uint32_t);
         if(vm->stack[vm->stack_top - 1].i == 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPNZ: {
         assert_stack(vm->stack_top - 1);
         get_arg(uint32_t);
         if(vm->stack[vm->stack_top - 1].i != 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPSUB: {
         get_arg(uint32_t);
         vm->stack[vm->stack_top].i = vm->pc;
         inc_stack();
         vm->pc = arg;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_RET: {
         --vm->stack_top;
         assert_stack(vm->stack_top);
         vm->pc = vm->stack[vm->stack_top].i;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_CALL: {
         get_arg(uint32_t);
         if(arg >= vm->flist_entries) {
            vm->state = BUZZVM_STATE_ERROR;
            vm->error = BUZZVM_ERROR_FLIST;
            return vm->state;
         }
         vm->flist[arg](vm);
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
