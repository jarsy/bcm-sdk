/*
 * $Id: ramon_fe1600_defs.h,v 1.10 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON_FE1600 DEFS H
 */
 
#ifndef _SOC_RAMON_FE1600_FABRIC_DEFS_H_
#define _SOC_RAMON_FE1600_FABRIC_DEFS_H_

#include <soc/dnxf/cmn/dnxf_defs.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxf/fe1600/fe1600_interrupts.h>
#include <soc/dnxf/fe1600/fe1600_config_defs.h>


#define SOC_RAMON_FE1600_REDUCED_NOF_LINKS 64

#define SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_MAC 16

#define SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_MAC_FSRD 4

#define SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_DCH 2

#define SOC_RAMON_FE1600_REDUCED_NOF_INSTANCES_DCL 2

#define SOC_RAMON_FE1600_REDUCED_BLK_NOF_INSTANCES_BRDC_FMACH 0


#define SOC_RAMON_FE1600_ONLY(unit)         assert(SOC_IS_FE1600(unit))


#endif /*_SOC_RAMON_FE1600_FABRIC_DEFS_H_*/

