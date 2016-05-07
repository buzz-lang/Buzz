#ifndef BUZZVM_H
#define BUZZVM_H

#include <buzz/buzzheap.h>
#include <buzz/buzzstrman.h>
#include <buzz/buzzinmsg.h>
#include <buzz/buzzoutmsg.h>
#include <buzz/buzzvstig.h>
#include <buzz/buzzswarm.h>
#include <buzz/buzzneighbors.h>

#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

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
      BUZZVM_STATE_ERROR,      // Error occurred
      BUZZVM_STATE_STOPPED     // Stopped due to a breakpoint
   } buzzvm_state;
   extern const char *buzzvm_state_desc[];

   /*
    * VM error codes
    */
   typedef enum {
      BUZZVM_ERROR_NONE = 0, // No error
      BUZZVM_ERROR_INSTR,    // Unknown instruction 
      BUZZVM_ERROR_STACK,    // Empty stack
      BUZZVM_ERROR_LNUM,     // Wrong number of local variables
      BUZZVM_ERROR_PC,       // Program counter out of range
      BUZZVM_ERROR_FLIST,    // Function call id out of range
      BUZZVM_ERROR_TYPE,     // Type mismatch
      BUZZVM_ERROR_STRING,   // Unknown string id
      BUZZVM_ERROR_SWARM     // Unknown swarm id
   } buzzvm_error;
   extern const char *buzzvm_error_desc[];

   /*
    * VM instructions
    */
   typedef enum {
      /*
       * Opcodes without argument
       */
      BUZZVM_INSTR_NOP = 0,    // No operation
      BUZZVM_INSTR_DONE,       // End of the program
      BUZZVM_INSTR_PUSHNIL,    // Push nil onto stack
      BUZZVM_INSTR_DUP,        // Duplicate stack top
      BUZZVM_INSTR_POP,        // Pop value from stack
      BUZZVM_INSTR_RET0,       // Returns from closure call, see buzzvm_ret0()
      BUZZVM_INSTR_RET1,       // Returns from closure call, see buzzvm_ret1()
      BUZZVM_INSTR_ADD,        // Push stack(#1) + stack(#2), pop operands
      BUZZVM_INSTR_SUB,        // Push stack(#1) - stack(#2), pop operands
      BUZZVM_INSTR_MUL,        // Push stack(#1) * stack(#2), pop operands
      BUZZVM_INSTR_DIV,        // Push stack(#1) / stack(#2), pop operands
      BUZZVM_INSTR_MOD,        // Push stack(#1) % stack(#2), pop operands
      BUZZVM_INSTR_POW,        // Push stack(#1) ^ stack(#2), pop operands
      BUZZVM_INSTR_UNM,        // Push -stack(#1), pop operand
      BUZZVM_INSTR_AND,        // Push stack(#1) & stack(#2), pop operands
      BUZZVM_INSTR_OR,         // Push stack(#1) | stack(#2), pop operands
      BUZZVM_INSTR_NOT,        // Push !stack(#1), pop operand
      BUZZVM_INSTR_EQ,         // Push stack(#1) == stack(#2), pop operands
      BUZZVM_INSTR_NEQ,        // Push stack(#1) != stack(#2), pop operands
      BUZZVM_INSTR_GT,         // Push stack(#1) > stack(#2), pop operands
      BUZZVM_INSTR_GTE,        // Push stack(#1) >= stack(#2), pop operands
      BUZZVM_INSTR_LT,         // Push stack(#1) < stack(#2), pop operands
      BUZZVM_INSTR_LTE,        // Push stack(#1) <= stack(#2), pop operands
      BUZZVM_INSTR_GLOAD,      // Push global variable corresponding to string at stack #1, pop operand
      BUZZVM_INSTR_GSTORE,     // Store stack-top value into global variable at stack #2, pop operands
      BUZZVM_INSTR_PUSHT,      // Push empty table
      BUZZVM_INSTR_TPUT,       // Put key (stack(#2)), value (stack #1) in table (stack #3), pop key and value
      BUZZVM_INSTR_TGET,       // Push value for key (stack(#1)) in table (stack #2), pop key
      BUZZVM_INSTR_CALLC,      // Calls the closure on top of the stack as a normal closure
      BUZZVM_INSTR_CALLS,      // Calls the closure on top of the stack as a swarm closure
      /*
       * Opcodes with argument
       */
      /* Float argument */
      BUZZVM_INSTR_PUSHF,    // Push float constant onto stack
      /* Integer argument */
      BUZZVM_INSTR_PUSHI,    // Push integer constant onto stack
      BUZZVM_INSTR_PUSHS,    // Push string constant onto stack
      BUZZVM_INSTR_PUSHCN,   // Push native closure onto stack
      BUZZVM_INSTR_PUSHCC,   // Push c-function closure onto stack
      BUZZVM_INSTR_PUSHL,    // Push native closure lambda onto stack
      BUZZVM_INSTR_LLOAD,    // Push local variable at given position
      BUZZVM_INSTR_LSTORE,   // Store stack-top value into local variable at given position, pop operand
      BUZZVM_INSTR_JUMP,     // Set PC to argument
      BUZZVM_INSTR_JUMPZ,    // Set PC to argument if stack top is zero, pop operand
      BUZZVM_INSTR_JUMPNZ,   // Set PC to argument if stack top is not zero, pop operand
      BUZZVM_INSTR_COUNT     // Used to count how many instructions have been defined
   } buzzvm_instr;
   extern const char *buzzvm_instr_desc[];

   /*
    * Function pointer for BUZZVM_INSTR_CALL.
    * @param vm The VM data.
    * @return The updated VM state.
    */
   struct buzzvm_s;
   typedef int (*buzzvm_funp)(struct buzzvm_s* vm);

   /*
    * Data for local symbols
    */
   struct buzzvm_lsyms_s {
      /* The symbol list */
      buzzdarray_t syms;
      /* 1 if this is a swarm closure, 0 if not */
      uint8_t isswarm;
   };
   typedef struct buzzvm_lsyms_s* buzzvm_lsyms_t;

   /*
    * Creates a new local symbol table.
    * @param isswarm 0 if this is a normal symbol table, 1 if it's for a swarm closure call.
    * @param syms A list fo symbols.
    * @return A new local symbol table.
    */
   extern buzzvm_lsyms_t buzzvm_lsyms_new(uint8_t isswarm,
                                          buzzdarray_t syms);

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
      /* Current stack content */
      buzzdarray_t stack;
      /* Stack list */
      buzzdarray_t stacks;
      /* Current local variable table */
      buzzvm_lsyms_t lsyms;
      /* Local variable table list */
      buzzdarray_t lsymts;
      /* Global symbols */
      buzzdict_t gsyms;
      /* Strings */
      buzzstrman_t strings;
      /* Heap content */
      buzzheap_t heap;
      /* Registered functions */
      buzzdarray_t flist;
      /* List of known swarms */
      buzzdict_t swarms;
      /* List of known swarms */
      buzzdarray_t swarmstack;
      /* Swarm members */
      buzzswarm_members_t swarmmembers;
      /* Input message FIFO */
      buzzinmsg_queue_t inmsgs;
      /* Output message FIFO */
      buzzoutmsg_queue_t outmsgs;
      /* Virtual stigmergy maps */
      buzzdict_t vstigs;
      /* Neighbor value listeners */
      buzzdict_t listeners;
      /* Current VM state */
      buzzvm_state state;
      /* Current VM error */
      buzzvm_error error;
      /* Current VM error message */
      char* errormsg;
      /* Robot id */
      uint16_t robot;
   };
   typedef struct buzzvm_s* buzzvm_t;

   /*
    * Prints the current state of the VM.
    * @param vm The VM data.
    */
   extern void buzzvm_dump(buzzvm_t vm);

   /**
    * Returns a description of the current error state.
    * The returned string is created internally with malloc().
    * You should free() it after use.
    * @param vm The VM data.
    * @return A description of the current error state.
    */
   extern const char* buzzvm_strerror(buzzvm_t vm);

   /*
    * Creates a new VM.
    * @param robot The robot id.
    * @return The VM data.
    */
   extern buzzvm_t buzzvm_new(uint16_t robot);

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
    * @return 0 if everything OK, a non-zero value in case of error
    */
   extern int buzzvm_set_bcode(buzzvm_t vm,
                               const uint8_t* bcode,
                               uint32_t bcode_size);

   /*
    * Processes the input message queue.
    * @param vm The VM data.
    */
   extern void buzzvm_process_inmsgs(buzzvm_t vm);

   /*
    * Executes the next step in the bytecode, if possible.
    * @param vm The VM data.
    * @return The updated VM state.
    */
   extern buzzvm_state buzzvm_step(buzzvm_t vm);

   /*
    * Executes the script up to completion.
    * @param vm The VM data.
    * @return The updated VM state.
    */
   extern buzzvm_state buzzvm_execute_script(buzzvm_t vm);
   
   /*
    * Calls a Buzz closure.
    * It expects the stack to be as follows:
    * #1   arg1
    * #2   arg2
    * ...
    * #N   argN
    * #N+1 closure
    * This function pops all arguments.
    * @param vm The VM data.
    * @param argc The number of arguments.
    * @return 0 if everything OK, a non-zero value in case of error
    */
   extern buzzvm_state buzzvm_closure_call(buzzvm_t vm,
                                           uint32_t argc);

   /*
    * Calls a function defined in Buzz.
    * It expects the stack to be as follows:
    * #1 arg1
    * #2 arg2
    * ...
    * #N argN
    * This function pops all arguments.
    * @param vm The VM data.
    * @param fname The function name.
    * @param argc The number of arguments.
    * @return 0 if everything OK, a non-zero value in case of error
    */
   extern buzzvm_state buzzvm_function_call(buzzvm_t vm,
                                            const char* fname,
                                            uint32_t argc);

   /*
    * Registers a function in the VM.
    * @param vm The VM data.
    * @param funp The function pointer to register.
    * @return The function id.
    */
   extern uint32_t buzzvm_function_register(buzzvm_t vm,
                                            buzzvm_funp funp);

   /*
    * Calls a closure.
    * Internally checks whether the operation is valid.
    * This function expects the stack to be as follows:
    * #1   An integer for the number of closure parameters N
    * #2   Closure arg1
    * ...
    * #1+N Closure argN
    * #2+N The closure
    * This function pushes a new stack and a new local variable table filled with the
    * activation record entries and the closure arguments. In addition, it leaves the stack
    * beneath as follows:
    * #1 An integer for the return address
    * @param vm The VM data.
    * @param isswrm 0 for a normal closure, 1 for a swarm closure
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_call(buzzvm_t vm, int isswrm);

   /*
    * Pops the stack.
    * Internally checks whether the operation is valid.
    * @param vm The VM data.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pop(buzzvm_t vm);

   /*
    * Duplicates the current stack top.
    * @param vm The VM data.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_dup(buzzvm_t vm);

   /*
    * Pushes a variable on the stack.
    * @param vm The VM data.
    * @param v The variable.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_push(buzzvm_t vm, buzzobj_t v);

   /*
    * Pushes a userdata on the stack.
    * @param vm The VM data.
    * @param v The C pointer to the user data.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushu(buzzvm_t vm, void* v);

   /*
    * Pushes nil on the stack.
    * @param vm The VM data.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushnil(buzzvm_t vm);

   /*
    * Pushes a 32 bit signed int value on the stack.
    * @param vm The VM data.
    * @param v The value.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushi(buzzvm_t vm, int32_t v);

   /*
    * Pushes a float value on the stack.
    * @param vm The VM data.
    * @param v The value.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushf(buzzvm_t vm, float v);

   /*
    * Pushes a native closure on the stack.
    * Internally checks whether the operation is valid.
    * This function is designed to be used within int-returning functions such as
    * BuzzVM hook functions or buzzvm_step().
    * @param vm The VM data.
    * @param rfrnc The closure reference.
    * @param nat 1 if the closure in native, 0 if not
    * @param v The value.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushc(buzzvm_t vm, int32_t rfrnc, int32_t nat);

   /*
    * Pushes a string on the stack.
    * @param vm The VM data.
    * @param strid The string id.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushs(buzzvm_t vm, uint16_t strid);

   /*
    * Pushes a lambda native closure on the stack.
    * Internally checks whether the operation is valid.
    * This function is designed to be used within int-returning functions such as
    * BuzzVM hook functions or buzzvm_step().
    * @param vm The VM data.
    * @param addr The closure address.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_pushl(buzzvm_t vm, int32_t addr);

   /*
    * Stores a (idx,value) pair in a table.
    * Internally checks whether the operation is valid.
    * This function is designed to be used within int-returning functions such as
    * BuzzVM hook functions or buzzvm_step().
    * The stack is expected to be as follows:
    * #1 value
    * #2 idx
    * #3 table
    * This operation pops #1 and #2, leaving the table at the stack top.
    * @param vm The VM data.
    */
   extern buzzvm_state buzzvm_tput(buzzvm_t vm);
   
   /*
    * Fetches a (idx,value) pair from a table.
    * Internally checks whether the operation is valid.
    * This function is designed to be used within int-returning functions such as
    * BuzzVM hook functions or buzzvm_step().
    * The stack is expected to be as follows:
    * #1 idx
    * #2 table
    * This operation pops #1 and pushes the value, leaving the table at
    * stack #2. If the element for the given idx is not found, nil is
    * pushed as value.
    * @param vm The VM data.
    */
   extern buzzvm_state buzzvm_tget(buzzvm_t vm);

   /*
    * Pushes the global variable located at the given stack index.
    * Internally checks whether the operation is valid.
    * @param vm The VM data.
    */
   extern buzzvm_state buzzvm_gload(buzzvm_t vm);

   /*
    * Stores the object located at the stack top into the a global variable, pops operand.
    * Internally checks whether the operation is valid.
    * @param vm The VM data.
    * @param idx The local variable index.
    */
   extern buzzvm_state buzzvm_gstore(buzzvm_t vm);

   /*
    * Returns from a closure without setting a return value.
    * Internally checks whether the operation is valid.
    * This function expects at least two stacks to be present. The
    * first stack is popped. The stack beneath, now the top stack, is
    * expected to have at least one element: the return address at
    * #1. The return address is popped and used to update the program
    * counter.
    * @param vm The VM data.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_ret0(buzzvm_t vm);

   /*
    * Returns from a closure setting a return value.
    * Internally checks whether the operation is valid.
    * This function expects at least two stacks to be present. The
    * first stack must have at least one element, which is saved as
    * the return value of the call. The stack is then popped. The
    * stack beneath, now the top stack, is expected to have at least
    * one element: the return address at #1. The return address is
    * popped and used to update the program counter. Then, the saved
    * return value is pushed on the stack.
    * @param vm The VM data.
    * @return The VM state.
    */
   extern buzzvm_state buzzvm_ret1(buzzvm_t vm);

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
#define buzzvm_stack_assert(vm, idx) if(buzzvm_stack_top(vm) - (idx) < 0) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_STACK; asprintf(&(vm)->errormsg, "%s", buzzvm_error_desc[(vm)->error]); return (vm)->state; }

