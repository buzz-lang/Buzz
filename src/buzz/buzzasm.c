#include "buzzasm.h"
#include "buzzdebug.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

/****************************************/
/****************************************/

/*
 * Resizes the bytecode buffer if necessary
 */
#define bcode_resize(INC)                       \
   if((*size) + (INC) >= bcode_max_size) {      \
      bcode_max_size += INC;                    \
      *buf = realloc(*buf, bcode_max_size);     \
   }                                            \

/*
 * Adds an instruction to the bytecode buffer
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
      int32_t pos = *size;                                              \
      buzzdict_set(labsubs, &pos, &label);                              \
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
#define bcode_add_arg_l()                                               \
   bcode_resize(sizeof(int32_t));                                       \
   if(argstr == 0 || *argstr == 0) {                                    \
      fprintf(stderr, "ERROR: %s:%zu missing argument\n", fname, lineno); \
      return 2;                                                         \
   }                                                                    \
   char* label = strdup(argstr);                                        \
   int32_t pos = *size;                                                 \
   buzzdict_set(labsubs, &pos, &label);                                 \
   (*size) += sizeof(int32_t);

/****************************************/
/****************************************/

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

/****************************************/
/****************************************/

void strkeydstryf(const void* key, void* data, void* params) {
   free(*(char**)key);
}

void strdatadstryf(const void* key, void* data, void* params) {
   free(*(char**)data);
}

/****************************************/
/****************************************/

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
   const int32_t* labpos = buzzdict_get(state->labpos, &sublab, int32_t);
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

/****************************************/
/****************************************/

