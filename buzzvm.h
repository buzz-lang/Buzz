#ifndef BUZZVM_H
#define BUZZVM_H

#include <buzzheap.h>
#include <buzzinmsg.h>
#include <buzzoutmsg.h>
#include <buzzvstig.h>
#include <buzzswarm.h>
#include <buzzneighbors.h>
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
      BUZZVM_ERROR_STACK,    // Empty stack
      BUZZVM_ERROR_PC,       // Program counter out of range
      BUZZVM_ERROR_FLIST,    // Function call id out of range
      BUZZVM_ERROR_TYPE,     // Type mismatch
      BUZZVM_ERROR_STRING,   // Unknown string id
      BUZZVM_ERROR_SWARM     // Unknown swarm id
   } buzzvm_error;
   static const char *buzzvm_error_desc[] = { "none", "unknown instruction", "empty stack", "pc out of range", "function id out of range", "type mismatch", "unknown string id", "unknown swarm id" };

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
      BUZZVM_INSTR_GLOAD,      // Push global variable corresponding to string at stack #1, pop operand
      BUZZVM_INSTR_GSTORE,     // Store stack-top value into global variable at stack #2, pop operands
      BUZZVM_INSTR_PUSHT,      // Push empty table
      BUZZVM_INSTR_TPUT,       // Put key (stack(#2)), value (stack #1) in table (stack #3), pop key and value
      BUZZVM_INSTR_TGET,       // Push value for key (stack(#1)) in table (stack #2), pop key
      BUZZVM_INSTR_PUSHA,      // Push empty array
      BUZZVM_INSTR_APUT,       // Put idx (stack(#2)), value (stack #1) in array (stack #3), pop idx and value
      BUZZVM_INSTR_AGET,       // Push value for idx (stack(#1)) in array (stack #2), pop idx
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
   } buzzvm_instr;
   static const char *buzzvm_instr_desc[] = {"nop", "done", "pushnil", "pop", "ret0", "ret1", "add", "sub", "mul", "div", "mod", "pow", "unm", "and", "or", "not", "eq", "neq", "gt", "gte", "lt", "lte", "shout", "gload", "gstore", "pusht", "tput", "tget", "pusha", "aput", "aget", "callc", "calls", "pushf", "pushi", "pushs", "pushcn", "pushcc", "pushl", "lload", "lstore", "jump", "jumpz", "jumpnz"};

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
      buzzdarray_t strings;
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
      /* Current VM state */
      buzzvm_state state;
      /* Current VM error */
      buzzvm_error error;
      /* Robot id */
      uint16_t robot;
   };
   typedef struct buzzvm_s* buzzvm_t;

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
    * Executes the next step in the bytecode, if possible.
    * @param vm The VM data.
    * @return The updated VM state.
    */
   extern buzzvm_state buzzvm_step(buzzvm_t vm);

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
    * Registers a new string in the VM.
    * @param vm The VM data.
    * @param str The string to register.
    * @return The id of the string.
    */
   extern uint16_t buzzvm_string_register(buzzvm_t vm,
                                          const char* str);

   /*
    * Returns the string corresponding to the given id.
    * Returns NULL if the string is not found.
    * @param vm The VM data.
    * @param sid The string id.
    * @return The string data, or NULL if not found.
    */
   extern const char* buzzvm_string_get(buzzvm_t vm,
                                        uint16_t sid);
   
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
   extern int buzzvm_call(buzzvm_t vm, int isswrm);

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
 * Checks whether the type at the given stack position is correct.
 * If the type is wrong, it updates the VM state and exits the current function.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The stack index, where 0 is the stack top and >0 goes down the stack.
 * @param type The type to check
 */
#define buzzvm_type_assert(vm, idx, tpe) if(buzzvm_stack_at(vm, idx)->o.type != tpe) { (vm)->state = BUZZVM_STATE_ERROR; (vm)->error = BUZZVM_ERROR_TYPE; return (vm)->state; }

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
 * Pushes a userdata on the stack.
 * @param vm The VM data.
 * @param v The C pointer to the user data.
 */
#define buzzvm_pushuserdata(vm, v) { buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_USERDATA); o->u.value = (v); buzzvm_push(vm, o); }

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
 * Pushes a string on the stack.
 * @param vm The VM data.
 * @param strid The string id.
 */
