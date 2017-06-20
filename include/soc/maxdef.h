/*
 * $Id: maxdef.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        maxdef.h
 */

#ifndef _SOC_MAXDEF_H_
#define _SOC_MAXDEF_H_

#ifdef BCM_ROBO_SUPPORT
#if SOC_MAX_NUM_BLKS < SOC_ROBO_MAX_NUM_BLKS
#undef  SOC_MAX_NUM_BLKS
#define SOC_MAX_NUM_BLKS           SOC_ROBO_MAX_NUM_BLKS
#endif

#if SOC_MAX_NUM_PORTS < SOC_ROBO_MAX_NUM_PORTS
#undef  SOC_MAX_NUM_PORTS
#define SOC_MAX_NUM_PORTS           SOC_ROBO_MAX_NUM_PORTS
#endif

#if SOC_MAX_MEM_BYTES < SOC_ROBO_MAX_MEM_BYTES
#undef  SOC_MAX_MEM_BYTES
#define SOC_MAX_MEM_BYTES           SOC_ROBO_MAX_MEM_BYTES
#endif
#endif /* BCM_ROBO_SUPPORT */


#endif
