/* sysMotTsecEnd.c - system configuration module for motTsecEnd driver */

/* $Id: sysMotTsecEnd.c,v 1.2 2011/07/21 16:14:17 yshtil Exp $
 * Copyright (c) 2006 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify, or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
01a,31jan06,kds  Modified PHY ADDRS for wrSbc8548 from cds8548/sysMotTsecEnd.c/01c
*/

/*
DESCRIPTION
This is the WRS-supplied configuration module for the VxWorks
motTsecEnd END driver.

INCLUDE FILES:
*/

#include <vxWorks.h>
#include <config.h>
#include <vmLib.h>
#include <stdio.h>
#include <sysLib.h>
#include <logLib.h>
#include <stdlib.h>
#include <string.h>
#include <end.h>
#include <cacheLib.h>
#include <intLib.h>
#include <lstLib.h>
#include <miiLib.h>

#ifdef INCLUDE_MOT_ETSEC_HEND
#include <hwif/vxbus/vxBus.h>
#include <hwif/vxbus/vxbPlbLib.h>
#include <hwif/util/hwMemLib.h>
#include <../src/hwif/h/hEnd/etsecHEnd.h>
#include <../src/hwif/h/hEnd/hEnd.h>
#include <../src/hwif/h/hEnd/hEndParamSys.h>
#else
#include <drv/end/motTsecEnd.h>
#endif



#undef TSEC_SUPPORT_EXTRA_DEVICES

/* debug */

#undef PHY_LOOPBACKTEST

/* defines */
#define MOT_TSEC_PHY_ADDRS_DEV_1   1 
#define MOT_TSEC_PHY_ADDRS_DEV_2   2 
#define MOT_TSEC_PHY_ADDRS_DEV_3   3 
#define MOT_TSEC_PHY_ADDRS_DEV_4   4 

/* PHY's default operating mode */
#define MOT_TSEC_DEF_PHY_MODE      MII_PHY_TBL /* use auto-negotiation table */

/* imports */
#ifdef INCLUDE_MOT_ETSEC_HEND
IMPORT END_OBJ * motEtsecHEndLoad (char *, void *);
#define TSEC_DRV_CTRL MOT_ETSEC_DRV_CTRL
#define MOT_TSEC_PHY_PARAMS MOT_ETSEC_PHY_PARAMS
#define MOT_TSEC_INT_CTRL MOT_ETSEC_INT_CTRL
#define MOT_TSEC_FUNC_TABLE MOT_ETSEC_FUNC_TABLE
#define MOT_TSEC_PHY_STATUS MOT_ETSEC_PHY_STATUS
#define MOT_TSEC_USR_MODE_GMII MOT_ETSEC_USR_MODE_GMII
#define MOT_TSEC_USR_STAT_ENABLE MOT_ETSEC_USR_STAT_ENABLE
#define MOT_TSEC_MAX_DEVS MOT_ETSEC_MAX_DEVS
#define MOT_TSEC_DEV_1 MOT_ETSEC_DEV_1
#define MOT_TSEC_DEV_2 MOT_ETSEC_DEV_2
#define MOT_TSEC_DEV_3 MOT_ETSEC_DEV_3
#define MOT_TSEC_DEV_4 MOT_ETSEC_DEV_4
#define MOT_TSEC_PARAMS MOT_ETSEC_PARAMS
#define MOT_TSEC_USR_MODE_RGMII MOT_ETSEC_USR_MODE_RGMII
#define MOT_TSEC_USR_MODE_MASK MOT_ETSEC_USR_MODE_MASK
#define MOT_TSEC_USR_MODE_RGMII_100 MOT_ETSEC_USR_MODE_RGMII_100
#define MOT_TSEC_USR_MODE_TBI MOT_ETSEC_USR_MODE_TBI
#define MOT_TSEC_USR_MODE_RTBI MOT_ETSEC_USR_MODE_RTBI
#define MOT_TSEC_DEV_NAME MOT_ETSEC_DEV_NAME
#define MOT_TSEC_ADRS_OFFSET_1 MOT_ETSEC_ADRS_OFFSET_1
IMPORT void motEtsecHEndParamSend (UINT32);
#else
IMPORT END_OBJ * motTsecEndLoad (char *);
#endif


IMPORT STATUS    sysEnetAddrGet (UINT32, UCHAR *);
IMPORT STATUS    sysL2ExtWriteBufferAlloc(char *adrs,UINT size,BOOL lock);

/* globals */

/* locals */

