#include "buzzdebug.h"
#include "buzzasm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/****************************************/
/****************************************/

buzzdebug_entry_t buzzdebug_entry_new(uint64_t l,
                                      uint64_t c,
                                      const char* fn) {
   buzzdebug_entry_t x = malloc(sizeof(struct buzzdebug_entry_s));
   x->line = l;
   x->col = c;
   x->fname = fn;
   return x;
}

void buzzdebug_off2script_destroyf(const void* key, void* data, void* params) {
   free((void*)key);
   free(*(buzzdebug_entry_t*)data);
   free(data);
}

void buzzdebug_script2off_destroyf(const void* key, void* data, void* params) {
   free(*(buzzdebug_entry_t*)key);
   free((void*)key);
   free(data);
}

uint32_t buzzdebug_entryhash(const void* key) {
   buzzdebug_entry_t x = *(const buzzdebug_entry_t*)key;
   char* str;
   uint32_t h;
   asprintf(&str, "%s:%" PRIu64 ":%" PRIu64, x->fname, x->line, x->col);
   h = buzzdict_strkeyhash(&str);
   free(str);
   return h;
}

int buzzdebug_entrycmp(const void* a, const void* b) {
   buzzdebug_entry_t x = *(const buzzdebug_entry_t*)a;
   buzzdebug_entry_t y = *(const buzzdebug_entry_t*)b;
   int cmp = strcmp(x->fname, y->fname);
   if(cmp == 0) {
      if(x->line < y->line) return -1;
      if(x->line > y->line) return 1;
      if(x->col < y->col) return -1;
      if(x->col > y->col) return 1;
      return 0;
   }
   else
      return cmp;
}

/****************************************/
/****************************************/

int buzzdebug_fnames_cmp(const void* a, const void* b) {
   const char* d1 = *(const char**)a;
   const char* d2 = *(const char**)b;
   return strcmp(d1, d2);
}

void buzzdebug_fnames_destroy(void* data, void* params) {
   free(*(char**)data);
}

/****************************************/
/****************************************/

buzzdebug_t buzzdebug_new() {
   /* Make room for the debug information structure */
   buzzdebug_t x = (buzzdebug_t)malloc(sizeof(struct buzzdebug_s));
   /* Create new file name set  */
   x->fnames = buzzset_new(sizeof(char*),
                           buzzdebug_fnames_cmp,
                           buzzdebug_fnames_destroy);
   /* Make hash map for offset -> script data */
   x->off2script = buzzdict_new(100,
                                sizeof(int32_t),
                                sizeof(buzzdebug_entry_t),
                                buzzdict_int32keyhash,
                                buzzdict_int32keycmp,
                                buzzdebug_off2script_destroyf);
   /* Make hash map for script -> offset data */
   x->script2off = buzzdict_new(100,
                                sizeof(buzzdebug_entry_t),
                                sizeof(int32_t),
                                buzzdebug_entryhash,
                                buzzdebug_entrycmp,
                                buzzdebug_script2off_destroyf);
   /* Make list of breakpoints */
   x->breakpoints = buzzdarray_new(5, sizeof(int32_t), NULL);
   return x;
}

/****************************************/
/****************************************/

void buzzdebug_destroy(buzzdebug_t* dbg) {
   buzzdarray_destroy(&(*dbg)->breakpoints);
   buzzdict_destroy(&(*dbg)->off2script);
   buzzdict_destroy(&(*dbg)->script2off);
   buzzset_destroy(&(*dbg)->fnames);
   free(*dbg);
   *dbg = NULL;
}

/****************************************/
/****************************************/

int buzzdebug_fromfile(buzzdebug_t dbg,
                       const char* fname) {
   /* Open file */
   FILE* fd = fopen(fname, "rb");
   if(!fd) return 0;
   /* Keep reading until you reach the end of the data or an error */
   uint32_t offset;
   uint64_t line, col;
   uint16_t srcfnlen, srcfnlenmax = 512;
   char* srcfname = (char*)malloc(srcfnlenmax);
   /* char fepmark; */
   while(!feof(fd) && !ferror(fd)) {
      /* Read fields */
      if(fread(&offset,   sizeof(offset),   1, fd) == 1 &&
         fread(&line,     sizeof(line),     1, fd) == 1 &&
         fread(&col,      sizeof(col),      1, fd) == 1 &&
         fread(&srcfnlen, sizeof(srcfnlen), 1, fd) == 1) {
         /* Read source file name, resizing buffer if necessary */
         if(srcfnlen > srcfnlenmax) {
            srcfnlenmax = srcfnlen + 1;
            srcfname = realloc(srcfname, srcfnlenmax);
         }
         if(fread(srcfname, 1, srcfnlen, fd) == srcfnlen) {
            srcfname[srcfnlen] = 0;
            buzzdebug_info_set(dbg, offset, line, col, srcfname);
         }
         /* Read fields */
         /* if(fread(&fepmark, sizeof(fepmark), 1, fd) == 1 && */
         /*    fepmark > 0) { */
         /* } */
      }
   }
   int err = ferror(fd);
   free(srcfname);
   fclose(fd);
   return err ? 0 : 1;
}

