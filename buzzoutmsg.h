#ifndef BUZZOUTMSG_H
#define BUZZOUTMSG_H

#include <buzzdarray.h>
#include <buzzmsg.h>
#include <buzzvstig.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data of a Buzz message queue.
    */
   struct buzzoutmsg_queue_s {
      /* One queue for each message type */
      buzzdarray_t queues[4];
      /* Vstig message dict for fast duplicate management */
      buzzdict_t vstig;
   };
   typedef struct buzzoutmsg_queue_s* buzzoutmsg_queue_t;

   /*
    * Create a new message queue.
    * @return A new message queue.
    */
   buzzoutmsg_queue_t buzzoutmsg_queue_new();

   /*
    * Destroys a message queue.
    * @param msgq The message queue.
    */
   void buzzoutmsg_queue_destroy(buzzoutmsg_queue_t* msgq);

   /*
    * Returns the size of a message queue.
    * @param msgq The message queue.
    * @return The size of a message queue.
    */
   uint32_t buzzoutmsg_queue_size(buzzoutmsg_queue_t msgq);

   /*
    * Appends a new shout message.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param msgq The message queue.
    * @param payload The message payload.
    */
   extern void buzzoutmsg_queue_append_shout(buzzoutmsg_queue_t msgq,
                                             buzzmsg_payload_t payload);

   /*
    * Appends a new swarm message.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param msgq The message queue.
    * @param payload The message payload.
    */
   extern void buzzoutmsg_queue_append_swarm(buzzoutmsg_queue_t msgq,
                                             buzzmsg_payload_t payload);

   /*
    * Appends a new virtual stigmergy message.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param msgq The message queue.
    * @param type The message type (BUZZMSG_VSTIG_PUT or BUZZMSG_VSTIG_QUERY)
    * @param id The id of the virtual stigmergy.
    * @param key The key.
    * @param data The data of the entry.
    */
   extern void buzzoutmsg_queue_append_vstig(buzzoutmsg_queue_t msgq,
                                             int type,
                                             uint16_t id,
                                             const buzzobj_t key,
                                             const buzzvstig_elem_t data);

   /*
    * Returns the first serialized message in the queue.
    * You are in charge of freeing both the message data and the payload.
    * @param msgq The message queue.
    * @return The message data or NULL.
    * @see buzzoutmsg_queue_first
    */
   extern buzzmsg_payload_t buzzoutmsg_queue_first(buzzoutmsg_queue_t msgq);

   /*
    * Removes the first message from the queue.
    * @param msgq The message queue.
    * @see buzzoutmsg_queue_first
    */
   extern void buzzoutmsg_queue_next(buzzoutmsg_queue_t msgq);

#ifdef __cplusplus
}
#endif

/*
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define buzzoutmsg_queue_isempty(msgq) (buzzoutmsg_queue_size(msgq) == 0)

#endif
