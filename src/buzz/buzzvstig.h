#ifndef BUZZVMSTIG_H
#define BUZZVMSTIG_H

#include <buzz/buzztype.h>
#include <buzz/buzzdict.h>

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
      uint16_t robot;
   };
   typedef struct buzzvstig_elem_s* buzzvstig_elem_t;

   /*
    * The virtual stigmergy data.
    */
   struct buzzvstig_s {
      buzzdict_t data;
      buzzobj_t onconflict;
      buzzobj_t onconflictlost;
   };
   typedef struct buzzvstig_s* buzzvstig_t;

   /*
    * Forward declaration of the Buzz VM.
    */
   struct buzzvm_s;

   /*
    * Registers the virtual stigmergy methods in the vm.
    * @param vm The state of the 
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_register(struct buzzvm_s* vm);

   /*
    * Creates a new virtual stigmergy entry.
    * @param data The data associated to the entry.
    * @param timestamp The timestamp (Lamport clock).
    * @param robot The robot id.
    * @return The new virtual stigmergy entry.
    */
   extern buzzvstig_elem_t buzzvstig_elem_new(buzzobj_t data,
                                              uint16_t timestamp,
                                              uint16_t robot);

   /*
    * Clones a virtual stigmergy entry.
    * @param vm The Buzz VM state.
    * @param e The entry to clone.
    * @return A new virtual stigmergy entry.
    */
   extern buzzvstig_elem_t buzzvstig_elem_clone(struct buzzvm_s* vm,
                                                const buzzvstig_elem_t e);

   /*
    * Creates a new virtual stigmergy structure.
    * @return The new virtual stigmergy structure.
    */
   extern buzzvstig_t buzzvstig_new();

   /*
    * Destroys a virtual stigmergy structure.
    * @param vs The virtual stigmergy structure.
    */
   extern void buzzvstig_destroy(buzzvstig_t* vs);

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
   extern int buzzvstig_create(struct buzzvm_s* vm);

   /*
    * Buzz C closure to get the number of elements in a virtual stigmergy structure.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_size(struct buzzvm_s* vm);

   /*
    * Buzz C closure to put an element in a stigmergy object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_put(struct buzzvm_s* vm);

   /*
    * Buzz C closure to get an element from a stigmergy object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_get(struct buzzvm_s* vm);

   /*
    * Buzz C closure to loop through the elements of a stigmergy object.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_foreach(struct buzzvm_s* vm);

   /*
    * Buzz C closure to loop through the elements of a stigmergy object and produce a single value.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_reduce(struct buzzvm_s* vm);

   /*
    * Buzz C closure to set the function to call on write conflict.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_onconflict(struct buzzvm_s* vm);

   /*
    * Buzz C closure to set the function to call on loss of conflict.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern int buzzvstig_onconflictlost(struct buzzvm_s* vm);

   /*
    * Calls the write conflict manager.
    * @param vm The Buzz VM state.
    * @return The updated VM state.
    */
   extern buzzvstig_elem_t buzzvstig_onconflict_call(struct buzzvm_s* vm,
                                                     buzzvstig_t vs,
                                                     buzzobj_t k,
                                                     buzzvstig_elem_t lv,
                                                     buzzvstig_elem_t rv);

   /*
    * Calls the lost conflict manager.
    * @param vm The Buzz VM state.
    */
   extern void buzzvstig_onconflictlost_call(struct buzzvm_s* vm,
                                             buzzvstig_t vs,
                                             buzzobj_t k,
                                             buzzvstig_elem_t lv);

#ifdef __cplusplus
}
#endif

/*
 * Looks for an element in a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key to look for.
 * @return The data associated to the key, or NULL if not found.
 */
#define buzzvstig_fetch(vs, key) buzzdict_get((vs)->data, (key), buzzvstig_elem_t)

/*
 * Puts data into a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key.
 * @param el The element.
 */
#define buzzvstig_store(vs, key, el) buzzdict_set((vs)->data, (key), (el));

/*
 * Deletes data from a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key.
 */
#define buzzvstig_remove(vs, key) buzzdict_remove((vs)->data, (key));

/*
 * Applies the given function to each element in the virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param fun The function.
 * @param params A buffer to pass along.
 * @see buzzdict_foreach()
 */
#define buzzvstig_foreach_elem(vs, fun, params) buzzdict_foreach((vs)->data, fun, params);

#endif
