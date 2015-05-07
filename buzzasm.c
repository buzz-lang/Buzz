#include "buzzasm.h"
#include "buzzdict.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/****************************************/
/****************************************/

/*
 * Resizes the bytecode buffer if necessary
 */
#define bcode_resize(INC)                       \
   if((*size) + (INC) >= bcode_max_size) {      \
      bcode_max_size *= 2;                      \
      *buf = realloc(*buf, bcode_max_size);     \
   }                                            \

/*
 * Adds an instructions to the bytecode buffer
 */
#define bcode_add_instr(OPCODE)                 \
   bcode_resize(1);                             \
   (*buf)[*size] = (OPCODE);                    \
   ++(*size);

/*
 * Adds an argument to the bytecode buffer
 * @param T The type of the argument (uint32_t or float)
 * @param CONFUN The function call to convert a string into T
 */
#define bcode_add_arg(T, CONVFUN)                                       \
   bcode_resize(sizeof(T))                                              \
   if(argstr == 0 || *argstr == 0) {                                    \
      fprintf(stderr, "ERROR: %s:%zu missing argument\n", fname, lineno); \
      return 2;                                                         \
   }                                                                    \
   char* endptr;                                                        \
   T arg = CONVFUN;                                                     \
   if((arg == 0) && (argstr == endptr)) {                               \
      char* label = strdup(argstr);                                     \
      buzzdict_set(labsubs, size, &label);                              \
   }                                                                    \
   memcpy((*buf) + (*size), (uint8_t*)(&arg), sizeof(T));               \
   (*size) += sizeof(T);

/*
 * Adds an integer argument to the bytecode buffer
 */
#define bcode_add_arg_i() bcode_add_arg(int32_t, strtoul(argstr, &endptr, 10))

/*
 * Adds a float argument to the bytecode buffer
 */
#define bcode_add_arg_f() bcode_add_arg(float, strtof(argstr, &endptr))

/*
 * Adds a label argument to the bytecode buffer
 */
#define bcode_add_arg_l() bcode_add_arg(int32_t, 0); { char* label = strdup(argstr); buzzdict_set(labsubs, size, &label); }

/*
 * Prints a warning if an argument is passed for an opcode that doesn't want one
 */
#define assert_noarg() if(argstr != 0 && *argstr != 0) fprintf(stderr, "WARNING: %s:%zu ignored argument \"%s\"\n", fname, lineno, argstr);

/*
 * Check whether the opcode matches the string label, and if so add stuff to the bytecode
 */
#define noarg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); assert_noarg(); continue; }
#define i_arg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); bcode_add_arg_i(); continue; }
#define f_arg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); bcode_add_arg_f(); continue; }
#define l_arg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); bcode_add_arg_l(); continue; }

void strkeydstryf(const void* key, void* data, void* params) {
   char* k = (char*)key;
   free(k);
   free(data);
}

void strdatadstryf(const void* key, void* data, void* params) {
   char* d = (char*)data;
   free((int32_t*)key);
   free(d);
}

struct buzz_asm_info_s {
   const char* fname;
   uint8_t* buf;
   uint32_t size;
   buzzdict_t labpos;
   int retval;
};

void buzz_asm_labsub(const void* key, void* data, void* params) {
   /* Get compilation state */
   struct buzz_asm_info_s* state = (struct buzz_asm_info_s*)params;
   /* Error found? Don't continue */
   if(state->retval != 0) return;
   /* Get sub position and label */
   int32_t subpos = *(const int32_t*)key;
   char* sublab = *(char**)data;
   /* Look for the sub label in the list of labels */
   int32_t* labpos = buzzdict_get(state->labpos, &sublab, int32_t);
   if(!labpos) {
      /* Label not found, error */
      fprintf(stderr, "ERROR: %s: unknown label \"%s\"\n", state->fname, sublab);
      state->retval = 2;
   }
   else if((*labpos) >= state->size) {
      /* Label beyond bytecode size, error */
      fprintf(stderr, "ERROR: %s: label \"%s\" at %d is beyond the bytecode size %u\n", state->fname, sublab, (*labpos), state->size);
      state->retval = 2;
   }
   else {
      /* Put the label value at its bytecode position */
      memcpy(state->buf + subpos, labpos, sizeof(int32_t));
   }
}

