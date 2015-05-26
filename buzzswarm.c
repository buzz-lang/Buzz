#include "buzzswarm.h"
#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>

/****************************************/
/****************************************/

/*
 * An element in the data structure that stores robot
 * membership data.
 */
struct buzzswarm_elem_s {
   buzzdarray_t swarms;
   uint16_t age;
};
typedef struct buzzswarm_elem_s* buzzswarm_elem_t;

buzzswarm_elem_t buzzswarm_elem_new() {
   buzzswarm_elem_t e = (buzzswarm_elem_t)malloc(sizeof(struct buzzswarm_elem_s));
   e->swarms = buzzdarray_new(1, sizeof(uint16_t), NULL);
   e->age = 0;
   return e;
}

void buzzswarm_elem_destroy(const void* key, void* data, void* params) {
   buzzswarm_elem_t e = *(buzzswarm_elem_t*)data;
   buzzdarray_destroy(&(e->swarms));
   free(e);
}

/****************************************/
/****************************************/

buzzswarm_members_t buzzswarm_members_new() {
   return buzzdict_new(10,
                       sizeof(uint16_t),
                       sizeof(buzzswarm_elem_t),
                       buzzdict_uint16keyhash,
                       buzzdict_uint16keycmp,
                       buzzswarm_elem_destroy);
}

/****************************************/
/****************************************/

void buzzswarm_members_destroy(buzzswarm_members_t* m) {
   buzzdict_destroy(m);
}

/****************************************/
/****************************************/

void buzzswarm_members_join(buzzswarm_members_t m,
                            uint16_t robot,
                            uint16_t swarm) {
   /* Is an entry for the passed robot already present? */
   buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
   if(e) {
      /* Yes, update it */
      (*e)->age = 0;
      /* Search for the passed id; if found, nothing to do */
      uint32_t i;
      for(i = 0; i < buzzdarray_size((*e)->swarms); ++i) {
         if(buzzdarray_get((*e)->swarms, i, uint16_t) == swarm) return;
      }
      /* If we get here, it's because we got new information - add it */
      buzzdarray_push((*e)->swarms, &swarm);
   }
   else {
      /* No, create it */
      buzzswarm_elem_t ne = buzzswarm_elem_new();
      buzzdarray_push(ne->swarms, &swarm);
      /* Add it to the structure */
      buzzdict_set(m, &robot, &ne);
   }
}

/****************************************/
/****************************************/

void buzzswarm_members_leave(buzzswarm_members_t m,
                             uint16_t robot,
                             uint16_t swarm) {
   /* Is an entry for the passed robot present? */
   buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
   if(e) {
      /* Yes, update it */
      (*e)->age = 0;
      /* Search for the passed id */
      uint32_t i;
      for(i = 0; i < buzzdarray_size((*e)->swarms); ++i) {
         if(buzzdarray_get((*e)->swarms, i, uint16_t) == swarm) break;
      }
      /* If the element was found, remove it */
      if(i < buzzdarray_size((*e)->swarms))
         buzzdarray_remove((*e)->swarms, i);
      /* If no swarm id is known for this robot, remove the entry altogether */
      if(buzzdarray_isempty((*e)->swarms))
         buzzdict_remove(m, &robot);
   }
   /* Nothing to do if you get a 'leave' message for someone you don't know */
}

/****************************************/
/****************************************/

void buzzswarm_members_refresh(buzzswarm_members_t m,
                               uint16_t robot,
                               buzzdarray_t swarms) {
   /* Is an entry for the passed robot already present? */
   buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
   if(e) {
      /* Yes, update it */
      (*e)->age = 0;
      buzzdarray_destroy(&((*e)->swarms));
      (*e)->swarms = swarms;
   }
   else {
      /* No, create it */
      buzzswarm_elem_t ne = buzzswarm_elem_new();
      buzzdarray_destroy(&(ne->swarms));
      ne->swarms = swarms;
      /* Add it to the structure */
      buzzdict_set(m, &robot, &ne);
   }
}