/****************************************/
/****************************************/

struct buzzdebug_entry_dump_info_s {
   FILE* fd;
   int ok; /* 1 if no error, 0 otherwise */
};

void buzzdebug_entry_dump(const void* key, void* data, void* params) {
   /* Get data */
   struct buzzdebug_entry_dump_info_s* i = (struct buzzdebug_entry_dump_info_s*)params;
   buzzdebug_entry_t e = *(buzzdebug_entry_t*)data;
   /* Make sure no error occurred */
   if(i->ok) {
      /* Write data */
      uint16_t strfnlen = strlen(e->fname);
      if(fwrite(key,       sizeof(uint32_t), 1, i->fd) < 1 ||
         fwrite(&e->line,  sizeof(uint64_t), 1, i->fd) < 1 ||
         fwrite(&e->col,   sizeof(uint64_t), 1, i->fd) < 1 ||
         fwrite(&strfnlen, sizeof(uint16_t), 1, i->fd) < 1 ||
         fwrite(e->fname,  1, strfnlen, i->fd) < strfnlen)
         i->ok = 0;
   }
}

int buzzdebug_tofile(const char* fname,
                     buzzdebug_t dbg) {
   /* Open file */
   FILE* fd = fopen(fname, "wb");
   if(!fd) return 0;
   /* Write entries */
   struct buzzdebug_entry_dump_info_s i = {
      .fd = fd,
      .ok = 1
   };
   buzzdict_foreach(dbg->off2script, buzzdebug_entry_dump, &i);
   /* Close file */
   fclose(fd);
   return i.ok ? 0 : 1;
}

/****************************************/
/****************************************/

void buzzdebug_info_set(buzzdebug_t dbg,
                        int32_t offset,
                        uint64_t line,
                        uint64_t col,
                        const char* fname) {
   /* Save filename */
   /* Check whether it's already in the set */
   const char* fn;
   const char** test = buzzset_fetch(dbg->fnames, &fname, char*);
   if(test) {
      /* File name found in the set */
      fn = *test;
   }
   else {
      /* Not in the set - make a duplicate */
      fn = strdup(fname);
      /* Add the duplicate to the set */
      buzzset_insert(dbg->fnames, &fn);
   }
   /* Make new entry */
   buzzdebug_entry_t e = buzzdebug_entry_new(line, col, fn);
   /* Add entry to offset -> script structure */
   buzzdict_set(dbg->off2script, &offset, &e);
   /* Add entry to script -> offset structure only if
    * 1. it does not exist, OR
    * 2. it exists, but the stored offset is larger than the passed one
    */
   if(!buzzdict_exists(dbg->script2off, &e) ||
      *buzzdict_get(dbg->script2off, &e, int32_t) > offset) {
      /* Make new entry */
      e = buzzdebug_entry_new(line, col, fn);
      /* Add entry to script -> offset structure */
      buzzdict_set(dbg->script2off, &e, &offset);
   }
}

/****************************************/
/****************************************/

const int32_t* buzzdebug_info_get_fromscript(buzzdebug_t dbg,
                                             uint64_t line,
                                             uint64_t col,
                                             const char* fname) {
   /* Make entry */
   buzzdebug_entry_t e = buzzdebug_entry_new(line, col, fname);
   /* Search for it in the structure */
   const int32_t* found = buzzdict_get(dbg->script2off, &e, int32_t);
   /* Cleanup */
   free(e);
   /* Return result */
   return found;
}

/****************************************/
/****************************************/

int offset_compare(const void* a, const void* b) {
   int32_t x = *(int32_t*)a;
   int32_t y = *(int32_t*)b;
   if(x < y) return -1;
   if(x > y) return 1;
   return 0;
}

void buzzdebug_breakpoint_set_offset(buzzdebug_t dbg,
                                     int32_t off) {
   if(buzzdarray_find(dbg->breakpoints, offset_compare, &off) ==
      buzzdarray_size(dbg->breakpoints)) {
      buzzdarray_push(dbg->breakpoints, &off);
   }
}

/****************************************/
/****************************************/

int buzzdebug_breakpoint_exists(buzzdebug_t dbg,
                                int32_t off) {
   return
      buzzdarray_find(dbg->breakpoints, offset_compare, &off) <
      buzzdarray_size(dbg->breakpoints);
}

/****************************************/
/****************************************/

