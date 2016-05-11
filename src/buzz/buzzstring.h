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
    * Two signatures are possible:
    * - string.sub(s, n):
    *   Returns the substring starting at n (0 is the first character).
    * - string.sub(s, n, m):
    *   Returns the substring starting at n and ending at m.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_sub(buzzvm_t vm);

   /**
    * Returns a new string which is the concatenation of the given strings.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_concat(buzzvm_t vm);

   /**
    * Transforms an object into a string.
    * It only works with int and floats. With other objects, nil is
    * returned.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_tostring(buzzvm_t vm);

   /**
    * Transforms a string into int.
    * If the conversion fails, nil is returned.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_toint(buzzvm_t vm);

   /**
    * Transforms a string into float.
    * If the conversion fails, nil is returned.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzstring_tofloat(buzzvm_t vm);

#ifdef __cplusplus
}
#endif

#endif
