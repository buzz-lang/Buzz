#include "buzzset.h"
#include <stdlib.h>
#include <string.h>

/****************************************/
/****************************************/

struct buzzset_tree_s {
   void* data;
   struct buzzset_tree_s* left;
   struct buzzset_tree_s* right;
};
typedef struct buzzset_tree_s* buzzset_tree_t;

/****************************************/
/****************************************/

/* Creates a new tree node */
static buzzset_tree_t buzzset_tree_new(buzzset_t s,
                                       const void* data) {
   buzzset_tree_t x = (buzzset_tree_t)malloc(sizeof(struct buzzset_tree_s));
   x->data = malloc(s->data_size);
   memcpy(x->data, data, s->data_size);
   x->left = NULL;
   x->right = NULL;
   return x;
}

/* Destroys a node and its children */
static void buzzset_tree_destroy(buzzset_t s,
                                 buzzset_tree_t n) {
   if(!n) return;
   if(n->left) buzzset_tree_destroy(s, n->left);
   if(n->right) buzzset_tree_destroy(s, n->right);
   if(s->dstryf) s->dstryf(n->data, NULL);
   free(n->data);
   free(n);
}

/* Inserts a new node in the tree */
static buzzset_tree_t buzzset_tree_insert(buzzset_t s,
                                          buzzset_tree_t n,
                                          const void* data,
                                          int* added) {
   /* Insert element here */
   if(!n) {
      *added = 1;
      return buzzset_tree_new(s, data);
   }
   else {
      int res = s->cmpf(data, n->data);
      /* Insert element on the left */
      if(res < 0)
         n->left  = buzzset_tree_insert(s, n->left, data, added);
      /* Insert element on the right */
      else if(res > 0)
         n->right = buzzset_tree_insert(s, n->right, data, added);
      else
         /* If comparison is 0, the element has already been inserted,
          * nothing to do */
         *added = 0;
      /* Return current tree */
      return n;
   }
}

/****************************************/
/****************************************/

/* Visits every node in the tree and executes the given function */
static void buzzset_tree_foreach(buzzset_tree_t n,
                                 buzzset_elem_funp fun,
                                 void* params) {
   if(!n) return;
   fun(n->data, params);
   if(n->left)  buzzset_tree_foreach(n->left, fun, params);
   if(n->right) buzzset_tree_foreach(n->right, fun, params);
}

/****************************************/
/****************************************/

/* Finds the smallest node in a tree */
static buzzset_tree_t buzzset_tree_find_min(buzzset_tree_t n) {
   /* This node has no smaller child */
   if(n->left == NULL) return n;
   /* Go to the smaller child */
   return buzzset_tree_find_min(n->left);
}

/****************************************/
/****************************************/

/* Removes a node with the given data from the tree */
static buzzset_tree_t buzzset_tree_remove(buzzset_t s,
                                          buzzset_tree_t n,
                                          const void* data,
                                          int* removed) {
   /* If the tree is empty, nothing to do */
   if(!n) {
      *removed = 0;
      return NULL;
   }
   /* Call comparison function */
   int res = s->cmpf(data, n->data);
   /* If the id is smaller than the current id, remove on the left */
   if(res < 0)
      n->left = buzzset_tree_remove(s, n->left, data, removed);
   /* If the id is greater than the current id, remove on the right */
   else if(res > 0)
      n->right = buzzset_tree_remove(s, n->right, data, removed);
   else {
      /* We found the node to remove */
      *removed = 1;
      /* Get rid of the data */
      if(s->dstryf) s->dstryf(n->data, NULL);
      /* If the node has no children, simply remove it */
      if(!n->left && !n->right) {
         free(n);
         n = NULL;
      }
      /* If the element has one child, put the child in place of this
       * tree, and remove the tree */
      else if(!n->right) {
         buzzset_tree_t x = n;
         n = n->left;
         free(x);
      }
      else if(!n->left) {
         buzzset_tree_t x = n;
         n = n->right;
         free(x);
      }
      /* Both children exist for this node */
      else {
         /* Look for the minimum id on the right of the node to eliminate */
         buzzset_tree_t m = buzzset_tree_find_min(n->right);
         /* Copy its value over the current root */
         n->data = m->data;
         /* Delete the minimum node */
         n->right = buzzset_tree_remove(s, n->right, m->data, removed);
      }
   }
   return n;
}

/****************************************/
/****************************************/

/* Removes a node with the given data from the tree */
static buzzset_tree_t buzzset_tree_find(buzzset_t s,
                                        buzzset_tree_t n,
                                        const void* data) {
   /* If the tree is empty, nothing to do */
   if(!n) return NULL;
   /* Call comparison function */
   int res = s->cmpf(data, n->data);
   /* If the data is smaller than the current data, recur on the left */
   if(res < 0)
      return buzzset_tree_find(s, n->left, data);
   /* If the data is greater than the current data, recur on the right */
   else if(res > 0)
      return buzzset_tree_find(s, n->right, data);
   /* We found the node to remove */
   return n;
}

/****************************************/
/****************************************/

buzzset_t buzzset_new(uint32_t elem_size,
                      buzzset_elem_cmpp elem_cmp,
                      buzzset_elem_funp elem_destroy) {
   buzzset_t s = (buzzset_t)malloc(sizeof(struct buzzset_s));
   s->data = NULL;
   s->size = 0;
   s->dstryf = elem_destroy;
   s->cmpf = elem_cmp;
   s->data_size = elem_size;
   return s;
}

/****************************************/
/****************************************/

void buzzset_destroy(buzzset_t* s) {
   /* Get rid of every data element */
   buzzset_tree_destroy(*s, (*s)->data);
   /* Get rid of the rest */
   free(*s);
   /* Set s to NULL */
   *s = NULL;
}

/****************************************/
/****************************************/

void buzzset_insert(buzzset_t s,
                    const void* data) {
   int added;
   s->data = buzzset_tree_insert(s, s->data, data, &added);
   if(added) s->size++;
}

/****************************************/
/****************************************/

void buzzset_remove(buzzset_t s,
                    const void* data) {
   int removed;
   s->data = buzzset_tree_remove(s, s->data, data, &removed);
   if(removed) s->size--;
}

/****************************************/
/****************************************/

void* buzzset_find(buzzset_t s,
                   const void* data) {
   buzzset_tree_t res = buzzset_tree_find(s, s->data, data);
   if(!res) return NULL;
   return res->data;
}

/****************************************/
/****************************************/

void buzzset_foreach(buzzset_t s,
                     buzzset_elem_funp fun,
                     void* params) {
   buzzset_tree_foreach(s->data, fun, params);
}

/****************************************/
/****************************************/
