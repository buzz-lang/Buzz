#ifndef BUZZMSG_H
#define BUZZMSG_H

#include <buzzdarray.h>

#ifdef __cplusplus
extern "C" {
#endif

   /*
    * Buzz message type.
    * The types are ordered by decreasing priority
    */
   typedef enum {
      BUZZMSG_BROADCAST = 0, // Neighbor broadcast
      BUZZMSG_SWARM_LIST,    // Swarm listing
      BUZZMSG_VSTIG_PUT,     // Virtual stigmergy PUT
      BUZZMSG_VSTIG_QUERY,   // Virtual stigmergy QUERY
      BUZZMSG_SWARM_JOIN,    // Swarm joining
      BUZZMSG_SWARM_LEAVE,   // Swarm leaving
      BUZZMSG_TYPE_COUNT     // How many Buzz message types have been defined
   } buzzmsg_payload_type_e;

   /*
    * Data of a Buzz message.
    */
   typedef buzzdarray_t buzzmsg_payload_t;

   /*
    * Serializes a 8-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u8(buzzmsg_payload_t buf,
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
                                         buzzmsg_payload_t buf,
                                         uint32_t pos);

   /*
    * Serializes a 16-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u16(buzzmsg_payload_t buf,
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
                                          buzzmsg_payload_t buf,
                                          uint32_t pos);

   /*
    * Serializes a 32-bit unsigned integer.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_u32(buzzmsg_payload_t buf,
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
                                          buzzmsg_payload_t buf,
                                          uint32_t pos);

   /*
    * Serializes a float.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_float(buzzmsg_payload_t buf,
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
                                            buzzmsg_payload_t buf,
                                            uint32_t pos);

   /*
    * Serializes a string.
    * The data is appended to the given buffer. The buffer is treated as a
    * dynamic array of uint8_t.
    * @param buf The output buffer where the serialized data is appended.
    * @param data The data to serialize.
    */
   extern void buzzmsg_serialize_string(buzzmsg_payload_t buf,
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
                                             buzzmsg_payload_t buf,
                                             uint32_t pos);

#ifdef __cplusplus
}
#endif

/*
 * Create a new message payload.
 * @param cap The initial capacity of the message payload. Must be >0.
 */
#define buzzmsg_payload_new(cap) buzzdarray_new(cap, sizeof(uint8_t), NULL)

/*
 * Create a new message payload from the given buffer.
 * @param buf The buffer.
 * @param buf_size The size of the buffer in bytes.
 */
#define buzzmsg_payload_frombuffer(buf, buf_size) buzzdarray_frombuffer(buf, buf_size, sizeof(uint8_t), NULL)

/*
 * Destroys a message payload.
 * @param msg The message payload.
 */
#define buzzmsg_payload_destroy(msg) buzzdarray_destroy(msg)

/*
 * Returns the size of a message payload.
 * @param msg The message payload.
 * @return The size of a message payload.
 */
#define buzzmsg_payload_size(msg) buzzdarray_size(msg)

/*
 * Returns the byte at the given position.
 * @param msg The message payload.
 * @param pos The position.
 * @return The byte at the given position.
 */
#define buzzmsg_payload_get(msg, pos) buzzdarray_get(msg, pos, uint8_t)

#endif
