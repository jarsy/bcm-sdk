/*
 * $Id: field.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        field.h
 * Purpose:     Register/memory field descriptions
 */

#ifndef _SOC_SBX_FIELD_H
#define _SOC_SBX_FIELD_H

#define SBX_FIELD_NAME_LEN_MAX 128

typedef struct soc_sbx_field_info_s {
    char    name[SBX_FIELD_NAME_LEN_MAX];
    uint32  mask;
    uint8   shift;
    uint8   msb;
    uint8   lsb;
    uint32  type;
    uint32  default_val;
} soc_sbx_field_info_t;

#endif	/* !_SOC_SBX_FIELD_H */
