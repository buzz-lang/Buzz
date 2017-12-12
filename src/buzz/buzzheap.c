#include "buzzheap.h"
#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>

/****************************************/
/****************************************/

#define BUZZHEAP_GC_INIT_MAXOBJS 1

/****************************************/
/****************************************/

void buzzheap_destroy_obj(uint32_t pos, void* data, void* params) {
   buzzobj_destroy((buzzobj_t*)data);
}

buzzheap_t buzzheap_new() {
   /* Create heap state */
   buzzheap_t h = (buzzheap_t)malloc(sizeof(struct buzzheap_s));
   /* Create object list */
   h->objs = buzzdarray_new(10, sizeof(buzzobj_t), buzzheap_destroy_obj);
   /* Initialize GC max object threshold */
   h->max_objs = BUZZHEAP_GC_INIT_MAXOBJS;
   /* Initialize the marker */
   h->marker = 0;
   /* All done */
   return h;
}

/****************************************/
/****************************************/

void buzzheap_destroy(buzzheap_t* h) {
   /* Get rid of object list */
   buzzdarray_destroy(&((*h)->objs));
   /* Get rid of heap state */
   free(*h);
   /* Set heap to NULL */
   *h = NULL;
}

/****************************************/
/****************************************/

buzzobj_t buzzheap_newobj(buzzvm_t vm,
                          uint16_t type) {
   /* Create a new object. calloc() fills it with zeroes */
   buzzobj_t o = buzzobj_new(type);
   /* Set the object marker */
   o->o.marker = vm->heap->marker;
   /* Add object to list */
   buzzdarray_push(vm->heap->objs, &o);
   /* All done */
   return o;
}

/****************************************/
/****************************************/

struct buzzheap_clone_tableelem_s {
   buzzvm_t vm;
   buzzdict_t t;
};

void buzzheap_clone_tableelem(const void* key, void* data, void* params) {
   struct buzzheap_clone_tableelem_s* p = (struct buzzheap_clone_tableelem_s*)params;
   buzzobj_t k = buzzheap_clone(p->vm, *(buzzobj_t*)key);
   buzzobj_t d = buzzheap_clone(p->vm, *(buzzobj_t*)data);
   buzzdict_set(p->t, &k, &d);
}

buzzobj_t buzzheap_clone(buzzvm_t vm, const buzzobj_t o) {
   buzzobj_t x = (buzzobj_t)malloc(sizeof(union buzzobj_u));
   x->o.type = o->o.type;
   x->o.marker = o->o.marker;
   buzzdarray_push(vm->heap->objs, &x);
   switch(o->o.type) {
      case BUZZTYPE_NIL: {
         return x;
      }
      case BUZZTYPE_INT: {
         x->i.value = o->i.value;
         return x;
      }
      case BUZZTYPE_FLOAT: {
         x->f.value = o->f.value;
         return x;
      }
      case BUZZTYPE_STRING: {
         x->s.value.sid = o->s.value.sid;
         x->s.value.str = o->s.value.str;
         return x;
      }
      case BUZZTYPE_USERDATA: {
         x->u.value = o->u.value;
         return x;
      }
      case BUZZTYPE_CLOSURE: {
         x->c.value.ref = o->c.value.ref;
         x->c.value.actrec = buzzdarray_clone(o->c.value.actrec);
         x->c.value.isnative = o->c.value.isnative;
         return x;
      }
      case BUZZTYPE_TABLE: {
         buzzdict_t orig = o->t.value;
         x->t.value = buzzdict_new(orig->num_buckets,
                                   orig->key_size,
                                   orig->data_size,
                                   orig->hashf,
                                   orig->keycmpf,
                                   orig->dstryf);
         struct buzzheap_clone_tableelem_s p = {
            .vm = vm,
            .t = x->t.value
         };
         buzzdict_foreach(orig, buzzheap_clone_tableelem, &p);
         return x;
      }
      default:
         fprintf(stderr, "[BUG] %s:%d: Clone for Buzz object type %d\n", __FILE__, __LINE__, o->o.type);
         abort();
   }
}

/****************************************/
/****************************************/

static void buzzheap_objmark(buzzobj_t o, buzzvm_t h);
static void buzzheap_darrayobj_mark(uint32_t pos, void* data, void* params);
static void buzzheap_dictobj_mark(const void* key, void* data, void* params);

