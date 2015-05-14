#ifndef BUZZVM_H
#define BUZZVM_H

#include <buzzmsg.h>
#include <buzzvstig.h>
#include <buzzheap.h>
#include <math.h>

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
      BUZZVM_INSTR_NOP = 0,    // No operation
      BUZZVM_INSTR_DONE,       // End of the program
      BUZZVM_INSTR_PUSHNIL,    // Push nil onto stack
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
      BUZZVM_INSTR_SHOUT,      // Broadcast variable stack(#1), pop operands
      BUZZVM_INSTR_PUSHT,      // Push empty table
      BUZZVM_INSTR_TPUT,       // Put key (stack(#2)), value (stack #1) in table (stack #3), pop key and value
      BUZZVM_INSTR_TGET,       // Push value for key (stack(#1)) in table (stack #2), pop key
      BUZZVM_INSTR_PUSHA,      // Push empty array
      BUZZVM_INSTR_APUT,       // Put idx (stack(#2)), value (stack #1) in array (stack #3), pop idx and value
      BUZZVM_INSTR_AGET,       // Push value for idx (stack(#1)) in array (stack #2), pop idx
      BUZZVM_INSTR_VSCREATE,   // Create virtual stigmergy from integer id stack(#1), pop operands
      BUZZVM_INSTR_VSPUT,      // Put (key stack(#2),value stack(#1)) in virtual stigmergy stack(#3), pop operands
      BUZZVM_INSTR_VSGET,      // Push virtual stigmergy stack(#2) value of key stack(#1), pop operand
      BUZZVM_INSTR_CALLCN,     // Calls the native closure on top of the stack
      BUZZVM_INSTR_CALLCC,     // Calls the c-function closure on top the stack
      BUZZVM_INSTR_PUSHCN,     // Push native closure, pop operands (jump addr, argnum stack(#2), args (stack #3-#(3+argnum)))
      BUZZVM_INSTR_PUSHCC,     // Push c-function closure, pop operands (ref, argnum stack(#2), args (stack #3-#(3+argnum)))
      BUZZVM_INSTR_JOINSWARM,  // Joins the swarm with id at stack #1, pops operand
      BUZZVM_INSTR_LEAVESWARM, // Leaves the swarm with id at stack #1, pops operand
      BUZZVM_INSTR_INSWARM,    // Checks whether robot is in swarm at stack #1, pushes 1 for true or 0 for false, pops operand
      /*
       * Opcodes with argument
       */
      /* Float argument */
      BUZZVM_INSTR_PUSHF,    // Push float constant onto stack
      /* Integer argument */
      BUZZVM_INSTR_PUSHI,    // Push integer constant onto stack
      BUZZVM_INSTR_LLOAD,    // Push local variable at given position
      BUZZVM_INSTR_LSTORE,   // Store stack-top value into local variable at given position, pop operand
      BUZZVM_INSTR_JUMP,     // Set PC to argument
      BUZZVM_INSTR_JUMPZ,    // Set PC to argument if stack top is zero, pop operand
      BUZZVM_INSTR_JUMPNZ,   // Set PC to argument if stack top is not zero, pop operand
   } buzzvm_instr;
   static const char *buzzvm_instr_desc[] = {"nop", "done", "pushnil", "pop", "ret0", "ret1", "add", "sub", "mul", "div", "mod", "pow", "unm", "and", "or", "not", "eq", "neq", "gt", "gte", "lt", "lte", "shout", "pusht", "tput", "tget", "pusha", "aput", "aget", "vscreate", "vsput", "vsget", "callcn", "callcc", "pushcn", "pushcc", "joinswarm", "leaveswarm", "inswarm", "pushf", "pushi", "lload", "lstore", "jump", "jumpz", "jumpnz", "jumpsub"};

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
      /* Current stack content */
      buzzdarray_t stack;
      /* Stack list */
      buzzdarray_t stacks;
      /* Heap content */
      buzzheap_t heap;
      /* Registered functions */
      buzzdarray_t flist;
      /* List of known swarms */
      buzzdict_t swarms;
      /* Input message FIFO */
      buzzmsg_queue_t inmsgs;
      /* Output message FIFO */
      buzzmsg_queue_t outmsgs;
      /* Virtual stigmergy maps */
      buzzdict_t vstigs;
      /* Current VM state */
      buzzvm_state state;
      /* Current VM error */
      buzzvm_error error;
      /* Robot id */
      uint32_t robot;
   };
   typedef struct buzzvm_s* buzzvm_t;

   /*
    * Creates a new VM.
    * @param robot The robot id.
    * @return The VM data.
    */
   extern buzzvm_t buzzvm_new(uint32_t robot);

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
#define buzzvm_stack_at(vm, idx) buzzdarray_get((vm)->stack, (buzzvm_stack_top(vm) - (idx)), buzzobj_t)

