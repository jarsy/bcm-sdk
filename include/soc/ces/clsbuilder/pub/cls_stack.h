/*******************************************************************

Copyright Redux Communications Ltd.
 
Module Name:
    Stack
File Name: 
    stack.h - created 15/11/2000
File Description:
    Stack definition and types.

  $Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

*******************************************************************/
#ifndef STACK_H
#define STACK_H

#include "ag_common.h"
#include "classification/cls_results.h"
#include "clsb_types_priv.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define STACK_EMPTY -1

extern void cls_stack_init( ClsbStack *p_old_stack, ClsbStack *p_new_stack);
extern AgResult cls_stack_push( ClsbStack *p_stack,  ClsbStackElement *p_element);
extern AgResult cls_stack_pop( ClsbStack *p_stack,  ClsbStackElement *p_element);
extern AgResult cls_stack_state( ClsbStack *p_stack);

extern ClsbStackElement* cls_stack_pop_8( ClsbStack *p_stack);
extern AgResult cls_stack_push_8(AG_U32 n_ent_num, 
								 AG_U16 n_rec,
								 AG_U32 n_ent_val, 
								 AG_U8 n_table,
								 ClsbStack *p_stack);

#ifdef __cplusplus
} /*end of extern "C"*/
#endif


#endif
