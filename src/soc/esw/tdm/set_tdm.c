/*
 * $Id: tdm_main.c.$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * File:        set_tdm.c
 * Purpose:     TDM algorithm
 */
#ifdef _TDM_STANDALONE
	#include <tdm_top.h>
	#include <sdk_drv.h>
#else
	#include <soc/tdm/core/tdm_top.h>
	#include <soc/drv.h>
#endif

size_t stack_size = 0;

#ifndef _TDM_STANDALONE
tdm_mod_t
*SOC_SEL_TDM(tdm_soc_t *chip)
{
	tdm_mod_t *_tdm;
	uint16 dev_id;
	uint16 drv_dev_id;
	uint8 rev_id;
	uint8 drv_rev_id;
	
	_tdm = TDM_ALLOC(sizeof(tdm_mod_t),"TDM constructor allocation");
	if (!_tdm) {
		return NULL;
	}
	_tdm->_chip_data.soc_pkg = (*chip);
	
	soc_cm_get_id( _tdm->_chip_data.soc_pkg.unit, &dev_id, &rev_id );
	soc_cm_get_id_driver( dev_id, rev_id, &drv_dev_id, &drv_rev_id );

	switch (dev_id&0xfff0) {
		case (BCM56850_DEVICE_ID&0xfff0):
		case (BCM56860_DEVICE_ID&0xfff0):
		case (BCM56830_DEVICE_ID&0xfff0):
			{
#if ( defined(BCM_TRIDENT2PLUS_SUPPORT) || defined(BCM_TRIDENT2_SUPPORT) )
                int (*core_exec[TDM_EXEC_CORE_SIZE])( tdm_mod_t* ) = {
                    &tdm_core_init,
                    &tdm_core_post,
                    &tdm_td2p_proc_cal, /* tdm_td2p_proc_cal_tdm5 */
                    &tdm_core_vbs_scheduler_wrapper, /* &tdm_core_vbs_scheduler, */
                    &tdm_core_vbs_scheduler_ovs,
                    &tdm_core_null, /* TDM_CORE_EXEC__EXTRACT */
                    &tdm_core_null, /* TDM_CORE_EXEC__FILTER */
                    &tdm_core_acc_alloc,
                    &tdm_core_vmap_prealloc,
                    &tdm_core_vmap_alloc,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_td2p_check_ethernet,
                    &tdm_pick_vec,
                    &tdm_td2p_which_tsc
                };
                int (*chip_exec[TDM_EXEC_CHIP_SIZE])( tdm_mod_t* ) = {
                    &tdm_td2p_init,
                    &tdm_td2p_pmap_transcription,
                    &tdm_td2p_lls_wrapper,
                    &tdm_td2p_vbs_wrapper,
                    &tdm_td2p_filter, /* tdm_td2p_filter_tdm5 */
                    &tdm_td2p_parse_mmu_tdm_tbl,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_td2p_chk,
                    &tdm_td2p_free,
                    &tdm_td2p_corereq,
                    &tdm_td2p_post
                };
				TDM_COPY(_tdm->_core_exec,core_exec,sizeof(_tdm->_core_exec));
				TDM_COPY(_tdm->_chip_exec,chip_exec,sizeof(_tdm->_chip_exec));
#endif
			}
			break;
		case (BCM56960_DEVICE_ID&0xfff0):
        case BCM56930_DEVICE_ID:
        case (BCM56168_DEVICE_ID&0xfff0):
			{
#ifdef BCM_TOMAHAWK_SUPPORT
                int (*core_exec[TDM_EXEC_CORE_SIZE])( tdm_mod_t* ) = {
                    &tdm_core_init,
                    &tdm_core_post,
                    &tdm_th_proc_cal, /* &tdm_th_proc_cal_tdm5, */
                    &tdm_core_vbs_scheduler_wrapper, /* &tdm_core_vbs_scheduler, */
                    &tdm_core_vbs_scheduler_ovs,
                    &tdm_core_null, /* TDM_CORE_EXEC__EXTRACT */
                    &tdm_core_null, /* TDM_CORE_EXEC__FILTER */
                    &tdm_core_acc_alloc,
                    &tdm_core_vmap_prealloc,
                    &tdm_core_vmap_alloc,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_th_check_ethernet,
                    &tdm_pick_vec,
                    &tdm_th_which_tsc
                };
                int (*chip_exec[TDM_EXEC_CHIP_SIZE])( tdm_mod_t* ) = {
                    &tdm_th_init,
                    &tdm_th_pmap_transcription,
                    &tdm_th_scheduler_wrap,
                    &tdm_core_null,
                    &tdm_th_filter, /* &tdm_th_filter_tdm5, */
                    &tdm_th_parse_tdm_tbl,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_core_null,
                    &tdm_th_chk, /* &tdm_th_chk_tdm5, */
                    &tdm_th_free,
                    &tdm_th_corereq,
                    &tdm_th_post
                };
				TDM_COPY(_tdm->_core_exec,core_exec,sizeof(_tdm->_core_exec));
				TDM_COPY(_tdm->_chip_exec,chip_exec,sizeof(_tdm->_chip_exec));
#endif
			}
			break;
		case (BCM56970_DEVICE_ID&0xfff0):
			{
#ifdef BCM_TOMAHAWK2_SUPPORT
				int (*core_exec[TDM_EXEC_CORE_SIZE])( tdm_mod_t* ) = {
					&tdm_core_init,
					&tdm_core_post,
					&tdm_th2_vmap_alloc,
					&tdm_th2_vbs_scheduler, /* &tdm_core_vbs_scheduler, */
					&tdm_chip_th2_shim__core_vbs_scheduler_ovs,
					&tdm_core_null, /* TDM_CORE_EXEC__EXTRACT */
					&tdm_core_null, /* TDM_CORE_EXEC__FILTER */
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_th2_check_ethernet,
					&tdm_pick_vec,
					&tdm_th2_which_tsc
				};
				int (*chip_exec[TDM_EXEC_CHIP_SIZE])( tdm_mod_t* ) = {
					&tdm_th2_init,
					&tdm_th2_pmap_transcription,
					&tdm_th2_scheduler_wrap,
					&tdm_core_null,
					&tdm_th2_filter_chain,
					&tdm_th2_parse_tdm_tbl_new, /* &tdm_th2_parse_tdm_tbl, */
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_th2_acc_alloc,
					&tdm_th2_corereq,
					&tdm_th2_post
				};
				TDM_COPY(_tdm->_core_exec,core_exec,sizeof(_tdm->_core_exec));
				TDM_COPY(_tdm->_chip_exec,chip_exec,sizeof(_tdm->_chip_exec));
#endif
			}
			break;
		case (BCM56560_DEVICE_ID&0xfff0):
		case (BCM56760_DEVICE_ID&0xfff0):
		case (BCM56069_DEVICE_ID&0xfff0):
			{
#ifdef BCM_APACHE_SUPPORT 
				int (*core_exec[TDM_EXEC_CORE_SIZE])( tdm_mod_t* ) = {
					&tdm_core_init,
					&tdm_core_post,
					&tdm_ap_vmap_alloc,
					&tdm_core_vbs_scheduler,
					&tdm_chip_ap_shim__core_vbs_scheduler_ovs,
					&tdm_core_null, /* TDM_CORE_EXEC__EXTRACT */
					&tdm_core_null, /* TDM_CORE_EXEC__FILTER */
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_ap_check_ethernet,
					&tdm_pick_vec,
					&tdm_ap_which_tsc
				};
				int (*chip_exec[TDM_EXEC_CHIP_SIZE])( tdm_mod_t* ) = {
					&tdm_ap_init,
					&tdm_ap_pmap_transcription,
					&tdm_ap_lls_wrapper,
					&tdm_ap_vbs_wrapper,
					&tdm_ap_filter_chain,
					&tdm_ap_parse_mmu_tdm_tbl,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_core_null,
					&tdm_ap_chk,
					&tdm_ap_free,
					&tdm_ap_corereq,
					&tdm_ap_post
				};
				TDM_COPY(_tdm->_core_exec,core_exec,sizeof(_tdm->_core_exec));
				TDM_COPY(_tdm->_chip_exec,chip_exec,sizeof(_tdm->_chip_exec));
#endif
			}
			break;
		default:
			TDM_FREE(_tdm);
			TDM_ERROR1("Unrecognized device ID %0x for TDM scheduling algorithm.\n",dev_id);
			return NULL;
	}
	
	return _tdm;
}
#endif


tdm_mod_t
*_soc_set_tdm_tbl( tdm_mod_t *_tdm )
{
	int index, tdm_ver_chk[8];
	
    /* stack_size = (size_t)&index; */
    
	if (!_tdm) {
		return NULL;
	}
	TDM_BIG_BAR
	TDM_PRINT0("TDM: Release version: ");
	_soc_tdm_ver(tdm_ver_chk);
	TDM_PRINT2("%d%d",tdm_ver_chk[0],tdm_ver_chk[1]);
	for (index=2; index<8; index+=2) {
		TDM_PRINT2(".%d%d",tdm_ver_chk[index],tdm_ver_chk[index+1]);
	}
	TDM_PRINT0("\n"); TDM_SML_BAR
	
	/* Path virtualized API starting in chip executive */
	return ((_tdm->_chip_exec[TDM_CHIP_EXEC__INIT](_tdm))==PASS)?(_tdm):(NULL);
}
