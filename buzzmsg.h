#ifndef BUZZMSG_H
#define BUZZMSG_H

#include <buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data of Buzz message queue.
    */
   typedef buzzdarray_t buzzmsg_t;

   /*
    * Buzz message type.
    */
   typedef enum {
      BUZZMSG_VSTIG = 0, // Virtual stigmergy-related
      BUZZMSG_USER       // gossip() command in Buzz
   } buzzmsg_type_e;

   /*
    * Buzz message data.
    */
   struct buzzmsg_data_s {
      buzzmsg_type_e type;
      uint8_t* payload;
      uint32_t size;
   };
   typedef struct buzzmsg_data_s* buzzmsg_data_t;

   /*
    * Appends a message to the queue.
    * The payload is copied inside the structure. You can safely
    * free() the passed payload buffer.
    * @param msgq The message queue.
    * @param type The message type.
    * @param payload The message payload buffer.
    * @param size The message size.
    */
   extern void buzzmsg_append(buzzmsg_t msgq,
                              buzzmsg_type_e type,
                              uint8_t* payload,
                              uint32_t size);

   /*
    * Extracts a message from the queue.
    * @param msgq The message queue.
    * @return The message data or NULL. You are in charge of freeing the payload.
    */
   extern buzzmsg_data_t buzzmsg_extract(buzzmsg_t msgq);

   /*
    * Serializes a 32-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u32(buzzdarray_t buf,
                                     uint32_t data);

   /*
    * Deserializes a 32-bit unsigned integer.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzmsg_deserialize_u32(uint32_t* data,
                                          buzzdarray_t buf,
                                          uint32_t pos);

#ifdef __cplusplus
}
#endif

/*
 * Create a new message queue.
 * @param cap The initial capacity of the queue. Must be >0.
 */
#define buzzmsg_new(cap) buzzdarray_new(cap, sizeof(buzzmsg_data_t), NULL)

/*
 * Destroys a message queue.
 * @param msgq The message queue.
 */
#define buzzmsg_destroy(msgq) buzzdarray_destroy(msgq)

/*
 * Returns the size of a message queue.
 * @param da The message queue.
 * @return The size of a message queue.
 */
#define buzzmsg_size(msgq) buzzdarray_size(msgq)

/*
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define buzzmsg_isempty(msgq) buzzdarray_isempty(msgq)

#endif
