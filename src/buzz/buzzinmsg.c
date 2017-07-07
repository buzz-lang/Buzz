#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

/****************************************/
/****************************************/

void buzzvm_inmsg_queue_destroy_entry(const void* key, void* data, void* param) {
   free((void*)key);
   buzzdarray_destroy((buzzdarray_t*)data);
   free(data);
}

/****************************************/
/****************************************/

void buzzinmsg_queue_append(buzzvm_t vm,
                            uint16_t rid,
                            buzzmsg_payload_t payload) {
   /* Check if id is already present */
   if(!buzzdict_exists(vm->inmsgs, &rid)) {
      /* Not present, create a new queue */
      buzzdarray_t q = buzzdarray_new(1, sizeof(buzzmsg_payload_t), NULL);
      /* Add it to the dict */
      buzzdict_set(vm->inmsgs, &rid, &q);
   }
   /* Get queue corresponding to given robot id */
   buzzdarray_t* q = (buzzdarray_t*)buzzdict_rawget(vm->inmsgs, &rid);
   /* Append payload to queue */
   buzzdarray_push(*q, &payload);
}

/****************************************/
/****************************************/

struct buzzdict_entry_s {
   void* key;
   void* data;
};

int buzzinmsg_queue_extract(buzzvm_t vm,
                            uint16_t* rid,
                            buzzmsg_payload_t* payload) {
   /* Nothing to do if queue is empty */
   if(buzzinmsg_queue_isempty(vm->inmsgs)) return 0;
   /* Look for (id,queue) in first non-empty bucket in dict */
   buzzdarray_t q = NULL;
   for(uint32_t i = 0; i < vm->inmsgs->num_buckets; ++i) {
      if(vm->inmsgs->buckets[i]) {
         const struct buzzdict_entry_s* e =
            &buzzdarray_get(vm->inmsgs->buckets[i],
                            0,
                            struct buzzdict_entry_s);
         *rid = *(uint16_t*)(e->key);
         q = *(buzzdarray_t*)(e->data);
         break;
      }
   }
   /* Extract payload from array */
   *payload = buzzdarray_last(q, buzzmsg_payload_t);
   buzzdarray_pop(q);
   /* If array is empty, remove (id,array entry) from dict */
   if(buzzdarray_isempty(q))
      buzzdict_remove(vm->inmsgs, rid);
   /* All done */
   return 1;
}

/****************************************/
/****************************************/
