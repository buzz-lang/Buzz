#include "buzzvm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/****************************************/
/****************************************/

buzzvm_t buzzvm_new(uint32_t stack_size,
                    uint32_t flist_size) {
   /* Create VM state. calloc() takes care of zeroing everything */
   buzzvm_t vms = (buzzvm_t)calloc(1, sizeof(struct buzzvm_s));
   /* Create stack. calloc() takes care of zeroing everything */
   vms->stack_size = stack_size;
   vms->stack = calloc(stack_size, sizeof(buzzvm_stack_elem));
   /* Create function list. calloc() takes care of zeroing everything */
   vms->flist_size = flist_size;
   vms->flist = calloc(flist_size, sizeof(buzzvm_funp));
   /* Return new vm */
   return vms;
}

/****************************************/
/****************************************/

void buzzvm_reset(buzzvm_t vm) {
   memset(vm->stack, 0, vm->stack_size);
   vm->stack_top = 0;
   vm->pc = 0;
   if(vm->bcode) vm->state = BUZZVM_STATE_READY;
   else vm->state = BUZZVM_STATE_NOCODE;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

void buzzvm_destroy(buzzvm_t* vm) {
   free((*vm)->stack);
   free(*vm);
   *vm = 0;
}

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
      fprintf(stderr, "ERROR: %s:%zu can't convert \"%s\" to number\n", fname, lineno, argstr); \
      return 2;                                                         \
   }                                                                    \
   memcpy((*buf) + (*size), (uint8_t*)(&arg), sizeof(T));               \
   (*size) += sizeof(T);

/*
 * Adds an integer argument to the bytecode buffer
 */
#define bcode_add_arg_i() bcode_add_arg(uint32_t, strtoul(argstr, &endptr, 10))

/*
 * Adds a float argument to the bytecode buffer
 */
#define bcode_add_arg_f() bcode_add_arg(float, strtof(argstr, &endptr))

/*
 * Prints a warning if an argument is passed for an opcode that doesn't want one
 */
#define assert_noarg() if(argstr != 0 && *argstr != 0) fprintf(stderr, "WARNING: %s:%zu ignored argument \"%s\"\n", fname, lineno, argstr);

/*
 * Check whether the opcode matches the string label, and if so add stuff to the bytecode
 */
#define noarg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); assert_noarg(); continue; }
#define f_arg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); bcode_add_arg_f(); continue; }
#define i_arg_instr(OP) if(strcmp(instr, buzzvm_instr_desc[OP]) == 0) { bcode_add_instr(OP); bcode_add_arg_i(); continue; }

int buzzvm_asm(buzzvm_t vm,
               const char* fname,
               uint8_t** buf,
               uint32_t* size) {
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
      /* Fetch the instruction */
      fprintf(stderr, "[DEBUG] %s:%zu %s", fname, lineno, trimline);
      char* instr = strsep(&trimline, " \n\t");
      char* argstr = strsep(&trimline, " \n\t");
      fprintf(stderr, "\t[%s] [%s]\n", instr, argstr);
      /* Interpret the instruction */
      noarg_instr(BUZZVM_INSTR_NOP);
      noarg_instr(BUZZVM_INSTR_DONE);
      f_arg_instr(BUZZVM_INSTR_PUSH);
      i_arg_instr(BUZZVM_INSTR_AT);
      noarg_instr(BUZZVM_INSTR_POP);
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
      i_arg_instr(BUZZVM_INSTR_JUMP);
      i_arg_instr(BUZZVM_INSTR_JUMPZ);
      i_arg_instr(BUZZVM_INSTR_JUMPNZ);
      i_arg_instr(BUZZVM_INSTR_JUMPSUB);
      noarg_instr(BUZZVM_INSTR_RET);
      i_arg_instr(BUZZVM_INSTR_CALL);
      /* No match, error */
      fprintf(stderr, "ERROR: %s:%zu unknown instruction \"%s\"\n", fname, lineno, instr); \
      return 2;
   }
   /* Close file */
   fclose(fd);
   return 0;
}

/****************************************/
/****************************************/

