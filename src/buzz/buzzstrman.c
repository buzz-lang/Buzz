#include "buzzstrman.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

/****************************************/
/****************************************/

/*
 * String container for the id2str dictionary.
 * The string container stores both a pointer to the string data and
 * the 'protect' flag.
 */
struct buzzid2strdata_s {
   char* str;
   int protect;
};
typedef struct buzzid2strdata_s* buzzid2strdata_t;

static buzzid2strdata_t buzzid2strdata_new(char* str,
                                           int protect) {
   buzzid2strdata_t x = (buzzid2strdata_t)malloc(sizeof(struct buzzid2strdata_s));
   x->str = str;
   x->protect = protect;
   return x;
}

static void buzzid2strdata_destroy(const void* key,
                                   void* data,
                                   void* params) {
   free((void*)key);
   free(*(buzzid2strdata_t*)data);
   free(data);
}

/****************************************/
/****************************************/

/*
 * This is a binary tree used to manage garbage collection of strings.
 *
 * If a string is stored in the tree, it will be considered unmarked
 * and, thus, to dispose of.  A binary tree is used because it ensures
 * fast insertion and removal even when many strings are present.
 *
 * Also, the string ids are sparse, making it important to have a data
 * structure that does not care about holes in the id set.
 */
struct buzzidtree_s {
   uint16_t sid;
   struct buzzidtree_s* left;
   struct buzzidtree_s* right;
};
typedef struct buzzidtree_s* buzzidtree_t;

/* Creates a new tree node */
buzzidtree_t buzzidtree_new(uint16_t sid) {
   buzzidtree_t x = (buzzidtree_t)malloc(sizeof(struct buzzidtree_s));
   x->sid = sid;
   x->left = NULL;
   x->right = NULL;
   return x;
}

/* Destroys a tree and its children */
void buzzidtree_destroy(buzzidtree_t t) {
   if(!t) return;
   if(t->left) buzzidtree_destroy(t->left);
   if(t->right) buzzidtree_destroy(t->right);
   free(t);
}

/* Inserts a new node in the tree */
static buzzidtree_t buzzidtree_insert(buzzidtree_t t,
                                      uint16_t sid) {
   /* Insert element here */
   if(!t) return buzzidtree_new(sid);
   /* Insert element on the left */
   else if(sid < t->sid) t->left = buzzidtree_insert(t->left, sid);
   /* Insert element on the right */
   else if(sid > t->sid) t->right = buzzidtree_insert(t->right, sid);
   /* Return current tree */
   return t;
}

/* Finds the smallest node in a tree */
static buzzidtree_t buzzidtree_find_min(buzzidtree_t t) {
   /* This node has no smaller child */
   if(t->left == NULL) return t;
   /* Go to the smaller child */
   return buzzidtree_find_min(t->left);
}

/* Removes a node with the given id from the tree */
static buzzidtree_t buzzidtree_remove(buzzidtree_t t,
                                      uint16_t sid) {
   /* If the tree is empty, nothing to do */
   if(!t) return NULL;
   /* If the id is smaller than the current id, remove on the left */
   if(sid < t->sid) t->left = buzzidtree_remove(t->left, sid);
   /* If the id is greater than the current id, remove on the right */
   else if(sid > t->sid) t->right = buzzidtree_remove(t->right, sid);
   else {
      /* We found the node to remove */
      /* If the node has no children, simply remove it */
      if(!t->left && !t->right) {
         free(t);
         t = NULL;
      }
      /* If the element has one child, put the child in place of this tree, and remove the tree */
      else if(!t->right) {
         buzzidtree_t x = t;
         t = t->left;
         free(x);
      }
      else if(!t->left) {
         buzzidtree_t x = t;
         t = t->right;
         free(x);
      }
      /* Both children exist for this node */
      else {
         /* Look for the minimum id on the right of the node to eliminate */
         buzzidtree_t m = buzzidtree_find_min(t->right);
         /* Copy its value over the current root */
         t->sid = m->sid;
         /* Delete the minimum node */
         t->right = buzzidtree_remove(t->right, m->sid);
      }
   }
   return t;
}

/* Function pointer for buzzidtree_foreach() */
typedef void (*buzzidtree_funp)(uint16_t sid, void* params);

/* Visits every node in the tree and executes the given function */
static void buzzidtree_foreach(buzzidtree_t t,
                               buzzidtree_funp fun,
                               void* params) {
   if(!t) return;
   fun(t->sid, params);
   if(t->left)  buzzidtree_foreach(t->left, fun, params);
   if(t->right) buzzidtree_foreach(t->right, fun, params);
}

/****************************************/
/****************************************/

buzzstrman_t buzzstrman_new() {
   buzzstrman_t x = (buzzstrman_t)malloc(sizeof(struct buzzstrman_s));
   x->str2id = buzzdict_new(10,
                            sizeof(char*),
                            sizeof(uint16_t),
                            buzzdict_strkeyhash,
                            buzzdict_strkeycmp,
                            NULL);
   x->id2str = buzzdict_new(10,
                            sizeof(uint16_t),
                            sizeof(buzzid2strdata_t),
                            buzzdict_int16keyhash,
                            buzzdict_int16keycmp,
                            buzzid2strdata_destroy);
   x->maxsid = 0;
   x->gcdata = NULL;
   return x;
}

/****************************************/
/****************************************/

