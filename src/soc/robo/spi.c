/*
 * $Id: spi.c,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Routines for accessing BCM53xx SPI memory mapped registers
 */

#include <sal/core/libc.h>
#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>

#include <soc/robo/mcm/driver.h>
#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/drv.h>

#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/bcmenetrxh.h>
#include <shared/et/et_dbg.h>
#include <shared/et/et_export.h>
#include <soc/etc.h>

#include <ibde.h>

int
soc_spi_read(int unit, uint32 addr, uint8 *buf, int len)
{
    CMVEC(unit).spi_read(&CMDEV(unit).dev, addr, buf, len);
    return SOC_E_NONE;
}

int
soc_spi_write(int unit, uint32 addr, uint8 *buf, int len)
{
    CMVEC(unit).spi_write(&CMDEV(unit).dev, addr, buf, len);
    return SOC_E_NONE;
}
