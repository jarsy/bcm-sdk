/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_LIST_H__
#define __NEMO_LIST_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct AgNdList 
{
    struct AgNdList *p_prev;
    struct AgNdList *p_next;

} AgNdList;

#ifndef BCM_CES_SDK /*IF compiling bcm on another compiler may need to change as bellow*/
#define STATIC 
#endif

  /*
   * Moved to .c file to avoid the STATIC/INLINE def hell
   */
void ag_nd_list_init(AgNdList *p_head);
AG_BOOL ag_nd_list_is_empty(AgNdList *p_head);
void ag_nd_list_del_helper(AgNdList *p_prev, AgNdList *p_next);
void ag_nd_list_del(AgNdList *p_ptr);
void ag_nd_list_add_helper(AgNdList *p_prev, AgNdList *p_next, AgNdList *p_item);
void ag_nd_list_add(AgNdList *p_head, AgNdList *p_ptr);
void ag_nd_list_add_tail(AgNdList *p_head, AgNdList *p_ptr);


/* */
/* iterates over a list */
/* */
#define AG_ND_LIST_FOREACH(pos, head)  for (pos = (head)->p_next; pos != (head); pos = pos->p_next)

/* */
/* iterate over a list safe against removal of list entry */
/* */
#define AG_ND_LIST_FOREACH_SAFE(pos, aux, head) \
    for (pos = (head)->p_next, aux = pos->p_next; pos != (head); pos = aux, aux = pos->p_next)

/* */
/* get the struct for this entry */
/* head:    the &struct AgNdList pointer. */
/* type:    the type of the struct this is embedded in. */
/* member:  the name of the AgNdList within the struct. */
/* */
#define AG_ND_LIST_ENTRY(entry, type, member) \
    ((type *)((char *)(entry)-offsetof(type,member)))

#ifdef __cplusplus
}
#endif

#endif /* __NEMO_LIST_H__ */

