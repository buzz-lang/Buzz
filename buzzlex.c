#include "buzzlex.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/uio.h>

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

/****************************************/
/****************************************/

static buzztok_t buzzlex_newtok(buzztok_type_e type,
                                char* value,
                                uint64_t line,
                                uint64_t col) {
   buzztok_t retval = (buzztok_t)malloc(sizeof(struct buzztok_s));
   retval->type = type;
   retval->value = value;
   retval->line = line;
   retval->col = col;
   return retval;
}

/****************************************/
/****************************************/

buzzlex_t buzzlex_new(const char* fname) {
   /* Create the lexer - calloc() zeroes everything by default */
   buzzlex_t retval = (buzzlex_t)calloc(1, sizeof(struct buzzlex_s));
   /* Open the file */
   int fd = open(fname, O_RDONLY);
   if(fd < 0) {
      perror(fname);
      free(retval);
      return NULL;
   }
   /* Get the file size */
   retval->buf_size = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   /* Create a buffer large enough to contain the data */
   retval->buf = (char*)malloc(retval->buf_size);
   /* Copy the content of the file in the buffer */
   ssize_t readnow;
   size_t readsofar = 0;
   do {
      readnow = read(fd,
                     retval->buf + readsofar,
                     retval->buf_size - readsofar);
      if(readnow == -1) {
         /* Read error */
         free(retval->buf);
         free(retval);
         perror(fname);
         return NULL;
      }
      else {
         /* More characters read */
         readsofar += readnow;
      }
   }
   while(readnow > 0);
   retval->buf[retval->buf_size] = '\n';
   /* Done reading, close file */
   if(close(fd) < 0) {
      /* I/O error */
      free(retval->buf);
      free(retval);
      perror(fname);
      return NULL;
   }
   /* Copy the file name */
   retval->fname = strdup(fname);
   /* Copy the file name */
   retval->cur_line = 1;
   retval->cur_col = 0;
   /* Return the lexer */
   return retval;
}

/****************************************/
/****************************************/

void buzzlex_destroy(buzzlex_t* lex) {
   free((*lex)->buf);
   free((*lex)->fname);
   free(*lex);
   *lex = NULL;
}

/****************************************/
/****************************************/

#define nextchar() ++lex->cur_c; ++lex->cur_col;


#define casetokchar(CHAR, TOKTYPE)              \
   case (CHAR): {                               \
      return buzzlex_newtok(TOKTYPE,            \
                            NULL,               \
                            lex->cur_line,      \
                            lex->cur_col);      \
   }

#define readval(CHARCOND)                                       \
   size_t start = lex->cur_c - 1;                               \
   while(lex->cur_c < lex->buf_size &&                          \
         CHARCOND(lex->buf[lex->cur_c])) {                      \
      nextchar();                                               \
   }                                                            \
   char* val = (char*)malloc(lex->cur_c - start + 1);           \
   strncpy(val, lex->buf + start, lex->cur_c - start);          \
   val[lex->cur_c - start] = '\0';

#define checkkeyword(KW, TOKTYPE)               \
   if(strcmp(val, KW) == 0)                     \
      return buzzlex_newtok(TOKTYPE,            \
                            val,                \
                            lex->cur_line,      \
                            lex->cur_col);

