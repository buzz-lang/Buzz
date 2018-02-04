#include "buzzparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

/****************************************/
/****************************************/

static int PARSE_ERROR    =  0;
static int PARSE_OK       =  1;

static int TYPE_BASIC     = -1;
static int TYPE_CLOSURE   = -2;
static int TYPE_TABLE     = -3;

static int SCOPE_LOCAL    =  0;
static int SCOPE_GLOBAL   =  1;
static int SCOPE_AUTO     =  2;

/****************************************/
/****************************************/

#define DEBUG(MSG, ...) fprintf(stderr, "[DEBUG] " MSG, ##__VA_ARGS__)
#define TODO(MSG, ...) fprintf(stderr, "[TODO] " MSG, ##__VA_ARGS__)

/****************************************/
/****************************************/

/*
 * At the end of parsing, the strings are output in the bytecode
 * file. The writing occurs in order, where the order is dictated by
 * the increasing string pos field. 
 * To achieve this, the contents of the string dictionary are copied
 * to an array, which is later sorted.
 */
struct strarray_data_s {
   char* str;
   uint16_t pos;
};

void string_copy(const void* key, void* data, void* params) {
   struct strarray_data_s* x = (struct strarray_data_s*)malloc(sizeof(struct strarray_data_s));
   x->str = *(char**)key;
   x->pos = *(uint16_t*)data;
   buzzdarray_t arr = (buzzdarray_t)params;
   buzzdarray_push(arr, &x);
}

int string_cmp(const void* a, const void* b) {
   if((*(struct strarray_data_s**)a)->pos < (*(struct strarray_data_s**)b)->pos) return -1;
   if((*(struct strarray_data_s**)a)->pos > (*(struct strarray_data_s**)b)->pos) return  1;
   return 0;
}

void string_destroy(uint32_t pos, void* data, void* params) {
   free((*(struct strarray_data_s**)data)->str);
   free(*(struct strarray_data_s**)data);
}

uint32_t string_add(buzzdict_t strings, const char* str) {
   const uint16_t* ppos = buzzdict_get(strings, &str, uint16_t);
   if(!ppos) {
      /* String not found */
      char* dup = strdup(str);
      uint16_t pos = buzzdict_size(strings);
      buzzdict_set(strings, &dup, &pos);
      return pos;
   }
   else {
      /* String found */
      return *ppos;
   }
}

void string_print(uint32_t pos, void* data, void* params) {
   fprintf((FILE*)params, "'%s\n", (*(struct strarray_data_s**)data)->str);
}

/****************************************/
/****************************************/

struct sym_s {
   /* Position of the symbol in the activation record (-1 for unknown) */
   int64_t pos;
   /* Symbol type
    * Can be any of the TYPE_* declarations
    * or a value >0 which is an offset in the local variable list
    */
   int type;
   /* 1 if the symbol is global, 0 if local */
   int global;
};

const struct sym_s* sym_lookup(const char* sym,
                               buzzdarray_t symstack) {
   const struct sym_s* symdata = NULL;
   /* Go through the symbol tables, from the top to the bottom */
   int64_t i;
   for(i = buzzdarray_size(symstack)-1; i >= 0; --i) {
      /* Get symbol table */
      buzzdict_t st = buzzdarray_get(symstack, i, buzzdict_t);
      /* Look for the symbol - if found, return immediately */
      symdata = buzzdict_get(st, &sym, struct sym_s);
      if(symdata) return symdata;
   }
   /* Symbol not found */
   return NULL;
}

void sym_add(buzzparser_t par, const char* sym, int scope) {
   /* Copy string */
   char* key = strdup(sym);
   /* Check whether symbol is global or local */
   int global;
   if(scope == SCOPE_AUTO) {
      global = (buzzdarray_size(par->symstack) == 1);
   }
   else {
      global = scope;
   }
   /* Calculate position attribute */
   uint32_t pos;
   /* For a global symbol, the position corresponds to the string id */
   if(global) pos = string_add(par->strings, sym);
   /* For a local symbol, the position is that in the activation record */
   else       pos = buzzdict_size(par->syms);
   /* Create symbol and save it */
   struct sym_s symdata = {
      .pos  = pos,
      .type = TYPE_BASIC,
      .global = global
   };
   /* A global symbol must be stored in the base symbol table */
   if(global) buzzdict_set(buzzdarray_get(par->symstack, 0, buzzdict_t),
                           &key, &symdata);
   /* A local symbol must be stored in the current symbol table */
   else       buzzdict_set(par->syms, &key, &symdata);
}

void sym_clone(const void* key, void* data, void* params) {
   struct sym_s* toclone = (struct sym_s*)data;
   /* Copy key */
   char* newkey = strdup(*(char**)key);
   /* Create symbol */
   struct sym_s newsym = {
      .pos  = toclone->pos,
      .type = toclone->type,
      .global = toclone->global
   };
   /* Store the symbol */
   buzzdict_set(((buzzparser_t)params)->syms, &newkey, &newsym);
}

void sym_destroy(const void* key,
                void* data,
                void* params) {
   free(*(char**)key);
   free((void*)key);
   free(data);
}

#define SYMT_BUCKETS 100

#define symt_push() par->syms = buzzdict_new(SYMT_BUCKETS, sizeof(char*), sizeof(struct sym_s), buzzdict_strkeyhash, buzzdict_strkeycmp, sym_destroy); buzzdarray_push(par->symstack, &(par->syms));

