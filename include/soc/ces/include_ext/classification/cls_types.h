
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 

File Name: 
    cls_types.h

File Description: 

$Revision: 1.1.2.1 $  - Visual SourceSafe revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    09/05/01
*******************************************************************/
#ifndef CLS_TYPES_H 
#define CLS_TYPES_H 

#include "ag_common.h"

#ifdef BCM_CES_SDK /*Needed bcm sal?*/
#include "bcm_ces_sal.h"
#else
#include "agos/agos.h"
#include "drivers/gpc_drv.h"
#endif   /*BCM_CES_SDK*/
 
/*BCMadd this here*/
/****these definitions can be defined in another place*****/
typedef enum AgMtrType_E
{
    AG_MTR_TYPE_NONE	= 0,
    AG_MTR_BYTES	    = 1,
    AG_MTR_FRAMES  	    = 2,
    AG_MTR_BYTE_FRAME	= 3,
    AG_MTR_TIME	        = 4,
    AG_MTR_BYTE_TIME	= 5,
    AG_MTR_FRAME_TIME	= 6,
    AG_MTR_ALL_TYPES    = 7

} AgMtrType;
/******************************/ 

struct AgFlow_S; /*forward decleration*/

#define   AG_CT_ENT_SIZE        8 /*in word (4 byte) units*/
/* packing is not needed in ARM , the alignement is on 4 words by default*/
typedef struct AgClsTblEntry_S {
    AG_U32 a_val[AG_CT_ENT_SIZE - 1];
	struct AgFlow_S*  p_sw_act;
} AgClsTblEntry;
/*typedef struct AgClsTblEntry_S AgClsTblEntry; */

typedef enum AgCtEntryType_E
{
    AG_CT_DEF_ENTRY,
    AG_CT_ALLOC_FRAME_ENTRY
} AgCtEntryType;

struct AgClsGpcAct_S
{
    AG_U32 n_grp_index;
    AgMtrType n_grp_ctrl; 
};
typedef struct AgClsGpcAct_S AgClsGpcAct;

struct AgClsSwAct_S
{
    AgosQueueInfo		*p_queue; /* NULL to unregister. */
	void				*p_arg;
	AgQFullAction		e_action;
};
typedef struct AgClsSwAct_S AgClsSwAct;

typedef struct AgClsMemUsage_S
{
	AG_U32 n_mp; /*consumes (n_mp * 4) bytes*/
	AG_U32 n_grp; /*consumes (n_grp * 4) bytes*/
	AG_U32 n_buffer_96; 
	AG_U32 n_buffer_40; 
	AG_U32 n_buffer_24; 
	AG_U32 n_buffer_12; 
} AgClsMemUsage;


/* Well known tags */
#define   AG_CLSB_UNCLASSIFIED_TAG        0
#define   AG_CLSB_CLS_OVERRUN             1
#define   AG_CLSB_LATE_CLS                2
#define   AG_CLSB_DEF_ALLOC_FRAME_TAG	  6
#define   AG_CLSB_FIRST_FREE_TAG          7

#endif  /* CLS_TYPES_H */
