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
                                    uint32_t robot) {
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
   free(*(buzzvstig_elem_t*)data);
}

/****************************************/
/****************************************/

buzzvstig_t buzzvstig_new() {
   return buzzdict_new(
      20,
      sizeof(buzzobj_t),
      sizeof(buzzvstig_elem_t),
      buzzvstig_key_hash,
      buzzvstig_key_cmp,
      buzzvstig_elem_destroy);
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
   buzzmsg_serialize_u32(buf, data->robot);
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
   *data = (buzzvstig_elem_t)malloc(sizeof(struct buzzvstig_elem_s));
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
   p = buzzmsg_deserialize_u32(&((*data)->robot), buf, p);
   if(p < 0) return -1;
   return p;
}

/****************************************/
/****************************************/

int buzzvm_vstig_create(buzzvm_t vm) {
   /* Get vstig id */
   buzzvm_lload(vm, 1);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   buzzvm_pop(vm);
   /* Look for virtual stigmergy */
   buzzvstig_t* vs = buzzdict_get(vm->vstigs, &id, buzzvstig_t);
   if(vs)
      /* Found, destroy it */
      buzzvstig_destroy(vs);
   /* Create a new virtual stigmergy */
   buzzvstig_t nvs = buzzvstig_new();
   buzzdict_set(vm->vstigs, &id, &nvs);
   /* Create a table and add data and methods */
   buzzobj_t t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "put"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_put));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "get"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzvm_vstig_get));
   buzzvm_tput(vm);
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "vstig"));
   buzzvm_pushi(vm, id);
   buzzvm_tput(vm);
   /* Push the table on the stack */
   buzzvm_push(vm, t);
   /* Return */
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/

int buzzvm_vstig_put(buzzvm_t vm) {
   /* Get vstig id */
   buzzvm_lload(vm, 0);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "vstig"));
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
         buzzvstig_store(*vs, &k, x);
         /* Append a PUT message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, *x);
         fprintf(stderr, "[DEBUG] robot %d sends <PUT, d=%d, ts=%d, r=%d>\n",
                 vm->robot, k->i.value, (*x)->timestamp, (*x)->robot);
      }
      else {
         /* Element not found, create a new one */
         buzzvstig_elem_t y = buzzvstig_elem_new(v, 1, vm->robot);
         buzzvstig_store(*vs, &k, &y);
         /* Append a PUT message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_PUT, id, k, y);
         fprintf(stderr, "[DEBUG] robot %d sends <PUT, d=%d, ts=%d, r=%d>\n",
                 vm->robot, k->i.value, y->timestamp, y->robot);
      }
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
   buzzvm_pushs(vm, buzzvm_string_register(vm, "vstig"));
   buzzvm_tget(vm);
   uint16_t id = buzzvm_stack_at(vm, 1)->i.value;
   /* Get key */
   buzzvm_lload(vm, 1);
   buzzobj_t k = buzzvm_stack_at(vm, 1);
   /* Look for virtual stigmergy */
   buzzdict_t* vs = buzzdict_get(vm->vstigs, &id, buzzdict_t);
   if(vs) {
      /* Virtual stigmergy found */
      /* Look for key and push result */
      buzzvstig_elem_t* e = buzzvstig_fetch(*vs, &k);
      if(e) {
         /* Key found */
         buzzvm_push(vm, (*e)->data);
         fprintf(stderr, "[DEBUG] robot %d sends <QUERY, d=%d, ts=%d, r=%d>\n",
                 vm->robot, k->i.value, (*e)->timestamp, (*e)->robot);
         /* Append the message to the out message queue */
         buzzoutmsg_queue_append_vstig(vm->outmsgs, BUZZMSG_VSTIG_QUERY, id, k, *e);
      }
      else {
         /* Key not found, add a new one containing nil */
         buzzvm_pushnil(vm);
         buzzvstig_elem_t x =
            buzzvstig_elem_new(buzzvm_stack_at(vm, 1),
                               1, vm->robot);
         fprintf(stderr, "[DEBUG] robot %d sends <QUERY, d=%d, ts=%d, r=%d>\n",
                 vm->robot, k->i.value, x->timestamp, x->robot);
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
   buzzvm_ret1(vm);
   return BUZZVM_STATE_READY;
}

/****************************************/
/****************************************/