void buzzstrman_str_destroy(const void* key, void* data, void* params) {
   free(*(char**)key);
}

void buzzstrman_destroy(buzzstrman_t* sm) {
   /* Dispose of the strings */
   buzzdict_foreach((*sm)->str2id, buzzstrman_str_destroy, NULL);
   /* Dispose of the structures */
   buzzdict_destroy(&((*sm)->str2id));
   buzzdict_destroy(&((*sm)->id2str));
   /* Dispose of the manager */
   free(*sm);
   *sm = 0;
}

/****************************************/
/****************************************/

uint16_t buzzstrman_register(buzzstrman_t sm,
                             const char* str,
                             int protect) {
   /* Look for the id */
   const uint16_t* id = buzzdict_get(sm->str2id, &str, uint16_t);
   /* Found? */
   if(id) {
      /* Yes; is the passed 'protect' flag set? */
      if(protect) {
         /* Set the flag for the record too */
         buzzid2strdata_t sd = *buzzdict_get(sm->id2str, id, buzzid2strdata_t);
         sd->protect = 1;
      }
      /* Return the found id */
      return *id;
   }
   /* Not found, add a new string */
   uint16_t id2 = sm->maxsid;
   ++sm->maxsid;

   /* Avoid id = 0 */
   if( !sm->maxsid ) ++sm->maxsid;

   /* Avoid overwriting existing strings */
   while(buzzdict_get(sm->id2str, &sm->maxsid, buzzid2strdata_t))
     ++sm->maxsid;

   char* str2 = strdup(str);
   buzzid2strdata_t sd = buzzid2strdata_new(str2, protect);
   buzzdict_set(sm->str2id, &str2, &id2);
   buzzdict_set(sm->id2str, &id2, &sd);
   return id2;
}

/****************************************/
/****************************************/

const char* buzzstrman_get(buzzstrman_t sm,
                           uint16_t sid) {
   const buzzid2strdata_t* x = buzzdict_get(sm->id2str, &sid, buzzid2strdata_t);
   if(x) return (*x)->str;
   return NULL;
}

/****************************************/
/****************************************/

void buzzstrman_gc_unmark(const void* key,
                          void* data,
                          void* param) {
   /* Make sure we're not adding protected strings to the tree */
   const buzzid2strdata_t* sd = buzzdict_get(((buzzstrman_t)param)->id2str,
                                             (uint16_t*)data,
                                             buzzid2strdata_t);
   if(sd) {
      /* Nothing to do if string is protected */
      if((*sd)->protect) return;
      /* Add string */
      ((buzzstrman_t)param)->gcdata =
         buzzidtree_insert(((buzzstrman_t)param)->gcdata,
                           *(uint16_t*)data);
   }
}

void buzzstrman_gc_clear(buzzstrman_t sm) {
   /* Go through all the strings and add them to the tree */
   buzzdict_foreach(sm->str2id, buzzstrman_gc_unmark, sm);
}

/****************************************/
/****************************************/

void buzzstrman_gc_mark(buzzstrman_t sm,
                        uint16_t sid) {
   /* Get string corresponding to given sid */
   const buzzid2strdata_t* sd = buzzdict_get(sm->id2str, &sid, buzzid2strdata_t);
   if(sd) {
      /* Nothing to do if string is protected */
      if((*sd)->protect) return;
      /* Remove string from the tree */
      sm->gcdata = buzzidtree_remove((buzzidtree_t)sm->gcdata, sid);
   }
}

/****************************************/
/****************************************/

void buzzstrman_gc_dispose(uint16_t sid,
                           void* param) {
   /* Get string corresponding to given sid */
   buzzid2strdata_t sd = *buzzdict_get(((buzzstrman_t)param)->id2str, &sid, buzzid2strdata_t);
   /* Get rid of both the sid and the string */
   char* str = sd->str;
   buzzdict_remove(((buzzstrman_t)param)->str2id, &str);
   buzzdict_remove(((buzzstrman_t)param)->id2str, &sid);
   free(str);
}

void buzzstrman_gc_prune(buzzstrman_t sm) {
   /* Remove all the strings in the tree */
   buzzidtree_foreach((buzzidtree_t)sm->gcdata, buzzstrman_gc_dispose, sm);
   /* Get rid of the tree */
   buzzidtree_destroy(sm->gcdata);
   sm->gcdata = NULL;
}

/****************************************/
/****************************************/

void buzzstrman_print_id2str(const void* key,
                             void* data,
                             void* param) {
   buzzid2strdata_t sd = *(buzzid2strdata_t*)data;
   char c = ' ';
   if(sd->protect) c = '*';
   printf("\t[%c] %" PRIu16 " -> '%s'\n", c, *(uint16_t*)key, sd->str);
}

void buzzstrman_print_str2id(const void* key,
                             void* data,
                             void* param) {
   printf("\t'%s' -> %" PRIu16 "\n", *(char**)key, *(uint16_t*)data);
}

void buzzstrman_print(buzzstrman_t sm) {
   printf("ID -> STRING (%" PRIu32 " elements)\n", buzzdict_size(sm->id2str));
   buzzdict_foreach(sm->id2str, buzzstrman_print_id2str, sm);
   printf("STRING -> ID (%" PRIu32 " elements)\n", buzzdict_size(sm->str2id));
   buzzdict_foreach(sm->str2id, buzzstrman_print_str2id, sm);
   printf("\n\n");
}

/****************************************/
/****************************************/
