#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
      case BUZZTYPE_ARRAY:
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
      case BUZZTYPE_NIL:     return 1;
      case BUZZTYPE_INT:     return (a->i.value == b->i.value);
      case BUZZTYPE_FLOAT:   return (a->f.value == b->f.value);
      case BUZZTYPE_STRING:  return 0; // TODO
      case BUZZTYPE_TABLE:   return ((uintptr_t)(a->t.value) == (uintptr_t)(b->t.value));
      case BUZZTYPE_ARRAY:   return ((uintptr_t)(a->a.value) == (uintptr_t)(b->a.value));
      case BUZZTYPE_CLOSURE:
         return((a->c.value.isnative == b->c.value.isnative) &&
                (a->c.value.ref      == b->c.value.ref)      &&
                (a->c.value.actrec   == b->c.value.actrec));
      default:
         fprintf(stderr, "[BUG] %s:%d: Equality test between wrong Buzz objects types %d and %d\n", __FILE__, __LINE__, a->o.type, b->o.type);
         exit(1);
   }
}

/****************************************/
/****************************************/

int buzzobj_cmp(const buzzobj_t a,
                const buzzobj_t b) {
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
   if(a->o.type == BUZZTYPE_STRING && b->o.type == BUZZTYPE_STRING) {
      return strcmp(a->s.value.str, b->s.value.str);
   }
   if(a->o.type == BUZZTYPE_TABLE && b->o.type == BUZZTYPE_TABLE) {
      if((uintptr_t)(a->t.value) < (uintptr_t)(b->t.value)) return -1;
      if((uintptr_t)(a->t.value) > (uintptr_t)(b->t.value)) return 1;
      return 0;
   }
   // TODO better error management
   fprintf(stderr, "[TODO] %s:%d: error for comparison between Buzz objects of types %d and %d\n", __FILE__, __LINE__, a->o.type, b->o.type);
   exit(1);
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
      case BUZZTYPE_ARRAY: {
         buzzmsg_serialize_u32(buf, buzzdarray_size(data->a.value));
         buzzdarray_foreach(data->a.value, buzzobj_serialize_arrayelem, buf);
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
         uint32_t size;
         p = buzzmsg_deserialize_u32(&size, buf, p);
         if(p < 0) return -1;
         for(uint32_t i = 0; i < size; ++i) {
            buzzobj_t* k;
            buzzobj_t* v;
            buzzobj_deserialize(k, buf, p, vm);
            if(p < 0) return -1;
            buzzobj_deserialize(v, buf, p, vm);
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