/*
 * Terminates the current Buzz script.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_done(vm) { (vm)->state = BUZZVM_STATE_DONE; return (vm)->state; }

/*
 * Pops the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_pop(vm) if(buzzdarray_isempty((vm)->stack)) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_STACK; return (vm)->state; } buzzdarray_pop(vm->stack);

/*
 * Pushes a variable on the stack.
 * @param vm The VM data.
 * @param v The variable.
 */
#define buzzvm_push(vm, v) buzzdarray_push((vm)->stack, &(v))

/*
 * Pushes nil on the stack.
 * @param vm The VM data.
 */
#define buzzvm_pushnil(vm) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_NIL); buzzvm_push(vm, o); }

/*
 * Pushes a 32 bit signed int value on the stack.
 * @param vm The VM data.
 * @param v The value.
 */
#define buzzvm_pushi(vm, v) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_INT); o->i.value = (v); buzzvm_push(vm, o); }

/*
 * Pushes a float value on the stack.
 * @param vm The VM data.
 * @param v The value.
 */
#define buzzvm_pushf(vm, v) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT); o->f.value = (v); buzzvm_push(vm, o); }

/*
 * Pushes a native closure on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * Expects the following objects in the stack:
 * #1   jump address
 * #2   number of arguments N
 * #3   arg1
 * ...
 * #2+N argN
 * This function pops all the above elements and leaves a native closure
 * object on the stack top.
 * @param vm The VM data.
 */
#define buzzvm_pushcn(vm) {                                             \
      buzzvm_stack_assert(vm, 2);                                       \
      buzzobj_t o = buzzheap_newobj(((vm)->heap), BUZZTYPE_CLOSURE);    \
      o->c.isnative = 1;                                                \
      o->c.value.native.addr = buzzvm_stack_at(vm, 1)->i.value;         \
      int32_t nargs = buzzvm_stack_at(vm, 2)->i.value;                  \
      buzzvm_pop(vm);                                                   \
      buzzvm_pop(vm);                                                   \
      buzzvm_stack_assert(vm, nargs);                                   \
      for(int i = 0; i < nargs; ++i) {                                  \
         buzzdarray_push(o->c.value.native.actrec, &buzzvm_stack_at(vm, 1)); \
         buzzvm_pop(vm);                                                \
      }                                                                 \
      buzzvm_push(vm, o);                                               \
   }

/*
 * Pushes a c-function closure on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * Expects the following objects in the stack:
 * #1   function id
 * #2   number of arguments N
 * #3   arg1
 * ...
 * #2+N argN
 * This function pops all the above elements and leaves a c-function closure
 * object on the stack top.
 * @param vm The VM data.
 */
