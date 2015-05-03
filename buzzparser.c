#include "buzzparser.h"
#include <stdio.h>
#include <stdlib.h>

static int PARSE_ERROR = 0;
static int PARSE_OK    = 1;

/****************************************/
/****************************************/

#define fetchtok()                                              \
   buzzlex_destroytok(&par->tok);                               \
   par->tok = buzzlex_nexttok(par->lex);                        \
   if(!par->tok) return PARSE_ERROR;

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
              "%s:%llu:%llu: Syntax error: expected %s, found %s\n",
              par->lex->fname,
              par->tok->line,
              par->tok->col,
              buzztok_desc[type],
              buzztok_desc[par->tok->type]);
      return PARSE_ERROR;
   }
   else {
      fprintf(stderr,
              "%s:%llu:%llu: Matched %s\n",
              par->lex->fname,
              par->tok->line,
              par->tok->col,
              buzztok_desc[type]);
      fetchtok();
      return PARSE_OK;
   }
}

#define tokmatch(tok) if(!match(par, tok)) return PARSE_ERROR;

/****************************************/
/****************************************/

int parse_script(buzzparser_t par);

int parse_blocklist(buzzparser_t par);
int parse_blocklistrest(buzzparser_t par);
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

int parse_conditionlist(buzzparser_t par);
int parse_conditionlistrest(buzzparser_t par);
int parse_condition(buzzparser_t par);
int parse_conditionrest(buzzparser_t par);
int parse_comparison(buzzparser_t par);
int parse_comparisonrest(buzzparser_t par);

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

int parse_idlist(buzzparser_t par);
int parse_idlistrest(buzzparser_t par);
int parse_idref(buzzparser_t par);
int parse_idrefrest(buzzparser_t par);

/****************************************/
/****************************************/

int parse_script(buzzparser_t par) {
   par->tok = buzzlex_nexttok(par->lex);
   if(!par->tok) {
      fprintf(stderr,
              "%s: Empty file\n",
              par->lex->fname);
      return PARSE_ERROR;
   }
   return parse_blocklist(par);
}

/****************************************/
/****************************************/

int parse_blocklist(buzzparser_t par) {
   fprintf(stderr, "Parsing block list start\n");
   return parse_block(par) && parse_blocklistrest(par);
}

int parse_blocklistrest(buzzparser_t par) {
   fprintf(stderr, "Parsing block list rest\n");
   if(par->tok->type == BUZZTOK_BLOCKOPEN) {
      return parse_blocklist(par);
   }
   else {
      fprintf(stderr, "Event list end\n");
      return PARSE_OK;
   }
}

int parse_block(buzzparser_t par) {
   fprintf(stderr, "Parsing block start\n");
   tokmatch(BUZZTOK_BLOCKOPEN);
   return parse_blockrest(par);
}

int parse_blockrest(buzzparser_t par) {
   fprintf(stderr, "Parsing block rest\n");
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      fprintf(stderr, "Block end\n");
      fetchtok();
      return PARSE_OK;
   }
   else {
      if(parse_statlist(par)) {
         tokmatch(BUZZTOK_BLOCKCLOSE);
         fprintf(stderr, "Block end\n");
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
   fprintf(stderr, "Parsing statement list start\n");
   return parse_stat(par) && parse_statrest(par);
}

int parse_statrest(buzzparser_t par) {
   fprintf(stderr, "Parsing statement list rest\n");
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      fprintf(stderr, "Statement list end\n");
      return PARSE_OK;
   }
   else {
      return parse_statlist(par);
   }
}

int parse_stat(buzzparser_t par) {
   fprintf(stderr, "Parsing statement\n");
   if(par->tok->type == BUZZTOK_STATEND) {
      fprintf(stderr, "Statement end\n");
      fetchtok();
      return PARSE_OK;
   }
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
   fprintf(stderr, "Parsing function definition\n");
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
   fprintf(stderr, "Parsing if start\n");
   tokmatch(BUZZTOK_IF);
   tokmatch(BUZZTOK_PAROPEN);
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   if(!parse_block(par)) return PARSE_ERROR;
   return parse_endif(par);
}

