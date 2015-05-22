#include <buzzoutmsg.h>
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

/*
 * Shout message data
 */
struct buzzoutmsg_shout_s {
   int type;
   buzzmsg_payload_t payload;
};

/*
 * Virtual stigmergy message data
 */
struct buzzoutmsg_vstig_s {
   int type;
   uint16_t id;
   buzzobj_t key;
   buzzvstig_elem_t data;
};

/*
 * Generic message data
 */
union buzzoutmsg_u {
   int type;
   struct buzzoutmsg_shout_s sh;
   struct buzzoutmsg_vstig_s vs;
};
typedef union buzzoutmsg_u* buzzoutmsg_t;

/****************************************/
/****************************************/

uint32_t buzzoutmsg_obj_hash(const void* key) {
   return buzzobj_hash(*(buzzobj_t*)key);
}

int buzzoutmsg_obj_cmp(const void* a, const void* b) {
   return buzzobj_cmp(*(const buzzobj_t*)a, *(const buzzobj_t*)b);
}

void buzzoutmsg_destroy(uint32_t pos, void* data, void* params) {
   buzzoutmsg_t m = *(buzzoutmsg_t*)data;
   switch(m->type) {
      case BUZZMSG_SHOUT:
         free(m->sh.payload);
         break;
      case BUZZMSG_SWARMS:
         // TODO
         break;
      case BUZZMSG_VSTIG_PUT:
      case BUZZMSG_VSTIG_QUERY:
         // Leaks memory if structured Buzz objs are stored
         free(m->vs.key);
         free(m->vs.data);
         break;
   }
   free(m);
}

void buzzoutmsg_vstig_destroy(const void* key, void* data, void* params) {
   free(*(buzzdict_t*)data);
}

int buzzoutmsg_vstig_cmp(const void* a, const void* b) {
   if((uintptr_t)a < (uintptr_t)b) return -1;
   if((uintptr_t)a < (uintptr_t)b) return  1;
   return 0;
}

/****************************************/
/****************************************/

buzzoutmsg_queue_t buzzoutmsg_queue_new() {
   buzzoutmsg_queue_t q = (buzzoutmsg_queue_t)malloc(sizeof(struct buzzoutmsg_queue_s));
   q->queues[BUZZMSG_SHOUT]       = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_SWARMS]      = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_VSTIG_PUT]   = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_VSTIG_QUERY] = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->vstig = buzzdict_new(10,
                           sizeof(uint16_t),
                           sizeof(buzzdict_t),
                           buzzdict_uint16keyhash,
                           buzzdict_uint16keycmp,
                           buzzoutmsg_vstig_destroy);
   return q;
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_destroy(buzzoutmsg_queue_t* msgq) {
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_SHOUT]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_SWARMS]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_VSTIG_PUT]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_VSTIG_QUERY]));
   buzzdict_destroy(&((*msgq)->vstig));
}

/****************************************/
/****************************************/