/* forward declarations */

LOCAL STATUS sysMiiPhyInit(PHY_INFO *);
LOCAL STATUS sysMiiPhyStatusGet (PHY_INFO *,MOT_TSEC_PHY_STATUS *);
STATUS sysMotEtsecEnetAddrGet (int, UCHAR *);
LOCAL STATUS sysMotEtsecEnetAddrSet (int, UCHAR *);
LOCAL STATUS sysMotEtsecEnetEnable (UINT32, UINT32);
LOCAL STATUS sysMotEtsecEnetDisable (UINT32, UINT32);

/*
* this table may be customized by the user to force a
* particular order how different technology abilities may be
* negotiated by the PHY. Entries in this table may be freely combined
* and even OR'd together.
 */

UINT32 sysMotEtsecNumByUnit[MOT_TSEC_MAX_DEVS] =
{MOT_TSEC_DEV_1, MOT_TSEC_DEV_2
#ifdef TSEC_SUPPORT_EXTRA_DEVICES
,MOT_TSEC_DEV_3, MOT_TSEC_DEV_4
#endif /* TSEC_SUPPORT_EXTRA_DEVICES */
};

LOCAL INT16 sysMotEtsecAnOrderTbl [] =
    {
    MII_TECH_100BASE_TX,    /* 100Base-T */
    MII_TECH_100BASE_T4,    /* 10Base-T */
    MII_TECH_10BASE_T,      /* 100Base-T4 */
    MII_TECH_10BASE_FD,     /* 100Base-T FD*/
    MII_TECH_100BASE_TX_FD, /* 10Base-T FD */
    -1                      /* end of table */
    };

#ifndef INCLUDE_MOT_ETSEC_HEND
/* PhyParms table indexed by unit number */
LOCAL MOT_TSEC_PHY_PARAMS sysMotEtsecPhyParms[MOT_TSEC_MAX_DEVS] =
    {
        {
        MOT_TSEC_PHY_ADDRS_DEV_1,
        MOT_TSEC_DEF_PHY_MODE,
        MII_PHY_DEF_DELAY,
        1,
        (MII_AN_ORDER_TBL *) sysMotEtsecAnOrderTbl,
        },
        {
        MOT_TSEC_PHY_ADDRS_DEV_2,
        MOT_TSEC_DEF_PHY_MODE,
        MII_PHY_DEF_DELAY,
        1,
        (MII_AN_ORDER_TBL *) sysMotEtsecAnOrderTbl,
        }
#ifdef TSEC_SUPPORT_EXTRA_DEVICES
        ,
        {
        MOT_TSEC_PHY_ADDRS_DEV_3,
        MOT_TSEC_DEF_PHY_MODE,
        MII_PHY_DEF_DELAY,
        1,
        (MII_AN_ORDER_TBL *) sysMotEtsecAnOrderTbl,
        },
        {
        MOT_TSEC_PHY_ADDRS_DEV_4,
        MOT_TSEC_DEF_PHY_MODE,
        MII_PHY_DEF_DELAY,
        1,
        (MII_AN_ORDER_TBL *) sysMotEtsecAnOrderTbl
        }
#endif /* TSEC_SUPPORT_EXTRA_DEVICES */
    };


LOCAL MOT_TSEC_INT_CTRL sysMotIntCtrl[MOT_TSEC_MAX_DEVS] =
    {
    {
    EPIC_TSEC1TX_INT_VEC,    /* Transmit Interrupt */
    EPIC_TSEC1RX_INT_VEC,    /* Receive Interrupt */
    EPIC_TSEC1ERR_INT_VEC,    /* Error Interrupt */
    NULL,     /* function to convert INUM to IVEC */
    NULL      /* function to convert IVEC to INUM */
    },
    {
    EPIC_TSEC2TX_INT_VEC,    /* Transmit Interrupt */
    EPIC_TSEC2RX_INT_VEC,    /* Receive Interrupt */
    EPIC_TSEC2ERR_INT_VEC,    /* Error Interrupt */
    NULL,     /* function to convert INUM to IVEC */
    NULL      /* function to convert IVEC to INUM */
    }
#ifdef TSEC_SUPPORT_EXTRA_DEVICES
    ,
    {
    EPIC_TSEC3TX_INT_VEC,    /* Transmit Interrupt */
    EPIC_TSEC3RX_INT_VEC,    /* Receive Interrupt */
    EPIC_TSEC3ERR_INT_VEC,    /* Error Interrupt */
    NULL,     /* function to convert INUM to IVEC */
    NULL      /* function to convert IVEC to INUM */
    },
    {
    EPIC_TSEC4TX_INT_VEC,    /* Transmit Interrupt */
    EPIC_TSEC4RX_INT_VEC,    /* Receive Interrupt */
    EPIC_TSEC4ERR_INT_VEC,    /* Error Interrupt */
    NULL,     /* function to convert INUM to IVEC */
    NULL      /* function to convert IVEC to INUM */
    }
#endif /* TSEC_SUPPORT_EXTRA_DEVICES */
    };
