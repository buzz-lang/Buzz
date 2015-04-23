#ifndef BUZZTYPE_H
#define BUZZTYPE_H

#include <buzzdict.h>
#include <buzzmsg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Variable types in Buzz
    */
   typedef enum {
      BUZZTYPE_INT = 0,  // 32 bit signed integer
      BUZZTYPE_FLOAT,    // 32 bit float value
      BUZZTYPE_STRING,   // string
      BUZZTYPE_TABLE,    // table
      BUZZTYPE_SWARM     // swarm
   } buzztype_e;

   /*
    * Integer
    */
   typedef struct {
      buzztype_e type;
      int32_t value;
   } buzzint_t;

   /*
    * Floating-point
    */
   typedef struct {
      buzztype_e type;
      float value;
   } buzzfloat_t;

   /*
    * String
    */
   typedef struct {
      buzztype_e type;
      char* value;
   } buzzstring_t;

   /*
    * Table
    */
   typedef struct {
      buzztype_e type;
      buzzdict_t value;
   } buzztable_t;

   /*
    * Swarm
    */
   typedef struct {
      buzztype_e type;
      buzzdict_t value;
   } buzzswarm_t;

   /*
    * A handle for a variable
    */
   typedef union {
      buzztype_e type; // variable type
      buzzint_t    i;  // as integer
      buzzfloat_t  f;  // as floating-point
      buzzstring_t s;  // as string
      buzztable_t  t;  // as table
      buzzswarm_t  g;  // as swarm (group)
   } buzzvar_t;

   /*
    * Serializes a Buzz variable.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzvar_serialize(buzzdarray_t buf,
                                 buzzvar_t data);

   /*
    * Deserializes a Buzz variable.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzvar_deserialize(buzzvar_t* data,
                                      buzzdarray_t buf,
                                      uint32_t pos);
   
#ifdef __cplusplus
}
#endif

#endif
