#include <buzz/buzzstrman.h>
#include <stdio.h>
#include <inttypes.h>

void buzzstrman_print_id2str(const void* key,
                             void* data,
                             void* param) {
   char c = ' ';
   if(((buzzstrman_t)param)->protect > *(uint32_t*)key)
      c = '*';
   printf("\t[%c] %" PRIu32 " -> '%s'\n", c, *(uint32_t*)key, *(char**)data);
}

void buzzstrman_print_str2id(const void* key,
                             void* data,
                             void* param) {
   char c = ' ';
   if(((buzzstrman_t)param)->protect > *(uint32_t*)data)
      c = '*';
   printf("\t[%c] '%s' -> %" PRIu32 "\n", c, *(char**)key, *(uint32_t*)data);
}

void buzzstrman_print(buzzstrman_t sm) {
   printf("ID -> STRING (%" PRIu32 " elements)\n", buzzdict_size(sm->id2str));
   buzzdict_foreach(sm->id2str, buzzstrman_print_id2str, sm);
   printf("STRING -> ID (%" PRIu32 " elements)\n", buzzdict_size(sm->str2id));
   buzzdict_foreach(sm->str2id, buzzstrman_print_str2id, sm);
   printf("\n");
}

int main() {
   buzzstrman_t sm = buzzstrman_new();

   printf("=== ADDING PROTECTED STRINGS ===\n\n");
   buzzstrman_register(sm, "ciao");
   buzzstrman_register(sm, "come va?");
   buzzstrman_register(sm, "io bene");
   buzzstrman_protect(sm);
   buzzstrman_print(sm);
   
   printf("\n=== ADDING UNPROTECTED STRINGS ===\n\n");
   buzzstrman_register(sm, "la famiglia?");
   buzzstrman_register(sm, "eh, si tira avanti");
   buzzstrman_register(sm, "si, son ragazzi");
   buzzstrman_register(sm, "i figli so piezz'e core");
   buzzstrman_print(sm);

   printf("\n=== ADDING DUPLICATE STRING ===\n\n");
   buzzstrman_register(sm, "ciao");
   buzzstrman_print(sm);

   printf("\n=== GARBAGE COLLECTION ===\n\n");
   buzzstrman_gc_clear(sm);
   buzzstrman_gc_mark(sm, buzzstrman_register(sm, "la famiglia?"));
   buzzstrman_gc_mark(sm, buzzstrman_register(sm, "si, son ragazzi"));
   buzzstrman_gc_prune(sm);
   buzzstrman_print(sm);

   buzzstrman_destroy(&sm);
   return 0;
}
