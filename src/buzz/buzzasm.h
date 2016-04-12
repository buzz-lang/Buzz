#ifndef BUZZASM_H
#define BUZZASM_H

#include <buzz/buzzvm.h>
#include <buzz/buzzdebug.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Compiles an assembly file into bytecode.
    * @param fname The file name where the code is located.
    * @param buf The buffer in which the bytecode will be stored. Created internally.
    * @param size The size of the bytecode buffer.
    * @param dbg The debug data structure to fill into. Created internally.
    * @return 0 if no error occurred, 1 for I/O error, 2 for compilation error.
    */
   extern int buzz_asm(const char* fname,
                       uint8_t** buf,
                       uint32_t* size,
                       buzzdebug_t* dbg);

   /*
    * Decompiles bytecode into an assembly file.
    * @param buf The buffer in which the bytecode is stored.
    * @param size The size of the bytecode buffer.
    * @param dbg The debug data structure.
    * @param fname The file name where the assembly will be written.
    * @return 0 if no error occurred, 1 for I/O error, 2 for decompilation error.
    */
   extern int buzz_deasm(const uint8_t* buf,
                         uint32_t size,
                         buzzdebug_t dbg,
                         const char* fname);

   /*
    * Decompiles the bytecode of a single instruction.
    * This function writes the decompiled instruction into a new string pointed to by
    * *buf. When you're done with it, you must free it.
    * @param bcode The buffer in which the bytecode is stored.
    * @param off The offset at which decompilation must occur.
    * @param buf The bytecode where the 
    * @return 1 if no error occurred, 0 for decompilation error.
    */
   extern int buzz_instruction_deasm(const uint8_t* bcode,
                                     uint32_t off,
                                     char** buf);

#ifdef __cplusplus
}
#endif

#endif