#define buzzvm_pushcc(vm) {                                             \
      buzzvm_stack_assert(vm, 2);                                       \
      buzzobj_t o = buzzheap_newobj(((vm)->heap), BUZZTYPE_CLOSURE);    \
      o->c.isnative = 0;                                                \
      o->c.value.cfun.id = buzzvm_stack_at(vm, 1)->i.value;             \
      int32_t nargs = buzzvm_stack_at(vm, 2)->i.value;                  \
      buzzvm_pop(vm);                                                   \
      buzzvm_pop(vm);                                                   \
      buzzvm_stack_assert(vm, nargs);                                   \
      for(int i = 0; i < nargs; ++i) {                                  \
         buzzdarray_push(o->c.value.cfun.actrec, &buzzvm_stack_at(vm, 1)); \
         buzzvm_pop(vm);                                                \
      }                                                                 \
      buzzvm_push(vm, o);                                               \
   }

/*
 * Pushes the object located at the given stack index.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes up the stack.
 */
#define buzzvm_lload(vm, idx) {                                         \
      if((idx) >= buzzdarray_size((vm)->stack)) {                       \
         (vm)->state = BUZZVM_STATE_ERROR;                              \
         (vm)->error = BUZZVM_ERROR_STACK;                              \
         return (vm)->state;                                            \
      }                                                                 \
      buzzvm_push(vm, buzzdarray_get((vm)->stack, (idx), buzzobj_t));   \
   }

/*
 * Stores the object located at the stack top into the given stack index, pops operand.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes up the stack.
 */
#define buzzvm_lstore(vm, idx) {                                         \
      if((idx) >= buzzdarray_size((vm)->stack)) {                       \
         (vm)->state = BUZZVM_STATE_ERROR;                              \
         (vm)->error = BUZZVM_ERROR_STACK;                              \
         return (vm)->state;                                            \
      }                                                                 \
      buzzdarray_set((vm)->stack,                                       \
                     (idx),                                             \
                     buzzvm_stack_at((vm), buzzvm_stack_top(vm)));      \
      buzzvm_pop(vm);                                                   \
   }

/*
 * Pops two float operands from the stack and pushes the result of a binary operation on them.
 * The order of the operation is stack(#2) oper stack(#1).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. + - * /
 */
#define buzzvm_binary_op_ff(vm, oper) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2)->f.value = (buzzvm_stack_at(vm, 2)->f.value oper buzzvm_stack_at(vm, 1)->f.value); buzzdarray_pop(vm->stack);

/*
 * Pops two 32 bit unsigned int operands from the stack and pushes the result of a binary operation on them.
 * The order of the operation is stack(#2) oper stack(#1).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. & |
 */
#define buzzvm_binary_op_ii(vm, oper) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2)->i.value = (buzzvm_stack_at(vm, 2)->i.value oper buzzvm_stack_at(vm, 1)->i.value); buzzdarray_pop(vm->stack);

/*
 * Pops two float operands from the stack and pushes the result of a binary operation on them as an integer.
 * The order of the operation is stack(#2) oper stack(#1).
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param oper The binary operation, e.g. == > >= < <=
 */
#define buzzvm_binary_op_if(vm, oper) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2)->i.type = BUZZTYPE_INT; buzzvm_stack_at(vm, 2)->i.value = (buzzvm_stack_at(vm, 2)->f.value oper buzzvm_stack_at(vm, 1)->f.value); buzzdarray_pop(vm->stack);

