#ifndef BUZZPARSER_H
#define BUZZPARSER_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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
    * In this function the arguments must be in the following order:
    * [0] IGNORED The current command name.
    * [1] REQUIRED The input script file name.
    * [2] REQUIRED The output assembler file name.
    * [3] OPTIONAL The symbol table file name.
    * @param argv The number of command line arguments
    * @param argv The list of command line arguments
    * @return The parser state.
    */
   extern buzzparser_t buzzparser_new(int argc,
                                      char** argv);

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
