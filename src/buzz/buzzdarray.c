#include "buzzdarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

#define buzzdarray_rawget(da, pos) ((uint8_t*)(da)->data + (pos) * (da)->elem_size)

void buzzdarray_elem_destroy(uint32_t pos, void* data, void* params) {}

/****************************************/
/****************************************/

buzzdarray_t buzzdarray_new(uint32_t cap,
                            uint32_t elem_size,
                            buzzdarray_elem_funp elem_destroy) {
   /* Create the dynamic array. calloc() zeroes everything. */
   buzzdarray_t da = (buzzdarray_t)calloc(1, sizeof(struct buzzdarray_s));
   /* Set info */
   da->capacity = cap;
   da->elem_size = elem_size;
   da->elem_destroy = elem_destroy ? elem_destroy : buzzdarray_elem_destroy;
   /* Create initial data */
   da->data = calloc(cap, elem_size);
   /* Done */
   return da;
}

/****************************************/
/****************************************/

buzzdarray_t buzzdarray_clone(const buzzdarray_t da) {
   /* Create the dynamic array. */
   buzzdarray_t clone = (buzzdarray_t)malloc(sizeof(struct buzzdarray_s));
   /* Copy info */
   clone->size = da->size;
   clone->capacity = clone->size > 0 ? clone->size : 1;
   clone->elem_size = da->elem_size;
   clone->elem_destroy = da->elem_destroy;
   /* Create data buffer */
   clone->data = malloc(clone->capacity * clone->elem_size);
   memcpy(clone->data, da->data, clone->size * clone->elem_size);
   /* Done */
   return clone;
}

/****************************************/
/****************************************/

buzzdarray_t buzzdarray_frombuffer(const void* buf,
                                   uint32_t buf_size,
                                   uint32_t elem_size,
                                   buzzdarray_elem_funp elem_destroy) {
   /* Create the dynamic array. calloc() zeroes everything. */
   buzzdarray_t da = (buzzdarray_t)calloc(1, sizeof(struct buzzdarray_s));
   /* Set info */
   da->capacity = buf_size / elem_size;
   da->elem_size = elem_size;
   da->elem_destroy = elem_destroy ? elem_destroy : buzzdarray_elem_destroy;
   da->size = da->capacity;
   /* Create initial data */
   da->data = malloc(buf_size);
   memcpy(da->data, buf, buf_size);
   /* Done */
   return da;
}

/****************************************/
/****************************************/

void buzzdarray_destroy(buzzdarray_t* da) {
   /* Get rid of every element */
   buzzdarray_foreach(*da, (*da)->elem_destroy, NULL);
   /* Get rid of the rest */
   free((*da)->data);
   free(*da);
   /* Set da to NULL */
   *da = NULL;
}

/****************************************/
/****************************************/

void* buzzdarray_makeslot(buzzdarray_t da,
                          uint32_t pos) {
   /* Calculate actual position of the element to add
      Making sure we are not adding beyond the current size */
   uint32_t i = pos < buzzdarray_size(da) ? pos : buzzdarray_size(da);
   /* Increase the capacity if necessary */
   if(buzzdarray_size(da)+1 >= da->capacity) {
      do { da->capacity *= 2; } while(buzzdarray_size(da)+1 >= da->capacity);
      void* nd = realloc(da->data, da->capacity * da->elem_size);
      if(!nd) {
         fprintf(stderr, "[FATAL] Can't reallocate dynamic array.\n");
         abort();
      }
      da->data = nd;
   }
   /* Move elements from i onwards one step to the right */
   if(!buzzdarray_isempty(da) && i < buzzdarray_size(da)) {
      memmove(
         buzzdarray_rawget(da, i+1),
         buzzdarray_rawget(da, i),
         (buzzdarray_size(da) - i) * da->elem_size);
   }
   /* Increase size */
   ++(da->size);
   /* Return pointer to the slot */
   return buzzdarray_rawget(da, pos);
}

/****************************************/
/****************************************/

void buzzdarray_insert(buzzdarray_t da,
                       uint32_t pos,
                       const void* data) {
   /* Create the slot */
   void* slot = buzzdarray_makeslot(da, pos);
   /* Add element at the specified position */
   if(slot != NULL) buzzdarray_set(da, pos, data);
}