int buzz_asm(const char* fname,
             uint8_t** buf,
             uint32_t* size) {
   /* Make new label position dictionary */
   buzzdict_t labpos = buzzdict_new(100,
                                    sizeof(char*),
                                    sizeof(int32_t),
                                    buzzdict_strkeyhash,
                                    buzzdict_strkeycmp,
                                    strkeydstryf);
   /* Make new label substitution dictionary */
   buzzdict_t labsubs = buzzdict_new(100,
                                     sizeof(int32_t),
                                     sizeof(char*),
                                     buzzdict_intkeyhash,
                                     buzzdict_intkeycmp,
                                     strdatadstryf);
   /*
    * Perform first pass - compilation and label collection
    */
   /* Open file */
   FILE* fd = fopen(fname, "r");
   if(!fd) {
      perror(fname);
      return 1;
   }
   /* Create initial bytecode buffer */
   *buf = malloc(256);
   size_t bcode_max_size = 256;
   *size = 0;
   /* Read each line */
   ssize_t read;
   char* rawline = 0;
   char* trimline = 0;
   size_t len = 0;
   size_t lineno = 0;
   while((read = getline(&rawline, &len, fd)) != -1) {
      /* Increase line count */
      ++lineno;
      /* Trim leading space */
      trimline = rawline;
      while(isspace(*trimline)) ++trimline;
      /* Skip empty lines and comment lines */
      if(*trimline == 0 || *trimline == '#') continue;
      /* Trim trailing space */
      char* endc = trimline + strlen(trimline) - 1;
      while(endc > trimline && isspace(*endc)) --endc;
      *(endc + 1) = 0;
      /* Is the line a label? */
      if(*trimline == '@') {
         char* label = strdup(trimline);
         buzzdict_set(labpos, &label, size);
         fprintf(stderr, "[DEBUG] %s:%zu %s\tLABEL at %u (%p)\n", fname, lineno, label, *size, label);
         continue;
      }
      /* Fetch the instruction */
      fprintf(stderr, "[DEBUG] %s:%zu %s", fname, lineno, trimline);
      char* instr = strsep(&trimline, " \n\t");
      char* argstr = strsep(&trimline, " \n\t");
      fprintf(stderr, "\t[%s] [%s]\n", instr, argstr);
      /* Interpret the instruction */
      noarg_instr(BUZZVM_INSTR_NOP);
      noarg_instr(BUZZVM_INSTR_DONE);
      noarg_instr(BUZZVM_INSTR_PUSHNIL);
      noarg_instr(BUZZVM_INSTR_POP);
      noarg_instr(BUZZVM_INSTR_RET0);
      noarg_instr(BUZZVM_INSTR_RET1);
      noarg_instr(BUZZVM_INSTR_ADD);
      noarg_instr(BUZZVM_INSTR_SUB);
      noarg_instr(BUZZVM_INSTR_MUL);
      noarg_instr(BUZZVM_INSTR_DIV);
      noarg_instr(BUZZVM_INSTR_AND);
      noarg_instr(BUZZVM_INSTR_OR);
      noarg_instr(BUZZVM_INSTR_NOT);
      noarg_instr(BUZZVM_INSTR_EQ);
      noarg_instr(BUZZVM_INSTR_GT);
      noarg_instr(BUZZVM_INSTR_GTE);
      noarg_instr(BUZZVM_INSTR_LT);
      noarg_instr(BUZZVM_INSTR_LTE);
      noarg_instr(BUZZVM_INSTR_SHOUT);
      noarg_instr(BUZZVM_INSTR_PUSHT);
      noarg_instr(BUZZVM_INSTR_TPUT);
      noarg_instr(BUZZVM_INSTR_TGET);
      noarg_instr(BUZZVM_INSTR_VSCREATE);
      noarg_instr(BUZZVM_INSTR_VSPUT);
      noarg_instr(BUZZVM_INSTR_VSGET);
      noarg_instr(BUZZVM_INSTR_CALLCN);
      noarg_instr(BUZZVM_INSTR_CALLCC);
      noarg_instr(BUZZVM_INSTR_PUSHCN);
      noarg_instr(BUZZVM_INSTR_PUSHCC);
      f_arg_instr(BUZZVM_INSTR_PUSHF);
      i_arg_instr(BUZZVM_INSTR_PUSHI);
      i_arg_instr(BUZZVM_INSTR_DUP);
      l_arg_instr(BUZZVM_INSTR_JUMP);
      l_arg_instr(BUZZVM_INSTR_JUMPZ);
      l_arg_instr(BUZZVM_INSTR_JUMPNZ);
      /* No match, error */
      fprintf(stderr, "ERROR: %s:%zu unknown instruction \"%s\"\n", fname, lineno, instr);
      return 2;
   }
   /* Close file */
   fclose(fd);
   /*
    * Perform second pass: label substitution
    */
   /* Go through the substitutions */
   struct buzz_asm_info_s state = {
      .fname = fname,
      .buf = *buf,
      .size = *size,
      .labpos = labpos,
      .retval = 0
   };
   buzzdict_foreach(labsubs, buzz_asm_labsub, &state);
   /* Cleanup */
   free(rawline);
   buzzdict_destroy(&labpos);
   buzzdict_destroy(&labsubs);
   return state.retval;
}

/****************************************/
/****************************************/

#define write_arg(T, FMT)                                               \
   if(i + sizeof(T) >= size) {                                          \
      fprintf(stderr, "ERROR: %s: not enough bytes in bytecode for argument of %s at %u\n", fname, buzzvm_instr_desc[op], i); \
      return 2;                                                         \
   }                                                                    \
   fprintf(fd, " " FMT, (*(T*)(buf+i+1)));                              \
   i += sizeof(T);

int buzz_deasm(const uint8_t* buf,
               uint32_t size,
               const char* fname) {
   /* Open file */
   FILE* fd = fopen(fname, "w");
   if(!fd) {
      perror(fname);
      return 1;
   }
   /* Calculate max opcode */
   uint32_t maxop = sizeof(buzzvm_instr_desc) / sizeof(char*);
   /* Go through the bytecode */
   for(uint32_t i = 0; i < size; ++i) {
      /* Fetch instruction */
      uint8_t op = buf[i];
      /* Check that it's in the allowed range */
      if(op >= maxop) {
         fprintf(stderr, "ERROR: %s: unknown opcode %u at %u\n", fname, op, i);
         return 2;
      }
      /* Write the op description */
      fprintf(fd, "%s", buzzvm_instr_desc[op]);
      /* Does the opcode have an argument? */
      if(op == BUZZVM_INSTR_PUSHF) {
         /* Float argument */
         write_arg(float, "%f");
      }
      else if(op > BUZZVM_INSTR_PUSHF) {
         /* Integer argument */
         write_arg(int32_t, "%u");
      }
      /* Newline */
      fprintf(fd, "\n");
   }
   /* Close file */
   fclose(fd);
   return 0;
}

/****************************************/
/****************************************/