/*
 * Pushes stack(#2) + stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_add(vm) buzzvm_binary_op_ff(vm, +);

/*
 * Pushes stack(#2) - stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_sub(vm) buzzvm_binary_op_ff(vm, -);

/*
 * Pushes stack(#2) * stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_mul(vm) buzzvm_binary_op_ff(vm, *);

/*
 * Pushes stack(#2) / stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_div(vm) buzzvm_binary_op_ff(vm, /);

/*
 * Pushes stack(#2) % stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_mod(vm) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2)->f.value = fmodf(buzzvm_stack_at(vm, 2)->f.value, buzzvm_stack_at(vm, 1)->f.value); buzzdarray_pop(vm->stack);

/*
 * Pushes stack(#2) ^ stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_pow(vm) buzzvm_stack_assert((vm), 2); buzzvm_stack_at(vm, 2)->f.value = powf(buzzvm_stack_at(vm, 2)->f.value, buzzvm_stack_at(vm, 1)->f.value); buzzdarray_pop(vm->stack);

/*
 * Unary minus operator of the value currently at the top of the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_unm(vm) buzzvm_stack_assert(vm, 1); buzzvm_stack_at(vm, 1)->f.value = -buzzvm_stack_at(vm, 1)->f.value;

/*
 * Pushes stack(#2) & stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_and(vm) buzzvm_binary_op_ii(vm, &);

/*
 * Pushes stack(#2) | stack(#1) and pops the operands.
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
#define buzzvm_not(vm) buzzvm_stack_assert(vm, 1); buzzvm_stack_at(vm, 1)->i.value = !buzzvm_stack_at(vm, 1)->i.value;

/*
 * Pushes stack(#2) == stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_eq(vm) buzzvm_binary_op_if(vm, ==);

/*
 * Pushes stack(#2) != stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_neq(vm) buzzvm_binary_op_if(vm, !=);

/*
 * Pushes stack(#2) > stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_gt(vm) buzzvm_binary_op_if(vm, >);

/*
 * Pushes stack(#2) >= stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_gte(vm) buzzvm_binary_op_if(vm, >=);

/*
 * Pushes stack(#2) < stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_lt(vm) buzzvm_binary_op_if(vm, <);

/*
 * Pushes stack(#2) <= stack(#1) and pops the operands.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_lte(vm) buzzvm_binary_op_if(vm, <=);

/*
 * Returns from a closure without setting a return value.
 * Internally checks whether the operation is valid.  This function is
 * designed to be used within int-returning functions such as BuzzVM
 * hook functions or buzzvm_step().  This function expects at least
 * two stacks to be present. The first stack is popped. The stack
 * beneath, now the top stack, is expected to have at least one
 * element: the return address at #1. The return address is popped and
 * used to update the program counter.
 */
#define buzzvm_ret0(vm) {                                         \
      buzzdarray_pop((vm)->stacks);                               \
      (vm)->stack = buzzdarray_last((vm)->stacks, buzzdarray_t);  \
      buzzvm_stack_assert(vm, 1);                                 \
      (vm)->pc = buzzvm_stack_at(vm, 1)->i.value;                 \
      buzzvm_pop(vm);                                             \
   }

/*
 * Returns from a closure setting a return value.
 * Internally checks whether the operation is valid.  This function is
 * designed to be used within int-returning functions such as BuzzVM
 * hook functions or buzzvm_step().
 * This function expects at least two stacks to be present. The first
 * stack must have at least one element, which is saved as the return
 * value of the call. The stack is then popped. The stack beneath, now
 * the top stack, is expected to have at least one element: the return
 * address at #1. The return address is popped and used to update the
 * program counter. Then, the saved return value is pushed on the
 * stack.
 */
#define buzzvm_ret1(vm) {                                               \
                         buzzobj_t ret = buzzvm_stack_at(vm, 1);        \
                         buzzdarray_pop((vm)->stacks);                  \
                         (vm)->stack = buzzdarray_last((vm)->stacks, buzzdarray_t); \
                         buzzvm_stack_assert(vm, 1);                    \
                         (vm)->pc = buzzvm_stack_at(vm, 1)->i.value;    \
                         buzzvm_pop(vm);                                \
                         buzzvm_push(vm, ret);                          \
                         }

/**
 * Calls a native closure.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * This function expects the stack to be as follows:
 * #1   An integer for the number of closure parameters N
 * #2   Closure arg1
 * ...
 * #1+N Closure argN
 * #2+N The closure
 * This function pushes a new stack filled with the activation record entries
 * and the closure arguments. In addition, it leaves the stack beneath as follows:
 * #1 An integer for the return address
 */
