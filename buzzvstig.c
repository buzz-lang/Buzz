#include "buzzvstig.h"
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
