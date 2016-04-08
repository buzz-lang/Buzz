#include <buzz/buzzasm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(const char* path, int status) {
   fprintf(stderr, "Usage:\n\t%s [--trace] <file.bo> <file.bdbg>\n\n", path);
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
   char* bcfname;
   char* dbgfname;
   int trace = 0;
   /* Parse command line */
   if(argc < 3 || argc > 4) usage(argv[0], 0);
   if(argc == 3) {
      bcfname = argv[1];
      dbgfname = argv[2];
   }
   else {
      bcfname = argv[2];
      dbgfname = argv[3];
      if(strcmp(argv[1], "--trace") != 0) {
         fprintf(stderr, "error: %s: unrecognized option '%s'\n", argv[0], argv[1]);
         usage(argv[0], 1);
      }
      trace = 1;
   }
   /* Read bytecode and fill in data structure */
   FILE* fd = fopen(bcfname, "rb");
   if(!fd) perror(bcfname);
   fseek(fd, 0, SEEK_END);
   size_t bcode_size = ftell(fd);
   rewind(fd);
   uint8_t* bcode_buf = (uint8_t*)malloc(bcode_size);
   if(fread(bcode_buf, 1, bcode_size, fd) < bcode_size) {
      perror(bcfname);
   }
   fclose(fd);
   /* Read debug information */
   buzzdebuginfo_t dbg_buf = buzzdebuginfo_new();
   if(!buzzdebuginfo_fromfile(dbg_buf, dbgfname)) {
      perror(dbgfname);
   }
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
              bcfname);
   }
   else {
      if(trace) dump(vm);
      if(buzzdebuginfo_exists(dbg_buf, &vm->pc)) {
         buzzdebuginfo_entry_t dbg = buzzdebuginfo_get(dbg_buf, &vm->pc);
         fprintf(stderr, "%s: execution terminated abnormally at %s:%llu:%llu : %s\n\n",
                 bcfname,
                 dbg->fname,
                 dbg->line,
                 dbg->col,
                 buzzvm_error_desc[vm->error]);
      }
      else {
         fprintf(stderr, "%s: execution terminated abnormally at bytecode offset %d: %s\n\n",
                 bcfname,
                 vm->pc,
                 buzzvm_error_desc[vm->error]);
      }
   }
   /* Destroy VM */
   free(bcode_buf);
   buzzdebuginfo_destroy(&dbg_buf);
   buzzvm_destroy(&vm);
   return 0;
}
