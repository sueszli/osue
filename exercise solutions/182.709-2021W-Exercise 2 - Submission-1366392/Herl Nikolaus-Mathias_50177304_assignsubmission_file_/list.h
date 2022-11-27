#include <stdlib.h>
#include <stdbool.h>

/**
 * list.h
 * 
 * declarations of types and functions for a List implementation
 * rn: 1 type only, i.e. [Int]
 * future: generic, i.e.  [a]
 */

// The macros one needs to put in
// NB1: print_elem comment
// NB2: free_nodes already works for any List??
/*
#define ListA_Module(ElemT)                  \
  NodeA_Type(ElemT) ListA_Type(ElemT)        \
  decl_of(ElemT) decl_empty(ElemT)        \
  decl_add(ElemT) decl_contains(ElemT)     \
  decl_add_unless_dup(ElemT)                      \
  decl_free_nodes(ElemT)       
*/

#define ListOf_Module (ElemT)           \
  NodeA_Type(ElemT) ListA_Type(ElemT)   \
  decl_of(ElemT)

#define NodeA_Type(ElemT)                             \
  typedef struct ElemT##_Node_Strct {                 \
    ElemT node_data;                                  \
    struct ElemT##_Node_Strct* next;                  \
  } ElemT##_Node;

#define ListA_Type(ElemT)                \
  typedef struct ElemT##List_Strct {     \
      ElemT##_Node* head;                \
      int length;                        \
  } ElemT##List;                 

/**
 * Creates a list-node containing the given Elem (and no/NULL 'next')
 * Allocates memory in the process, which should be freed.
 * Returns a pointer to the node.
 */
#define decl_of(ElemT) \
  ElemT##_Node* of_##ElemT ( ElemT e);
#define of(ElemT)                                       \
  ElemT##_Node* of_##ElemT ( ElemT e) {                 \
      ElemT##_Node* n = malloc(sizeof( ElemT##_Node));  \
      n->node_data = e;                                 \
      n->next = NULL;                                   \
      return n;                                         \
  }

#define decl_empty(ElemT) \
  ElemT##List empty_##ElemT ();
#define empty(ElemT)                         \
  ElemT##List empty_##ElemT () {             \
      ElemT##List lst = { .head = NULL,      \
                   .length = 0 };            \
      return lst;                            \
  }

#define decl_add(ElemT)                              \
  void add_##ElemT ( ElemT##List* lst, ElemT e);
#define add(ElemT)                                   \
  void add_##ElemT ( ElemT##List* lst, ElemT e) {    \
      ElemT##_Node* n = of_##ElemT (e);              \
      if (lst->length == 0) {                        \
          lst->head   = n;                           \
          lst->length = 1;                           \
      } else {                                       \
          /* find last node of list*/                \
          ElemT##_Node* cur_n = lst->head;           \
          while (cur_n->next != NULL) {              \
              cur_n = cur_n->next ;                  \
          }                                          \
          cur_n->next = n;                           \
          lst->length++;                             \
      }                                              \
  }

#define decl_contains(ElemT)       \
  bool contains_##ElemT ( ElemT##List* lst, ElemT e);
#define contains(ElemT)                                     \
  bool contains_##ElemT ( ElemT##List* lst, ElemT e) {      \
      if (lst->head == NULL) {                              \
          return false;                                     \
      }                                                     \
      ElemT##_Node* cur_n = lst->head;                      \
      while (cur_n->next != NULL) {                         \
          if (eq_##ElemT (cur_n->node_data, e)) {           \
              return true;                                  \
          }                                                 \
      }                                                     \
      return eq_##ElemT (cur_n->node_data, e);              \
  }


// add the Elem e to the List, unless it's already in List
#define decl_add_unless_dup(ElemT)    \
  void add_unless_dup_##ElemT ( ElemT##List* lst, ElemT e);
#define add_unless_dup(ElemT)                                  \
  void add_unless_dup_##ElemT ( ElemT##List* lst, ElemT e) {   \
      if (contains_##ElemT (lst,e)) {                          \
          return;                                              \
      } else {                                                 \
          add_##ElemT(lst, e);                                 \
      }                                                        \
  }

// 0-indexed
// assumes the list is long enough for the index
#define decl_at_index(ElemT)                                      \
  ElemT ElemT##_at_index( ElemT##List* lst, int index);
#define at_index(ElemT)                                           \
  ElemT ElemT##_at_index( ElemT##List* lst, int index) {          \
    ElemT##_Node* cur_n = lst->head;                              \
    for(int i = 0; cur_n != NULL; cur_n = cur_n->next, i++) {     \
        if (i == index) {                                         \
            return cur_n->node_data;                              \
        }                                                         \
    }                                                             \
    assert(false);                                                \
}

// takes f(ElemT elem, int index) 
#define decl_for_each(ElemT)                                            \
  void for_each_##ElemT ( ElemT##List* lst, void (*f)( ElemT, int));
#define for_each(ElemT)                                                 \
  void for_each_##ElemT ( ElemT##List* lst, void (*f)( ElemT, int)) {   \
      if (lst->length != 0) {                                           \
          ElemT##_Node* cur_n = lst->head;                              \
          for(int i = 0; cur_n != NULL; cur_n = cur_n->next, i++) {     \
              f(cur_n->node_data, i);                                   \
          }                                                             \
      }                                                                 \
  }

// removes and frees the node at 'index' (0-based)
// returns the removed node's data
// precondition: the node in question exists
#define decl_remove_at(ElemT)                                         \
  ElemT remove_##ElemT##_at(ElemT##List* lst, int index);
#define remove_at(ElemT)                                              \
  ElemT remove_##ElemT##_at(ElemT##List* lst, int index) {            \
      if (index >= lst->length) {                                     \
          assert(false);                                              \
      }                                                               \
      ElemT res;                                                      \
      if (index == 0) {                                               \
          ElemT##_Node* to_remove_n = lst->head;                      \
          res = to_remove_n->node_data;                               \
          ElemT##_Node* after_removed_n = to_remove_n->next;          \
          free(to_remove_n);                                          \
          lst->head = after_removed_n;                                \
          lst->length--;                                              \
          return res;                                                 \
      } else {                                                        \
          int i = 1;                                                  \
          ElemT##_Node* before_removed_n = lst->head;                 \
          ElemT##_Node* cur_removal_candidate = lst->head->next;      \
          while(i != index) {                                         \
              before_removed_n = before_removed_n->next;              \
              cur_removal_candidate = cur_removal_candidate->next;    \
              i++;                                                    \
          }                                                           \
          /* cur_removal_candidate is the node to remove*/            \
          res = cur_removal_candidate->node_data;                     \
          ElemT##_Node* after_removed_n = cur_removal_candidate->next;\
          free(cur_removal_candidate);                                \
          before_removed_n->next = after_removed_n;                   \
          lst->length--;                                              \
          return res;                                                 \
      }                                                               \
  }


/**
 * Frees up each of the List's nodes' memory
 * Resets's the List's props, i.e. makes it identical to a new empty()-result
 */
#define decl_free_nodes(ElemT)                     \
  void free_nodes_##ElemT ( ElemT##List* lst);
#define free_nodes(ElemT)                          \
  void free_nodes_##ElemT ( ElemT##List* lst) {    \
      ElemT##_Node* cur_n;                         \
      ElemT##_Node* next_n;                        \
      cur_n = lst->head;                           \
      while (cur_n != NULL) {                      \
          next_n = cur_n->next;                    \
          free(cur_n);                             \
          cur_n = next_n;                          \
      }                                            \
      lst->head = NULL;                            \
      lst->length = 0;                             \
  }  
