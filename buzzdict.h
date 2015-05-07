#ifndef BUZZDICT_H
#define BUZZDICT_H

#include <buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   // TODO
   // CONSIDER USING BRENT'S VARIATION ON HASH COLLISION SOLVING
   // TODO

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
      buzzdarray_t* buckets;     // Bucket data
      uint32_t size;             // Number of inserted elements
      uint32_t num_buckets;      // Number of buckets
      buzzdict_hashfunp hashf;   // Key hashing function
      buzzdict_key_cmpp keycmpf; // Key comparison function
      buzzdict_elem_funp dstryf; // Element destroy function
      uint32_t key_size;         // Key size in bytes
      uint32_t data_size;        // Data size in bytes
   };
   typedef struct buzzdict_s* buzzdict_t;

   /*
    * Create a new dictionary.
    * @param buckets The number of buckets.
    * @param key_size The size of a key.
    * @param data_size The size of a data element.
    * @param hashf The function to hash the keys.
    * @param keycmpf The function to compare the keys.
    * @param dstryf The function to destroy an element. Can be NULL.
    * @return A new dictionary.
    */
   extern buzzdict_t buzzdict_new(uint32_t buckets,
                                  uint32_t key_size,
                                  uint32_t data_size,
                                  buzzdict_hashfunp hashf,
                                  buzzdict_key_cmpp keycmpf,
                                  buzzdict_elem_funp dstryf);

   /*
    * Destroys the given dictionary.
    * @param dt The dictionary.
    */
   extern void buzzdict_destroy(buzzdict_t* dt);

   /*
    * Looks for the element with the given key.
    * @param dt The dictionary.
    * @param key The key.
    * @return A void pointer to the element if found, or NULL.
    * @see buzzdict_get
    */
   extern void* buzzdict_rawget(buzzdict_t dt,
                                const void* key);

   /*
    * Sets a (key, data) pair.
    * @param dt The dictionary.
    * @param key The key.
    * @param data The data.
    */
   extern void buzzdict_set(buzzdict_t dt,
                            const void* key,
                            void* data);

   /*
    * Removes the element with the given key.
    * If the element is not found, nothing is done.
    * @param dt The dictionary.
    * @param key The key.
    * @return 1 if the element was found and removed; 0 otherwise
    */
   extern int buzzdict_remove(buzzdict_t dt,
                              const void* key);

   /*
    * Applies the given function to each element in the dictionary.
    * @param dt The dictionary.
    * @param fun The function.
    * @param params A buffer to pass along.
    */
   extern void buzzdict_foreach(buzzdict_t dt,
                                buzzdict_elem_funp fun,
                                void* params);

   /*
    * Hash functions for strings.
    * This is the djb2() hash function presented in
    * http://www.cse.yorku.ca/~oz/hash.html.
    * @param key The key to hash, cast to string.
    * @return A hash for the given key.
    */
   uint32_t buzzdict_strkeyhash(const void* key);

   /*
    * Comparison function for string keys.
    * @param a The first string key.
    * @param b The second string key.
    * @return -1 if a < b; 1 if a > b; 0 if a == b.
    */
   int buzzdict_strkeycmp(const void* a, const void* b);

   /*
    * Hash functions for integers.
    * TODO
    * @param key The key to hash, cast to int32_t.
    * @return A hash for the given key.
    */
   uint32_t buzzdict_intkeyhash(const void* key);

   /*
    * Comparison function for integer keys.
    * @param a The first key, cast to int32_t.
    * @param b The second key, cast to int32_t.
    * @return -1 if a < b; 1 if a > b; 0 if a == b.
    */
   int buzzdict_intkeycmp(const void* a, const void* b);

#ifdef __cplusplus
}
#endif

/*
 * Returns the number of elements in the dictionary.
 * @param dt The dictionary.
 */
#define buzzdict_size(dt) (dt)->size

/*
 * Returns 1 if the dictionary is empty, 0 otherwise.
 * @param dt The dictionary.
 */
#define buzzdict_isempty(dt) ((dt)->size == 0)

/*
 * Returns 1 if an element with the given key exists, 0 otherwise.
 * @param dt The dictionary.
 * @param key The key.
 */
#define buzzdict_exists(dt, key) (buzzdict_rawget(dt, key) != NULL)

/*
 * Returns the element corresponding to the given key.
 * The returned element is casted to a pointer to the given type.
 * @param dt The dictionary.
 * @param key The key.
 * @param type The element type.
 * @see buzzdict_rawget
 */
#define buzzdict_get(dt, key, type) ((type*)buzzdict_rawget(dt, key))

#endif
