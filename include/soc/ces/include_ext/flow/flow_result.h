/******************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name:
     flow

File Name:
     flow_result.h

File Description:
	 flow result

$Revision: 1.1.2.1 $ - visual sourcsafe revision number

History:
	Shay Hadar 10/5/2001   Initial Creation

******************************************/
#ifndef FLOW_RESULT_H
#define FLOW_RESULT_H

#include "ag_common.h"


#define AG_FLOW_S_DISCARDED (AG_S_OK | (AG_RES_FLW_BASE + 0x001))
#define AG_FLOW_S_OVERWRITE (AG_S_OK | (AG_RES_FLW_BASE + 0x002))
#define AG_FLOW_E_OVERWRITE (AG_E_FAIL | (AG_RES_FLW_BASE + 0x003))


#endif  /* FLOW_RESULT_H */