#define symt_clone() { buzzdict_t toclone = buzzdarray_last(par->symstack, buzzdict_t); symt_push(); buzzdict_foreach(toclone, sym_clone, par); }

#define symt_pop() buzzdarray_pop(par->symstack); par->syms = buzzdarray_last(par->symstack, buzzdict_t);

void symt_destroy(uint32_t pos, void* data, void* params) {
   buzzdict_destroy((buzzdict_t*)data);
}

struct idrefinfo_s {
   /* When < 0 correspond to TYPE_*; when >=0 corresponds to offset in variable list */
   int info;
   /* For info >=0, says whether the idref is for local or global variable */
   int global;
};

/****************************************/
/****************************************/

#define LABELREF "@__label_"

/*
 * A chunk of code
 * This can be either
 * - the code in the global scope
 * - a function
 * - a lambda (anonymous function)
 */
struct chunk_s {
   /* The label for this chunk */
   uint32_t label;
   /* The code for this chunk */
   char* code;
   /* not-NULL if a symbol must be registered (function), NULL if not (lambda) */
   const struct sym_s* sym;
   /* The code size */
   size_t csize;
   /* The buffer capacity */
   size_t ccap;
};
typedef struct chunk_s* chunk_t;

chunk_t chunk_new(uint32_t label, const struct sym_s* sym) {
   chunk_t c = (chunk_t)malloc(sizeof(struct chunk_s));
   c->label = label;
   c->csize = 0;
   c->ccap = 10;
   c->code = (char*)malloc(c->ccap);
   c->sym = sym;
   return c;
}

void chunk_destroy(uint32_t pos, void* data, void* params) {
   chunk_t* c = (chunk_t*)data;
   free((*c)->code);
   free(*c);
   *c = NULL;
}

void chunk_addcode(chunk_t c, char* code, buzztok_t tok) {
   /* Append code to debug information */
   char* instr;
   if(tok) {
      asprintf(&instr, "%s\t|%" PRIu64 ",%" PRIu64 ",%s\n", code, tok->line, tok->col, tok->fname);
   }
   else {
      asprintf(&instr, "%s\n", code);
   }
   /* Get length of the code */
   size_t l = strlen(instr);
   /* Resize the code buffer */
   if(c->csize + l >= c->ccap) {
      do { c->ccap *= 2; } while(c->csize + l >= c->ccap);
      c->code = realloc(c->code, c->ccap);
   }
   /* Copy the code */
   strncpy(c->code + c->csize, instr, l);
   /* Update size */
   c->csize += l;
   /* Cleanup */
   free(instr);
}

void chunk_finalize(chunk_t c) {
   /* Shrink the memory allocation */
   c->ccap = c->csize + 1;
   c->code = realloc(c->code, c->ccap);
   /* Add a \0 at the end */
   c->code[c->csize] = 0;
}

#define chunk_append(...) {                         \
      char* str;                                    \
      asprintf(&str, ##__VA_ARGS__);                \
      chunk_addcode(par->chunk, str, par->tok);     \
      free(str);                                    \
   }

void chunk_register(uint32_t pos, void* data, void* params) {
   /* Cast params */
   chunk_t c = *(chunk_t*)data;
   FILE* f = (FILE*)params;
   /* Print registration code */
   if(c->sym) {
      if(c->sym->global) fprintf(f, "\tpushs %" PRId64 "\n", c->sym->pos);
      fprintf(f, "\tpushcn " LABELREF "%u\n", c->label);
      if(c->sym->global) fprintf(f, "\tgstore\n");
      else               fprintf(f, "\tlstore %" PRId64 "\n", c->sym->pos);
   }
}

void chunk_print(uint32_t pos, void* data, void* params) {
   /* Cast params */
   chunk_t c = *(chunk_t*)data;
   FILE* f = (FILE*)params;
   /* Print the label */
   fprintf(f, "\n" LABELREF "%u\n", c->label);
   /* Print the code */
   fprintf(f, "%s", c->code);
}

#define chunk_push(SYM)                                        \
   chunk_t oldc = par->chunk;                                  \
   par->chunk = chunk_new(par->labels, (SYM));                 \
   buzzdarray_push(par->chunks, &par->chunk);                  \
   ++(par->labels);

#define chunk_pop() chunk_finalize(par->chunk); par->chunk = oldc;

#define chunk_buf_push()                                       \
   char* tmpcode = par->chunk->code;                           \
   size_t tmpsize = par->chunk->csize;                         \
   size_t tmpcap = par->chunk->ccap;                           \
   par->chunk->code = malloc(par->chunk->ccap);                \
   par->chunk->csize = 0;                                      \

#define chunk_buf_pop()                                                 \
   if(tmpsize + par->chunk->csize >= tmpcap) {                          \
      do { tmpcap *= 2; } while(tmpsize + par->chunk->csize >= tmpcap); \
      tmpcode = realloc(tmpcode, tmpcap);                               \
   }                                                                    \
   strncpy(tmpcode + tmpsize, par->chunk->code, par->chunk->csize);     \
   tmpsize += par->chunk->csize;                                        \
   free(par->chunk->code);                                              \
   par->chunk->code = tmpcode;                                          \
   par->chunk->csize = tmpsize;                                         \
   par->chunk->ccap = tmpcap;

