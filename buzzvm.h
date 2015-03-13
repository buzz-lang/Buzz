#ifndef BUZZVM_H
#define BUZZVM_H

#include <stdint.h>

/*
 * VM states
 */
typedef enum {
   BUZZVM_STATE_NOCODE = 0, // No code loaded
   BUZZVM_STATE_READY,      // Ready to execute next instruction
   BUZZVM_STATE_DONE,       // Program finished
   BUZZVM_STATE_ERROR       // Error occurred
} buzzvm_state;
static char *buzzvm_state_desc[] = { "no code", "ready", "done", "error" };

/*
 * VM error codes
 */
typedef enum {
   BUZZVM_ERROR_NONE = 0, // No error
   BUZZVM_ERROR_STACK,    // Stack operation out of range
   BUZZVM_ERROR_PC,       // Program counter out of range
   BUZZVM_ERROR_FLIST     // Function call id out of range
} buzzvm_error;
static char *buzzvm_error_desc[] = { "none", "stack out of range", "pc out of range", "function id out of range" };

/*
 * VM instructions
 */
typedef enum {
   /*
    * Opcodes without argument
    */
   BUZZVM_INSTR_NOP = 0, // No operation
   BUZZVM_INSTR_DONE,    // End of the program
   BUZZVM_INSTR_POP,     // Pop value from stack
   BUZZVM_INSTR_RET,     // Sets PC to value at stack top, then pops it
   BUZZVM_INSTR_ADD,     // Push stack(#1) + stack(#2), pop operands
   BUZZVM_INSTR_SUB,     // Push stack(#1) - stack(#2), pop operands
   BUZZVM_INSTR_MUL,     // Push stack(#1) * stack(#2), pop operands
   BUZZVM_INSTR_DIV,     // Push stack(#1) / stack(#2), pop operands
   BUZZVM_INSTR_AND,     // Push stack(#1) & stack(#2), pop operands
   BUZZVM_INSTR_OR,      // Push stack(#1) | stack(#2), pop operands
   BUZZVM_INSTR_NOT,     // Push !stack(#1), pop operand
   BUZZVM_INSTR_EQ,      // Push stack(#1) == stack(#2), pop operands
   BUZZVM_INSTR_GT,      // Push stack(#1) > stack(#2), pop operands
   BUZZVM_INSTR_GTE,     // Push stack(#1) >= stack(#2), pop oper
   BUZZVM_INSTR_LT,      // Push stack(#1) < stack(#2), pop operands
   BUZZVM_INSTR_LTE,     // Push stack(#1) <= stack(#2), pop operands
   /*
    * Opcodes with argument
    */
   /* Float argument */
   BUZZVM_INSTR_PUSH,    // Push float constant onto stack
   /* Integer argument */
   BUZZVM_INSTR_DUP,     // Push float value in stack at given position (0 = top, >0 beneath)
   BUZZVM_INSTR_JUMP,    // Set PC to argument
   BUZZVM_INSTR_JUMPZ,   // Set PC to argument if stack top is zero
   BUZZVM_INSTR_JUMPNZ,  // Set PC to argument if stack top is not zero
   BUZZVM_INSTR_JUMPSUB, // Push current PC and sets PC to argument
   BUZZVM_INSTR_CALL     // Calls the C function pointed to by the argument
} buzzvm_instr;
static char *buzzvm_instr_desc[] = {"nop", "done", "pop", "ret", "add", "sub", "mul", "div", "and", "or", "not", "eq", "gt", "gte", "lt", "lte", "push", "dup", "jump", "jumpz", "jumpnz", "jumpsub", "call"};

/*
 * An element in the VM stack
 */
