#ifndef BUZZHEAP_H
#define BUZZHEAP_H

#include <buzz/buzztype.h>
#include <buzz/buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   /**
    * Forward declaration of the VM state.
    */
   struct buzzvm_s;

   /**
    * The state of the object heap
    */
   struct buzzheap_s {
      /* The list of all objects */
      buzzdarray_t objs;
      /* The maximum number of vars after which GC is triggered */
      uint32_t max_objs;
      /* Current marker for garbage collection */
      uint16_t marker;
   };
   typedef struct buzzheap_s* buzzheap_t;

   /**
    * Creates a new heap.
    * @return The state of a new heap.
    */
   buzzheap_t buzzheap_new();

   /**
    * Destroys a heap.
    * @param A pointer to the heap.
    */
   void buzzheap_destroy(buzzheap_t* h);

   /**
    * Creates a new Buzz object.
    * @param h The heap.
    * @param The object type.
    * @param The new object.
    */
   buzzobj_t buzzheap_newobj(buzzheap_t h,
                             uint16_t type);

   /**
    * Performs garbage collection, if necessary.
    * Internally uses a simple mark-and-sweep algorithm.
    * @param vm The Buzz VM.
    * @param h The heap.
    */
   void buzzheap_gc(struct buzzvm_s* vm);

#ifdef __cplusplus
}
#endif

#define buzzheap_addvar();

#endif
