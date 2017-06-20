/*
 *
 * ====================================================================
 * ==  sbG2EplibCt.c - QE2000 Egress Class Table initialization functions ==
 * ====================================================================
 *
 * WORKING REVISION: $Id: sbG2EplibCt.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * MODULE NAME:
 *
 *     sbG2EplibCt.c
 *
 * ABSTRACT:
 *
 *     QE2000 Egress Class Table initialization functions
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Josh Weissman
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     30-March-2005
 *
 */
#include <sbQe2000Elib.h>
#include "soc/drv.h"
#include "sbG2Eplib.h"
#include "sbG2EplibI.h"
#include "sbG2EplibCt.h"
#include "sbG2EplibTable.h"

/*--------------------------------------------------------------------------
 * Module-Scoped Types and Function Prototypes
 *--------------------------------------------------------------------------*/

/* class resolution table, pattern description */
typedef struct sbG2EplibPatternDesc_s {
    uint8  bValid;
    uint8  ucClass;
    uint8  aucPattern[SB_QE2000_ELIB_NUM_CRT_BITS_K];
    uint32 ulEnMask;
    uint32 ulEnPat;
} sbG2EplibPatternDesc_t;

static uint8
sbG2EplibGetClass(uint32 nIdx);

bool_t
sbG2EplibIsClassValid(uint8 nClass);

static void
sbG2EplibSetupEnableMasks(void);

/*--------------------------------------------------------------------------
 * Class Header Selection Bits - From the shawsheen ucode specification
 * We select the following bits for classification:
 *
 * Note ERH bit numbers are in decimal.
 * Name      #of bits      ERH Bit Number
 * mvtda       2              2, 3
 * oix         4              39, 40, 41, 42
 * mc          1              56
 * mvtdb       2              61, 62
 * ipmc        1              63
 * ----------------------------------
 *            10
 *
 *  ep_cl_hdr_sel[0-9] select the above 10 bits (concatenated) to form
 *  the 10 bit number --
 *  {nocr:1, crmp:1, stkd:1, mcsub:2, ucsub:2, type:2, mc:1}
 *--------------------------------------------------------------------------*/

static SB_QE2000_ELIB_CRT_BIT_SEL_ST asBitSel[SB_QE2000_ELIB_NUM_CRT_BITS_K] =
{
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 2}, /* bit 0 -- mvtda [1:1] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 3}, /* bit 1 -- mvtda [0:0] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 39}, /* bit 2 -- oix [3:3] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 40}, /* bit 3 -- oix [2:2] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 41}, /* bit 4 -- oix [1:1] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 42}, /* bit 5 -- oix [0:0] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 56}, /* bit 6 -- mc */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 61}, /* bit 7 -- mvtdb [2:2] */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 62}, /* bit 8 -- mvtdb [1:1]) */
    /*   {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 63}  bit 9 -- icmp */
    {SB_QE2000_ELIB_CRT_BS_SRC_ERH, 43}  /* bit 9 -- icmp */
};

/*--------------------------------------------------------------------------
 * This array defines patterns that are used to construct
 * the Class Resolution Table (CRT) in the Classifier memory.
 * This table determines which EP Instruction class to execute.
 * This table format allows wildcarding patterns. Population
 * of the array patterns is performed in an 'in-order' fashion,
 * allowing first wild-card match in array to have precedence.
 * The 'Enable mask' is composed from the described pattern for
 * performance reasons.
 *--------------------------------------------------------------------------*/

