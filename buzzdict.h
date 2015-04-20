#ifndef BUZZDICT_H
#define BUZZDICT_H

#include <buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Function pointer for an element-wise function:
    *
    * void f(const void* key, void* data, void* params)
    *
    * This function pointer is used to destroy elements by
    * buzzdict_destroy() and in methods such as
    * buzzdict_foreach().
    */
   typedef void (*buzzdict_elem_funp)(const void* key, void* data, void* params);
   
   /*
    * Function pointer for key hashing.
    *
    * uin32_t f(const void* key)
    */
   typedef uint32_t (*buzzdict_hashfunp)(const void* key);

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
      uint32_t size;
      uint32_t num_buckets;
      buzzdict_hashfunp hashf;
      buzzdict_key_cmpp keycmpf;
      uint32_t key_size;
      uint32_t data_size;
   };
   typedef struct buzzdict_s* buzzdict_t;

   /*
    * Create a new dictionary.
    * @param buckets The number of buckets.
    * @param key_size The size of a key.
    * @param data_size The size of a data element.
    * @param hashf The function to hash the keys.
    * @param keycmpf The function to compare the keys.
    * @return A new dictionary.
    */
   extern buzzdict_t buzzdict_new(uint32_t buckets,
                                  uint32_t key_size,
                                  uint32_t data_size,
                                  buzzdict_hashfunp hashf,
                                  buzzdict_key_cmpp keycmpf);

   extern void buzzdict_destroy(buzzdict_t* dt);

   extern void* buzzdict_rawget(buzzdict_t dt,
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

#define buzzdict_exists(dt, key) (buzzdict_rawget(dt, key) != NULL)

#define buzzdict_get(dt, key, type) ((type*)buzzdict_rawget(dt, key))

#endif
