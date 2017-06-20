/*
 * $Id: dcmn_ser_correction.h, v1 16/06/2014 09:55:39 itamar $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement header for ser correction.
 */

#ifndef _DCMN_SER_CORRECTION_H_
#define _DCMN_SER_CORRECTION_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/defs.h>
#include <soc/dcmn/error.h>

/*************
 * FUNCTIONS *
 *************/

soc_error_t calc_ecc(
        int unit,
        const uint32 data_len_bits,
        const uint32 *data,
        uint32 *ecc_data);

soc_error_t ecc_correction(
        int unit,
        soc_mem_t mem,
        const uint32 data_len_bits,
        int xor_flag,
        uint32 *data, 
        uint32 *ecc);

#endif /*_DCMN_SER_CORRECTION_H_ */