/****************************************/
/****************************************/

int buzzswarm_members_isrobotin(buzzswarm_members_t m,
                                uint16_t robot,
                                uint16_t swarm) {
   /* Is an entry for the passed robot present?
    * If not, return false */
   buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
   if(!e) return 0;
   /* If we get here, an entry is present
    * Does it contain the passed swarm id? */
   uint32_t i;
   for(i = 0; i < buzzdarray_size((*e)->swarms); ++i) {
      if(buzzdarray_get((*e)->swarms, i, uint16_t) == swarm) return 1;
   }
   /* If we get here, it's because we couldn't find the id */
   return 0;
}

/****************************************/
/****************************************/

void print_swarm_elem(const void* key, void* data, void* params) {
   buzzswarm_elem_t e = *(buzzswarm_elem_t*)data;
   fprintf(stdout, "ROBOT %u -> R%u:", *(uint16_t*)params, *(uint16_t*)key);
   for(uint32_t i = 0; i < buzzdarray_size(e->swarms); ++i) {
      fprintf(stdout,
              " %u",
              buzzdarray_get(e->swarms, i, uint16_t));
   }
   fprintf(stdout, "\n");
}

void buzzswarm_members_print(buzzswarm_members_t m, uint16_t id) {
   fprintf(stdout, "ROBOT %u -> Swarm member table size: %u\n", id, buzzdict_size(m));
   buzzdict_foreach(m, print_swarm_elem, &id);
}

/****************************************/
/****************************************/

void buzzswarm_members_update(buzzswarm_members_t m) {
   // TODO
}

/****************************************/
/****************************************/

