#include <buzz/buzzlex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

int main(int argc, char* argv[]) {
   /* Parse command line */
   if(argc != 2) {
      fprintf(stderr, "Usage:\n\t%s <script.bzz>\n\n", argv[0]);
      return 0;
   }
   /* Create lexer */
   buzzlex_t lex = buzzlex_new(argv[1]);
   if(!lex) {
      return 1;
   }
   fprintf(stderr, "PARSED\n\n%s\n\n", buzzlex_getfile(lex)->buf);
   /* Parse the script */
   buzztok_t tok;
   int done = 0;
   do {
      tok = buzzlex_nexttok(lex);
      if(!tok) done = 1;
      else {
         fprintf(stdout, "TOKEN %s %2" PRIu64 " %2" PRIu64 " %-20s '%s'\n",
                 tok->fname,
                 tok->line,
                 tok->col,
                 buzztok_desc[tok->type],
                 (tok->value ? tok->value : "/NULL/"));
         buzzlex_destroytok(&tok);
      }
   }
   while(!done);
   /* Destroy the lexer */
   buzzlex_destroy(&lex);
   return 0;
}
