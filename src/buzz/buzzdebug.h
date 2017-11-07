#ifndef BUZZDEBUG_H
#define BUZZDEBUG_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <buzz/buzzvm.h>
#include <buzz/buzzset.h>

#ifdef __cplusplus
extern "C" {
#endif

   struct buzzdebug_entry_s {
      /* Script line */
      uint64_t line;
      /* Script column */
      uint64_t col;
      /* Script file name */
      const char* fname;
   };
   typedef struct buzzdebug_entry_s* buzzdebug_entry_t;

   struct buzzdebug_fep_entry_s {
      /* Bytecode offset where the function starts */
      int64_t off;
      /* Function name, NULL for anonymous functions */
      char* fname;
      /* Parameter names */
      buzzdarray_t params;
   };

   /*
    * Definition of the buzz debug data structure.
    */
   struct buzzdebug_s {
      /* File name set */
      buzzset_t fnames;
      /* Offset -> script information */
      buzzdict_t off2script;
      /* Script -> offset information */
      buzzdict_t script2off;
      /* Function entry points */
      buzzdarray_t feps;
      /* Breakpoint list */
      buzzdarray_t breakpoints;
   };
   typedef struct buzzdebug_s* buzzdebug_t;

   /*
    * Creates a new debug structure.
    * @return A new debug structure.
    */
   extern buzzdebug_t buzzdebug_new();

   /*
    * Destroys the given debug structure.
    * @param dbg The debug structure.
    */
   extern void buzzdebug_destroy(buzzdebug_t* dbg);

   /*
    * Parses a debug info file and fills into the given data structure.
    * The debug structure is erased before being filled.
    * @param dbg The debug structure.
    * @param fname The file to read from.
    * @returns 1 if no error, 0 otherwise.
    */
   extern int buzzdebug_fromfile(buzzdebug_t dbg,
                                 const char* fname);

   /*
    * Writes the content of a debug data structure to file.
    * The target file is truncated before being written into.
    * @param fname The file to write into.
    * @param dbg The debug structure.
    * @returns 1 if no error, 0 otherwise.
    */
   extern int buzzdebug_tofile(const char* fname,
                               buzzdebug_t dbg);

   /*
    * Sets the given debug information for a specific bytecode offset.
    * @param dbg The debug data structure.
    * @param offset The bytecode offset.
    * @param line The script line number.
    * @param col The script column number.
    * @param fname The file name.
    */
   extern void buzzdebug_info_set(buzzdebug_t dbg,
                                  int32_t offset,
                                  uint64_t line,
                                  uint64_t col,
                                  const char* fname);

   /*
    * Retrieves a pointer to the offset corresponding to the given script position.
    * If nothing was found, NULL is returned.
    * @param dbg The debug data structure.
    * @param line The line in the script.
    * @param col The column in the script.
    * @param fname The file name in the script.
    * @return A pointer to the value of the found offset, or NULL.
    */
   extern const int32_t* buzzdebug_info_get_fromscript(buzzdebug_t dbg,
                                                       uint64_t line,
                                                       uint64_t col,
                                                       const char* fname);
   
   /*
    * Sets a breakpoint at the given offset.
    * @param dbg The debug data structure.
    * @param idx The wanted breakpoint offset.
    */
   extern void buzzdebug_breakpoint_set_offset(buzzdebug_t dbg,
                                               int32_t off);

   /*
    * Returns 1 if a breakpoint exists at the given offset.
    * @param dbg The debug data structure.
    * @param idx The index of the wanted breakpoint.
    * @return 1 if a breakpoint exists at the given offset.
    */
   extern int buzzdebug_breakpoint_exists(buzzdebug_t dbg,
                                          int32_t off);

   /*
    * Executes a script up to completion, stopping for beakpoints.
    * @param vm The VM data.
    * @param dbg The debug data structure.
    * @return The updated VM state.
    */
   extern buzzvm_state buzzdebug_continue(buzzvm_t vm,
                                          buzzdebug_t dbg);

   /*
    * Calls a Buzz closure, stopping for beakpoints.
    * It expects the stack to be as follows:
    * #1   arg1
    * #2   arg2
    * ...
    * #N   argN
    * #N+1 closure
    * This function pops all arguments.
    * @param vm The VM data.
    * @param argc The number of arguments.
    * @param dbg The debug data structure.
    * @return 0 if everything OK, a non-zero value in case of error
    */
   extern buzzvm_state buzzdebug_closure_call(buzzvm_t vm,
                                              uint32_t argc,
                                              buzzdebug_t dbg);

   /*
    * Calls a function defined in Buzz, stopping for beakpoints.
    * It expects the stack to be as follows:
    * #1 arg1
    * #2 arg2
    * ...
    * #N argN
    * This function pops all arguments.
    * @param vm The VM data.
    * @param fname The function name.
    * @param argc The number of arguments.
    * @param dbg The debug data structure.
    * @return 0 if everything OK, a non-zero value in case of error
    */
   extern buzzvm_state buzzdebug_function_call(buzzvm_t vm,
                                               const char* fname,
                                               uint32_t argc,
                                               buzzdebug_t dbg);

   /**
    * Dumps the current state of a stack.
    * The index goes from 1 to (vm->stacks->size).
    * The currently active stack is at index 1.
    * @param vm The VM data.
    * @param idx The index of the stack to dump.
    * @param stream The output stream.
    */
   extern void buzzdebug_stack_dump(buzzvm_t vm,
                                    uint32_t idx,
                                    FILE* stream);

   /**
    * Prints the backtrace (i.e., the call stack).
    * @param vm The VM data.
    * @param dbg The debug data structure.
    * @param stream The output stream.
    */
   extern void buzzdebug_backtrace(buzzvm_t vm,
                                   buzzdebug_t dbg,
                                   FILE* stream);

#ifdef __cplusplus
}
#endif