#define buzzvm_pushs(vm, strid) {                                       \
      if((strid) >= buzzdarray_size((vm)->strings)) {                   \
         (vm)->state = BUZZVM_STATE_ERROR;                              \
         (vm)->error = BUZZVM_ERROR_STRING;                             \
         return (vm)->state;                                            \
      }                                                                 \
      buzzobj_t o = buzzheap_newobj((vm)->heap, BUZZTYPE_STRING);       \
      o->s.value.sid = (strid);                                         \
      o->s.value.str = buzzdarray_get((vm)->strings, (strid), char*);   \
      buzzvm_push(vm, o);                                               \
   }

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
 * @param vm The VM data.
 * @param rfrnc The closure reference.
 * @param nat 1 if the closure in native, 0 if not
 */
#define buzzvm_pushc(vm, rfrnc, nat) {                                \
      buzzobj_t o = buzzheap_newobj(((vm)->heap), BUZZTYPE_CLOSURE);  \
      o->c.value.isnative = (nat);                                    \
      o->c.value.ref = (rfrnc);                                       \
      buzzobj_t nil = buzzheap_newobj(vm->heap, BUZZTYPE_NIL);        \
      buzzdarray_push(o->c.value.actrec, &nil);                       \
      buzzvm_push(vm, o);                                             \
   }

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
 * Pushes a lambda native closure on the stack.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param addr The closure address.
 */
#define buzzvm_pushl(vm, addr) {                                      \
      buzzobj_t o = buzzheap_newobj(((vm)->heap), BUZZTYPE_CLOSURE);  \
      o->c.value.isnative = 1;                                        \
      o->c.value.ref = (addr);                                        \
      if((vm)->lsyms) {                                               \
         for(int i = 0; i < buzzdarray_size((vm)->lsyms->syms); ++i)  \
            buzzdarray_push(o->c.value.actrec,                        \
                            &buzzdarray_get((vm)->lsyms->syms,        \
                                            i, buzzobj_t));           \
      }                                                               \
      else {                                                          \
         buzzobj_t nil = buzzheap_newobj(vm->heap, BUZZTYPE_NIL);     \
         buzzdarray_push(o->c.value.actrec,                           \
                         &nil);                                       \
      }                                                               \
      buzzvm_push(vm, o);                                             \
   }

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
 * Pushes the global variable located at the given stack index.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The local variable index.
 */
#define buzzvm_gload(vm) {                                              \
      buzzvm_stack_assert((vm), 1);                                     \
      buzzvm_type_assert((vm), 1, BUZZTYPE_STRING);                     \
      buzzobj_t str = buzzvm_stack_at((vm), 1);                         \
      buzzvm_pop(vm);                                                   \
      buzzobj_t* o = buzzdict_get((vm)->gsyms, &(str->s.value), buzzobj_t); \
      if(!o) {                                                          \
         buzzvm_pushnil(vm);                                            \
      }                                                                 \
      else {                                                            \
         buzzvm_push(vm, (*o));                                         \
      }                                                                 \
   }

/*
 * Stores the object located at the stack top into the a global variable, pops operand.
 * Internally checks whether the operation is valid.
 * This function is designed to be used within int-returning functions such as
 * BuzzVM hook functions or buzzvm_step().
 * @param vm The VM data.
 * @param idx The local variable index.
 */
