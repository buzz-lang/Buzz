#include "buzzvm.h"
#include "buzzvstig.h"
#include "buzzswarm.h"
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

buzzvm_lsyms_t buzzvm_lsyms_new(uint8_t isswarm,
                                buzzdarray_t syms) {
   buzzvm_lsyms_t s = (buzzvm_lsyms_t)malloc(sizeof(struct buzzvm_lsyms_s));
   s->isswarm = isswarm;
   s->syms = syms;
   return s;
}

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

#define buzzdarray_string_find(vm, str) buzzdarray_find(vm->strings, buzzvm_string_cmp, str)

/****************************************/
/****************************************/

void buzzvm_vstig_destroy(const void* key, void* data, void* params) {
   buzzvstig_destroy((buzzvstig_t*)data);
}

/****************************************/
/****************************************/

void buzzvm_inmsg_destroy(uint32_t pos, void* data, void* param) {
   buzzmsg_payload_destroy((buzzmsg_payload_t*)data);
}

void buzzvm_outmsg_destroy(uint32_t pos, void* data, void* param) {
   fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
}

void buzzvm_process_inmsgs(buzzvm_t vm) {
   /* Go through the messages */
   while(!buzzinmsg_queue_isempty(vm->inmsgs)) {
      /* Extract the message data */
      buzzmsg_payload_t msg = buzzinmsg_queue_extract(vm->inmsgs);
      /* Dispatch the message wrt its type in msg->payload[0] */
      switch(buzzmsg_payload_get(msg, 0)) {
         case BUZZMSG_SHOUT: {
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
            if(!vs) break;
            /* Virtual stigmergy found */
            /* Deserialize key and value from msg */
            buzzobj_t k;        // key
            buzzvstig_elem_t v; // value
            if(buzzvstig_elem_deserialize(&k, &v, msg, pos, vm) < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_PUT message received\n", vm->robot);
               break;
            }
            /* Deserialization successful */
            /* Fetch local vstig element */
            buzzvstig_elem_t* l = buzzvstig_fetch(*vs, &k);
            if((!l)                             || /* Element not found */
               ((*l)->timestamp < v->timestamp) || /* Local element is older */
               ((*l)->timestamp == v->timestamp &&
                (*l)->robot < v->robot)) {         /* Same timestamp and local id is lower */
               /* Local element must be updated */
               /* Store element */
               buzzvstig_store(*vs, &k, &v);
               /* Append a PUT message to the out message queue */
               buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, v);
               fprintf(stderr, "[DEBUG] robot %d relays <PUT, d=%d, ts=%d, r=%d>\n",
                       vm->robot, k->i.value, v->timestamp, v->robot);
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
            if(!vs) break;
            /* Virtual stigmergy found */
            /* Deserialize key and value from msg */
            buzzobj_t k;        // key
            buzzvstig_elem_t v; // value
            if(buzzvstig_elem_deserialize(&k, &v, msg, pos, vm) < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_PUT message received\n", vm->robot);
               break;
            }
            /* Deserialization successful */
            /* Fetch local vstig element */
            buzzvstig_elem_t* l = buzzvstig_fetch(*vs, &k);
            if((!l) ||                           /* Element not found */
               (*l)->timestamp < v->timestamp) { /* Local element is older */
               /* Store element */
               buzzvstig_store(*vs, &k, &v);
               /* Append a PUT message to the out message queue */
               buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, v);
               fprintf(stderr, "[DEBUG] robot %d sends <PUT, d=%d, ts=%d, r=%d> after QUERY\n",
                       vm->robot, k->i.value, v->timestamp, v->robot);
            }
            else if((*l)->timestamp > v->timestamp) {
               /* Local element is newer */
               /* Append a PUT message to the out message queue */
               buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, *l);
               fprintf(stderr, "[DEBUG] robot %d sends <PUT, d=%d, ts=%d, r=%d> after QUERY\n",
                       vm->robot, k->i.value, (*l)->timestamp, (*l)->robot);
            }
            break;
         }
         case BUZZMSG_SWARM_LIST: {
            fprintf(stderr, "[DEBUG] Received BUZZMSG_SWARM_LIST message\n");
            /* Deserialize robot id */
            uint16_t rid;
            int64_t pos = buzzmsg_deserialize_u16(&rid, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LIST message received\n", vm->robot);
               break;
            }
            /* Deserialize number of swarm ids */
            uint16_t nsids;
            pos = buzzmsg_deserialize_u16(&nsids, msg, pos);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LIST message received\n", vm->robot);
               break;
            }
            /* Deserialize swarm ids */
            buzzdarray_t sids = buzzdarray_new(nsids, sizeof(uint16_t), NULL);
            for(uint16_t i = 0; i < nsids; ++i) {
               pos = buzzmsg_deserialize_u16(buzzdarray_makeslot(sids, i), msg, pos);
               if(pos < 0) {
                  fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LIST message received\n", vm->robot);
                  break;
               }
            }
            /* Update the information */
            buzzswarm_members_refresh(vm->swarmmembers, rid, sids);
            break;
         }
         case BUZZMSG_SWARM_JOIN: {
            /* Deserialize robot id */
            uint16_t rid;
            int64_t pos = buzzmsg_deserialize_u16(&rid, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_JOIN message received\n", vm->robot);
               break;
            }
            /* Deserialize swarm id */
            uint16_t sid;
            pos = buzzmsg_deserialize_u16(&sid, msg, pos);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_JOIN message received\n", vm->robot);
               break;
            }
            /* Update the information */
            fprintf(stderr, "[DEBUG] [ROBOT %u] Received BUZZMSG_SWARM_JOIN (R%u, S%u)\n", vm->robot, rid, sid);
            buzzswarm_members_join(vm->swarmmembers, rid, sid);
            break;
         }
         case BUZZMSG_SWARM_LEAVE: {
            /* Deserialize robot id */
            uint16_t rid;
            int64_t pos = buzzmsg_deserialize_u16(&rid, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LEAVE message received\n", vm->robot);
               break;
            }
            /* Deserialize swarm id */
            uint16_t sid;
            pos = buzzmsg_deserialize_u16(&sid, msg, pos);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LEAVE message received\n", vm->robot);
               break;
            }
            /* Update the information */
            fprintf(stderr, "[DEBUG] [ROBOT %u] Received BUZZMSG_SWARM_LEAVE (R%u, S%u)\n", vm->robot, rid, sid);
            buzzswarm_members_leave(vm->swarmmembers, rid, sid);
            break;
         }
      }
      /* Get rid of the message */
      buzzmsg_payload_destroy(&msg);
   }
}

