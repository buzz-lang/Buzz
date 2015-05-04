#ifndef BUZZVMSTIG_H
#define BUZZVMSTIG_H

#include <buzztype.h>
#include <buzzdict.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * An entry in virtual stigmergy.
    */
   struct buzzvstig_elem_s {
      buzzobj_t data;
      uint32_t timestamp;
      uint32_t robot;
   };
   typedef struct buzzvstig_elem_s buzzvstig_elem_t;

   /*
    * The virtual stigmergy data.
    */
   typedef buzzdict_t buzzvstig_t;

   /*
    * Creates a new virtual stigmergy structure.
    * @return The new virtual stigmergy structure.
    */
   extern buzzvstig_t buzzvstig_new();

   /*
    * Serializes an element in the virtual stigmergy.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param key The key of the element to serialize.
    * @param data The data of the element to serialize.
    */
   extern void buzzvstig_elem_serialize(buzzmsg_t buf,
                                        int32_t key,
                                        const buzzvstig_elem_t* data);

   /*
    * Deserializes a virtual stigmergy element.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param key The deserialized key of the element.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzvstig_elem_deserialize(int32_t* key,
                                             buzzvstig_elem_t* data,
                                             buzzmsg_t buf,
                                             uint32_t pos);
   
#ifdef __cplusplus
}
#endif

/*
 * Destroys a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 */
#define buzzvstig_destroy(vs) buzzdict_destroy(vs)

/*
 * Looks for an element in a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key to look for.
 * @return The data associated to the key, or NULL if not found.
 */
#define buzzvstig_fetch(vs, key) buzzdict_get((vs), &(key), buzzvstig_elem_t)

/*
 * Puts data into a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key.
 * @param el The element.
 */
#define buzzvstig_store(vs, key, el) buzzdict_set((vs), &(key), (el));

/*
 * Applies the given function to each element in the virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param fun The function.
 * @param params A buffer to pass along.
 * @see buzzdict_foreach()
 */
#define buzzvstig_foreach(vs, fun, params) buzzdict_foreach(vs, fun, params);

#endif