#endif

MOT_TSEC_FUNC_TABLE sysMotEtsecFuncs =
    {
    sysMiiPhyInit,      /* bsp MiiPhy init function */
    NULL,               /* Interrupt End Driver function called by BSP */
    sysMiiPhyStatusGet, /* status call back */
    NULL,               /* BSP BYTE Read function called by BSP */
    NULL,               /* BSP BYTE Write function called by BSP */
    sysMotEtsecEnetAddrGet,    /* Driver call back to get the Ethernet address */
    sysMotEtsecEnetAddrSet,    /* Driver call back to set the Ethernet address */
    sysMotEtsecEnetEnable,     /* Driver call back to enable the ENET interface */
    sysMotEtsecEnetDisable,     /* Driver call back to disable the ENET interface */
    NULL
    };

MOT_TSEC_PARAMS sysMotEtsecParms =
    {
    NULL,  /* Buffer pointer for allocated buffer space */
    0,     /* Buffer pool size */
    NULL,  /* Descriptor Base Address */
    0,     /* Descriptor Size */
    32,    /* Number of Receive Buffer Descriptors  */
    32    /* Number of Transmit Buffer Descriptors */
    };

#ifndef INCLUDE_MOT_ETSEC_HEND

#ifdef TSEC_EXT_PARMS_USED

/* TSEC hardware defaults */
LOCAL MOT_TSEC_EXT_PARAMS sysMotEtsecExtParms =
    {
    0, /* TSEC specific user bit flags */
    0, /* TSEC specific user reg flags */
    0, /* Ethernet Minimum Frame Length */
    0, /* Ethernet Maximum Frame Length */

       /* TSEC Specific Device Parameters */
    0, /* ext + pause time value */
    0, /* Ten Bit Interface physical address */

        /* Tx FIFO Manipulation */
    0,  /* UINT32 fifoTxTheshold; */
    0,  /* UINT32 fifoTxStarve; */
    0,  /* UINT32 fifoTxStarveShutOff; */

        /* MAC specific Parameters */
    {0,0,0,0,0,0,0,0}, /* initial individual addresses [8] */
    {0,0,0,0,0,0,0,0}, /* initial group addresses [8] */

    0,  /* UINT32 macPreambleLength; */
    0,  /* UINT32 macIfMode; */

    0,  /* UINT32 macIpgifgNbbipg1; */
    0,  /* UINT32 macIpgifgNbbipg2; */
    0,  /* UINT32 macIpgifgMifge; */
    0,  /* UINT32 macIpgifgBbipg; */

        /* MAC half duplex specific parameters */
    0,  /* UINT32 macHalfDuplexAltBebTruncation; */
    0,  /* UINT32 macHalfDuplexRetryMaximum; */
    0,  /* UINT32 macHalfDuplexCollisionWindow; */
    0,  /* UINT32 miiMgmtClockSelect; */
    0,  /* UINT32 phyAddress; */

        /* Misc */
    0,  /* UINT32 extL2Cache; */
    0,  /* UINT32 bdL2Cache;  */
    0,  /* UINT32 dmaExtLength; */
    0   /* UINT32 dmaExtIndex; */
    };
#endif

#endif

#ifdef INCLUDE_MOT_ETSEC_HEND
#   ifdef INCLUDE_HEND_PARAM_SYS
#define MAC_ADDR_LEN 6



/******************************************************************************
*
* sysParamsSend
*
*/

void sysParamSend
    (
    UINT32 unit
    )
    {
    UINT32 value;
    char * pName;
    int loop=0;
    char* str=NULL;
    HEND_RX_QUEUE_PARAM * pRxParam;

    pName = "motetsecHEnd";
    value = unit;

    str=malloc(0x20);

    for(loop=0;loop< _c_(NUM_RX_QUEUES);loop++)
	{

	pRxParam = malloc(sizeof (HEND_RX_QUEUE_PARAM));
	bzero((char *)pRxParam,(sizeof (HEND_RX_QUEUE_PARAM)));
	pRxParam->jobQueId = NULL;
	pRxParam->priority = loop;

	sysHEndParamAttach(pRxParam,HEND_RX_Q_PARAM,HEND_VOID_PTR,pName,unit);

	}
    free(str);


    }