/****************************************/
/****************************************/

void buzzvm_darray_destroy(uint32_t pos,
                           void* data,
                           void* params) {
   buzzdarray_t* s = (buzzdarray_t*)data;
   buzzdarray_destroy(s);
}

void buzzvm_lsyms_destroy(uint32_t pos,
                          void* data,
                          void* params) {
   buzzvm_lsyms_t s = *(buzzvm_lsyms_t*)data;
   buzzdarray_destroy(&(s->syms));
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
                               sizeof(buzzvm_lsyms_t),
                               buzzvm_lsyms_destroy);
   vm->lsyms = NULL;
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
                             sizeof(uint16_t),
                             sizeof(uint8_t),
                             buzzdict_uint16keyhash,
                             buzzdict_uint16keycmp,
                             NULL);
   /* Create swarm stack */
   vm->swarmstack = buzzdarray_new(10,
                                   sizeof(uint16_t),
                                   NULL);
   /* Create swarm member structure */
   vm->swarmmembers = buzzswarm_members_new();
   /* Create message queues */
   vm->inmsgs = buzzinmsg_queue_new(20);
   vm->outmsgs = buzzoutmsg_queue_new(robot);
   /* Create virtual stigmergy. */
   vm->vstigs = buzzdict_new(10,
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
   buzzdarray_destroy(&(*vm)->swarmstack);
   buzzswarm_members_destroy(&((*vm)->swarmmembers));
   /* Get rid of the message queues */
   buzzdarray_foreach((*vm)->inmsgs, buzzvm_inmsg_destroy, NULL);
   buzzdarray_destroy(&(*vm)->inmsgs);
   buzzoutmsg_queue_destroy(&(*vm)->outmsgs);
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
   uint16_t count;
   memcpy(&count, bcode, sizeof(uint16_t));
   /* Go through the strings and store them */
   uint32_t i = sizeof(uint16_t);
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
    * Register function definitions
    * Stop when you find a 'nop'
    */
   while(vm->bcode[vm->pc] != BUZZVM_INSTR_NOP)
      if(buzzvm_step(vm) != BUZZVM_STATE_READY) return vm->state;
   buzzvm_step(vm);
   /*
    * Register global symbols
    */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_pushi(vm, vm->robot);
   buzzvm_gstore(vm);
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
   /*
    * Register swarm methods
    */
   /* Add 'swarm' table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_pusht(vm);
   t = buzzvm_stack_at(vm, 1);
   buzzvm_gstore(vm);
   /* Add the 'create' method */
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "create"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_create));
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
         buzzmsg_payload_t buf = buzzmsg_payload_new(16);
         buzzobj_serialize(buf, var);
         /* Append it to the out message queue */
         buzzoutmsg_queue_append_shout(vm->outmsgs, buf);
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
      case BUZZVM_INSTR_CALLS: {
         inc_pc();
         buzzvm_callc(vm);
         assert_pc(vm->pc);
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

buzzvm_state buzzvm_function_call(buzzvm_t vm,
                                  const char* fname,
                                  uint32_t argc) {
   /* Save arguments */
   buzzobj_t* args;
   if(argc > 0) {
      buzzvm_stack_assert(vm, argc);
      args = (buzzobj_t*)malloc(sizeof(buzzobj_t) * argc);
      for(int i = 1; i <= argc; ++i) {
         args[argc - i] = buzzvm_stack_at(vm, i);
         buzzvm_pop(vm);
      }
   }
   /* Push the function name (return with error if not found) */
   buzzvm_pushs(vm, buzzdarray_string_find(vm, &fname));
   /* Get associated symbol */
   buzzvm_gload(vm);
   /* Make sure it's a closure */
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   /* Push back the arguments and the count */
   if(argc > 0) {
      for(int i = 0; i < argc; ++i)
         buzzvm_push(vm, args[i]);
      free(args);
   }
   buzzvm_pushi(vm, argc);
   /* Save the current stack depth */
   uint32_t stacks = buzzdarray_size(vm->stacks);
   /* Call the closure and keep stepping until
    * the stack count is back to the saved value */
   buzzvm_callc(vm);
   do if(buzzvm_step(vm) != BUZZVM_STATE_READY) return (vm)->state;
   while(stacks < buzzdarray_size(vm->stacks));
   return (vm)->state;
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
