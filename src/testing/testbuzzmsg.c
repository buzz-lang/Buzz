#include <buzzinmsg.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
   buzzmsg_payload_t buf = buzzmsg_payload_new(10);
   fprintf(stderr, "Serializing 10\n");
   buzzmsg_serialize_u32(buf, 10);
   fprintf(stderr, "Serializing -2.5\n");
   buzzmsg_serialize_float(buf, -2.5);
   fprintf(stderr, "Serializing 'sti gran cazzi'\n");
   buzzmsg_serialize_string(buf, "sti gran cazzi");
   fprintf(stderr, "Serializing -30\n");
   buzzmsg_serialize_u32(buf, -30);
   fprintf(stderr, "Serializing 3.14\n");
   buzzmsg_serialize_float(buf, -3.14);

   buzzinmsg_queue_t q = buzzinmsg_queue_new(1);
   buzzinmsg_queue_append(q, buf);
   
   buzzmsg_payload_t buf2 = buzzinmsg_queue_extract(q);
   int64_t pos = 0;
   int32_t x;
   float y;
   char* s;
   fprintf(stderr, "Deserializing 10:");
   pos = buzzmsg_deserialize_u32((uint32_t*)(&x), buf2, pos);
   fprintf(stderr, "%d (%lld)\n", x, pos);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing -2.5:");
   pos = buzzmsg_deserialize_float(&y, buf2, pos);
   fprintf(stderr, "%f (%lld)\n", y, pos);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing 'sti gran cazzi':");
   pos = buzzmsg_deserialize_string(&s, buf2, pos);
   fprintf(stderr, "%s (%lld)\n", s, pos);
   free(s);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing -30:");
   pos = buzzmsg_deserialize_u32((uint32_t*)(&x), buf2, pos);
   fprintf(stderr, "%d (%lld)\n", x, pos);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing 3.14:");
   pos = buzzmsg_deserialize_float(&y, buf2, pos);
   fprintf(stderr, "%f (%lld)\n", y, pos);
   if(pos < 0) return 1;
   
   buzzmsg_payload_destroy(&buf2);
   buzzinmsg_queue_destroy(&q);
   return 0;
}
