#include "buzzvm.h"
#include "buzzvstig.h"
#include "buzzswarm.h"
#include "buzzmath.h"
#include "buzzio.h"
#include "buzzstring.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/****************************************/
/****************************************/

const char *buzzvm_state_desc[] = { "no code", "ready", "done", "error", "stopped" };

const char *buzzvm_error_desc[] = { "none", "unknown instruction", "stack error", "wrong number of local variables", "pc out of range", "function id out of range", "type mismatch", "unknown string id", "unknown swarm id" };

const char *buzzvm_instr_desc[] = {"nop", "done", "pushnil", "dup", "pop", "ret0", "ret1", "add", "sub", "mul", "div", "mod", "pow", "unm", "and", "or", "not", "eq", "neq", "gt", "gte", "lt", "lte", "gload", "gstore", "pusht", "tput", "tget", "callc", "calls", "pushf", "pushi", "pushs", "pushcn", "pushcc", "pushl", "lload", "lstore", "jump", "jumpz", "jumpnz"};

static uint16_t SWARM_BROADCAST_PERIOD = 10;

/****************************************/
/****************************************/

void buzzvm_dump(buzzvm_t vm) {
   int64_t i, j;
   fprintf(stderr, "============================================================\n");
   fprintf(stderr, "state: %d\terror: %d\n", vm->state, vm->error);
   fprintf(stderr, "code size: %u\tpc: %d\n", vm->bcode_size, vm->pc);
   fprintf(stderr, "stacks: %" PRId64 "\tcur elem: %" PRId64 " (size %" PRId64 ")\n", buzzdarray_size(vm->stacks), buzzvm_stack_top(vm), buzzvm_stack_top(vm));
   for(i = buzzdarray_size(vm->stacks)-1; i >= 0 ; --i) {
      fprintf(stderr, "===== stack: %" PRId64 " =====\n", i);
      for(j = buzzdarray_size(buzzdarray_get(vm->stacks, i, buzzdarray_t)) - 1; j >= 0; --j) {
         fprintf(stderr, "\t%" PRId64 "\t", j);
         buzzobj_t o = buzzdarray_get(buzzdarray_get(vm->stacks, i, buzzdarray_t), j, buzzobj_t);
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
   }
   fprintf(stderr, "============================================================\n\n");
}

/****************************************/
/****************************************/

const char* buzzvm_strerror(buzzvm_t vm) {
   static const char* noerr = "No error";
   return vm->state == BUZZVM_STATE_ERROR ?
      vm->errormsg
      :
      noerr;
}

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

void buzzvm_lsyms_destroy(uint32_t pos,
                          void* data,
                          void* params) {
   buzzvm_lsyms_t s = *(buzzvm_lsyms_t*)data;
   buzzdarray_destroy(&(s->syms));
   free(s);
}

/****************************************/
/****************************************/

void buzzvm_vstig_destroy(const void* key, void* data, void* params) {
   free((void*)key);
   buzzvstig_destroy((buzzvstig_t*)data);
   free(data);
}

/****************************************/
/****************************************/

void buzzvm_outmsg_destroy(uint32_t pos, void* data, void* param) {
   fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
}

void buzzvm_process_inmsgs(buzzvm_t vm) {
   /* Go through the messages */
   while(!buzzinmsg_queue_isempty(vm->inmsgs)) {
      /* Make sure the VM is in the right state */
      if(vm->state != BUZZVM_STATE_READY) return;
      /* Extract the message data */
      uint16_t rid;
      buzzmsg_payload_t msg;
      buzzinmsg_queue_extract(vm, &rid, &msg);
      /* Dispatch the message wrt its type in msg->payload[0] */
      switch(buzzmsg_payload_get(msg, 0)) {
         case BUZZMSG_BROADCAST: {
            /* Deserialize the topic */
            buzzobj_t topic;
            int64_t pos = buzzobj_deserialize(&topic, msg, 1, vm);
            /* Make sure there's a listener to call */
            const buzzobj_t* l = buzzdict_get(vm->listeners, &topic->s.value.sid, buzzobj_t);
            if(!l) {
               /* No listener, ignore message */
               break;
            }
            /* Deserialize value */
            buzzobj_t value;
            pos = buzzobj_deserialize(&value, msg, pos, vm);
            /* Make an object for the robot id */
            buzzobj_t rido = buzzheap_newobj(vm, BUZZTYPE_INT);
            rido->i.value = rid;
            /* Call listener */
            buzzvm_push(vm, *l);
            buzzvm_push(vm, topic);
            buzzvm_push(vm, value);
            buzzvm_push(vm, rido);
            buzzvm_closure_call(vm, 3);
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
            const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
            if(!vs) break;
            /* Virtual stigmergy found */
            /* Deserialize key and value from msg */
            buzzobj_t k;          // key
            buzzvstig_elem_t v =  // value
               (buzzvstig_elem_t)malloc(sizeof(struct buzzvstig_elem_s));
            if(buzzvstig_elem_deserialize(&k, &v, msg, pos, vm) < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_PUT message received\n", vm->robot);
               free(v);
               break;
            }
            /* Deserialization successful */
            /* Fetch local vstig element */
            const buzzvstig_elem_t* l = buzzvstig_fetch(*vs, &k);
            if((!l)                             || /* Element not found */
               ((*l)->timestamp < v->timestamp)) { /* Local element is older */
               /* Local element must be updated */
               /* Store element */
               buzzvstig_store(*vs, &k, &v);
               buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, v);
            }
            else if(((*l)->timestamp == v->timestamp) && /* Same timestamp */
                    ((*l)->robot != v->robot)) {         /* Different robot */
               /* Conflict! */
               /* Call conflict manager */
               buzzvstig_elem_t c =
                  buzzvstig_onconflict_call(vm, *vs, k, *l, v);
               if(!c) {
                  fprintf(stderr, "[WARNING] [ROBOT %u] Error resolving PUT conflict\n", vm->robot);
                  break;
               }
               /* Get rid of useless vstig element */
               free(v);
               /* Did this robot lose the conflict? */
               if((c->robot != vm->robot) &&
                  ((*l)->robot == vm->robot)) {
                  /* Yes */
                  /* Save current local entry */
                  buzzvstig_elem_t ol = buzzvstig_elem_clone(vm, *l);
                  /* Store winning value */
                  buzzvstig_store(*vs, &k, &c);
                  /* Call conflict lost manager */
                  buzzvstig_onconflictlost_call(vm, *vs, k, ol);
               }
               else {
                  /* This robot did not lose the conflict */
                  /* Just propagate the PUT message */
                  buzzvstig_store(*vs, &k, &c);
               }
               buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, c);
            }
            else {
               /* Remote element is older, ignore it */
               /* Get rid of useless vstig element */
               free(v);
            }
            break;
         }
         case BUZZMSG_VSTIG_QUERY: {
            /* Deserialize the vstig id */
            uint16_t id;
            int64_t pos = buzzmsg_deserialize_u16(&id, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_QUERY message received (1)\n", vm->robot);
               break;
            }
            /* Deserialize key and value from msg */
            buzzobj_t k;         // key
            buzzvstig_elem_t v = // value
               (buzzvstig_elem_t)malloc(sizeof(struct buzzvstig_elem_s));
            if(buzzvstig_elem_deserialize(&k, &v, msg, pos, vm) < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_VSTIG_QUERY message received (2)\n", vm->robot);
               free(v);
               break;
            }
            /* Look for virtual stigmergy */
            const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
            if(!vs) {
               /* Virtual stigmergy not found, simply propagate the message */
               buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_QUERY, id, k, v);
               free(v);
               break;
            }
            /* Virtual stigmergy found */
            /* Fetch local vstig element */
            const buzzvstig_elem_t* l = buzzvstig_fetch(*vs, &k);
            if(!l) {
               /* Element not found */
               if(v->data->o.type == BUZZTYPE_NIL) {
                  /* This robot knows nothing about the query, just propagate it */
                  buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_QUERY, id, k, v);
                  free(v);
               }
               else {
                  /* Store element and propagate PUT message */
                  buzzvstig_store(*vs, &k, &v);
                  buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, v);
               }
               break;
            }
            /* Element found */
            if((*l)->timestamp < v->timestamp) {
               /* Local element is older */
               /* Store element */
               buzzvstig_store(*vs, &k, &v);
               buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, v);
            }
            else if((*l)->timestamp > v->timestamp) {
               /* Local element is newer */
               /* Append a PUT message to the out message queue */
               buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, *l);
               free(v);
            }
            else if(((*l)->timestamp == v->timestamp) && /* Same timestamp */
                    ((*l)->robot != v->robot)) {         /* Different robot */
               /* Conflict! */
               /* Call conflict manager */
               buzzvstig_elem_t c =
                  buzzvstig_onconflict_call(vm, *vs, k, *l, v);
               free(v);
               /* Make sure conflict manager returned with an element to process */
               if(!c) break;
               /* Did this robot lose the conflict? */
               if((c->robot != vm->robot) &&
                  ((*l)->robot == vm->robot)) {
                  /* Yes */
                  /* Save current local entry */
                  buzzvstig_elem_t ol = buzzvstig_elem_clone(vm, *l);
                  /* Store winning value */
                  buzzvstig_store(*vs, &k, &c);
                  /* Call conflict lost manager */
                  buzzvstig_onconflictlost_call(vm, *vs, k, ol);
               }
               else {
                  /* This robot did not lose the conflict */
                  /* Just propagate the PUT message */
                  buzzvstig_store(*vs, &k, &c);
               }
               buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, c);
            }
            else {
               /* Remote element is same as local, ignore it */
               /* Get rid of useless vstig element */
               free(v);
            }
            break;
         }
         case BUZZMSG_SWARM_LIST: {
            /* Deserialize number of swarm ids */
            uint16_t nsids;
            int64_t pos = buzzmsg_deserialize_u16(&nsids, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LIST message received\n", vm->robot);
               break;
            }
            if(nsids < 1) break;
            /* Deserialize swarm ids */
            buzzdarray_t sids = buzzdarray_new(nsids, sizeof(uint16_t), NULL);
            uint16_t i;
            for(i = 0; i < nsids; ++i) {
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
            /* Deserialize swarm id */
            uint16_t sid;
            int64_t pos = buzzmsg_deserialize_u16(&sid, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_JOIN message received\n", vm->robot);
               break;
            }
            /* Update the information */
            buzzswarm_members_join(vm->swarmmembers, rid, sid);
            break;
         }
         case BUZZMSG_SWARM_LEAVE: {
            /* Deserialize swarm id */
            uint16_t sid;
            int64_t pos = buzzmsg_deserialize_u16(&sid, msg, 1);
            if(pos < 0) {
               fprintf(stderr, "[WARNING] [ROBOT %u] Malformed BUZZMSG_SWARM_LEAVE message received\n", vm->robot);
               break;
            }
            /* Update the information */
            buzzswarm_members_leave(vm->swarmmembers, rid, sid);
            break;
         }
      }
      /* Get rid of the message */
      buzzmsg_payload_destroy(&msg);
   }
   /* Update swarm membership */
   buzzswarm_members_update(vm->swarmmembers);
}