#define buzzvm_callcn(vm) {                                           \
      buzzvm_stack_assert(vm, 1);                                     \
      int32_t argn = buzzvm_stack_at(vm, 1)->i.value;                 \
      buzzvm_pop(vm);                                                 \
      buzzvm_stack_assert(vm, argn+1);                                \
      buzzobj_t c = buzzvm_stack_at(vm, argn+1);                      \
      buzzdarray_t prstack = (vm)->stack;                             \
      (vm)->stack = buzzdarray_clone(c->c.value.native.actrec);       \
      buzzdarray_push((vm)->stacks, &((vm)->stack));                  \
      for(int32_t i = argn; i > 0; --i)                               \
         buzzvm_push(vm,                                              \
                     buzzdarray_get(prstack,                          \
                                    buzzdarray_size(prstack) - i,     \
                                    buzzobj_t));                      \
      for(int32_t i = argn+1; i > 0; --i)                             \
         buzzdarray_pop(prstack);                                     \
      buzzobj_t retaddr = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);  \
      retaddr->i.value = (vm)->pc;                                    \
      buzzdarray_push(prstack, &retaddr);                             \
      (vm)->pc = c->c.value.native.addr;                              \
   }

/**
 * Calls a c-function closure.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * This function expects the stack to be as follows:
 * #1   An integer for the number of closure parameters N
 * #2   Closure arg1
 * ...
 * #1+N Closure argN
 * #2+N The closure
 * This function pushes a new stack filled with the activation record entries
 * and the closure arguments. In addition, it leaves the stack beneath as follows:
 * #1 An integer for the return address
 */
#define buzzvm_callcc(vm) {                                             \
      buzzvm_stack_assert(vm, 1);                                       \
      int32_t argn = buzzvm_stack_at(vm, 1)->i.value;                   \
      buzzvm_pop(vm);                                                   \
      buzzobj_t c = buzzvm_stack_at(vm, argn+1);                        \
      if((c->c.value.cfun.id) >= buzzdarray_size((vm)->flist)) {        \
         (vm)->state = BUZZVM_STATE_ERROR;                              \
         (vm)->error = BUZZVM_ERROR_FLIST;                              \
         return vm->state;                                              \
      }                                                                 \
      buzzvm_stack_assert(vm, argn);                                    \
      buzzdarray_t prstack = (vm)->stack;                               \
      (vm)->stack = buzzdarray_clone(c->c.value.cfun.actrec);           \
      buzzdarray_push((vm)->stacks, &((vm)->stack));                    \
      for(int32_t i = argn; i > 0; --i)                                 \
         buzzvm_push(vm,                                                \
                     buzzdarray_get(prstack,                            \
                                    buzzdarray_size(prstack) - i,       \
                                    buzzobj_t));                        \
      for(int32_t i = argn+1; i > 0; --i)                               \
         buzzdarray_pop(prstack);                                       \
      buzzobj_t retaddr = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);    \
      retaddr->i.value = (vm)->pc;                                      \
      buzzdarray_push(prstack, &retaddr);                               \
      buzzdarray_get((vm)->flist, c->c.value.cfun.id, buzzvm_funp)(vm); \
   }

/*
 * Pushes an empty table onto the stack.
 * @param vm The VM data.
 */
#define buzzvm_pusht(vm) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_TABLE); buzzvm_push(vm, o); }

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
#define buzzvm_tput(vm) {                       \
      buzzvm_stack_assert(vm, 3);               \
      buzzobj_t v = buzzvm_stack_at(vm, 1);     \
      buzzobj_t k = buzzvm_stack_at(vm, 2);     \
      buzzobj_t t = buzzvm_stack_at(vm, 3);     \
      buzzdict_set(t->t.value, &k, &v);         \
      buzzvm_pop(vm);                           \
      buzzvm_pop(vm);                           \
   }

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
#define buzzvm_tget(vm) {                                     \
      buzzvm_stack_assert(vm, 2);                             \
      buzzobj_t  k = buzzvm_stack_at(vm, 1);                  \
      buzzobj_t  t = buzzvm_stack_at(vm, 2);                  \
      buzzobj_t* v = buzzdict_get(t->t.value, &k, buzzobj_t); \
      buzzvm_pop(vm);                                         \
      if(v) buzzvm_push(vm, *v);                              \
      else buzzvm_pushnil(vm);                                \
   }

