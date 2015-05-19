#include "buzztype.h"
#include <stdio.h>
#include <stdlib.h>

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
      case BUZZTYPE_TABLE:   return (a->t.value == b->t.value);
      case BUZZTYPE_ARRAY:   return (a->a.value == b->a.value);
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
      if(a->s.value < b->s.value) return -1;
      if(a->s.value > b->s.value) return 1;
      return 0;
   }
   // TODO better error management
   fprintf(stderr, "[TODO] %s:%d: error for comparison between Buzz objects of types %d and %d\n", __FILE__, __LINE__, a->o.type, b->o.type);
   exit(1);
}

/****************************************/
/****************************************/

void buzzobj_serialize_darrayelem(uint32_t pos, void* data, void* params) {
   buzzobj_serialize((buzzdarray_t)params, *(buzzobj_t*)data);
}

void buzzobj_serialize(buzzdarray_t buf,
                       const buzzobj_t data) {
   switch(data->o.type) {
      case BUZZTYPE_NIL: {
         buzzmsg_serialize_u16(buf, data->n.type);
         break;
      }
      case BUZZTYPE_INT: {
         buzzmsg_serialize_u16(buf, data->i.type);
         buzzmsg_serialize_u32(buf, data->i.value);
         break;
      }
      case BUZZTYPE_FLOAT: {
         buzzmsg_serialize_u16(buf, data->f.type);
         buzzmsg_serialize_float(buf, data->f.value);
         break;
      }
      /* case BUZZTYPE_STRING: { */
      /*    buzzmsg_serialize_u16(buf, data->s.type); */
      /*    buzzmsg_serialize_string(buf, data->s.value); */
      /*    break; */
      /* } */
      case BUZZTYPE_ARRAY: {
         buzzmsg_serialize_u16(buf, data->a.type);
         buzzmsg_serialize_u32(buf, buzzdarray_size(data->a.value));
         buzzdarray_foreach(data->a.value, buzzobj_serialize_darrayelem, buf);
         break;
      }
      default:
         fprintf(stderr, "[TODO] %s %u\n", __FILE__, __LINE__);
   }
}

/****************************************/
/****************************************/

int64_t buzzobj_deserialize(buzzobj_t data,
                            buzzdarray_t buf,
                            uint32_t pos) {
   switch(data->o.type) {
      case BUZZTYPE_NIL: {
         int64_t p = pos;
         p = buzzmsg_deserialize_u16(&data->n.type, buf, p);
         return p;
      }
      case BUZZTYPE_INT: {
         int64_t p = pos;
         p = buzzmsg_deserialize_u16(&data->i.type, buf, p);
         if(p < 0) return -1;
         p = buzzmsg_deserialize_u32((uint32_t*)(&data->i.value), buf, p);
         return p;
      }
      case BUZZTYPE_FLOAT: {
         int64_t p = pos;
         p = buzzmsg_deserialize_u16(&data->f.type, buf, p);
         if(p < 0) return -1;
         p = buzzmsg_deserialize_float(&data->f.value, buf, p);
         return p;
      }
      /* case BUZZTYPE_STRING: { */
      /*    int64_t p = pos; */
      /*    p = buzzmsg_deserialize_u16(&data->s.type, buf, p); */
      /*    if(p < 0) return -1; */
      /*    p = buzzmsg_deserialize_string(&data->s.value, buf, p); */
      /*    return p; */
      /* } */
      default:
         fprintf(stderr, "TODO: %s %u\n", __FILE__, __LINE__);
         return -1;
   }
}

/****************************************/
/****************************************/