/*
 * Checks whether the type at the given stack position is correct.
 * If the type is wrong, it updates the VM state and exits the current function.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 * @param tpe The type to check
 */
#define buzzvm_type_assert(vm, idx, tpe) if(buzzvm_stack_at(vm, idx)->o.type != tpe) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_TYPE; asprintf(&(vm)->errormsg, "%s: expected %s, got %s", buzzvm_error_desc[(vm)->error], buzztype_desc[tpe], buzztype_desc[buzzvm_stack_at(vm, idx)->o.type]); return (vm)->state; }

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
#define buzzvm_stack_at(vm, idx) buzzdarray_get((vm)->stack, (buzzvm_stack_top(vm) - (idx)), buzzobj_t)

/*
 * Terminates the current Buzz script.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_done(vm) { (vm)->state = BUZZVM_STATE_DONE; return (vm)->state; }

/*
 * Pushes a native closure on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param addr The closure address.
 */
#define buzzvm_pushcn(vm, addr) buzzvm_pushc(vm, addr, 1)

/*
 * Pushes a c-function closure on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * This function expects the function id in the stack top.
 * It pops the function id and pushes the c-function closure.
 * @param vm The VM data.
 * @param cid The closure id.
 */
#define buzzvm_pushcc(vm, cid) buzzvm_pushc(vm, cid, 0)

