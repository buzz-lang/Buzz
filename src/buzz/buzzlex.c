#include "buzzlex.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>

/****************************************/
/****************************************/

char *buzztok_desc[] = {
      "identifier", "numeric constant", "string", "variable",
      "nil", "if", "else", "function", "return",
      "for", "while", "and/or", "not", "+ or -", "* or /",
      "%", "^", "{", "}", "(", ")", "[", "]", "; or newline",
      ",", "=", ".", "== != < <= > >=" };

/****************************************/
/****************************************/

buzzlex_file_t buzzlex_file_new(const char* fname) {
   /* Find the file, possibly using the include path */
   char fpath[PATH_MAX];
   strcpy(fpath, fname);
   FILE* fd = fopen(fpath, "rb");
   if(!fd) {
      /* Get the include path from the environment, if present */
      if(!getenv("BUZZ_INCLUDE_PATH")) {
         return NULL;
      }
      /* Fetch the directories in the include path */
      char* incpath = strdup(getenv("BUZZ_INCLUDE_PATH"));
      char* curpath = incpath;
      char* dir = strsep(&curpath, ":");
      /* Go through them and try to open the file */
      while(dir && !fd) {
         /* Make sure it's not an empty field */
         if(dir[0] != '\0') {
            /* fpath = dir */
            strcpy(fpath, dir);
            /* Add / at the end if missing */
            if(fpath[strlen(fpath)-1] != '/') strcat(fpath, "/");
            /* fpath += fname */
            strcat(fpath, fname);
            /* Try to open the file */
            fd = fopen(fpath, "rb");
         }
         /* Get next dir */
         dir = strsep(&curpath, ":");
      }
      free(incpath);
      /* Did we find the file? */
      if(!dir && !fd) {
         /* Nope */
         return NULL;
      }
   }
   /* Get absolute path; this internally creates a new string in the heap */
   char* afpath = realpath(fpath, NULL);
   /* Create memory structure */
   buzzlex_file_t x = (buzzlex_file_t)malloc(sizeof(struct buzzlex_file_s));
   /* Get the file size */
   fseek(fd, 0, SEEK_END);
   x->buf_size = ftell(fd);
   rewind(fd);
   /* Create a buffer large enough to contain the data */
   x->buf = (char*)malloc(x->buf_size + 2);
   /* Copy the content of the file in the buffer */
   if(fread(x->buf, 1, x->buf_size, fd) < x->buf_size) {
      /* Read error */
      fclose(fd);
      free(x->buf);
      free(x);
      return NULL;
   }
   x->buf[x->buf_size] = '\n';
   x->buf[x->buf_size+1] = '\0';
   /* Done reading, close file */
   fclose(fd);
   /* Store the file name */
   x->fname = afpath;
   /* Initialize line and column counters */
   x->cur_line = 1;
   x->cur_col = 0;
   x->cur_c = 0;
   return x;
}

void buzzlex_file_destroy(uint32_t pos, void* data, void* params) {
   buzzlex_file_t f = *(buzzlex_file_t*)data;
   free(f->buf);
   free(f->fname);
   free(f);
}

int buzzlex_file_cmp(const void* a, const void* b) {
   buzzlex_file_t f1 = *(buzzlex_file_t*)a;
   buzzlex_file_t f2 = *(buzzlex_file_t*)b;
   return strcmp(f1->fname, f2->fname);
}

/****************************************/
/****************************************/

static int buzzlex_isspace(char c) {
   return (c == ' ') || (c == '\t') || (c == '\r');
}

static int buzzlex_isid(char c) {
   return isalnum(c) || (c == '_');
}

static int buzzlex_isarith(char c) {
   return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '%') || (c == '^');
}

static int buzzlex_isquote(char c) {
   return (c == '\'') || (c == '"');
}

static int buzzlex_isnumber(char c) {
   return isdigit(c) || (c == '.');
}

/****************************************/
/****************************************/

static buzztok_t buzzlex_newtok(buzztok_type_e type,
                                char* value,
                                uint64_t line,
                                uint64_t col,
                                char* fname) {
   buzztok_t retval = (buzztok_t)malloc(sizeof(struct buzztok_s));
   retval->type = type;
   retval->value = value;
   retval->line = line;
   retval->col = col + 1;
   retval->fname = strdup(fname);
   return retval;
}

/****************************************/
/****************************************/