void buzzvm_set_bcode(buzzvm_t vm,
                      const uint8_t* bcode,
                      uint32_t bcode_size) {
   vm->bcode_size = bcode_size;
   vm->bcode = bcode;
   vm->pc = 0;
   vm->state = BUZZVM_STATE_READY;
   vm->error = BUZZVM_ERROR_NONE;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { vm->state = BUZZVM_STATE_ERROR; vm->error = BUZZVM_ERROR_PC; return vm->state; }

#define assert_stack(IDX) if((IDX) < 0 || (IDX) >= vm->stack_size) { vm->state = BUZZVM_STATE_ERROR; vm->error = BUZZVM_ERROR_STACK; return vm->state; }

#define inc_pc() ++vm->pc; assert_pc(vm->pc);

#define inc_stack() ++vm->stack_top; assert_stack(vm->stack_top);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(&(vm->bcode[vm->pc+1]))); vm->pc += sizeof(TYPE) + 1;

#define binary_op_f(OP) assert_stack(vm->stack_top-2); --vm->stack_top; vm->stack[vm->stack_top-1].f = vm->stack[vm->stack_top].f OP vm->stack[vm->stack_top-1].f; inc_pc();

#define binary_op_i(OP) assert_stack(vm->stack_top-2); --vm->stack_top; vm->stack[vm->stack_top-1].i = vm->stack[vm->stack_top].i OP vm->stack[vm->stack_top-1].i; inc_pc();

buzzvm_state buzzvm_step(buzzvm_t vm) {
   /* Can't execute if not ready */
   if(vm->state != BUZZVM_STATE_READY) return vm->state;
   /* Fetch instruction and (potential) argument */
   uint8_t instr = vm->bcode[vm->pc];
   /* Execute instruction */
   switch(instr) {
      case BUZZVM_INSTR_NOP: {
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_DONE: {
         vm->state = BUZZVM_STATE_DONE;
         break;
      }
      case BUZZVM_INSTR_PUSH: {
         get_arg(float);
         vm->stack[vm->stack_top].f = arg;
         inc_stack();
         break;
      }
      case BUZZVM_INSTR_AT: {
         get_arg(uint32_t);
         assert_stack(arg);
         vm->stack[vm->stack_top].f = vm->stack[arg].f;
         inc_stack();
         break;
      }
      case BUZZVM_INSTR_POP: {
         --vm->stack_top;
         assert_stack(vm->stack_top);
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_ADD: {
         binary_op_f(+);
         break;
      }
      case BUZZVM_INSTR_SUB: {
         binary_op_f(-);
         break;
      }
      case BUZZVM_INSTR_MUL: {
         binary_op_f(*);
         break;
      }
      case BUZZVM_INSTR_DIV: {
         binary_op_f(/);
         break;
      }
      case BUZZVM_INSTR_AND: {
         binary_op_i(&);
         break;
      }
      case BUZZVM_INSTR_OR: {
         binary_op_i(|);
         break;
      }
      case BUZZVM_INSTR_NOT: {
         vm->stack[vm->stack_top-1].i = !vm->stack[vm->stack_top-1].i;
         inc_pc();
         break;
      }
      case BUZZVM_INSTR_EQ: {
         binary_op_f(==);
         break;
      }
      case BUZZVM_INSTR_GT: {
         binary_op_f(>);
         break;
      }
      case BUZZVM_INSTR_GTE: {
         binary_op_f(>=);
         break;
      }
      case BUZZVM_INSTR_LT: {
         binary_op_f(>);
         break;
      }
      case BUZZVM_INSTR_LTE: {
         binary_op_f(>=);
         break;
      }
      case BUZZVM_INSTR_JUMP: {
         get_arg(uint32_t);
         vm->pc = arg;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_JUMPZ: {
         assert_stack(vm->stack_top - 1);
         get_arg(uint32_t);
         if(vm->stack[vm->stack_top - 1].i == 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPNZ: {
         assert_stack(vm->stack_top - 1);
         get_arg(uint32_t);
         if(vm->stack[vm->stack_top - 1].i != 0) {
            vm->pc = arg;
            assert_pc(vm->pc);
         }
         break;
      }
      case BUZZVM_INSTR_JUMPSUB: {
         get_arg(uint32_t);
         vm->stack[vm->stack_top].i = vm->pc;
         inc_stack();
         vm->pc = arg;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_RET: {
         --vm->stack_top;
         assert_stack(vm->stack_top);
         vm->pc = vm->stack[vm->stack_top].i;
         assert_pc(vm->pc);
         break;
      }
      case BUZZVM_INSTR_CALL: {
         get_arg(uint32_t);
         if(arg >= vm->flist_entries) {
            vm->state = BUZZVM_STATE_ERROR;
            vm->error = BUZZVM_ERROR_FLIST;
            return vm->state;
         }
         vm->flist[arg](vm);
         break;
      }
   }
   return vm->state;
}

/****************************************/
/****************************************/

int64_t buzzvm_register_function(buzzvm_t vm,
                                  buzzvm_funp funp) {
   if(vm->flist_entries >= vm->flist_size) {
      return -1;
   }
   vm->flist[vm->flist_entries] = funp;
   ++vm->flist_entries;
   return vm->flist_entries - 1;
}

/****************************************/
/****************************************/