/*
 * Returns the number of local variables in the current local symbol stack.
 * @param vm The VM data.
 */
#define buzzvm_lnum(vm) (buzzdarray_size((vm)->lsyms->syms)-1)

/*
 * Checks whether the current function was passed a certain number of parameters.
 * If the check fails, this function updates the VM state and exits the current function.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param num The number of expected local variables.
 */
#define buzzvm_lnum_assert(vm, num) if(buzzvm_lnum(vm) != num) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_LNUM; asprintf(&(vm)->errormsg, "%s: expected %d parameters, got %" PRId64, buzzvm_error_desc[(vm)->error], num, buzzvm_lnum(vm)); return (vm)->state; }

/*
 * Pushes the local variable located at the given stack index.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The local variable index.
 */
#define buzzvm_lload(vm, idx) buzzvm_push(vm, buzzdarray_get((vm)->lsyms->syms, idx, buzzobj_t));
   
/*
 * Stores the object located at the stack top into the a local variable, pops operand.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The local variable index.
 */
#define buzzvm_lstore(vm, idx) {                                  \
      buzzvm_stack_assert((vm), 1);                               \
      buzzobj_t o = buzzvm_stack_at(vm, 1);                       \
      buzzvm_pop(vm);                                             \
      buzzdarray_set((vm)->lsyms->syms, idx, &o);                 \
   }

