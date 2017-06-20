/*
 * $Id: nlmcmcrc.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#ifndef INCLUDED_NLMCM_CRC_H
#define INCLUDED_NLMCM_CRC_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include "nlmcmexterncstart.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif


void
NlmCm__ComputeCRC10Table(void);

nlm_u32
NlmCm__FastCRC10(
    nlm_u8 *data_p,
    nlm_u32 numBytes);

#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif


#endif

