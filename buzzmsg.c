#include <buzzmsg.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/****************************************/
/****************************************/

void buzzmsg_append(buzzmsg_t msgq,
                    buzzmsg_type_e type,
                    uint8_t* payload,
                    uint32_t size) {
   buzzmsg_data_t* m = buzzdarray_makeslot(msgq, buzzmsg_size(msgq));
   *m = malloc(sizeof(struct buzzmsg_data_s));
   (*m)->type = type;
   (*m)->size = size;
   (*m)->payload = (uint8_t*)malloc(size);
   memcpy((*m)->payload, payload, size);
}

/****************************************/
/****************************************/

buzzmsg_data_t buzzmsg_extract(buzzmsg_t msgq) {
   if(buzzmsg_isempty(msgq)) return NULL;
   buzzmsg_data_t m = *buzzdarray_get(msgq, 0, buzzmsg_data_t);
   buzzdarray_remove(msgq, 0);
   return m;
}

/****************************************/
/****************************************/

void buzzmsg_serialize_u32(buzzdarray_t buf,
                           uint32_t data) {
   uint32_t x = htonl(data);
   buzzdarray_push(buf, (uint8_t*)(&x));
   buzzdarray_push(buf, (uint8_t*)(&x)+1);
   buzzdarray_push(buf, (uint8_t*)(&x)+2);
   buzzdarray_push(buf, (uint8_t*)(&x)+3);
}

/****************************************/
/****************************************/

int64_t buzzmsg_deserialize_u32(uint32_t* data,
                                buzzdarray_t buf,
                                uint32_t pos) {
   /* Deserialize the key */
   if(pos + sizeof(uint32_t) >= buzzdarray_size(buf)) return -1;
   *data =
      (*buzzdarray_get(buf, pos,   uint8_t)     ) +
      (*buzzdarray_get(buf, pos+1, uint8_t) << 1) +
      (*buzzdarray_get(buf, pos+2, uint8_t) << 2) +
      (*buzzdarray_get(buf, pos+3, uint8_t) << 3);
   *data = ntohl(*data);
   return pos + sizeof(uint32_t);
}

/****************************************/
/****************************************/