/*
 * Pushes an empty array onto the stack.
 * @param vm The VM data.
 */
#define buzzvm_pusha(vm) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_ARRAY); buzzvm_push(vm, o); }

/*
 * Stores a (idx,value) pair in a array.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * The stack is expected to be as follows:
 * #1 value
 * #2 idx
 * #3 array
 * This operation pops #1 and #2, leaving the array at the stack top.
 * @param vm The VM data.
 */
#define buzzvm_aput(vm) {                         \
      buzzvm_stack_assert(vm, 3);                 \
      buzzobj_t v = buzzvm_stack_at(vm, 1);       \
      buzzobj_t i = buzzvm_stack_at(vm, 2);       \
      buzzobj_t a = buzzvm_stack_at(vm, 3);       \
      buzzdarray_set(a->a.value, i->i.value, &v); \
      buzzvm_pop(vm);                             \
      buzzvm_pop(vm);                             \
   }

/*
 * Fetches a (idx,value) pair from a array.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * The stack is expected to be as follows:
 * #1 idx
 * #2 array
 * This operation pops #1 and pushes the value, leaving the array at
 * stack #2. If the element for the given idx is not found, nil is
 * pushed as value.
 * @param vm The VM data.
 */
#define buzzvm_aget(vm) {                                               \
      buzzvm_stack_assert(vm, 2);                                       \
      buzzobj_t i = buzzvm_stack_at(vm, 1);                             \
      buzzobj_t a = buzzvm_stack_at(vm, 2);                             \
      buzzvm_pop(vm);                                                   \
      if(i->i.value < buzzdarray_size(a->a.value)) {                    \
         buzzobj_t v = buzzdarray_get(a->a.value, i->i.value, buzzobj_t); \
         buzzvm_push(vm, v);                                            \
      }                                                                 \
      else buzzvm_pushnil(vm);                                          \
   }

/*
 * Joins the swarm indicated by the id at stack #1. Pops the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_joinswarm(vm) {                        \
      buzzvm_stack_assert(vm, 1);                     \
      buzzobj_t i = buzzvm_stack_at(vm, 1);           \
      buzzvm_pop(vm);                                 \
      uint8_t v = 1;                                  \
      buzzdict_set((vm)->swarms, &(i->i.value), &v);  \
   }

/*
 * Joins the swarm indicated by the id at stack #1. Pops the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_leaveswarm(vm) {                       \
      buzzvm_stack_assert(vm, 1);                     \
      buzzobj_t i = buzzvm_stack_at(vm, 1);           \
      buzzvm_pop(vm);                                 \
      uint8_t v = 0;                                  \
      buzzdict_set((vm)->swarms, &(i->i.value), &v);  \
   }

/*
 * Checks whether this robot is part of the swarm indicated by the id
 * at stack #1. Pops the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 */
#define buzzvm_inswarm(vm) {                                            \
      buzzvm_stack_assert(vm, 1);                                       \
      buzzobj_t i = buzzvm_stack_at(vm, 1);                             \
      buzzvm_pop(vm);                                                   \
      uint8_t* v = buzzdict_get((vm)->swarms, &(i->i.value), uint8_t);  \
      buzzobj_t r = buzzheap_newobj((vm)->heap, BUZZTYPE_INT);          \
      r->i.value = (v && (*v));                                         \
      buzzvm_push((vm), r);                                             \
   }

#endif
