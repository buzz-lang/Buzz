#ifndef BUZZOUTMSG_H
#define BUZZOUTMSG_H

#include <buzz/buzzdarray.h>
#include <buzz/buzzmsg.h>
#include <buzz/buzzvstig.h>

struct buzzvm_s;

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data of a Buzz message queue.
    */
   struct buzzoutmsg_queue_s {
      /* One queue for each message type */
      buzzdarray_t queues[BUZZMSG_TYPE_COUNT];
      /* Vstig message dict for fast duplicate management */
      buzzdict_t vstig;
   };
   typedef struct buzzoutmsg_queue_s* buzzoutmsg_queue_t;

   /*
    * Create a new message queue.
    * @return A new message queue.
    */
   extern buzzoutmsg_queue_t buzzoutmsg_queue_new();

   /*
    * Destroys a message queue.
    * @param msgq The message queue.
    */
   extern void buzzoutmsg_queue_destroy(buzzoutmsg_queue_t* msgq);

   /*
    * Returns the size of a message queue.
    * @param vm The Buzz VM.
    * @return The size of the message queue.
    */
   extern uint32_t buzzoutmsg_queue_size(struct buzzvm_s* vm);

   /*
    * Appends a new broadcast message.
    * @param vm The Buzz VM.
    * @param topic The topic on which to send (a string object)
    * @param value The value.
    */
   extern void buzzoutmsg_queue_append_broadcast(struct buzzvm_s* vm,
                                                 buzzobj_t topic,
                                                 buzzobj_t value);

   /*
    * Appends a new swarm list message.
    * @param vm The Buzz VM.
    * @param ids A list of swarm ids in which the robot is a member.
    */
   extern void buzzoutmsg_queue_append_swarm_list(struct buzzvm_s* vm,
                                                  const buzzdict_t ids);
   
   /*
    * Appends a new swarm join/leave message.
    * @param vm The Buzz VM.
    * @param type Either BUZZMSG_SWARM_JOIN or BUZZMSG_SWARM_LEAVE
    * @param id The swarm id whose membership has changed
    */
   extern void buzzoutmsg_queue_append_swarm_joinleave(struct buzzvm_s* vm,
                                                       int type,
                                                       uint16_t id);

   /*
    * Appends a new virtual stigmergy message.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param vm The Buzz VM.
    * @param type The message type (BUZZMSG_VSTIG_PUT or BUZZMSG_VSTIG_QUERY)
    * @param id The id of the virtual stigmergy.
    * @param key The key.
    * @param data The data of the entry.
    */
   extern void buzzoutmsg_queue_append_vstig(struct buzzvm_s* vm,
                                             int type,
                                             uint16_t id,
                                             const buzzobj_t key,
                                             const buzzvstig_elem_t data);

   /*
    * Returns the first serialized message in the queue.
    * You are in charge of freeing both the message data and the payload.
    * @param vm The Buzz VM.
    * @return The message data or NULL.
    * @see buzzoutmsg_queue_first
    */
   extern buzzmsg_payload_t buzzoutmsg_queue_first(struct buzzvm_s* vm);

   /*
    * Removes the first message from the queue.
    * @param vm The Buzz VM.
    * @see buzzoutmsg_queue_first
    */
   extern void buzzoutmsg_queue_next(struct buzzvm_s* vm);

   /*
    * Performs garbage collection.
    * You should never call this function. It is called by
    * buzzheap_gc() when necessary.
    * @param vm The Buzz VM.
    */
   extern void buzzoutmsg_gc(struct buzzvm_s* vm);

#ifdef __cplusplus
}
#endif

/*
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define buzzoutmsg_queue_isempty(vm) (buzzoutmsg_queue_size(vm) == 0)

#endif
