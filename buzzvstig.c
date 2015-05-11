#include "buzzvstig.h"
#include "buzzmsg.h"
#include <stdlib.h>

/****************************************/
/****************************************/

void buzzvstig_elem_destroy(const void* key, void* data, void* params) {
   buzzvstig_elem_t* x = (buzzvstig_elem_t*)data;
   // TODO: take care of string, table, swarm, and closure
   free((void*)key);
   free(data);
}

/****************************************/
/****************************************/

buzzvstig_t buzzvstig_new() {
   return buzzdict_new(
      20,
      sizeof(int32_t),
      sizeof(buzzvstig_elem_t),
      buzzdict_int32keyhash,
      buzzdict_int32keycmp,
      buzzvstig_elem_destroy);
}

/****************************************/
/****************************************/

void buzzvstig_elem_serialize(buzzdarray_t buf,
                              int32_t key,
                              const buzzvstig_elem_t* data) {
   /* Serialize the key */
   buzzmsg_serialize_u32(buf, key);
   /* Serialize the data */
   buzzobj_serialize(buf, data->data);
   /* Serialize the timestamp */
   buzzmsg_serialize_u32(buf, data->timestamp);
   /* Serialize the robot */
   buzzmsg_serialize_u32(buf, data->robot);
}

/****************************************/
/****************************************/

int64_t buzzvstig_elem_deserialize(int32_t* key,
                                   buzzvstig_elem_t* data,
                                   buzzdarray_t buf,
                                   uint32_t pos) {
   int64_t p = pos;
   /* Deserialize the key */
   p = buzzmsg_deserialize_u32((uint32_t*)key, buf, p);
   if(p < 0) return -1;
   /* Deserialize the data */
   p = buzzobj_deserialize(data->data, buf, p);
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
