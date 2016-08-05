#ifndef BUZZPARSER_H
#define BUZZPARSER_H

#define _GNU_SOURCE
#include <buzz/buzzlex.h>
#include <buzz/buzzdarray.h>
#include <buzz/buzzdict.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

   /* Forward declaration to contain a code chunk */
   struct chunk_s;

   /* The parser state */
   struct buzzparser_s {
      /* The output assembler file name */
      char* asmfn;
      /* The output assembler file stream */
      FILE* asmstream;
      /* The lexer */
      buzzlex_t lex;
      /* The last fetched token */
      buzztok_t tok;
      /* The list of chunks */
      buzzdarray_t chunks;
      /* The current chunk being written */
      struct chunk_s* chunk;
      /* Symbol table stack */
      buzzdarray_t symstack;
      /* The top of the symbol table stack */
      buzzdict_t syms;
      /* List of string symbols */
      buzzdict_t strings;
      /* Label counter */
      uint32_t labels;
   };
   typedef struct buzzparser_s* buzzparser_t;

   /*
    * Creates a new parser.
    * @param fscript The input script file name.
    * @param fasm The output assembler file name.
    * @return The parser state.
    */
   extern buzzparser_t buzzparser_new(const char* fscript,
                                      const char* fasm);

   /*
    * Destroys the parser.
    * @param par The parser.
    */
   extern void buzzparser_destroy(buzzparser_t* par);

   /*
    * Parses the script.
    * @return 1 if successful, 0 in case of error
    */
   extern int buzzparser_parse(buzzparser_t par);

#ifdef __cplusplus
}
#endif

#endif
