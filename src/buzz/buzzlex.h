#ifndef BUZZLEX_H
#define BUZZLEX_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Tokens types recognized by the lexer
    */
   typedef enum {
      BUZZTOK_ID = 0,
      BUZZTOK_CONST,
      BUZZTOK_STRING,
      BUZZTOK_VAR,
      BUZZTOK_NIL,
      BUZZTOK_IF,
      BUZZTOK_ELSE,
      BUZZTOK_FUN,
      BUZZTOK_RETURN,
      BUZZTOK_FOR,
      BUZZTOK_WHILE,
      BUZZTOK_ANDOR,
      BUZZTOK_NOT,
      BUZZTOK_ADDSUB,
      BUZZTOK_MULDIV,
      BUZZTOK_MOD,
      BUZZTOK_POW,
      BUZZTOK_BLOCKOPEN,
      BUZZTOK_BLOCKCLOSE,
      BUZZTOK_PAROPEN,
      BUZZTOK_PARCLOSE,
      BUZZTOK_IDXOPEN,
      BUZZTOK_IDXCLOSE,
      BUZZTOK_STATEND,
      BUZZTOK_LISTSEP,
      BUZZTOK_ASSIGN,
      BUZZTOK_DOT,
      BUZZTOK_CMP,
   } buzztok_type_e;
   static char *buzztok_desc[] = {
      "identifier", "numeric constant", "string", "variable",
      "nil", "if", "else", "function", "return",
      "for", "while", "and/or", "not", "+ or -", "* or /",
      "%", "^", "{", "}", "(", ")", "[", "]", "; or newline",
      ",", "=", ".", "== != < <= > >=" };

   /*
    * Token data record
    */
   struct buzztok_s {
      /* Type of token */
      buzztok_type_e type;
      /* The actual token read */
      char* value;
      /* Line number where the token is located */
      uint64_t line;
      /* Column number where the token is located */
      uint64_t col;
      /* Source file name */
      char* fname;
   };
   typedef struct buzztok_s* buzztok_t;

   /*
    * State of the lexer
    */
   struct buzzlex_s {
      /* The current line being read */
      uint64_t cur_line;
      /* The current column within the current line */
      uint64_t cur_col;
      /* The size of the buffer */
      size_t buf_size;
      /* The index of the next character to read */
      size_t cur_c;
      /* The buffer in which the file is stored */
      char* buf;
      /* The name of the file */
      char* fname;
   };
   typedef struct buzzlex_s* buzzlex_t;

   /*
    * Creates a new lexer.
    * @param fname The name of the file to process.
    * @return The lexer state.
    */
   extern buzzlex_t buzzlex_new(const char* fname);

   /*
    * Destroys the lexer.
    * @param lex The lexer state.
    */
   extern void buzzlex_destroy(buzzlex_t* lex);
   
   /*
    * Processes the next token.
    * @param lex The lexer state.
    * @return The token or NULL if EOF or an error occurred.
    */
   extern buzztok_t buzzlex_nexttok(buzzlex_t lex);

   /*
    * Clones the given token.
    * @param tok The token to clone.
    * @return The cloned token.
    */
   extern buzztok_t buzzlex_clonetok(buzztok_t tok);

   /*
    * Destroys a token.
    * @param tok The token to destroy.
    */
   extern void buzzlex_destroytok(buzztok_t* tok);

#ifdef __cplusplus
}
#endif

#endif
