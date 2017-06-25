#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

#define BUZZTYPE_TABLE_BUCKETS 10

const char *buzztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata" };

/****************************************/
/****************************************/

uint32_t buzzobj_table_hash(const void* key) {
   buzzobj_t k = *(buzzobj_t*)key;
   switch(k->o.type) {
      case BUZZTYPE_INT: {
         return (k->i.value % BUZZTYPE_TABLE_BUCKETS);
      }
      case BUZZTYPE_FLOAT: {
         return ((uint32_t)(k->f.value) % BUZZTYPE_TABLE_BUCKETS);
      }
      case BUZZTYPE_STRING: {
         return ((uint32_t)(k->s.value.sid) % BUZZTYPE_TABLE_BUCKETS);
      }
      default:
         fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
         abort();
   }
}

int buzzobj_table_keycmp(const void* a, const void* b) {
   return buzzobj_cmp(*(buzzobj_t*)a, *(buzzobj_t*)b);
}

buzzobj_t buzzobj_new(uint16_t type) {
   /* Create a new object. calloc() fills it with zeroes */
   buzzobj_t o = (buzzobj_t)calloc(1, sizeof(union buzzobj_u));
   /* Set the object type */
   o->o.type = type;
   /* Set the object marker */
   o->o.marker = 0;
   /* Take care of special initialization for specific types */
   if(type == BUZZTYPE_TABLE) {
      o->t.value = buzzdict_new(BUZZTYPE_TABLE_BUCKETS,
                                sizeof(buzzobj_t),
                                sizeof(buzzobj_t),
                                buzzobj_table_hash,
                                buzzobj_table_keycmp,
                                NULL);
   }
   else if(type == BUZZTYPE_CLOSURE) {
      o->c.value.actrec = buzzdarray_new(1, sizeof(buzzobj_t), NULL);
   }
   /* All done */
   return o;
}

/****************************************/
/****************************************/

void buzzobj_iclone_tableelem(const void* key, void* data, void* params) {
   buzzobj_t k = buzzobj_iclone(*(buzzobj_t*)key);
   buzzobj_t d = buzzobj_iclone(*(buzzobj_t*)data);
   buzzdict_set((buzzdict_t)params, &k, &d);
}

buzzobj_t buzzobj_iclone(const buzzobj_t o) {
   buzzobj_t x = (buzzobj_t)malloc(sizeof(union buzzobj_u));
   x->o.type = o->o.type;
   x->o.marker = o->o.marker;
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
         buzzdict_foreach(orig, buzzobj_iclone_tableelem, x->t.value);
         return x;
      }
      default:
         fprintf(stderr, "[BUG] %s:%d: Clone for Buzz object type %d\n", __FILE__, __LINE__, o->o.type);
         abort();
   }
}

/****************************************/
/****************************************/

void buzzobj_destroy(buzzobj_t* o) {
   if((*o)->o.type == BUZZTYPE_TABLE) {
      buzzdict_destroy(&((*o)->t.value));
   }
   else if((*o)->o.type == BUZZTYPE_CLOSURE) {
      buzzdarray_destroy(&((*o)->c.value.actrec));
   }
   free(*o);
   *o = NULL;
}

/****************************************/
/****************************************/

uint32_t buzzobj_hash(const buzzobj_t o) {
   switch(o->o.type) {
      case BUZZTYPE_NIL: {
         return 0;
      }
      case BUZZTYPE_INT: {
         return buzzdict_int32keyhash(&(o->i.value));
      }
      case BUZZTYPE_FLOAT: {
         int32_t x = o->f.value;
         return buzzdict_int32keyhash(&x);
      }
      case BUZZTYPE_STRING: {
         return buzzdict_strkeyhash(&(o->s.value.str));
      }
      case BUZZTYPE_TABLE: {
         uint32_t p = (uintptr_t)(o->t.value);
         return buzzdict_uint32keyhash(&p);
      }
      case BUZZTYPE_USERDATA: {
         uint32_t p = (uintptr_t)(o->u.value);
         return buzzdict_uint32keyhash(&p);
      }
      case BUZZTYPE_CLOSURE:
      default:
         fprintf(stderr, "[BUG] %s:%d: Hash for Buzz object type %d\n", __FILE__, __LINE__, o->o.type);
         abort();
   }
}

