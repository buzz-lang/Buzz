#include "buzzdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/****************************************/
/****************************************/

buzzdebuginfo_entry_t buzzdebuginfo_entry_new(uint64_t l, uint64_t c) {
   buzzdebuginfo_entry_t x = malloc(sizeof(struct buzzdebuginfo_entry_s));
   x->line = l;
   x->col = c;
   return x;
}

void buzzdebuginfo_entry_destroyf(const void* key, void* data, void* params) {
   free(*(struct debug_data_s**)data);
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
   while(!feof(fd) && !ferror(fd)) {
      if(fread(&offset, sizeof(offset), 1, fd) == 1 &&
         fread(&line,   sizeof(line),   1, fd) == 1 &&
         fread(&col,    sizeof(col),    1, fd) == 1) {
         buzzdebuginfo_set(dbg, offset, line, col);
      }
   }
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
      if(fwrite(key,      sizeof(uint32_t), 1, i->fd) < 1 ||
         fwrite(&e->line, sizeof(uint64_t), 1, i->fd) < 1 ||
         fwrite(&e->col,  sizeof(uint64_t), 1, i->fd) < 1)
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
                       uint64_t col) {
   buzzdebuginfo_entry_t e = buzzdebuginfo_entry_new(line, col);
   buzzdict_set(dbg, &offset, &e);
}

/****************************************/
/****************************************/
