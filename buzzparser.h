#ifndef BUZZPARSER_H
#define BUZZPARSER_H

#include <buzzlex.h>
#include <buzzdarray.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

   typedef enum {
      BUZZNODE_SCRIPT = 0,
      BUZZNODE_BLOCK,
      BUZZNODE_STAT,
      BUZZNODE_FUNDEF,
      BUZZNODE_IF,
      BUZZNODE_FORLOOP,
      BUZZNODE_WHILELOOP,
      BUZZNODE_CONDITION,
      BUZZNODE_COMPARISON,
      BUZZNODE_EXPRESSION,
      BUZZNODE_PRODUCT,
      BUZZNODE_MODULO,
      BUZZNODE_POWER,
      BUZZNODE_OPERAND,
      BUZZNODE_COMMAND,
      BUZZNODE_IDREF
   } buzzptree_nodetype_e;

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
      /* Symbol table stack */
      buzzdarray_t symstack;
      /* Label counter */
      uint64_t labels;
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
