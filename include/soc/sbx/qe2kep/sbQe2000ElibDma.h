#if !defined(_SBQE2000_ELIB_DMA_H_)
#define _SBQE2000_ELIB_DMA_H_
/*
 * $Id: sbQe2000ElibDma.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include "sbTypes.h"

#if !defined(SIM) && defined(SB_LINUX) && !defined(NO_QENIC)
#include <sys/ioctl.h>
#include <linux/sbqeioctl.h>
#include <net/if.h>
#include <errno.h>
#endif

#include "glue.h"
#include "sbQe2000Elib.h"

#if !defined(SIM) && defined(SB_LINUX) && !defined(NO_QENIC)
extern int sbQe2kNicDMA(sbhandle hSbQe, struct sb_qe2000_elib_dma_func_params_s *pDmaArg );
#endif

#if defined(SIM) || defined(NO_QENIC)
extern int sbQe2kDMA(sbhandle hSbQe, struct sb_qe2000_elib_dma_func_params_s *pDmaArg );
#endif

int sbQe2kUserSemCreate(void *pUserSemData);
int sbQe2kUserSemGive(int nSemId, void* pUserData);
int sbQe2kUserSemWaitGet(int nSemId, void* pUserSemData, int nTimeOut);

#endif /* _SBQE2000_ELIB_DMA_H_ */