int buzz_asm(const char* fname,
             uint8_t** buf,
             uint32_t* size,
             buzzdebug_t* dbg) {
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
                                     buzzdict_int32keyhash,
                                     buzzdict_int32keycmp,
                                     strdatadstryf);
   /* Make new debug data structure */
   *dbg = buzzdebug_new();
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
   /*
    * Perform first pass - compilation and label collection
    */
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
      /* Is the line a string? */
      if(*trimline == '\'') {
         ++trimline;
         trimline[strlen(trimline)-1] = 0;
         size_t l = strlen(trimline) + 1;
         bcode_resize(l);
         strcpy((char*)(*buf + *size), trimline);
         *size += l;
         continue;
      }
      /* Trim trailing space */
      char* endc = trimline + strlen(trimline) - 1;
      while(endc > trimline && isspace(*endc)) --endc;
      *(endc + 1) = 0;
      /* Is the line a string count marker? */
      if(*trimline == '!') {
         ++trimline;
         uint16_t n = strtol(trimline, NULL, 10);
         bcode_resize(sizeof(uint16_t));
         memcpy((*buf) + (*size), &n, sizeof(uint16_t)); \
         *size += sizeof(uint16_t);
         continue;
      }      
      /* Is the line a label? */
      if(*trimline == '@') {
         /* Parse label and debug info */
         char* labelinfo = strsep(&trimline, "|\n");
         char* debuginfo = strsep(&trimline, "|\n");
         /* Add debug information, if any */
         if(debuginfo && *debuginfo) {
            /* Parse line and column data */
            char* line = strsep(&debuginfo, ",\n");
            char* col = strsep(&debuginfo, ",\n");
            char* srcfname = strsep(&debuginfo, ",\n");
            if(srcfname) {
               uint64_t l = 0;
               uint64_t c = 0;
               if(line) l = strtol(line, NULL, 10);
               if(col)  c = strtol(col, NULL, 10);
               buzzdebug_info_set(*dbg, *size, l, c, srcfname);
            }
         }
         /* Remove trailing space from label info */
         endc = labelinfo + strlen(labelinfo) - 1;
         while(endc > labelinfo && isspace(*endc)) --endc;
         *(endc + 1) = 0;
         /* Copy label information for storage */
         char* label = strdup(labelinfo);
         buzzdict_set(labpos, &label, size);
         continue;
      }
      /* Fetch the instruction */
      char* instrinfo = strsep(&trimline, "|\n");
      char* debuginfo = strsep(&trimline, "|\n");
      char* instr = strsep(&instrinfo, " \n\t");
      char* argstr = strsep(&instrinfo, " \n\t");
      /* Add debug information, if any */
      if(debuginfo && *debuginfo) {
         /* Parse line and column data */
         char* line = strsep(&debuginfo, ",\n");
         char* col = strsep(&debuginfo, ",\n");
         char* srcfname = strsep(&debuginfo, ",\n");
         if(srcfname) {
            uint64_t l = 0;
            uint64_t c = 0;
            if(line) l = strtol(line, NULL, 10);
            if(col)  c = strtol(col, NULL, 10);
            buzzdebug_info_set(*dbg, *size, l, c, srcfname);
         }
      }
      /* Interpret the instruction */
      noarg_instr(BUZZVM_INSTR_NOP);
      noarg_instr(BUZZVM_INSTR_DONE);
      noarg_instr(BUZZVM_INSTR_PUSHNIL);
      noarg_instr(BUZZVM_INSTR_DUP);
      noarg_instr(BUZZVM_INSTR_POP);
      noarg_instr(BUZZVM_INSTR_RET0);
      noarg_instr(BUZZVM_INSTR_RET1);
      noarg_instr(BUZZVM_INSTR_ADD);
      noarg_instr(BUZZVM_INSTR_SUB);
      noarg_instr(BUZZVM_INSTR_MUL);
      noarg_instr(BUZZVM_INSTR_DIV);
      noarg_instr(BUZZVM_INSTR_MOD);
      noarg_instr(BUZZVM_INSTR_POW);
      noarg_instr(BUZZVM_INSTR_UNM);
      noarg_instr(BUZZVM_INSTR_LAND);
      noarg_instr(BUZZVM_INSTR_LOR);
      noarg_instr(BUZZVM_INSTR_LNOT);
      noarg_instr(BUZZVM_INSTR_BAND);
      noarg_instr(BUZZVM_INSTR_BOR);
      noarg_instr(BUZZVM_INSTR_BNOT);
      noarg_instr(BUZZVM_INSTR_LSHIFT);
      noarg_instr(BUZZVM_INSTR_RSHIFT);
      noarg_instr(BUZZVM_INSTR_EQ);
      noarg_instr(BUZZVM_INSTR_NEQ);
      noarg_instr(BUZZVM_INSTR_GT);
      noarg_instr(BUZZVM_INSTR_GTE);
      noarg_instr(BUZZVM_INSTR_LT);
      noarg_instr(BUZZVM_INSTR_LTE);
      noarg_instr(BUZZVM_INSTR_GLOAD);
      noarg_instr(BUZZVM_INSTR_GSTORE);
      noarg_instr(BUZZVM_INSTR_PUSHT);
      noarg_instr(BUZZVM_INSTR_TPUT);
      noarg_instr(BUZZVM_INSTR_TGET);
      noarg_instr(BUZZVM_INSTR_CALLC);
      noarg_instr(BUZZVM_INSTR_CALLS);
      f_arg_instr(BUZZVM_INSTR_PUSHF);
      i_arg_instr(BUZZVM_INSTR_PUSHI);
      i_arg_instr(BUZZVM_INSTR_PUSHS);
      i_arg_instr(BUZZVM_INSTR_PUSHCN);
      i_arg_instr(BUZZVM_INSTR_PUSHCC);
      i_arg_instr(BUZZVM_INSTR_PUSHL);
      i_arg_instr(BUZZVM_INSTR_LLOAD);
      i_arg_instr(BUZZVM_INSTR_LSTORE);
      i_arg_instr(BUZZVM_INSTR_LREMOVE);
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
      fprintf(stderr, "ERROR: %s: not enough bytes in bytecode for argument of %s at %" PRIu32 "\n", fname, buzzvm_instr_desc[op], i); \
      fclose(fd);                                                       \
      return 2;                                                         \
   }                                                                    \
   fprintf(fd, " " FMT, (*(T*)(buf+i+1)));                              \
   if(buzzdebug_info_exists_offset(dbg, &i)) {                          \
      fprintf(fd, "\t|%" PRIu64 ",%" PRIu64 ",%s",                                    \
              (*buzzdebug_info_get_fromoffset(dbg, &i))->line,          \
              (*buzzdebug_info_get_fromoffset(dbg, &i))->col,           \
              (*buzzdebug_info_get_fromoffset(dbg, &i))->fname);        \
   }                                                                    \
   i += sizeof(T);

