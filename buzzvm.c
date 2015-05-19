#include "buzzvm.h"
#include "buzzvstig.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/****************************************/
/****************************************/

#define BUZZVM_STACKS_INIT_CAPACITY  20
#define BUZZVM_STACK_INIT_CAPACITY   20
#define BUZZVM_LSYMTS_INIT_CAPACITY  20
#define BUZZVM_SYMS_INIT_CAPACITY    20
#define BUZZVM_STRINGS_INIT_CAPACITY 20

/****************************************/
/****************************************/

void buzzvm_string_add(buzzvm_t vm,
                       const char* str) {
   char* s = strdup(str);
   buzzdarray_push(vm->strings, &s);
}

void buzzvm_string_destroy(uint32_t pos,
                           void* data,
                           void* params) {
   free(*(char**)data);
}

int buzzvm_string_cmp(const void* a, const void* b) {
   return strcmp(*(char**)a, *(char**)b);
}

#define buzzdarray_string_find(vm, str) buzzdarray_find(vm->strings, buzzvm_string_cmp, str);

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
            fprintf(stderr, "[TODO] [ROBOT %u] %s:%d\n", vm->robot, __FILE__, __LINE__);
            break;
         }
         case BUZZMSG_VSTIG_PUT: {
            /* Deserialize the vstig id */
            uint16_t id;
            int64_t pos = buzzmsg_deserialize_u16(&id, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_PUT message received\n", vm->robot);
               break;
            }
            /* Look for virtual stigmergy */
            buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
            if(vs) {
               /* Virtual stigmergy found */
               /* Deserialize key and value from msg */
               buzzobj_t* k;        // key
               buzzvstig_elem_t* v; // value
               if(buzzvstig_elem_deserialize(k, v, msg, pos, vm) > 0) {
                  /* Deserialization successful */
                  /* Fetch local vstig element */
                  buzzvstig_elem_t* l = buzzvstig_fetch(*vs, k);
                  if((!l) ||                     /* Element not found */
                     (*l)->timestamp <= (*v)->timestamp /* Local element might be equal age or older */
                     ) {
                     if((*l)->timestamp < (*v)->timestamp) {
                        /* Local element is older */
                        /* Store element */
                        buzzvstig_store(*vs, k, v);
                        /* Broadcast PUT */
                        buzzdarray_t buf = buzzmsg_new(16);
                        buzzmsg_serialize_u8(buf, BUZZMSG_VSTIG_PUT);
                        buzzmsg_serialize_u16(buf, id);
                        buzzvstig_elem_serialize(buf, *k, *v);
                        /* Append the message to the out message queue */
                        buzzmsg_queue_append(vm->outmsgs, buf);
                     }
                     else if((*l)->robot != (*v)->robot) {
                        /* Local element conflicts with received one */
                        // TODO
                     }
                  }
               }
               else {
                  fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_PUT message received\n", vm->robot);
               }
            }
            break;
         }
         case BUZZMSG_VSTIG_QUERY: {
            /* Deserialize the vstig id */
            uint16_t id;
            int64_t pos = buzzmsg_deserialize_u16(&id, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_PUT message received\n", vm->robot);
               break;
            }
            /* Look for virtual stigmergy */
            buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
            if(vs) {
               /* Virtual stigmergy found */
               /* Deserialize key and value from msg */
               buzzobj_t* k;        // key
               buzzvstig_elem_t* v; // value
               if(buzzvstig_elem_deserialize(k, v, msg, pos, vm) > 0) {
                  /* Deserialization successful */
                  /* Fetch local vstig element */
                  buzzvstig_elem_t* l = buzzvstig_fetch(*vs, k);
                  if((!l) ||                           /* Element not found */
                     (*l)->timestamp < (*v)->timestamp /* Local element is older */
                     ) {
                     /* Store element */
                     buzzvstig_store(*vs, k, v);
                     /* Broadcast PUT */
                     buzzdarray_t buf = buzzmsg_new(16);
                     buzzmsg_serialize_u8(buf, BUZZMSG_VSTIG_PUT);
                     buzzmsg_serialize_u16(buf, id);
                     buzzvstig_elem_serialize(buf, *k, *v);
                     /* Append the message to the out message queue */
                     buzzmsg_queue_append(vm->outmsgs, buf);
                  }
                  else if((*l)->timestamp > (*v)->timestamp) {
                     /* Local element is newer */
                     /* Broadcast PUT */
                     buzzdarray_t buf = buzzmsg_new(16);
                     buzzmsg_serialize_u8(buf, BUZZMSG_VSTIG_PUT);
                     buzzmsg_serialize_u16(buf, id);
                     buzzvstig_elem_serialize(buf, *k, *l);
                     /* Append the message to the out message queue */
                     buzzmsg_queue_append(vm->outmsgs, buf);
                  }
               }
            }
            break;
         }
         case BUZZMSG_SWARMS: {
            /* Deserialize robot id */
            int16_t rid;
            int64_t pos = buzzmsg_deserialize_u16((uint16_t*)(&rid), msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARMS message received\n", vm->robot);
               break;
            }
            /* Deserialize number of swarm ids */
            uint16_t numsids;
            pos = buzzmsg_deserialize_u16(&numsids, msg, pos);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARMS message received\n", vm->robot);
               break;
            }
            /* Go through swarm ids and deserialize them */
            uint8_t sid;
            for(uint16_t i = 0; i < numsids; ++i) {
               pos = buzzmsg_deserialize_u8(&sid, msg, pos);
               if(pos < 0) {
                  fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARMS message received\n", vm->robot);
                  break;
               }
               /* Add the swarm id to the list of swarm ids of the robot with id rid */
               // TODO
            }
            break;
         }
            /* Get rid of the message */
            buzzmsg_destroy(&msg);
      }
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

