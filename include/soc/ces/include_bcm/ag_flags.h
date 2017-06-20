
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 
    AG general include files

File Name: 
    ag_flags.h 
 
File Description: 
    Contains system wide compilation flags.

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    11/02/01        Gadi Admoni     Created
    17/06/01        Yaeli Karni     Combine AG_SIM with AG_MNT. 
                                    Add AG_VERILOG for compilation under Verilog 
                                    which can not use the Nucleus (currently).
    4/07/01         Yaeli Karni     Change AG_VERILOG to NO_OS_SUPPORT 
    4/07/01         Yaeli Karni     REMOVE AG_MNT definition - it comes from the makefile
	NOV-12-01		Yehuda Mozes	remove AG_PREGx_ON_RPB1_D to project setup
*******************************************************************/
#ifndef AG_FLAGS_H 
#define AG_FLAGS_H 

/*AG_MNT specifies that the environment is MNT.
When AG_MNT is not defined it means the environment is the ARM compiler
This is set automatically by the makefile .*/
#if defined AG_MNT && !defined AG_OFFL_RULES
	#define AG_SIM      /* for working with the AG simulator - This is only under MNT environment... */
#endif /* AG_MNT */

#if defined(AG_MNT) || defined(AG_PC_PROG) || defined AG_LNX
	#define AG_LITTLE_ENDIAN /*since running on intel processor*/
#endif /* AG_MNT || AG_PC_PROG */

/* if you want to work with AG_DBG, define it via the environment flags */
/*when using AG_DBG, memory monitoring is also enabled.*/
#ifdef AG_DBG
    #ifndef AG_MEM_MONITOR
        #define AG_MEM_MONITOR
    #endif
#endif

/* SET internal AG_CHECK_PARAMS with 'positive' logic based on 
    the system wide NO_VALIDATION flag (when NO_VALIDATION flag is on, 
    parameters checking is disabled ) */
#ifdef NO_VALIDATION
#define AG_CHECK_PARAMS 0
#else
#define AG_CHECK_PARAMS 1
#endif


#endif  /* AG_FLAGS_H */
