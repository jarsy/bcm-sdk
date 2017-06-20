/*
 * $Id: phymod_acc.c,v 1.1.2.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <phymod/phymod_acc.h>

int
phymod_acc_check(const phymod_access_t *pa)
{
    if (pa == 0 ||
        PHYMOD_ACC_BUS(pa) == 0 ||
        PHYMOD_ACC_BUS(pa)->read == 0 ||
        PHYMOD_ACC_BUS(pa)->write == 0) {
        return -1;
    }
    return 0;
}

int
phymod_bus_read(const phymod_access_t *pa, uint32_t reg, uint32_t *data)
{
    /* Read raw PHY data */
    return PHYMOD_ACC_BUS(pa)->read(PHYMOD_ACC_USER_ACC(pa),
                                    PHYMOD_ACC_BUS_ADDR(pa),
                                    reg, data);
}

int
phymod_bus_write(const phymod_access_t *pa, uint32_t reg, uint32_t data)
{
    /* Write raw PHY data */
    return PHYMOD_ACC_BUS(pa)->write(PHYMOD_ACC_USER_ACC(pa),
                                     PHYMOD_ACC_BUS_ADDR(pa),
                                     reg, data);
}

int
phymod_is_write_disabled(const phymod_access_t *pa, uint32_t *data)
{
    if((PHYMOD_ACC_BUS(pa)->is_write_disabled) == NULL) { *data = 0; } else {
        return PHYMOD_ACC_BUS(pa)->is_write_disabled(PHYMOD_ACC_USER_ACC(pa),
                                     data);
    }
    return 0;
}