/****************************************/
/****************************************/

void buzzvm_process_outmsgs(buzzvm_t vm) {
   /* Must broadcast swarm list message? */
   if(vm->swarmbroadcast > 0)
      --vm->swarmbroadcast;
   if(vm->swarmbroadcast == 0 &&
      !buzzdict_isempty(vm->swarms)) {
      vm->swarmbroadcast = SWARM_BROADCAST_PERIOD;
      buzzoutmsg_queue_append_swarm_list(vm,
                                         vm->swarms);
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

buzzvm_t buzzvm_new(uint16_t robot) {
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
   vm->strings = buzzstrman_new();
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
   vm->swarmbroadcast = SWARM_BROADCAST_PERIOD;
   /* Create message queues */
   vm->inmsgs = buzzinmsg_queue_new();
   vm->outmsgs = buzzoutmsg_queue_new(robot);
   /* Create virtual stigmergy */
   vm->vstigs = buzzdict_new(10,
                             sizeof(uint16_t),
                             sizeof(buzzvstig_t),
                             buzzdict_uint16keyhash,
                             buzzdict_uint16keycmp,
                             buzzvm_vstig_destroy);
   /* Create virtual stigmergy */
   vm->listeners = buzzdict_new(10,
                                sizeof(uint16_t),
                                sizeof(buzzobj_t),
                                buzzdict_uint16keyhash,
                                buzzdict_uint16keycmp,
                                NULL);
   /* Take care of the robot id */
   vm->robot = robot;
   /* Initialize empty random number generator (buzzvm_math takes care of creating it) */
   vm->rngstate = NULL;
   vm->rngidx = 0;
   /* Return new vm */
   return vm;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   /* Get rid of the rng state */
   free((*vm)->rngstate);
   /* Get rid of the stack */
   buzzstrman_destroy(&(*vm)->strings);
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
   buzzinmsg_queue_destroy(&(*vm)->inmsgs);
   buzzoutmsg_queue_destroy(&(*vm)->outmsgs);
   /* Get rid of the virtual stigmergy structures */
   buzzdict_destroy(&(*vm)->vstigs);
   /* Get rid of neighbor value listeners */
   buzzdict_destroy(&(*vm)->listeners);
   free(*vm);
   *vm = 0;
}

/****************************************/
/****************************************/

void buzzvm_seterror(buzzvm_t vm,
                     buzzvm_error errcode,
                     const char* errmsg,
                     ...) {
   /* Set error state */
   vm->state = BUZZVM_STATE_ERROR;
   vm->error = errcode;
   /* Get rid of old error message */
   if(vm->errormsg) free(vm->errormsg);
   /* Was a custom message passed? */
   if(errmsg) {
      /* Yes, use it */
      /* Compose user-defined error message */
      char* msg;
      va_list al;
      va_start(al, errmsg);
      vasprintf(&msg, errmsg, al);
      va_end(al);
      /* Concatenate error description and user defined message */
      asprintf(&vm->errormsg,
               "%s: %s",
               buzzvm_error_desc[vm->error],
               msg);
      /* Get rid of user-defined error message */
      free(msg);
   }
   else {
      /* No, use the default error description */
      vm->errormsg = strdup(buzzvm_error_desc[vm->error]);
   }
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
      buzzvm_string_register(vm, (char*)(bcode + i), 1);
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
   vm->oldpc = vm->pc;
   /*
    * Register function definitions
    * Stop when you find a 'nop'
    */
   while(vm->bcode[vm->pc] != BUZZVM_INSTR_NOP)
      if(buzzvm_step(vm) != BUZZVM_STATE_READY) return vm->state;
   buzzvm_step(vm);
   /* Initialize empty neighbors */
   buzzneighbors_reset(vm);
   /* Register robot id */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id", 1));
   buzzvm_pushi(vm, vm->robot);
   buzzvm_gstore(vm);
   /* Register basic functions */
   buzzobj_register(vm);
   /* Register stigmergy methods */
   buzzvstig_register(vm);
   /* Register swarm methods */
   buzzswarm_register(vm);
   /* Register math methods */
   buzzmath_register(vm);
   /* Register io methods */
   buzzio_register(vm);
   /* Register string methods */
   buzzstring_register(vm);
   /* All done */
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { buzzvm_seterror(vm, BUZZVM_ERROR_PC, NULL); return vm->state; }

#define inc_pc() vm->oldpc = vm->pc; ++vm->pc; assert_pc(vm->pc);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(vm->bcode + vm->pc)); vm->pc += sizeof(TYPE);

buzzvm_state buzzvm_step(buzzvm_t vm) {
   /* buzzvm_dump(vm); */
   /* Can't execute if not ready */
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Execute GC */
   buzzheap_gc(vm);
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
      case BUZZVM_INSTR_DUP: {
         inc_pc();
         buzzvm_dup(vm);
         break;
      }
      case BUZZVM_INSTR_POP: {
         if(buzzvm_pop(vm) != BUZZVM_STATE_READY) return vm->state;
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_RET0: {
         if(buzzvm_ret0(vm) != BUZZVM_STATE_READY) return vm->state;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_RET1: {
         if(buzzvm_ret1(vm) != BUZZVM_STATE_READY) return vm->state;
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
      case BUZZVM_INSTR_UNM: {
         buzzvm_unm(vm);
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
      case BUZZVM_INSTR_GLOAD: {
         inc_pc();
         buzzvm_gload(vm);
         break;
      }
      case BUZZVM_INSTR_GSTORE: {
         inc_pc();
         if(buzzvm_gstore(vm) != BUZZVM_STATE_READY) return vm->state;
         break;
      }
      case BUZZVM_INSTR_PUSHT: {
         buzzvm_pusht(vm);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_TPUT: {
         if(buzzvm_tput(vm) != BUZZVM_STATE_READY) return vm->state;
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_TGET: {
         if(buzzvm_tget(vm) != BUZZVM_STATE_READY) return vm->state;
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_CALLC: {
         inc_pc();
         if(buzzvm_callc(vm) != BUZZVM_STATE_READY) return vm->state;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_CALLS: {
         inc_pc();
         if(buzzvm_calls(vm) != BUZZVM_STATE_READY) return vm->state;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_PUSHF: {
         inc_pc();
         get_arg(float);
         if(buzzvm_pushf(vm, arg) != BUZZVM_STATE_READY) return vm->state;
         break;
      }
      case BUZZVM_INSTR_PUSHI: {
         inc_pc();
         get_arg(int32_t);
         if(buzzvm_pushi(vm, arg) != BUZZVM_STATE_READY) return vm->state;
         break;
      }
      case BUZZVM_INSTR_PUSHS: {
         inc_pc();
         get_arg(int32_t);
         if(buzzvm_pushs(vm, arg) != BUZZVM_STATE_READY) return vm->state;
         break;
      }
      case BUZZVM_INSTR_PUSHCN: {
         inc_pc();
         get_arg(uint32_t);
         if(buzzvm_pushcn(vm, arg) != BUZZVM_STATE_READY) return vm->state;
         break;
      }
      case BUZZVM_INSTR_PUSHCC: {
         inc_pc();
         get_arg(uint32_t);
         if(buzzvm_pushcc(vm, arg) != BUZZVM_STATE_READY) return vm->state;
         break;
      }
      case BUZZVM_INSTR_PUSHL: {
         inc_pc();
         get_arg(uint32_t);
         if(buzzvm_pushl(vm, arg) != BUZZVM_STATE_READY) return vm->state;
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
         buzzvm_seterror(vm, BUZZVM_ERROR_INSTR, NULL);
         break;
   }
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_execute_script(buzzvm_t vm) {
   while(buzzvm_step(vm) == BUZZVM_STATE_READY);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_closure_call(buzzvm_t vm,
                                 uint32_t argc) {
   /* Insert the self table right before the closure */
   buzzdarray_insert(vm->stack,
                     buzzdarray_size(vm->stack) - argc - 1,
                     buzzheap_newobj(vm, BUZZTYPE_NIL));
   /* Push the argument count */
   buzzvm_pushi(vm, argc);
   /* Save the current stack depth */
   uint32_t stacks = buzzdarray_size(vm->stacks);
   /* Call the closure and keep stepping until
    * the stack count is back to the saved value */
   buzzvm_callc(vm);
   do if(buzzvm_step(vm) != BUZZVM_STATE_READY) return vm->state;
   while(stacks < buzzdarray_size(vm->stacks));
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_function_call(buzzvm_t vm,
                                  const char* fname,
                                  uint32_t argc) {
   /* Reset the VM state if it's DONE */
   if(vm->state == BUZZVM_STATE_DONE)
      vm->state = BUZZVM_STATE_READY;
   /* Don't continue if the VM has an error */
   if(vm->state != BUZZVM_STATE_READY)
      return vm->state;
   /* Push the function name (return with error if not found) */
   buzzvm_pushs(vm, buzzvm_string_register(vm, fname, 0));
   /* Get associated symbol */
   buzzvm_gload(vm);
   /* Make sure it's a closure */
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_NIL) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "cannot find function '%s()'",
                      fname);
      return vm->state;
   }
   if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_CLOSURE) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_TYPE,
                      "function '%s()': expected closure, got %s",
                      fname,
                      buzztype_desc[buzzvm_stack_at(vm, 1)->o.type]
         );
      return vm->state;
   }
   /* Move closure before arguments */
   if(argc > 0) {
      buzzdarray_insert(vm->stack,
                        buzzdarray_size(vm->stack) - argc - 1,
                        buzzvm_stack_at(vm, 1));
      buzzvm_pop(vm);
   }
   /* Call the closure */
   return buzzvm_closure_call(vm, argc);
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

buzzvm_state buzzvm_call(buzzvm_t vm, int isswrm) {
   /* Get argument number and pop it */
   buzzvm_stack_assert(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   int32_t argn = buzzvm_stack_at(vm, 1)->i.value;
   buzzvm_pop(vm);
   /* Make sure the stack has enough elements */
   buzzvm_stack_assert(vm, argn+1);
   /* Make sure the closure is where expected */
   buzzvm_type_assert(vm, argn+1, BUZZTYPE_CLOSURE);
   buzzobj_t c = buzzvm_stack_at(vm, argn+1);
   /* Make sure that that data about C closures is correct */
   if((!c->c.value.isnative) &&
      ((c->c.value.ref) >= buzzdarray_size(vm->flist))) {
      buzzvm_seterror(vm, BUZZVM_ERROR_FLIST, NULL);
      return vm->state;
   }
   /* Create a new local symbol list copying the parent's */
   vm->lsyms =
      buzzvm_lsyms_new(isswrm,
                       buzzdarray_clone(c->c.value.actrec));
   buzzdarray_push(vm->lsymts, &(vm->lsyms));
   /* Add function arguments to the local symbols */
   int32_t i;
   for(i = argn; i > 0; --i)
      buzzdarray_push(vm->lsyms->syms,
                      &buzzdarray_get(vm->stack,
                                      buzzdarray_size(vm->stack) - i,
                                      buzzobj_t));
   /* Get rid of the function arguments */
   for(i = argn+1; i > 0; --i)
      buzzdarray_pop(vm->stack);
   /* Pop unused self table */
   buzzdarray_pop(vm->stack);
   /* Push return address */
   buzzvm_pushi((vm), vm->pc);
   /* Make a new stack for the function */
   vm->stack = buzzdarray_new(1, sizeof(buzzobj_t), NULL);
   buzzdarray_push(vm->stacks, &(vm->stack));
   /* Jump to/execute the function */
   if(c->c.value.isnative) {
      vm->oldpc = vm->pc;
      vm->pc = c->c.value.ref;
   }
   else buzzdarray_get(vm->flist,
                       c->c.value.ref,
                       buzzvm_funp)(vm);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pop(buzzvm_t vm) {
   if(buzzdarray_isempty(vm->stack)) {
      buzzvm_seterror(vm, BUZZVM_ERROR_STACK, "empty stack");
      return vm->state;
   }
   else {
      buzzdarray_pop(vm->stack);
   }
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_dup(buzzvm_t vm) {
   if(buzzdarray_isempty(vm->stack)) {
      buzzvm_seterror(vm, BUZZVM_ERROR_STACK, "empty stack");
      return vm->state;
   }
   else {
      buzzobj_t x = buzzvm_stack_at(vm, 1);
      buzzdarray_push(vm->stack, &x);
   }
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_push(buzzvm_t vm, buzzobj_t v) {
   buzzdarray_push(vm->stack, &v);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushu(buzzvm_t vm, void* v) {
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_USERDATA);
   o->u.value = v;
   buzzvm_push(vm, o);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushnil(buzzvm_t vm) {
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_NIL);
   buzzvm_push(vm, o);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushc(buzzvm_t vm, int32_t rfrnc, int32_t nat) {
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_CLOSURE);
   o->c.value.isnative = nat;
   o->c.value.ref = rfrnc;
   buzzobj_t nil = buzzheap_newobj(vm, BUZZTYPE_NIL);
   buzzdarray_push(o->c.value.actrec, &nil);
   buzzvm_push(vm, o);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushi(buzzvm_t vm, int32_t v) {
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_INT);
   o->i.value = v;
   buzzvm_push(vm, o);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushf(buzzvm_t vm, float v) {
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_FLOAT);
   o->f.value = v;
   buzzvm_push(vm, o);
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushs(buzzvm_t vm, uint16_t strid) {
   if(!buzzstrman_get(vm->strings, strid)) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_STRING,
                      "id read = %" PRIu16,
                      strid);
      return vm->state;
   }
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_STRING);
   o->s.value.sid = (strid);
   o->s.value.str = buzzstrman_get(vm->strings, strid);
   buzzvm_push(vm, o);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_pushl(buzzvm_t vm, int32_t addr) {
   buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_CLOSURE);
   o->c.value.isnative = 1;
   o->c.value.ref = addr;
   if(vm->lsyms) {
      int i;
      for(i = 0; i < buzzdarray_size(vm->lsyms->syms); ++i)
         buzzdarray_push(o->c.value.actrec,
                         &buzzdarray_get(vm->lsyms->syms,
                                         i, buzzobj_t));
   }
   else {
      buzzobj_t nil = buzzheap_newobj(vm, BUZZTYPE_NIL);
      buzzdarray_push(o->c.value.actrec,
                      &nil);
   }
   return buzzvm_push(vm, o);
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_tput(buzzvm_t vm) {
   buzzvm_stack_assert(vm, 3);
   buzzvm_type_assert(vm, 3, BUZZTYPE_TABLE);
   buzzobj_t v = buzzvm_stack_at(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 2);
   buzzobj_t t = buzzvm_stack_at(vm, 3);
   buzzvm_pop(vm);
   buzzvm_pop(vm);
   buzzvm_pop(vm);
   if(k->o.type != BUZZTYPE_INT &&
      k->o.type != BUZZTYPE_FLOAT &&
      k->o.type != BUZZTYPE_STRING) {
      buzzvm_seterror(vm, BUZZVM_ERROR_TYPE, "a %s value can't be used as table key", buzztype_desc[k->o.type]);
      return vm->state;
   }
   if(v->o.type == BUZZTYPE_NIL) {
      /* Nil, erase entry */
      buzzdict_remove(t->t.value, &k);
   }
   else if(v->o.type == BUZZTYPE_CLOSURE) {
      /* Method call */
      int i;
      buzzobj_t o = buzzheap_newobj(vm, BUZZTYPE_CLOSURE);
      o->c.value.isnative = v->c.value.isnative;
      o->c.value.ref = v->c.value.ref;
      buzzdarray_push(o->c.value.actrec, &t);
      for(i = 1; i < buzzdarray_size(v->c.value.actrec); ++i)
         buzzdarray_push(o->c.value.actrec,
                         &buzzdarray_get(v->c.value.actrec,
                                         i, buzzobj_t));
      buzzdict_set(t->t.value, &k, &o);
   }
   else {
      buzzdict_set(t->t.value, &k, &v);
   }
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_tget(buzzvm_t vm) {
   buzzvm_stack_assert(vm, 2);
   buzzvm_type_assert(vm, 2, BUZZTYPE_TABLE);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   buzzobj_t t = buzzvm_stack_at(vm, 2);
   buzzvm_pop(vm);
   buzzvm_pop(vm);
   if(k->o.type != BUZZTYPE_INT &&
      k->o.type != BUZZTYPE_FLOAT &&
      k->o.type != BUZZTYPE_STRING) {
      buzzvm_seterror(vm, BUZZVM_ERROR_TYPE, "a %s value can't be used as table key", k->o.type);
      return vm->state;
   }
   const buzzobj_t* v = buzzdict_get(t->t.value, &k, buzzobj_t);
   if(v) buzzvm_push(vm, *v);
   else buzzvm_pushnil(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_gload(buzzvm_t vm) {
   buzzvm_stack_assert(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   buzzobj_t str = buzzvm_stack_at(vm, 1);
   buzzvm_pop(vm);
   const buzzobj_t* o = buzzdict_get(vm->gsyms, &(str->s.value.sid), buzzobj_t);
   if(!o) { buzzvm_pushnil(vm); }
   else { buzzvm_push(vm, (*o)); }
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_gstore(buzzvm_t vm) {
   buzzvm_stack_assert((vm), 2);
   buzzvm_type_assert((vm), 2, BUZZTYPE_STRING);
   buzzobj_t str = buzzvm_stack_at((vm), 2);
   buzzobj_t o = buzzvm_stack_at((vm), 1);
   buzzvm_pop(vm);
   buzzvm_pop(vm);
   buzzdict_set((vm)->gsyms, &(str->s.value.sid), &o);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_ret0(buzzvm_t vm) {
   /* Pop swarm stack */
   if(vm->lsyms->isswarm)
      buzzdarray_pop(vm->swarmstack);
   /* Pop local symbol table */
   buzzdarray_pop(vm->lsymts);
   /* Set local symbol table pointer */
   vm->lsyms = !buzzdarray_isempty(vm->lsymts) ?
      buzzdarray_last(vm->lsymts, buzzvm_lsyms_t) :
      NULL;
   /* Pop stack */
   buzzdarray_pop(vm->stacks);
   /* Set stack pointer */
   vm->stack = buzzdarray_last(vm->stacks, buzzdarray_t);
   /* Make sure the stack contains at least one element */
   buzzvm_stack_assert(vm, 1);
   /* Make sure that element is an integer */
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   /* Use that element as program counter */
   vm->oldpc = vm->pc;
   vm->pc = buzzvm_stack_at(vm, 1)->i.value;
   /* Pop the return address */
   buzzvm_pop(vm);
   /* Push nil as the return value */
   return buzzvm_pushnil(vm);
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_ret1(buzzvm_t vm) {
   /* Pop swarm stack */
   if(vm->lsyms->isswarm)
      buzzdarray_pop(vm->swarmstack);
   /* Pop local symbol table */
   buzzdarray_pop(vm->lsymts);
   /* Set local symbol table pointer */
   vm->lsyms = !buzzdarray_isempty(vm->lsymts) ?
      buzzdarray_last(vm->lsymts, buzzvm_lsyms_t) :
      NULL;
   /* Make sure there's an element on the stack */
   buzzvm_stack_assert(vm, 1);
   /* Save it, it's the return value to pass to the lower stack */
   buzzobj_t ret = buzzvm_stack_at(vm, 1);
   /* Pop stack */
   buzzdarray_pop(vm->stacks);
   /* Set stack pointer */
   vm->stack = buzzdarray_last(vm->stacks, buzzdarray_t);
   /* Make sure the stack contains at least one element */
   buzzvm_stack_assert(vm, 1);
   /* Make sure that element is an integer */
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   /* Use that element as program counter */
   vm->oldpc = vm->pc;
   vm->pc = buzzvm_stack_at(vm, 1)->i.value;
   /* Pop the return address */
   buzzvm_pop(vm);
   /* Push the return value */
   return buzzvm_push(vm, ret);
}

/****************************************/
/****************************************/

buzzvm_state buzzvm_lload(buzzvm_t vm, uint32_t idx) {
   /* Make sure there are sufficient local symbols in the stack */
   if(buzzvm_lnum(vm) < idx) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_LNUM,
                      "not enough local symbols in stack (maybe you called a function with an insufficient number of parameters?)"
         );
      return vm->state;
   }
   /* Return the local symbol */
   buzzvm_push(vm, buzzdarray_get(vm->lsyms->syms, idx, buzzobj_t));
   return vm->state;
}
