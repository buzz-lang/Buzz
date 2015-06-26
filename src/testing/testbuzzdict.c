#include <buzz/buzzdict.h>
#include <stdio.h>

void di_print_elem(const void* key, void* data, void* params) {
   int16_t k = *(const int16_t*)key;
   float d = *(float*)data;
   fprintf(stdout, "[%d] %f\n", k, d);
}

void di_print(buzzdict_t di) {
   fprintf(stdout, "size: %u\n", buzzdict_size(di));
   buzzdict_foreach(di, di_print_elem, NULL);
   fprintf(stdout, "\n");
}

uint32_t di_hash(const void* key) {
   return *(const int16_t*)key % 4;
}

int di_cmp(const void* a, const void* b) {
   int16_t k1 = *(const int16_t*)a;
   int16_t k2 = *(const int16_t*)b;
   if(k1 < k2) return -1;
   if(k1 > k2) return  1;
   return 0;
}

int main() {
   buzzdict_t di = buzzdict_new(4,
                                sizeof(int16_t),
                                sizeof(float),
                                di_hash,
                                di_cmp,
                                NULL);
   int16_t k;
   float d;
   di_print(di);
   int i;
   for(i = 0; i < 10; ++i) {
      k = i;
      d = (100.0f + k);
      fprintf(stdout, "adding (%d, %f)\n", k, d);
      buzzdict_set(di, &k, &d);
      di_print(di);
   }

   k = 5;
   d = (200.0f + k);
   fprintf(stdout, "assigning (%d, %f)\n", k, d);
   buzzdict_set(di, &k, &d);
   di_print(di);

   k = 10;
   d = (200.0f + k);
   fprintf(stdout, "assigning (%d, %f)\n", k, d);
   buzzdict_set(di, &k, &d);
   di_print(di);

   k = 4;
   fprintf(stdout, "removing (%d)\n", k);
   buzzdict_remove(di, &k);
   di_print(di);

   k = 10;
   fprintf(stdout, "removing (%d)\n", k);
   buzzdict_remove(di, &k);
   di_print(di);

   buzzdict_destroy(&di);
   return 0;
}
