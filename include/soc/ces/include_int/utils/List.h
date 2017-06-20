
/*******************************************************************

Copyright Redux Communications Ltd.
 
Module Name:
    Utils
File Name: 
    List.h
File Description:
    A doubly linked list.
    To define an element that can be iserted to the list, define the
    ListElement as its first member.

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 
*******************************************************************/

#ifndef List_h
#define List_h

#include "ag_common.h"

#ifdef DECLARE_LIST
    #define EXTERN
#else
    #define EXTERN extern
#endif

struct List_S;

/* Doubly linked list element */
struct ListElement_S
{
	struct ListElement_S *p_next;
	struct ListElement_S *p_prev;
/*    struct List_S        *p_list; // the list it belongs to. */

};
typedef struct ListElement_S ListElement;

/* Doubly linked list */
struct List_S
{
    ListElement  *p_head; /* pointer to the first element */
	ListElement  *p_tail; /* pointer to the last element */
	AG_U32 	    n_size; /* the number of elements  the list */
};
typedef struct List_S List;

#ifdef __cplusplus
extern "C"
{
#endif 

/*------------------------------**
** Doubly linked list functions **
**------------------------------*/

/*!!!!!! the init function must be called before using the list !!!!!!!!!!!!!!*/
EXTERN AgResult list_init ( List *p_list);

EXTERN AG_U32 list_get_size ( List* p_list);

EXTERN void list_insert_head ( List *p_list,  ListElement *p_element);

EXTERN void list_insert_tail ( List *p_list,  ListElement *p_element);

/*
inserts the element pointed by p_new_element after element 
pointed by p_element 
*/
EXTERN void list_insert_after ( List *p_list,
                             ListElement *p_element,
                             ListElement *p_new_element);

/* 
removes the first element from the list and returns a pointer to it.
if the list is empty returns NULL.
*/
EXTERN ListElement* list_remove_head ( List* p_list);

/*
removes the last element from the list and returns a pointer to it.
if the list is empty returns NULL.
*/
EXTERN ListElement* list_remove_tail ( List* p_list);

/*removes the specified element from the list*/
EXTERN void list_unlink ( List* p_list,  ListElement *p_element);

/* 
returns the head pointer without removing it.
if the list is empty returns NULL.
*/
EXTERN ListElement* list_head ( List* p_list);

/* 
returns the tail pointer without removing it 
if the list is empty returns NULL.
*/
EXTERN ListElement* list_tail( List* p_list);

/*
returns a pointer to the element that follows p_element 
without removing it.
if p_element is the last entry returns NULL
*/
EXTERN ListElement* list_next( List *p_list, ListElement *p_element);

/*
returns a pointer to the element that precedes p_element 
without removing it
if p_element is the first entry returns NULL
*/
EXTERN ListElement* list_prev ( List *p_list,
                     ListElement *p_element);

EXTERN AG_BOOL list_is_empty( List* p_list);

/*
The function checks the list to ensure that it is a valid doubly linked list,
with the same number of elements that the list header specifies.
*/
EXTERN AgResult list_test( List* p_list);

#ifdef __cplusplus
} /*end of extern "C"*/
#endif


#undef EXTERN

#endif /* List_h */
