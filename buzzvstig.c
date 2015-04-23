#include "buzzvstig.h"
#include <stdlib.h>

/****************************************/
/****************************************/

void buzzvstig_elem_destroy(const void* key, void* data, void* params) {
   buzzvstig_elem_t* x = (buzzvstig_elem_t*)data;
   // TODO: take care of string, table, and swarm
   free(*x);
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
      sizeof(buzzvstig_elem_t),
      buzzvstig_intkeyhash,
      buzzvstig_intkeycmp);
}

/****************************************/
/****************************************/

void buzzvstig_destroy(buzzvstig_t* vs) {
   buzzdict_foreach(*vs, buzzvstig_elem_destroy, NULL);
   buzzdict_destroy(vs);
}

/****************************************/
/****************************************/

buzzvstig_elem_t buzzvstig_newelem(buzzvar_t data,
                                   uint32_t timestamp,
                                   uint32_t robot) {
   buzzvstig_elem_t x = (buzzvstig_elem_t)malloc(sizeof(struct buzzvstig_elem_s));
   x->data = data;
   x->timestamp = timestamp;
   x->robot = robot;
   return x;
}

/****************************************/
/****************************************/

buzzvstig_elem_t buzzvstig_fetch(buzzvstig_t vs,
                                 int32_t key) {
   buzzvstig_elem_t* x = buzzdict_get(vs, &key, buzzvstig_elem_t);
   return x ? *x : NULL;
}

/****************************************/
/****************************************/
