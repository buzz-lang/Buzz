#ifndef BUZZVM_H
#define BUZZVM_H

#include "buzzdict.h"
#include "buzztype.h"

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * VM states
    */
   typedef enum {
      BUZZVM_STATE_NOCODE = 0, // No code loaded
      BUZZVM_STATE_READY,      // Ready to execute next instruction
      BUZZVM_STATE_DONE,       // Program finished
      BUZZVM_STATE_ERROR       // Error occurred
   } buzzvm_state;
   static const char *buzzvm_state_desc[] = { "no code", "ready", "done", "error" };

   /*
    * VM error codes
    */
   typedef enum {
      BUZZVM_ERROR_NONE = 0, // No error
      BUZZVM_ERROR_INSTR,    // Unknown instruction 
      BUZZVM_ERROR_STACK,    // Stack top out of range
      BUZZVM_ERROR_PC,       // Program counter out of range
      BUZZVM_ERROR_FLIST     // Function call id out of range
   } buzzvm_error;
   static const char *buzzvm_error_desc[] = { "none", "unknown instruction", "pc out of range", "function id out of range" };

   /*
    * VM instructions
    */
   typedef enum {
      /*
       * Opcodes without argument
       */
      BUZZVM_INSTR_NOP = 0,  // No operation
      BUZZVM_INSTR_DONE,     // End of the program
      BUZZVM_INSTR_POP,      // Pop value from stack
      BUZZVM_INSTR_RET,      // Sets PC to value at stack top, then pops it
      BUZZVM_INSTR_ADD,      // Push stack(#1) + stack(#2), pop operands
      BUZZVM_INSTR_SUB,      // Push stack(#1) - stack(#2), pop operands
      BUZZVM_INSTR_MUL,      // Push stack(#1) * stack(#2), pop operands
      BUZZVM_INSTR_DIV,      // Push stack(#1) / stack(#2), pop operands
      BUZZVM_INSTR_AND,      // Push stack(#1) & stack(#2), pop operands
      BUZZVM_INSTR_OR,       // Push stack(#1) | stack(#2), pop operands
      BUZZVM_INSTR_NOT,      // Push !stack(#1), pop operand
      BUZZVM_INSTR_EQ,       // Push stack(#1) == stack(#2), pop operands
      BUZZVM_INSTR_GT,       // Push stack(#1) > stack(#2), pop operands
      BUZZVM_INSTR_GTE,      // Push stack(#1) >= stack(#2), pop operands
      BUZZVM_INSTR_LT,       // Push stack(#1) < stack(#2), pop operands
      BUZZVM_INSTR_LTE,      // Push stack(#1) <= stack(#2), pop operands
      BUZZVM_INSTR_VSCREATE, // Create virtual stigmergy from integer id stack(#1), pop operands
      BUZZVM_INSTR_VSPUT,    // Put (key stack(#2),value stack(#1)) in virtual stigmergy stack(#3), pop operands
      BUZZVM_INSTR_VSGET,    // Push virtual stigmergy stack(#2) value of key stack(#1), pop operand
      /*
       * Opcodes with argument
       */
      /* Float argument */
      BUZZVM_INSTR_PUSHF,    // Push float constant onto stack
      /* Integer argument */
      BUZZVM_INSTR_PUSHI,    // Push integer constant onto stack
      BUZZVM_INSTR_DUP,      // Push variable in stack at given position (0 = top, >0 beneath)
      BUZZVM_INSTR_JUMP,     // Set PC to argument
      BUZZVM_INSTR_JUMPZ,    // Set PC to argument if stack top is zero
      BUZZVM_INSTR_JUMPNZ,   // Set PC to argument if stack top is not zero
      BUZZVM_INSTR_JUMPSUB,  // Push current PC and sets PC to argument
      BUZZVM_INSTR_CALL,     // Calls the C function pointed to by the argument
   } buzzvm_instr;
   static const char *buzzvm_instr_desc[] = {"nop", "done", "pop", "ret", "add", "sub", "mul", "div", "and", "or", "not", "eq", "gt", "gte", "lt", "lte", "vscreate", "vsput", "vsget", "pushf", "pushi", "dup", "jump", "jumpz", "jumpnz", "jumpsub", "call"};

   /*
    * Function pointer for BUZZVM_INSTR_CALL.
    * @param vm The VM data.
    * @return The updated VM state.
    */
   struct buzzvm_s;
   typedef int (*buzzvm_funp)(struct buzzvm_s* vm);

   /*
    * VM data
    */
   struct buzzvm_s {
      /* Bytecode content */
      const uint8_t* bcode;
      /* Size of the loaded bytecode */
      uint32_t bcode_size;
      /* Program counter */
      int32_t pc;
      /* Stack content */
      buzzdarray_t stack;
      /* Registered functions */
      buzzdarray_t flist;
      /* Input message FIFO */
      buzzdarray_t inmsglist;
      /* Output message FIFO */
      buzzdarray_t outmsglist;
      /* Virtual stigmergy maps */
      buzzdict_t vstigs;
      /* Current VM state */
      buzzvm_state state;
      /* Current VM error */
      buzzvm_error error;
   };
   typedef struct buzzvm_s* buzzvm_t;

   /*
    * Creates a new VM.
    * @return The VM data.
    */
   extern buzzvm_t buzzvm_new();

   /*
    * Resets the VM.
    * It brings the data of the VM back to what it was right after
    * initialization. It keeps the loaded bytecode and function list,
    * if any.
    * @param vm The VM data.
    */
   extern void buzzvm_reset(buzzvm_t vm);

   /*
    * Destroys the VM.
    * @param vm The VM data.
    */
   extern void buzzvm_destroy(buzzvm_t* vm);

   /*
    * Sets the bytecode in the VM.
    * The passed buffer cannot be deleted until the VM is done with it.
    * @param vm The VM data.
    * @param bcode_size The size (in bytes) of the bytecode.
    * @param bcode The bytecode buffer.
    */
   extern void buzzvm_set_bcode(buzzvm_t vm,
                                const uint8_t* bcode,
                                uint32_t bcode_size);

   /*
    * Executes the next step in the bytecode, if possible.
    * @param vm The VM data.
    * @return The updated VM state.
    */
   extern buzzvm_state buzzvm_step(buzzvm_t vm);

   /*
    * Registers a function in the VM.
    * @param vm The VM data.
    * @param funp The function pointer to register.
    * @return The id associated to the function, or -1 in case of error. A valid id must be used with BUZZVM_INSTR_CALL.
    */
   extern int64_t buzzvm_register_function(buzzvm_t vm,
                                           buzzvm_funp funp);

