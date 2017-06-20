/******************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name:
     flow

File Name:
     flow_types.h

File Description:
	 flow types

$Revision: 1.1.2.1 $ - visual sourcsafe revision number

History:
	Shay Hadar 17/4/2001   Initial Creation

******************************************/
#ifndef FLOW_TYPES_H
#define FLOW_TYPES_H
#ifdef BCM_CES_SDK
#include "bcm_ces_sal.h"
#else
#include "agos/agos.h"
#endif
#include "classification/cls_types.h"

/***  extern definition - for functions prototype ***/


/********************************
 *  General Define
 ********************************/


/* Flow bundle message */
#define AG_MES_FLOW_BUNDLE_IN (AG_BUNDLE_MESSAGE | AG_MES_GR_FLOW | 0x1)
#define AG_MES_FLOW_LIST_BUNDLES_IN (AG_BUNDLE_MESSAGE | AG_MES_GR_FLOW | 0x2)
 /********************************
 *  General Structures
 ********************************/

/*The reserved for software field in the classification table is a pointer
to the struct: AgFlow which hold flow's information.
(In the future it may include more fields).*/

struct AgFlow_S
{


	AgClsSwAct x_cls_sw_act;

	/*place to add one more parameter:*/

};
typedef struct AgFlow_S AgFlow;

#endif  /* FLOW_TYPES_H */