#define chunk_buf_append(...) {                                                      \
      char* str;                                                                     \
      struct chunk_s tmpchunk = {.code = tmpcode, .csize = tmpsize, .ccap = tmpcap}; \
      asprintf(&str, ##__VA_ARGS__);                                                 \
      chunk_addcode(&tmpchunk, str, par->tok);                                       \
      free(str);                                                                     \
      tmpcode = tmpchunk.code;                                                       \
      tmpsize = tmpchunk.csize;                                                      \
      tmpcap  = tmpchunk.ccap;                                                       \
   }

/****************************************/
/****************************************/

#define fetchtok()                                                      \
   buzzlex_destroytok(&par->tok);                                       \
   par->tok = buzzlex_nexttok(par->lex);                                \
   if(!par->tok)                                                        \
      return PARSE_ERROR;

int match(buzzparser_t par,
          buzztok_type_e type) {
   if(!par->tok) {
      fprintf(stderr,
              "%s: Syntax error: expected %s, found end of file\n",
              par->scriptfn,
              buzztok_desc[type]);
      return PARSE_ERROR;
   }
   if(par->tok->type != type) {
      fprintf(stderr,
              "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected %s, found %s\n",
              buzzlex_getfile(par->lex)->fname,
              par->tok->line,
              par->tok->col,
              buzztok_desc[type],
              buzztok_desc[par->tok->type]);
      return PARSE_ERROR;
   }
   else return PARSE_OK;
}

#define tokmatch(tok) if(!match(par, tok)) return PARSE_ERROR;

/****************************************/
/****************************************/

int parse_script(buzzparser_t par);

int parse_statlist(buzzparser_t par);
int parse_stat(buzzparser_t par);
int parse_block(buzzparser_t par);
int parse_blockstat(buzzparser_t par);

int parse_var(buzzparser_t par);
int parse_fun(buzzparser_t par);
int parse_if(buzzparser_t par);
int parse_for(buzzparser_t par);
int parse_while(buzzparser_t par);

int parse_conditionlist(buzzparser_t par,
                        int* numargs);
int parse_condition(buzzparser_t par);
int parse_comparison(buzzparser_t par);

int parse_expression(buzzparser_t par);
int parse_product(buzzparser_t par);
int parse_modulo(buzzparser_t par);
int parse_power(buzzparser_t par);
int parse_powerrest(buzzparser_t par);
int parse_operand(buzzparser_t par);

int parse_command(buzzparser_t par);

int parse_idlist(buzzparser_t par);
int parse_idreflist(buzzparser_t par);
int parse_idref(buzzparser_t par,
                int lvalue,
                struct idrefinfo_s* idrefinfo);

int parse_lambda(buzzparser_t par);

/****************************************/
/****************************************/

int parse_script(buzzparser_t par) {
   /* Fetch the first token */
   par->tok = buzzlex_nexttok(par->lex);
   if(!par->tok) return PARSE_ERROR;
   /* Add a symbol table */
   symt_push();
   /* Add the first chunk for the global scope */
   chunk_push(0);
   /* Parse the statements */
   if(!parse_statlist(par)) return PARSE_ERROR;
   /* Finalize the output */
   chunk_append("\n@__exitpoint");
   chunk_append("\tdone");
   chunk_pop();
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_statlist(buzzparser_t par) {
   /* Parse first statement */
   if(!parse_stat(par)) return PARSE_ERROR;
   /* Keep parsing statements as long as you find tokens */
   while(par->tok && par->tok->type != BUZZTOK_BLOCKCLOSE) {
      while(par->tok && par->tok->type == BUZZTOK_STATEND) {
         buzzlex_destroytok(&par->tok);
         par->tok = buzzlex_nexttok(par->lex);
      }
      /* Make sure a file inclusion error did not happen */
      if(!par->tok && !buzzlex_done(par->lex))
         return PARSE_ERROR;
      /* Parse the statement */
      if(par->tok && !parse_stat(par)) return PARSE_ERROR;
   }
   return PARSE_OK;
}

int parse_stat(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_STATEND || par->tok->type == BUZZTOK_BLOCKCLOSE)
      return PARSE_OK;
   if(par->tok->type == BUZZTOK_VAR)
      return parse_var(par);
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

/* Data passed along when searching for variables to erase */
struct symdeldata_s {
   /* List of variables to erase, initially empty */
   buzzdarray_t dellist;
   /* Maximum position a variable can have - those with higher pos are deleted */
   uint32_t maxpos;
};

void buzzparser_symtodel(const void* key, void* data, void* params) {
   struct symdeldata_s* symdeldata = (struct symdeldata_s*)params;
   if(((struct sym_s*)data)->pos >= symdeldata->maxpos) {
      buzzdarray_push(symdeldata->dellist, (char**)key);
   }
}

void buzzparser_symdel(uint32_t pos, void* data, void* params) {
   buzzdict_remove((buzzdict_t)params, (char**)data);
}

int parse_block(buzzparser_t par) {
   tokmatch(BUZZTOK_BLOCKOPEN);
   fetchtok();
   if(par->tok->type == BUZZTOK_BLOCKCLOSE) {
      fetchtok();
      return PARSE_OK;
   }
   else {
      /* Save the current number of variables */
      uint32_t numvars = buzzdict_size(par->syms);
      if(parse_statlist(par)) {
         tokmatch(BUZZTOK_BLOCKCLOSE);
         fetchtok();
         /* Remove the variables defined in this block */
         struct symdeldata_s symdeldata = {
            .dellist = buzzdarray_new(1, sizeof(char*), NULL),
            .maxpos = numvars
         };
         buzzdict_foreach(par->syms, buzzparser_symtodel, &symdeldata);
         buzzdarray_foreach(symdeldata.dellist, buzzparser_symdel, par->syms);
         buzzdarray_destroy(&(symdeldata.dellist));
         return PARSE_OK;
      }
      else {
         return PARSE_ERROR;
      }
   }
}

/****************************************/
/****************************************/

int parse_blockstat(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_BLOCKOPEN) {
      /* It's a block */
      return parse_block(par);
   }
   else {
      /* It's a single statement, eat extra statement end characters */
      while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
      return parse_stat(par);
   }
}

