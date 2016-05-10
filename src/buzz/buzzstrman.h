#ifndef BUZZSTRMAN_H
#define BUZZSTRMAN_H

#include <buzz/buzzdict.h>

#ifdef __cplusplus
extern "C" {
#endif

   struct buzzstrman_s {
      buzzdict_t str2id;  /* string -> id data */
      buzzdict_t id2str;  /* id -> string data */
      uint16_t maxsid;    /* maximum string id ever assigned */
      void* gcdata;       /* pointer to data for garbage collection */
   };
   typedef struct buzzstrman_s* buzzstrman_t;

   /**
    * Creates a new string manager.
    * @return A new string manager.
    */
   extern buzzstrman_t buzzstrman_new();

   /**
    * Disposes of a string manager.
    * @param sm The string manager.
    */
   extern void buzzstrman_destroy(buzzstrman_t* sm);

   /**
    * Registers a string into the string manager.
    * The string is cloned internally.
    * If a string has already been registered, its index is
    * returned. Only one copy of each string is kept.
    * If a previously unprotected string is re-registered as
    * protected, the protected flag is set.
    * @param sm The string manager.
    * @param str The string.
    * @param protect Whether the string is protected (!= 0) or not (== 0).
    * @return The id associated to the given string.
    */
   extern uint16_t buzzstrman_register(buzzstrman_t sm,
                                       const char* str,
                                       int protect);

   /*
    * Get the string corresponding to the given string id.
    * @param sm The string manager.
    * @param sid The id associated to the wanted string.
    * @return The string, or NULL if nothing is found.
    */
   extern const char* buzzstrman_get(buzzstrman_t sm,
                                     uint16_t sid);

   /*
    * Clears the marks for garbage collection.
    * @param sm The string manager.
    */
   extern void buzzstrman_gc_clear(buzzstrman_t sm);

   /*
    * Marks a string to be spared from garbage collection.
    * @param sm The string manager.
    * @param sid The id associated to the string.
    */
   extern void buzzstrman_gc_mark(buzzstrman_t sm,
                                  uint16_t sid);
   
   /*
    * Performs garbage collection on the unmarked strings.
    * The strings whose id is lower than the protected id are exempt
    * from garbage collection.
    * @param sm The string manager.
    */
   extern void buzzstrman_gc_prune(buzzstrman_t sm);

   /*
    * Prints the string manager data.
    * @param sm The string manager.
    */
   extern void buzzstrman_print(buzzstrman_t sm);

#ifdef __cplusplus
}
#endif

#endif
