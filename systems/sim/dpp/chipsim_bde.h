/* $Id: chipsim_bde.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        chipsim_bde.h
 * Purpose:     expose DPP chipsim bde interface
 */
#ifndef   _CHIPSIM_BDE_H_
#define   _CHIPSIM_BDE_H_

typedef struct chipsim_bde_bus_s {
    uint32 base_addr_start;
    int int_line;
    int be_pio;
    int be_packet;
    int be_other;
} chipsim_bde_bus_t;


#ifndef   _CHIPSIM_PB_MAIN_H_
#define   _CHIPSIM_PB_MAIN_H_
extern int
soc_pb_sim_main(uint32 chip_ver, 
            uint32 run_ui, 
            uint32 run_test,
            uint32 csr, 
            uint32 hw, 
            uint32 run_device_embedded_app);
#endif /* _CHIPSIM_PB_MAIN_H_ */

extern int chipsim_bde_create(chipsim_bde_bus_t *, ibde_t **);

#endif /* _CHIPSIM_BDE_H_ */

