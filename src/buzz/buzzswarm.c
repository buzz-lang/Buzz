#include "buzzswarm.h"
#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>

/****************************************/
/****************************************/

#define function_register(FNAME)                                        \
   buzzvm_dup(vm);                                                      \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));             \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzswarm_ ## FNAME)); \
   buzzvm_tput(vm);

#define id_get()                                          \
   buzzvm_lload(vm, 0);                                   \
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id", 1)); \
   buzzvm_tget(vm);                                       \
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;

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
   free((void*)key);
   free(data);
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
   const buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
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
   const buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
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
   const buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
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
   const buzzswarm_elem_t* e = buzzdict_get(m, &robot, buzzswarm_elem_t);
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

struct buzzswarm_members_print_s {
   FILE* stream;
   uint16_t robot;
};
typedef struct buzzswarm_members_print_s* buzzswarm_members_print_t;

void print_swarm_elem(const void* key, void* data, void* params) {
   buzzswarm_elem_t e = *(buzzswarm_elem_t*)data;
   buzzswarm_members_print_t p = (buzzswarm_members_print_t)params;
   fprintf(stdout, "   %u:%u:", p->robot, *(uint16_t*)key);
   if(!buzzdarray_isempty(e->swarms)) {
      fprintf(p->stream,
              "%u",
              buzzdarray_get(e->swarms, 0, uint16_t));
      for(uint32_t i = 1; i < buzzdarray_size(e->swarms); ++i) {
         fprintf(p->stream,
                 " %u",
                 buzzdarray_get(e->swarms, i, uint16_t));
      }
   }
   fprintf(stdout, "\n");
}

void buzzswarm_members_print(FILE* stream,
                             buzzswarm_members_t m,
                             uint16_t robot) {
   fprintf(stream,
           "ROBOT %u: swarm member table size: %u\n",
           robot,
           buzzdict_size(m));
   struct buzzswarm_members_print_s x = {
      .stream = stream,
      .robot = robot
   };
   buzzdict_foreach(m, print_swarm_elem, &x);
}

/****************************************/
/****************************************/

/* Maximum age (in steps) for swarm membership to be remembered */
static int MEMBERSHIP_AGE_MAX = 50;

/* Information on element to delete */
struct buzzswarm_members_todelete_s {
   uint16_t key;
   struct buzzswarm_members_todelete_s* next;
};
typedef struct buzzswarm_members_todelete_s* buzzswarm_members_todelete_t;

/* Function that checks whether an element must be deleted */
void check_swarm_elem(const void* key, void* data, void* params) {
   buzzswarm_elem_t elem = *(buzzswarm_elem_t*)data;
   buzzswarm_members_todelete_t* todel = (buzzswarm_members_todelete_t*)params;
   /* Increase the element's age */
   ++elem->age;
   /* Mark the element to delete if it exceeds the maximum age */
   if(elem->age > MEMBERSHIP_AGE_MAX) {
      buzzswarm_members_todelete_t x =
         (buzzswarm_members_todelete_t)malloc(
            sizeof(struct buzzswarm_members_todelete_s));
      x->key = *(uint16_t*)key;
      x->next = *todel;
      *todel = x;
   }
}

void buzzswarm_members_update(buzzswarm_members_t m) {
   /* Go through elements and make a list of the elements to erase */
   buzzswarm_members_todelete_t todel = NULL;
   buzzdict_foreach(m, check_swarm_elem, &todel);
   /* Erase elements */
   buzzswarm_members_todelete_t next;
   while(todel) {
      next = todel->next;
      buzzdict_remove(m, &todel->key);
      free(todel);
      todel = next;
   }
}

/****************************************/
/****************************************/