#ifdef __cplusplus
}
#endif

/*
 * Checks whether the given stack idx is valid.
 * If the idx is not valid, it updates the VM state and exits the current function.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 */
#define buzzvm_stack_assert(vm, idx) if(buzzvm_stack_top(vm) - (idx) < 0) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_STACK; return (vm)->state; }

/*
 * Returns the size of the stack.
 * The most recently pushed element in the stack is at top - 1.
 * @param vm The VM data.
 */
#define buzzvm_stack_top(vm) buzzdarray_size((vm)->stack)

/*
 * Returns the stack element at the passed index.
 * Does not perform any check on the validity of the index.
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 */
#define buzzvm_stack_at(vm, idx) (*buzzdarray_get((vm)->stack, (buzzvm_stack_top(vm) - idx), buzzvm_var_t))

/*
 * Terminates the current Buzz script.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_done(vm) (vm)->state = BUZZVM_STATE_DONE; return (vm)->state;

/*
 * Pushes a variable on the stack.
 * @param vm The VM data.
 * @param v The variable.
 */
#define buzzvm_push(vm, v) buzzdarray_push((vm)->stack, (v));

/*
 * Pushes a 32 bit signed int value on the stack.
 * @param vm The VM data.
 * @param v The value.
 */
#define buzzvm_pushi(vm, v) { buzzvm_var_t* var = (buzzvm_var_t*)buzzdarray_makeslot((vm)->stack, buzzvm_stack_top(vm)); var->i.type = BUZZTYPE_INT; var->i.value = (v); }

