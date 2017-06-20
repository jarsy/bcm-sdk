/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BCM5301X spi register definition
 */


#ifndef _nspspi_core_h_
#define _nspspi_core_h_

typedef volatile struct _nspspiregs {
    uint32 	cca_gsioctl;
    uint32 	cca_gsioaddress;
    uint32 	cca_gsiodata;
} nspspiregs_t;

#define CCA_GSIOCTL_STARTBUSY_MASK	0x80000000
#define CCA_GSIOCTL_GSIOOP_MASK         0x000000FF
#define CCA_GSIOCTL_GSIOOP_READ         0x60
#define CCA_GSIOCTL_GSIOOP_WRITE        0x61
#define CCA_GSIOCTL_GSIOCODE_MASK       0x00000700
#define CCA_GSIOCTL_GSIOCODE_SHIFT      8
#define CCA_GSIOCTL_NUMADDR_MASK        0x00003000
#define CCA_GSIOCTL_NUMADDR_SHIFT       12
#define CCA_GSIOCTL_NUMADDR_DEF         0
#define CCA_GSIOCTL_NUMDATA_MASK        0x00030000
#define CCA_GSIOCTL_NUMDATA_SHIFT       16

#define CCA_GSIOCTL_GSIOCODE_0          0x0
#define CCA_GSIOCTL_GSIOCODE_1          0x1
#define CCA_GSIOCTL_GSIOCODE_2          0x2

#define BCM53134_SPI_DATA_REG           0xF0
#define BCM53134_SPI_STS_REG            0xFE
#define BCM53134_SPI_PAGE_REG           0xFF

#define BCM53134_SPI_SPIF_MASK          0x80
#define BCM53134_SPI_RACK_MASK          0x20

#define UINT32_MASK                     0xFFFFFFFF

#endif /* _nspspi_core_h_ */
