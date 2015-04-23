#include <buzzmsg.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
   buzzdarray_t buf = buzzdarray_new(10, sizeof(uint8_t), 0);
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
   int64_t pos = 0;
   int32_t x;
   float y;
   char* s;
   fprintf(stderr, "Deserializing 10:");
   pos = buzzmsg_deserialize_u32((uint32_t*)(&x), buf, pos);
   fprintf(stderr, "%d (%lld)\n", x, pos);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing -2.5:");
   pos = buzzmsg_deserialize_float(&y, buf, pos);
   fprintf(stderr, "%f (%lld)\n", y, pos);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing 'sti gran cazzi':");
   pos = buzzmsg_deserialize_string(&s, buf, pos);
   fprintf(stderr, "%s (%lld)\n", s, pos);
   free(s);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing -30:");
   pos = buzzmsg_deserialize_u32((uint32_t*)(&x), buf, pos);
   fprintf(stderr, "%d (%lld)\n", x, pos);
   if(pos < 0) return 1;
   fprintf(stderr, "Deserializing 3.14:");
   pos = buzzmsg_deserialize_float(&y, buf, pos);
   fprintf(stderr, "%f (%lld)\n", y, pos);
   if(pos < 0) return 1;
   buzzdarray_destroy(&buf);
   return 0;
}
