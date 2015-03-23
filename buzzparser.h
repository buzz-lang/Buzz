#ifndef BUZZPARSER_H
#define BUZZPARSER_H

#include "buzzlex.h"

#ifdef __cplusplus
extern "C" {
#endif

   /* The parser state */
   struct buzzparser_s {
      /* The output assembler file name */
      char* fasm;
      /* The lexer */
      buzzlex_t* lex;
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
   extern int buzzparser_parse();

#ifdef __cplusplus
}
#endif

#endif
