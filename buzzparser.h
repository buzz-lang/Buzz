#ifndef BUZZPARSER_H
#define BUZZPARSER_H

#include "buzzlex.h"

#ifdef __cplusplus
extern "C" {
#endif

   typedef enum {
      BUZZNODE_SCRIPT = 0,
      BUZZNODE_BLOCK,
      BUZZNODE_STAT,
      BUZZNODE_FUNDEF,
      BUZZNODE_IF,
      BUZZNODE_FORLOOP,
      BUZZNODE_WHILELOOP,
      BUZZNODE_CONDITION,
      BUZZNODE_COMPARISON,
      BUZZNODE_EXPRESSION,
      BUZZNODE_PRODUCT,
      BUZZNODE_MODULO,
      BUZZNODE_POWER,
      BUZZNODE_OPERAND,
      BUZZNODE_COMMAND,
      BUZZNODE_IDREF
   } buzzptree_nodetype_e;

   /* A node of the parse tree */
   struct buzzptree_s {
      /* The token */
      buzztok_t tok;
      /* The parent node */
      struct buzzptree_s* parent;
      /* The next sibling node */
      struct buzzptree_s* next;
      /* The first child node */
      struct buzzptree_s* child;
   };
   typedef struct buzzptree_s* buzzptree_t;

   /*
    * Hook function for tree traversal.
    * @param t The tree to traverse.
    * @param p A data structure to carry along while traversing.
    */
   typedef void (*buzzptree_hook_t)(buzzptree_t t, void* p);

   /* The parser state */
   struct buzzparser_s {
      /* The output assembler file name */
      char* fasm;
      /* The lexer */
      buzzlex_t lex;
      /* The last fetched token */
      buzztok_t tok;
      /* The parse tree */
      buzzptree_t ptree;
      /* The current parse tree node */
      buzzptree_t cnode;
   };
   typedef struct buzzparser_s* buzzparser_t;

   /*
    * Creates a new parser.
    * @param fscript The input script file name.
    * @param fasm The output assembler file name.
    * @return The parser state.
    */
   extern buzzparser_t buzzparser_new(const char* fscript,
                                      const char* fasm);

   /*
    * Destroys the parser.
    * @param par The parser.
    */
   extern void buzzparser_destroy(buzzparser_t* par);

   /*
    * Parses the script.
    * @return 1 if successful, 0 in case of error
    */
   extern int buzzparser_parse(buzzparser_t par);

   /*
    * Creates a new parse tree node.
    * @param tok The token to store in the tree node.
    * @return The node state.
    */
   extern buzzptree_t buzzparser_newtree(buzztok_t tok);

   /*
    * Adds a child to a tree.
    * @param t The tree to which the child must be added.
    * @param c The child to add.
    */
   extern void buzzparser_addtreechild(buzzptree_t t,
                                       buzzptree_t c);

   /*
    * Executes the given function for each node in the tree.
    * The tree traversal is post-order depth first.
    * @param t The tree to traverse.
    * @param f The function to execute for each node.
    * @param p A data structure to pass along the traversal.
    */
   extern void buzzparser_fortree(buzzptree_t t,
                                  buzzptree_hook_t f,
                                  void* p);

   /*
    * Destroys the given parse tree.
    * @param t The tree state.
    */
   extern void buzzparser_destroytree(buzzptree_t* t);

#ifdef __cplusplus
}
#endif

#endif
