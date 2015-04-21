#ifndef BUZZDARRAY
#define BUZZDARRAY

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Function pointer for an element-wise function:
    *
    * void f(uint32_t pos, void* data, void* params)
    *
    * This function pointer is used to destroy elements by
    * buzzdarray_destroy() and in methods such as
    * buzzdarray_foreach().
    */
   typedef void (*buzzdarray_elem_funp)(uint32_t pos, void* data, void* params);

   /*
    * Function pointer to compare buzzdarray elements.
    *
    * int f(const void* a, const void* b)
    *
    * The function must return:
    * -1 if *a < *b
    * 0  if *a == *b
    * 1  if *a > *b
    */
   typedef int (*buzzdarray_elem_cmpp)(const void* a, const void* b);

   /*
    * Buzz dynamic array data.
    */
   struct buzzdarray_s {
      void** data;
      int64_t size;
      uint32_t elem_size;
      uint32_t capacity;
      buzzdarray_elem_funp elem_destroy;
   };
   typedef struct buzzdarray_s* buzzdarray_t;

   /*
    * Creates a new Buzz dynamic array.
    * @param cap The initial capacity of the array. Must be >0.
    * @param elem_size The size of an element.
    * @param elem_destroy The function to destroy an element. Can be NULL.
    */
   extern buzzdarray_t buzzdarray_new(uint32_t cap,
                                      uint32_t elem_size,
                                      buzzdarray_elem_funp elem_destroy);

   /*
    * Destroys a dynamic array.
    * Internally calls da.elem_destroy(), if not NULL.
    * @param da The dynamic array.
    */
   extern void buzzdarray_destroy(buzzdarray_t* da);

   /*
    * Adds an empty slot to the dynamic array.
    * Differently from buzzdarray_insert(), which adds the slot
    * and also copies the given value within it, this function
    * just creates the slot. This way, you can add the data
    * yourself.
    * NOTE: The slot is not initialized in any way.
    * @param da The dynamic array.
    * @param pos The position.
    * @return A pointer to the new slot, or NULL in case of error.
    */
   extern void* buzzdarray_makeslot(buzzdarray_t da,
                                    uint32_t pos);

   /*
    * Inserts an element at the given position.
    * @param da The dynamic array.
    * @param pos The position.
    * @param data The element to add.
    */
   extern void buzzdarray_insert(buzzdarray_t da,
                                 uint32_t pos,
                                 const void* data);

   /*
    * Removes the element at the given position.
    * @param da The dynamic array.
    * @param pos The position.
    */
   extern void buzzdarray_remove(buzzdarray_t da,
                                 uint32_t pos);

   /*
    * Erases all the elements of the dynamic array.
    * @param da The dynamic array.
    * @param cap The capacity of the array after clearing. Must be >0.
    */
   extern void buzzdarray_clear(buzzdarray_t da,
                                uint32_t cap);

   /*
    * Sets a new value for the element at the given position.
    * If the position is out of bounds, the passed value is not
    * set.
    * @param da The dynamic array.
    * @param pos The position.
    * @param value The new value.
    */
   extern void buzzdarray_set(buzzdarray_t da,
                              uint32_t pos,
                              const void* value);

   /*
    * Applies a function to each element of the dynamic array.
    * @param da The dynamic array.
    * @param fun The function to apply to each element.
    * @param params A data structure to pass along.
    */
   extern void buzzdarray_foreach(buzzdarray_t da,
                                  buzzdarray_elem_funp fun,
                                  void* params);

   /*
    * Finds the position of an element.
    * If the element is not found, the returned position
    * is equal to the darray size.
    * @param da The dynamic array.
    * @param cmp The element comparison function.
    * @param data The element to find.
    * @return The position of the found element, or da.size if not found.
    */
   extern uint32_t buzzdarray_find(buzzdarray_t da,
                                   buzzdarray_elem_cmpp cmp,
                                   const void* data);

   /*
    * Sorts the element in the dynamic array.
    * Internally uses the quicksort algorithm.
    * @param da The dynamic array.
    * @param cmp The element comparison function.
    */
   extern void buzzdarray_sort(buzzdarray_t da,
                               buzzdarray_elem_cmpp cmp);

#ifdef __cplusplus
}
#endif

/*
 * Returns the element at the given position.
 * @param da The dynamic array.
 * @param pos The position.
 * @return The element at the given position.
 */
#define buzzdarray_get(da, pos, type) ((type*)((da)->data) + (pos))

/*
 * Returns the size of the dynamic array.
 * @param da The dynamic array.
 * @return The size of the dynamic array.
 */
#define buzzdarray_size(da) (da)->size

/*
 * Returns the capacity of the dynamic array.
 * @param da The dynamic array.
 * @return The capacity of the dynamic array.
 */
#define buzzdarray_capacity(da) (da)->capacity

/*
 * Returns <tt>true</tt> if the dynamic array is empty.
 * @param da The dynamic array.
 * @return <tt>true</tt> if the dynamic array is empty.
 */
#define buzzdarray_isempty(da) (buzzdarray_size(da) == 0)

/*
 * Pushes an element in the dynamic array.
 * The element is appended.
 * @param da The dynamic array.
 * @param data The element to add.
 */
#define buzzdarray_push(da, data) buzzdarray_insert(da, buzzdarray_size(da), data)

/*
 * Pops an element from the dynamic array.
 * The last element is removed.
 * @param da The dynamic array.
 */
#define buzzdarray_pop(da) buzzdarray_remove(da, buzzdarray_size(da)-1)

#endif
