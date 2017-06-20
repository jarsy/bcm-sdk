
/*******************************************************************

Copyright Redux Communications Ltd.
 
Module Name:

File Name: 
    cls_results.h

File Description:

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

*******************************************************************/

#ifndef cls_results_h
#define cls_results_h
#include "ag_common.h"

/* SUCCESS RESULTS */
#define AG_S_CLS_RELEASED               (AG_S_OK   | (AG_RES_CLS_BASE + 1))
#define AG_S_CLS_FULL					(AG_S_OK   | (AG_RES_CLS_BASE + 2))
#define AG_S_CLS_PATH_EXIST 			(AG_S_OK   | (AG_RES_CLS_BASE + 3))
#define AG_S_CLS_EMPTY					(AG_S_OK   | (AG_RES_CLS_BASE + 4))


/* ERROR RESULTS */
#define AG_E_CLS_TAGGED	    (AG_E_FAIL | (AG_RES_CLS_BASE + 1))
#define AG_E_CLS_FULL		(AG_E_FAIL | (AG_RES_CLS_BASE + 2))
#define AG_E_CLS_AMBIG		(AG_E_FAIL | (AG_RES_CLS_BASE + 3))
#define AG_E_CLS_EMPTY		(AG_E_FAIL | (AG_RES_CLS_BASE + 4))
#define AG_E_CLS_OPEN		(AG_E_FAIL | (AG_RES_CLS_BASE + 5))
#define AG_E_CLS_MISMATCH	(AG_E_FAIL | (AG_RES_CLS_BASE + 6))
#define AG_E_CLS_CONSIST	(AG_E_FAIL | (AG_RES_CLS_BASE + 7))

#define AG_E_CLS_FATAL         		(AG_E_FAIL | (AG_RES_CLS_BASE + 8))

#define AG_E_CLS_MAX_MP_EXCEEDED		(AG_E_FAIL | (AG_RES_CLS_BASE + 9))
#define AG_E_CLS_BAD_MP_ID   		    (AG_E_FAIL | (AG_RES_CLS_BASE + 10))
#define AG_E_CLS_MP_EMPTY   		    (AG_E_FAIL | (AG_RES_CLS_BASE + 11))
#define AG_E_CLS_INVALID_OPER			(AG_E_FAIL | (AG_RES_CLS_BASE + 12))
#define AG_E_CLS_GRP_LINKED			    (AG_E_FAIL | (AG_RES_CLS_BASE + 13))
#define AG_E_CLS_GRP_NOT_LINKED      	(AG_E_FAIL | (AG_RES_CLS_BASE + 14))
#define AG_E_CLS_BAD_GRP_ID      		(AG_E_FAIL | (AG_RES_CLS_BASE + 15))
#define AG_E_CLS_GRP_EMPTY       		(AG_E_FAIL | (AG_RES_CLS_BASE + 16))
#define AG_E_CLS_MAX_GRP_EXCEEDED		(AG_E_FAIL | (AG_RES_CLS_BASE + 17))
#define AG_E_CLS_UNMATCHED_PRGS  	    (AG_E_FAIL | (AG_RES_CLS_BASE + 18))
#define AG_E_CLS_LEAF_NOT_FOUND  		(AG_E_FAIL | (AG_RES_CLS_BASE + 19))
#define AG_E_CLS_ROOT_AMBIG			    (AG_E_FAIL | (AG_RES_CLS_BASE + 20))
#define AG_E_CLS_BAD_PRG				(AG_E_FAIL | (AG_RES_CLS_BASE + 21))
#define AG_E_CLS_BAD_SKIP				(AG_E_FAIL | (AG_RES_CLS_BASE + 22))
#define AG_E_CLS_INVALID_MASK			(AG_E_FAIL | (AG_RES_CLS_BASE + 23))
#define AG_E_CLS_INVALID_TAG			(AG_E_FAIL | (AG_RES_CLS_BASE + 24))
#define AG_E_CLS_INVALID_OPC			(AG_E_FAIL | (AG_RES_CLS_BASE + 25))
#define AG_E_CLS_PATH_CONT			    (AG_E_FAIL | (AG_RES_CLS_BASE + 26))
#define AG_E_CLS_DIFFERENT_TAG   		(AG_E_FAIL | (AG_RES_CLS_BASE + 27))
#endif
