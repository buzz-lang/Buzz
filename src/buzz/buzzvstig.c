#include "buzzvstig.h"
#include "buzzmsg.h"
#include "buzzvm.h"
#include <stdlib.h>
#include <stdio.h>

/****************************************/
/****************************************/

#define function_register(FNAME)                                        \
   buzzvm_dup(vm);                                                      \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));             \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvstig_ ## FNAME)); \
   buzzvm_tput(vm);

#define id_get()                                          \
   buzzvm_lload(vm, 0);                                   \
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id", 1)); \
   buzzvm_tget(vm);                                       \
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;

#define add_field(FIELD, VAL, METHOD)                       \
   buzzvm_dup(vm);                                          \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FIELD, 1)); \
   buzzvm_ ## METHOD(vm, VAL->FIELD);                       \
   buzzvm_tput(vm);

/****************************************/
/****************************************/

int buzzvstig_register(struct buzzvm_s* vm) {
   /* Push 'stigmergy' table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "stigmergy", 1));
   buzzvm_pusht(vm);
   /* Add 'create' function */
   buzzvm_dup(vm);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "create", 1));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvstig_create));
   buzzvm_tput(vm);
   /* Register the 'stigmergy' table */
   buzzvm_gstore(vm);
   return vm->state;
}

/****************************************/
/****************************************/

uint32_t buzzvstig_key_hash(const void* key) {
   return buzzobj_hash(*(buzzobj_t*)key);
}

int buzzvstig_key_cmp(const void* a, const void* b) {
   return buzzobj_cmp(*(buzzobj_t*)a, *(buzzobj_t*)b);
}

/****************************************/
/****************************************/

buzzvstig_elem_t buzzvstig_elem_new(buzzobj_t data,
                                    uint16_t timestamp,
                                    uint16_t robot) {
   buzzvstig_elem_t e = (buzzvstig_elem_t)malloc(sizeof(struct buzzvstig_elem_s));
   e->data = data;
   e->timestamp = timestamp;
   e->robot = robot;
   return e;
}

/****************************************/
/****************************************/

buzzvstig_elem_t buzzvstig_elem_clone(const buzzvstig_elem_t e) {
   buzzvstig_elem_t x = (buzzvstig_elem_t)malloc(sizeof(struct buzzvstig_elem_s));
   x->data      = buzzobj_clone(e->data);
   x->timestamp = e->timestamp;
   x->robot     = e->robot;
   return x;
}

/****************************************/
/****************************************/

void buzzvstig_elem_destroy(const void* key, void* data, void* params) {
   free((void*)key);
   free(*(buzzvstig_elem_t*)data);
   free(data);
}

/****************************************/
/****************************************/

buzzvstig_t buzzvstig_new() {
   buzzvstig_t x = (buzzvstig_t)malloc(sizeof(struct buzzvstig_s));
   x->data = buzzdict_new(
      10,
      sizeof(buzzobj_t),
      sizeof(buzzvstig_elem_t),
      buzzvstig_key_hash,
      buzzvstig_key_cmp,
      buzzvstig_elem_destroy);
   x->onconflict = NULL;
   x->onconflictlost = NULL;
   return x;
}

/****************************************/
/****************************************/

void buzzvstig_destroy(buzzvstig_t* vs) {
   buzzdict_destroy(&((*vs)->data));
   free((*vs)->onconflict);
   free((*vs)->onconflictlost);
   free(*vs);
}

/****************************************/
/****************************************/

void buzzvstig_elem_serialize(buzzmsg_payload_t buf,
                              const buzzobj_t key,
                              const buzzvstig_elem_t data) {
   buzzobj_serialize    (buf, key);
   buzzobj_serialize    (buf, data->data);
   buzzmsg_serialize_u16(buf, data->timestamp);
   buzzmsg_serialize_u16(buf, data->robot);
}

/****************************************/
/****************************************/

