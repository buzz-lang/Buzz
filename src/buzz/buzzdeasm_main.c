#include <buzz/buzzasm.h>
#include <buzz/buzzdebug.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
   /* Parse command line */
   if(argc != 4) {
      fprintf(stderr, "Usage:\n\t%s <bytecodefile.bo> <debugfile.bdbg> <outfile.basm>\n\n", argv[0]);
      return 0;
   }
   /* Open bytecode file */
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
   /* Read debug information */
   buzzdebuginfo_t dbg = buzzdebuginfo_new();
   if(!buzzdebuginfo_fromfile(dbg, argv[2]))
      perror(argv[2]);
   /* Go through bytecode */
   int rv = buzz_deasm(bcode_buf, bcode_size, dbg, argv[3]);
   /* Cleanup */
   free(bcode_buf);
   buzzdebuginfo_destroy(&dbg);
   return rv;
}
