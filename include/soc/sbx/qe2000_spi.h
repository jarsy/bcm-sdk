
/*
 * $Id: qe2000_spi.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == qe2000_spi.h - QE SPI Initialization      ==
 */

#ifndef _QE2000_SPI_INIT_H
#define _QE2000_SPI_INIT_H

/* #include "sbTypes.h" */
#include "glue.h"
#include "qe2000_init.h"

void      hwQe2000Spi4TxForceTrainingOn(sbhandle userDeviceHandle, uint32 spi_ul);
void      hwQe2000Spi4TxForceTrainingOff(sbhandle userDeviceHandle, uint32 spi_ul);
uint32  hwQe2000Spi4RxStatus(sbhandle userDeviceHandle, uint32 spi_ul);
uint32  hwQe2000Spi4TxStatus(sbhandle userDeviceHandle, uint32 spi_ul);
void      hwQe2000Spi4RxEnable(sbhandle userDeviceHandle, uint32 enable_ul, uint32 spi_ul);
void      hwQe2000Spi4TxEnable(sbhandle userDeviceHandle, uint32 enable_ul, uint32 spi_ul);
void      hwQe2000Spi4RxEnableGet(sbhandle userDeviceHandle, uint32 *enable_ul, uint32 spi_ul);
void      hwQe2000Spi4TxEnableGet(sbhandle userDeviceHandle, uint32 *enable_ul, uint32 spi_ul);
void      hwQe2000Spi4RxRequestTraining(sbhandle userDeviceHandle, uint32 enable_ul, uint32 spi_ul);

/*========
**= QE   =
**========
*/

/* bitfield access */
#define HW_QE2000_BIT_N(x)                (0x1 << (x))
#define HW_QE2000_MSK_NBITS(n)            (0xFFFFFFFF >> (32 - (n)))
#define HW_QE2000_ZIN_MASK64(i, mask64)   (!!(COMPILER_64_BITTEST(mask64,i)))

#define HW_QE2000_MAX_SPI4_PORTS_K   (49)

/* pkt sizes */
#define HW_QE2000_SHIMHDR_SZ_K            (12)
#define HW_QE2000_MIN_FRM_SZ_K            (64)
#define HW_QE2000_MAX_FRM_SZ_NRML_K     (1518)
#define HW_QE2000_MAX_FRM_SZ_JMBO_K     (9216)
#define HW_QE2000_CRC_SZ_K                 (4)

#endif /* _INIT_SPI_QE2000_H */
