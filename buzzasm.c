#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>

void dump(buzzvm_t vm, const char* prefix) {
   fprintf(stderr, "%s============================================================\n", prefix);
   fprintf(stderr, "%sstate: %d\terror: %d\n", prefix, vm->state, vm->error);
   fprintf(stderr, "%scode size: %u\tpc: %lld\n", prefix, vm->bcode_size, vm->pc);
   fprintf(stderr, "%sstack max: %u\tcur: %lld\n", prefix, vm->stack_size, vm->stack_top);
   for(int64_t i = vm->stack_top-1; i >= 0; --i) {
      fprintf(stderr, "%s\t%lld\t%u\t%f\n", prefix, i, vm->stack[i].i, vm->stack[i].f);
   }
   fprintf(stderr, "%s============================================================\n\n", prefix);
}

void hook(buzzvm_t vm) {
   fprintf(stdout, "Hook called!\n\n");
}

int main(int argc, char** argv) {
   /* Parse command line */
   if(argc != 2) {
      fprintf(stderr, "Usage:\n\t%s <script.basm>\n\n", argv[0]);
   }
   /* Create new VM */
   buzzvm_t vm = buzzvm_new(10, 1);
   /* Register hook function */
   buzzvm_register_function(vm, hook);
   /* Parse script */
   uint8_t* bcode_buf;
   uint32_t bcode_size;
   buzzvm_asm(vm, argv[1], &bcode_buf, &bcode_size);
   /* Set byte code */
   buzzvm_set_bcode(vm, bcode_buf, bcode_size);
   /* Run byte code and dump state */
   do dump(vm, "[DEBUG] ");
   while(buzzvm_step(vm) == BUZZVM_STATE_READY);
   if(vm->state == BUZZVM_STATE_DONE) {
      dump(vm, "[DEBUG] ");
   }
   else {
      fprintf(stderr, "%s: execution terminated abnormally\n", argv[1]);
      dump(vm, "");
   }
   /* Destroy VM*/
   free(bcode_buf);
   buzzvm_destroy(&vm);
   return 0;
}