typedef union {
   uint32_t i;
   float f;
} buzzvm_stack_elem;

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
   /* Max size of the stack */
   uint32_t stack_size;
   /* Size of the loaded bytecode */
   uint32_t bcode_size;
   /* Max size for the function list */
   uint32_t flist_size;
   /* Current stack top */
   int64_t stack_top;
   /* Program counter */
   int64_t pc;
   /* Numbers of currently registered functions */
   uint32_t flist_entries;
   /* Stack content */
   buzzvm_stack_elem* stack;
   /* Bytecode content */
   const uint8_t* bcode;
   /* Registered functions */
   buzzvm_funp* flist;
   /* Current VM state */
   buzzvm_state state;
   /* Current VM error */
   buzzvm_error error;
};
typedef struct buzzvm_s* buzzvm_t;

/*
 * Creates a new VM.
 * @param stack_size The max size of the stack.
 * @param flist_size The max size for the function list.
 * @return The VM data.
 */
extern buzzvm_t buzzvm_new(uint32_t stack_size,
                           uint32_t flist_size);

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

/*
 * Checks whether the given stack idx is valid.
 * If the idx is not valid, it updates the VM state and exits the current function.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 */
#define buzzvm_assert_stack(vm, idx) if(((vm)->stack_top - (idx)) < 0 || ((vm)->stack_top - (idx)) >= (vm)->stack_size) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_STACK; return (vm)->state; }

/*
 * Returns the stack element at the passed index.
 * Does not perform any check on the validity of the index.
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 */
#define buzzvm_at(vm, idx) (vm)->stack[((vm)->stack_top - idx)]

/*
 * Terminates the current Buzz script.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_done(vm) (vm)->state = BUZZVM_STATE_DONE; return (vm)->state;

/*
 * Pushes a float value on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param v The value.
 */
#define buzzvm_pushf(vm, v) buzzvm_assert_stack(vm, 0); buzzvm_at(vm, 0).f = (v); ++((vm)->stack_top);

/*
 * Pushes a 32 bit unsigned int value on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param v The value.
 */
#define buzzvm_pushi(vm, v) buzzvm_assert_stack(vm, 0); buzzvm_at(vm, 0).i = (v); ++((vm)->stack_top);

/*
 * Pops the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_pop(vm) --((vm)->stack_top); buzzvm_assert_stack(vm, 0);

/*
 * Pushes the float value located at the given stack index.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 */
#define buzzvm_dup(vm, idx) buzzvm_assert_stack(vm, idx); buzzvm_pushf(vm, buzzvm_at(vm, idx).f);

/*
 * Pops two float operands from the stack and pushes the result of a binary operation on them.
 * The order of the operation is stack(#1) oper stack(#2).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. + - * /
 */
#define buzzvm_binary_op_ff(vm, oper) buzzvm_assert_stack((vm), 2); --(vm)->stack_top; buzzvm_at(vm, 1).f = (buzzvm_at(vm, 0).f oper buzzvm_at(vm, 1).f);

/*
 * Pops two 32 bit unsigned int operands from the stack and pushes the result of a binary operation on them.
 * The order of the operation is stack(#1) oper stack(#2).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. & |
 */
#define buzzvm_binary_op_ii(vm, oper) buzzvm_assert_stack((vm), 2); --(vm)->stack_top; buzzvm_at(vm, 1).i = (buzzvm_at(vm, 0).i oper buzzvm_at(vm, 1).i);

/*
 * Pops two float operands from the stack and pushes the result of a binary operation on them as 32 bit unsigned int.
 * The order of the operation is stack(#1) oper stack(#2).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. == > >= < <=
 */
#define buzzvm_binary_op_if(vm, oper) buzzvm_assert_stack((vm), 2); --(vm)->stack_top; buzzvm_at(vm, 1).i = (buzzvm_at(vm, 0).f oper buzzvm_at(vm, 1).f);

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
#define buzzvm_not(vm) buzzvm_assert_stack(vm, 1); buzzvm_at(vm, 1).i = !buzzvm_at(vm, 1).i;

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
#define buzzvm_call(vm, fid) if((fid) >= (vm)->flist_entries) {(vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_FLIST; return vm->state;} (vm)->flist[(fid)](vm);

#endif
