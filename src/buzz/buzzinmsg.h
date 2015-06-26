#ifndef BUZZINMSG_H
#define BUZZINMSG_H

#include <buzzdarray.h>
#include <buzzmsg.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data of a Buzz message queue.
    */
   typedef buzzdarray_t buzzinmsg_queue_t;

   /*
    * Appends a message to the queue.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param msgq The message queue.
    * @param payload The message payload.
    */
   extern void buzzinmsg_queue_append(buzzinmsg_queue_t msgq,
                                      buzzmsg_payload_t payload);

   /*
    * Extracts a message from the queue.
    * You are in charge of freeing both the message data and the payload.
    * @param msgq The message queue.
    * @return The message data or NULL.
    */
   extern buzzmsg_payload_t buzzinmsg_queue_extract(buzzinmsg_queue_t msgq);

#ifdef __cplusplus
}
#endif

/*
 * Create a new message queue.
 * @param cap The initial capacity of the queue. Must be >0.
 */
#define buzzinmsg_queue_new(cap) buzzdarray_new(cap, sizeof(buzzmsg_payload_t), NULL)

/*
 * Destroys a message queue.
 * @param msgq The message queue.
 */
#define buzzinmsg_queue_destroy(msgq) buzzdarray_destroy(msgq)

/*
 * Returns the size of a message queue.
 * @param msgq The message queue.
 * @return The size of a message queue.
 */
#define buzzinmsg_queue_size(msgq) buzzdarray_size(msgq)

/*
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define buzzinmsg_queue_isempty(msgq) buzzdarray_isempty(msgq)

/*
 * Returns the message at the given position in the queue.
 * @param msg The message queue.
 * @param pos The position.
 * @return The message at the given position.
 */
#define buzzinmsg_queue_get(msg, pos) buzzdarray_get(msg, pos, buzzmsg_payload_t)

#endif
