#ifndef BUZZDICT_H
#define BUZZDICT_H

#include <buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Function pointer for an element-wise function:
    *
    * void f(uint32_t pos, void* data, void* params)
    *
    * This function pointer is used to destroy elements by
    * buzzdict_destroy() and in methods such as
    * buzzdict_foreach().
    */
   typedef void (*buzzdict_elem_funp)(void* key, void* data, void* params);
   
   /*
    * Function pointer for key hashing.
    *
    * uin32_t f(const void* data)
    */
   typedef uint32_t (*buzzdict_hashfunp)(const void* data);

   /*
    * Function pointer to compare dictionary keys.
    *
    * int f(const void* a, const void* b)
    *
    * The function must return:
    * -1 if *a < *b
    * 0  if *a == *b
    * 1  if *a > *b
    */
   typedef int (*buzzdict_key_cmpp)(const void* a, const void* b);

   /*
    * The Buzz dictionary.
    */
   struct buzzdict_s {
      buzzdarray_t* buckets;
      uint32_t num_buckets;
      uint32_t size;
      buzzdict_hashfunp hashf;
      buzzdict_key_cmpp keycmpf;
   };
   typedef struct buzzdict_s* buzzdict_t;

   /*
    * Create a new dictionary.
    * @param buckets The number of buckets.
    * @param hashf The function to hash the keys.
    * @param keycmpf The function to compare the keys.
    * @return A new dictionary.
    */
   extern buzzdict_t buzzdict_new(uint32_t buckets,
                                  buzzdict_hashfunp hashf,
                                  buzzdict_key_cmpp keycmpf);

   extern void buzzdict_destroy(buzzdict_t* dt);

   extern void* buzzdict_get(buzzdict_t dt,
                             const void* key);

   extern void buzzdict_set(buzzdict_t dt,
                            const void* key,
                            void* data);

   extern void buzzdict_remove(buzzdict_t dt,
                               const void* key);

   extern void buzzdict_foreach(buzzdict_t dt,
                                buzzdict_elem_funp fun,
                                void* params);

#ifdef __cplusplus
}
#endif

#define buzzdict_size(dt) (dt)->size

#define buzzdict_isempty(dt) ((dt)->size == 0)

#endif