#endif /* HEND_PARAM_SYS */
#endif /* MOT_ETSEC_HEND */

/***********************************************************************
*
* sysMotEtsecHEndLoad - load an instance of the motEtsecHEnd driver
*
* This routine loads the motETsecHEnd driver with proper parameters.
*
* The END device load string formed by this routine is in the following
* format.
* <unit>:<tsecAddrs>:<tsecNum>:<MAC Address>:<MOT_TSEC_FUNC_TABLE>:
* <MOT_TSEC_PARAMS>:<MOT_TSEC_EXT_PARAMS>
*
* .IP <unit>
* The unit number passed by the Mux.
* .IP <CCSBAR>
* The MPC85xx Internal memory base address. eg.0xfe000000
* .IP <tsecNum>
* This Tsec's physical port, 0 or 1. Not the same as the unit number.
* .IP <MAC ADDRESS>
* This TSEC's MAC address eg. 00-0a-1e-12-34-56
* .IP <usrFlags>
* User Init Flags
* .IP <MOT_TSEC_PHY_PARAMS>
* PHY Init parameters
* .IP <MOT_TSEC_FUNC_TABLE>
* Structure Address of external and driver functions
* .IP <MOT_TSEC_PARAMS>
* Structure Address of Buffer Allocation Parameters
* .IP <MOT_TSEC_EXT_PARAMS>
* Structure Address of TSEC specific parameters
*
* This routine only loads and initializes one instance of the device.
* If the user wishes to use more than one motETsecHEnd devices, this routine
* should be changed.
*
* RETURNS: pointer to END object or NULL.
*
* SEE ALSO: motETsecHEndLoad ()
*/
#ifdef INCLUDE_MOT_ETSEC_HEND

END_OBJ * sysMotEtsecHEndLoad
    (
    char * pParamStr,   /* ptr to initialization parameter string */
    void * pBusDev       /* unused optional argument */
    )
    {
    END_OBJ * pEnd;

    UINT32 unit;
    char *  tok;             /* an initString token */
    char *  holder = NULL;   /* points to initString fragment beyond tok */

#ifdef INCLUDE_L2_CACHE
    sysMotEtsecFuncs.extWriteL2AllocFunc = sysL2ExtWriteBufferAlloc;
#endif

    if (strlen (pParamStr) == 0)
        {
       /*
        * muxDevLoad() calls us twice.  If the string is
        * zero length, then this is the first time through
        * this routine.
        */

        pEnd = (END_OBJ *) motEtsecHEndLoad  (pParamStr, pBusDev);
        }
    else
        {
        /*
        * On the second pass through here, we actually create
        * the initialization parameter string on the fly.
        * Note that we will be handed our unit number on the
        * second pass and we need to preserve that information.
        * So we use the unit number handed from the input string.
        */

        /* extract the unit number */
        tok = strtok_r (pParamStr, ":", &holder);
        if (tok == NULL)
           return (NULL);

        unit = (int) strtoul (tok, NULL, 16);

	motEtsecHEndParamSend (unit);
	sysParamSend(unit);

        if ((pEnd = (END_OBJ *) motEtsecHEndLoad  (pParamStr, pBusDev)) == (END_OBJ *)NULL)
            logMsg ("Error: motEtsecHEndLoad  failed to load driver\n", 0, 0, 0, 0, 0, 0);

        }

    return pEnd;
    }


#else

