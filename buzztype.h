#ifndef BUZZTYPE_H
#define BUZZTYPE_H

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
   } buzz_type_e;

   /*
    * Integer
    */
   typedef struct {
      buzz_type_e type;
      int32_t value;
   } buzz_int_t;

   /*
    * Floating-point
    */
   typedef struct {
      buzz_type_e type;
      float value;
   } buzz_float_t;

   /*
    * String
    */
   typedef struct {
      buzz_type_e type;
      char* value;
   } buzz_string_t;

   /*
    * Table
    */
   typedef struct {
      buzz_type_e type;
      buzzdict_t value;
   } buzz_table_t;

   /*
    * Swarm
    */
   typedef struct {
      buzz_type_e type;
      buzzdict_t value;
   } buzz_swarm_t;

   /*
    * A handle for a variable
    */
   typedef union {
      struct {
         buzz_type_e type;
         void* value;
      }             generic; // generic type info
      buzz_int_t    i;       // as integer
      buzz_float_t  f;       // as floating-point
      buzz_string_t s;       // as string
      buzz_table_t  t;       // as table
      buzz_swarm_t  g;       // as swarm (group)
   } buzzvm_var_t;

#ifdef __cplusplus
}
#endif

#endif
