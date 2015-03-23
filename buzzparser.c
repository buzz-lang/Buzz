#include "buzzparser.h"

/****************************************/
/****************************************/

buzzparser_t buzzparser_new(const char* fscript,
                            const char* fasm) {
   /* Create parser state */
   buzzparser_t retval = (buzzparser_t)malloc(sizeof(struct buzzparser_s));
   /* Create lexer */
   retval->lex = buzzlex_new(fscript);
   if(!retval->lex) {
      free(retval);
   }
   /* Copy string */
   retval->fasm = strdup(fasm);
   /* Return parser state */
   return retval;
}

/****************************************/
/****************************************/

void buzzparser_destroy(buzzparser_t* par) {
   free((*par)->fasm);
   buzzlex_destroy((*par)->lex);
   free(*par);
   *par = NULL;
}

/****************************************/
/****************************************/

int buzzparser_parse() {
   return 0;
}

/****************************************/
/****************************************/
