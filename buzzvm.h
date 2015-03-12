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
   BUZZVM_INSTR_NOP = 0, // No operation
   BUZZVM_INSTR_DONE,    // End of the program
   BUZZVM_INSTR_PUSH,    // Push constant onto stack
   BUZZVM_INSTR_AT,      // Push value in stack at given position (0 = top, >0 beneath)
   BUZZVM_INSTR_POP,     // Pop value from stack
   BUZZVM_INSTR_ADD,     // Push stack(#1) + stack(#2), pop operands
   BUZZVM_INSTR_SUB,     // Push stack(#1) - stack(#2), pop operands
   BUZZVM_INSTR_MUL,     // Push stack(#1) * stack(#2), pop operands
   BUZZVM_INSTR_DIV,     // Push stack(#1) / stack(#2), pop operands
   BUZZVM_INSTR_AND,     // Push stack(#1) & stack(#2), pop operands
   BUZZVM_INSTR_OR,      // Push stack(#1) | stack(#2), pop operands
   BUZZVM_INSTR_NOT,     // Push !stack(#1), pop operand
   BUZZVM_INSTR_EQ,      // Push stack(#1) == stack(#2), pop operands
   BUZZVM_INSTR_GT,      // Push stack(#1) > stack(#2), pop operands
   BUZZVM_INSTR_GTE,     // Push stack(#1) >= stack(#2), pop operands
   BUZZVM_INSTR_LT,      // Push stack(#1) < stack(#2), pop operands
   BUZZVM_INSTR_LTE,     // Push stack(#1) <= stack(#2), pop operands
   BUZZVM_INSTR_JUMP,    // Set PC to argument
   BUZZVM_INSTR_JUMPZ,   // Set PC to argument if stack top is zero
   BUZZVM_INSTR_JUMPNZ,  // Set PC to argument if stack top is not zero
   BUZZVM_INSTR_JUMPSUB, // Push current PC and sets PC to argument
   BUZZVM_INSTR_RET,     // Sets PC to value at stack top, then pops it
   BUZZVM_INSTR_CALL     // Calls the C function pointed to by the argument
} buzzvm_instr;
static char *buzzvm_instr_desc[] = {
   "nop", "done", "push", "at", "pop",
   "add", "sub", "mul", "div", "and",
   "or", "not", "eq", "gt", "gte",
   "lt", "lte", "jump", "jumpz", "jumpnz",
   "jumpsub", "ret", "call"
};

/*
 * An element in the VM stack
 */
typedef union {
   uint32_t i;
   float f;
} buzzvm_stack_elem;

/*
 * Function pointer for BUZZVM_INSTR_CALL.
 */
struct buzzvm_s;
typedef void (*buzzvm_funp)(struct buzzvm_s*);

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
 * @param stack_size The max size for the function list.
 * @return The VM data.
 */
extern buzzvm_t buzzvm_new(uint32_t stack_size,
                           uint32_t flist_size);

/*
 * Resets the VM.
 * It brings the data of the VM back to what it was right after
 * initialization. It keeps the loaded bytecode, if any.
 * @param vm The VM data.
 */
extern void buzzvm_reset(buzzvm_t vm);

/*
 * Destroys the VM.
 * @param vm The VM data.
 */
extern void buzzvm_destroy(buzzvm_t* vm);

/*
 * Compiles an assembly file into bytecode.
 * @param vm The VM data.
 * @param fname The file name where the code is located.
 * @param buf The buffer in which the bytecode will be stored. Created internally.
 * @param size The size of the created buffer.
 * @return 0 if no error occurred, 1 for I/O error, 2 for compilation error
 */
extern int buzzvm_asm(buzzvm_t vm,
                      const char* fname,
                      uint8_t** buf,
                      uint32_t* size);

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
 * @return The current VM state.
 */
extern buzzvm_state buzzvm_step(buzzvm_t vm);

/*
 * Registers a function in the VM.
 * @param vm The VM data.
 * @param funp The function pointer to register.
 * @return The id associated to the function, or -1 in case of error. A Valid id must be used with BUZZVM_INSTR_CALL.
 */
extern int64_t buzzvm_register_function(buzzvm_t vm,
                                        buzzvm_funp funp);

#endif
