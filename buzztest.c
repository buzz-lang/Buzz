#include "buzzasm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dump(buzzvm_t vm, const char* prefix) {
   fprintf(stderr, "%s============================================================\n", prefix);
   fprintf(stderr, "%sstate: %d\terror: %d\n", prefix, vm->state, vm->error);
   fprintf(stderr, "%scode size: %u\tpc: %d\n", prefix, vm->bcode_size, vm->pc);
   fprintf(stderr, "%sstack max: %lld\tcur: %lld\n", prefix, buzzvm_stack_top(vm), buzzvm_stack_top(vm));
   for(int64_t i = buzzvm_stack_top(vm)-1; i >= 0; --i) {
      fprintf(stderr, "%s\t%lld\t", prefix, i);
      buzzobj_t o = buzzdarray_get(vm->stack, i, buzzobj_t);
      switch(o->type) {
         case BUZZTYPE_NIL:
            fprintf(stderr, "[nil]\n");
            break;
         case BUZZTYPE_INT:
            fprintf(stderr, "[int] %d\n", o->i.value);
            break;
         case BUZZTYPE_FLOAT:
            fprintf(stderr, "[float] %f\n", o->f.value);
            break;
         default:
            fprintf(stderr, "[TODO]\n");
      }
   }
   fprintf(stderr, "%s============================================================\n\n", prefix);
}

int hook(buzzvm_t vm) {
   fprintf(stdout, "Hook called!\n\n");
   return 0;
}

int main(int argc, char** argv) {
   /* Parse command line */
   if(argc != 2) {
      fprintf(stderr, "Usage:\n\t%s <script.basm>\n\n", argv[0]);
      return 0;
   }
   /* Parse script */
   uint8_t* bcode_buf;
   uint32_t bcode_size;
   if(buzz_asm(argv[1], &bcode_buf, &bcode_size) != 0) {
      return 1;
   }
   /* Test deassembler */
   char fdeasm[100];
   strcpy(fdeasm, argv[1]);
   strcat(fdeasm, ".tmp");
   buzz_deasm(bcode_buf, bcode_size, fdeasm);
   /* Create new VM */
   buzzvm_t vm = buzzvm_new(1);
   /* Register hook function */
   buzzvm_register_function(vm, hook);
   /* Set byte code */
   buzzvm_set_bcode(vm, bcode_buf, bcode_size);
   /* Run byte code and dump state */
   do dump(vm, "[DEBUG] ");
   while(buzzvm_step(vm) == BUZZVM_STATE_READY);
   if(vm->state == BUZZVM_STATE_DONE) {
      dump(vm, "[DEBUG] ");
   }
   else {
      fprintf(stderr, "%s: execution terminated abnormally: %s\n",
              argv[1],
              buzzvm_error_desc[vm->error]);
      dump(vm, "");
   }
   /* Destroy VM*/
   free(bcode_buf);
   buzzvm_destroy(&vm);
   return 0;
}