END_OBJ * sysMotEtsecEndLoad
    (
    char * pParamStr,   /* ptr to initialization parameter string */
    void * unused       /* unused optional argument */
    )
    {
    END_OBJ * pEnd;
    char   paramStr [300];
    UINT32 unit, usrFlags;
    char *  tok;             /* an initString token */
    char *  holder = NULL;   /* points to initString fragment beyond tok */
    char enetAddr[8];
    /*
    * <CCSBAR>:
    * <tsecNum>:
    * <MAC Address>:
    * <UsrFlags>:
    * <MOT_TSEC_PHY_PARAMS>:
    * <MOT_TSEC_FUNC_TABLE>:
    * <MOT_TSEC_PARAMS>:
    * <MOT_TSEC_EXT_PARAMS>
    * Note that unit string is prepended by the mux, so we
    * don't put it here.
    */

#ifdef INCLUDE_L2_CACHE
    sysMotEtsecFuncs.extWriteL2AllocFunc = sysL2ExtWriteBufferAlloc;
#endif

    if (strlen (pParamStr) == 0)
        {
        /*
        * muxDevLoad() calls us twice.  If the string is
        * zero length, then this is the first time through
        * this routine.
        */

        pEnd = (END_OBJ *) motTsecEndLoad  (pParamStr);
        }
    else
        {
        /*
        * On the second pass through here, we actually create
        * the initialization parameter string on the fly.
        * Note that we will be handed our unit number on the
        * second pass and we need to preserve that information.
        * So we use the unit number handed from the input string.
        */

        /* extract the unit number */
        tok = strtok_r (pParamStr, ":", &holder);
        if (tok == NULL)
           return (NULL);

        unit = (int) strtoul (tok, NULL, 16);


	sysMotEtsecEnetAddrGet(unit,enetAddr);


        if (unit > MOT_TSEC_MAX_DEVS )
            return (NULL);

	if(unit>1)
	    {
	    usrFlags = MOT_TSEC_USR_STAT_ENABLE | MOT_TSEC_USR_MODE_RGMII;
	    }
	else
	    {
	    /* enable stats and put into GMII mode */
	    usrFlags = MOT_TSEC_USR_STAT_ENABLE | MOT_TSEC_USR_MODE_GMII;
	    }

        /* finish off the initialization parameter string */
        sprintf (paramStr, "%d:0x%x:0x%x:%02x-%02x-%02x-%02x-%02x-%02x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                 unit,
                 CCSBAR,
                 sysMotEtsecNumByUnit[unit],
                 enetAddr[5],enetAddr[4],enetAddr[3],enetAddr[2],enetAddr[1],enetAddr[0],
                 usrFlags,
                 &sysMotEtsecPhyParms[unit],
                 &sysMotEtsecFuncs,
                 &sysMotEtsecParms,
                 (UINT32)NULL,
				 (UINT32)NULL,
				 (UINT32)&sysMotIntCtrl[unit]
                 );

        if ((pEnd = (END_OBJ *) motTsecEndLoad  (paramStr)) == (END_OBJ *)NULL)
            logMsg ("Error: motTsecEndLoad  failed to load driver\n", 0, 0, 0, 0, 0, 0);
#if 0
        else
            logMsg ("Load successful: 0x%08x\n", (int)pEnd, 0, 0, 0, 0, 0);
#endif

        }

    return pEnd;
    }
#endif



/***********************************************************************
*
* sysMiiPhyStatusGet - Return hardware-dependent PHY status
*
* This routine returns the status for all PHY attributes that are
* hardware dependent.
*
* RETURNS: ERROR or OK.
*/

LOCAL STATUS sysMiiPhyStatusGet
    (
    PHY_INFO            * pPhyInfo,
    MOT_TSEC_PHY_STATUS * pStatus
    )
    {
    UINT16 miiStat;
    int    retVal = OK;

    if (sysMotEtsecFuncs.miiPhyRead != NULL)
        {
        retVal = sysMotEtsecFuncs.miiPhyRead(pPhyInfo->phyAddr,17,&miiStat);
        if (retVal != OK)
            return ERROR;

        pStatus->duplex = (miiStat&0x2000)?0:1;
        pStatus->speed  = (miiStat&0xc000)>>13;
        }
    return retVal;
    }

#ifdef TSEC_MII_INT_SUPPORT
/***********************************************************************
*
* sysMiiInt - MII interrupt service routine
*
* This routine checks if the link up or down and update a flag
*
* RETURNS: None.
*/

LOCAL void sysMiiInt
    (
    PHY_INFO * pPhyInfo
    )
    {
    UINT16 miiIntStatusReg;
    UINT32 event;

    /* Clear MII interrupt by reading Int status reg */
    if (sysMotEtsecFuncs.miiPhyRead != NULL)
        {
        sysMotEtsecFuncs.miiPhyRead (pPhyInfo->phyAddr,20,&miiIntStatusReg);
        event = miiIntStatusReg;
        if (sysMotEtsecFuncs.miiPhyInt != NULL)
            {
            sysMotEtsecFuncs.miiPhyInt (pPhyInfo->pDrvCtrl, event);
            }
        }
    return;
    }
