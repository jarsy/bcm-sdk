/*
 * $Id: bm9600_diags.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == bm9600_diags.h - Polaris Diagnostics      ==
 */

#ifndef _BM9600_DIAGS_H_
#define _BM9600_DIAGS_H_

#include <soc/sbx/hal_pl_auto.h>
#include <bcm/port.h>
#include "sbx_diags.h"

/* theoretical maximum is 96 */
#define MAX_QE_NODES 8
#define MAX_LINK_ENABLES 20 /* includes SCI links */

#define PRBS_TO_PL 0
#define PRBS_TO_QEXX 1

#define PRBS_TO_PL 0
#define PRBS_TO_QEXX 1

/* to do move dev_cmd_rec from sbx.c to sbx.h */
/* make extern in sbx.h */
#if 1
typedef struct my_dev_cmd_rec_s {
    int cmd;
    int unit;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
    int arg6;
} my_dev_cmd_rec;
#endif

int sbBme9600DiagsPrbsTest(sbxDiagsInfo_t *pDiagsInfo);
int sbBme9600DiagsSCIPrbsTest(sbxDiagsInfo_t *pDiagsInfo);
int sbBme9600DiagsSFIPrbsTest(sbxDiagsInfo_t *pDiagsInfo,
			      my_dev_cmd_rec *pcmd,
			      int qexx_unit);

int sbBme9600DiagsFindAllQExx(int unit,
				   int qexx_lcm_ids[]);

int sbBme9600DiagsStartPrbs(int rx_unit,
			    int rx_port,
			    int tx_unit,
			    int tx_port,
			    uint8 bForceError,
			    bcm_port_prbs_t prbs_type,
			    int link,
			    int *prbs_status);

int sbBme9600DiagsStopPrbs(int rx_unit,
			   int rx_port,
			   int tx_unit,
			   int tx_port);

int sbBme9600DiagsGetQExxXbarMapping(my_dev_cmd_rec *p,
				     int qexx_modid, 
				     int nQePhysicalPort);


int sbBme9600DiagsRegTest(sbxDiagsInfo_t *pDiagsInfo);
extern void polaris_sigcatcher(int signum);
#endif /* _BM9600_DIAGS_H_ */
