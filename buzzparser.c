#include "buzzparser.h"
#include "buzzdict.h"
#include <stdio.h>
#include <stdlib.h>

static int PARSE_ERROR = 0;
static int PARSE_OK    = 1;

/****************************************/
/****************************************/

#define fetchtok()                              \
   buzzlex_destroytok(&par->tok);               \
   par->tok = buzzlex_nexttok(par->lex);        \
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
              "[DEBUG] %s:%llu:%llu: Matched %s\n",
              par->lex->fname,
              par->tok->line,
              par->tok->col,
              buzztok_desc[type]);
      return PARSE_OK;
   }
}

#define tokmatch(tok) if(!match(par, tok)) return PARSE_ERROR;

#define putlabel(L) fprintf(par->asmstream, "\n@__label%lld__\n", (L));
#define reflabel(L) fprintf(par->asmstream, "@__label%lld__\n", (L));

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

int parse_for(buzzparser_t par);

int parse_while(buzzparser_t par);

int parse_conditionlist(buzzparser_t par);
int parse_condition(buzzparser_t par);
int parse_comparison(buzzparser_t par);

int parse_expression(buzzparser_t par);
int parse_product(buzzparser_t par);
int parse_modulo(buzzparser_t par);
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
              "[DEBUG] %s: Empty file\n",
              par->lex->fname);
      return PARSE_ERROR;
   }
   int retval = parse_blocklist(par);
   if(retval == PARSE_OK) {
      putlabel(par->labels);
      fprintf(par->asmstream, "\tdone\n\n");
   }
   return retval;
}

/****************************************/
/****************************************/

int parse_blocklist(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing block list start\n");
   return parse_block(par) && parse_blocklistrest(par);
}

int parse_blocklistrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing block list rest\n");
   if(par->tok->type == BUZZTOK_BLOCKOPEN) {
      return parse_blocklist(par);
   }
   else {
      fprintf(stderr, "[DEBUG] Event list end\n");
      return PARSE_OK;
   }
}

int parse_block(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing block start\n");
   tokmatch(BUZZTOK_BLOCKOPEN);
   fetchtok();
   return parse_blockrest(par);
}

int parse_blockrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing block rest\n");
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      fprintf(stderr, "[DEBUG] Block end\n");
      fetchtok();
      return PARSE_OK;
   }
   else {
      if(parse_statlist(par)) {
         tokmatch(BUZZTOK_BLOCKCLOSE);
         fetchtok();
         fprintf(stderr, "[DEBUG] Block end\n");
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
   fprintf(stderr, "[DEBUG] Parsing statement list start\n");
   return parse_stat(par) && parse_statrest(par);
}

int parse_statrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing statement list rest\n");
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      fprintf(stderr, "[DEBUG] Statement list end\n");
      return PARSE_OK;
   }
   else {
      return parse_statlist(par);
   }
}

