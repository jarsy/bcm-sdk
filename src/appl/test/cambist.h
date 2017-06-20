/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */ 

#define NO_MODE 99
#define NO_MODE_FIELD 0xffffffff
#define NO_CONTROL_REGISTER -1
#define NO_CONTROL_ENABLE_FIELD 0xffffffff

typedef struct cambist_s {
    char *cambist_name;
    uint32 mode;
    soc_field_t mode_field;
    soc_reg_t cambist_config;
    soc_reg_t cambist_control;
    soc_field_t cambist_enable;
    uint32 cambist_count;
    soc_reg_t cambist_status;
} cambist_t;
