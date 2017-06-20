/* configNet.h - network configuration header */

/* $Id: configNet.h,v 1.4 2011/07/21 16:14:13 yshtil Exp $
 * Copyright (c) 1999, 2002-2003, 2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01g,21feb06,dtr  SPR #117719
01f,27jan06,dtr  Tidyup - remove fei.
01e,20aug03,dtr  Adding in TSEC config.
01d,31jul03,dtr  Adding fei secondary enet.
01c,07jul03,mil  Added FCC END.
01b,12jun02,kab  SPR 74987: cplusplus protection
01a,10oct99,mtl   written from yk 750 by teamF1
*/

#define INCLUDE_END
#define INCLUDE_NETWORK
#if (defined(INCLUDE_END) && defined(INCLUDE_NETWORK))

#ifndef INCnetConfigh
#define INCnetConfigh

#ifdef __cplusplus
    extern "C" {
#endif

/* includes */

#include <vxWorks.h>
#include "config.h"
#include <end.h>

/* defines */

/*
 * Six devices configurable :
 * primary/secondary/teriary/quaternary/quinary/senary
 * TSEC assumed highest buffer priority followed by FCC then GEI
 */

#define SYS_END_BUFF_LOAN_PRI   1
#define SYS_END_BUFF_LOAN_SEC   2
#define SYS_END_BUFF_LOAN_TER   3
#define SYS_END_BUFF_LOAN_QUA   4
#define SYS_END_BUFF_LOAN_QUI   5
#define SYS_END_BUFF_LOAN_SEN   6


#ifdef INCLUDE_GEI8254X_END
#define END_LOAD_FUNC_QUI       sysGei8254xEndLoad
#define END_BUFF_LOAN_QUI       SYS_END_BUFF_LOAN_QUI
#define END_LOAD_FUNC_SEN       sysGei8254xEndLoad
#define END_BUFF_LOAN_SEN       SYS_END_BUFF_LOAN_SEN
IMPORT END_OBJ * sysGei8254xEndLoad (char *, void *);
#endif


#if (!defined (INCLUDE_MOT_ETSEC_HEND) && !defined(INCLUDE_MOT_TSEC_HEND))
#ifdef INCLUDE_MOT_TSEC_END
IMPORT struct end_object * sysMotTsecEndLoad(char *, void *);

#define END_LOAD_FUNC_PRI       sysMotTsecEndLoad
#define END_BUFF_LOAN_PRI       SYS_END_BUFF_LOAN_PRI
#define END_LOAD_FUNC_SEC       sysMotTsecEndLoad
#define END_BUFF_LOAN_SEC       SYS_END_BUFF_LOAN_SEC

#endif
#endif


/* Secondary network device */
/* secondary devices are currently unsupported in this release */

/* The END_LOAD_STRING is defined empty and created dynamicaly */

#define END_LOAD_STRING ""      /* created in sys<device>End.c */

/* define IP_MAX_UNITS to the actual number in the table. */

#ifndef IP_MAX_UNITS
#define IP_MAX_UNITS            (NELEMENTS(endDevTbl) - 1)
#endif  /* ifndef IP_MAX_UNITS */

#if defined(INCLUDE_PRIMARY_TSEC_END)
IMPORT END_OBJ * END_LOAD_FUNC_PRI (char *, void*);
#endif
#if defined(INCLUDE_SECONDARY_TSEC_END)
IMPORT END_OBJ * END_LOAD_FUNC_SEC (char *, void*);
#endif

#if defined(INCLUDE_PRIMARY_GEI_END)
IMPORT END_OBJ * END_LOAD_FUNC_QUI (char *, void*);
#endif
#if defined(INCLUDE_SECONDARY_GEI_END)
IMPORT END_OBJ * END_LOAD_FUNC_SEN (char *, void*);
#endif

#ifdef INCLUDE_SMEND
#   define SMEND_LOAD_STRING    ""
#   define SMEND_LOAD_FUNC      sysSmEndLoad
IMPORT END_OBJ* SMEND_LOAD_FUNC (char*, void*);
#endif /* INCLUDE_SMEND */

/* each entry will be processed by muxDevLoad() */

#ifdef SYSLIB_CALL
    IMPORT END_TBL_ENTRY endDevTbl [];
#else
END_TBL_ENTRY endDevTbl [] =
    {
#if (!defined(INCLUDE_MOT_ETSEC_HEND) && !defined(INCLUDE_MOT_TSEC_HEND) )

#ifdef INCLUDE_MOT_TSEC_END
#if defined(INCLUDE_PRIMARY_TSEC_END)
    {0, END_LOAD_FUNC_PRI, END_LOAD_STRING, END_BUFF_LOAN_PRI, NULL},
#endif
#if defined(INCLUDE_SECONDARY_TSEC_END)
    {1, END_LOAD_FUNC_SEC, END_LOAD_STRING, END_BUFF_LOAN_SEC, NULL},
#endif
#endif /* INCLUDE_MOT_TSEC_END  */
#endif /* not HEND */
#if defined(INCLUDE_PRIMARY_GEI_END)
    {0, END_LOAD_FUNC_QUI, END_LOAD_STRING, END_BUFF_LOAN_QUI, NULL},
#endif
#if defined(INCLUDE_SECONDARY_GEI_END)
    {1, END_LOAD_FUNC_SEN, END_LOAD_STRING, END_BUFF_LOAN_SEN, NULL},
#endif
#ifdef INCLUDE_SMEND
    { 0, SMEND_LOAD_FUNC, SMEND_LOAD_STRING, 0, NULL, FALSE},
#endif
    {0, END_TBL_END, NULL, 0, NULL} 	/* must be last */
    };
#endif

#endif /* INCnetConfigh */

#ifdef __cplusplus
    }
#endif

#endif /* (defined (INCLUDE_END) && defined (INCLUDE_NETWORK)) */