int parse_endif(buzzparser_t par) {
   fprintf(stderr, "Parsing if end\n");
   if(par->tok->type == BUZZTOK_ELSE) {
      fprintf(stderr, "Else found\n");
      fetchtok();
      return parse_block(par);
   }
   fprintf(stderr, "If end\n");
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_for(buzzparser_t par) {
   fprintf(stderr, "Parsing for\n");
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
   fprintf(stderr, "Parsing while\n");
   tokmatch(BUZZTOK_WHILE);
   tokmatch(BUZZTOK_PAROPEN);
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_conditionlist(buzzparser_t par) {
   fprintf(stderr, "Parsing condition list start\n");
   return parse_condition(par) && parse_conditionlistrest(par);
}

int parse_conditionlistrest(buzzparser_t par) {
   fprintf(stderr, "Parsing condition list rest\n");
   if(par->tok->type == BUZZTOK_LISTSEP) {
      fetchtok();
      return parse_conditionlist(par);
   }
   return PARSE_OK;
}

int parse_condition(buzzparser_t par) {
   fprintf(stderr, "Parsing condition start\n");
   if(par->tok->type == BUZZTOK_NOT) {
      fprintf(stderr, "Parsing not condition\n");
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
   fprintf(stderr, "Parsing condition rest\n");
   if(par->tok->type == BUZZTOK_ANDOR) {
      fprintf(stderr, "Parsing and/or condition\n");
      fetchtok();
      return parse_condition(par);
   }
   fprintf(stderr, "Condition end\n");
   return PARSE_OK;
}

int parse_comparison(buzzparser_t par) {
   fprintf(stderr, "Parsing comparison start\n");
   return (parse_expression(par) && parse_comparisonrest(par));
}

int parse_comparisonrest(buzzparser_t par) {
   fprintf(stderr, "Parsing comparison rest\n");
   if(par->tok->type == BUZZTOK_CMP) {
      fprintf(stderr, "Parsing comparison operator\n");
      fetchtok();
      return parse_expression(par);
   }
   fprintf(stderr, "Comparison end\n");
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_expression(buzzparser_t par) {
   fprintf(stderr, "Parsing expression start\n");
   return parse_product(par) && parse_expressionrest(par);
}

int parse_expressionrest(buzzparser_t par) {
   fprintf(stderr, "Parsing expression rest\n");
   if(par->tok->type == BUZZTOK_ADDSUB) {
      fprintf(stderr, "Parsing +- expression\n");
      fetchtok();
      return parse_expression(par);
   }
   fprintf(stderr, "Expression end\n");
   return PARSE_OK;
}

int parse_product(buzzparser_t par) {
   fprintf(stderr, "Parsing product start\n");
   return parse_modulo(par) && parse_productrest(par);
}

int parse_productrest(buzzparser_t par) {
   fprintf(stderr, "Parsing product rest\n");
   if(par->tok->type == BUZZTOK_MULDIV) {
      fprintf(stderr, "Parsing */ product\n");
      fetchtok();
      return parse_product(par);
   }
   fprintf(stderr, "Product end\n");
   return PARSE_OK;
}

int parse_modulo(buzzparser_t par) {
   fprintf(stderr, "Parsing modulo start\n");
   return parse_power(par) && parse_modulorest(par);
}

int parse_modulorest(buzzparser_t par) {
   fprintf(stderr, "Parsing modulo rest\n");
   if(par->tok->type == BUZZTOK_MOD) {
      fprintf(stderr, "Parsing % modulo\n");
      fetchtok();
      return parse_modulo(par);
   }
   fprintf(stderr, "Modulo end\n");
   return PARSE_OK;
}

int parse_power(buzzparser_t par) {
   fprintf(stderr, "Parsing power start\n");
   return parse_operand(par) && parse_powerrest(par);
}

int parse_powerrest(buzzparser_t par) {
   fprintf(stderr, "Parsing power rest\n");
   if(par->tok->type == BUZZTOK_POW) {
      fprintf(stderr, "Parsing ^ power\n");
      fetchtok();
      return parse_power(par);
   }
   return PARSE_OK;
}

int parse_operand(buzzparser_t par) {
   fprintf(stderr, "Parsing operand\n");
   if(par->tok->type == BUZZTOK_BOOL) {
      fprintf(stderr, "Operand is token true/false\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_CONST) {
      fprintf(stderr, "Operand is numeric constant\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_STRING) {
      fprintf(stderr, "Operand is string\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_PAROPEN) {
      fprintf(stderr, "Operand is (expression)\n");
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      return PARSE_OK;
   }
   fprintf(stderr, "Operand is idref\n");
   return parse_idref(par);
}

/****************************************/
/****************************************/

int parse_command(buzzparser_t par) {
   fprintf(stderr, "Parsing command start\n");
   return parse_idref(par) && parse_commandrest(par);
}

int parse_commandrest(buzzparser_t par) {
   fprintf(stderr, "Parsing command rest\n");
   if(par->tok->type == BUZZTOK_STATEND) {
      fprintf(stderr, "Statement end\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_ASSIGN) {
      fprintf(stderr, "Parsing assignment\n");
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_STATEND);
      return PARSE_OK;
   }
   fprintf(stderr,
           "%s:%llu:%llu: Syntax error: expected = or (, found %s\n",
           par->lex->fname,
           par->tok->line,
           par->tok->col,
           buzztok_desc[par->tok->type]);
   return PARSE_ERROR;
}

/****************************************/
/****************************************/

int parse_idlist(buzzparser_t par) {
   fprintf(stderr, "Parsing idlist start\n");
   return parse_idref(par) && parse_idlistrest(par);
}

int parse_idlistrest(buzzparser_t par) {
   fprintf(stderr, "Parsing idlist rest\n");
   if(par->tok->type == BUZZTOK_LISTSEP) {
      fprintf(stderr, "Parsing next idlist item\n");
      fetchtok();
      return parse_idlist(par);
   }
   if(par->tok->type == BUZZTOK_PARCLOSE) {
      fprintf(stderr, "Idlist end\n");
      return PARSE_OK;
   }
   fprintf(stderr,
           "%s:%llu:%llu: Syntax error: expected , or ), found %s\n",
           par->lex->fname,
           par->tok->line,
           par->tok->col,
           buzztok_desc[par->tok->type]);
   return PARSE_ERROR;
}

int parse_idref(buzzparser_t par) {
   fprintf(stderr, "Parsing idref start\n");
   if(par->tok->type == BUZZTOK_LISTSEP ||
      par->tok->type == BUZZTOK_PARCLOSE) {
      fprintf(stderr, "Idref end\n");
      return PARSE_OK;
   }
   tokmatch(BUZZTOK_ID);
   return parse_idrefrest(par);
}

int parse_idrefrest(buzzparser_t par) {
   fprintf(stderr, "Parsing idref rest\n");
   if(par->tok->type == BUZZTOK_DOT) {
      fprintf(stderr, "Parsing idref.idref\n");
      fetchtok();
      return parse_idref(par);
   }
   if(par->tok->type == BUZZTOK_IDXOPEN) {
      fprintf(stderr, "Parsing idref[expression]\n");
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_IDXCLOSE);
      return parse_idrefrest(par);
   }
   else if(par->tok->type == BUZZTOK_PAROPEN) {
      fprintf(stderr, "Parsing function call\n");
      fetchtok();
      if(!parse_conditionlist(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      return parse_idrefrest(par);
   }
   fprintf(stderr, "Idref end\n");
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
   /* Parse the script */
   return parse_script(par);
}

/****************************************/
/****************************************/

buzzptree_t buzzparser_newtree(buzztok_t tok) {
   /* Create node. calloc() zeroes the struct */
   buzzptree_t n = (buzzptree_t)calloc(1, sizeof(struct buzzptree_s));
   /* Copy the token into this node */
   n->tok = buzzlex_clonetok(tok);
   /* Return new token */
   return n;
}

/****************************************/
/****************************************/

void buzzparser_addtreechild(buzzptree_t t,
                             buzzptree_t c) {
   if(t->child == NULL) {
      /* Add as first child */
      t->child = c;
   }
   else {
      /* Look for last child */
      buzzptree_t l = t->child;
      while(l->next != NULL) l = l->next;
      /* Add c as last child */
      l->next = c;
   }
   /* Set t as c's parent */
   c->parent = t;
   /* c is the last child */
   c->next = NULL;
}

/****************************************/
/****************************************/

void buzzparser_fortree(buzzptree_t t,
                        buzzptree_hook_t f,
                        void* p) {
   for(buzzptree_t c = t->child; c != NULL; c = c->next) {
      buzzparser_fortree(c, f, p);
   }
   f(t, p);
}

/****************************************/
/****************************************/

void buzzparser_destroytree(buzzptree_t* t) {
   if((*t)->child) {
      buzzptree_t cur, next;
      next = (*t)->child;
      do {
         cur = next;
         next = cur->next;
         buzzparser_destroytree(&cur);
      } while(next != NULL);
   }   
   buzzlex_destroytok(&((*t)->tok));
   free(*t);
}

/****************************************/
/****************************************/