/*
 * Pops two numeric operands from the stack and pushes the result of a binary arithmethic operation on them.
 * The order of the operation is stack(#2) oper stack(#1).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. & |
 */
#define buzzvm_binary_op_arith(vm, oper)                                \
   buzzvm_stack_assert((vm), 2);                                        \
   buzzobj_t op1 = buzzvm_stack_at(vm, 1);                              \
   buzzobj_t op2 = buzzvm_stack_at(vm, 2);                              \
   if((op1->o.type != BUZZTYPE_INT &&                                   \
       op1->o.type != BUZZTYPE_FLOAT) ||                                \
      (op2->o.type != BUZZTYPE_INT &&                                   \
       op2->o.type != BUZZTYPE_FLOAT))  {                               \
      (vm)->state = BUZZVM_STATE_ERROR;                                 \
      (vm)->error = BUZZVM_ERROR_TYPE;                                  \
      return (vm)->state;                                               \
   }                                                                    \
   buzzdarray_pop(vm->stack);                                           \
   buzzdarray_pop(vm->stack);                                           \
   if(op1->o.type == BUZZTYPE_INT &&                                    \
      op2->o.type == BUZZTYPE_INT) {                                    \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);        \
      res->i.value = op2->i.value oper op1->i.value;                    \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_INT &&                               \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = op2->f.value oper op1->i.value;                    \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_INT) {                               \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = op2->i.value oper op1->f.value;                    \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else {                                                               \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = op2->f.value oper op1->f.value;                    \
      buzzvm_push(vm, res);                                             \
   }

/*
 * Pops two operands from the stack and pushes the result of a binary logic operation on them.
 * The order of the operation is stack(#2) oper stack(#1).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. & |
 */
#define buzzvm_binary_op_logic(vm, oper)                                \
   buzzvm_stack_assert((vm), 2);                                        \
   buzzobj_t op1 = buzzvm_stack_at(vm, 1);                              \
   buzzobj_t op2 = buzzvm_stack_at(vm, 2);                              \
   buzzdarray_pop(vm->stack);                                           \
   buzzdarray_pop(vm->stack);                                           \
   buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);           \
   res->i.value =                                                       \
      !(op2->o.type == BUZZTYPE_NIL ||                                  \
        (op2->i.type == BUZZTYPE_INT && op2->i.value == 0))             \
      oper                                                              \
      !(op1->o.type == BUZZTYPE_NIL ||                                  \
        (op1->i.type == BUZZTYPE_INT && op1->i.value == 0));            \
   res->o.type = BUZZTYPE_INT;                                          \
   buzzvm_push(vm, res);