int64_t buzzvstig_elem_deserialize(buzzobj_t* key,
                                   buzzvstig_elem_t* data,
                                   buzzmsg_payload_t buf,
                                   uint32_t pos,
                                   struct buzzvm_s* vm) {
   /* Initialize the position */
   int64_t p = pos;
   /* Create a new vstig entry */
   /* Deserialize the key */
   p = buzzobj_deserialize(key, buf, p, vm);
   if(p < 0) return -1;
   /* Deserialize the data */
   p = buzzobj_deserialize(&((*data)->data), buf, p, vm);
   if(p < 0) return -1;
   /* Deserialize the timestamp */
   p = buzzmsg_deserialize_u16(&((*data)->timestamp), buf, p);
   if(p < 0) return -1;
   /* Deserialize the robot */
   p = buzzmsg_deserialize_u16(&((*data)->robot), buf, p);
   if(p < 0) return -1;
   return p;
}

/****************************************/
/****************************************/

int buzzvstig_create(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   buzzvm_pop(vm);
   /* Look for virtual stigmergy */
   const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Found, destroy it */
      buzzdict_remove(vm->vstigs, &id);
   }
   /* Create a new virtual stigmergy */
   buzzvstig_t nvs = buzzvstig_new();
   buzzdict_set(vm->vstigs, &id, &nvs);
   /* Create a table */
   buzzvm_pusht(vm);
   /* Add data and methods */
   buzzvm_dup(vm);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id", 1));
   buzzvm_pushi(vm, id);
   buzzvm_tput(vm);
   function_register(size);
   function_register(put);
   function_register(get);
   function_register(onconflict);
   function_register(onconflictlost);
   /* Return the table */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzvstig_put(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get vstig id */
   id_get();
   /* Get key */
   buzzvm_lload(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   /* Get value */
   buzzvm_lload(vm, 2);
   buzzobj_t v = buzzvm_stack_at(vm, 1);
   /* Look for virtual stigmergy */
   const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Look for the element */
      const buzzvstig_elem_t* x = buzzvstig_fetch(*vs, &k);
      if(x) {
         /* Element found, update it */
         (*x)->data = v;
         ++((*x)->timestamp);
         (*x)->robot = vm->robot;
         /* Append a PUT message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, *x);
      }
      else {
         /* Element not found, create a new one */
         buzzvstig_elem_t y = buzzvstig_elem_new(v, 1, vm->robot);
         buzzvstig_store(*vs, &k, &y);
         /* Append a PUT message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_PUT, id, k, y);
      }
   }
   /* Return */
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzvstig_size(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Get vstig id */
   id_get();
   /* Look for virtual stigmergy */
   const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Virtual stigmergy found, return its size */
      buzzvm_pushi(vm, buzzdict_size((*vs)->data));
   }
   else {
      /* Virtual stigmergy not found, return 0 */
      buzzvm_pushi(vm, 0);
   }
   /* Return */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzvstig_get(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   id_get();
   /* Get key */
   buzzvm_lload(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   /* Look for virtual stigmergy */
   const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Virtual stigmergy found */
      /* Look for key and push result */
      const buzzvstig_elem_t* e = buzzvstig_fetch(*vs, &k);
      if(e) {
         /* Key found */
         buzzvm_push(vm, (*e)->data);
         /* Append the message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_QUERY, id, k, *e);
      }
      else {
         /* Key not found, add a new one containing nil */
         buzzvm_pushnil(vm);
         buzzvstig_elem_t x =
            buzzvstig_elem_new(buzzvm_stack_at(vm, 1),
                               1, vm->robot);
         /* Append the message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm, BUZZMSG_VSTIG_QUERY, id, k, x);
      }
   }
   else {
      /* No virtual stigmergy found, just push false */
      /* If this happens, its a bug */
      buzzvm_pushnil(vm);
      fprintf(stderr, "[BUG] [ROBOT %u] Can't find virtual stigmergy %u\n", vm->robot, id);
   }
   /* Return the value found */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzvstig_onconflict(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   id_get();
   /* Look for virtual stigmergy */
   const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Virtual stigmergy found */
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      /* Clone the closure */
      if((*vs)->onconflict) free((*vs)->onconflict);
      (*vs)->onconflict = buzzobj_clone(buzzvm_stack_at(vm, 1));
   }
   else {
      /* No virtual stigmergy found, just push false */
      /* If this happens, its a bug */
      buzzvm_pushnil(vm);
      fprintf(stderr, "[BUG] [ROBOT %u] Can't find virtual stigmergy %u\n", vm->robot, id);
   }
   /* Return the value found */
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzvstig_onconflictlost(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   id_get();
   /* Look for virtual stigmergy */
   const buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Virtual stigmergy found */
      /* Get closure */
      buzzvm_lload(vm, 1);
      buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
      /* Clone the closure */
      if((*vs)->onconflictlost) free((*vs)->onconflictlost);
      (*vs)->onconflictlost = buzzobj_clone(buzzvm_stack_at(vm, 1));
   }
   else {
      /* No virtual stigmergy found, just push false */
      /* If this happens, its a bug */
      buzzvm_pushnil(vm);
      fprintf(stderr, "[BUG] [ROBOT %u] Can't find virtual stigmergy %u\n", vm->robot, id);
   }
   /* Return the value found */
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

buzzvstig_elem_t buzzvstig_onconflict_call(buzzvm_t vm,
                                           buzzvstig_t vs,
                                           buzzobj_t k,
                                           buzzvstig_elem_t lv,
                                           buzzvstig_elem_t rv) {
   /* Was a conflict manager defined? */
   if(vs->onconflict) {
      /* Push closure */
      buzzvm_push(vm, vs->onconflict);
      /* Push key */
      buzzvm_push(vm, k);
      /* Make table for local value */
      buzzvm_pusht(vm);
      add_field(robot, lv, pushi);
      add_field(data, lv, push);
      add_field(timestamp, lv, pushi);
      /* Make table for remote value */
      buzzvm_pusht(vm);
      add_field(robot, rv, pushi);
      add_field(data, rv, push);
      add_field(timestamp, rv, pushi);
      /* Call closure (key, lv, rv on the stack) */
      buzzvm_closure_call(vm, 3);
      /* Make new entry with return value */
      /* Make sure it's a table */
      if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_TABLE) {
         fprintf(stderr, "[WARNING] [ROBOT %u] Return value type is %d\n", vm->robot, buzzvm_stack_at(vm, 1)->o.type);
         return NULL;
      }
      /* Get the robot id */
      buzzobj_t ret = buzzvm_stack_at(vm, 1);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "robot", 1));
      buzzvm_tget(vm);
      if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_INT)
         return NULL;
      uint16_t robot = buzzvm_stack_at(vm, 1)->i.value;
      buzzvm_pop(vm);
      /* Get the data */
      buzzvm_push(vm, ret);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data", 1));
      buzzvm_tget(vm);
      buzzobj_t data = buzzvm_stack_at(vm, 1);
      buzzvm_pop(vm);
      /* Make new entry */
      return buzzvstig_elem_new(data, lv->timestamp, robot);
   }
   else {
      /* No conflict manager, use default behavior */
      if(lv->robot > rv->robot) return buzzvstig_elem_clone(lv);
      else return buzzvstig_elem_clone(rv);
   }
}

/****************************************/
/****************************************/

void buzzvstig_onconflictlost_call(buzzvm_t vm,
                                   buzzvstig_t vs,
                                   buzzobj_t k,
                                   buzzvstig_elem_t lv) {
   /* Was a conflict manager defined? */
   if(vs->onconflictlost) {
      /* Push closure */
      buzzvm_push(vm, vs->onconflictlost);
      /* Push key */
      buzzvm_push(vm, k);
      /* Make table for local value */
      buzzvm_pusht(vm);
      add_field(robot, lv, pushi);
      add_field(data, lv, push);
      add_field(timestamp, lv, pushi);
      /* Call closure (key and table are on the stack) */
      buzzvm_closure_call(vm, 2);
   }
}

/****************************************/
/****************************************/