/****************************************/
/****************************************/

int parse_var(buzzparser_t par) {
   /* Match the 'var' token */
   tokmatch(BUZZTOK_VAR);
   fetchtok();
   /* Match an id */
   tokmatch(BUZZTOK_ID);
   /* Look it up in the symbol table */
   const struct sym_s* s = sym_lookup(par->tok->value, par->symstack);
   if(s) {
      fprintf(stderr,
              "%s:%" PRIu64 ":%" PRIu64 ": Duplicated symbol '%s'\n",
              buzzlex_getfile(par->lex)->fname,
              par->tok->line,
              par->tok->col,
              par->tok->value);
      return PARSE_ERROR;
   }
   /* Add a symbol for this variable */
   sym_add(par, par->tok->value, SCOPE_AUTO);
   s = sym_lookup(par->tok->value, par->symstack);
   /* Is lvalue a global symbol? If so, push its string id */
   if(s->global) chunk_append("\tpushs %" PRId64, s->pos);
   /* Is the variable initialized? */
   fetchtok();
   if(par->tok->type == BUZZTOK_ASSIGN) {
      /* Initialization on the spot */
      /* Consume the = */
      fetchtok();
      /* Parse the expression */
      if(!parse_expression(par)) return PARSE_ERROR;
   }
   else {
      /* No initialization, push nil as placeholder */
      chunk_append("\tpushnil");
   }
   if(s->global) {
      /* The lvalue is a global symbol */
      chunk_append("\tgstore");
   }
   else {
      /* The lvalue is a local variable */
      chunk_append("\tlstore %" PRId64, s->pos);
   }
   return PARSE_OK;
}