int create_table(buzzvm_t vm, uint16_t id) {
   /* Create a table and add data and methods */
   buzzobj_t t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "others"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_others));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "join"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_join));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "leave"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_leave));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "in"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_in));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "select"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_select));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "exec"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_swarm_exec));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_pushi(vm, id);
   buzzvm_tput(vm);
   /* Push the table on the stack */
   buzzvm_push(vm, t);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_swarm_create(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 1);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Created swarm id = %u\n", id);
   /* Add a new entry if necessary */
   if(!buzzdict_exists(vm->swarms, &id)) {
      uint8_t v = 0;
      buzzdict_set(vm->swarms, &id, &v);
   }
   /* Create a table to return */
   create_table(vm, id);
   /* Return */
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_swarm_others(buzzvm_t vm) {
   /* Get the id of the current swarm */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_tget(vm);
   uint16_t id1 = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Creating other swarm from id1 = %u\n", id1);
   /* Get the swarm entry */
   uint8_t* x = buzzdict_get(vm->swarms, &id1, uint8_t);
   if(!x) {
      vm->state = BUZZVM_STATE_ERROR;
      vm->error = BUZZVM_ERROR_SWARM;
      return BUZZVM_STATE_ERROR;
   }
   /* Get the id of the new swarm to create */
   buzzvm_lload(vm, 1);
   uint16_t id2 = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Created other swarm with id2 = %u\n", id2);
   /* Add a new entry for the swarm */
   uint8_t v = *x ? 0 : 1;
   buzzdict_set(vm->swarms, &id2, &v);
   /* Send update, if necessary */
   if(v)
      buzzoutmsg_queue_append_swarm_joinleave(
         vm->outmsgs, BUZZMSG_SWARM_JOIN, id2);
   /* Create a table to return */
   create_table(vm, id2);
   /* Return */
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_swarm_join(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Joining swarm with id = %u\n", id);
   /* Join the swarm, if known */
   if(buzzdict_exists(vm->swarms, &id)) {
      /* Store membership */
      uint8_t v = 1;
      buzzdict_set(vm->swarms, &id, &v);
      /* Send update */
      buzzoutmsg_queue_append_swarm_joinleave(
         vm->outmsgs, BUZZMSG_SWARM_JOIN, id);
      /* Return */
      buzzvm_ret0(vm);
      return BUZZVM_STATE_READY;
   }
   else {
      vm->state = BUZZVM_STATE_ERROR;
      vm->error = BUZZVM_ERROR_SWARM;
      return BUZZVM_STATE_ERROR;
   }
}

/****************************************/
/****************************************/

int buzzvm_swarm_leave(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Leaving swarm with id = %u\n", id);
   /* Leave the swarm, if known */
   if(buzzdict_exists(vm->swarms, &id)) {
      /* Store membership */
      uint8_t v = 0;
      buzzdict_set(vm->swarms, &id, &v);
      /* Send update */
      buzzoutmsg_queue_append_swarm_joinleave(
         vm->outmsgs, BUZZMSG_SWARM_LEAVE, id);
      /* Return */
      buzzvm_ret0(vm);
      return BUZZVM_STATE_READY;
   }
   else {
      vm->state = BUZZVM_STATE_ERROR;
      vm->error = BUZZVM_ERROR_SWARM;
      return BUZZVM_STATE_ERROR;
   }
}

/****************************************/
/****************************************/

int buzzvm_swarm_in(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Checking membership for swarm id = %u\n", id);
   /* Get the swarm entry */
   uint8_t* x = buzzdict_get(vm->swarms, &id, uint8_t);
   if(!x) {
      vm->state = BUZZVM_STATE_ERROR;
      vm->error = BUZZVM_ERROR_SWARM;
      return BUZZVM_STATE_ERROR;
   }
   /* Push the return value */
   buzzvm_pushi(vm, *x);
   /* Return */
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_swarm_select(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Get the result of the condition check */
   buzzvm_lload(vm, 1);
   uint8_t in = buzzvm_stack_at(vm, 1)->i.value;
   fprintf(stderr, "[DEBUG] Selecting membership for swarm id = %u: %u\n", id, in);
   /* Update the swarm, if known */
   if(buzzdict_exists(vm->swarms, &id)) {
      /* Store membership */
      buzzdict_set(vm->swarms, &id, &in);
      /* Send update */
      buzzoutmsg_queue_append_swarm_joinleave(
         vm->outmsgs,
         in ? BUZZMSG_SWARM_JOIN : BUZZMSG_SWARM_LEAVE,
         id);
      /* Return */
      buzzvm_ret0(vm);
      return BUZZVM_STATE_READY;
   }
   else {
      vm->state = BUZZVM_STATE_ERROR;
      vm->error = BUZZVM_ERROR_SWARM;
      return BUZZVM_STATE_ERROR;
   }
}

/****************************************/
/****************************************/

int buzzvm_swarm_exec(buzzvm_t vm) {
   /* Get the id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Get the swarm entry */
   uint8_t* x = buzzdict_get(vm->swarms, &id, uint8_t);
   if(!x) {
      vm->state = BUZZVM_STATE_ERROR;
      vm->error = BUZZVM_ERROR_SWARM;
      return BUZZVM_STATE_ERROR;
   }
   /* Check whether the robot is in the swarm */
   if(*x) {
      fprintf(stderr, "[DEBUG] Executing closure for swarm with id = %u\n", id);
      /* Get the closure */
      buzzvm_lload(vm, 1);
      buzzobj_t c = buzzvm_stack_at(vm, 1);
      /* Get rid of the current call structure */
      buzzvm_ret0(vm);
      /* Push the current swarm in the stack */
      buzzdarray_push(vm->swarmstack, &id);
      /* Call the closure */
      buzzvm_push(vm, c);
      int32_t numargs = 0;
      buzzvm_pushi(vm, numargs);
      buzzvm_calls(vm);
   }
   else {
      /* Get rid of the current call structure */
      buzzvm_ret0(vm);
   }
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/
