/*
 * $Id: c3hppc_utils.h,v 1.9 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * c3hppc_utils.h :
 *
 *-----------------------------------------------------------------------------*/
#ifndef _C3HPPC_UTILS_H_
#define _C3HPPC_UTILS_H_

#include <sal/types.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <sal/appl/io.h>
#include <sal/appl/sal.h>
#include <sal/core/time.h>

#define C3HPPC_MIN(a, b)                       ((a) < (b) ? (a) : (b))
#define C3HPPC_MAX(a, b)                       ((a) > (b) ? (a) : (b))


int    c3hppcUtils_poll_field(int nUnit, int nPort, int nRegIndex, int nFieldIndex,
                              uint32 uFieldValue, int nTimeOut, uint8 bClear, sal_time_t *TimeStamp);

int    c3hppcUtils_display_register_notzero(int nUnit, int nPort, int nRegIndex);

uint64 c3hppcUtils_generate_64b_mask(int nHiBit, int nLoBit);
uint32 c3hppcUtils_generate_32b_mask(int nHiBit, int nLoBit);
uint64 c3hppcUtils_64bit_flip( uint64 uuDataIn );
uint64 c3hppcUtils_64b_byte_reflect( uint64 uuDataIn );
uint32 c3hppcUtils_ceil_power_of_2_exp( uint32 uDataIn );
uint32 c3hppcUtils_ceil_power_of_2( uint32 uDataIn );
uint32 c3hppcUtils_floor_power_of_2_exp( uint32 uDataIn );
int    c3hppcUtils_floor_power_of_2_exp_real( double dDataIn );
double c3hppcUtils_negative_exp_to_decimal_value( int nDataIn );
int    c3hppcUtils_first_bit_set( uint32 uDataIn );
int c3hppcUtils_enable_output_to_file( char *pFileName );
int c3hppcUtils_disable_output_to_file(void);
int c3hppcUtils_wc_aer_write( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr, uint16 uData );
uint16 c3hppcUtils_wc_aer_read( int nUnit, int nPhyID, uint16 uLane, uint16 uAddr );

#endif /* _C3HPPC_UTILS_H_ */
