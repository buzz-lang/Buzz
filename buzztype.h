#ifndef BUZZTYPE_H
#define BUZZTYPE_H

#include <buzzdict.h>
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
      struct {
         buzztype_e type;
         void* value;
      }             generic; // generic type info
      buzzint_t    i;        // as integer
      buzzfloat_t  f;        // as floating-point
      buzzstring_t s;        // as string
      buzztable_t  t;        // as table
      buzzswarm_t  g;        // as swarm (group)
   } buzzvar_t;

#ifdef __cplusplus
}
#endif

#endif
