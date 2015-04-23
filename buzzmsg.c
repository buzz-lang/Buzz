#include <buzzmsg.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

/****************************************/
/****************************************/

static int32_t MAX_MANTISSA = 2147483646; // 2 << 31 - 2;

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
   if(pos + sizeof(uint32_t) > buzzdarray_size(buf)) return -1;
   *data =
      (*buzzdarray_get(buf, pos,   uint8_t)     ) +
      (*buzzdarray_get(buf, pos+1, uint8_t) << 8) +
      (*buzzdarray_get(buf, pos+2, uint8_t) << 16) +
      (*buzzdarray_get(buf, pos+3, uint8_t) << 24);
   *data = ntohl(*data);
   return pos + sizeof(uint32_t);
}

/****************************************/
/****************************************/

void buzzmsg_serialize_float(buzzdarray_t buf,
                             float data) {
   /* The mantissa */
   int32_t mant;
   /* The exponent */
   int32_t exp;
   /*
    * Split the data into a normalized fraction (nf) and an integral power of 2
    * when data == 0, nf = 0 and exp = 0
    * otherwise data = nf * 2^exp and nf is in +-[0.5,1)
    */
   float nf = frexpf(data, &exp);
   /* Take the magnitude of nf and translate it by -0.5 */
   nf = fabsf(nf) - 0.5f;
   /* Now we can safely test for zero */
   if(nf < 0.0f) {
      /* The mantissa is zero */
      mant = 0;
   }
   else {
      /* Let's calculate the mantissa
       *
       * Idea:
       * nf now is in [0,0.5), we want to map it to the entire range of uint32_t
       *
       * 1. multiply nf by 2 so it's in [0,1)
       * 2. multiply the result by the maximum value for the mantissa
       * 3. problem: 0 maps to 0 and it would indistinguishable from nf == 0 above
       *    solution: simply sum 1 to the mantissa 
       */
      mant = (int32_t)(nf * 2.0f * MAX_MANTISSA) + 1;
      /* Now take care of the sign */
      if(data < 0.0f) mant = -mant;
   }
   /* Serialize the data */
   buzzmsg_serialize_u32(buf, mant);
   buzzmsg_serialize_u32(buf, exp);
}

/****************************************/
/****************************************/

int64_t buzzmsg_deserialize_float(float* data,
                                  buzzdarray_t buf,
                                  uint32_t pos) {
   /* Make sure enough bytes are left to read */
   if(pos + 2*sizeof(uint32_t) > buzzdarray_size(buf)) return -1;
   /* Read the mantissa and the exponent */
   int32_t mant;
   int32_t exp;
   pos = buzzmsg_deserialize_u32((uint32_t*)(&mant), buf, pos);
   pos = buzzmsg_deserialize_u32((uint32_t*)(&exp), buf, pos);
   /* A zero mantissa corresponds to a zero float */
   if(mant == 0) {
      *data = 0;
   }
   else {
      /* We must calculate the float
       *
       * Idea: use the function ldexpf(), which is the opposite of frexpf()
       * For this, we need to transform the mantissa into a normalized
       * fraction in the range [0.5,1)
       *
       * 1. Take the absolute value of the mantissa, which is in [1, MAX_MANTISSA+1]
       * 2. Subtract 1, so it's in [0, MAX_MANTISSA]
       * 3. divide by MAX_MANTISSA, so it's in [0,1)
       * 4. divide by 2, so it's in [0,0.5)
       * 4. Add 0.5, so it's in [0.5,1)
       */
      *data = (float)(abs(mant) - 1) / (2.0f * MAX_MANTISSA) + 0.5f;
      /* Now use ldexpf() to calculate the absolute value */
      *data = ldexpf(*data, exp);
      /* Finally take care of the sign */
      if(mant < 0) *data = -*data;
   }
   return pos;
}

/****************************************/
/****************************************/

void buzzmsg_serialize_string(buzzdarray_t buf,
                              const char* data) {
   /* Get the length of the string */
   uint32_t len = strlen(data);
   /* Push that into the buffer */
   buzzmsg_serialize_u32(buf, len);
   /* Go through the characters and push them into the buffer */
   const char* c = data;
   while(*c) {
      buzzdarray_push(buf, (uint8_t*)(c));
      ++c;
   }
}

/****************************************/
/****************************************/

extern int64_t buzzmsg_deserialize_string(char** data,
                                          buzzdarray_t buf,
                                          uint32_t pos) {
   /* Make sure there are enough bytes to read the string length */
   if(pos + sizeof(uint32_t) > buzzdarray_size(buf)) return -1;
   /* Read the string length */
   uint32_t len;
   pos = buzzmsg_deserialize_u32(&len, buf, pos);
   /* Make sure there are enough bytes to read the string itself */   
   if(pos + len > buzzdarray_size(buf)) return -1;
   /* Create a buffer for the string */
   *data = (char*)malloc(len * sizeof(char) + 1);
   /* Read the string characters */
   memcpy(*data, (uint8_t*)buf->data + pos, len * sizeof(char));
   /* Set the termination character */
   *(*data + len) = 0;
   /* Return new position */
   return pos + len;
}

/****************************************/
/****************************************/
