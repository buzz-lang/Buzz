#ifndef BUZZINMSG_H
#define BUZZINMSG_H

#include <buzz/buzzdarray.h>
#include <buzz/buzzmsg.h>

struct buzzvm_s;

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data of a Buzz message queue.
    */
   typedef buzzdict_t buzzinmsg_queue_t;

   /*
    * Appends a message to the queue.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param vm The Buzz VM.
    * @param id The id of the robot who sent the message.
    * @param payload The message payload.
    */
   extern void buzzinmsg_queue_append(struct buzzvm_s* vm,
                                      uint16_t id,
                                      buzzmsg_payload_t payload);

   /*
    * Extracts a message from the queue.
    * You are in charge of freeing both the message data and the payload.
    * If the queue is empty, the values of *id and *payload are left untouched.
    * @param vm The Buzz VM.
    * @param id The id of the robot who sent the message.
    * @param payload The message payload.
    * @return 1 if the extraction was successful; 0 if no messages are left
    */
   extern int buzzinmsg_queue_extract(struct buzzvm_s* vm,
                                      uint16_t* id,
                                      buzzmsg_payload_t* payload);

   /**
    * Internally used to cleanup a queue entry.
    * @param key A pointer to the robot id (uint16_t)
    * @param data A pointer to buzzdarray_t
    * @param param Unused
    */
   extern void buzzvm_inmsg_queue_destroy_entry(const void* key,
                                                void* data,
                                                void* param);

#ifdef __cplusplus
}
#endif

/*
 * Create a new message queue.
 */
#define buzzinmsg_queue_new() buzzdict_new(20, sizeof(uint16_t), sizeof(buzzmsg_payload_t), buzzdict_uint16keyhash, buzzdict_uint16keycmp, buzzvm_inmsg_queue_destroy_entry)

/*
 * Destroys a message queue.
 * @param msgq The message queue.
 */
#define buzzinmsg_queue_destroy(msgq) buzzdict_destroy(msgq)

/*
 * Returns the size of a message queue.
 * @param msgq The message queue.
 * @return The size of a message queue.
 */
#define buzzinmsg_queue_size(msgq) buzzdict_size(msgq)

/*
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define buzzinmsg_queue_isempty(msgq) buzzdict_isempty(msgq)

/*
 * Returns the message at the given position in the queue.
 * @param msg The message queue.
 * @param pos The position.
 * @return The message at the given position.
 */
#define buzzinmsg_queue_get(msg, pos) buzzdict_get(msg, pos, buzzmsg_payload_t)

#endif
