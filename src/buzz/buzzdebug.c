#include "buzzdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/****************************************/
/****************************************/

buzzdebug_entry_t buzzdebug_entry_new(uint64_t l,
                                      uint64_t c,
                                      const char* fn) {
   buzzdebug_entry_t x = malloc(sizeof(struct buzzdebug_entry_s));
   x->line = l;
   x->col = c;
   x->fname = strdup(fn);
   return x;
}

void buzzdebug_entry_destroyf(const void* key, void* data, void* params) {
   buzzdebug_entry_t x = *(buzzdebug_entry_t*)data;
   free(x->fname);
   free(x);
}

/****************************************/
/****************************************/

buzzdebug_t buzzdebug_new() {
   buzzdebug_t x = (buzzdebug_t)malloc(sizeof(struct buzzdebug_s));
   x->off2script = buzzdict_new(100,
                                sizeof(int32_t),
                                sizeof(buzzdebug_entry_t),
                                buzzdict_int32keyhash,
                                buzzdict_int32keycmp,
                                buzzdebug_entry_destroyf);
   x->breakpoints = buzzdarray_new(5, sizeof(int32_t), NULL);
   return x;
}

/****************************************/
/****************************************/

void buzzdebug_destroy(buzzdebug_t* dbg) {
   buzzdarray_destroy(&(*dbg)->breakpoints);
   buzzdict_destroy(&(*dbg)->off2script);
   *dbg = NULL;
}

/****************************************/
/****************************************/

int buzzdebug_fromfile(buzzdebug_t dbg,
                       const char* fname) {
   /* Open file */
   FILE* fd = fopen(fname, "rb");
   if(!fd) return 0;
   /* Keep reading until you reach the end of the data or an error */
   uint32_t offset;
   uint64_t line, col;
   uint16_t srcfnlen, srcfnlenmax = 100;
   char* srcfname = (char*)malloc(srcfnlenmax);
   while(!feof(fd) && !ferror(fd)) {
      /* Read fields */
      if(fread(&offset,   sizeof(offset),   1, fd) == 1 &&
         fread(&line,     sizeof(line),     1, fd) == 1 &&
         fread(&col,      sizeof(col),      1, fd) == 1 &&
         fread(&srcfnlen, sizeof(srcfnlen), 1, fd) == 1) {
         /* Read source file name, resizing buffer if necessary */
         if(srcfnlen > srcfnlenmax) {
            srcfnlenmax = srcfnlen + 1;
            srcfname = realloc(srcfname, srcfnlenmax);
         }
         if(fread(srcfname, 1, srcfnlen, fd) == srcfnlen) {
            srcfname[srcfnlen] = 0;
            buzzdebug_off2script_set(dbg, offset, line, col, srcfname);
         }
      }
   }
   free(srcfname);
   fclose(fd);
   return ferror(fd) ? 0 : 1;
}

/****************************************/
/****************************************/

struct buzzdebug_entry_dump_info_s {
   FILE* fd;
   int ok; /* 1 if no error, 0 otherwise */
};

void buzzdebug_entry_dump(const void* key, void* data, void* params) {
   /* Get data */
   struct buzzdebug_entry_dump_info_s* i = (struct buzzdebug_entry_dump_info_s*)params;
   buzzdebug_entry_t e = *(buzzdebug_entry_t*)data;
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

int buzzdebug_tofile(const char* fname,
                     buzzdebug_t dbg) {
   /* Open file */
   FILE* fd = fopen(fname, "wb");
   if(!fd) return 0;
   /* Write entries */
   struct buzzdebug_entry_dump_info_s i = {
      .fd = fd,
      .ok = 1
   };
   buzzdict_foreach(dbg->off2script, buzzdebug_entry_dump, &i);
   /* Close file */
   fclose(fd);
   return i.ok ? 0 : 1;
}

/****************************************/
/****************************************/

void buzzdebug_off2script_set(buzzdebug_t dbg,
                              uint32_t offset,
                              uint64_t line,
                              uint64_t col,
                              const char* fname) {
   buzzdebug_entry_t e = buzzdebug_entry_new(line, col, fname);
   buzzdict_set(dbg->off2script, &offset, &e);
}

/****************************************/
/****************************************/

int offset_compare(const void* a, const void* b) {
   int32_t x = *(int32_t*)a;
   int32_t y = *(int32_t*)b;
   if(x < y) return -1;
   if(x > y) return 1;
   return 0;
}

void buzzdebug_breakpoint_set_offset(buzzdebug_t dbg,
                                     int32_t off) {
   if(buzzdarray_find(dbg->breakpoints, offset_compare, &off) ==
      buzzdarray_size(dbg->breakpoints)) {
      buzzdarray_push(dbg->breakpoints, &off);
   }
}

/****************************************/
/****************************************/
