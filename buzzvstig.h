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
      /* The data associated to the entry */
      buzzobj_t data;
      /* The timestamp (Lamport clock) */
      uint16_t timestamp;
      /* The robot id */
      uint32_t robot;
   };
   typedef struct buzzvstig_elem_s* buzzvstig_elem_t;

   /*
    * The virtual stigmergy data.
    */
   typedef buzzdict_t buzzvstig_t;

   /*
    * Forward declaration of the Buzz VM.
    */
   struct buzzvm_s;

   /*
    * Creates a new virtual stigmergy entry.
    * @param data The data associated to the entry.
    * @param timestamp The timestamp (Lamport clock).
    * @param robot The robot id.
    * @return The new virtual stigmergy entry.
    */
   extern buzzvstig_elem_t buzzvstig_elem_new(buzzobj_t data,
                                              uint16_t timestamp,
                                              uint32_t robot);

   /*
    * Clones a virtual stigmergy entry.
    * @param e The entry to clone.
    * @return A new virtual stigmergy entry.
    */
   extern buzzvstig_elem_t buzzvstig_elem_clone(const buzzvstig_elem_t e);

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
   extern void buzzvstig_elem_serialize(buzzmsg_payload_t buf,
                                        const buzzobj_t key,
                                        const buzzvstig_elem_t data);

   /*
    * Deserializes a virtual stigmergy element.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param key The deserialized key of the element.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @param vm The Buzz VM data.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzvstig_elem_deserialize(buzzobj_t* key,
                                             buzzvstig_elem_t* data,
                                             buzzmsg_payload_t buf,
                                             uint32_t pos,
                                             struct buzzvm_s* vm);
   
   /*
    * Buzz C closure to create a new stigmergy object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   int buzzvm_vstig_create(struct buzzvm_s* vm);

   /*
    * Buzz C closure to put an element in a stigmergy object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   int buzzvm_vstig_put(struct buzzvm_s* vm);

   /*
    * Buzz C closure to get an element from a stigmergy object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   int buzzvm_vstig_get(struct buzzvm_s* vm);

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
#define buzzvstig_fetch(vs, key) buzzdict_get((vs), (key), buzzvstig_elem_t)

/*
 * Puts data into a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key.
 * @param el The element.
 */
#define buzzvstig_store(vs, key, el) buzzdict_set((vs), (key), (el));

/*
 * Applies the given function to each element in the virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param fun The function.
 * @param params A buffer to pass along.
 * @see buzzdict_foreach()
 */
#define buzzvstig_foreach(vs, fun, params) buzzdict_foreach(vs, fun, params);

#endif