void buzzheap_objmark(buzzobj_t o,
                      buzzvm_t vm) {
   /*
    * Nothing to do if the object is already marked
    * This avoids infinite looping when cycles are present
    */
   if(o->o.marker == vm->heap->marker) return;
   /* Update marker */
   o->o.marker = vm->heap->marker;
   /* Take care of composite types */
   if(o->o.type == BUZZTYPE_TABLE)
      buzzdict_foreach(o->t.value,
                       buzzheap_dictobj_mark,
                       vm);
   else if(o->o.type == BUZZTYPE_CLOSURE)
      buzzdarray_foreach(o->c.value.actrec,
                         buzzheap_darrayobj_mark,
                         vm);
   else if(o->o.type == BUZZTYPE_STRING)
      buzzstrman_gc_mark(vm->strings,
                         o->s.value.sid);
}

void buzzheap_dictobj_mark(const void* key, void* data, void* params) {
   buzzheap_objmark(*(buzzobj_t*)key, params);
   buzzheap_objmark(*(buzzobj_t*)data, params);
}

void buzzheap_darrayobj_mark(uint32_t pos, void* data, void* params) {
   buzzheap_objmark(*(buzzobj_t*)data, (buzzvm_t)params);
}

void buzzheap_stack_mark(uint32_t pos, void* data, void* params) {
   buzzdarray_foreach(*(buzzdarray_t*)data,
                      buzzheap_darrayobj_mark,
                      params);
}

void buzzheap_lsyms_mark(uint32_t pos, void* data, void* params) {
   buzzdarray_foreach((*(buzzvm_lsyms_t*)data)->syms,
                      buzzheap_darrayobj_mark,
                      params);
}

void buzzheap_vstigobj_mark(const void* key, void* data, void* params) {
   buzzheap_objmark((*(buzzobj_t*)key), params);
   buzzheap_objmark((*(buzzvstig_elem_t*)data)->data, params);
}

void buzzheap_vstig_mark(const void* key, void* data, void* params) {
   buzzvstig_foreach_elem(*(buzzvstig_t*)data,
                          buzzheap_vstigobj_mark,
                          params);
}

void buzzheap_listener_mark(const void* key, void* data, void* params) {
   buzzstrman_gc_mark(((buzzvm_t)params)->strings, *(uint16_t*)key);
   buzzheap_objmark(*(buzzobj_t*)data, params);
}

void buzzheap_gsymobj_mark(const void* key, void* data, void* params) {
   buzzheap_objmark(*(buzzobj_t*)data, params);
}

void buzzheap_gc(struct buzzvm_s* vm) {
   buzzheap_t h = vm->heap;
   /* Is GC necessary? */
   if(buzzdarray_size(h->objs) < h->max_objs) return;
   /* Increase the marker */
   ++h->marker;
   /* Prepare string gc */
   buzzstrman_gc_clear(vm->strings);
   /* Go through all the objects in the global symbols and mark them */
   buzzdict_foreach(vm->gsyms, buzzheap_gsymobj_mark, vm);
   /* Go through all the objects in the VM stack and mark them */
   buzzdarray_foreach(vm->stacks, buzzheap_stack_mark, vm);
   /* Go through all the objects in the local symbol stack and mark them */
   buzzdarray_foreach(vm->lsymts, buzzheap_lsyms_mark, vm);
   /* Go through all the objects in the virtual stigmergy and mark them */
   buzzdict_foreach(vm->vstigs, buzzheap_vstig_mark, vm);
   /* Go through all the objects in the listeners and mark them */
   buzzdict_foreach(vm->listeners, buzzheap_listener_mark, vm);
   /* Go through all the objects in the out message queue and mark them */
   buzzoutmsg_gc(vm);
   /* Go through all the objects in the object list and delete the unmarked ones */
   int64_t i = buzzdarray_size(h->objs) - 1;
   while(i >= 0) {
      /* Check whether the marker is set to the latest value */
      if(buzzdarray_get(h->objs, i, buzzobj_t)->o.marker != h->marker) {
         /* No, erase the element */
         buzzdarray_remove(h->objs, i);
      }
      /* Next element */
      --i;
   }
   /* Perform string gc */
   buzzstrman_gc_prune(vm->strings);
   /* Update the max objects threshold */
   h->max_objs = buzzdarray_isempty(h->objs) ? BUZZHEAP_GC_INIT_MAXOBJS : 2 * buzzdarray_size(h->objs);
}

/****************************************/
/****************************************/
