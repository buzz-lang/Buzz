#include "buzzasm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void dump(buzzvm_t vm) {
   int64_t i;
   fprintf(stdout, "============================================================\n");
   fprintf(stdout, "state: %d\terror: %d\n", vm->state, vm->error);
   fprintf(stdout, "code size: %u\tpc: %d\n", vm->bcode_size, vm->pc);
   fprintf(stdout, "stacks: %lld\tcur elem: %lld (size %lld)\n", buzzdarray_size(vm->stacks), buzzvm_stack_top(vm), buzzvm_stack_top(vm));
   for(i = buzzvm_stack_top(vm)-1; i >= 0; --i) {
      fprintf(stdout, "\t%lld\t", i);
      buzzobj_t o = buzzdarray_get(vm->stack, i, buzzobj_t);
      switch(o->o.type) {
         case BUZZTYPE_NIL:
            fprintf(stdout, "[nil]\n");
            break;
         case BUZZTYPE_INT:
            fprintf(stdout, "[int] %d\n", o->i.value);
            break;
         case BUZZTYPE_FLOAT:
            fprintf(stdout, "[float] %f\n", o->f.value);
            break;
         case BUZZTYPE_TABLE:
            fprintf(stdout, "[table] %d elements\n", buzzdict_size(o->t.value));
            break;
         case BUZZTYPE_ARRAY:
            fprintf(stdout, "[array] %lld\n", buzzdarray_size(o->a.value));
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative)
               fprintf(stdout, "[n-closure] %d\n", o->c.value.ref);
            else
               fprintf(stdout, "[c-closure] %d\n", o->c.value.ref);
            break;
         case BUZZTYPE_STRING:
            fprintf(stdout, "[string] %d:'%s'\n", o->s.value.sid, o->s.value.str);
            break;
         default:
            fprintf(stdout, "[TODO] type = %d\n", o->o.type);
      }
   }
   fprintf(stdout, "============================================================\n\n");
}

int hook(buzzvm_t vm) {
   fprintf(stdout, "Hook called!\n\n");
   buzzvm_pushf(vm, 17.0);
   return buzzvm_ret1(vm);
}

int main(int argc, char** argv) {
   /* Parse command line */
   if(argc != 2) {
      fprintf(stderr, "Usage:\n\t%s <file.bo>\n\n", argv[0]);
      return 0;
   }
   /* Open file */
   int fd = open(argv[1], O_RDONLY);
   if(fd < 0) perror(argv[1]);
   /* Read data */
   size_t bcode_size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   uint8_t* bcode_buf = (uint8_t*)malloc(bcode_size);
   ssize_t rd;
   size_t tot = 0;
   while(tot < bcode_size) {
      rd = read(fd, bcode_buf + tot, bcode_size - tot);
      if(rd < 0) perror(argv[1]);
      tot += rd;
   }
   close(fd);
   /* Create new VM */
   buzzvm_t vm = buzzvm_new(1);
   /* Set byte code */
   buzzvm_set_bcode(vm, bcode_buf, bcode_size);
   /* Register hook function */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "hook"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, hook));
   buzzvm_gstore(vm);
   /* Run byte code and dump state */
   do dump(vm);
   while(buzzvm_step(vm) == BUZZVM_STATE_READY);
   if(vm->state == BUZZVM_STATE_DONE) {
      dump(vm);
      fprintf(stdout, "%s: execution terminated correctly\n\n",
              argv[1]);
   }
   else {
      fprintf(stderr, "%s: execution terminated abnormally: %s\n\n",
              argv[1],
              buzzvm_error_desc[vm->error]);
   }
   /* Destroy VM */
   free(bcode_buf);
   buzzvm_destroy(&vm);
   return 0;
}