buzzlex_t buzzlex_new(const char* fname) {
   /* The lexer corresponds to a stack of file information */
   buzzlex_t x = buzzdarray_new(10,
                                sizeof(struct buzzlex_file_s*),
                                buzzlex_file_destroy);
   /* Read file */
   buzzlex_file_t f = buzzlex_file_new(fname);
   if(!f) return NULL;
   buzzdarray_push(x, &f);
   /* Return the lexer state */
   return x;
}

/****************************************/
/****************************************/

#define nextchar() ++lexf->cur_c; ++lexf->cur_col;

#define casetokchar(CHAR, TOKTYPE)               \
   case (CHAR): {                                \
      return buzzlex_newtok(TOKTYPE,             \
                            NULL,                \
                            lexf->cur_line,      \
                            lexf->cur_col,       \
                            lexf->fname);        \
   }

#define readval(CHARCOND)                                         \
   size_t start = lexf->cur_c - 1;                                \
   while(lexf->cur_c < lexf->buf_size &&                          \
         CHARCOND(lexf->buf[lexf->cur_c])) {                      \
      nextchar();                                                 \
   }                                                              \
   char* val = (char*)malloc(lexf->cur_c - start + 1);            \
   strncpy(val, lexf->buf + start, lexf->cur_c - start);          \
   val[lexf->cur_c - start] = '\0';

#define checkkeyword(KW, TOKTYPE)                \
   if(strcmp(val, KW) == 0)                      \
      return buzzlex_newtok(TOKTYPE,             \
                            val,                 \
                            lexf->cur_line,      \
                            lexf->cur_col,       \
                            lexf->fname);

