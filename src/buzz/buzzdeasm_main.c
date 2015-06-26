#include "buzzasm.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
   /* Parse command line */
   if(argc != 3) {
      fprintf(stderr, "Usage:\n\t%s <infile.bo> <outfile.basm>\n\n", argv[0]);
      return 0;
   }
   /* Open input file */
   int ifd = open(argv[1], O_RDONLY);
   if(ifd < 0) perror(argv[1]);
   /* Read data */
   size_t bcode_size = lseek(ifd, 0, SEEK_END);
   lseek(ifd, 0, SEEK_SET);
   uint8_t* bcode_buf = (uint8_t*)malloc(bcode_size);
   ssize_t rd;
   size_t tot = 0;
   while(tot < bcode_size) {
      rd = read(ifd, bcode_buf + tot, bcode_size - tot);
      if(rd < 0) perror(argv[1]);
      tot += rd;
   }
   close(ifd);
   /* Go through bytecode */
   int rv = buzz_deasm(bcode_buf, bcode_size, argv[2]);
   /* Cleanup */
   free(bcode_buf);
   return rv;
}
