
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 
    general include files

File Name: 
    ag_types.h

File Description: 
    contains the basic data types of the AG.

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    3/7/00          Gadi Admoni     initial version
    22/1/00         Gadi Admoni     Added AG_ to all types.
                    Yaeli Karni     Add sleep mode (ARM option) to sys global
    22/1/02         Yaeli Karni     Add base-mode to sys-global with relevant enum
    06/02/02        Yaeli Karni     Add RX100 System Messages.
                                    change AgSysGlobals - to allocate the partition info
                                    as part of the structure.
    24/06/02		Yaeli Karni		add AG_PC_PROG support for UI/PC programs which
									dont need RTOS.
									Add general character string pointer structure for 
									support of const char definition in ARM
*******************************************************************/
#ifndef AG_TYPES_H 
#define AG_TYPES_H 


#include "ag_basic_types.h"

#define AG_MAC_ADDRESS_SIZE        6

typedef struct AgMacAddress_S
{
	AG_U8 a_mac[AG_MAC_ADDRESS_SIZE];
} AgMacAddress;

#define AG_IP_ADDRESS_SIZE        4

typedef union AgIpAddr_U
{
	AG_U8  a_ip8[AG_IP_ADDRESS_SIZE]; /* network order */
	AG_U32 n_ip32;	   /* network order */
} AgIpAddr;

/*BCM_CES_SDKadd definition*/
/*
The alignment of the all base addresses that are set 
in the configuration block must be 2KB. 
the ag_align_addr function can be used to force it.
*/
#define AG_REG_BASE_ALIGN   0x800 /*2kb*/


#endif  /* AG_TYPES_H */