buzztok_t buzzlex_nexttok(buzzlex_t lex) {
   do {
      /* Keep reading until you find a non-space character or end of stream */
      while(lex->cur_c < lex->buf_size &&
            buzzlex_isspace(lex->buf[lex->cur_c])) {
         nextchar();
      }
      /* End of stream? No token */
      if(lex->cur_c >= lex->buf_size) return NULL;
      /* If the current character is a '#' ignore the rest of the line */
      if(lex->buf[lex->cur_c] == '#') {
         do {
            nextchar();
         }
         while(lex->cur_c < lex->buf_size &&
               lex->buf[lex->cur_c] != '\n');
         /* End of stream? No token */
         if(lex->cur_c >= lex->buf_size) return NULL;
         /* New line and carry on */
         ++lex->cur_line;
         lex->cur_col = 0;
         ++lex->cur_c;
      }
      else {
         /* The current character must be parsed */
         break;
      }
   }
   while(1);
   /* If we get here it's because we read a non-space character */
   char c = lex->buf[lex->cur_c];
   nextchar();
   /* Consider the 1-char non-alphanumeric cases first */
   switch(c) {
      case '\n': {
         buzztok_t tok = buzzlex_newtok(BUZZTOK_STATEND,
                                        NULL,
                                        lex->cur_line,
                                        lex->cur_col);
         ++lex->cur_line;
         lex->cur_col = 0;
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
      casetokchar('=', BUZZTOK_ASSIGN);
      casetokchar('.', BUZZTOK_DOT);
   }
   /* If we get here, it's because we found either a constant, an
    * identifier, a keyword, an assignment, a comparison operator,
    * an arithmetic operator, or an unexpected character */
   if(isdigit(c)) {
      /* It's a constant */
      readval(isnumber);
      return buzzlex_newtok(BUZZTOK_CONST,
                            val,
                            lex->cur_line,
                            lex->cur_col);
   }
   else if(isalpha(c)) {
      /* It's either a keyword or an identifier */
      readval(buzzlex_isid);
      /* Go through the possible keywords */
      checkkeyword("local",    BUZZTOK_LOCAL);
      checkkeyword("if",       BUZZTOK_IF);
      checkkeyword("else",     BUZZTOK_ELSE);
      checkkeyword("function", BUZZTOK_FUN);
      checkkeyword("for",      BUZZTOK_FOR);
      checkkeyword("while",    BUZZTOK_WHILE);
      checkkeyword("and",      BUZZTOK_ANDOR);
      checkkeyword("or",       BUZZTOK_ANDOR);
      checkkeyword("not",      BUZZTOK_NOT);
      checkkeyword("true",     BUZZTOK_BOOL);
      checkkeyword("false",    BUZZTOK_BOOL);
      /* No keyword found, consider it an id */
      return buzzlex_newtok(BUZZTOK_ID,
                            val,
                            lex->cur_line,
                            lex->cur_col);
   }
   else if(c == '=') {
      /* Either an assignment or a comparison */
      if(lex->cur_c < lex->buf_size &&
         lex->buf[lex->cur_c] == '=') {
         /* It's a comparison */
         nextchar();
         return buzzlex_newtok(BUZZTOK_CMP,
                               strdup("=="),
                               lex->cur_line,
                               lex->cur_col);
      }
      else {
         /* It's an assignment */
         return buzzlex_newtok(BUZZTOK_ASSIGN,
                               NULL,
                               lex->cur_line,
                               lex->cur_col);
      }
   }
   else if(c == '!') {
      /* Comparison operator? */
      if(lex->cur_c < lex->buf_size &&
         lex->buf[lex->cur_c] == '=') {
         /* It's a comparison */
         nextchar();
         return buzzlex_newtok(BUZZTOK_CMP,
                               strdup("!="),
                               lex->cur_line,
                               lex->cur_col);
      }
      else {
         /* Syntax error */
         fprintf(stderr,
                 "%s:%llu:%llu: Syntax error: expected '=' after '!'\n",
                 lex->fname,
                 lex->cur_line,
                 lex->cur_col);
         return NULL;
      }
   }
   else if((c == '<') || (c == '>')) {
      /* It's a comparison operator */
      size_t start = lex->cur_c - 1;
      /* Include the '=' if present */
      if(lex->cur_c < lex->buf_size &&
         lex->buf[lex->cur_c] == '=') {
         nextchar();
      }
      char* val = (char*)malloc(lex->cur_c - start + 1);
      strncpy(val, lex->buf + start, lex->cur_c - start);
      val[lex->cur_c - start] = 0;
      return buzzlex_newtok(BUZZTOK_ASSIGN,
                            val,
                            lex->cur_line,
                            lex->cur_col);
   }
   else if(buzzlex_isarith(c)) {
      /* Arithmetic operator */
      char* val = (char*)malloc(2);
      strncpy(val, lex->buf + lex->cur_c - 1, 1);
      val[1] = 0;
      switch(c) {
         case '+': case '-': {
            return buzzlex_newtok(BUZZTOK_ADDSUB,
                                  val,
                                  lex->cur_line,
                                  lex->cur_col);
         }
         case '*': case '/': {
            return buzzlex_newtok(BUZZTOK_MULDIV,
                                  val,
                                  lex->cur_line,
                                  lex->cur_col);
         }
         case '%': {
            return buzzlex_newtok(BUZZTOK_MOD,
                                  val,
                                  lex->cur_line,
                                  lex->cur_col);
         }
         case '^': {
            return buzzlex_newtok(BUZZTOK_POW,
                                  val,
                                  lex->cur_line,
                                  lex->cur_col);
         }
         default:
            return NULL;
      }
   }
   else if(c == '\'') {
      /* String - eat any character until you find the next ' */
      nextchar();
      size_t start = lex->cur_c;
      while(lex->cur_c < lex->buf_size &&
            lex->buf[lex->cur_c] != '\'') {
         if(lex->buf[lex->cur_c] != '\n') {
            nextchar();
         }
         else {
            ++lex->cur_line;
            lex->cur_col = 0;
            ++lex->cur_c;
         }
      }
      /* End of stream? Syntax error */
      if(lex->cur_c >= lex->buf_size) {
         fprintf(stderr,
                 "%s:%llu:%llu: Syntax error: string closing quote not found\n",
                 lex->fname,
                 lex->cur_line,
                 lex->cur_col);
         return NULL;
      }
      /* Valid string */
      char* val = (char*)malloc(lex->cur_c - start + 1);
      strncpy(val, lex->buf + start, lex->cur_c - start);
      val[lex->cur_c - start] = '\0';
      return buzzlex_newtok(BUZZTOK_STRING,
                            val,
                            lex->cur_line,
                            lex->cur_col);
   }
   else {
      /* Unknown character */
      fprintf(stderr,
              "%s:%llu:%llu: Syntax error: unknown character '%c' (octal: %o; hex: %x)\n",
              lex->fname,
              lex->cur_line,
              lex->cur_col,
              c, c, c);
      return NULL;
   }
}

/****************************************/
/****************************************/

void buzzlex_destroytok(buzztok_t* tok) {
   free((*tok)->value);
   free(*tok);
   *tok = NULL;
}

/****************************************/
/****************************************/