/****************************************/
/****************************************/

int buzzobj_eq(const buzzobj_t a,
               const buzzobj_t b) {
   /* Make sure the type is the same */
   if(a->o.type != b->o.type) return 0;
   /* Equality means different things for different types */
   switch(a->o.type) {
      case BUZZTYPE_NIL:    return 1;
      case BUZZTYPE_INT:    return (a->i.value == b->i.value);
      case BUZZTYPE_FLOAT:  return (a->f.value == b->f.value);
      case BUZZTYPE_STRING: return 0; // TODO
      case BUZZTYPE_TABLE:  return ((uintptr_t)(a->t.value) == (uintptr_t)(b->t.value));
      case BUZZTYPE_CLOSURE:
         return((a->c.value.isnative == b->c.value.isnative) &&
                (a->c.value.ref      == b->c.value.ref)      &&
                (a->c.value.actrec   == b->c.value.actrec));
      case BUZZTYPE_USERDATA: return ((uintptr_t)(a->u.value) == (uintptr_t)(b->u.value));
      default:
         fprintf(stderr, "[BUG] %s:%d: Equality test between wrong Buzz objects types %d and %d\n", __FILE__, __LINE__, a->o.type, b->o.type);
         abort();
   }
}

/****************************************/
/****************************************/

int buzzobj_cmp(const buzzobj_t a,
                const buzzobj_t b) {
   /* Nil */
   if(a->o.type == BUZZTYPE_NIL && b->o.type == BUZZTYPE_NIL) {
      return 0;
   }
   if(a->o.type == BUZZTYPE_NIL) {
      return -1;
   }
   if(b->o.type == BUZZTYPE_NIL) {
      return 1;
   }
   /* Numeric types */
   if(a->o.type == BUZZTYPE_INT && b->o.type == BUZZTYPE_INT) {
      if(a->i.value < b->i.value) return -1;
      if(a->i.value > b->i.value) return 1;
      return 0;
   }
   if(a->o.type == BUZZTYPE_INT && b->o.type == BUZZTYPE_FLOAT) {
      if(a->i.value < b->f.value) return -1;
      if(a->i.value > b->f.value) return 1;
      return 0;
   }
   if(a->o.type == BUZZTYPE_FLOAT && b->o.type == BUZZTYPE_INT) {
      if(a->f.value < b->i.value) return -1;
      if(a->f.value > b->i.value) return 1;
      return 0;
   }
   if(a->o.type == BUZZTYPE_FLOAT && b->o.type == BUZZTYPE_FLOAT) {
      if(a->f.value < b->f.value) return -1;
      if(a->f.value > b->f.value) return 1;
      return 0;
   }
   /* String and other types */
   if(a->o.type == BUZZTYPE_STRING && b->o.type == BUZZTYPE_STRING) {
      return strcmp(a->s.value.str, b->s.value.str);
   }

   if(a->o.type == BUZZTYPE_STRING && b->o.type == BUZZTYPE_INT) {
      char str[30];
      sprintf(str, "%d", b->i.value);
      return strcmp(a->s.value.str, str);
   }
   if(a->o.type == BUZZTYPE_STRING && b->o.type == BUZZTYPE_FLOAT) {
      char str[30];
      sprintf(str, "%f", b->f.value);
      return strcmp(a->s.value.str, str);
   }

   if(a->o.type == BUZZTYPE_INT && b->o.type == BUZZTYPE_STRING) {
      char str[30];
      sprintf(str, "%d", a->i.value);
      return strcmp(str, b->s.value.str);
   }
   if(a->o.type == BUZZTYPE_FLOAT && b->o.type == BUZZTYPE_STRING) {
      char str[30];
      sprintf(str, "%f", a->f.value);
      return strcmp(str, b->s.value.str);
   }
   /* Tables */
   if(a->o.type == BUZZTYPE_TABLE && b->o.type == BUZZTYPE_TABLE) {
      if((uintptr_t)(a->t.value) < (uintptr_t)(b->t.value)) return -1;
      if((uintptr_t)(a->t.value) > (uintptr_t)(b->t.value)) return 1;
      return 0;
   }
   /* Userdata */
   if(a->o.type == BUZZTYPE_USERDATA && b->o.type == BUZZTYPE_USERDATA) {
      if((uintptr_t)(a->u.value) < (uintptr_t)(b->u.value)) return -1;
      if((uintptr_t)(a->u.value) > (uintptr_t)(b->u.value)) return 1;
      return 0;
   }
   // TODO better error management
   fprintf(stderr, "[TODO] %s:%d: Error for comparison between Buzz objects of types %d and %d\n", __FILE__, __LINE__, a->o.type, b->o.type);
   abort();
}