static sbG2EplibPatternDesc_t s_sbG2EplibCrtPat[] =
  {
    /*----+-------+------------------------------------------+-----+------
      val | class | mvtda13 mvtda12 oix16 oix15 oix14 oix13 mc mvtdb2 mvtdb1 icmp|    Msk | Pat
      ----+-------+------------------------------------------+-----+------*/
    {1,      0,   {'x','x','0','0','0','0','0','x','x','0'},                    0x0,  0x0  }, /* traditional bridging Ucast */
    {1,      5,   {'x','x','0','0','0','0','0','x','x','1'},                    0x0,  0x0  }, /* IPMC: oi is in 4k-8k */
    {1,      2,   {'0','0','x','x','x','x','1','0','0','x'},                    0x0,  0x0  }, /* traditional bridging Mcast */
    {1,      1,   {'x','x','x','x','x','1','0','x','x','x'},                    0x0,  0x0  }, /* logical interface */
    {1,      1,   {'x','x','x','x','1','x','0','x','x','x'},                    0x0,  0x0  }, /* logical interface */
    {1,      1,   {'x','x','x','1','x','x','0','x','x','x'},                    0x0,  0x0  }, /* logical interface */
    {1,      1,   {'x','x','x','x','x','x','0','x','x','x'},                    0x0,  0x0  }, /* logical interface */
    {1,      3,   {'x','1','x','0','x','x','1','x','x','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      3,   {'1','x','x','0','x','x','1','x','x','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      3,   {'x','x','x','0','x','x','1','x','1','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      3,   {'x','x','x','0','x','x','1','1','x','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      4,   {'x','1','x','1','x','x','1','x','x','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      4,   {'1','x','x','1','x','x','1','x','x','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      4,   {'x','x','x','1','x','x','1','x','1','x'},                    0x0,  0x0  }, /* logical interface Mcast */
    {1,      4,   {'x','x','x','1','x','x','1','1','x','x'},                    0x0,  0x0  }, /* logical interface Mcast */


    /* leave as last */
    {0,     15,   {'0','0','0','0','0','0','0','0','0','0'}}, /* unused */
  };

/*--------------------------------------------------------------------------
 * sbG2EplibSetupEnableMasks()
 *
 *  For speed, create enable masks for the individual patterns
 *
 *  returns - nothing
 *--------------------------------------------------------------------------*/
static void
sbG2EplibSetupEnableMasks(void)
{
    uint32 nPat, nBit, nEnMask, nEnPat;

    for (nPat = 0; s_sbG2EplibCrtPat[nPat].bValid; nPat++)
    {
	nEnMask = 0, nEnPat = 0;
	for (nBit = 0; nBit < SB_QE2000_ELIB_NUM_CRT_BITS_K; nBit++)
	{
	    if ((s_sbG2EplibCrtPat[nPat].aucPattern[nBit] == '0') ||
		(s_sbG2EplibCrtPat[nPat].aucPattern[nBit] == '1'))
		nEnMask |= BIT_N(nBit);
	    if (s_sbG2EplibCrtPat[nPat].aucPattern[nBit] == '1')
		nEnPat |= BIT_N(nBit);
	}
	s_sbG2EplibCrtPat[nPat].ulEnMask = nEnMask;
	s_sbG2EplibCrtPat[nPat].ulEnPat = nEnPat;
    }
}

/*--------------------------------------------------------------------------
 * Go through the pattern array, and find the first pattern
 * that matches the table index. Return the associated class
 * instruction stream that the matching packet should enter.
 *--------------------------------------------------------------------------*/
uint8
sbG2EplibGetClass(uint32 nIdx)
{
    uint32 nPat;

    for (nPat = 0; s_sbG2EplibCrtPat[nPat].bValid; nPat++)
    {
	if ((nIdx & s_sbG2EplibCrtPat[nPat].ulEnMask) ==
	    (s_sbG2EplibCrtPat[nPat].ulEnPat))
	    return s_sbG2EplibCrtPat[nPat].ucClass;
    }

    /* Default to class 15 -- Unconditionally Dropped */
    return 0xf;
}

/*--------------------------------------------------------------------------
 * Go through the pattern array, and look for any entry which
 * matches the class. If found return true.
 *--------------------------------------------------------------------------*/
bool_t
sbG2EplibIsClassValid(uint8 nClass)
{
    uint32 nPat;

    for (nPat = 0; s_sbG2EplibCrtPat[nPat].bValid; nPat++)
    {
      if (nClass == s_sbG2EplibCrtPat[nPat].ucClass)
        return TRUE;
    }

    /* Default to bridging (class 0) */
    return FALSE;
}

extern uint32 aulKaEpClassInstr[16][8];
extern uint32 aulKaQeSsEpClassInstr[16][8];

/*--------------------------------------------------------------------------
 * sbG2EplibCITInit()
 *
 *  Setup Class Instruction Table
 *
 *  EpHandle - Pointer to Context Structure for Qe2k Eplib
 *  returns  - status (zero on success)
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibCITInit(sbG2EplibCtxt_pst pEgCtxt)
{
    SB_QE2000_ELIB_CIT_ENTRY_ST sInstr;
    int nClass, nInstr;
    sbElibStatus_et nStatus;
    uint32 (*aulEpClassInst)[8];
    SB_QE2000_ELIB_HANDLE EpHandle = pEgCtxt->EpHandle;

    SB_MEMSET(&sInstr, 0, sizeof(sInstr));

    switch (pEgCtxt->eUcode) {
    case G2EPLIB_UCODE:
        aulEpClassInst = aulKaEpClassInstr;
        break;
    case G2EPLIB_QESS_UCODE:
        aulEpClassInst = aulKaQeSsEpClassInstr;
        break;
    default:
        return SB_ELIB_INIT_CIT_FAIL;
    }

    /* Setup the Class Instruction Table
     * NOTE: all instructions are prepended. */
    for(nClass = 0; nClass < SB_QE2000_ELIB_NUM_CLASSES_K; nClass++)
    {
	for(nInstr = 0; nInstr < SB_QE2000_ELIB_NUM_CLASS_INSTR_K; nInstr++)
	{
	    sInstr.ulInst[nInstr] = aulEpClassInst[nClass][nInstr];
	    sInstr.bInstValid[nInstr] = TRUE;
	    sInstr.bAppend[nInstr] = TRUE;
	}

	nStatus = sbQe2000ElibCITSet(EpHandle, nClass, sInstr);
	if (nStatus)
	    return nStatus;
    }

    return SB_ELIB_OK;
}

/*--------------------------------------------------------------------------
 * sbG2EplibCRTInit()
 *
 *  Setup Class Resolution Table
 *
 *  EpHandle - Pointer to Context Structure for Qe2k Eplib
 *  returns  - status (zero on success)
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibCRTInit(SB_QE2000_ELIB_HANDLE EpHandle)
{
    uint32 nClsSel;
    int nIdx;
    sbElibStatus_et nStatus;

    /* Setup the Classifier Header Bit Selection */
    nStatus = sbQe2000ElibCRTBitSelectSet(EpHandle, asBitSel);
    if (nStatus)
	return SB_ELIB_INIT_CRT_FAIL;

    /* create enable masks for speed */
    sbG2EplibSetupEnableMasks();

    /* Go through all entries in the CRT, and program
     * the associated instruction class. The index into
     * the CRT table is the concatenation of the CIT bits */
    for (nIdx = 0; nIdx < SB_QE2000_ELIB_NUM_CRT_ENTRIES_K; nIdx++)
    {
	nClsSel = sbG2EplibGetClass(nIdx);
	nStatus = sbQe2000ElibCRTSet(EpHandle, nIdx, nClsSel);
	if (nStatus)
	    return nStatus;
    }

    nStatus = sbQe2000ElibCRTPush(EpHandle, TRUE);
    if (nStatus)
	return nStatus;

    return SB_ELIB_OK;
}

/*--------------------------------------------------------------------------
 * sbG2EplibSegmentInit()
 *
 *  Setup the IP memory segment registers. We define the data width,
 *  the segment size, and the starting address of the segment.
 *
 *  pHalCtx  - Pointer to Hal Context Structure for Qe2k Register Set
 *  returns  - status (zero on success)
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibSegmentInit(sbG2EplibCtxt_pst pCtxt)
{
    uint32 ulSegIdx, ulBase, ulBase8, ulEnd, ulWidth, ulLength, ulSegReg;
    void *pHalCtx = pCtxt->pHalCtx;

    SB_ASSERT(pCtxt);

    for (ulSegIdx = 0; ulSegIdx < SEG_SEGMENTS; ulSegIdx++)
    {
	/* get segment information from descriptor table */
	ulBase   = sbG2EplibGetSegBase(pCtxt, ulSegIdx);
	ulEnd    = sbG2EplibGetSegEnd(pCtxt, ulSegIdx);
	ulWidth  = sbG2EplibGetSegWidth(pCtxt, ulSegIdx);

	/* calculate length of segment in 8-bit chunks */
	ulLength = SEG_BSIZE(ulBase, ulEnd);

	/* Base is defined in chunks of 8-bytes */
	ulBase8 = ulBase >> 3;

	/* compose segment register value */
	ulSegReg =
	    (SAND_HAL_SET_FIELD(KA, EP_IP_SEGMENT0, LOG2_SIZE, ulWidth) |
	     SAND_HAL_SET_FIELD(KA, EP_IP_SEGMENT0, LENGTH, ulLength) |
	     SAND_HAL_SET_FIELD(KA, EP_IP_SEGMENT0, BASE, ulBase8));

	/* write out segment register-n */
	SAND_HAL_WRITE_INDEX(pHalCtx, KA, EP_IP_SEGMENT0, ulSegIdx, ulSegReg);
    }

    return SB_ELIB_OK;
}

