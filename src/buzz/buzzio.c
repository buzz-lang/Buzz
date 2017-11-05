#include "buzzio.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

/****************************************/
/****************************************/

#define function_register(TABLE, FNAME)                                 \
   buzzvm_push(vm, TABLE);                                              \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));             \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzio_ ## FNAME));   \
   buzzvm_tput(vm);

#define filehandle_get(VAR)                                   \
   buzzvm_lload(vm, 1);                                       \
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);                 \
   buzzvm_pushs(vm, buzzvm_string_register(vm, "handle", 1)); \
   buzzvm_tget(vm);                                           \
   FILE* VAR = buzzvm_stack_at(vm, 1)->u.value;               \
   buzzvm_pop(vm);

/****************************************/
/****************************************/

static void buzzio_update_error(buzzvm_t vm) {
   /* Get table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "io", 1));
   buzzvm_gload(vm);
   buzzobj_t t = buzzvm_stack_at(vm, 1);
   /* Update error id */
   buzzvm_push(vm, t);
   buzzvm_pushs(vm, buzzvm_string_register(vm, "errno", 1));
   buzzvm_pushi(vm, errno);
   buzzvm_tput(vm);
   /* Update error message */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "error_message", 1));
   if(errno) buzzvm_pushs(vm, buzzvm_string_register(vm, strerror(errno), 0));
   else buzzvm_pushs(vm, buzzvm_string_register(vm, "No error", 0));
   buzzvm_tput(vm);
}

/****************************************/
/****************************************/

int buzzio_register(buzzvm_t vm) {
   /* Make "io" table */
   buzzobj_t t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   /* Register methods */
   function_register(t, fopen);
   function_register(t, fclose);
   function_register(t, fsize);
   function_register(t, fforeach);
   function_register(t, fwrite);
   /* Register "io" table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "io", 1));
   buzzvm_push(vm, t);
   buzzvm_gstore(vm);
   /* Register error information */
   errno = 0;
   buzzio_update_error(vm);
   /* All done */
   return vm->state;
}

/****************************************/
/****************************************/

int buzzio_fopen(buzzvm_t vm) {
   /* Make sure two parameters have been passed */
   buzzvm_lnum_assert(vm, 2);
   /* Get file name */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   const char* fname = buzzvm_stack_at(vm, 1)->s.value.str;
   buzzvm_pop(vm);
   /* Get open mode */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   const char* fmode = buzzvm_stack_at(vm, 1)->s.value.str;
   buzzvm_pop(vm);
   /* Try to open the file */
   FILE* f = fopen(fname, fmode);
   /* Register error information */
   buzzio_update_error(vm);
   if(!f) {
      /* Error occurred, return nil */
      buzzvm_pushnil(vm);
   }
   else {
      /* Create new table */
      buzzobj_t t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
      /* Add file handle */
      buzzvm_push(vm, t);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "handle", 1));
      buzzvm_pushu(vm, f);
      buzzvm_tput(vm);
      /* Add file name */
      buzzvm_push(vm, t);
      buzzvm_pushs(vm, buzzvm_string_register(vm, "name", 1));
      buzzvm_pushs(vm, buzzvm_string_register(vm, fname, 0));
      buzzvm_tput(vm);
      /* Push the table on the stack */
      buzzvm_push(vm, t);
   }
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzio_fclose(buzzvm_t vm) {
   /* Make sure there is one parameter */
   buzzvm_lnum_assert(vm, 1);
   /* Get file handle */
   filehandle_get(f);
   /* Close the file */
   fclose(f);
   /* Register error information */
   buzzio_update_error(vm);
   /* All done */
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzio_fsize(buzzvm_t vm) {
   /* Make sure there are is one parameter */
   buzzvm_lnum_assert(vm, 1);
   /* Get file handle */
   filehandle_get(f);
   /* Remember the current position */
   long int cur = ftell(f);
   /* Get the file size */
   fseek(f, 0, SEEK_END);
   long int sz = ftell(f);
   /* Go back to saved position */
   fseek(f, cur, SEEK_SET);
   /* Put size on the stack */
   buzzvm_pushi(vm, sz);
   /* Register error information */
   buzzio_update_error(vm);
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzio_fforeach(buzzvm_t vm) {
   /* Make sure there are two parameters */
   buzzvm_lnum_assert(vm, 2);
   /* Get file handle */
   filehandle_get(f);
   /* Get closure */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   /* Go through the file lines */
   int vmstate = vm->state;
   size_t cap; /* line buffer capacity */
   char* line = NULL;
   ssize_t len = getline(&line, &cap, f);
   while(len >= 0 &&
         vmstate == BUZZVM_STATE_READY) {
      /* Remove newline if present */
      if(line[len-1] == '\n') line[len-1] = '\0';
      /* Put closure on the stack (will be wiped by buzzvm_closure_call) */
      buzzvm_dup(vm);
      /* Push string argument */
      buzzvm_pushs(vm, buzzvm_string_register(vm, line, 0));
      /* Call closure */
      vmstate = buzzvm_closure_call(vm, 1);
      /* Next line */
      len = getline(&line, &cap, f);
   }
   /* Register error information */
   buzzio_update_error(vm);
   /* All done */
   free(line);
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

int buzzio_fwrite(buzzvm_t vm) {
   /* Make sure there are at least two parameters */
   if(buzzvm_lnum(vm) < 2) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_LNUM,
                      "expected at least 2 parameters, got %" PRId64,
                      buzzvm_lnum(vm));
      return vm->state;
   }
   /* Used to store the return value of fprintf */
   int err = 0;
   /* Get file handle */
   filehandle_get(f);
   /* Go through the arguments */
   for(int i = 2; err >= 0 && i <= buzzvm_lnum(vm); ++i) {
      /* Get argument */
      buzzvm_lload(vm, i);
      buzzobj_t o = buzzvm_stack_at(vm, 1);
      buzzvm_pop(vm);
      switch(o->o.type) {
         case BUZZTYPE_NIL:
            err = fprintf(f, "[nil]");
            break;
         case BUZZTYPE_INT:
            err = fprintf(f, "%d", o->i.value);
            break;
         case BUZZTYPE_FLOAT:
            err = fprintf(f, "%f", o->f.value);
            break;
         case BUZZTYPE_TABLE:
            err = fprintf(f, "[table with %" PRIu32" elems]", buzzdict_size(o->t.value));
            break;
         case BUZZTYPE_CLOSURE:
            if(o->c.value.isnative)
               err = fprintf(f, "[n-closure @%" PRId32 "]", o->c.value.ref);
            else
               err = fprintf(f, "[c-closure @%" PRId32 "]", o->c.value.ref);
            break;
         case BUZZTYPE_STRING:
            err = fprintf(f, "%s", o->s.value.str);
            break;
         case BUZZTYPE_USERDATA:
            err = fprintf(f, "[userdata @%p]", o->u.value);
            break;
         default:
            err = -1;
            break;
      }
   }
   /* Add newline at the end */
   if(err >= 0) fprintf(f, "\n");
   /* Register error information */
   buzzio_update_error(vm);
   /* All done */
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/
