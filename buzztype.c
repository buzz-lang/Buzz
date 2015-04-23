#include "buzztype.h"
#include <stdio.h>

/****************************************/
/****************************************/

void buzzvar_serialize(buzzdarray_t buf,
                       buzzvar_t data) {
   switch(data.type) {
      case BUZZTYPE_INT: {
         buzzmsg_serialize_u32(buf, data.i.type);
         buzzmsg_serialize_u32(buf, data.i.value);
         break;
      }
      case BUZZTYPE_FLOAT: {
         buzzmsg_serialize_u32(buf, data.f.type);
         buzzmsg_serialize_float(buf, data.f.value);
         break;
      }
      case BUZZTYPE_STRING: {
         buzzmsg_serialize_u32(buf, data.s.type);
         buzzmsg_serialize_string(buf, data.s.value);
         break;
      }
      default:
         fprintf(stderr, "TODO: %s %u\n", __FILE__, __LINE__);
   }
}

/****************************************/
/****************************************/

int64_t buzzvar_deserialize(buzzvar_t* data,
                            buzzdarray_t buf,
                            uint32_t pos) {
   switch(data->type) {
      case BUZZTYPE_INT: {
         int64_t p = pos;
         buzzmsg_deserialize_u32(&data->i.type, buf, p);
         if(p < 0) return -1;
         buzzmsg_deserialize_u32((uint32_t*)(&data->i.value), buf, p);
         if(p < 0) return -1;
         return p;
         break;
      }
      case BUZZTYPE_FLOAT: {
         int64_t p = pos;
         buzzmsg_deserialize_u32(&data->f.type, buf, p);
         if(p < 0) return -1;
         buzzmsg_deserialize_float(&data->f.value, buf, p);
         if(p < 0) return -1;
         return p;
         break;
      }
      case BUZZTYPE_STRING: {
         int64_t p = pos;
         buzzmsg_deserialize_u32(&data->s.type, buf, p);
         if(p < 0) return -1;
         buzzmsg_deserialize_string(&data->s.value, buf, p);
         if(p < 0) return -1;
         return p;
         break;
      }
      default:
         fprintf(stderr, "TODO: %s %u\n", __FILE__, __LINE__);
         return -1;
   }
}

/****************************************/
/****************************************/
