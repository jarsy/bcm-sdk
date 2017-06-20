/*
 * $Id: missing.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

int soc_sbx_g2p3_is_fe2kxt(int unit)
{
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    return SAND_HAL_IS_FE2KXT(sbx->sbhdl);

}