/*
 * Pops two numeric operands from the stack and pushes the result of a comparison operation on them.
 * The order of the operation is stack(#2) oper stack(#1).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. == > >= < <=
 */
#define buzzvm_binary_op_cmp(vm, oper)                                  \
   buzzvm_stack_assert((vm), 2);                                        \
   buzzobj_t op1 = buzzvm_stack_at(vm, 1);                              \
   buzzobj_t op2 = buzzvm_stack_at(vm, 2);                              \
   buzzdarray_pop(vm->stack);                                           \
   buzzdarray_pop(vm->stack);                                           \
   buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);           \
   res->i.value = (buzzobj_cmp(op2, op1) oper 0);                       \
   buzzvm_push(vm, res);

/*
 * Pushes stack(#2) + stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_add(vm) buzzvm_binary_op_arith(vm, +);

/*
 * Pushes stack(#2) - stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_sub(vm) buzzvm_binary_op_arith(vm, -);

/*
 * Pushes stack(#2) * stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_mul(vm) buzzvm_binary_op_arith(vm, *);

/*
 * Pushes stack(#2) / stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_div(vm) buzzvm_binary_op_arith(vm, /);

/*
 * Pushes stack(#2) % stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_mod(vm)                                                  \
   buzzvm_stack_assert((vm), 2);                                        \
   buzzobj_t op1 = buzzvm_stack_at(vm, 1);                              \
   buzzobj_t op2 = buzzvm_stack_at(vm, 2);                              \
   buzzdarray_pop(vm->stack);                                           \
   buzzdarray_pop(vm->stack);                                           \
   if(op1->o.type == BUZZTYPE_INT &&                                    \
      op2->o.type == BUZZTYPE_INT) {                                    \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);        \
      res->i.value = op2->i.value % op1->i.value;                       \
      if(res->i.value < 0) res->i.value += op1->i.value;                \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = fmodf(op2->f.value, op1->f.value);                 \
      if(res->f.value < 0.) res->f.value += op1->f.value;               \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_INT &&                               \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = fmodf(op2->f.value, op1->i.value);                 \
      if(res->f.value < 0.) res->f.value += op1->f.value;               \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_INT) {                               \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = fmodf(op2->i.value, op1->f.value);                 \
      if(res->f.value < 0.) res->f.value += op1->i.value;               \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else {                                                               \
      (vm)->state = BUZZVM_STATE_ERROR;                                 \
      (vm)->error = BUZZVM_ERROR_TYPE;                                  \
      return (vm)->state;                                               \
   }

/*
 * Pushes stack(#2) ^ stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_pow(vm)                                                  \
   buzzvm_stack_assert((vm), 2);                                        \
   buzzobj_t op1 = buzzvm_stack_at(vm, 1);                              \
   buzzobj_t op2 = buzzvm_stack_at(vm, 2);                              \
   buzzdarray_pop(vm->stack);                                           \
   buzzdarray_pop(vm->stack);                                           \
   if(op1->o.type == BUZZTYPE_INT &&                                    \
      op2->o.type == BUZZTYPE_INT) {                                    \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = powf(op2->i.value, op1->i.value);                  \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = powf(op2->f.value, op1->f.value);                  \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_INT &&                               \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = powf(op2->f.value, op1->i.value);                  \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_INT) {                               \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = powf(op2->i.value, op1->f.value);                  \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else {                                                               \
      (vm)->state = BUZZVM_STATE_ERROR;                                 \
      (vm)->error = BUZZVM_ERROR_TYPE;                                  \
      return (vm)->state;                                               \
   }

/*
 * Unary minus operator of the value currently at the top of the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_unm(vm)                                                  \
   buzzvm_stack_assert(vm, 1);                                          \
   buzzobj_t op = buzzvm_stack_at(vm, 1);                               \
   buzzdarray_pop(vm->stack);                                           \
   if(op->o.type == BUZZTYPE_INT) {                                     \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);        \
      res->i.value = -op->i.value;                                      \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op->o.type == BUZZTYPE_FLOAT) {                              \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = -op->f.value;                                      \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else {                                                               \
      (vm)->state = BUZZVM_STATE_ERROR;                                 \
      (vm)->error = BUZZVM_ERROR_TYPE;                                  \
      return (vm)->state;                                               \
   }

/*
 * Pushes stack(#2) & stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_and(vm) buzzvm_binary_op_logic(vm, &);

/*
 * Pushes stack(#2) | stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_or(vm) buzzvm_binary_op_logic(vm, |);

/*
 * Negates the value currently at the top of the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_not(vm)                                                  \
   buzzvm_stack_assert((vm), 1);                                        \
   buzzobj_t op = buzzvm_stack_at(vm, 1);                               \
   buzzdarray_pop(vm->stack);                                           \
   buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);           \
   res->i.value =                                                       \
      (op->o.type == BUZZTYPE_NIL ||                                    \
       (op->i.type == BUZZTYPE_INT && op->i.value == 0));               \
   buzzvm_push(vm, res);
   
/*
 * Pushes stack(#2) == stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_eq(vm) buzzvm_binary_op_cmp(vm, ==);

/*
 * Pushes stack(#2) != stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_neq(vm) buzzvm_binary_op_cmp(vm, !=);

/*
 * Pushes stack(#2) > stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_gt(vm) buzzvm_binary_op_cmp(vm, >);

/*
 * Pushes stack(#2) >= stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_gte(vm) buzzvm_binary_op_cmp(vm, >=);

/*
 * Pushes stack(#2) < stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_lt(vm) buzzvm_binary_op_cmp(vm, <);

/*
 * Pushes stack(#2) <= stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_lte(vm) buzzvm_binary_op_cmp(vm, <=);

/**
 * Calls a normal closure.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * This function expects the stack to be as follows:
 * #1   An integer for the number of closure parameters N
 * #2   Closure arg1
 * ...
 * #1+N Closure argN
 * #2+N The closure
 * This function pushes a new stack and a new local variable table filled with the
 * activation record entries and the closure arguments. In addition, it leaves the stack
 * beneath as follows:
 * #1 An integer for the return address
 */
#define buzzvm_callc(vm) buzzvm_call(vm, 0)

/**
 * Calls a swarm closure.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * This function expects the stack to be as follows:
 * #1   An integer for the number of closure parameters N
 * #2   Closure arg1
 * ...
 * #1+N Closure argN
 * #2+N The closure
 * This function pushes a new stack and a new local variable table filled with the
 * activation record entries and the closure arguments. In addition, it leaves the stack
 * beneath as follows:
 * #1 An integer for the return address
 */
#define buzzvm_calls(vm) buzzvm_call(vm, 1)

/*
 * Pushes an empty table onto the stack.
 * @param vm The VM data.
 */
#define buzzvm_pusht(vm) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_TABLE); buzzvm_push(vm, o); }

/*
 * Registers a string in the virtual machine.
 * @param vm The VM data.
 * @param str The string to register.
 */
#define buzzvm_string_register(vm, str) buzzstrman_register(vm->strings, str)

/*
 * Registers a string in the virtual machine.
 * @param vm The VM data.
 * @param sid The id of the string.
 */
#define buzzvm_string_get(vm, sid) buzzstrman_get(vm->strings, sid)

#endif
