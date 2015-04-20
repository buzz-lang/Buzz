#include "buzzdarray.h"
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

#define buzzdarray_rawget(da, pos) ((uint8_t*)((da)->data) + ((pos) * (da)->elem_size))

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

void buzzdarray_insert(buzzdarray_t da,
                       uint32_t pos,
                       const void* data) {
   /* Calculate actual position of the element to add
      Making sure we are not adding beyond the current size */
   uint32_t i = pos < buzzdarray_size(da) ? pos : buzzdarray_size(da);
   /* Increase the capacity if necessary */
   if(i >= da->capacity) {
      do { da->capacity *= 2; } while(i >= da->capacity);
      da->data = realloc(da->data, da->capacity * da->elem_size);
   }
   /* Move elements from i onwards one step to the right */
   if(!buzzdarray_isempty(da) && i < buzzdarray_size(da))
      memmove(
         buzzdarray_rawget(da, i+1),
         buzzdarray_rawget(da, i),
         (buzzdarray_size(da) - i) * da->elem_size);
   /* Add element at the specified position */
   buzzdarray_set(da, i, data);
   /* Increase size */
   ++(da->size);
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
      da->data = realloc(da->data, da->capacity * da->elem_size);
   }
}

/****************************************/
/****************************************/

void buzzdarray_set(buzzdarray_t da,
                    uint32_t pos,
                    const void* value) {
   /* Copy value */
   memcpy(
      buzzdarray_rawget(da, pos),
      value,
      da->elem_size);
}

/****************************************/
/****************************************/

void buzzdarray_foreach(buzzdarray_t da,
                        buzzdarray_elem_funp fun,
                        void* params) {
   for(uint32_t i = 0; i < buzzdarray_size(da); ++i) {
      fun(i, buzzdarray_rawget(da, i), params);
   }
}

/****************************************/
/****************************************/

uint32_t buzzdarray_find(buzzdarray_t da,
                         buzzdarray_elem_cmpp cmp,
                         const void* data) {
   for(uint32_t i = 0; i < buzzdarray_size(da); ++i) {
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
   int64_t pt = lo;
   for(int64_t k = lo; k < hi; ++k) {
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
