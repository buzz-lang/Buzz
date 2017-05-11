#include "buzzparser.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
   if(argc < 3 || argc > 4) {
      fprintf(stderr, "Usage:\n\t%s <infile.bzz> <outfile.basm> [stringlist.bst]\n\n", argv[0]);
      return 0;
   }
   buzzparser_t p = buzzparser_new(argc, argv);
   if(!p) {
      return 1;
   }
   int retval = buzzparser_parse(p);
   buzzparser_destroy(&p);
   return retval == 1 ? 0 : 1;
}
