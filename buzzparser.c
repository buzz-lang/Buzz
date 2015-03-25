#include "buzzparser.h"
#include <stdio.h>
#include <stdlib.h>

static int PARSE_ERROR = 0;
static int PARSE_OK    = 1;

/****************************************/
/****************************************/

#define fetchtok()                                              \
   buzzlex_destroytok(&par->tok);                               \
   par->tok = buzzlex_nexttok(par->lex);

int match(buzzparser_t par,
          buzztok_type_e type) {
   if(!par->tok) {
      fprintf(stderr,
              "%s: Syntax error: expected token, found EOFn",
              par->lex->fname);
      return PARSE_ERROR;
   }
   if(par->tok->type != type) {
      fprintf(stderr,
              "%s:%llu:%llu: Syntax error: expected '%s'\n",
              par->lex->fname,
              par->tok->line,
              par->tok->col,
              buzztok_desc[type]);
      return PARSE_ERROR;
   }
   else {
      fetchtok();
      return PARSE_OK;
   }
}

#define tokmatch(tok) if(!match(par, tok)) return PARSE_ERROR;

/****************************************/
/****************************************/

int parse_script(buzzparser_t par);

int parse_block(buzzparser_t par);
int parse_blockrest(buzzparser_t par);

int parse_statlist(buzzparser_t par);
int parse_statrest(buzzparser_t par);
int parse_stat(buzzparser_t par);

int parse_fun(buzzparser_t par);

int parse_if(buzzparser_t par);
int parse_endif(buzzparser_t par);

int parse_for(buzzparser_t par);

int parse_while(buzzparser_t par);

int parse_condition(buzzparser_t par);
int parse_conditionrest(buzzparser_t par);
int parse_comparison(buzzparser_t par);
int parse_comparisonrest(buzzparser_t par);
int parse_comparand(buzzparser_t par);
int parse_conditionlist(buzzparser_t par);
int parse_conditionlistrest(buzzparser_t par);

int parse_expression(buzzparser_t par);
int parse_expressionrest(buzzparser_t par);
int parse_product(buzzparser_t par);
int parse_productrest(buzzparser_t par);
int parse_modulo(buzzparser_t par);
int parse_modulorest(buzzparser_t par);
int parse_power(buzzparser_t par);
int parse_powerrest(buzzparser_t par);
int parse_operand(buzzparser_t par);

int parse_command(buzzparser_t par);
int parse_commandrest(buzzparser_t par);

int parse_idref(buzzparser_t par);
int parse_idrefrest(buzzparser_t par);
int parse_idlist(buzzparser_t par);
int parse_idlistrest(buzzparser_t par);

/****************************************/
/****************************************/

int parse_script(buzzparser_t par) {
   par->tok = buzzlex_nexttok(par->lex);
   if(!par->tok) {
      fprintf(stderr,
              "%s: Syntax error: expected token, found EOFn",
              par->lex->fname);
      return PARSE_ERROR;
   }
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_block(buzzparser_t par) {
   tokmatch(BUZZTOK_BLOCKOPEN);
   return parse_blockrest(par);
}

int parse_blockrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      fetchtok();
      return PARSE_OK;
   }
   else {
      if(parse_statlist(par)) {
         tokmatch(BUZZTOK_BLOCKCLOSE);
         return PARSE_OK;
      }
      else {
         return PARSE_ERROR;
      }
   }
}

/****************************************/
/****************************************/

int parse_statlist(buzzparser_t par) {
   return parse_stat(par) && parse_statrest(par);
}

int parse_statrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      return PARSE_OK;
   }
   else {
      return parse_statlist(par);
   }
}

int parse_stat(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_STATEND)
      fetchtok();
      return PARSE_OK;
   if(par->tok->type == BUZZTOK_FUN)
      return parse_fun(par);
   if(par->tok->type == BUZZTOK_IF)
      return parse_if(par);
   if(par->tok->type == BUZZTOK_FOR)
      return parse_for(par);
   if(par->tok->type == BUZZTOK_WHILE)
      return parse_while(par);
   return parse_command(par);
}

/****************************************/
/****************************************/

