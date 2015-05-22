#include "buzzvstig.h"
#include "buzzmsg.h"
#include <stdlib.h>

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
