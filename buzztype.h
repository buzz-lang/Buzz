#ifndef BUZZTYPE_H
#define BUZZTYPE_H

#include <buzzdict.h>
#include <buzzmsg.h>
#include <stdint.h>

/*
 * Object types in Buzz
 */
#define BUZZTYPE_NIL     0
#define BUZZTYPE_INT     1
#define BUZZTYPE_FLOAT   2
#define BUZZTYPE_STRING  3
#define BUZZTYPE_TABLE   4
#define BUZZTYPE_SWARM   5
#define BUZZTYPE_CLOSURE 6

/*
 * Info extraction from an object
 */
#define buzzobj_type(v)

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Nil
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
   } buzznil_t;

   /*
    * 32-bit signed integer
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
      int32_t  value;
   } buzzint_t;

   /*
    * 32-bit floating-point
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
      float    value;
   } buzzfloat_t;

   /*
    * String
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
      char*    value;
   } buzzstring_t;

   /*
    * Table
    */
   typedef struct {
      uint16_t   type;
      uint16_t   marker;
      buzzdict_t value;
   } buzztable_t;

   /*
    * Swarm
    */
   typedef struct {
      uint16_t   type;
      uint16_t   marker;
      buzzdict_t value;
   } buzzswarm_t;

   /*
    * Closure
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
      uint8_t* value;
   } buzzclosure_t;

   /*
    * A handle for a object
    */
   union buzzobj_u {
      struct {
         uint16_t   type;   // object type
         uint16_t    marker; // marker for garbage collection
      }             o;      // as a generic object
      buzznil_t     n;      // as nil
      buzzint_t     i;      // as integer
      buzzfloat_t   f;      // as floating-point
      buzzstring_t  s;      // as string
      buzztable_t   t;      // as table
      buzzswarm_t   g;      // as swarm (group)
      buzzclosure_t c;      // as closure
   };
   typedef union buzzobj_u* buzzobj_t;

   /*
    * Serializes a Buzz object.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzobj_serialize(buzzdarray_t buf,
                                 const buzzobj_t data);

   /*
    * Deserializes a Buzz object.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzobj_deserialize(buzzobj_t data,
                                      buzzdarray_t buf,
                                      uint32_t pos);
   
#ifdef __cplusplus
}
#endif

#endif
