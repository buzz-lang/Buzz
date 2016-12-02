#include "buzzvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

/****************************************/
/****************************************/

static int32_t MAX_QUEUE = 100;

/****************************************/
/****************************************/

void buzzinmsg_queue_append(buzzvm_t vm,
                            buzzmsg_payload_t payload) {
   /*
    * Find the position in the queue where the message must be queued
    */
   uint32_t pos = 0;
   /* Skip the messages with higher probabilities and
    * queue after all messages with equal priority */
   while(pos < buzzinmsg_queue_size(vm->inmsgs) &&
         buzzmsg_payload_get(payload, 0) >=
         buzzmsg_payload_get(buzzinmsg_queue_get(vm->inmsgs, pos), 0))
      ++pos;
   /*
    * Add the message
    */
   buzzmsg_payload_t* m = buzzdarray_makeslot(vm->inmsgs, pos);
   *m = payload;
   /* Limit the queue size to MAX_QUEUE messages */
   while(buzzinmsg_queue_size(vm->inmsgs) > MAX_QUEUE)
      buzzdarray_pop(vm->inmsgs);
}

/****************************************/
/****************************************/

buzzmsg_payload_t buzzinmsg_queue_extract(buzzvm_t vm) {
   if(buzzinmsg_queue_isempty(vm->inmsgs)) return NULL;
   buzzmsg_payload_t m = buzzdarray_get(vm->inmsgs, 0, buzzmsg_payload_t);
   buzzdarray_remove(vm->inmsgs, 0);
   return m;
}

/****************************************/
/****************************************/
