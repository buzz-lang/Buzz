#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

#define BUZZTYPE_TABLE_BUCKETS 10

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
         return ((uint32_t)(k->f.value) % BUZZTYPE_TABLE_BUCKETS);
      }
      default:
         fprintf(stderr, "[TODO] %s:%d\n", __FILE__, __LINE__);
         exit(1);
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

void buzzobj_clone_tableelem(const void* key, void* data, void* params) {
   buzzobj_t k = buzzobj_clone(*(buzzobj_t*)key);
   buzzobj_t d = buzzobj_clone(*(buzzobj_t*)data);
   buzzdict_set((buzzdict_t)params, &k, &d);
}

buzzobj_t buzzobj_clone(const buzzobj_t o) {
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
         buzzdict_foreach(orig, buzzobj_clone_tableelem, x->t.value);
         return x;
      }
      default:
         fprintf(stderr, "[BUG] %s:%d: Clone for Buzz object type %d\n", __FILE__, __LINE__, o->o.type);
         exit(1);
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
         exit(1);
   }
}

/****************************************/
/****************************************/

int buzzobj_eq(const buzzobj_t a,
               const buzzobj_t b) {
   /* Make sure the type is the same */
   if(a->o.type != a->o.type) return 0;
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
         exit(1);
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
   exit(1);
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
         fprintf(stderr, "[TODO] %s %u\n", __FILE__, __LINE__);
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
         (*data)->s.value.sid = buzzvm_string_register(vm, str);
         (*data)->s.value.str = buzzvm_string_get(vm, (*data)->s.value.sid);
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
         fprintf(stderr, "TODO: %s %u\n", __FILE__, __LINE__);
         return -1;
   }
}

/****************************************/
/****************************************/
