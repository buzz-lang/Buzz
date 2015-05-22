#include <buzzinmsg.h>
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

void buzzinmsg_queue_append(buzzinmsg_queue_t msgq,
                            buzzmsg_payload_t payload) {
   /*
    * Find the position in the queue where the message must be queued
    */
   uint32_t pos = 0;
   /* Skip the messages with higher probabilities and
    * queue after all messages with equal priority */
   while(pos < buzzinmsg_queue_size(msgq) &&
         buzzmsg_payload_get(payload, 0) >=
         buzzmsg_payload_get(buzzinmsg_queue_get(msgq, pos), 0))
      ++pos;
   /*
    * Add the message
    */
   buzzmsg_payload_t* m = buzzdarray_makeslot(msgq, pos);
   *m = payload;
   /* Limit the queue size to MAX_QUEUE messages */
   while(buzzinmsg_queue_size(msgq) > MAX_QUEUE)
      buzzdarray_pop(msgq);
}

/****************************************/
/****************************************/

buzzmsg_payload_t buzzinmsg_queue_extract(buzzinmsg_queue_t msgq) {
   if(buzzinmsg_queue_isempty(msgq)) return NULL;
   buzzmsg_payload_t m = buzzdarray_get(msgq, 0, buzzmsg_payload_t);
   buzzdarray_remove(msgq, 0);
   return m;
}

/****************************************/
/****************************************/
