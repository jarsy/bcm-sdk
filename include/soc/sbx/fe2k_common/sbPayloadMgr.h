#ifndef _SB_PAYLOAD_MGR_H_
#define _SB_PAYLOAD_MGR_H_
/* --------------------------------------------------------------------------
**
** $Id: sbPayloadMgr.h,v 1.7 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
**
** sbPayloadMgr.h: payload memory manager interface
**
** --------------------------------------------------------------------------*/

typedef struct sbPayloadMgr sbPayloadMgr_t, *sbPayloadMgr_p_t;
typedef struct sbPayloadDesc *sbPayloadHandle_t;

#define SB_PAYLOAD_IS_EMPTY(x) ((x) == NULL)
#define SB_PAYLOAD_EMPTY NULL
#define SB_PAYLOAD_IS_ALIGNED(pmgr) ((pmgr)->aligned)

sbPayloadHandle_t 
sbPayloadAlloc(sbPayloadMgr_p_t pm, unsigned int sz, sbBool_t hiPri, 
	       sbStatus_t *errP);

sbPayloadHandle_t
sbPayloadRecover (sbPayloadMgr_p_t pm, uint32 addr,
                  uint sz, sbStatus_t *errP);

void 
sbPayloadFree(sbPayloadMgr_p_t, sbPayloadHandle_t);

uint32 
sbPayloadSize(sbPayloadMgr_p_t, sbPayloadHandle_t);

/* divide, rounding up */
#define DIV_RU(x,y) (((x)+(y)-1)/(y))

/* size in words */
#define SIZEOFW(t) (DIV_RU(sizeof(t), sizeof(uint32)))

#ifdef DEBUG
void dumpPayloadMgr(struct sbPayloadMgr *pm, int full);
#endif
#endif
