#ifndef BUZZSTRING_H
#define BUZZSTRING_H

#include <buzz/buzzvm.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Registers all the string functions.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_register(buzzvm_t vm);

   /**
    * Returns the length of a string.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_length(buzzvm_t vm);

   /**
    * Returns a substring of the given string.
    * Three signatures are possible:
    * - string.sub(s, n):
    *   Returns the substring starting at n (0 is the first character).
    * - string.sub(s, n, m):
    *   Returns the substring starting at n and ending at m.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_sub(buzzvm_t vm);

#ifdef __cplusplus
}
#endif

#endif