/*--------------------------------------------------------------------------
 * sbG2EplibClassTypesSet()
 *
 *  Configure the Expected header types on a per-class basis. This is used
 *  by the hardware to make assumptions about what type of headers will be
 *  arriving in each of the instruction streams.
 *
 *  pHalCtx    - Pointer to Hal Context Structure for Qe2k Register Set
 *  eHwClsType - Array of Enumerated Types of the different headers
 *  returns    - status (zero on success)
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibClassTypesSet(void *pHalCtx, sbG2EplibClassHwTypes_t eHwClsType[])
{
    uint32 data, idx;

    /* compose the data register */
    data = 0;
    for (idx = 0; idx < 16; idx++)
	data |= (0x3 & eHwClsType[idx]) << (2 * idx);

    /* write out the data to the register */
    SAND_HAL_WRITE(pHalCtx, KA, EP_IP_HW, data);

    return SB_ELIB_OK;
}

/*--------------------------------------------------------------------------
 * sbG2EplibClassTypesGet()
 *
 *  Retrieve the Expected header types on a per-class basis. This is used
 *  by the hardware to make assumptions about what type of headers will be
 *  arriving in each of the instruction streams.
 *
 *  pHalCtx    - Pointer to Hal Context Structure for Qe2k Register Set
 *  eHwClsType - Array of Enumerated Types of the different headers
 *  returns    - status (zero on success)
 *--------------------------------------------------------------------------*/
sbElibStatus_et
sbG2EplibClassTypesGet(void *pHalCtx, sbG2EplibClassHwTypes_t eHwClsType[])
{
    uint32 data, idx;

    /* read the hw class type register */
    data = SAND_HAL_READ(pHalCtx, KA, EP_IP_HW);

    /* parse the data register */
    data = 0;
    for (idx = 0; idx < 16; idx++)
	eHwClsType[idx] = (data >> (2 * idx)) & 0x3;

    return SB_ELIB_OK;
}