/*
 * Returns the debug data corresponding to the given offset.
 * This function returns NULL if the offset is not stored in the structure.
 * @param dbg The debug data structure.
 * @param off The bytecode offset.
 * @return The debug data corresponding to the given offset.
 */
#define buzzdebug_info_get_fromoffset(dbg, off) buzzdict_get(dbg->off2script, off, buzzdebug_entry_t)

/*
 * Returns the number of elements in the data structure.
 * @param dbg The debug data structure.
 */
#define buzzdebug_info_count(dbg) buzzdict_size(dbg->off2script)

/*
 * Returns 1 if the data structure is empty, 0 otherwise.
 * @param dbg The debug data structure.
 */
#define buzzdebug_info_isempty(dbg) buzzdict_isempty(dbg->off2script)

/*
 * Returns 1 if debug data at the offset exists, 0 otherwise.
 * @param dbg The debug data structure.
 * @param off The bytecode offset.
 */
#define buzzdebug_info_exists_offset(dbg, off) buzzdict_exists(dbg->off2script, off)

/*
 * Returns the offset of a breakpoint given its index.
 * @param dbg The debug data structure.
 * @param idx The index of the wanted breakpoint.
 */
#define buzzdebug_breakpoint_get_offset(dbg, idx) buzzdarray_get(dbg->breakpoints, idx, int32_t)

/*
 * Removes a breakpoint given its index.
 * @param dbg The debug data structure.
 * @param idx The index of the wanted breakpoint.
 */
#define buzzdebug_breakpoint_remove(dbg, idx) buzzdarray_remove(dbg->breakpoints, idx)

/*
 * Removes all breakpoints.
 * @param dbg The debug data structure.
 */
#define buzzdebug_breakpoint_clear(dbg) buzzdarray_clear(dbg->breakpoints, 5)

/*
 * Returns the number of breakpoints currently set.
 * @param dbg The debug data structure.
 */
#define buzzdebug_breakpoint_count(dbg) buzzdarray_size(dbg->breakpoints)

#endif
