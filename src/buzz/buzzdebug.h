#ifndef BUZZDEBUG_H
#define BUZZDEBUG_H

#include <buzz/buzzdict.h>

#ifdef __cplusplus
extern "C" {
#endif

   struct buzzdebuginfo_entry_s {
      uint64_t line;
      uint64_t col;
   };
   typedef struct buzzdebuginfo_entry_s* buzzdebuginfo_entry_t;

   /*
    * Definition of the buzz debug data structure.
    */
   typedef buzzdict_t buzzdebuginfo_t;

   /*
    * Creates a new debug structure.
    * @return A new debug structure.
    */
   buzzdebuginfo_t buzzdebuginfo_new();

   /*
    * Destroys the given debug structure.
    * @param dbg The debug structure.
    */
#define buzzdebuginfo_destroy(dbg) buzzdict_destroy(dbg)

   /*
    * Parses a debug info file and fills into the given data structure.
    * The debug structure is erased before being filled.
    * @param dbg The debug structure.
    * @param fname The file to read from.
    * @returns 1 if no error, 0 otherwise.
    */
   int buzzdebuginfo_fromfile(buzzdebuginfo_t dbg,
                              const char* fname);

   /*
    * Writes the content of a data structure to file.
    * The target file is truncated before being written into.
    * @param fname The file to write into.
    * @param dbg The debug structure.
    * @returns 1 if no error, 0 otherwise.
    */
   int buzzdebuginfo_tofile(const char* fname, buzzdebuginfo_t dbg);

   /*
    * Sets the given debug information for a specific bytecode offset.
    * @param dbg The debug data structure.
    * @param offset The bytecode offset.
    * @param line The script line number.
    * @param col The script column number.
    */
   void buzzdebuginfo_set(buzzdebuginfo_t dbg,
                          uint32_t offset,
                          uint64_t line,
                          uint64_t col);

   /*
    * Returns the debug data corresponding to the given offset.
    * @param dbg The debug data structure.
    * @param off The bytecode offset.
    * @return The debug data corresponding to the given offset.
    */
#define buzzdebuginfo_get(dbg, off) (*buzzdict_get(dbg, off, buzzdebuginfo_entry_t))

   /*
    * Returns the number of elements in the data structure.
    * @param dbg The debug data structure.
    */
#define buzzdebuginfo_size(dbg) buzzdict_size(dbg)

   /*
    * Returns 1 if the data structure is empty, 0 otherwise.
    * @param dbg The debug data structure.
    */
#define buzzdebuginfo_isempty(dbg) buzzdict_isempty(dbg)

   /*
    * Returns 1 if debug data at the offset exists, 0 otherwise.
    * @param dbg The debug data structure.
    * @param off The bytecode offset.
    */
#define buzzdebuginfo_exists(dbg, off) buzzdict_exists(dbg, off)

#ifdef __cplusplus
}
#endif

#endif
