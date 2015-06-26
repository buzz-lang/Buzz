#ifndef BUZZASM_H
#define BUZZASM_H

#include "buzzvm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Compiles an assembly file into bytecode.
 * @param fname The file name where the code is located.
 * @param buf The buffer in which the bytecode will be stored. Created internally.
 * @param size The size of the bytecode buffer.
 * @return 0 if no error occurred, 1 for I/O error, 2 for compilation error.
 */
extern int buzz_asm(const char* fname,
                    uint8_t** buf,
                    uint32_t* size);

/*
 * Decompiles bytecode into an assembly file.
 * @param buf The buffer in which the bytecode is stored.
 * @param size The size of the bytecode buffer.
 * @param fname The file name where the assembly will be written.
 * @return 0 if no error occurred, 1 for I/O error, 2 for decompilation error.
 */
extern int buzz_deasm(const uint8_t* buf,
                      uint32_t size,
                      const char* fname);

#ifdef __cplusplus
}
#endif

#endif