int buzz_deasm(const uint8_t* buf,
               uint32_t size,
               buzzdebug_t dbg,
               const char* fname) {
   /* Open file */
   FILE* fd = fopen(fname, "w");
   if(!fd) {
      perror(fname);
      return 1;
   }
   /*
    * Phase 1: fetch the strings
    */
   /* Fetch and print the string count */
   uint16_t count;
   memcpy(&count, buf, sizeof(uint16_t));
   fprintf(fd, "!%u\n", count);
   /* Go through the strings and print them */
   uint32_t i = sizeof(uint16_t);
   long int c = 0;
   for(; (c < count) && (i < size); ++c) {
      /* Print string */
      fprintf(fd, "'%s\n", ((char*)buf + i));
      /* Advance to first character of next string */
      while(*(buf + i) != 0) ++i;
      ++i;
   }
   if(c < count) {
      fprintf(stderr, "ERROR: %s: scanning string went up to end of file (%ld still to parse)\n", fname, (count - c));
      fclose(fd);
      return 2;
   }
   /*
    * Phase 2: deassemble the opcodes
    */
   /* Calculate max opcode */
   uint32_t maxop = BUZZVM_INSTR_COUNT;
   /* Go through the bytecode */
   for(; i < size; ++i) {
      /* Fetch instruction */
      uint8_t op = buf[i];
      /* Check that it's in the allowed range */
      if(op >= maxop) {
         fprintf(stderr, "ERROR: %s: unknown opcode %u at %u\n", fname, op, i);
         fclose(fd);
         return 2;
      }
      /* Write the op description */
      fprintf(fd, "%u:\t%s", i, buzzvm_instr_desc[op]);
      /* Does the opcode have an argument? */
      if(op == BUZZVM_INSTR_PUSHF) {
         /* Float argument */
         write_arg(float, "%f");
      }
      else if(op > BUZZVM_INSTR_PUSHF) {
         /* Integer argument */
         write_arg(int32_t, "%" PRId32);
      }
      else {
         if(buzzdebug_info_exists_offset(dbg, &i)) {
            fprintf(fd, "\t|%" PRIu64 ",%" PRIu64 ",%s",
                    (*buzzdebug_info_get_fromoffset(dbg, &i))->line,
                    (*buzzdebug_info_get_fromoffset(dbg, &i))->col,
                    (*buzzdebug_info_get_fromoffset(dbg, &i))->fname);
         }
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

int buzz_instruction_deasm(const uint8_t* bcode,
                           uint32_t off,
                           char** buf) {
   /* Fetch instruction */
   uint8_t op = bcode[off];
   /* Check that it's in the allowed range */
   if(op >= BUZZVM_INSTR_COUNT) return 0;
   /* Does the opcode have an argument? */
   if(op == BUZZVM_INSTR_PUSHF) {
      /* Float argument */
      asprintf(buf, "%s %f",
               buzzvm_instr_desc[op],
               *(float*)(bcode+off+1));
   }
   else if(op > BUZZVM_INSTR_PUSHF) {
      /* Integer argument */
      asprintf(buf, "%s %d",
               buzzvm_instr_desc[op],
               *(int32_t*)(bcode+off+1));
   }
   else {
      /* No argument */
      asprintf(buf, "%s", buzzvm_instr_desc[op]);
   }
   /* All OK */
   return 1;
}

/****************************************/
/****************************************/