/****************************************/
/****************************************/

int buzzobj_type(buzzvm_t vm) {
   /* Get parameter */
   buzzvm_lnum_assert(vm, 1);
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   buzzvm_pop(vm);
   /* Return a string with the type */
   buzzvm_pushs(vm, buzzvm_string_register(vm, buzztype_desc[o->o.type], 0));
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzobj_clone(buzzvm_t vm) {
   /* Get parameter */
   buzzvm_lnum_assert(vm, 1);
   buzzvm_lload(vm, 1);
   buzzobj_t o = buzzvm_stack_at(vm, 1);
   buzzvm_pop(vm);
   /* Return a clone of the object */
   buzzvm_push(vm, buzzobj_iclone(o));
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

int buzzobj_size(buzzvm_t vm) {
   /* Get table parameter and make sure it's a table */
   buzzvm_lnum_assert(vm, 1);
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   buzzobj_t t = buzzvm_stack_at(vm, 1);
   buzzvm_pop(vm);
   buzzvm_pushi(vm, buzzdict_size(t->t.value));
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

struct buzzobj_foreach_params {
   buzzvm_t vm;
   buzzobj_t fun;
};

void buzzobj_foreach_entry(const void* key, void* data, void* params) {
   /* Cast params */
   struct buzzobj_foreach_params* p = (struct buzzobj_foreach_params*)params;
   if(p->vm->state != BUZZVM_STATE_READY) return;
   /* Push closure and params (key and value) */
   buzzvm_push(p->vm, p->fun);
   buzzvm_push(p->vm, *(buzzobj_t*)key);
   buzzvm_push(p->vm, *(buzzobj_t*)data);
   /* Call closure */
   p->vm->state = buzzvm_closure_call(p->vm, 2);
}

int buzzobj_foreach(buzzvm_t vm) {
   /* Table and closure expected */
   buzzvm_lnum_assert(vm, 2);
   /* Get table */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   buzzobj_t t = buzzvm_stack_at(vm, 1);
   /* Get closure */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   buzzobj_t c = buzzvm_stack_at(vm, 1);
   /* Go through the table element and apply the closure */
   struct buzzobj_foreach_params p = { .vm = vm, .fun = c };
   buzzdict_foreach(t->t.value, buzzobj_foreach_entry, &p);
   return buzzvm_ret0(vm);
}

/****************************************/
/****************************************/

struct buzzobj_map_params {
   buzzvm_t vm;
   buzzobj_t fun;
};

void buzzobj_map_entry(const void* key, void* data, void* params) {
   /* Cast params */
   struct buzzobj_map_params* p = (struct buzzobj_map_params*)params;
   if(p->vm->state != BUZZVM_STATE_READY) return;
   /* Duplicate the output table */
   buzzvm_dup(p->vm);
   /* Push current key (later we'll set the value in the output table) */
   buzzvm_push(p->vm, *(buzzobj_t*)key);
   /* Save current stack size */
   uint32_t ss = buzzvm_stack_top(p->vm);
   /* Push closure and params (current key and value) */
   buzzvm_push(p->vm, p->fun);
   buzzvm_push(p->vm, *(buzzobj_t*)key);
   buzzvm_push(p->vm, *(buzzobj_t*)data);
   /* Call closure */
   p->vm->state = buzzvm_closure_call(p->vm, 2);
   /* Make sure a value was returned */
   if(buzzvm_stack_top(p->vm) <= ss) {
      /* Error */
      buzzvm_seterror(p->vm,
                      BUZZVM_ERROR_STACK,
                      "map(table,function) expects the function to return a value");
      return;
   }
   /* Set new table value */
   buzzvm_tput(p->vm);
}

int buzzobj_map(buzzvm_t vm) {
   /* Table and closure expected */
   buzzvm_lnum_assert(vm, 2);
   /* Get input table */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   buzzobj_t t = buzzvm_stack_at(vm, 1);
   /* Get closure */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   buzzobj_t c = buzzvm_stack_at(vm, 1);
   /* Create output table */
   buzzvm_pusht(vm);
   /* Go through the table element and apply the closure */
   struct buzzobj_map_params p = { .vm = vm, .fun = c };
   buzzdict_foreach(t->t.value, buzzobj_map_entry, &p);
   /* Return the output table */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

struct buzzobj_reduce_params {
   buzzvm_t vm;
   buzzobj_t fun;
};

void buzzobj_reduce_entry(const void* key, void* data, void* params) {
   /* Cast params */
   struct buzzobj_reduce_params* p = (struct buzzobj_reduce_params*)params;
   if(p->vm->state != BUZZVM_STATE_READY) return;
   /* Save and pop accumulator from the stack */
   buzzobj_t accum = buzzvm_stack_at(p->vm, 1);
   buzzvm_pop(p->vm);
   /* Save current stack size */
   uint32_t ss = buzzvm_stack_top(p->vm);
   /* Push closure and params (key and value) */
   buzzvm_push(p->vm, p->fun);
   buzzvm_push(p->vm, *(buzzobj_t*)key);
   buzzvm_push(p->vm, *(buzzobj_t*)data);
   buzzvm_push(p->vm, accum);
   /* Call closure */
   p->vm->state = buzzvm_closure_call(p->vm, 3);
   /* Make sure a value was returned */
   if(buzzvm_stack_top(p->vm) <= ss)
      /* Error */
      buzzvm_seterror(p->vm,
                      BUZZVM_ERROR_STACK,
                      "reduce(table,function,accumulator) expects the function to return a value");
}

int buzzobj_reduce(buzzvm_t vm) {
   /* Table, closure, and initial value expected */
   buzzvm_lnum_assert(vm, 3);
   /* Get table */
   buzzvm_lload(vm, 1);
   buzzvm_type_assert(vm, 1, BUZZTYPE_TABLE);
   buzzobj_t t = buzzvm_stack_at(vm, 1);
   /* Get closure */
   buzzvm_lload(vm, 2);
   buzzvm_type_assert(vm, 1, BUZZTYPE_CLOSURE);
   buzzobj_t c = buzzvm_stack_at(vm, 1);
   /* Put initial accumulator value on the stack */
   buzzvm_lload(vm, 3);
   /* Go through the table element and apply the closure */
   struct buzzobj_reduce_params p = { .vm = vm, .fun = c };
   buzzdict_foreach(t->t.value, buzzobj_reduce_entry, &p);
   /* The final value of the accumulator is on the stack */
   return buzzvm_ret1(vm);
}

/****************************************/
/****************************************/

void buzzobj_serialize_tableelem(const void* key, void* data, void* params) {
   buzzobj_serialize((buzzdarray_t)params, *(buzzobj_t*)key);
   buzzobj_serialize((buzzdarray_t)params, *(buzzobj_t*)data);
}

void buzzobj_serialize_arrayelem(uint32_t pos, void* data, void* params) {
   buzzobj_serialize((buzzdarray_t)params, *(buzzobj_t*)data);
}

void buzzobj_serialize(buzzdarray_t buf,
                       const buzzobj_t data) {
   buzzmsg_serialize_u16(buf, data->o.type);
   switch(data->o.type) {
      case BUZZTYPE_NIL: {
         break;
      }
      case BUZZTYPE_INT: {
         buzzmsg_serialize_u32(buf, data->i.value);
         break;
      }
      case BUZZTYPE_FLOAT: {
         buzzmsg_serialize_float(buf, data->f.value);
         break;
      }
      case BUZZTYPE_STRING: {
         buzzmsg_serialize_string(buf, data->s.value.str);
         break;
      }
      case BUZZTYPE_TABLE: {
         buzzmsg_serialize_u32(buf, buzzdict_size(data->t.value));
         buzzdict_foreach(data->t.value, buzzobj_serialize_tableelem, buf);
         break;
      }
      default:
         fprintf(stderr, "[TODO] %s %d\n", __FILE__, __LINE__);
   }
}

/****************************************/
/****************************************/

int64_t buzzobj_deserialize(buzzobj_t* data,
                            buzzdarray_t buf,
                            uint32_t pos,
                            struct buzzvm_s* vm) {
   int64_t p = pos;
   uint16_t type;
   p = buzzmsg_deserialize_u16(&type, buf, p);
   if(p < 0) return -1;
   *data = buzzheap_newobj(vm->heap, type);
   switch(type) {
      case BUZZTYPE_NIL: {
         return p;
      }
      case BUZZTYPE_INT: {
         return buzzmsg_deserialize_u32((uint32_t*)(&((*data)->i.value)), buf, p);
      }
      case BUZZTYPE_FLOAT: {
         return buzzmsg_deserialize_float(&((*data)->f.value), buf, p);
      }
      case BUZZTYPE_STRING: {
         char* str;
         p = buzzmsg_deserialize_string(&str, buf, p);
         if(p < 0) return -1;
         (*data)->s.value.sid = buzzstrman_register(vm->strings, str, 0);
         (*data)->s.value.str = buzzstrman_get(vm->strings, (*data)->s.value.sid);
         free(str);
         return p;
      }
      case BUZZTYPE_TABLE: {
         uint32_t size, i;
         p = buzzmsg_deserialize_u32(&size, buf, p);
         if(p < 0) return -1;
         for(i = 0; i < size; ++i) {
            buzzobj_t k;
            buzzobj_t v;
            p = buzzobj_deserialize(&k, buf, p, vm);
            if(p < 0) return -1;
            p = buzzobj_deserialize(&v, buf, p, vm);
            if(p < 0) return -1;
            buzzdict_set((*data)->t.value, &k, &v);
         }
         return p;
      }
      default:
         fprintf(stderr, "TODO: %s %d\n", __FILE__, __LINE__);
         return -1;
   }
}

/****************************************/
/****************************************/

#define function_register(FNAME)                                        \
   buzzvm_pushs(vm, buzzvm_string_register(vm, #FNAME, 1));             \
   buzzvm_pushcc(vm, buzzvm_function_register(vm, buzzobj_ ## FNAME));  \
   buzzvm_gstore(vm);

int buzzobj_register(struct buzzvm_s* vm) {
   function_register(type);
   function_register(clone);
   function_register(size);
   function_register(foreach);
   function_register(map);
   function_register(reduce);
   return vm->state;
}

/****************************************/
/****************************************/
