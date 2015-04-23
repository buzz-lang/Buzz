#include "buzzvm.h"
#include "buzzvstig.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/****************************************/
/****************************************/

void buzzvm_vstig_destroy(const void* key, void* data, void* params) {
   free((void*)key);
   buzzvstig_t* x = (buzzvstig_t*)data;
   buzzvstig_destroy(x);
   free(x);
}

/*
 * This is the djb2() hash function presented in
 * http://www.cse.yorku.ca/~oz/hash.html.
 */
uint32_t buzzvm_dict_strkeyhash(const void* key) {
   /* Treat the key as a string */
   const char* s = (const char*)key;
   /* Initialize the hash to something */
   uint32_t h = 5381;
   /* Go through the string */
   int c;
   while((c = *s++)) {
      /*
       * This is equivalent to
       *
       * h = h * 33 + c
       *   = (h * 32 + h) + c
       *
       * Why 33 is a good choice, nobody knows
       * NOTE: in the Java VM they use 31 instead
       */
      h = ((h << 5) + h) + c;
   }
   return h;
}

uint32_t buzzvm_dict_intkeyhash(const void* key) {
   return *(int32_t*)key;
}

int buzzvm_dict_strkeycmp(const void* a, const void* b) {
   return strcmp((const char*)a, (const char*)b);
}

int buzzvm_dict_intkeycmp(const void* a, const void* b) {
   if(*(int32_t*)a < *(int32_t*)b) return -1;
   if(*(int32_t*)a > *(int32_t*)b) return  1;
   return 0;
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
   /* Create virtual stigmergy. */
   vm->vstigs = buzzdict_new(1,
                             sizeof(int32_t),
                             sizeof(buzzvstig_t),
                             buzzvm_dict_intkeyhash,
                             buzzvm_dict_intkeycmp,
                             buzzvm_vstig_destroy);
   /* Return new vm */
   return vm;
}

/****************************************/
/****************************************/

void buzzvm_reset(buzzvm_t vm) {
   buzzdarray_clear(vm->stack, 20);
   buzzdict_destroy(&(vm->vstigs));
   vm->vstigs = buzzdict_new(1,
                             sizeof(int32_t),
                             sizeof(buzzvstig_t),
                             buzzvm_dict_intkeyhash,
                             buzzvm_dict_intkeycmp,
                             NULL);
   vm->pc = 0;
   if(vm->bcode) vm->state = BUZZVM_STATE_READY;
   else vm->state = BUZZVM_STATE_NOCODE;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   buzzdarray_destroy(&(*vm)->stack);
   buzzdarray_destroy(&(*vm)->flist);
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
