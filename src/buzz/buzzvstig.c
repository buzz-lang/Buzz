#include "buzzvstig.h"
#include "buzzmsg.h"
#include "buzzvm.h"
#include <stdlib.h>
#include <stdio.h>

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
   /* Serialize the key */
   buzzobj_serialize(buf, key);
   /* Serialize the data */
   buzzobj_serialize(buf, data->data);
   /* Serialize the timestamp */
   buzzmsg_serialize_u16(buf, data->timestamp);
   /* Serialize the robot */
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

int buzzvm_vstig_create(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   buzzvm_pop(vm);
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs)
      /* Found, destroy it */
      buzzdict_remove(vm->vstigs, &id);
   /* Create a new virtual stigmergy */
   buzzvstig_t nvs = buzzvstig_new();
   buzzdict_set(vm->vstigs, &id, &nvs);
   /* Create a table and add data and methods */
   buzzobj_t t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "size"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_size));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "put"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_put));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "get"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_get));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "onconflict"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_setonconflict));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "onconflictlost"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_setonconflictlost));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_pushi(vm, id);
   buzzvm_tput(vm);
   /* Push the table on the stack */
   buzzvm_push(vm, t);
   /* Return */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzvm_vstig_put(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 2);
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_tget(vm);
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
      /* Look for the element */
      buzzvstig_elem_t* x = buzzvstig_fetch(*vs, &k);
      if(x) {
         /* Element found, update it */
         (*x)->data = v;
         ++((*x)->timestamp);
         (*x)->robot = vm->robot;
         /* Append a PUT message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, *x);
      }
      else {
         /* Element not found, create a new one */
         buzzvstig_elem_t y = buzzvstig_elem_new(v, 1, vm->robot);
         buzzvstig_store(*vs, &k, &y);
         /* Append a PUT message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, y);
      }
   }
   /* Return */
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzvm_vstig_size(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 0);
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
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

int buzzvm_vstig_get(buzzvm_t vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Get key */
   buzzvm_lload(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs) {
      /* Virtual stigmergy found */
      /* Look for key and push result */
      buzzvstig_elem_t* e = buzzvstig_fetch(*vs, &k);
      if(e) {
         /* Key found */
         buzzvm_push(vm, (*e)->data);
         /* Append the message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_QUERY, id, k, *e);
      }
      else {
         /* Key not found, add a new one containing nil */
         buzzvm_pushnil(vm);
         buzzvstig_elem_t x =
            buzzvstig_elem_new(buzzvm_stack_at(vm, 1),
                               1, vm->robot);
         /* Append the message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_QUERY, id, k, x);
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

int buzzvm_vstig_setonconflict(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
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

int buzzvm_vstig_setonconflictlost(struct buzzvm_s* vm) {
   buzzvm_lnum_assert(vm, 1);
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "id"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
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

buzzvstig_elem_t buzzvm_vstig_onconflict(buzzvm_t vm,
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
      buzzobj_t loc = buzzvm_stack_at(vm, 1);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "robot"));
      buzzvm_pushi(vm, lv->robot);
      buzzvm_tput(vm);
      buzzvm_push(vm, loc);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
      buzzvm_push(vm, lv->data);
      buzzvm_tput(vm);
      buzzvm_push(vm, loc);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "timestamp"));
      buzzvm_pushi(vm, lv->timestamp);
      buzzvm_tput(vm);
      /* Make table for remote value */
      buzzvm_pusht(vm);
      buzzobj_t rem = buzzvm_stack_at(vm, 1);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "robot"));
      buzzvm_pushi(vm, rv->robot);
      buzzvm_tput(vm);
      buzzvm_push(vm, rem);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
      buzzvm_push(vm, rv->data);
      buzzvm_tput(vm);
      buzzvm_push(vm, rem);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "timestamp"));
      buzzvm_pushi(vm, rv->timestamp);
      buzzvm_tput(vm);
      /* Call closure with 3 arguments */
      buzzvm_push(vm, loc);
      buzzvm_push(vm, rem);
      buzzvm_closure_call(vm, 3);
      /* Make new entry with return value */
      /* Make sure it's a table */
      if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_TABLE) {
         fprintf(stderr, "[WARNING] [ROBOT %u] Return value type is %d\n", vm->robot, buzzvm_stack_at(vm, 1)->o.type);
         return NULL;
      }
      /* Get the robot id */
      buzzobj_t ret = buzzvm_stack_at(vm, 1);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "robot"));
      buzzvm_tget(vm);
      if(buzzvm_stack_at(vm, 1)->o.type != BUZZTYPE_INT)
         return NULL;
      uint16_t robot = buzzvm_stack_at(vm, 1)->i.value;
      buzzvm_pop(vm);
      /* Get the data */
      buzzvm_push(vm, ret);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
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

void buzzvm_vstig_onconflictlost(buzzvm_t vm,
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
      buzzobj_t loc = buzzvm_stack_at(vm, 1);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "robot"));
      buzzvm_pushi(vm, lv->robot);
      buzzvm_tput(vm);
      buzzvm_push(vm, loc);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "data"));
      buzzvm_push(vm, lv->data);
      buzzvm_tput(vm);
      buzzvm_push(vm, loc);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "timestamp"));
      buzzvm_pushi(vm, lv->timestamp);
      buzzvm_tput(vm);
      /* Call closure with 2 arguments */
      buzzvm_push(vm, loc);
      buzzvm_closure_call(vm, 2);
   }
}

/****************************************/
/****************************************/
