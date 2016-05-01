#include "buzzstrman.h"
#include <stdlib.h>
#include <string.h>

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
                            sizeof(char*),
                            buzzdict_int16keyhash,
                            buzzdict_int16keycmp,
                            NULL);
   x->protect = 0;
   x->maxsid = 0;
   x->gcdata = NULL;
   return x;
}

/****************************************/
/****************************************/

void buzzstrman_str_destroy(const void* key, void* data, void* params) {
   free(*(char**)data);
}

void buzzstrman_destroy(buzzstrman_t* sm) {
   /* Dispose of the strings */
   buzzdict_foreach((*sm)->id2str, buzzstrman_str_destroy, NULL);
   /* Dispose of the structures */
   buzzdict_destroy(&((*sm)->str2id));
   buzzdict_destroy(&((*sm)->id2str));
   /* Dispose of the manager */
   free(*sm);
   *sm = 0;
}

/****************************************/
/****************************************/

void buzzstrman_protect(buzzstrman_t sm) {
   sm->protect = buzzdict_size(sm->str2id);
}

/****************************************/
/****************************************/

uint16_t buzzstrman_register(buzzstrman_t sm,
                             const char* str) {
   /* Look for the id */
   const uint16_t* id = buzzdict_get(sm->str2id, &str, uint16_t);
   /* Found? */
   if(id) return *id;
   /* Not found, add a new string */
   uint16_t id2 = sm->maxsid;
   ++sm->maxsid;
   char* str2 = strdup(str);
   buzzdict_set(sm->str2id, &str2, &id2);
   buzzdict_set(sm->id2str, &id2, &str2);
   return id2;
}

/****************************************/
/****************************************/

const char* buzzstrman_get(buzzstrman_t sm,
                           uint16_t sid) {
   const char** x = buzzdict_get(sm->id2str, &sid, char*);
   if(x) return (*x);
   return NULL;
}

/****************************************/
/****************************************/

void buzzstrman_gc_unmark(const void* key,
                          void* data,
                          void* param) {
   /* Make sure we're not adding protected strings to the tree */
   if(*(uint16_t*)data >= ((buzzstrman_t)param)->protect) {
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
   /* Nothing to do if sid < protected */
   if(sid < sm->protect) return;
   /* Remove string from the tree */
   sm->gcdata = buzzidtree_remove((buzzidtree_t)sm->gcdata, sid);
}

/****************************************/
/****************************************/

void buzzstrman_gc_dispose(uint16_t sid,
                           void* param) {
   /* Get string corresponding to given sid */
   const char* str = *buzzdict_get(((buzzstrman_t)param)->id2str, &sid, char*);
   /* Get rid of both the sid and the string */
   buzzdict_remove(((buzzstrman_t)param)->str2id, &str);
   buzzdict_remove(((buzzstrman_t)param)->id2str, &sid);
   free((char*)str);
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
