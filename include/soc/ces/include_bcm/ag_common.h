
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 

File Name: 
    ag_common.h

File Description: 

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    30/07/00        Gadi Admoni     Created

*******************************************************************/
#ifndef AG_COMMON_H 
#define AG_COMMON_H 

#include "ag_flags.h"
#include "ag_types.h"
#include "ag_results.h"
#include "ag_macros.h"


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef AG_MNT
	/* definition of redux version which is defined in .s file - to be available in
		NT environment */
	extern AG_U8 agReduxProductVer[];
#endif
#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#endif  /* AG_COMMON_H */
