#ifndef BUZZSET_H
#define BUZZSET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Function pointer for an element-wise function:
    *
    * void f(void* data, void* params)
    *
    * This function pointer is used to destroy elements by
    * buzzset_destroy() and in methods such as buzzset_foreach().
    */
   typedef void (*buzzset_elem_funp)(void* data, void* params);

   /*
    * Function pointer to compare set elements.
    *
    * int f(const void* a, const void* b)
    *
    * The function must return:
    * -1 if *a < *b
    * 0  if *a == *b
    * 1  if *a > *b
    */
   typedef int (*buzzset_elem_cmpp)(const void* a, const void* b);

   /*
    * The Buzz set.
    */
   struct buzzset_s {
      void* data;               // Actual data
      uint32_t size;            // Number of inserted elements
      buzzset_elem_funp dstryf; // Element destroy function
      buzzset_elem_cmpp cmpf;   // Element comparison function
      uint32_t data_size;       // Size of data element.
   };
   typedef struct buzzset_s* buzzset_t;

   /*
    * Creates a new Buzz set.
    * @param elem_cmp     The element comparison function.
    * @param elem_destroy The function to destroy an element. Can be NULL.
    * @return A new set.
    */
   extern buzzset_t buzzset_new(uint32_t elem_size,
                                buzzset_elem_cmpp elem_cmp,
                                buzzset_elem_funp elem_destroy);
   
   /*
    * Destroys a set.
    * Internally calls s.elem_destroy(), if not NULL.
    * @param s The set.
    */
   extern void buzzset_destroy(buzzset_t* s);

   /*
    * Inserts an element.
    * The element must be passed as a pointer. The pointed data is copied
    * into the data structure.
    * @param s The set.
    * @param data A pointer to the element to add.
    */
   extern void buzzset_insert(buzzset_t s,
                              const void* data);

   /*
    * Removes the passed element.
    * @param s The set.
    * @param data A pointer to the element to remove.
    */
   extern void buzzset_remove(buzzset_t s,
                              const void* data);

   /*
    * Finds an element.
    * @param s The set.
    * @param data The element to find.
    * @return The position of the found element, or NULL.
    */
   extern void* buzzset_find(buzzset_t s,
                             const void* data);

   /*
    * Applies a function to each element of the set.
    * @param s The set.
    * @param fun The function to apply to each element.
    * @param params A data structure to pass along.
    */
   extern void buzzset_foreach(buzzset_t s,
                               buzzset_elem_funp fun,
                               void* params);
   
#ifdef __cplusplus
}
#endif

/*
 * Returns the size of the set.
 * @param s The set.
 * @return The size of the set.
 */
#define buzzset_size(s) (s)->size

/*
 * Returns <tt>true</tt> if the set is empty.
 * @param s The set.
 * @return <tt>true</tt> if the set is empty.
 */
#define buzzset_isempty(s) (buzzset_size(s) == 0)

/*
 * Fetches the element that matches the given data, or NULL is nothing is found.
 * The returned element is casted to a pointer to the given type.
 * @param s The set.
 * @param data The data.
 * @param type The element type.
 * @see buzzdict_find
 */
#define buzzset_fetch(s, data, type) (const type*)(buzzset_find(s, data))

#endif
