#include "buzzdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/****************************************/
/****************************************/

buzzdebuginfo_entry_t buzzdebuginfo_entry_new(uint64_t l,
                                              uint64_t c,
                                              char* fn) {
   buzzdebuginfo_entry_t x = malloc(sizeof(struct buzzdebuginfo_entry_s));
   x->line = l;
   x->col = c;
   x->fname = strdup(fn);
   return x;
}

void buzzdebuginfo_entry_destroyf(const void* key, void* data, void* params) {
   buzzdebuginfo_entry_t x = *(buzzdebuginfo_entry_t*)data;
   free(x->fname);
   free(x);
}

/****************************************/
/****************************************/

buzzdebuginfo_t buzzdebuginfo_new() {
   return buzzdict_new(100,
                       sizeof(int32_t),
                       sizeof(buzzdebuginfo_entry_t),
                       buzzdict_int32keyhash,
                       buzzdict_int32keycmp,
                       buzzdebuginfo_entry_destroyf);
}

/****************************************/
/****************************************/

int buzzdebuginfo_fromfile(buzzdebuginfo_t dbg,
                           const char* fname) {
   /* Open file */
   FILE* fd = fopen(fname, "rb");
   if(!fd) return 0;
   /* Keep reading until you reach the end of the data or an error */
   uint32_t offset;
   uint64_t line, col;
   uint16_t srcfnlen, srcfnlenmax = 100;
   char* srcfname = malloc(srcfnlenmax);
   while(!feof(fd) && !ferror(fd)) {
      /* Read fields */
      if(fread(&offset,   sizeof(offset),   1, fd) == 1 &&
         fread(&line,     sizeof(line),     1, fd) == 1 &&
         fread(&col,      sizeof(col),      1, fd) == 1 &&
         fread(&srcfnlen, sizeof(srcfnlen), 1, fd) == 1) {
         /* Read source file name, resizing buffer if necessary */
         if(srcfnlen > srcfnlenmax) {
            srcfnlenmax = srcfnlen;
            srcfname = realloc(srcfname, srcfnlenmax);
         }
         if(fread(srcfname, 1, srcfnlen, fd) == srcfnlen)
            buzzdebuginfo_set(dbg, offset, line, col, srcfname);
      }
   }
   free(srcfname);
   fclose(fd);
   return ferror(fd) ? 0 : 1;
}

/****************************************/
/****************************************/

struct buzzdebuginfo_entry_dump_info_s {
   FILE* fd;
   int ok; /* 1 if no error, 0 otherwise */
};

void buzzdebuginfo_entry_dump(const void* key, void* data, void* params) {
   /* Get data */
   struct buzzdebuginfo_entry_dump_info_s* i = (struct buzzdebuginfo_entry_dump_info_s*)params;
   buzzdebuginfo_entry_t e = *(buzzdebuginfo_entry_t*)data;
   /* Make sure no error occurred */
   if(i->ok) {
      /* Write data */
      uint16_t strfnlen = strlen(e->fname);
      if(fwrite(key,       sizeof(uint32_t), 1, i->fd) < 1 ||
         fwrite(&e->line,  sizeof(uint64_t), 1, i->fd) < 1 ||
         fwrite(&e->col,   sizeof(uint64_t), 1, i->fd) < 1 ||
         fwrite(&strfnlen, sizeof(uint16_t), 1, i->fd) < 1 ||
         fwrite(e->fname,  1, strfnlen, i->fd) < strfnlen)
         i->ok = 0;
   }
}

int buzzdebuginfo_tofile(const char* fname,
                         buzzdebuginfo_t dbg) {
   /* Open file */
   FILE* fd = fopen(fname, "wb");
   if(!fd) return 0;
   /* Write entries */
   struct buzzdebuginfo_entry_dump_info_s i = {
      .fd = fd,
      .ok = 1
   };
   buzzdict_foreach(dbg, buzzdebuginfo_entry_dump, &i);
   /* Close file */
   fclose(fd);
   return i.ok ? 0 : 1;
}

/****************************************/
/****************************************/

void buzzdebuginfo_set(buzzdebuginfo_t dbg,
                       uint32_t offset,
                       uint64_t line,
                       uint64_t col,
                       char* fname) {
   buzzdebuginfo_entry_t e = buzzdebuginfo_entry_new(line, col, fname);
   buzzdict_set(dbg, &offset, &e);
}

/****************************************/
/****************************************/
