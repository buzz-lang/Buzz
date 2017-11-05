#include "buzzstring.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/****************************************/
/****************************************/

#define function_register(TABLE, FNAME)                                   \
   buzzvm_push(vm, TABLE);                                                \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));               \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzstring_ ## FNAME)); \
   buzzvm_tput(vm);

/****************************************/
/****************************************/

int buzzstring_register(buzzvm_t vm) {
   /* Make "string" table */
   buzzobj_t t = buzzheap_newobj(vm->heap, BUZZTYPE_TABLE);
   /* Register methods */
   function_register(t, length);
   function_register(t, sub);
   function_register(t, concat);
   function_register(t, tostring);
   function_register(t, toint);
   function_register(t, tofloat);
   /* Register "string" table */
   buzzvm_pushs(vm, buzzvm_string_register(vm, "string", 1));
   buzzvm_push(vm, t);
   buzzvm_gstore(vm);
   /* All done */
   return vm->state;
}

/****************************************/
/****************************************/

int buzzstring_length(buzzvm_t vm) {
   /* Make sure one parameter has been passed */
   buzzvm_lnum_assert(vm, 1);
   /* Get the string */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   const char* s = buzzvm_stack_at(vm, 1)->s.value.str;
   /* Get its length and put it on the stack */
   buzzvm_pushi(vm, strlen(s));
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzstring_sub(buzzvm_t vm) {
   /* Make sure two or three parameters have been passed */
   if(buzzvm_lnum(vm) != 2 &&
      buzzvm_lnum(vm) != 3) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_LNUM,
                      "expected 2 or 3 parameters, got %" PRId64,
                      buzzvm_lnum(vm));
      return (vm)->state;
   }
   /* Get the string and its length */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   const char* s = buzzvm_stack_at(vm, 1)->s.value.str;
   int32_t ls = strlen(s);
   /* Get the starting index */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
   int32_t n = buzzvm_stack_at(vm, 1)->i.value;
   if(n >= ls) {
      /* Out of bounds */
      buzzvm_pushnil(vm);
      return buzzvm_ret1(vm);
   }
   /* Get the ending index */
   int32_t m = ls;
   if(buzzvm_lnum(vm) == 3) {
      buzzvm_lload(vm, 3);
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);
      m = buzzvm_stack_at(vm, 1)->i.value;
      if(m < n) {
         /* Out of bounds */
         buzzvm_pushnil(vm);
         return buzzvm_ret1(vm);
      }
      else if(m >= ls)
         /* Readjust m, because it goes beyond the string limits */
         m = ls;
   }
   /* Make new string for the substring */
   char* s2 = (char*)malloc(m - n + 1);
   strncpy(s2, s + n, m - n);
   s2[m - n] = 0;
   buzzvm_pushs(vm, buzzvm_string_register(vm, s2, 0));
   free(s2);
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzstring_concat(buzzvm_t vm) {
   /* Make sure at least two parameters have been passed */
   if(buzzvm_lnum(vm) < 2) {
      buzzvm_seterror(vm,
                      BUZZVM_ERROR_LNUM,
                      "expected at least 2 parameters, got %" PRId64,
                      buzzvm_lnum(vm));
      return (vm)->state;
   }
   /* Go through the parameters, make sure they are the right type, and calculate total length */
   uint32_t len = 0;
   for(uint32_t i = 1; i <= buzzvm_lnum(vm); ++i) {
      buzzvm_lload(vm, i);
      buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
      len += strlen(buzzvm_stack_at(vm, 1)->s.value.str);
   }
   /* Make a buffer to store the concatenated string */
   char* str = (char*)malloc(len+1);
   char* strp = str;
   const char* arg;
   /* Go through the strings and copy them into the buffer */
   for(uint32_t i = buzzvm_lnum(vm); i > 0; --i) {
      arg = buzzvm_stack_at(vm, i)->s.value.str;
      strcpy(strp, arg);
      strp += strlen(arg);
   }
   str[len] = 0;
   /* Make a new string */
   buzzvm_pushs(vm, buzzvm_string_register(vm, str, 0));
   free(str);
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzstring_tostring(buzzvm_t vm) {
   /* Make sure one parameter has been passed */
   buzzvm_lnum_assert(vm, 1);
   /* Get the object */
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   /* Make sure it's an int or a float */
   if(o->o.type != BUZZTYPE_INT &&
      o->o.type != BUZZTYPE_FLOAT) {
      /* Can't convert */
      buzzvm_pushnil(vm);
   }
   /* Perform conversion */
   char* str;
   if(buzzvm_stack_at(vm, 1)->o.type == BUZZTYPE_INT)
      asprintf(&str, "%" PRId32, o->i.value);
   else
      asprintf(&str, "%f", o->f.value);
   buzzvm_pushs(vm, buzzvm_string_register(vm, str, 0));
   free(str);
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzstring_toint(buzzvm_t vm) {
   /* Make sure one parameter has been passed */
   buzzvm_lnum_assert(vm, 1);
   /* Get the string */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   const char* s = buzzvm_stack_at(vm, 1)->s.value.str;
   /* Convert the string to int */
   char* endptr;
   errno = 0;
   int32_t i = strtod(s, &endptr);
   /* Was the conversion successful? */
   if((errno != 0 && i == 0) || /* An error occurred */
      (endptr == s)) {          /* No digit found */
      /* Yes, an error occurred */
      buzzvm_pushnil(vm);
   }
   else {
      /* All OK, return converted value */
      buzzvm_pushi(vm, i);
   }
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzstring_tofloat(buzzvm_t vm) {
   /* Make sure one parameter has been passed */
   buzzvm_lnum_assert(vm, 1);
   /* Get the string */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_STRING);
   const char* s = buzzvm_stack_at(vm, 1)->s.value.str;
   /* Convert the string to int */
   char* endptr;
   errno = 0;
   float f = strtof(s, &endptr);
   /* Was the conversion successful? */
   if((errno != 0 && f == 0) || /* An error occurred */
      (endptr == s)) {          /* No digit found */
      /* Yes, an error occurred */
      buzzvm_pushnil(vm);
   }
   else {
      /* All OK, return converted value */
      buzzvm_pushf(vm, f);
   }
   /* All done */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/