int parse_fun(buzzparser_t par) {
   tokmatch(BUZZTOK_FUN);
   fetchtok();
   tokmatch(BUZZTOK_ID);
   /* Add a symbol for this function */
   sym_add(par, par->tok->value, SCOPE_AUTO);
   /* Make a new chunk for this function and get the associated symbol */
   chunk_push(sym_lookup(par->tok->value, par->symstack));
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   /* Make a new symbol table */
   symt_push();
   /* Add "self" symbol */
   const struct sym_s* sym = sym_lookup("self", par->symstack);
   if(!sym || sym->global) { sym_add(par, "self", SCOPE_LOCAL); }
   /* Parse arguments */
   if(!parse_idlist(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   /* Parse block */
   if(!parse_block(par)) return PARSE_ERROR;
   /* Add a default return */
   chunk_append("\tret0");
   /* Get rid of symbol table and close chunk */
   symt_pop();
   chunk_pop();
   return PARSE_OK;
}

int parse_if(buzzparser_t par) {
   /* Save labels for else branch and for if end */
   uint32_t lab1 = par->labels;
   uint32_t lab2 = par->labels + 1;
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
   chunk_append("\tjumpz " LABELREF "%u", lab1);
   /* Allow the use of non-cuddled brackets */
   while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
   if(!parse_blockstat(par)) return PARSE_ERROR;
   /* Eat away the newlines, if any */
   while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
   if(par->tok->type == BUZZTOK_ELSE) {
      fetchtok();
      /* Make true branch jump to label 2 => if end */
      chunk_append("\tjump " LABELREF "%u", lab2);
      /* Mark this place as label 1 and keep parsing */
      chunk_append(LABELREF "%u", lab1);
      /* Allow the use of non-cuddled brackets */
      while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
      if(!parse_blockstat(par)) return PARSE_ERROR;
      /* Mark the if end as label 2 */
      chunk_append(LABELREF "%u", lab2);
   }
   else {
      /* Mark the if end as label 1 */
      chunk_append(LABELREF "%u", lab1);
   }
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_for(buzzparser_t par) {
   tokmatch(BUZZTOK_FOR);
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   struct idrefinfo_s idrefinfo;
   if(!parse_idref(par, 1, &idrefinfo)) return PARSE_ERROR;
   tokmatch(BUZZTOK_ASSIGN);
   fetchtok();
   if(!parse_expression(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_LISTSEP);
   fetchtok();
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_LISTSEP);
   fetchtok();
   if(!parse_idref(par, 1, &idrefinfo)) return PARSE_ERROR;
   tokmatch(BUZZTOK_ASSIGN);
   fetchtok();
   if(!parse_expression(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   /* Allow the use of non-cuddled brackets */
   while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
   return parse_block(par);
}

/****************************************/
/****************************************/

int parse_while(buzzparser_t par) {
   /* Save labels for while start and end */
   uint32_t wstart = par->labels;
   uint32_t wend = par->labels + 1;
   par->labels += 2;
   /* Parse tokens */
   tokmatch(BUZZTOK_WHILE);
   fetchtok();
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   /* Place while start label */
   chunk_append(LABELREF "%u", wstart);
   /* Place the condition */
   if(!parse_condition(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   /* If the condition is false, jump to the end */
   chunk_append("\tjumpz " LABELREF "%u", wend);
   /* Allow the use of non-cuddled brackets */
   while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
   /* Parse block */
   if(!parse_blockstat(par)) return PARSE_ERROR;
   /* Jump back to while start */
   chunk_append("\tjump " LABELREF "%u", wstart);
   /* Place while end label */
   chunk_append(LABELREF "%u", wend);
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_conditionlist(buzzparser_t par,
                        int* numargs) {
   while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
   *numargs = 0;
   if(par->tok->type == BUZZTOK_PARCLOSE) return PARSE_OK;
   if(!parse_condition(par)) return PARSE_ERROR;
   ++(*numargs);
   while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
   while(par->tok->type == BUZZTOK_LISTSEP) {
      fetchtok();
      while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
      if(!parse_condition(par)) return PARSE_ERROR;
      ++(*numargs);
      while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
   }
   return PARSE_OK;
}

int parse_condition(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_NOT) {
      fetchtok();
      if(!parse_condition(par)) return PARSE_ERROR;
      chunk_append("\tnot");
      return PARSE_OK;
   }
   if(!parse_comparison(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_ANDOR) {
      char op[4];
      strcpy(op, par->tok->value);
      fetchtok();
      if(!parse_comparison(par)) return PARSE_ERROR;
      chunk_append("\t%s", op);
   }
   return PARSE_OK;
}

int parse_comparison(buzzparser_t par) {
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
      chunk_append("\t%s", op);
   }
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_expression(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_BLOCKOPEN) {
      /* Table definition */
      /* Consume the { */
      fetchtok();
      /* Eat away the newlines, if any */
      while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
      /* Make sure either id = expression or } follow */
      if(par->tok->type != BUZZTOK_DOT &&
         par->tok->type != BUZZTOK_BLOCKCLOSE) {
         fprintf(stderr,
                 "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected .id = expression or }, found %s\n",
                 buzzlex_getfile(par->lex)->fname,
                 par->tok->line,
                 par->tok->col,
                 buzztok_desc[par->tok->type]);
         return PARSE_ERROR;
      }
      /* Push empty table */
      chunk_append("\tpusht");
      if(par->tok->type == BUZZTOK_DOT) {
         /* Assignment list is present */
         /* Duplicate table on top of stack */
         chunk_append("\tdup");
         /* Consume the id */
         fetchtok();
         if(par->tok->type == BUZZTOK_ID) {
            chunk_append("\tpushs %u", string_add(par->strings, par->tok->value));
         }
         else if(par->tok->type == BUZZTOK_CONST) {
            if(strchr(par->tok->value, '.')) {
               chunk_append("\tpushf %s", par->tok->value);
            } else {
               chunk_append("\tpushi %s", par->tok->value);
            }
         }
         else {
            fprintf(stderr,
                    "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected id or numeric constant, found %s\n",
                    buzzlex_getfile(par->lex)->fname,
                    par->tok->line,
                    par->tok->col,
                    buzztok_desc[par->tok->type]);
            return PARSE_ERROR;
         }
         fetchtok();
         /* Consume the = */
         tokmatch(BUZZTOK_ASSIGN);
         fetchtok();
         /* Parse expression */
         if(!parse_expression(par)) return PARSE_ERROR;
         /* Store expression in the table */
         chunk_append("\ttput");
         /* Eat away the newlines, if any */
         while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
         /* Is there a , following? */
         while(par->tok->type == BUZZTOK_LISTSEP) {
            /* Duplicate table on top of stack */
            chunk_append("\tdup");
            /* Consume the , */
            fetchtok();
            /* Eat away the newlines, if any */
            while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
            /* Make sure .id is present */
            tokmatch(BUZZTOK_DOT);
            fetchtok();
            if(par->tok->type == BUZZTOK_ID) {
               chunk_append("\tpushs %u", string_add(par->strings, par->tok->value));
            }
            else if(par->tok->type == BUZZTOK_CONST) {
               if(strchr(par->tok->value, '.')) {
                  chunk_append("\tpushf %s", par->tok->value);
               } else {
                  chunk_append("\tpushi %s", par->tok->value);
               }
            }
            else {
               fprintf(stderr,
                       "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected id or numeric constant, found %s\n",
                       buzzlex_getfile(par->lex)->fname,
                       par->tok->line,
                       par->tok->col,
                       buzztok_desc[par->tok->type]);
               return PARSE_ERROR;
            }
            fetchtok();
            /* Make sure an = is present */
            tokmatch(BUZZTOK_ASSIGN);
            fetchtok();
            /* Parse expression */
            if(!parse_expression(par)) return PARSE_ERROR;
            /* Store expression in the table */
            chunk_append("\ttput");
            /* Eat away the newlines, if any */
            while(par->tok->type == BUZZTOK_STATEND && !par->tok->value) { fetchtok(); }
         }
      }
      tokmatch(BUZZTOK_BLOCKCLOSE);
      fetchtok();
      return PARSE_OK;
   }
   if(!parse_product(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_ADDSUB) {
      char op = par->tok->value[0];
      fetchtok();
      if(!parse_product(par)) return PARSE_ERROR;
      if     (op == '+') { chunk_append("\tadd"); }
      else if(op == '-') { chunk_append("\tsub"); }
   }
   return PARSE_OK;
}

int parse_product(buzzparser_t par) {
   if(!parse_modulo(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_MULDIV) {
      char op = par->tok->value[0];
      fetchtok();
      if(!parse_modulo(par)) return PARSE_ERROR;
      if(op == '*') {
         chunk_append("\tmul");
      }
      else if(op == '/') {
         chunk_append("\tdiv");
      }
   }
   return PARSE_OK;
}

int parse_modulo(buzzparser_t par) {
   if(!parse_power(par)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_MOD) {
      fetchtok();
      if(!parse_power(par)) return PARSE_ERROR;
      chunk_append("\tmod");
   }
   return PARSE_OK;
}

int parse_power(buzzparser_t par) {
   return parse_operand(par) && parse_powerrest(par);
}

int parse_powerrest(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_POW) {
      fetchtok();
      if(!parse_power(par)) return PARSE_ERROR;
      chunk_append("\tpow");
   }
   return PARSE_OK;
}

int parse_operand(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_FUN) {
      chunk_append("\tpushl " LABELREF "%u", par->labels);
      if(!parse_lambda(par)) return PARSE_ERROR;
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_NIL) {
      chunk_append("\tpushnil");      
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_CONST) {
      if(strchr(par->tok->value, '.')) {
         /* Floating-point constant */
         chunk_append("\tpushf %s", par->tok->value);
      }
      else {
         /* Integer constant */
         chunk_append("\tpushi %s", par->tok->value);
      }
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_STRING) {
      chunk_append("\tpushs %u", string_add(par->strings, par->tok->value));
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_PAROPEN) {
      fetchtok();
      if(!parse_condition(par)) return PARSE_ERROR;
      tokmatch(BUZZTOK_PARCLOSE);
      fetchtok();
      return PARSE_OK;
   }
   else if(par->tok->type == BUZZTOK_ADDSUB) {
      char op = par->tok->value[0];
      fetchtok();
      if(par->tok->type == BUZZTOK_CONST) {
         if(strchr(par->tok->value, '.')) {
            /* Floating-point constant */
            chunk_append("\tpushf %c%s", op, par->tok->value);
         }
         else {
            /* Integer constant */
            chunk_append("\tpushi %c%s", op, par->tok->value);
         }
         fetchtok();
         return PARSE_OK;
      }
      else {
         if(!parse_power(par)) return PARSE_ERROR;
         if(op == '-') chunk_append("\tunm");
         return PARSE_OK;
      }
   }
   struct idrefinfo_s  idrefinfo;
   return parse_idref(par, 0, &idrefinfo);
}

/****************************************/
/****************************************/

int parse_command(buzzparser_t par) {
   if(par->tok->type == BUZZTOK_RETURN) {
      /* Return statement */
      fetchtok();
      if(par->tok->type == BUZZTOK_STATEND ||
         par->tok->type == BUZZTOK_BLOCKCLOSE) {
         chunk_append("\tret0");
      }
      else {
         if(!parse_condition(par)) return PARSE_ERROR;
         chunk_append("\tret1");
      }
      return PARSE_OK;
   }
   else {
      /* Function call or assignment, both begin with an id */
      struct idrefinfo_s idrefinfo;
      if(!parse_idref(par, 1, &idrefinfo)) return PARSE_ERROR;
      if(par->tok->type == BUZZTOK_ASSIGN) {
         /* Is lvalue a closure? ERROR */
         if(idrefinfo.info == TYPE_CLOSURE) {
            fprintf(stderr,
                    "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: can't have a function call as lvalue\n",
                    buzzlex_getfile(par->lex)->fname,
                    par->tok->line,
                    par->tok->col);
            return PARSE_ERROR;
         }
         /* lvalue is OK */
         /* Is lvalue a global symbol? If so, push its string id */
         if(idrefinfo.global) chunk_append("\tpushs %d", idrefinfo.info);
         /* Consume the = */
         fetchtok();
         /* Parse the expression */
         if(!parse_expression(par)) return PARSE_ERROR;
         if(idrefinfo.global) {
            /* The lvalue is a global symbol, just add gstore */
            chunk_append("\tgstore");
         }
         else {
            /* The lvalue is a local symbol or a table reference */
            if(idrefinfo.info >= 0) {
               /* Local variable */
               chunk_append("\tlstore %d", idrefinfo.info);
            }
            else if(idrefinfo.info == TYPE_TABLE) {
               /* Table reference */
               chunk_append("\ttput");
            }
         }
         return PARSE_OK;
      }
      else if(idrefinfo.info == TYPE_CLOSURE)
         return PARSE_OK;
      fprintf(stderr,
              "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected function call or assignment\n",
              buzzlex_getfile(par->lex)->fname,
              par->tok->line,
              par->tok->col);
      return PARSE_ERROR;
   }
}

/****************************************/
/****************************************/

int parse_idlist(buzzparser_t par) {
   /* Empty list */
   if(par->tok->type == BUZZTOK_PARCLOSE)
      return PARSE_OK;
   /* Match an id for the first argument */
   tokmatch(BUZZTOK_ID);
   /* Look for the argument symbol in the context
    * If a symbol is found and is not global,
    * do not add a new symbol; otherwise do
    * This allows lambdas and nested functions to
    * reuse the parameters and local variables of
    * the parent
    */
   const struct sym_s* sym = sym_lookup(par->tok->value, par->symstack);
   if(!sym || sym->global) {
      sym_add(par, par->tok->value, SCOPE_LOCAL);
   }
   fetchtok();
   /* Match rest of the argument list */
   while(par->tok->type == BUZZTOK_LISTSEP) {
      fetchtok();
      tokmatch(BUZZTOK_ID);
      const struct sym_s* sym = sym_lookup(par->tok->value, par->symstack);
      if(!sym || sym->global) {
         sym_add(par, par->tok->value, SCOPE_LOCAL);
      }
      fetchtok();
   }
   return PARSE_OK;
}

int parse_idreflist(buzzparser_t par) {
   while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
   if(par->tok->type == BUZZTOK_PARCLOSE)
      return PARSE_OK;
   struct idrefinfo_s idrefinfo;
   if(!parse_idref(par, 0, &idrefinfo)) return PARSE_ERROR;
   while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
   while(par->tok->type == BUZZTOK_LISTSEP) {
      fetchtok();
      while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
      if(!parse_idref(par, 0, &idrefinfo)) return PARSE_ERROR;
      while(par->tok->type == BUZZTOK_STATEND) { fetchtok(); }
   }
   if(par->tok && par->tok->type == BUZZTOK_PARCLOSE) {
      return PARSE_OK;
   }
   fprintf(stderr,
           "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected , or ), found %s\n",
           buzzlex_getfile(par->lex)->fname,
           par->tok->line,
           par->tok->col,
           buzztok_desc[par->tok->type]);
   return PARSE_ERROR;
}

int parse_idref(buzzparser_t par,
                int lvalue,
                struct idrefinfo_s* idrefinfo) {
   /* Start with an id */
   tokmatch(BUZZTOK_ID);
   /* Look it up in the symbol table */
   const struct sym_s* s = sym_lookup(par->tok->value, par->symstack);
   if(!s) {
      /* Symbol not found, add it */
      sym_add(par, par->tok->value, SCOPE_GLOBAL);
      s = sym_lookup(par->tok->value, par->symstack);
   }
   /* Save symbol info */
   idrefinfo->info = s->pos;
   idrefinfo->global = s->global;
   /* Go on parsing the reference */
   fetchtok();
   chunk_buf_push();
   while(par->tok->type == BUZZTOK_DOT ||
         par->tok->type == BUZZTOK_IDXOPEN ||
         par->tok->type == BUZZTOK_PAROPEN) {
      /* Take care of structured types */
      if(idrefinfo->global) {
         // If the next token is a closure and is not called from a table, we push nil for the self table.
         if(par->tok->type == BUZZTOK_PAROPEN)
            chunk_append("\tpushnil");
         chunk_append("\tpushs %d", idrefinfo->info);
         chunk_append("\tgload");
      }
      else if(idrefinfo->info >= 0) {
         // If the next token is a closure and is not called from a table, we push nil for the self table.
         if(par->tok->type == BUZZTOK_PAROPEN)
            chunk_append("\tpushnil");
         chunk_append("\tlload %" PRId64, s->pos);
      }
      else if(idrefinfo->info == TYPE_TABLE)   { chunk_append("\ttget"); }
      else if(idrefinfo->info == TYPE_CLOSURE) { chunk_append("\tcallc"); }
      idrefinfo->global = 0;
      /* Go on parsing structure type */
      if(par->tok->type == BUZZTOK_DOT) {
         idrefinfo->info = TYPE_TABLE;
         fetchtok();
         tokmatch(BUZZTOK_ID);
         uint32_t tmp = string_add(par->strings, par->tok->value);
         fetchtok();
         if(par->tok->type == BUZZTOK_PAROPEN)
            chunk_append("\tdup");
         chunk_append("\tpushs %u", tmp);
      }
      else if(par->tok->type == BUZZTOK_IDXOPEN) {
         idrefinfo->info = TYPE_TABLE;
         fetchtok();
         {
            chunk_buf_push();
            if(!parse_expression(par)) return PARSE_ERROR;
            tokmatch(BUZZTOK_IDXCLOSE);
            fetchtok();
            if(par->tok->type == BUZZTOK_PAROPEN)
               chunk_buf_append("\tdup");
            chunk_buf_pop();
         }
      }
      else if(par->tok->type == BUZZTOK_PAROPEN) {
         idrefinfo->info = TYPE_CLOSURE;
         fetchtok();
         int numargs;
         if(!parse_conditionlist(par, &numargs)) return PARSE_ERROR;
         tokmatch(BUZZTOK_PARCLOSE);
         fetchtok();
         if(par->tok->type == BUZZTOK_PAROPEN)
            chunk_buf_append("\tpushnil");
         chunk_append("\tpushi %d", numargs);
      }
   }
   if(!lvalue ||
      idrefinfo->info == TYPE_CLOSURE) {
      if(idrefinfo->global) {
         chunk_append("\tpushs %d", idrefinfo->info);
         chunk_append("\tgload");
      }
      else if(idrefinfo->info >= 0) {
         chunk_append("\tlload %d", idrefinfo->info);
      }
      else if(idrefinfo->info == TYPE_TABLE) {
         chunk_append("\ttget");
      }
      else if(idrefinfo->info == TYPE_CLOSURE) {
         chunk_append("\tcallc");
      }
   }
   chunk_buf_pop();
   return PARSE_OK;
}

/****************************************/
/****************************************/

int parse_lambda(buzzparser_t par) {
   tokmatch(BUZZTOK_FUN);
   fetchtok();
   /* Make a new chunk for this function and get the associated symbol */
   chunk_push(NULL);
   tokmatch(BUZZTOK_PAROPEN);
   fetchtok();
   /* Check whether it's necessary to create a brand new symbol table
    * or clone the current one
    * If the parent symtable is the global one, it is necessary;
    * otherwise, clone the parent's
    */
   if(buzzdarray_size(par->symstack) == 1) {
      /* Add new symtable */
      symt_push();
      /* Add "self" symbol */
      const struct sym_s* sym = sym_lookup("self", par->symstack);
      if(!sym || sym->global) { sym_add(par, "self", SCOPE_LOCAL); }
   }
   else {
      symt_clone();
   }
   /* Parse lambda arguments */
   if(!parse_idlist(par)) return PARSE_ERROR;
   tokmatch(BUZZTOK_PARCLOSE);
   fetchtok();
   /* Parse block */
   if(!parse_block(par)) return PARSE_ERROR;
   /* Add a default return */
   chunk_append("\tret0");
   /* Get rid of symbol table and close chunk */
   symt_pop();
   chunk_pop();
   return PARSE_OK;
}

/****************************************/
/****************************************/

buzzparser_t buzzparser_new(int argc,
                            char** argv) {
   /* Argument parsing */
   if(argc < 3 || argc > 4) {
      fprintf(stderr, "buzzparser_new(): expected 3 or 4 arguments, got %d\n", argc);
      return NULL;
   }
   /* Create parser state */
   buzzparser_t par = (buzzparser_t)malloc(sizeof(struct buzzparser_s));
   /* Create lexer */
   par->lex = buzzlex_new(argv[1]);
   if(!par->lex) {
      free(par);
      return NULL;
   }
   /* Copy the script file name */
   par->scriptfn = strdup(argv[1]);
   /* Copy string */
   par->asmfn = strdup(argv[2]);
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
   /* Initialize chunk list */
   par->chunks = buzzdarray_new(1, sizeof(chunk_t), chunk_destroy);
   /* Initialize symbol table stack */
   par->symstack = buzzdarray_new(10, sizeof(buzzdict_t), symt_destroy);
   par->syms = NULL;
   /* Initialize string list */
   par->strings = buzzdict_new(100,
                               sizeof(char*),
                               sizeof(uint16_t),
                               buzzdict_strkeyhash,
                               buzzdict_strkeycmp,
                               NULL);
   /* If 4 arguments were passed, we have a symbol table to parse  */
   if(argc == 4) {
      /* Open the file */
      FILE* stf = fopen(argv[3], "r");
      if(!stf) {
         perror(argv[3]);
         return NULL;
      }
      /* Read the file line by line */
      size_t len;
      char line[1024];
      while(fgets(line, 1024, stf)) {
         /* For each line, add the string to par->strings */
         len = strlen(line);
         if(len > 0 && line[len-1] == '\n') line[len-1] = 0;
         string_add(par->strings, line);
      }
      /* Are we done because of an error? */
      if(ferror(stf)) {
         perror(argv[3]);
         return NULL;
      }
      /* Done with file */
      fclose(stf);
   }
   /* Return parser state */
   return par;
}

/****************************************/
/****************************************/

void buzzparser_destroy(buzzparser_t* par) {
   buzzdict_destroy(&((*par)->strings));
   buzzdarray_destroy(&((*par)->chunks));
   buzzdarray_destroy(&((*par)->symstack));
   free((*par)->asmfn);
   fclose((*par)->asmstream);
   free((*par)->scriptfn);
   buzzlex_destroy(&((*par)->lex));
   if((*par)->tok) buzzlex_destroytok(&((*par)->tok));
   free(*par);
   *par = NULL;
}

/****************************************/
/****************************************/

int buzzparser_parse(buzzparser_t par) {
   /*
    * Parse the script
    */
   if(!parse_script(par)) return PARSE_ERROR;
   /*
    * Write to file
    */
   /* Write strings */
   fprintf(par->asmstream, "!%u\n", buzzdict_size(par->strings));
   buzzdarray_t sarr = buzzdarray_new(10, sizeof(struct strarray_data_s*), string_destroy);
   buzzdict_foreach(par->strings, string_copy, sarr);
   buzzdarray_sort(sarr, string_cmp);
   buzzdarray_foreach(sarr, string_print, par->asmstream);
   buzzdarray_destroy(&sarr);
   fprintf(par->asmstream, "\n");
   /* Write chunk registration code (end it with a nop) */
   buzzdarray_foreach(par->chunks, chunk_register, par->asmstream);
   fprintf(par->asmstream, "\tnop\n");
   /* Write actual chunks */
   buzzdarray_foreach(par->chunks, chunk_print, par->asmstream);
   return PARSE_OK;
}

/****************************************/
/****************************************/