buzztok_t buzzlex_nexttok(buzzlex_t lex) {
   buzzlex_file_t lexf = buzzlex_getfile(lex);
   do {
      /* Look for a non-space character */
      do {
         /* Keep reading until you find a non-space character or end of stream */
         while(lexf->cur_c < lexf->buf_size &&
               buzzlex_isspace(lexf->buf[lexf->cur_c])) {
            nextchar();
         }
         /* End of stream? */
         if(lexf->cur_c >= lexf->buf_size) {
            /* Done with current file, go back to previous */
            buzzdarray_pop(lex);
            if(buzzdarray_isempty(lex))
               /* No file to go back to, done parsing */
               return NULL;
            lexf = buzzlex_getfile(lex);
         }
         else
            /* Non-space character found */
            break;
      } while(1);
      /* Non-space character found */
      /* If the current character is a '#' ignore the rest of the line */
      if(lexf->buf[lexf->cur_c] == '#') {
         do {
            nextchar();
         }
         while(lexf->cur_c < lexf->buf_size &&
               lexf->buf[lexf->cur_c] != '\n');
         /* End of stream? */
         if(lexf->cur_c >= lexf->buf_size) {
            /* Done with current file, go back to previous */
            buzzdarray_pop(lex);
            if(buzzdarray_isempty(lex))
               /* No file to go back to, done parsing */
               return NULL;
            lexf = buzzlex_getfile(lex);
         }
         else {
            /* New line and carry on */
            ++lexf->cur_line;
            lexf->cur_col = 0;
            ++lexf->cur_c;
         }
      }
      else if(strncmp(lexf->buf + lexf->cur_c, "include", 7) == 0) {
         /* Manage file inclusion */
         lexf->cur_c += 7;
         lexf->cur_col += 7;
         /* Skip whitespace */
         while(lexf->cur_c < lexf->buf_size &&
               buzzlex_isspace(lexf->buf[lexf->cur_c])) {
            nextchar();
         }
         /* End of file or not-string opening -> syntax error */
         if(lexf->cur_c >= lexf->buf_size ||
            !buzzlex_isquote(lexf->buf[lexf->cur_c])) {
            fprintf(stderr,
                    "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected string after include\n",
                    lexf->fname,
                    lexf->cur_line,
                    lexf->cur_col);
            return NULL;
         }
         /* Read string */
         char quote = lexf->buf[lexf->cur_c];
         size_t start = lexf->cur_c + 1;
         nextchar();
         while(lexf->cur_c < lexf->buf_size &&
               lexf->buf[lexf->cur_c] != quote &&
               lexf->buf[lexf->cur_c] != '\n') {
            nextchar();
         }
         /* End of file or newline -> syntax error */
         if(lexf->cur_c >= lexf->buf_size ||
            lexf->buf[lexf->cur_c] == '\n') {
            fprintf(stderr,
                    "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected end of string\n",
                    lexf->fname,
                    lexf->cur_line,
                    lexf->cur_col);
            return NULL;
         }
         /* Copy data into a new string */
         char* fname = (char*)malloc(lexf->cur_c - start + 1);
         strncpy(fname, lexf->buf + start, lexf->cur_c - start);
         fname[lexf->cur_c - start] = '\0';
         /* Get to next character in this file */
         nextchar();
         /* Create new file structure */
         buzzlex_file_t f = buzzlex_file_new(fname);
         if(!f) {
            fprintf(stderr,
                    "%s:%" PRIu64 ":%" PRIu64 ": Can't read '%s'\n",
                    lexf->fname,
                    lexf->cur_line,
                    lexf->cur_col,
                    fname);
            free(fname);
            return NULL;
         }
         free(fname);
         /* Make sure the file hasn't been already included */
         if(buzzdarray_find(lex, buzzlex_file_cmp, &f) < buzzdarray_size(lex)) {
            buzzlex_file_destroy(0, &f, NULL);
         }
         else {
            /* Push file structure */
            buzzdarray_push(lex, &f);
            lexf = buzzlex_getfile(lex);
         }
      }
      else
         /* The character must be parsed */
         break;
   }
   while(1);
   /* If we get here it's because we read potential token character */
   char c = lexf->buf[lexf->cur_c];
   nextchar();
   /* Consider the 1-char non-alphanumeric cases first */
   switch(c) {
      case '\n': {
         buzztok_t tok = buzzlex_newtok(BUZZTOK_STATEND,
                                        NULL,
                                        lexf->cur_line,
                                        lexf->cur_col,
                                        lexf->fname);
         ++lexf->cur_line;
         lexf->cur_col = 0;
         return tok;
      }
      casetokchar(';', BUZZTOK_STATEND);
      casetokchar('{', BUZZTOK_BLOCKOPEN);
      casetokchar('}', BUZZTOK_BLOCKCLOSE);
      casetokchar('(', BUZZTOK_PAROPEN);
      casetokchar(')', BUZZTOK_PARCLOSE);
      casetokchar('[', BUZZTOK_IDXOPEN);
      casetokchar(']', BUZZTOK_IDXCLOSE);
      casetokchar(',', BUZZTOK_LISTSEP);
      casetokchar('.', BUZZTOK_DOT);
   }
   /* If we get here, it's because we found either a constant, an
    * identifier, a keyword, an assignment, a comparison operator,
    * an arithmetic operator, or an unexpected character */
   if(isdigit(c)) {
      /* It's a constant */
      readval(buzzlex_isnumber);
      return buzzlex_newtok(BUZZTOK_CONST,
                            val,
                            lexf->cur_line,
                            lexf->cur_col,
                            lexf->fname);
   }
   else if(isalpha(c)) {
      /* It's either a keyword or an identifier */
      readval(buzzlex_isid);
      /* Go through the possible keywords */
      checkkeyword("var",      BUZZTOK_VAR);
      checkkeyword("if",       BUZZTOK_IF);
      checkkeyword("else",     BUZZTOK_ELSE);
      checkkeyword("function", BUZZTOK_FUN);
      checkkeyword("return",   BUZZTOK_RETURN);
      checkkeyword("for",      BUZZTOK_FOR);
      checkkeyword("while",    BUZZTOK_WHILE);
      checkkeyword("and",      BUZZTOK_ANDOR);
      checkkeyword("or",       BUZZTOK_ANDOR);
      checkkeyword("not",      BUZZTOK_NOT);
      checkkeyword("nil",      BUZZTOK_NIL);
      /* No keyword found, consider it an id */
      return buzzlex_newtok(BUZZTOK_ID,
                            val,
                            lexf->cur_line,
                            lexf->cur_col,
                            lexf->fname);
   }
   else if(c == '=') {
      /* Either an assignment or a comparison */
      if(lexf->cur_c < lexf->buf_size &&
         lexf->buf[lexf->cur_c] == '=') {
         /* It's a comparison */
         nextchar();
         return buzzlex_newtok(BUZZTOK_CMP,
                               strdup("=="),
                               lexf->cur_line,
                               lexf->cur_col,
                               lexf->fname);
      }
      else {
         /* It's an assignment */
         return buzzlex_newtok(BUZZTOK_ASSIGN,
                               NULL,
                               lexf->cur_line,
                               lexf->cur_col,
                               lexf->fname);
      }
   }
   else if(c == '!') {
      /* Comparison operator? */
      if(lexf->cur_c < lexf->buf_size &&
         lexf->buf[lexf->cur_c] == '=') {
         /* It's a comparison */
         nextchar();
         return buzzlex_newtok(BUZZTOK_CMP,
                               strdup("!="),
                               lexf->cur_line,
                               lexf->cur_col,
                               lexf->fname);
      }
      else {
         /* Syntax error */
         fprintf(stderr,
                 "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: expected '=' after '!'\n",
                 lexf->fname,
                 lexf->cur_line,
                 lexf->cur_col);
         return NULL;
      }
   }
   else if((c == '<') || (c == '>')) {
      /* It's a comparison operator */
      size_t start = lexf->cur_c - 1;
      /* Include the '=' if present */
      if(lexf->cur_c < lexf->buf_size &&
         lexf->buf[lexf->cur_c] == '=') {
         nextchar();
      }
      char* val = (char*)malloc(lexf->cur_c - start + 1);
      strncpy(val, lexf->buf + start, lexf->cur_c - start);
      val[lexf->cur_c - start] = 0;
      return buzzlex_newtok(BUZZTOK_CMP,
                            val,
                            lexf->cur_line,
                            lexf->cur_col,
                            lexf->fname);
   }
   else if(buzzlex_isarith(c)) {
      /* Arithmetic operator */
      char* val = (char*)malloc(2);
      strncpy(val, lexf->buf + lexf->cur_c - 1, 1);
      val[1] = 0;
      switch(c) {
         case '+': case '-': {
            return buzzlex_newtok(BUZZTOK_ADDSUB,
                                  val,
                                  lexf->cur_line,
                                  lexf->cur_col,
                                  lexf->fname);
         }
         case '*': case '/': {
            return buzzlex_newtok(BUZZTOK_MULDIV,
                                  val,
                                  lexf->cur_line,
                                  lexf->cur_col,
                                  lexf->fname);
         }
         case '%': {
            return buzzlex_newtok(BUZZTOK_MOD,
                                  val,
                                  lexf->cur_line,
                                  lexf->cur_col,
                                  lexf->fname);
         }
         case '^': {
            return buzzlex_newtok(BUZZTOK_POW,
                                  val,
                                  lexf->cur_line,
                                  lexf->cur_col,
                                  lexf->fname);
         }
         default:
            return NULL;
      }
   }
   else if(buzzlex_isquote(c)) {
      /* String - eat any character until you find the next matching quote */
      size_t start = lexf->cur_c;
      while(lexf->cur_c < lexf->buf_size &&
            lexf->buf[lexf->cur_c] != c) {
         if(lexf->buf[lexf->cur_c] != '\n') {
            nextchar();
         }
         else {
            ++lexf->cur_line;
            lexf->cur_col = 0;
            ++lexf->cur_c;
         }
      }
      /* End of stream? Syntax error */
      if(lexf->cur_c >= lexf->buf_size) {
         fprintf(stderr,
                 "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: string closing quote not found\n",
                 lexf->fname,
                 lexf->cur_line,
                 lexf->cur_col);
         return NULL;
      }
      /* Valid string */
      char* val = (char*)malloc(lexf->cur_c - start + 1);
      strncpy(val, lexf->buf + start, lexf->cur_c - start);
      val[lexf->cur_c - start] = '\0';
      nextchar();
      return buzzlex_newtok(BUZZTOK_STRING,
                            val,
                            lexf->cur_line,
                            lexf->cur_col,
                            lexf->fname);
   }
   else {
      /* Unknown character */
      fprintf(stderr,
              "%s:%" PRIu64 ":%" PRIu64 ": Syntax error: unknown character '%c' (octal: %o; hex: %x)\n",
              lexf->fname,
              lexf->cur_line,
              lexf->cur_col,
              c, c, c);
      return NULL;
   }
}

/****************************************/
/****************************************/

buzztok_t buzzlex_clonetok(buzztok_t tok) {
   return buzzlex_newtok(tok->type,
                         tok->value ? strdup(tok->value) : NULL,
                         tok->line,
                         tok->col,
                         tok->fname);
}

/****************************************/
/****************************************/

void buzzlex_destroytok(buzztok_t* tok) {
   free((*tok)->fname);
   free((*tok)->value);
   free(*tok);
   *tok = NULL;
}

/****************************************/
/****************************************/
