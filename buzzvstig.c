#include "buzzvstig.h"
#include "buzzmsg.h"
#include <stdlib.h>

/****************************************/
/****************************************/

void buzzvstig_elem_destroy(const void* key, void* data, void* params) {
   buzzvstig_elem_t x = (buzzvstig_elem_t)data;
   // TODO: take care of string, table, and swarm
   free((void*)key);
   free(data);
}

uint32_t buzzvstig_intkeyhash(const void* key) {
   return *(int32_t*)key;
}

int buzzvstig_intkeycmp(const void* a, const void* b) {
   if(*(int32_t*)a < *(int32_t*)b) return -1;
   if(*(int32_t*)a > *(int32_t*)b) return  1;
   return 0;
}

/****************************************/
/****************************************/

buzzvstig_t buzzvstig_new() {
   return buzzdict_new(
      20,
      sizeof(int32_t),
      sizeof(struct buzzvstig_elem_s),
      buzzvstig_intkeyhash,
      buzzvstig_intkeycmp,
      buzzvstig_elem_destroy);
}

/****************************************/
/****************************************/

void buzzvstig_elem_serialize(buzzdarray_t buf,
                              int32_t key,
                              buzzvstig_elem_t data) {
   /* Serialize the key */
   buzzmsg_serialize_u32(buf, key);
   /* Serialize the data */
   buzzvar_serialize(buf, data->data);
   /* Serialize the timestamp */
   buzzmsg_serialize_u32(buf, data->timestamp);
   /* Serialize the robot */
   buzzmsg_serialize_u32(buf, data->robot);
}

/****************************************/
/****************************************/

int64_t buzzvstig_elem_deserialize(int32_t* key,
                                   buzzvstig_elem_t data,
                                   buzzdarray_t buf,
                                   uint32_t pos) {
   int64_t p = pos;
   /* Deserialize the key */
   p = buzzmsg_deserialize_u32((uint32_t*)key, buf, p);
   if(p < 0) return -1;
   /* Deserialize the data */
   p = buzzvar_deserialize(&(data->data), buf, p);
   if(p < 0) return -1;
   /* Deserialize the timestamp */
   p = buzzmsg_deserialize_u32(&(data->timestamp), buf, p);
   if(p < 0) return -1;
   /* Deserialize the robot */
   p = buzzmsg_deserialize_u32(&(data->robot), buf, p);
   if(p < 0) return -1;
   return p;
}

/****************************************/
/****************************************/
