/*
 *
 * $Id: huracan_reg_access.h 2014/04/02 aman Exp $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *
 *
 */
#ifndef HURACAN_REG_ACCESS_H
#define HURACAN_REG_ACCESS_H

#define HURACAN_CLAUSE45_ADDR(_devad, _regad)     \
            ((((_devad) & 0x3F) << 16) |        \
             ((_regad) & 0xFFFF))




int huracan_reg_read(const phymod_access_t *pa, uint32_t reg_addr, uint32_t *data);
int huracan_reg_write(const phymod_access_t *pa, uint32_t addr, uint32_t data);
int huracan_reg_modify(const phymod_access_t *pa,
                     uint32_t addr,
                     uint16_t reg_data,
                     uint16_t reg_mask);

#endif

