#ifndef BUZZMSG_H
#define BUZZMSG_H

#include <buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Data of a Buzz message.
    */
   typedef buzzdarray_t buzzmsg_t;

   /*
    * Data of a Buzz message queue.
    */
   typedef buzzdarray_t buzzmsg_queue_t;

   /*
    * Buzz message type.
    */
   typedef enum {
      BUZZMSG_USER = 0,    // shout() command in Buzz
      BUZZMSG_VSTIG_PUT,   // Virtual stigmergy PUT
      BUZZMSG_VSTIG_QUERY, // Virtual stigmergy QUERY
   } buzzmsg_type_e;

   /*
    * Appends a message to the queue.
    * The ownership of the payload is assumed by the message queue. Make sure
    * the payload is in the heap.
    * @param msgq The message queue.
    * @param payload The message payload.
    */
   extern void buzzmsg_queue_append(buzzmsg_queue_t msgq,
                               buzzmsg_t payload);

   /*
    * Extracts a message from the queue.
    * You are in charge of freeing both the message data and the payload.
    * @param msgq The message queue.
    * @return The message data or NULL.
    */
   extern buzzmsg_t buzzmsg_queue_extract(buzzmsg_queue_t msgq);

   /*
    * Serializes a 8-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u8(buzzmsg_t buf,
                                    uint8_t data);

   /*
    * Deserializes a 8-bit unsigned integer.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzmsg_deserialize_u8(uint8_t* data,
                                         buzzmsg_t buf,
                                         uint32_t pos);

   /*
    * Serializes a 16-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u16(buzzmsg_t buf,
                                     uint16_t data);

   /*
    * Deserializes a 16-bit unsigned integer.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzmsg_deserialize_u16(uint16_t* data,
                                          buzzmsg_t buf,
                                          uint32_t pos);

   /*
    * Serializes a 32-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u32(buzzmsg_t buf,
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
                                          buzzmsg_t buf,
                                          uint32_t pos);

   /*
    * Serializes a float.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_float(buzzmsg_t buf,
                                       float data);

   /*
    * Deserializes a float.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzmsg_deserialize_float(float* data,
                                            buzzmsg_t buf,
                                            uint32_t pos);

   /*
    * Serializes a string.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_string(buzzmsg_t buf,
                                        const char* data);

   /*
    * Deserializes a string.
    * The data is read from the given buffer starting at the given position.
    * The buffer is treated as a dynamic array of uint8_t.
    * @param data The deserialized data of the element. You are in charge of freeing it.
    * @param buf The input buffer where the serialized data is stored.
    * @param pos The position at which the data starts.
    * @return The new position in the buffer, of -1 in case of error.
    */
   extern int64_t buzzmsg_deserialize_string(char** data,
                                             buzzmsg_t buf,
                                             uint32_t pos);

#ifdef __cplusplus
}
#endif

/*
 * Create a new message queue.
 * @param cap The initial capacity of the queue. Must be >0.
 */
#define buzzmsg_queue_new(cap) buzzdarray_new(cap, sizeof(buzzmsg_t), NULL)

/*
 * Destroys a message queue.
 * @param msgq The message queue.
 */
#define buzzmsg_queue_destroy(msgq) buzzdarray_destroy(msgq)

/*
 * Returns the size of a message queue.
 * @param msgq The message queue.
 * @return The size of a message queue.
 */
#define buzzmsg_queue_size(msgq) buzzdarray_size(msgq)

/*
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define buzzmsg_queue_isempty(msgq) buzzdarray_isempty(msgq)

/*
 * Returns the message at the given position in the queue.
 * @param msg The message queue.
 * @param pos The position.
 * @return The message at the given position.
 */
#define buzzmsg_queue_get(msg, pos) (*buzzdarray_get(msg, pos, buzzmsg_t))

/*
 * Create a new message.
 * @param cap The initial capacity of the message payload. Must be >0.
 */
#define buzzmsg_new(cap) buzzdarray_new(cap, sizeof(uint8_t), NULL)

/*
 * Create a new message from the given buffer.
 * @param buf The buffer.
 * @param buf_size The size of the buffer in bytes.
 */
#define buzzmsg_frombuffer(buf, buf_size) buzzdarray_frombuffer(buf, buf_size, sizeof(uint8_t), NULL)

/*
 * Destroys a message.
 * @param msg The message.
 */
#define buzzmsg_destroy(msg) buzzdarray_destroy(msg)

/*
 * Returns the size of a message.
 * @param msg The message.
 * @return The size of a message.
 */
#define buzzmsg_size(msg) buzzdarray_size(msg)

/*
 * Returns the byte at the given position.
 * @param msg The message.
 * @param pos The position.
 * @return The byte at the given position.
 */
#define buzzmsg_get(msg, pos) (*buzzdarray_get(msg, pos, uint8_t))

#endif
