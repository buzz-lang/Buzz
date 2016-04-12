#include "buzzoutmsg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>
#include <string.h>

/****************************************/
/****************************************/

/*
 * Broadcast message data
 */
struct buzzoutmsg_broadcast_s {
   int type;
   uint16_t id;
   buzzobj_t value;
};

/*
 * Swarm message data
 */
struct buzzoutmsg_swarm_s {
   int type;
   uint16_t* ids;
   uint16_t size;
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
   struct buzzoutmsg_broadcast_s bc;
   struct buzzoutmsg_swarm_s     sw;
   struct buzzoutmsg_vstig_s     vs;
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
      case BUZZMSG_BROADCAST:
         buzzobj_destroy(&m->bc.value);
         break;
      case BUZZMSG_SWARM_JOIN:
      case BUZZMSG_SWARM_LEAVE:
      case BUZZMSG_SWARM_LIST:
         if(m->sw.size > 0) free(m->sw.ids);
         break;
      case BUZZMSG_VSTIG_PUT:
      case BUZZMSG_VSTIG_QUERY:
         free(m->vs.key);
         buzzobj_destroy(&m->vs.data->data);
         free(m->vs.data);
         break;
   }
   free(m);
}

void buzzoutmsg_vstig_destroy(const void* key, void* data, void* params) {
   free((void*)key);
   buzzdict_destroy((buzzdict_t*)data);
   free(data);
}

int buzzoutmsg_vstig_cmp(const void* a, const void* b) {
   if((uintptr_t)a < (uintptr_t)b) return -1;
   if((uintptr_t)a < (uintptr_t)b) return  1;
   return 0;
}

/****************************************/
/****************************************/

buzzoutmsg_queue_t buzzoutmsg_queue_new(uint16_t robot) {
   buzzoutmsg_queue_t q = (buzzoutmsg_queue_t)malloc(sizeof(struct buzzoutmsg_queue_s));
   q->queues[BUZZMSG_BROADCAST]   = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_SWARM_LIST]  = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_SWARM_JOIN]  = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_SWARM_LEAVE] = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_VSTIG_PUT]   = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->queues[BUZZMSG_VSTIG_QUERY] = buzzdarray_new(1, sizeof(buzzoutmsg_t), buzzoutmsg_destroy);
   q->vstig = buzzdict_new(10,
                           sizeof(uint16_t),
                           sizeof(buzzdict_t),
                           buzzdict_uint16keyhash,
                           buzzdict_uint16keycmp,
                           buzzoutmsg_vstig_destroy);
   q->robot = robot;
   return q;
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_destroy(buzzoutmsg_queue_t* msgq) {
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_BROADCAST]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_SWARM_LIST]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_SWARM_JOIN]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_SWARM_LEAVE]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_VSTIG_PUT]));
   buzzdarray_destroy(&((*msgq)->queues[BUZZMSG_VSTIG_QUERY]));
   buzzdict_destroy(&((*msgq)->vstig));
   free(*msgq);
}

/****************************************/
/****************************************/