static int make_table(buzzvm_t vm, uint16_t id) {
   /* Create a table and add data and methods */
   buzzvm_pusht(vm);
   /* Add methods */
   function_register(others);
   function_register(join);
   function_register(leave);
   function_register(in);
   function_register(select);
   function_register(exec);
   function_register(others);
   /* Add swarm id */
   buzzvm_dup(vm);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id", 1));
   buzzvm_pushi(vm, id);
   buzzvm_tput(vm);
   /* Leave the table on the stack */
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzswarm_register(buzzvm_t vm) {
   /* Add 'swarm' table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "swarm", 1));
   buzzvm_pusht(vm);
   /* Add methods */
   function_register(create);
   function_register(id);
   /* Save 'swarm' table */
   buzzvm_gstore(vm);
   return vm->state;
}

int buzzswarm_create(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get the id */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Add a new entry if necessary */
   if(!buzzdict_exists(vm->swarms, &id)) {
      uint8_t v = 0;
      buzzdict_set(vm->swarms, &id, &v);
   }
   /* Create a table to return */
   make_table(vm, id);
   /* Return */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzswarm_id(struct buzzvm_s* vm) {
   /* Is the swarm stack empty? */
   if(buzzdarray_isempty(vm->swarmstack)) {
      /* Yes, no swarm to push, push nil instead */
      buzzvm_pushnil(vm);
   }
   else {
      /* Take the stack top by default */
      uint16_t stackpos = 1;
      /* Do we have an argument? */
      if(buzzvm_lnum(vm) > 1) {
         buzzvm_lload(vm, 1);
         buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
         stackpos = buzzvm_stack_at(vm, 1)->i.value;
      }
      /* Limit stackpos value to avoid out-of-bounds operations */
      if(stackpos > buzzdarray_size(vm->swarmstack))
         stackpos = buzzdarray_size(vm->swarmstack);
      /* Push the swarm id located at stackpos */
      buzzvm_pushi(vm,
                   buzzdarray_get(
                      vm->swarmstack,
                      buzzdarray_size(vm->swarmstack) - stackpos,
                      uint16_t));
   }
   /* Return the value */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzswarm_others(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get the id of the current swarm */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id", 1));
   buzzvm_tget(vm);
   uint16_t id1 = buzzvm_stack_at(vm, 1)->i.value;
   /* Get the swarm entry */
   const uint8_t* x = buzzdict_get(vm->swarms, &id1, uint8_t);
   if(!x) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
   }
   /* Get the id of the new swarm to create */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   uint16_t id2 = buzzvm_stack_at(vm, 1)->i.value;
   /* Add a new entry for the swarm */
   uint8_t v = *x ? 0 : 1;
   buzzdict_set(vm->swarms, &id2, &v);
   /* Send update, if necessary */
   if(v)
      buzzoutmsg_queue_append_swarm_joinleave(
         vm, BUZZMSG_SWARM_JOIN, id2);
   /* Create a table to return */
   make_table(vm, id2);
   /* Return */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzswarm_join(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Get the id */
   id_get();
   /* Join the swarm, if known */
   if(buzzdict_exists(vm->swarms, &id)) {
      /* Store membership */
      uint8_t v = 1;
      buzzdict_set(vm->swarms, &id, &v);
      /* Send update */
      buzzoutmsg_queue_append_swarm_joinleave(
         vm, BUZZMSG_SWARM_JOIN, id);
      /* Return */
      return buzzvm_ret0(vm);
   }
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
   }
}

/****************************************/
/****************************************/

int buzzswarm_leave(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Get the id */
   id_get();
   /* Leave the swarm, if known */
   if(buzzdict_exists(vm->swarms, &id)) {
      /* Store membership */
      uint8_t v = 0;
      buzzdict_set(vm->swarms, &id, &v);
      /* Send update */
      buzzoutmsg_queue_append_swarm_joinleave(
         vm, BUZZMSG_SWARM_LEAVE, id);
      /* Return */
      return buzzvm_ret0(vm);
   }
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
   }
}

/****************************************/
/****************************************/

int buzzswarm_in(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Get the id */
   id_get();
   /* Get the swarm entry */
   const uint8_t* x = buzzdict_get(vm->swarms, &id, uint8_t);
   if(!x) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
   }
   /* Push the return value */
   buzzvm_pushi(vm, *x);
   /* Return */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzswarm_select(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get the id */
   id_get();
   /* Get the result of the condition check */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   uint8_t in = buzzvm_stack_at(vm, 1)->i.value;
   /* Update the swarm, if known */
   if(buzzdict_exists(vm->swarms, &id)) {
      /* Store membership */
      buzzdict_set(vm->swarms, &id, &in);
      /* Send update */
      buzzoutmsg_queue_append_swarm_joinleave(
         vm,
         in ? BUZZMSG_SWARM_JOIN : BUZZMSG_SWARM_LEAVE,
         id);
      /* Return */
      return buzzvm_ret0(vm);
   }
   else {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
   }
}

/****************************************/
/****************************************/

int buzzswarm_exec(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get the id */
   id_get();
   /* Get the swarm entry */
   const uint8_t* x = buzzdict_get(vm->swarms, &id, uint8_t);
   if(!x) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_SWARM,
                      NULL);
      return vm->state;
   }
   /* Check whether the robot is in the swarm */
   if(*x) {
      /* Get the closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      buzzobj_t c = buzzvm_stack_at(vm, 1);
      /* Get rid of the current call structure */
      if(buzzvm_ret0(vm) != BUZZVM_STATE_READY) return vm->state;
      /* Save the current stack depth */
      uint32_t stacks = buzzdarray_size(vm->stacks);
      /* Push the current swarm in the stack */
      buzzdarray_push(vm->swarmstack, &id);
      /* Call the closure */
      buzzvm_push(vm, c);
      int32_t numargs = 0;
      buzzvm_pushi(vm, numargs);
      buzzvm_calls(vm);
      do if(buzzvm_step(vm) != BUZZVM_STATE_READY) return vm->state;
      while(stacks < buzzdarray_size(vm->stacks));
      return vm->state;
   }
   else {
      /* Get rid of the current call structure */
      return buzzvm_ret0(vm);
   }
}

/****************************************/
/****************************************/
