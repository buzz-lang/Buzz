#ifndef BUZZDEBUG_H
#define BUZZDEBUG_H

#include <buzz/buzzdict.h>

#ifdef __cplusplus
extern "C" {
#endif

   struct buzzdebug_entry_s {
      uint64_t line;
      uint64_t col;
      char* fname;
   };
   typedef struct buzzdebug_entry_s* buzzdebug_entry_t;

   /*
    * Definition of the buzz debug data structure.
    */
   struct buzzdebug_s {
      buzzdict_t off2script;
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
   extern void buzzdebug_off2script_set(buzzdebug_t dbg,
                                        uint32_t offset,
                                        uint64_t line,
                                        uint64_t col,
                                        const char* fname);

   /*
    * Sets a breakpoint at the given offset.
    * @param dbg The debug data structure.
    * @param idx The index of the wanted breakpoint.
    */
   extern void buzzdebug_breakpoint_set_offset(buzzdebug_t dbg,
                                               int32_t off);

#ifdef __cplusplus
}
#endif

/*
 * Returns the debug data corresponding to the given offset.
 * @param dbg The debug data structure.
 * @param off The bytecode offset.
 * @return The debug data corresponding to the given offset.
 */
#define buzzdebug_off2script_get(dbg, off) (*buzzdict_get(dbg->off2script, off, buzzdebug_entry_t))

/*
 * Returns the number of elements in the data structure.
 * @param dbg The debug data structure.
 */
#define buzzdebug_off2script_size(dbg) buzzdict_size(dbg->off2script)

/*
 * Returns 1 if the data structure is empty, 0 otherwise.
 * @param dbg The debug data structure.
 */
#define buzzdebug_off2script_isempty(dbg) buzzdict_isempty(dbg->off2script)

/*
 * Returns 1 if debug data at the offset exists, 0 otherwise.
 * @param dbg The debug data structure.
 * @param off The bytecode offset.
 */
#define buzzdebug_off2script_exists(dbg, off) buzzdict_exists(dbg->off2script, off)

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
#define buzzdebug_breakpoint_num(dbg) buzzdarray_size(dbg->breakpoints)

#endif
