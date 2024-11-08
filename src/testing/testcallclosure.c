#include <buzz/buzzvm.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

int main() {
   /* Get bytecode */
   FILE* fd = fopen("testclosure.bo", "rb");
   if(!fd) {
      perror("testclosure.bo");
      return 1;
   }
   fseek(fd, 0, SEEK_END);
   size_t bcode_size = ftell(fd);
   rewind(fd);
   uint8_t* bcode_buf = (uint8_t*)malloc(bcode_size);
   if(fread(bcode_buf, 1, bcode_size, fd) < bcode_size) {
      perror("testclosure.bo");
      return 1;
   }
   fclose(fd);
   /* Create VM */
   buzzvm_t vm = buzzvm_new(1);
   /* Set bytecode */
   buzzvm_set_bcode(vm, bcode_buf, bcode_size);
   puts("buzzvm_set_bcode\n");
   /* Call closure */
   buzzvm_pushi(vm, 3);
   puts("x -> 3\n");
   buzzvm_pushi(vm, 2);
   puts("y -> 2\n");
   buzzvm_function_call(vm, "g", 2);
   puts("buzzvm_closure\n");
   /* Check value at the top of the stack */
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   if(o->o.type != BUZZTYPE_INT) {
      fprintf(stderr, "Expected int return value, found %" PRIu16 "\n", o->o.type);
      return 1;
   }
   fprintf(stdout, "returned %" PRIi32 "\n", o->i.value);
   /* All done */
   free(bcode_buf);
   buzzvm_destroy(&vm);
   return 0;
}