void buzzvm_darray_destroy(uint32_t pos,
                           void* data,
                           void* params) {
   buzzdarray_t* s = (buzzdarray_t*)data;
   buzzdarray_destroy(s);
}

buzzvm_t buzzvm_new(uint32_t robot) {
   /* Create VM state. calloc() takes care of zeroing everything */
   buzzvm_t vm = (buzzvm_t)calloc(1, sizeof(struct buzzvm_s));
   /* Create stacks */
   vm->stacks = buzzdarray_new(BUZZVM_STACKS_INIT_CAPACITY,
                               sizeof(buzzdarray_t),
                               buzzvm_darray_destroy);
   vm->stack = buzzdarray_new(BUZZVM_STACK_INIT_CAPACITY,
                              sizeof(buzzobj_t),
                              NULL);
   buzzdarray_push(vm->stacks, &(vm->stack));
   /* Create local variable tables */
   vm->lsymts = buzzdarray_new(BUZZVM_LSYMTS_INIT_CAPACITY,
                               sizeof(buzzdarray_t),
                               buzzvm_darray_destroy);
   vm->lsyms = buzzdarray_new(BUZZVM_SYMS_INIT_CAPACITY,
                              sizeof(buzzobj_t),
                              NULL);
   buzzdarray_push(vm->lsymts, &(vm->lsyms));
   /* Create global variable tables */
   vm->gsyms = buzzdict_new(BUZZVM_SYMS_INIT_CAPACITY,
                            sizeof(int32_t),
                            sizeof(buzzobj_t),
                            buzzdict_int32keyhash,
                            buzzdict_int32keycmp,
                            NULL);
   /* Create string list */
   vm->strings = buzzdarray_new(BUZZVM_STRINGS_INIT_CAPACITY,
                                sizeof(char*),
                                buzzvm_string_destroy);
   /* Create heap */
   vm->heap = buzzheap_new();
   /* Create function list */
   vm->flist = buzzdarray_new(20, sizeof(buzzvm_funp), NULL);
   /* Create swarm list */
   vm->swarms = buzzdict_new(10,
                             sizeof(int16_t),
                             sizeof(uint8_t),
                             buzzdict_int16keyhash,
                             buzzdict_int16keycmp,
                             NULL);
   /* Create message queues */
   vm->inmsgs = buzzmsg_queue_new(20);
   vm->outmsgs = buzzmsg_queue_new(20);
   /* Create virtual stigmergy. */
   vm->vstigs = buzzdict_new(1,
                             sizeof(uint16_t),
                             sizeof(buzzvstig_t),
                             buzzdict_uint16keyhash,
                             buzzdict_uint16keycmp,
                             buzzvm_vstig_destroy);
   /* Take care of the robot id */
   vm->robot = robot;
   /* Return new vm */
   return vm;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   /* Get rid of the stack */
   buzzdarray_destroy(&(*vm)->strings);
   /* Get rid of the global variable table */
   buzzdict_destroy(&(*vm)->gsyms);
   /* Get rid of the local variable tables */
   buzzdarray_destroy(&(*vm)->lsymts);
   /* Get rid of the stack */
   buzzdarray_destroy(&(*vm)->stacks);
   /* Get rid of the heap */
   buzzheap_destroy(&(*vm)->heap);
   /* Get rid of the function list */
   buzzdarray_destroy(&(*vm)->flist);
   /* Get rid of the swarm list */
   buzzdict_destroy(&(*vm)->swarms);
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

int buzzvm_set_bcode(buzzvm_t vm,
                     const uint8_t* bcode,
                     uint32_t bcode_size) {
   /* Fetch the string count */
   long int count;
   memcpy(&count, bcode, sizeof(long int));
   /* Go through the strings and store them */
   uint32_t i = sizeof(long int);
   long int c = 0;
   for(; (c < count) && (i < bcode_size); ++c) {
      /* Store string */
      buzzvm_string_add(vm, (char*)(bcode + i));
      /* Advance to first character of next string */
      while(*(bcode + i) != 0) ++i;
      ++i;
   }
   /* Initialize VM state */
   vm->state = BUZZVM_STATE_READY;
   vm->error = BUZZVM_ERROR_NONE;
   /* Initialize bytecode data */
   vm->bcode_size = bcode_size;
   vm->bcode = bcode;
   /* Set program counter */
   vm->pc = i;
   /*
    * Register stigmergy methods
    */
   /* Add 'stigmergy' table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "stigmergy"));
   buzzvm_pusht(vm);
   buzzobj_t t = buzzvm_stack_at(vm, 1);
   buzzvm_gstore(vm);
   /* Add 'create' function */
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "create"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_create));
   buzzvm_tput(vm);
   /* Add the 'put' method */
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "put"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_put));
   buzzvm_tput(vm);
   /* Add the 'get' method */
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "get"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_get));
   buzzvm_tput(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { vm->state = BUZZVM_STATE_ERROR; vm->error = BUZZVM_ERROR_PC; return vm->state; }

#define inc_pc() ++vm->pc; assert_pc(vm->pc);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(vm->bcode + vm->pc)); vm->pc += sizeof(TYPE);

buzzvm_state buzzvm_step(buzzvm_t vm) {
   /* Can't execute if not ready */
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Execute GC */
   buzzheap_gc(vm);
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
      case BUZZVM_INSTR_PUSHNIL: {
         inc_pc();
         buzzvm_pushnil(vm);
         break;
      }
      case BUZZVM_INSTR_POP: {
         buzzvm_pop(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_RET0: {
         buzzvm_ret0(vm);
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_RET1: {
         buzzvm_ret1(vm);
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
      case BUZZVM_INSTR_MOD: {
         buzzvm_mod(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_POW: {
         buzzvm_pow(vm);
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
      case BUZZVM_INSTR_NEQ: {
         buzzvm_neq(vm);
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
         buzzobj_t var = buzzvm_stack_at(vm, 1);
         /* Serialize the message */
         buzzdarray_t buf = buzzmsg_new(16);
         buzzmsg_serialize_u8(buf, BUZZMSG_USER);
         buzzobj_serialize(buf, var);
         /* Append it to the out message queue */
         buzzmsg_queue_append(vm->outmsgs, buf);
         /* Next instruction */
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_GLOAD: {
         inc_pc();
         buzzvm_gload(vm);
         break;
      }
      case BUZZVM_INSTR_GSTORE: {
         inc_pc();
         buzzvm_gstore(vm);
         break;
      }
      case BUZZVM_INSTR_PUSHT: {
         buzzvm_pusht(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_TPUT: {
         buzzvm_tput(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_TGET: {
         buzzvm_tget(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_PUSHA: {
         buzzvm_pusha(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_APUT: {
         buzzvm_aput(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_AGET: {
         buzzvm_aget(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_CALLC: {
         inc_pc();
         buzzvm_callc(vm);
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_JOINSWARM: {
         inc_pc();
         buzzvm_joinswarm(vm);
         break;
      }
      case BUZZVM_INSTR_LEAVESWARM: {
         inc_pc();
         buzzvm_leaveswarm(vm);
         break;
      }
      case BUZZVM_INSTR_INSWARM: {
         inc_pc();
         buzzvm_inswarm(vm);
         break;
      }
      case BUZZVM_INSTR_PUSHI: {
         inc_pc();
         get_arg(int32_t);
         buzzvm_pushi(vm, arg);
         break;
      }
      case BUZZVM_INSTR_PUSHS: {
         inc_pc();
         get_arg(int32_t);
         buzzvm_pushs(vm, arg);
         break;
      }
      case BUZZVM_INSTR_PUSHF: {
         inc_pc();
         get_arg(float);
         buzzvm_pushf(vm, arg);
         break;
      }
      case BUZZVM_INSTR_PUSHCN: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_pushcn(vm, arg);
         break;
      }
      case BUZZVM_INSTR_PUSHCC: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_pushcc(vm, arg);
         break;
      }
      case BUZZVM_INSTR_LLOAD: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_lload(vm, arg);
         break;
      }
      case BUZZVM_INSTR_LSTORE: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_lstore(vm, arg);
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
         if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_NIL ||
            (buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_INT &&
             buzzvm_stack_at(vm, 1)->i.value == 0)) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         buzzvm_pop(vm);
         break;
      }
      case BUZZVM_INSTR_JUMPNZ: {
         inc_pc();
         get_arg(uint32_t);
         buzzvm_stack_assert(vm, 1);
         if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_NIL &&
            (buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_INT ||
             buzzvm_stack_at(vm, 1)->i.value != 0)) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         buzzvm_pop(vm);
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

uint16_t buzzvm_string_register(buzzvm_t vm,
                                const char* str) {
   /* Look for function name in the string list, add if missing */
   uint32_t sid = buzzdarray_string_find(vm, &str);
   if(sid == buzzdarray_size(vm->strings))
      buzzvm_string_add(vm, str);
   return sid;
}

/****************************************/
/****************************************/

const char* buzzvm_string_get(buzzvm_t vm,
                              uint16_t sid) {
   if(sid >= buzzdarray_size(vm->strings)) return NULL;
   return buzzdarray_get(vm->strings, sid, char*);
}

/****************************************/
/****************************************/

int buzzvm_function_cmp(const void* a, const void* b) {
   if(*(uintptr_t*)a < *(uintptr_t*)b) return -1;
   if(*(uintptr_t*)a > *(uintptr_t*)b) return  1;
   return 0;
}

uint32_t buzzvm_function_register(buzzvm_t vm,
                                  buzzvm_funp funp) {
   /* Look for function pointer to avoid duplicates */
   uint32_t fpos = buzzdarray_find(vm->flist, buzzvm_function_cmp, funp);
   if(fpos == buzzdarray_size(vm->flist)) {
      /* Add function to the list */
      buzzdarray_push(vm->flist, &funp);
   }
   return fpos;
}

/****************************************/
/****************************************/

int buzzvm_vstig_create(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 0);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Create virtual stigmergy if not present already */
   if(!buzzdict_get(vm->vstigs, &id, buzzdict_t)) {
      buzzvstig_t vs = buzzvstig_new();
      buzzdict_set(vm->vstigs, &id, &vs);
   }
   /* Push the vstig id on the stack */
   buzzvm_pushi(vm, id);
   /* Return */
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_vstig_put(buzzvm_t vm) {
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Get key */
   buzzvm_lload(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   /* Get value */
   buzzvm_lload(vm, 2);
   buzzobj_t v = buzzvm_stack_at(vm, 1);
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Found, prepare the PUT message */
      buzzdarray_t buf = buzzmsg_new(16);
      buzzmsg_serialize_u8(buf, BUZZMSG_VSTIG_PUT);
      buzzmsg_serialize_u16(buf, id);
      /* Update the element */
      buzzvstig_elem_t* x = buzzvstig_fetch(*vs, &k);
      if(x) {
         (*x)->data = v;
         ++((*x)->timestamp);
         (*x)->robot = vm->robot;
         buzzvstig_store(*vs, &k, x);
         buzzvstig_elem_serialize(buf, k, *x);
      }
      else {
         buzzvstig_elem_t y = buzzvstig_elem_new(v, 1, vm->robot);
         buzzvstig_store(*vs, &k, &y);
         buzzvstig_elem_serialize(buf, k, y);
      }
      /* Append the message to the out message queue */
      buzzmsg_queue_append(vm->outmsgs, buf);
   }
   else {
      /* Ignore commands for virtual stigmergy that is not there */
      fprintf(stderr, "[WARNING] [ROBOT %u] No virtual stigmergy with id %d\n", vm->robot, id);
   }
   /* Return */
   buzzvm_ret0(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_vstig_get(buzzvm_t vm) {
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] id = %u\n", id);
   /* Get key */
   buzzvm_lload(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   /* Prepare the QUERY message */
   buzzdarray_t buf = buzzmsg_new(16);
   buzzmsg_serialize_u8(buf, BUZZMSG_VSTIG_QUERY);
   buzzmsg_serialize_u16(buf, id);
   /* Look for virtual stigmergy */
   buzzdict_t* vs = buzzdict_get(vm->vstigs, &id, buzzdict_t);
   if(vs) {
      /* Virtual stigmergy found, look for key and push result */
      buzzvstig_elem_t* e = buzzvstig_fetch(*vs, &k);
      if(e) {
         buzzvm_push(vm, (*e)->data);
         buzzvstig_elem_serialize(buf, k, *e);
      }
      else {
         buzzvm_pushnil(vm);
         buzzvstig_elem_t x =
            buzzvstig_elem_new(buzzvm_stack_at(vm, 1),
                               1, vm->robot);
         buzzvstig_elem_serialize(buf, k, x);
      }
   }
   else {
      /* No virtual stigmergy found, push false */
      buzzvm_pushnil(vm);
         buzzvstig_elem_t x =
            buzzvstig_elem_new(buzzvm_stack_at(vm, 1),
                               1, vm->robot);
      buzzvstig_elem_serialize(buf, k, x);
   }
   /* Append the message to the out message queue */
   buzzmsg_queue_append(vm->outmsgs, buf);
   /* Return */
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/