#define buzzvm_gstore(vm) {                                             \
      buzzvm_stack_assert((vm), 2);                                     \
      buzzvm_type_assert((vm), 2, BUZZTYPE_STRING);                     \
      buzzobj_t str = buzzvm_stack_at((vm), 2);                         \
      buzzobj_t o = buzzvm_stack_at((vm), 1);                           \
      buzzvm_pop(vm);                                                   \
      buzzvm_pop(vm);                                                   \
      buzzdict_set((vm)->gsyms, &(str->s.value), &o);                   \
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
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = op2->f.value oper op1->f.value;                    \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else {                                                               \
      (vm)->state = BUZZVM_STATE_ERROR;                                 \
      (vm)->error = BUZZVM_ERROR_TYPE;                                  \
      return (vm)->state;                                               \
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
   if(op1->o.type == BUZZTYPE_INT &&                                    \
      op2->o.type == BUZZTYPE_INT) {                                    \
      res->i.value = op2->i.value oper op1->i.value;                    \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_INT &&                               \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      res->i.value = op2->f.value oper op1->i.value;                    \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_INT) {                               \
      res->i.value = op2->i.value oper op1->f.value;                    \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      res->i.value = op2->f.value oper op1->f.value;                    \
   }                                                                    \
   else {                                                               \
      (vm)->state = BUZZVM_STATE_ERROR;                                 \
      (vm)->error = BUZZVM_ERROR_TYPE;                                  \
      return (vm)->state;                                               \
   }                                                                    \
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
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = fmodf(op2->f.value, op1->f.value);                 \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_INT &&                               \
           op2->o.type == BUZZTYPE_FLOAT) {                             \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = fmodf(op2->f.value, op1->i.value);                 \
      buzzvm_push(vm, res);                                             \
   }                                                                    \
   else if(op1->o.type == BUZZTYPE_FLOAT &&                             \
           op2->o.type == BUZZTYPE_INT) {                               \
      buzzobj_t res = buzzheap_newobj((vm)->heap, BUZZTYPE_FLOAT);      \
      res->f.value = fmodf(op2->i.value, op1->f.value);                 \
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
      if((vm)->lsyms->isswarm) {                                  \
         buzzdarray_pop((vm)->swarmstack);                        \
      }                                                           \
      buzzdarray_pop((vm)->lsymts);                               \
      (vm)->lsyms = !buzzdarray_isempty((vm)->lsymts) ?           \
         buzzdarray_last((vm)->lsymts, buzzvm_lsyms_t) :          \
         NULL;                                                    \
      buzzdarray_pop((vm)->stacks);                               \
      (vm)->stack = buzzdarray_last((vm)->stacks, buzzdarray_t);  \
      buzzvm_stack_assert(vm, 1);                                 \
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);                    \
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
#define buzzvm_ret1(vm) {                                         \
      if((vm)->lsyms->isswarm) {                                  \
         buzzdarray_pop((vm)->swarmstack);                        \
      }                                                           \
      buzzdarray_pop((vm)->lsymts);                               \
      (vm)->lsyms = !buzzdarray_isempty((vm)->lsymts) ?           \
         buzzdarray_last((vm)->lsymts, buzzvm_lsyms_t) :          \
         NULL;                                                    \
      buzzvm_stack_assert(vm, 1);                                 \
      buzzobj_t ret = buzzvm_stack_at(vm, 1);                     \
      buzzdarray_pop((vm)->stacks);                               \
      (vm)->stack = buzzdarray_last((vm)->stacks, buzzdarray_t);  \
      buzzvm_stack_assert(vm, 1);                                 \
      buzzvm_type_assert(vm, 1, BUZZTYPE_INT);                    \
      (vm)->pc = buzzvm_stack_at(vm, 1)->i.value;                 \
      buzzvm_pop(vm);                                             \
      buzzvm_push(vm, ret);                                       \
   }
   
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
#define buzzvm_tput(vm) {                                               \
      buzzvm_stack_assert(vm, 3);                                       \
      buzzvm_type_assert(vm, 3, BUZZTYPE_TABLE);                        \
      buzzobj_t v = buzzvm_stack_at(vm, 1);                             \
      buzzobj_t k = buzzvm_stack_at(vm, 2);                             \
      buzzobj_t t = buzzvm_stack_at(vm, 3);                             \
      buzzvm_pop(vm);                                                   \
      buzzvm_pop(vm);                                                   \
      buzzvm_pop(vm);                                                   \
      if(v->o.type == BUZZTYPE_CLOSURE) {                               \
         buzzobj_t o = buzzheap_newobj(((vm)->heap), BUZZTYPE_CLOSURE); \
         o->c.value.isnative = v->c.value.isnative;                     \
         o->c.value.ref = v->c.value.ref;                               \
         buzzdarray_push(o->c.value.actrec, &t);                        \
         for(int i = 1; i < buzzdarray_size(v->c.value.actrec); ++i)    \
            buzzdarray_push(o->c.value.actrec,                          \
                            &buzzdarray_get(v->c.value.actrec,          \
                                            i, buzzobj_t));             \
         buzzdict_set(t->t.value, &k, &o);                              \
      }                                                                 \
      else {                                                            \
         buzzdict_set(t->t.value, &k, &v);                              \
      }                                                                 \
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
      buzzvm_type_assert(vm, 2, BUZZTYPE_TABLE);              \
      buzzobj_t k = buzzvm_stack_at(vm, 1);                   \
      buzzobj_t t = buzzvm_stack_at(vm, 2);                   \
      buzzvm_pop(vm);                                         \
      buzzvm_pop(vm);                                         \
      buzzobj_t* v = buzzdict_get(t->t.value, &k, buzzobj_t); \
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

#endif