/****************************************/
/****************************************/

void buzzdarray_remove(buzzdarray_t da,
                       uint32_t pos) {
   /* Can't remove elements past the size */
   if(pos >= buzzdarray_size(da)) return;
   /* Destroy element */
   da->elem_destroy(pos, buzzdarray_rawget(da, pos), NULL);
   /* Move the elements from pos onwards one spot to the left */
   memmove(
      buzzdarray_rawget(da, pos),
      buzzdarray_rawget(da, pos+1),
      (buzzdarray_size(da) - pos - 1) * da->elem_size);
   /* Update the size */
   --(da->size);
   /* Shrink the capacity if necessary */
   if((da->size > 0) &&
      (da->size <= da->capacity / 2)) {
      da->capacity /= 2;
      void* nd = realloc(da->data, da->capacity * da->elem_size);
      if(!nd) {
         fprintf(stderr, "[FATAL] Can't reallocate dynamic array.\n");
         abort();
      }
      da->data = nd;
   }
}

/****************************************/
/****************************************/

void buzzdarray_clear(buzzdarray_t da,
                      uint32_t cap) {
   /* Get rid of every element */
   buzzdarray_foreach(da, da->elem_destroy, NULL);
   /* Resize the array */
   da->capacity = cap;
   void* nd = realloc(da->data, da->capacity * da->elem_size);
   if(!nd) {
      fprintf(stderr, "[FATAL] Can't reallocate dynamic array.\n");
      abort();
   }
   da->data = nd;
   /* Zero the size */
   da->size = 0;
}

/****************************************/
/****************************************/

void buzzdarray_set(buzzdarray_t da,
                    uint32_t pos,
                    const void* value) {
   if(pos < buzzdarray_size(da)) {
      /* Copy value */
      memcpy(
         buzzdarray_rawget(da, pos),
         value,
         da->elem_size);
   }
   else {
      /* Insert value */
      buzzdarray_insert(da, pos, value);
   }
}

/****************************************/
/****************************************/

void buzzdarray_foreach(buzzdarray_t da,
                        buzzdarray_elem_funp fun,
                        void* params) {
   uint32_t i;
   for(i = 0; i < buzzdarray_size(da); ++i) {
      fun(i, buzzdarray_rawget(da, i), params);
   }
}

/****************************************/
/****************************************/

uint32_t buzzdarray_find(buzzdarray_t da,
                         buzzdarray_elem_cmpp cmp,
                         const void* data) {
   uint32_t i;
   for(i = 0; i < buzzdarray_size(da); ++i) {
      if(cmp(data, buzzdarray_rawget(da, i)) == 0)
         return i;
   }
   return buzzdarray_size(da);
}

/****************************************/
/****************************************/

#define SWAP(a,b) { memcpy(t, a, da->elem_size); memcpy(a, b, da->elem_size); memcpy(b, t, da->elem_size); }

uint32_t buzzdarray_part(buzzdarray_t da,
                         buzzdarray_elem_cmpp cmp,
                         int64_t lo,
                         int64_t hi) {
   /* Temporary used for swapping */
   void* t = malloc(da->elem_size);
   /* Use last element as pivot */
   int64_t pt = lo, k;
   for(k = lo; k < hi; ++k) {
      if(cmp(buzzdarray_rawget(da, k), buzzdarray_rawget(da, hi)) < 0) {
         SWAP(buzzdarray_rawget(da, k), buzzdarray_rawget(da, pt));
         ++pt;
      }
   }
   SWAP(buzzdarray_rawget(da, pt), buzzdarray_rawget(da, hi));
   /* Get rid of temporary */
   free(t);
   return pt;
}

void buzzdarray_qsort(buzzdarray_t da,
                      buzzdarray_elem_cmpp cmp,
                      int64_t lo,
                      int64_t hi) {
   if(lo < hi) {
      int64_t p = buzzdarray_part(da, cmp, lo, hi);
      buzzdarray_qsort(da, cmp, lo, p - 1);
      buzzdarray_qsort(da, cmp, p + 1, hi);
   }
}

void buzzdarray_sort(buzzdarray_t da,
                     buzzdarray_elem_cmpp cmp) {
   buzzdarray_qsort(da, cmp, 0, buzzdarray_size(da) - 1);
}

/****************************************/
/****************************************/
