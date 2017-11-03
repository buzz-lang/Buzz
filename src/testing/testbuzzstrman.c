#include <buzz/buzzstrman.h>
#include <stdio.h>
#include <inttypes.h>

int main() {
   buzzstrman_t sm = buzzstrman_new();

   printf("=== ADDING PROTECTED STRINGS ===\n\n");
   buzzstrman_register(sm, "ciao", 1);
   buzzstrman_register(sm, "come va?", 1);
   buzzstrman_register(sm, "io bene", 1);
   buzzstrman_print(sm);
   
   printf("\n=== ADDING UNPROTECTED STRINGS ===\n\n");
   buzzstrman_register(sm, "la famiglia?", 0);
   buzzstrman_register(sm, "eh, si tira avanti", 0);
   buzzstrman_register(sm, "si, son ragazzi", 0);
   buzzstrman_register(sm, "i figli so piezz'e core", 0);
   buzzstrman_print(sm);

   printf("\n=== ADDING DUPLICATE STRING ===\n\n");
   buzzstrman_register(sm, "ciao", 1);
   buzzstrman_register(sm, "eh, si tira avanti", 0);
   buzzstrman_print(sm);

   printf("\n=== GARBAGE COLLECTION ===\n\n");
   buzzstrman_gc_clear(sm);
   buzzstrman_gc_mark(sm, buzzstrman_register(sm, "la famiglia?", 0));
   buzzstrman_gc_mark(sm, buzzstrman_register(sm, "si, son ragazzi", 0));
   buzzstrman_gc_prune(sm);
   buzzstrman_print(sm);

   printf("\n=== HIGH ID ===\n\n");
   for( int i = 0 ; i < 60000 ; i++ ) {
     buzzstrman_register(sm, "la vita difficile", 0);
     buzzstrman_gc_clear(sm);
     buzzstrman_gc_prune(sm);
   }
   buzzstrman_register(sm, "mondo durissimo", 0);
   buzzstrman_print(sm);

   printf("\n=== ID OVERLOAD ===\n\n");
   for( int i = 0 ; i < 6000 ; i++ ) {
     buzzstrman_register(sm, "la vita difficile", 0);
     buzzstrman_gc_clear(sm);
     buzzstrman_gc_prune(sm);
   }
   buzzstrman_register(sm, "mondo durissimo", 0);
   buzzstrman_print(sm);
   
   buzzstrman_destroy(&sm);
   return 0;
}