uint32_t buzzoutmsg_queue_size(buzzoutmsg_queue_t msgq) {
   return
      buzzdarray_size(msgq->queues[BUZZMSG_SHOUT]) +
      buzzdarray_size(msgq->queues[BUZZMSG_SWARMS]) +
      buzzdarray_size(msgq->queues[BUZZMSG_VSTIG_PUT]) +
      buzzdarray_size(msgq->queues[BUZZMSG_VSTIG_QUERY]);
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_append_shout(buzzoutmsg_queue_t msgq,
                                   buzzmsg_payload_t payload) {
   buzzoutmsg_t m = (buzzoutmsg_t)malloc(sizeof(union buzzoutmsg_u));
   m->sh.type = BUZZMSG_SHOUT;
   m->sh.payload = payload;
   buzzdarray_push(msgq->queues[BUZZMSG_SHOUT], &m);
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_append_swarm(buzzoutmsg_queue_t msgq,
                                   buzzmsg_payload_t payload) {
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_append_vstig(buzzoutmsg_queue_t msgq,
                                   int type,
                                   uint16_t id,
                                   const buzzobj_t key,
                                   const buzzvstig_elem_t data) {
   /* Look for a duplicate message in the dictionary */
   struct buzzoutmsg_vstig_s** e = NULL;
   /* Look for the virtual stigmergy */
   buzzdict_t* vs = buzzdict_get(msgq->vstig, &id, buzzdict_t);
   if(vs) {
      /* Virtual stigmergy found, look for the key */
      e = buzzdict_get(*vs, &key, struct buzzoutmsg_vstig_s*);
   }
   else {
      /* Virtual stigmergy not found, create it */
      buzzdict_t nvs = buzzdict_new(10,
                                    sizeof(buzzobj_t),
                                    sizeof(struct buzzoutmsg_vstig_s*),
                                    buzzoutmsg_obj_hash,
                                    buzzoutmsg_obj_cmp,
                                    NULL);
      vs = &nvs;
      buzzdict_set(msgq->vstig, &id, vs);
   }
   /* Do we have a duplicate? */
   if(e) {
      /* Yes; if the duplicate is newer than the passed message, nothing to do */
      if((*e)->data->timestamp >= data->timestamp) return;
      /* The duplicate is older */
      /* Remove the entry from the queue */
      buzzdarray_remove(
         msgq->queues[(*e)->type],
         buzzdarray_find(msgq->queues[(*e)->type], buzzoutmsg_vstig_cmp, NULL));
   }
   /* Create a new message */
   buzzoutmsg_t m = (buzzoutmsg_t)malloc(sizeof(union buzzoutmsg_u));
   m->vs.type = type;
   m->vs.id = id;
   m->vs.key = buzzobj_clone(key);
   m->vs.data = buzzvstig_elem_clone(data);
   /* Update the dictionary */
   buzzdict_set(*vs, &m->vs.key, &m);
   /* Add a new message to the queue */
   buzzdarray_push(msgq->queues[type], &m);
}

/****************************************/
/****************************************/

buzzmsg_payload_t buzzoutmsg_queue_first(buzzoutmsg_queue_t msgq) {
   if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SHOUT])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_SHOUT],
                                      0, buzzoutmsg_t);
      /* Take its payload */
      buzzmsg_payload_t p = f->sh.payload;
      /* Copy payload into new message */
      buzzmsg_payload_t m = buzzmsg_payload_frombuffer(p, buzzmsg_payload_size(p));
      /* Prepend message type */
      buzzdarray_insert(m, 0, (uint8_t*)(&(f->type)));
      /* Return message */
      return m;
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARMS])) {
      // TODO
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_VSTIG_PUT])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_VSTIG_PUT],
                                      0, buzzoutmsg_t);
      /* Make a new message */
      buzzmsg_payload_t m = buzzmsg_payload_new(10);
      buzzmsg_serialize_u8(m, BUZZMSG_VSTIG_PUT);
      buzzmsg_serialize_u16(m, f->vs.id);
      buzzvstig_elem_serialize(m, f->vs.key, f->vs.data);
      /* Return message */
      return m;
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_VSTIG_QUERY])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_VSTIG_QUERY],
                                      0, buzzoutmsg_t);
      /* Make a new message */
      buzzmsg_payload_t m = buzzmsg_payload_new(10);
      buzzmsg_serialize_u8(m, BUZZMSG_VSTIG_QUERY);
      buzzmsg_serialize_u16(m, f->vs.id);
      buzzvstig_elem_serialize(m, f->vs.key, f->vs.data);
      /* Return message */
      return m;
   }
   /* Empty queue */
   return NULL;
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_next(buzzoutmsg_queue_t msgq) {
   if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SHOUT])) {
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_SHOUT], 0);
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARMS])) {
      // TODO
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_VSTIG_PUT])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_VSTIG_PUT],
                                      0, buzzoutmsg_t);
      /* Remove the element in the vstig dictionary */
      buzzdict_remove(
         *buzzdict_get(msgq->vstig, &f->vs.id, buzzdict_t),
         &f->vs.key);
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_VSTIG_PUT], 0);
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_VSTIG_QUERY])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_VSTIG_QUERY],
                                      0, buzzoutmsg_t);
      /* Remove the element in the vstig dictionary */
      buzzdict_remove(
         *buzzdict_get(msgq->vstig, &f->vs.id, buzzdict_t),
         &f->vs.key);
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_VSTIG_QUERY], 0);
   }
}

/****************************************/
/****************************************/
