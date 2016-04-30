#ifndef BUZZIO_H
#define BUZZIO_H

#include <buzz/buzzvm.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Registers all the I/O functions.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzio_register(buzzvm_t vm);

   /*
    * Opens a file.
    * Expects two parameters: the filename, and the mode.
    * The can be:
    * - "w" for writing
    * - "r" for reading
    * - "a" for appending
    * In Buzz, this function returns a table that contains the state
    * of the file and methods such as close(), size(), foreach(), etc.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzio_fopen(buzzvm_t vm);

   /*
    * Closes a file.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzio_fclose(buzzvm_t vm);

   /*
    * Returns the file size.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzio_fsize(buzzvm_t vm);

   /*
    * Executes a function for each line of the file.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzio_fforeach(buzzvm_t vm);

   /*
    * Writes the given string on file.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzio_fwrite(buzzvm_t vm);

#ifdef __cplusplus
}
#endif

#endif
