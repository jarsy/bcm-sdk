/*
 * $Id: ctrl_if.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _SOC_CTRL_IF_H
#define _SOC_CTRL_IF_H

typedef enum {
    socEaOamCtrlInitFalse,
    socEaOamCtrlInitTrue,
    socEaOamCtrlInitCount
}soc_ea_oam_ctrl_init_state_t;

typedef struct soc_ea_oam_ctrl_if_s {
    void (*init)(int u);                               		       /* oam control interface initialization */
    void (*deinit)(int u);                               		       /* oam control interface de-initialization */
    void (*request)(int u, uint8 *buf_ptr, uint32 buf_len);  /* oam control packet transmit */
    void (*response_rtn_call_reg)(int u, void *rtn_fn);    /* oam control packet receive callout register */
} soc_ea_oam_ctrl_if_t;

enum soc_ea_oam_ctrl_if_e {
    socEaOamCtrlIfEther,
    socEaOamCtrlIfI2C,

    socEaOamCtrlIfCount
};

soc_ea_oam_ctrl_if_t *
soc_ea_oam_ctrl_attach(uint32 type);

int
soc_ea_oam_ctrl_state_get(int unit);

int
soc_ea_oam_ctrl_state_set(int unit, int state);

#endif /* _SOC_CTRL_IF_H */
