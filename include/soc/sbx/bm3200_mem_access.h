/*
 * $Id: bm3200_mem_access.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains definitions for BM3200 memory access.
 */

#ifndef _SOC_SBX_BM3200_MEM_ACCESS_H
#define _SOC_SBX_BM3200_MEM_ACCESS_H


#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbWrappers.h>

#define BM3200_STATUS_OK         0
#define BM3200_STATUS_BAD_UNIT  -1
#define BM3200_STATUS_BAD_READ  -2
#define BM3200_STATUS_BAD_WRITE -3
#define BM3200_STATUS_BAD_FIELD -4


typedef struct bm3200MemConfigRec_s {
      char memname[30];
      int  memindex;
      int  rangemax;
      int  instances;
} bm3200MemConfigRec;

extern void sbBm3200ShowMemNames(void);

/*
 * include other specific structs defined for zframes
 */
#include "sbZfFabBm3200BwBwpEntry.hx"
#include "sbZfFabBm3200BwDstEntry.hx"
#include "sbZfFabBm3200BwLthrEntry.hx"
#include "sbZfFabBm3200BwNPC2QEntry.hx"
#include "sbZfFabBm3200BwPrtEntry.hx"
#include "sbZfFabBm3200BwQ2NPCEntry.hx"
#include "sbZfFabBm3200BwQlopEntry.hx"
#include "sbZfFabBm3200BwQltEntry.hx"
#include "sbZfFabBm3200BwWatEntry.hx"
#include "sbZfFabBm3200BwWctEntry.hx"
#include "sbZfFabBm3200BwWdtEntry.hx"
#include "sbZfFabBm3200BwWstEntry.hx"
#include "sbZfFabBm3200NextPriMemEntry.hx"
#include "sbZfFabBm3200NmEmapEntry.hx"
#include "sbZfFabBm3200PriMemEntry.hx"

enum {
    BM3200_PTBWBWP=0,
    BM3200_PTBWPRT,
    BM3200_PTBWWCT,
    BM3200_PTBWWDT,
    BM3200_PTBWWST,
    BM3200_PTNEXTPRIMEM,
    BM3200_PTNMEMAP,
    BM3200_PTPRIMEM,
    BM3200_MEM_MAX_INDEX,
    /* Cmode only memories, no need to support */
    BM3200_PTBWDST,
    BM3200_PTBWLTHR,
    BM3200_PTBWNPC2Q,
    BM3200_PTBWQ2NPC,
    BM3200_PTBWQLOP,
    BM3200_PTBWQLT,
    BM3200_PTBWWAT
};

#endif  /* !_SOC_SBX_BM3200_MEM_ACCESS_H */