/*
 * Pushes a float value on the stack.
 * @param vm The VM data.
 * @param v The value.
 */
#define buzzvm_pushf(vm, v) { buzzvm_var_t* var = (buzzvm_var_t*)buzzdarray_makeslot((vm)->stack, buzzvm_stack_top(vm)); var->f.type = BUZZTYPE_FLOAT; var->f.value = (v); }

/*
 * Pops the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_pop(vm) if(buzzdarray_isempty((vm)->stack)) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_STACK; return (vm)->state; } buzzdarray_pop(vm->stack);

/*
 * Pushes the float value located at the given stack index.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 */
#define buzzvm_dup(vm, idx) buzzvm_stack_assert(vm, idx); buzzvm_pushf(vm, buzzvm_stack_at(vm, idx).f.value);

/*
 * Pops two float operands from the stack and pushes the result of a binary operation on them.
 * The order of the operation is stack(#1) oper stack(#2).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. + - * /
 */
#define buzzvm_binary_op_ff(vm, oper) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2).f.value = (buzzvm_stack_at(vm, 1).f.value oper buzzvm_stack_at(vm, 2).f.value); buzzdarray_pop(vm->stack);

/*
 * Pops two 32 bit unsigned int operands from the stack and pushes the result of a binary operation on them.
 * The order of the operation is stack(#1) oper stack(#2).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. & |
 */
#define buzzvm_binary_op_ii(vm, oper) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2).i.value = (buzzvm_stack_at(vm, 1).i.value oper buzzvm_stack_at(vm, 2).i.value); buzzdarray_pop(vm->stack);

/*
 * Pops two float operands from the stack and pushes the result of a binary operation on them as an integer.
 * The order of the operation is stack(#1) oper stack(#2).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. == > >= < <=
 */
#define buzzvm_binary_op_if(vm, oper) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2).i.type = BUZZTYPE_INT; buzzvm_stack_at(vm, 2).i.value = (buzzvm_stack_at(vm, 1).f.value oper buzzvm_stack_at(vm, 2).f.value); buzzdarray_pop(vm->stack);

/*
 * Pushes stack(#1) + stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_add(vm) buzzvm_binary_op_ff(vm, +);

/*
 * Pushes stack(#1) - stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_sub(vm) buzzvm_binary_op_ff(vm, -);

/*
 * Pushes stack(#1) * stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_mul(vm) buzzvm_binary_op_ff(vm, *);

/*
 * Pushes stack(#1) / stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_div(vm) buzzvm_binary_op_ff(vm, /);

/*
 * Pushes stack(#1) & stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_and(vm) buzzvm_binary_op_ii(vm, &);

/*
 * Pushes stack(#1) | stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_or(vm) buzzvm_binary_op_ii(vm, |);

/*
 * Negates the value currently at the top of the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_not(vm) buzzvm_stack_assert(vm, 1); buzzvm_stack_at(vm, 1).i.value = !buzzvm_stack_at(vm, 1).i.value;

/*
 * Pushes stack(#1) == stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_eq(vm) buzzvm_binary_op_if(vm, ==);

/*
 * Pushes stack(#1) > stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_gt(vm) buzzvm_binary_op_if(vm, >);

/*
 * Pushes stack(#1) >= stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_gte(vm) buzzvm_binary_op_if(vm, >=);

/*
 * Pushes stack(#1) < stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_lt(vm) buzzvm_binary_op_if(vm, <);

/*
 * Pushes stack(#1) <= stack(#2) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_lte(vm) buzzvm_binary_op_if(vm, <=);

/*
 * Calls the c function with the given identifier.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param fid The function id.
 */
#define buzzvm_call(vm, fid) if((fid) >= buzzdarray_size((vm)->flist)) {(vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_FLIST; return vm->state;} (*buzzdarray_get((vm)->flist, fid, buzzvm_funp))(vm);

#endif