#endif
/***********************************************************************
*
* sysMiiPhyInit - initialize and configure the PHY devices
*
* This routine scans, initializes and configures the PHY device.
*
* RETURNS: OK, or ERROR.
*/

LOCAL STATUS sysMiiPhyInit
    (
    PHY_INFO * pPhyInfo
    )
    {

#ifndef INCLUDE_MOT_ETSEC_HEND
    UINT16 modeSet;
    UINT16 miiPhyEnableReg;

    /* Initialisation of Phy performed in driver */

    switch(((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->userFlags & MOT_TSEC_USR_MODE_MASK)
	{
	case MOT_TSEC_USR_MODE_GMII:
	    modeSet = 0x0000;
	    break;
	case MOT_TSEC_USR_MODE_RGMII:
	case MOT_TSEC_USR_MODE_RGMII_100:
	    modeSet = 0x1200;
	    break;
	case MOT_TSEC_USR_MODE_TBI:
	    modeSet = 0x2000;
	    break;
	case MOT_TSEC_USR_MODE_RTBI:
	    modeSet = 0x3000;
	    break;
	default:
	    modeSet = 0x0000;
	    break;
	}



    /* Reister 23 set the interface RGMII/GMII/TBI/RBTI */

    if (sysMotEtsecFuncs.miiPhyRead != NULL)
	{


        sysMotEtsecFuncs.miiPhyRead(pPhyInfo->pDrvCtrl,
				   ((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->phyInit->phyAddr,23,
				   &miiPhyEnableReg);

	miiPhyEnableReg &= ~0xff00;
	miiPhyEnableReg |= modeSet;

	sysMotEtsecFuncs.miiPhyWrite(pPhyInfo->pDrvCtrl,
				    ((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->phyInit->phyAddr, 23,
				    miiPhyEnableReg);



        sysMotEtsecFuncs.miiPhyRead(pPhyInfo->pDrvCtrl,
                                   ((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->phyInit->phyAddr,23,
				   &miiPhyEnableReg);

#ifdef PHY_LOOPBACKTEST
	if(((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->phyInit->phyAddr == 0)
	    {
	    sysMotEtsecFuncs.miiPhyRead(pPhyInfo->pDrvCtrl,
				       ((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->phyInit->phyAddr,0,
				       &miiPhyEnableReg);

            miiPhyEnableReg |= 0x4000;
            sysMotEtsecFuncs.miiPhyWrite(pPhyInfo->pDrvCtrl,
                                        ((TSEC_DRV_CTRL*)pPhyInfo->pDrvCtrl)->phyInit->phyAddr,0 ,
                                        miiPhyEnableReg);
            }
#endif

        }


#endif
    return OK;
    }

/***********************************************************************
*
* sysMotEtsecEnetEnable - enable the MII interface to the TSEC controller
*
* This routine is expected to perform any target specific functions required
* to enable the Ethernet device and to connect the MII interface to the TSEC.
*
* RETURNS: OK
*/

LOCAL STATUS sysMotEtsecEnetEnable
    (
    UINT32  immrVal,    /* base address of the on-chip RAM */
    UINT32  tsecNum     /* TSEC being used */
    )
    {
    int intLevel;

    intLevel = intLock ();

    intUnlock (intLevel);

    return(OK);
    }

/***********************************************************************
*
* sysMotEtsecEnetDisable - disable MII interface to the TSEC controller
*
* This routine is expected to perform any target specific functions required
* to disable the Ethernet device and the MII interface to the TSEC
* controller.  This involves restoring the default values for all the Port
* B and C signals.
*
* RETURNS: OK, always.
*/

LOCAL STATUS sysMotEtsecEnetDisable
    (
    UINT32  immrVal,    /* base address of the on-chip RAM */
    UINT32  tsecNum  /* TSEC being used */
    )
    {
    int intLevel;

    intLevel = intLock ();

    /* Disable the interrupt */
    intUnlock (intLevel);

    return(OK);
    }


/***********************************************************************
*
* sysMotEtsecEnetAddrGet - get the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual TSEC device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK
*/
STATUS sysMotEtsecEnetAddrGet
    (
    int     unit,
    UCHAR * pAddr
    )
    {

    sysNetMacNVRamAddrGet ("mottsec",unit,pAddr,8);

    return(OK);
    }

/***********************************************************************
*
* sysMotEtsecEnetAddrSet - Set the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual TSEC device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK
*/
LOCAL STATUS sysMotEtsecEnetAddrSet
    (
    int     unit,
    UCHAR * pAddr
    )
    {

    return OK;
    }

