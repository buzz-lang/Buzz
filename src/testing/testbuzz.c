#include <buzz/buzzasm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void usage(const char* path, int status) {
   fprintf(stderr, "Usage:\n\t%s [--trace] <file.bo>\n\n", path);
   exit(status);
}

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

int print(buzzvm_t vm) {
   for(int i = 1; i < buzzdarray_size(vm->lsyms->syms); ++i) {
      buzzvm_lload(vm, i);
      buzzobj_t o = buzzvm_stack_at(vm, 1);
      buzzvm_pop(vm);
      switch(o->o.type) {
         case BUZZTYPE_NIL:
            fprintf(stdout, "[nil]");
            break;
         case BUZZTYPE_INT:
            fprintf(stdout, "%d", o->i.value);
            break;
         case BUZZTYPE_FLOAT:
            fprintf(stdout, "%f", o->f.value);
            break;
         case BUZZTYPE_TABLE:
            fprintf(stdout, "[table with %d elems]", (buzzdict_size(o->t.value)));
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative)
               fprintf(stdout, "[n-closure @%d]", o->c.value.ref);
            else
               fprintf(stdout, "[c-closure @%d]", o->c.value.ref);
            break;
         case BUZZTYPE_STRING:
            fprintf(stdout, "%s", o->s.value.str);
            break;
         case BUZZTYPE_USERDATA:
            fprintf(stdout, "[userdata @%p]", o->u.value);
            break;
         default:
            break;
      }
   }
   fprintf(stdout, "\n");
   return buzzvm_ret0(vm);
}

int main(int argc, char** argv) {
   char* fname;
   int trace = 0;
   /* Parse command line */
   if(argc < 2 || argc > 3) usage(argv[0], 0);
   if(argc == 2) fname = argv[1];
   else {
      fname = argv[2];
      if(strcmp(argv[1], "--trace") != 0) {
         fprintf(stderr, "error: %s: unrecognized option '%s'\n", argv[0], argv[1]);
         usage(argv[0], 1);
      }
      trace = 1;
   }
   /* Open file */
   int fd = open(fname, O_RDONLY);
   if(fd < 0) perror(fname);
   /* Read data */
   size_t bcode_size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   uint8_t* bcode_buf = (uint8_t*)malloc(bcode_size);
   ssize_t rd;
   size_t tot = 0;
   while(tot < bcode_size) {
      rd = read(fd, bcode_buf + tot, bcode_size - tot);
      if(rd < 0) perror(fname);
      tot += rd;
   }
   close(fd);
   /* Create new VM */
   buzzvm_t vm = buzzvm_new(1);
   /* Set byte code */
   buzzvm_set_bcode(vm, bcode_buf, bcode_size);
   /* Register hook functions */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "print"));
   buzzvm_pushcc(vm, buzzvm_function_register(vm, print));
   buzzvm_gstore(vm);
   /* Run byte code and dump state */
   do if(trace) dump(vm);
   while(buzzvm_step(vm) == BUZZVM_STATE_READY);
   if(vm->state == BUZZVM_STATE_DONE) {
      if(trace) dump(vm);
      fprintf(stdout, "%s: execution terminated correctly\n\n",
              fname);
   }
   else {
      if(trace) dump(vm);
      fprintf(stderr, "%s: execution terminated abnormally: %s\n\n",
              fname,
              buzzvm_error_desc[vm->error]);
   }
   /* Destroy VM */
   free(bcode_buf);
   buzzvm_destroy(&vm);
   return 0;
}