int parse_fun(buzzparser_t par) {
   tokmatch(BUZZTOK_FUN);
   tokmatch(BUZZTOK_ID);
   tokmatch(BUZZTOK_PAROPEN);
   if(!parse_idlist(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_if(buzzparser_t par) {
   tokmatch(BUZZTOK_IF);
   tokmatch(BUZZTOK_PAROPEN);
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   if(!parse_block(par)) return PARSE_ERROR;
   return parse_endif(par);
}

int parse_endif(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_ELSE) {
      fetchtok();
      return parse_block(par);
   }
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_for(buzzparser_t par) {
   tokmatch(BUZZTOK_FOR);
   tokmatch(BUZZTOK_PAROPEN);
   if(!parse_idref(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_ASSIGN);
   if(!parse_expression(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_LISTSEP);
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_LISTSEP);
   if(!parse_idref(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_ASSIGN);
   if(!parse_expression(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_while(buzzparser_t par) {
   tokmatch(BUZZTOK_WHILE);
   tokmatch(BUZZTOK_PAROPEN);
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_condition(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_NOT) {
      fetchtok();
      return parse_condition(par);
   }
   else {
      if(!parse_comparison(par))
         return PARSE_ERROR;
      return parse_conditionrest(par);
   }
}

int parse_conditionrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_ANDOR) {
      fetchtok();
      return parse_condition(par);
   }
   return PARSE_OK;
}

int parse_comparison(buzzparser_t par) {
   return (parse_comparand(par) && parse_comparisonrest(par));
}

int parse_comparisonrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_CMP) {
      fetchtok();
      return parse_comparand(par);
   }
   return PARSE_OK;
}

int parse_comparand(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_BOOL) {
      fetchtok();
      return PARSE_OK;
   }
   if(par->tok->type == BUZZTOK_PAROPEN) {
      if(!parse_condition(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
   }
   return parse_expression(par);
}

int parse_conditionlist(buzzparser_t par) {
   return parse_condition(par) && parse_conditionlistrest(par);
}

int parse_conditionlistrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_LISTSEP) {
      return parse_conditionlist(par);
   }
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_expression(buzzparser_t par) {
   return parse_product(par) && parse_expressionrest(par);
}

int parse_expressionrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_ADDSUB) {
      fetchtok();
      return parse_expression(par);
   }
   return PARSE_OK;
}

int parse_product(buzzparser_t par) {
   return parse_modulo(par) && parse_productrest(par);
}

int parse_productrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_MULDIV) {
      fetchtok();
      return parse_product(par);
   }
   return PARSE_OK;
}

int parse_modulo(buzzparser_t par) {
   return parse_power(par) && parse_modulorest(par);
}

int parse_modulorest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_MOD) {
      fetchtok();
      return parse_modulo(par);
   }
   return PARSE_OK;
}

int parse_power(buzzparser_t par) {
   return parse_operand(par) && parse_powerrest(par);
}

int parse_powerrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_POW) {
      fetchtok();
      return parse_power(par);
   }
   return PARSE_OK;
}

int parse_operand(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_CONST) {
      fetchtok();
      return PARSE_OK;
   }
   if(par->tok->type == BUZZTOK_STRING) {
      fetchtok();
      return PARSE_OK;
   }
   if(par->tok->type == BUZZTOK_PAROPEN) {
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
   }
   return parse_idref(par);
}

/****************************************/
/****************************************/

int parse_command(buzzparser_t par) {
   return parse_idref(par) && parse_commandrest(par);
}

int parse_commandrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_ASSIGN) {
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_STATEND);
   }
   else if(par->tok->type == BUZZTOK_PAROPEN) {
      if(!parse_idlist(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      tokmatch(BUZZTOK_STATEND);
   }
   return PARSE_ERROR;
}

/****************************************/
/****************************************/

int parse_idref(buzzparser_t par) {
   tokmatch(BUZZTOK_ID);
   return parse_idrefrest(par);
}

int parse_idrefrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_DOT) {
      return parse_idref(par);
   }
   if(par->tok->type == BUZZTOK_IDXOPEN) {
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_IDXCLOSE);
   }
   return PARSE_OK;
}

int parse_idlist(buzzparser_t par) {
   return parse_idref(par) && parse_idlistrest(par);
}

int parse_idlistrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_LISTSEP) {
      return parse_idlist(par);
   }
   return PARSE_OK;
}

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
   buzzlex_destroy(&((*par)->lex));
   if((*par)->tok) buzzlex_destroytok(&((*par)->tok));
   free(*par);
   *par = NULL;
}

/****************************************/
/****************************************/

int buzzparser_parse(buzzparser_t par) {
   return 0;
}

/****************************************/
/****************************************/
