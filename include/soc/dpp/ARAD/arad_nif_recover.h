/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */


/* 
 *  Include file for XLP ports recovery routine
 *
 */

#ifndef __ARAD_NIF_RECOVER_H__
#define __ARAD_NIF_RECOVER_H__

#define ARAD_NIF_RECOVER_FLAGS_XLP_SET(flags)     (flags |= 0x10000000)
#define ARAD_NIF_RECOVER_FLAGS_XLP_GET(flags)     (flags & 0x10000000)
#define ARAD_NIF_RECOVER_FLAGS_XLP_CLEAR(flags)   (flags &= ~0x10000000)

#define ARAD_NIF_RECOVER_FLAGS_CLP_SET(flags)     (flags |= 0x20000000)
#define ARAD_NIF_RECOVER_FLAGS_CLP_GET(flags)     (flags & 0x20000000)
#define ARAD_NIF_RECOVER_FLAGS_CLP_CLEAR(flags)   (flags &= ~0x20000000)

#define ARAD_NIF_RECOVER_F_DONT_RUN_RX_TRAFFIC 0X1

int arad_nif_recover_run_recovery_test(int unit, soc_pbmp_t* pbmp, uint32 flags, int iterations, int is_init_sequence);


#endif

