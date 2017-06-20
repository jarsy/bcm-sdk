/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/******************************************************************************
 * $Id: sbCrc32.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 ******************************************************************************/

#ifndef _SB_CRC32_H_
#define _SB_CRC32_H_

/******************************************************************************
 *
 * Copyright (c) 2006, Broadcom Corporation All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 *
 * $Id: sbCrc32.h,v 1.6 Broadcom SDK $
 *
 * sbCrc32.h : hash function
 *
 ******************************************************************************/
/* 
 * Hash routine to mimic the LRP hash instruction.
 * x: 32 bit input value
 * return: value after 32 steps in an LFSR seeded with x
 */
uint32 sbCrc32(uint32 x);

/*
 * Reverse Hash routine.
 * x: 32 bit input value
 * return: value after 32 reverse steps seeded with x
 */
uint32 sbCrc32_reverse(uint32 x);

/*
 * Routine to mimic the LRP popcnt instruction.
 * x: 32 bit value
 * return: the number of bits set in x
 */
uint32 sbPopcnt(uint32 x);

#endif