buzzvm_state buzzdebug_continue(buzzvm_t vm,
                                buzzdebug_t dbg) {
   /* Make sure the VM is either ready or stopped */
   if(vm->state != BUZZVM_STATE_READY) {
      if(vm->state == BUZZVM_STATE_STOPPED)
         vm->state = BUZZVM_STATE_READY;
      else return vm->state;
   }
   /* Go on until the end, an error, or a breakpoint */
   while(!buzzdebug_breakpoint_exists(dbg, vm->pc) &&
         buzzvm_step(vm) == BUZZVM_STATE_READY);
   if(buzzdebug_breakpoint_exists(dbg, vm->pc))
      vm->state = BUZZVM_STATE_STOPPED;
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzdebug_closure_call(buzzvm_t vm,
                                    uint32_t argc,
                                    buzzdebug_t dbg) {
   /* Make sure the VM is either ready or stopped */
   if(vm->state != BUZZVM_STATE_READY) {
      if(vm->state == BUZZVM_STATE_STOPPED)
         vm->state = BUZZVM_STATE_READY;
      else return vm->state;
   }
   /* Insert the self table */
   buzzdarray_insert(vm->stack, buzzdarray_size(vm->stack) - argc - 1, buzzobj_new(BUZZTYPE_NIL));
   /* Push the argument count */
   buzzvm_pushi(vm, argc);
   /* Save the current stack depth */
   uint32_t stacks = buzzdarray_size(vm->stacks);
   /* Call the closure and keep stepping until
    * the stack count is back to the saved value */
   buzzvm_callc(vm);
   do {
      if(buzzdebug_breakpoint_exists(dbg, vm->pc))
         vm->state = BUZZVM_STATE_STOPPED;
      if(buzzvm_step(vm) != BUZZVM_STATE_READY)
         return vm->state;
   }
   while(stacks < buzzdarray_size(vm->stacks));
   return vm->state;
}

/****************************************/
/****************************************/

buzzvm_state buzzdebug_function_call(buzzvm_t vm,
                                     const char* fname,
                                     uint32_t argc,
                                     buzzdebug_t dbg) {
   /* Make sure the VM is either ready or stopped */
   if(vm->state != BUZZVM_STATE_READY) {
      if(vm->state == BUZZVM_STATE_STOPPED ||
         vm->state == BUZZVM_STATE_DONE)
         vm->state = BUZZVM_STATE_READY;
      else return vm->state;
   }
   /* Push the function name (return with error if not found) */
   buzzvm_pushs(vm, buzzvm_string_register(vm, fname, 0));
   /* Get associated symbol */
   buzzvm_gload(vm);
   /* Make sure it's a closure */
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   /* Move closure before arguments */
   if(argc > 0) {
      buzzdarray_insert(vm->stack,
                        buzzdarray_size(vm->stack) - argc - 1,
                        buzzvm_stack_at(vm, 1));
      buzzvm_pop(vm);
   }
   /* Call the closure */
   return buzzdebug_closure_call(vm, argc, dbg);
}

/****************************************/
/****************************************/

static void buzzdebug_print_obj(FILE* stream,
                                buzzobj_t o,
                                buzzvm_t vm);

struct buzzdebug_print_table_params_s {
   FILE* stream;
   buzzvm_t vm;
};

static void buzzdebug_print_table_elem(const void* key,
                                       void* data,
                                       void* params) {
   struct buzzdebug_print_table_params_s* p =
      (struct buzzdebug_print_table_params_s*)params;
   fprintf(p->stream, "\t\t\t");
   buzzdebug_print_obj(p->stream, *(buzzobj_t*)key, p->vm);
   fprintf(p->stream, " -> ");
   buzzdebug_print_obj(p->stream, *(buzzobj_t*)data, p->vm);
   fprintf(p->stream, "\n");
}

static void buzzdebug_print_table(FILE* stream,
                                  buzzdict_t t,
                                  buzzvm_t vm) {
   fprintf(stream, "[table] %" PRIu32 " elements\n", buzzdict_size(t));
   struct buzzdebug_print_table_params_s params = {
      .stream = stream,
      .vm = vm
   };
   buzzdict_foreach(t, buzzdebug_print_table_elem, &params);
}

void buzzdebug_print_obj(FILE* stream,
                         buzzobj_t o,
                         buzzvm_t vm) {
   switch(o->o.type) {
      case BUZZTYPE_NIL:
         fprintf(stream, "[nil]");
         break;
      case BUZZTYPE_INT:
         fprintf(stream, "[int] %d", o->i.value);
         break;
      case BUZZTYPE_FLOAT:
         fprintf(stream, "[float] %f", o->f.value);
         break;
      case BUZZTYPE_TABLE:
         buzzdebug_print_table(stream, o->t.value, vm);
         break;
      case BUZZTYPE_CLOSURE:
         if(o->c.value.isnative)
            fprintf(stream, "[n-closure] %d", o->c.value.ref);
         else
            fprintf(stream, "[c-closure] %d", o->c.value.ref);
         break;
      case BUZZTYPE_STRING:
         fprintf(stream, "[string] %d:'%s'", o->s.value.sid, o->s.value.str);
         break;
      default:
         fprintf(stream, "[TODO] type = %d", o->o.type);
   }
}

void buzzdebug_stack_dump(buzzvm_t vm,
                          uint32_t idx,
                          FILE* stream) {
   int64_t i;
   buzzdarray_t s = buzzdarray_get(vm->stacks, vm->stacks->size - idx, buzzdarray_t);
   char* curinstr = NULL;
   if(!buzz_instruction_deasm(vm->bcode, vm->oldpc, &curinstr)) curinstr = "deasm error";
   char* nextinstr = NULL;
   if(!buzz_instruction_deasm(vm->bcode, vm->pc, &nextinstr)) nextinstr = "deasm error";
   fprintf(stream, "============================================================\n");
   fprintf(stream, "state: %s\terror: %d\n", buzzvm_state_desc[vm->state], vm->error);
   fprintf(stream, "code size: %u\toldpc: %d\tpc: %d\n", vm->bcode_size, vm->oldpc, vm->pc);
   fprintf(stream, "stacks: %" PRIu64 "\tcur: %u\n", buzzdarray_size(vm->stacks), idx);
   fprintf(stream, "cur instr: %s\n", curinstr);
   fprintf(stream, "next instr: %s\n", nextinstr);
   for(i = buzzdarray_size(s)-1; i >= 0; --i) {
      fprintf(stream, "\t%" PRIu64 "\t", i);
      buzzobj_t o = buzzdarray_get(s, i, buzzobj_t);
      buzzdebug_print_obj(stream, o, vm);
      fprintf(stream, "\n");
   }
   fprintf(stream, "============================================================\n\n");
}

/****************************************/
/****************************************/

static int buzzdebug_backtrace_feps_cmp(const void* a,
                                        const void* b) {
   /* Convert parameters to actual type */
   struct buzzdebug_fep_entry_s* fep1 =
      (struct buzzdebug_fep_entry_s*)a;
   struct buzzdebug_fep_entry_s* fep2 =
      (struct buzzdebug_fep_entry_s*)b;
   /* Element is found if fep1's offset is greater than fep2's offset */
   return fep1->off > fep2->off;
}

void buzzdebug_backtrace_entry(uint32_t idx,
                               buzzdebug_t dbg,
                               uint32_t pc,
                               FILE* stream) {
   fprintf(stream, "#%u: ", idx);
   /* Look for the pc of the function entry point */
   struct buzzdebug_fep_entry_s test = {
      .off = pc,
      .fname = NULL,
      .params = NULL
   };   
   uint32_t i = buzzdarray_find(dbg->feps,
                                buzzdebug_backtrace_feps_cmp,
                                &test);
   if(i == 0) {
      fprintf(stream, "<function entry point for offset %u not found>\n", pc);
      return;
   }
   const struct buzzdebug_fep_entry_s* fep =
      buzzdarray_get(dbg->feps, i-1, struct buzzdebug_fep_entry_s*);
   /* Print the entry */
   fprintf(stream, "%s(", fep->fname);
   if(!buzzdarray_isempty(fep->params)) {
      fprintf(stream, ", %s",
              buzzdarray_get(fep->params, 0, char*));
      for(i = 1; i < buzzdarray_size(fep->params); ++i) {
         fprintf(stream, ", %s",
                 buzzdarray_get(fep->params, i, char*));
      }
   }
   fprintf(stream, ")\n");
}

void buzzdebug_backtrace(buzzvm_t vm,
                         buzzdebug_t dbg,
                         FILE* stream) {
   /* Make sure there's at least one stack present */
   if(buzzdarray_isempty(vm->stacks)) {
      fprintf(stream, "<no backtrace>\n");
      return;
   }
   /* If we're here, it's because there's at least one stack active */
   /* The top stack corresponds to the current pc */
   buzzdebug_backtrace_entry(1, dbg, vm->pc, stream);
   /* Go through the stacks beneath */
   /* For the stacks beneath, the return pc is at the stack top */
   /* The effective pc is that of the call instruction */
   for(int64_t i = buzzdarray_size(vm->stacks) - 2; i >=0; --i) {
      /* Get the return value */
      int32_t pc = buzzvm_stack_at(vm, 1)->i.value;
      /* Calculate the actual pc */
      pc -= 1;
      /* Handle the backtrace entry */
      buzzdebug_backtrace_entry(buzzdarray_size(vm->stacks) - i,
                                dbg,
                                pc,
                                stream);
   }
}

/****************************************/
/****************************************/
