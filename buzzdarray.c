#include "buzzdarray.h"
#include <stdlib.h>

/****************************************/
/****************************************/

void buzzdarray_elem_destroy(uint32_t pos,
                             void* data,
                             void* params) {
   free(data);
}

/****************************************/
/****************************************/

buzzdarray_t buzzdarray_new(uint32_t cap,
                            buzzdarray_elem_funp elem_destroy) {
   /* Create the dynamic array. calloc() zeroes everything. */
   buzzdarray_t da = (buzzdarray_t)calloc(1, sizeof(struct buzzdarray_s));
   /* Set info */
   da->capacity = cap;
   da->elem_destroy = elem_destroy ? elem_destroy : buzzdarray_elem_destroy;
   /* Create initial data */
   da->data = (void*)calloc(cap, sizeof(void*));
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
      da->data = realloc(da->data, da->capacity * sizeof(void*));
   }
   /* Move elements after i one step on the right */
   if(!buzzdarray_is_empty(da))
      for(uint32_t j = buzzdarray_size(da); j > i; --j) {
         buzzdarray_set(da, j, buzzdarray_get(da, j-1));
      }
   /* Add element at the specified position */
   buzzdarray_set(da, i, (void*)data);
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
   da->elem_destroy(pos, buzzdarray_get(da, pos), NULL);
   /* Move the elements from pos onwards one spot to the left */
   for(uint32_t i = pos; i < buzzdarray_size(da)-1; ++i) {
      buzzdarray_set(da, i, buzzdarray_get(da, i+1));
   }
   /* Update the size */
   --(da->size);
   /* Shrink the capacity if necessary */
   if((da->size > 0) &&
      (da->size <= da->capacity / 2)) {
      da->capacity /= 2;
      da->data = realloc(da->data, da->capacity * sizeof(void*));      
   }
}

/****************************************/
/****************************************/

void buzzdarray_foreach(buzzdarray_t da,
                        buzzdarray_elem_funp fun,
                        void* params) {
   for(uint32_t i = 0; i < buzzdarray_size(da); ++i) {
      fun(i, buzzdarray_get(da, i), params);
   }
}

/****************************************/
/****************************************/

uint32_t buzzdarray_find(buzzdarray_t da,
                         buzzdarray_elem_cmpp cmp,
                         const void* data) {
   for(uint32_t i = 0; i < buzzdarray_size(da); ++i) {
      if(cmp(data, buzzdarray_get(da, i)) == 0)
         return i;
   }
   return buzzdarray_size(da);
}

/****************************************/
/****************************************/

#define SWAP(a,b) { void* tmp = a; a = b; b = tmp; }

uint32_t buzzdarray_part(buzzdarray_t da,
                         buzzdarray_elem_cmpp cmp,
                         int64_t lo,
                         int64_t hi) {
   /* Use last element as pivot */
   int64_t pt = lo;
   for(int64_t k = lo; k < hi; ++k) {
      if(cmp(buzzdarray_get(da, k), buzzdarray_get(da, hi)) < 0) {
         SWAP(buzzdarray_get(da, k), buzzdarray_get(da, pt));
         ++pt;
      }
   }
   SWAP(buzzdarray_get(da, pt), buzzdarray_get(da, hi));
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
