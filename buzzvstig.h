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
      buzzvar_t data;
      uint32_t timestamp;
      uint32_t robot;
   };
   typedef struct buzzvstig_elem_s* buzzvstig_elem_t;

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
    * Destroys a virtual stigmergy structure.
    * @param vs The virtual stigmergy structure.
    */
   extern void buzzvstig_destroy(buzzvstig_t* vs);

   /*
    * Returns a new virtual stigmergy element.
    * @param data The data.
    * @param timestamp The timestamp.
    * @param robot The robot id.
    * @return A new virtual stigmergy element.
    */
   extern buzzvstig_elem_t buzzvstig_newelem(buzzvar_t data,
                                             uint32_t timestamp,
                                             uint32_t robot);

   /*
    * Looks for an element in a virtual stigmergy structure.
    * @param vs The virtual stigmergy structure.
    * @param key The key to look for.
    * @return The data associated to the key, or NULL if not found.
    */
   extern buzzvstig_elem_t buzzvstig_fetch(buzzvstig_t vs,
                                           int32_t key);

#ifdef __cplusplus
}
#endif

/*
 * Puts data into a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param key The key.
 * @param el The element.
 */
#define buzzvstig_store(vs, key, el) buzzdict_set((vs), &(key), &(el));

#endif
