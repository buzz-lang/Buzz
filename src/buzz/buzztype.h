#ifndef BUZZTYPE_H
#define BUZZTYPE_H

#include <buzz/buzzdict.h>
#include <buzz/buzzmsg.h>
#include <stdint.h>

/*
 * Object types in Buzz
 */
#define BUZZTYPE_NIL      0
#define BUZZTYPE_INT      1
#define BUZZTYPE_FLOAT    2
#define BUZZTYPE_STRING   3
#define BUZZTYPE_TABLE    4
#define BUZZTYPE_CLOSURE  5
#define BUZZTYPE_USERDATA 6

#ifdef __cplusplus
extern "C" {
#endif

   extern const char *buzztype_desc[];

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
      struct {
         uint16_t sid;    // The string id
         const char* str; // The actual string
      } value;
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
    * Closure
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
      struct {
         int32_t ref;         // jump address or function id
         buzzdarray_t actrec; // activation record
         uint8_t isnative;    // 1 for native closure, 0 for c closure
      } value;
   } buzzclosure_t;
   
   /*
    * User data
    */
   typedef struct {
      uint16_t type;
      uint16_t marker;
      void*    value;
   } buzzuserdata_t;

   /*
    * A handle for a object
    */
   union buzzobj_u {
      struct {
         uint16_t type;    // object type
         uint16_t marker;  // marker for garbage collection
      }              o;    // as a generic object
      buzznil_t      n;    // as nil
      buzzint_t      i;    // as integer
      buzzfloat_t    f;    // as floating-point
      buzzstring_t   s;    // as string
      buzztable_t    t;    // as table
      buzzclosure_t  c;    // as closure
      buzzuserdata_t u;    // as user data
   };
   typedef union buzzobj_u* buzzobj_t;

   /*
    * Forward declaration of the Buzz VM.
    */
   struct buzzvm_s;

   /*
    * Create a Buzz object.
    * @param type The type of the Buzz object.
    * @return The created object.
    */
   extern buzzobj_t buzzobj_new(uint16_t type);

   /*
    * Destroys a Buzz object.
    * @param o The object to destroy.
    */
   extern void buzzobj_destroy(buzzobj_t* o);

   /*
    * Returns the hash of the passed Buzz object.
    * @param o The Buzz object to hash.
    * @return The calculated hash.
    */
   extern uint32_t buzzobj_hash(const buzzobj_t o);

   /*
    * Returns 1 if two Buzz objects are equal, 0 otherwise.
    * To be equal, two objects must have the same type and equal value.
    * For numeric types, value equality is as expected; for closures,
    * equality means pointing to the same code; for tables, equality
    * means having the same reference (no deep check).
    * @param a The first object.
    * @param b The second object.
    * @return 1 if two Buzz objects are equal, 0 otherwise.
    */
   extern int buzzobj_eq(const buzzobj_t a,
                         const buzzobj_t b);

   /*
    * Compares two Buzz objects.
    * Returns:
    * -1 if a  < b
    *  1 if a  > b
    *  0 if a == b
    *  2 if a == nil or b == nil but not both
    * To be comparable, two objects must have the same type and equal value.
    * For numeric types, value comparison is as expected; for closures and
    * tables, equality is undefined and an error is issued.
    * @param a The first object.
    * @param b The second object.
    * @return -1,0,1,2 depending on types and values
    */
   extern int buzzobj_cmp(const buzzobj_t a,
                          const buzzobj_t b);

   /*
    * C-closure to return the type of an object.
    * @param vm The VM data.
    */
   extern int buzzobj_type(struct buzzvm_s* vm);

   /*
    * C-closure to convert a float to int.
    */
   extern int buzzobj_int(struct buzzvm_s* vm);

   /*
    * C-closure to convert an int to float.
    */
   extern int buzzobj_tofloat(struct buzzvm_s* vm);

   /*
    * C-closure to clone a Buzz object.
    * @param vm The VM data.
    */
   extern int buzzobj_clone(struct buzzvm_s* vm);

   /*
    * C-closure to return the size of a table.
    * @param vm The VM data.
    */
   extern int buzzobj_size(struct buzzvm_s* vm);

   /*
    * C-closure to loop through the elements of a table.
    * @param vm The VM data.
    */
   extern int buzzobj_foreach(struct buzzvm_s* vm);

   /*
    * C-closure to apply a function to all the elements of a table.
    * @param vm The VM data.
    */
   extern int buzzobj_map(struct buzzvm_s* vm);

   /*
    * C-closure to loop through the elements of a table and return an aggregated value.
    * @param vm The VM data.
    */
   extern int buzzobj_reduce(struct buzzvm_s* vm);

   /*
    * C-closure to loop through the elements of a table and return a new table with the elements that pass the filter.
    * @param vm The VM data.
    */
   extern int buzzobj_filter(struct buzzvm_s* vm);

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
    * @param vm The Buzz VM data.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzobj_deserialize(buzzobj_t* data,
                                      buzzdarray_t buf,
                                      uint32_t pos,
                                      struct buzzvm_s* vm);

   /*
    * Registers basic object methods into the virtual machine.
    * @param vm The Buzz VM data.
    * @return The new state of the VM.
    */
   extern int buzzobj_register(struct buzzvm_s* vm);
   
#ifdef __cplusplus
}
#endif

#define buzzobj_isnil(OBJ) ((OBJ)->o.type == BUZZTYPE_NIL)
#define buzzobj_isint(OBJ) ((OBJ)->o.type == BUZZTYPE_INT)
#define buzzobj_isfloat(OBJ) ((OBJ)->o.type == BUZZTYPE_FLOAT)
#define buzzobj_isstring(OBJ) ((OBJ)->o.type == BUZZTYPE_STRING)
#define buzzobj_istable(OBJ) ((OBJ)->o.type == BUZZTYPE_TABLE)
#define buzzobj_isclosure(OBJ) ((OBJ)->o.type == BUZZTYPE_CLOSURE)
#define buzzobj_isuserdata(OBJ) ((OBJ)->o.type == BUZZTYPE_USERDATA)

#define buzzobj_getint(OBJ) ((OBJ)->i.value)
#define buzzobj_getfloat(OBJ) ((OBJ)->f.value)
#define buzzobj_getstring(OBJ) ((OBJ)->s.value.str)
#define buzzobj_getuserdata(OBJ) ((OBJ)->y.value)

#endif
