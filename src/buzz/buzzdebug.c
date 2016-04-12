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

void buzzdebug_entry_destroy(buzzdebug_entry_t* e) {
   free((*e)->fname);
   free(*e);
   *e = NULL;
}

void buzzdebug_off2script_destroyf(const void* key, void* data, void* params) {
   free((void*)key);
   buzzdebug_entry_destroy((buzzdebug_entry_t*)data);
   free(data);
}

void buzzdebug_script2off_destroyf(const void* key, void* data, void* params) {
   buzzdebug_entry_destroy((buzzdebug_entry_t*)key);
   free((void*)key);
   free(data);
}

uint32_t buzzdebug_entryhash(const void* key) {
   buzzdebug_entry_t x = *(const buzzdebug_entry_t*)key;
   char* str;
   uint32_t h;
   asprintf(&str, "%s:%llu:%llu", x->fname, x->line, x->col);
   h = buzzdict_strkeyhash(&str);
   free(str);
   return h;
}

int buzzdebug_entrycmp(const void* a, const void* b) {
   buzzdebug_entry_t x = *(const buzzdebug_entry_t*)a;
   buzzdebug_entry_t y = *(const buzzdebug_entry_t*)b;
   int cmp = strcmp(x->fname, y->fname);
   if(cmp == 0) {
      if(x->line < y->line) return -1;
      if(x->line > y->line) return 1;
      if(x->col < y->col) return -1;
      if(x->col > y->col) return 1;
      return 0;
   }
   else
      return cmp;
}

/****************************************/
/****************************************/

buzzdebug_t buzzdebug_new() {
   /* Make room for the debug information structure */
   buzzdebug_t x = (buzzdebug_t)malloc(sizeof(struct buzzdebug_s));
   /* Make hash map for offset -> script data */
   x->off2script = buzzdict_new(100,
                                sizeof(int32_t),
                                sizeof(buzzdebug_entry_t),
                                buzzdict_int32keyhash,
                                buzzdict_int32keycmp,
                                buzzdebug_off2script_destroyf);
   /* Make hash map for script -> offset data */
   x->script2off = buzzdict_new(100,
                                sizeof(buzzdebug_entry_t),
                                sizeof(int32_t),
                                buzzdebug_entryhash,
                                buzzdebug_entrycmp,
                                buzzdebug_script2off_destroyf);
   /* Make list of breakpoints */
   x->breakpoints = buzzdarray_new(5, sizeof(int32_t), NULL);
   return x;
}

/****************************************/
/****************************************/

void buzzdebug_destroy(buzzdebug_t* dbg) {
   buzzdarray_destroy(&(*dbg)->breakpoints);
   buzzdict_destroy(&(*dbg)->off2script);
   buzzdict_destroy(&(*dbg)->script2off);
   free(*dbg);
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
            buzzdebug_info_set(dbg, offset, line, col, srcfname);
         }
      }
   }
   int err = ferror(fd);
   free(srcfname);
   fclose(fd);
   return err ? 0 : 1;
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

void buzzdebug_info_set(buzzdebug_t dbg,
                        int32_t offset,
                        uint64_t line,
                        uint64_t col,
                        const char* fname) {
   /* Make new entry */
   buzzdebug_entry_t e = buzzdebug_entry_new(line, col, fname);
   /* Add entry to offset -> script structure */
   buzzdict_set(dbg->off2script, &offset, &e);
   /* Add entry to script -> offset structure only if
    * 1. it does not exist, OR
    * 2. it exists, but the stored offset is larger than the passed one
    */
   if(!buzzdict_exists(dbg->script2off, &e) ||
      *buzzdict_get(dbg->script2off, &e, int32_t) > offset) {
      /* Make new entry */
      e = buzzdebug_entry_new(line, col, fname);
      /* Add entry to script -> offset structure */
      buzzdict_set(dbg->script2off, &e, &offset);
   }
}

/****************************************/
/****************************************/

const int32_t* buzzdebug_info_get_fromscript(buzzdebug_t dbg,
                                             uint64_t line,
                                             uint64_t col,
                                             const char* fname) {
   /* Make entry */
   buzzdebug_entry_t e = buzzdebug_entry_new(line, col, fname);
   /* Search for it in the structure */
   const int32_t* found = buzzdict_get(dbg->script2off, &e, int32_t);
   /* Cleanup */
   buzzdebug_entry_destroy(&e);
   /* Return result */
   return found;
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
