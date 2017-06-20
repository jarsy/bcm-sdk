/*
 * $Id: sbx_txrx.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _SBX_TXRX_H
#define _SBX_TXRX_H

#include <sal/types.h>
#include <soc/sbx/sbx_drv.h>

typedef enum SBX_rh_fields_e {
    SBX_rhf_queue_id,
    SBX_rhf_ksop,
    SBX_rhf_sdp,
    SBX_rhf_ecn,
    SBX_rhf_ect,
    SBX_rhf_test,
    SBX_rhf_length,
    SBX_rhf_length_adjust,
    SBX_rhf_ttl,
    SBX_rhf_s,
    SBX_rhf_fdp2,
    SBX_rhf_lbid,
    SBX_rhf_fcos,
    SBX_rhf_fcos2,
    SBX_rhf_fdp,
    SBX_rhf_rcos,
    SBX_rhf_rdp,
    SBX_rhf_ppeswop,
    SBX_rhf_sid,
    SBX_rhf_mc,
    SBX_rhf_outunion,
    SBX_rhf_invalid /* last */
} SBX_rh_fields_t;


extern int soc_sbx_hdr_field_set(int unit, uint8 *hdr, uint8 hdr_len,
                                 SBX_rh_fields_t field, uint32 val);

extern int soc_sbx_hdr_field_get(int unit, uint8 *hdr, uint8 hdr_len,
                                 SBX_rh_fields_t field, uint32 *val);


extern int soc_sbx_txrx_init(int unit);
extern int soc_sbx_txrx_init_hw_only(int unit);
extern int soc_sbx_txrx_uninit_hw_only(int unit);
extern int soc_sbx_txrx_give_rx_buffers(int unit, int bufs, void **bufps,
                                        soc_sbx_txrx_done_f donecb, 
                                        void **cookies);
extern int soc_sbx_txrx_remove_rx_buffers (int unit, int bufs);
extern int soc_sbx_txrx_tx(int unit, char *hdr, int hdrlen, int bufs,
                           void **bufps, int *buflens, soc_sbx_txrx_done_f 
                           donecb, void *cookie);
extern int soc_sbx_txrx_sync_tx(int unit, char *hdr, int hdrlen, char *buf,
                                int buflen, int waitusec);
int soc_sbx_txrx_sync_rx(int unit, char *buf, int *buflen, int waitusec);
extern void soc_sbx_txrx_intr(int unit, uint32 unused);

#endif /* _SBX_TXRX_H */

