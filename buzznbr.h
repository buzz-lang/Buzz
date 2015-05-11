#ifndef BUZZNBR_H
#define BUZZNBR_H

#include <buzzdict.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data about a neighbor
    */
   struct buzznbr_data_s {
      /* List of swarms this neighbor belongs to */
      buzzdarray_t swarms;
      /* Age of this information */
      uint8_t age;
   };
   typedef struct buzznbr_data_s* buzznbr_data_t;

   /*
    * The data structure to store neighbor data
    */
   typedef buzzdict_t buzznbr_t;

   /*
    * Creates a new neighbor data structure.
    */
   buzznbr_new();

   /*
    * Destroys a neighbor data structure.
    */
   buzznbr_destroy();

   /*
    * Updates the neighbor data structure.
    * It does two things:
    *
    * 1. if enough time passed since the last information broadcast,
    *    it broadcasts the 
    */
   buzznbr_update();

#ifdef __cplusplus
}
#endif

#endif