int parse_stat(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing statement\n");
   if(par->tok->type == BUZZTOK_STATEND) {
      fprintf(stderr, "[DEBUG] Statement end\n");
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
   fprintf(stderr, "[DEBUG] Parsing function definition\n");
   tokmatch(BUZZTOK_FUN);
   fetchtok();
   tokmatch(BUZZTOK_ID);
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   if(!parse_idlist(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_if(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing if start\n");
   /* Save labels for else branch and for if end */
   uint64_t lab1 = par->labels;
   uint64_t lab2 = par->labels + 1;
   par->labels += 2;
   /* Parse tokens */
   tokmatch(BUZZTOK_IF);
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   /* True branch follows condition; false branch follows true one */
   /* Jamp to label 1 if the condition is false */
   /* Label 1 is either if end (in case of no else branch) or else branch */
   fprintf(par->asmstream, "\tjumpz ");
   reflabel(lab1);
   if(!parse_block(par)) return PARSE_ERROR;
   /* Eat away the newlines, if any */
   while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
   if(par->tok->type == BUZZTOK_ELSE) {
      fprintf(stderr, "[DEBUG] Else found\n");
      fetchtok();
      /* Make true branch jump to label 2 => if end */
      fprintf(par->asmstream, "\tjump ");
      reflabel(lab2);
      /* Mark this place as label 1 and keep parsing */
      putlabel(lab1);
      if(!parse_block(par)) return PARSE_ERROR;
      /* Mark the if end as label 2 */
      putlabel(lab2);
   }
   else {
      /* Mark the if end as label 1 */
      putlabel(lab1);
   }
   fprintf(stderr, "[DEBUG] If end\n");
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_for(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing for\n");
   tokmatch(BUZZTOK_FOR);
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   if(!parse_idref(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_ASSIGN);
   fetchtok();
   if(!parse_expression(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_LISTSEP);
   fetchtok();
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_LISTSEP);
   fetchtok();
   if(!parse_idref(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_ASSIGN);
   fetchtok();
   if(!parse_expression(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_while(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing while\n");
   tokmatch(BUZZTOK_WHILE);
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_conditionlist(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing condition list start\n");
   if(!parse_condition(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_LISTSEP) {
      fetchtok();
      if(!parse_condition(par)) return PARSE_ERROR;
   }
   fprintf(stderr, "[DEBUG] Condition list end\n");
   return PARSE_OK;
}

int parse_condition(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing condition start\n");
   if(!parse_comparison(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_ANDOR) {
      char op[4];
      strcpy(op, par->tok->value);
      fetchtok();
      if(!parse_comparison(par)) return PARSE_ERROR;
      fprintf(par->asmstream, "\t%s\n", op);
   }
   fprintf(stderr, "[DEBUG] Condition end\n");
   return PARSE_OK;
}

int parse_comparison(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_PAROPEN) {
      fprintf(stderr, "[DEBUG] Parsing (condition) start\n");
      fetchtok();
      if(!parse_condition(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      fetchtok();
      fprintf(stderr, "[DEBUG] (condition) end\n");
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_NOT) {
      fprintf(stderr, "[DEBUG] Parsing NOT condition start\n");
      fetchtok();
      if(!parse_comparison(par)) return PARSE_ERROR;
      fprintf(par->asmstream, "\tnot\n");
      fprintf(stderr, "[DEBUG] NOT condition end\n");
      return PARSE_OK;
   }
   else {
      fprintf(stderr, "[DEBUG] Parsing comparison condition start\n");
      if(!parse_expression(par)) return PARSE_ERROR;
      if(par->tok->type == BUZZTOK_CMP) {
         char op[4];
         if     (strcmp(par->tok->value, "==") == 0) strcpy(op, "eq");
         else if(strcmp(par->tok->value, "!=") == 0) strcpy(op, "neq");
         else if(strcmp(par->tok->value, "<")  == 0) strcpy(op, "lt");
         else if(strcmp(par->tok->value, "<=") == 0) strcpy(op, "lte");
         else if(strcmp(par->tok->value, ">")  == 0) strcpy(op, "gt");
         else if(strcmp(par->tok->value, ">=") == 0) strcpy(op, "gte");
         fetchtok();
         if(!parse_expression(par)) return PARSE_ERROR;
         fprintf(par->asmstream, "\t%s\n", op);
      }
      fprintf(stderr, "[DEBUG] Parsing comparison condition end\n");
      return PARSE_OK;
   }
}

/****************************************/
/****************************************/

int parse_expression(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing expression start\n");
   if(!parse_product(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_ADDSUB) {
      fprintf(stderr, "[DEBUG] Parsing +- expression\n");
      char op = par->tok->value[0];
      fetchtok();
      if(!parse_product(par)) return PARSE_ERROR;
      if     (op == '+') fprintf(par->asmstream, "\tadd\n");
      else if(op == '-') fprintf(par->asmstream, "\tsub\n");
   }
   fprintf(stderr, "[DEBUG] Expression end\n");
   return PARSE_OK;
}

int parse_product(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing product start\n");
   if(!parse_modulo(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_MULDIV) {
      fprintf(stderr, "[DEBUG] Parsing */ product\n");
      char op = par->tok->value[0];
      fetchtok();
      if(!parse_modulo(par)) return PARSE_ERROR;
      if(op == '*') {
         fprintf(par->asmstream, "\tmul\n");
      }
      else if(op == '/') {
         fprintf(par->asmstream, "\tdiv\n");
      }
   }
   fprintf(stderr, "[DEBUG] Product end\n");
   return PARSE_OK;
}

int parse_modulo(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing modulo start\n");
   if(!parse_power(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_MOD) {
      fprintf(stderr, "[DEBUG] Parsing modulo\n");
      fetchtok();
      if(!parse_power(par)) return PARSE_ERROR;
      fprintf(par->asmstream, "\tmod\n");
   }
   fprintf(stderr, "[DEBUG] Modulo end\n");
   return PARSE_OK;
}

int parse_power(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing power start\n");
   return parse_operand(par) && parse_powerrest(par);
}

int parse_powerrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing power rest\n");
   if(par->tok->type == BUZZTOK_POW) {
      fprintf(stderr, "[DEBUG] Parsing power\n");
      fetchtok();
      if(!parse_power(par)) return PARSE_ERROR;
      fprintf(par->asmstream, "\tpow\n");
   }
   fprintf(stderr, "[DEBUG] End power\n");
   return PARSE_OK;
}

int parse_operand(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing operand\n");
   if(par->tok->type == BUZZTOK_BOOL) {
      fprintf(stderr, "[DEBUG] Operand is token true/false\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_CONST) {
      fprintf(stderr, "[DEBUG] Operand is numeric constant\n");
      if(strchr(par->tok->value, '.')) {
         /* Floating-point constant */
         fprintf(par->asmstream, "\tpushf %s\n", par->tok->value);
      }
      else {
         /* Integer constant */
         fprintf(par->asmstream, "\tpushi %s\n", par->tok->value);
      }
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_STRING) {
      fprintf(stderr, "[DEBUG] Operand is string\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_PAROPEN) {
      fprintf(stderr, "[DEBUG] Operand is (expression)\n");
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      fetchtok();
      fprintf(stderr, "[DEBUG] (expression) end\n");
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_ADDSUB) {
      char op = par->tok->value[0];
      fetchtok();
      if(par->tok->type == BUZZTOK_CONST) {
         fprintf(stderr, "[DEBUG] Operand is signed +- constant\n");
         if(strchr(par->tok->value, '.')) {
            /* Floating-point constant */
            fprintf(par->asmstream, "\tpushf %c%s\n", op, par->tok->value);
         }
         else {
            /* Integer constant */
            fprintf(par->asmstream, "\tpushi %c%s\n", op, par->tok->value);
         }
         fetchtok();
         return PARSE_OK;
      }
      else {
         fprintf(stderr, "[DEBUG] Operand is signed +-\n");
         if(!parse_power(par)) return PARSE_ERROR;
         if(op == '-') fprintf(par->asmstream, "\tunm\n");
         fprintf(stderr, "[DEBUG] Signed operand +- end\n");
         return PARSE_OK;
      }
   }
   fprintf(stderr, "[DEBUG] Operand is idref\n");
   return parse_idref(par);
}

/****************************************/
/****************************************/

int parse_command(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing command start\n");
   return parse_idref(par) && parse_commandrest(par);
}

int parse_commandrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing command rest\n");
   if(par->tok->type == BUZZTOK_STATEND) {
      fprintf(stderr, "[DEBUG] Statement end\n");
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_ASSIGN) {
      fprintf(stderr, "[DEBUG] Parsing assignment\n");
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_STATEND);
      fetchtok();
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
   fprintf(stderr, "[DEBUG] Parsing idlist start\n");
   return parse_idref(par) && parse_idlistrest(par);
}

int parse_idlistrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing idlist rest\n");
   if(par->tok->type == BUZZTOK_LISTSEP) {
      fprintf(stderr, "[DEBUG] Parsing next idlist item\n");
      fetchtok();
      return parse_idlist(par);
   }
   if(par->tok->type == BUZZTOK_PARCLOSE) {
      fprintf(stderr, "[DEBUG] Idlist end\n");
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
   fprintf(stderr, "[DEBUG] Parsing idref start\n");
   if(par->tok->type == BUZZTOK_LISTSEP ||
      par->tok->type == BUZZTOK_PARCLOSE) {
      fprintf(stderr, "[DEBUG] Idref end\n");
      return PARSE_OK;
   }
   tokmatch(BUZZTOK_ID);
   fetchtok();
   return parse_idrefrest(par);
}

int parse_idrefrest(buzzparser_t par) {
   fprintf(stderr, "[DEBUG] Parsing idref rest\n");
   if(par->tok->type == BUZZTOK_DOT) {
      fprintf(stderr, "[DEBUG] Parsing idref.idref\n");
      fetchtok();
      return parse_idref(par);
   }
   if(par->tok->type == BUZZTOK_IDXOPEN) {
      fprintf(stderr, "[DEBUG] Parsing idref[expression]\n");
      fetchtok();
      if(!parse_expression(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_IDXCLOSE);
      fetchtok();
      return parse_idrefrest(par);
   }
   else if(par->tok->type == BUZZTOK_PAROPEN) {
      fprintf(stderr, "[DEBUG] Parsing function call\n");
      fetchtok();
      if(!parse_conditionlist(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      fetchtok();
      return parse_idrefrest(par);
   }
   fprintf(stderr, "[DEBUG] Idref end\n");
   return PARSE_OK;
}

/****************************************/
/****************************************/

void buzzparser_destroy_symt(uint32_t pos, void* data, void* params) {
   buzzdict_destroy((buzzdict_t*)data);
}

buzzparser_t buzzparser_new(const char* fscript,
                            const char* fasm) {
   /* Create parser state */
   buzzparser_t par = (buzzparser_t)malloc(sizeof(struct buzzparser_s));
   /* Create lexer */
   par->lex = buzzlex_new(fscript);
   if(!par->lex) {
      free(par);
      return NULL;
   }
   /* Copy string */
   par->asmfn = strdup(fasm);
   /* Open file */
   par->asmstream = fopen(par->asmfn, "w");
   if(!par->asmstream) {
      perror(par->asmfn);
      free(par->asmfn);
      free(par);
      return NULL;
   }
   /* Initialize label counter */
   par->labels = 0;
   /* Symbol table stack */
   par->symstack = buzzdarray_new(10, sizeof(buzzdict_t), buzzparser_destroy_symt);
   /* Return parser state */
   return par;
}

/****************************************/
/****************************************/

void buzzparser_destroy(buzzparser_t* par) {
   buzzdarray_destroy(&((*par)->symstack));
   free((*par)->asmfn);
   fclose((*par)->asmstream);
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