uint32_t buzzoutmsg_queue_size(buzzoutmsg_queue_t msgq) {
   return
      buzzdarray_size(msgq->queues[BUZZMSG_BROADCAST]) +
      buzzdarray_size(msgq->queues[BUZZMSG_SWARM_LIST]) +
      buzzdarray_size(msgq->queues[BUZZMSG_SWARM_JOIN]) +
      buzzdarray_size(msgq->queues[BUZZMSG_SWARM_LEAVE]) +
      buzzdarray_size(msgq->queues[BUZZMSG_VSTIG_PUT]) +
      buzzdarray_size(msgq->queues[BUZZMSG_VSTIG_QUERY]);
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_append_broadcast(buzzoutmsg_queue_t msgq,
                                       uint16_t id,
                                       buzzobj_t value) {
   /* Make a new BROADCAST message */
   buzzoutmsg_t m = (buzzoutmsg_t)malloc(sizeof(union buzzoutmsg_u));
   m->bc.type = BUZZMSG_BROADCAST;
   m->bc.id = id;
   m->bc.value = buzzobj_clone(value);
   /* Queue it */
   buzzdarray_push(msgq->queues[BUZZMSG_BROADCAST], &m);   
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_append_swarm_list(buzzoutmsg_queue_t msgq,
                                        const buzzdarray_t ids) {
   /* Invariants:
    * - Only one list message can be queued at any time;
    * - If a list message is already queued, join/leave messages are not
    */
   /* Delete every existing SWARM related message */
   buzzdarray_clear(msgq->queues[BUZZMSG_SWARM_LIST], 1);
   buzzdarray_clear(msgq->queues[BUZZMSG_SWARM_JOIN], 1);
   buzzdarray_clear(msgq->queues[BUZZMSG_SWARM_LEAVE], 1);
   /* Make a new LIST message */
   buzzoutmsg_t m = (buzzoutmsg_t)malloc(sizeof(union buzzoutmsg_u));
   m->sw.type = BUZZMSG_SWARM_LIST;
   m->sw.size = buzzdarray_size(ids);
   m->sw.ids = (uint16_t*)malloc(m->sw.size * sizeof(uint16_t));
   memcpy(m->sw.ids, ids->data, m->sw.size * sizeof(uint16_t));
   /* Queue the new LIST message */
   buzzdarray_push(msgq->queues[BUZZMSG_SWARM_LIST], &m);
}

/****************************************/
/****************************************/

static void append_to_swarm_queue(buzzdarray_t q, uint16_t id, int type) {
   /* Is the queue empty? */
   if(buzzdarray_isempty(q)) {
      /* Yes, add the element at the end */
      buzzoutmsg_t m = (buzzoutmsg_t)malloc(sizeof(union buzzoutmsg_u));
      m->sw.type = type;
      m->sw.size = 1;
      m->sw.ids = (uint16_t*)malloc(sizeof(uint16_t));
      m->sw.ids[0] = id;
      buzzdarray_push(q, &m);
   }
   else {
      /* Queue not empty - look for a message with the same id */
      int found = 0;
      uint32_t i;
      for(i = 0; i < buzzdarray_size(q) && !found; ++i) {
         found = buzzdarray_get(q, i, buzzoutmsg_t)->sw.ids[0] == id;
      }
      /* Message found? */
      if(!found) {
         /* No, append a new message the passed id */
         buzzoutmsg_t m = (buzzoutmsg_t)malloc(sizeof(union buzzoutmsg_u));
         m->sw.type = type;
         m->sw.size = 1;
         m->sw.ids = (uint16_t*)malloc(sizeof(uint16_t));
         m->sw.ids[0] = id;
         buzzdarray_push(q, &m);
      }
   }
}

static void remove_from_swarm_queue(buzzdarray_t q, uint16_t id) {
   /* Is the queue empty? If so, nothing to do */
   if(buzzdarray_isempty(q)) return;
   /* Queue not empty - look for a message with the same id */
   uint32_t i;
   for(i = 0; i < buzzdarray_size(q); ++i) {
      if(buzzdarray_get(q, i, buzzoutmsg_t)->sw.ids[0] == id) break;
   }
   /* Message found? If so, remove it */
   if(i < buzzdarray_size(q)) buzzdarray_remove(q, i);
}

void buzzoutmsg_queue_append_swarm_joinleave(buzzoutmsg_queue_t msgq,
                                             int type,
                                             uint16_t id) {
   /* Invariants:
    * - Only one list message can be qeueued at any time;
    * - If a list message is already queued, join/leave messages are not
    */
   /* Is there a LIST message? */
   if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_LIST])) {
      /* Yes, get a handle to the message */
      buzzoutmsg_t l = buzzdarray_get(msgq->queues[BUZZMSG_SWARM_LIST], 0, buzzoutmsg_t);
      /* Go through the ids in the list and look for the passed id */
      uint16_t i = 0;
      while(i < l->sw.size && l->sw.ids[i] != id) ++i;
      /* Id found? */
      if(i < l->sw.size) {
         /* Yes; is the message a LEAVE? */
         if(type == BUZZMSG_SWARM_LEAVE) {
            /* Yes: remove it from the list */
            --(l->sw.size);
            memmove(l->sw.ids+i, l->sw.ids+i+1, (l->sw.size-i) * sizeof(uint16_t));
         }
         /* If the message is a JOIN, there's nothing to do */
      }
      else {
         /* Id not found; is the message a JOIN? */
         if(type == BUZZMSG_SWARM_JOIN) {
            /* Yes: add it to the list */
            ++(l->sw.size);
            l->sw.ids = realloc(l->sw.ids, l->sw.size * sizeof(uint16_t));
            l->sw.ids[l->sw.size-1] = id;
         }
         /* If the message is a LEAVE, there's nothing to do */
      }
   }
   else {
      /* No LIST message present - send an individual message */
      if(type == BUZZMSG_SWARM_JOIN) {
         /* Look for a duplicate in the JOIN queue - if not add one  */
         append_to_swarm_queue(msgq->queues[BUZZMSG_SWARM_JOIN], id, BUZZMSG_SWARM_JOIN);
         /* Look for an entry in the LEAVE queue and remove it  */
         remove_from_swarm_queue(msgq->queues[BUZZMSG_SWARM_LEAVE], id);
      }
      else if(type == BUZZMSG_SWARM_LEAVE) {
         /* Look for an entry in the JOIN queue and remove it */
         remove_from_swarm_queue(msgq->queues[BUZZMSG_SWARM_JOIN], id);
         /* Look for a duplicate in the LEAVE queue - if not add one  */
         append_to_swarm_queue(msgq->queues[BUZZMSG_SWARM_LEAVE], id, BUZZMSG_SWARM_LEAVE);
      }
   }
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_append_vstig(buzzoutmsg_queue_t msgq,
                                   int type,
                                   uint16_t id,
                                   const buzzobj_t key,
                                   const buzzvstig_elem_t data) {
   /* Look for a duplicate message in the dictionary */
   const struct buzzoutmsg_vstig_s** e = NULL;
   /* Virtual stigmergy to actually use */
   buzzdict_t vs = NULL;
   /* Look for the virtual stigmergy */
   const buzzdict_t* tvs = buzzdict_get(msgq->vstig, &id, buzzdict_t);
   if(tvs) {
      /* Virtual stigmergy found, look for the key */
      vs = *tvs;
      e = buzzdict_get(vs, &key, struct buzzoutmsg_vstig_s*);
   }
   else {
      /* Virtual stigmergy not found, create it */
      vs = buzzdict_new(10,
                        sizeof(buzzobj_t),
                        sizeof(struct buzzoutmsg_vstig_s*),
                        buzzoutmsg_obj_hash,
                        buzzoutmsg_obj_cmp,
                        NULL);
      buzzdict_set(msgq->vstig, &id, &vs);
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
   buzzdict_set(vs, &m->vs.key, &m);
   /* Add a new message to the queue */
   buzzdarray_push(msgq->queues[type], &m);
}

/****************************************/
/****************************************/

buzzmsg_payload_t buzzoutmsg_queue_first(buzzoutmsg_queue_t msgq) {
   if(!buzzdarray_isempty(msgq->queues[BUZZMSG_BROADCAST])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_BROADCAST],
                                      0, buzzoutmsg_t);
      /* Make a new message */
      buzzmsg_payload_t m = buzzmsg_payload_new(10);
      buzzmsg_serialize_u8(m, BUZZMSG_BROADCAST);
      buzzmsg_serialize_u16(m, msgq->robot);
      buzzmsg_serialize_u16(m, f->bc.id);
      buzzobj_serialize(m, f->bc.value);
      /* Return message */
      return m;
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_LIST])) {
      uint16_t i;
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_SWARM_LIST],
                                      0, buzzoutmsg_t);
      /* Make a new message */
      buzzmsg_payload_t m = buzzmsg_payload_new(10);
      buzzmsg_serialize_u8(m, BUZZMSG_SWARM_LIST);
      buzzmsg_serialize_u16(m, msgq->robot);
      buzzmsg_serialize_u16(m, f->sw.size);
      for(i = 0; i < f->sw.size; ++i) {
         buzzmsg_serialize_u16(m, f->sw.ids[i]);
      }
      /* Return message */
      return m;      
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
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_JOIN])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_SWARM_JOIN],
                                      0, buzzoutmsg_t);
      /* Make a new message */
      buzzmsg_payload_t m = buzzmsg_payload_new(5);
      buzzmsg_serialize_u8(m, BUZZMSG_SWARM_JOIN);
      buzzmsg_serialize_u16(m, msgq->robot);
      buzzmsg_serialize_u16(m, f->sw.ids[0]);
      /* Return message */
      return m;      
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_LEAVE])) {
      /* Take the first message in the queue */
      buzzoutmsg_t f = buzzdarray_get(msgq->queues[BUZZMSG_SWARM_LEAVE],
                                      0, buzzoutmsg_t);
      /* Make a new message */
      buzzmsg_payload_t m = buzzmsg_payload_new(5);
      buzzmsg_serialize_u8(m, BUZZMSG_SWARM_LEAVE);
      buzzmsg_serialize_u16(m, msgq->robot);
      buzzmsg_serialize_u16(m, f->sw.ids[0]);
      /* Return message */
      return m;      
   }
   /* Empty queue */
   return NULL;
}

/****************************************/
/****************************************/

void buzzoutmsg_queue_next(buzzoutmsg_queue_t msgq) {
   if(!buzzdarray_isempty(msgq->queues[BUZZMSG_BROADCAST])) {
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_BROADCAST], 0);
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_LIST])) {
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_SWARM_LIST], 0);
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
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_JOIN])) {
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_SWARM_JOIN], 0);
   }
   else if(!buzzdarray_isempty(msgq->queues[BUZZMSG_SWARM_LEAVE])) {
      /* Remove the first message in the queue */
      buzzdarray_remove(msgq->queues[BUZZMSG_SWARM_LEAVE], 0);
   }
}

/****************************************/
/****************************************/
