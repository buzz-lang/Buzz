#include "buzzparser.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
   if(argc != 3) {
      fprintf(stderr, "Usage:\n\t%s <infile.bzz> <outfile.basm>\n\n", argv[0]);
      return 0;
   }
   buzzparser_t p = buzzparser_new(argv[1], argv[2]);
   if(!p) return 1;
   int retval = buzzparser_parse(p);
   buzzparser_destroy(&p);
   return !retval;
}
