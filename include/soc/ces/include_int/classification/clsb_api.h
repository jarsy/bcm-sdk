
/*******************************************************************

Copyright Redux Communications Ltd.

Module Name: 

File Name: 
    clsb_api.h

  $Revision: 1.1.2.1 $  - Visual SourceSafe revision number 

File Description: 

History:
    Date            Name            Comment
    ----            ----            -------
    07/05/01   
*******************************************************************/
#ifndef CLSB_API_H 
#define CLSB_API_H 

#include "ag_common.h"
#include "ag_hl_api.h"
#include "classification/clsb_types.h"
#include "classification/cls_results.h"
#include "classification/cls_types.h"

#ifdef __cplusplus
#include "utils/arc_media_interfce.hpp"
#endif

#ifdef CLSB_API_H
#define AG_EXTERN
#else
#define AG_EXTERN extern
#endif /* CLSB_API_H */

#ifdef __cplusplus
extern "C"
{
#endif


AG_EXTERN AgResult ag_clsb_get_mem_usage(AgClsMemUsage* p_mem_usage);
AG_EXTERN AgResult ag_clsb_get_hw_mem_utilization(AgClsbMemHndl n_cls_mem_handle, AgClsbHwMemUtil* p_utilization);
AG_EXTERN AgResult ag_clsb_get_hw_mem_usage(AgClsbMemHndl n_cls_mem_handle, AgClsbHwMemUse* p_usage);
AG_EXTERN void ag_clsb_print_mem_usage(void);

/* the function returns the previous mode of validation		 */
AG_EXTERN AgEnable ag_clsb_set_validation_mode(AgEnable e_enable);

AG_EXTERN AgResult ag_clsb_set_active_prg(AgComIntf n_interf,PrgRoot *p_root, void * p_prg, AG_U32 n_mode_of_work);
AG_EXTERN AgResult ag_clsb_set_mode(AgComIntf e_rx_intf,AG_U32 n_mode_of_work);
AG_EXTERN AgResult ag_clsb_set_prg_base(AgComIntf e_rx_intf,void * p_prg);
AG_EXTERN AgResult ag_clsb_set_late_cls_hndl(AgComIntf e_rx_intf,AG_BOOL b_pass_to_cpu);
AG_EXTERN AgResult ag_clsb_get_late_cls_hndl(AgComIntf e_rx_intf,AG_BOOL* p_pass_to_cpu);
AG_EXTERN AgResult ag_clsb_set_overrun_hndl(AgComIntf e_rx_intf,AG_BOOL b_pass_to_cpu);
AG_EXTERN AgResult ag_clsb_get_overrun_hndl(AgComIntf e_rx_intf,AG_BOOL* p_pass_to_cpu);


AG_EXTERN AgResult ag_clsb_create_std_prg (AgClsbMemHndl n_cls_mem_handle , AG_U8 init_mask, AG_U8 init_opcode,  MpId *p_mpid, AG_U32 n_static_flag);
AG_EXTERN AgResult ag_clsb_cfg (AG_U32 n_max_mem, AgClsMemUsage* p_mem_req);
AG_EXTERN AgResult ag_clsb_create_grp (LeafGrpId* p_grpid, AgClsbMemHndl n_cls_mem_handle);
AG_EXTERN AgResult ag_clsb_link ( AG_U8 n_opc ,  AG_U8 n_mask, LeafGrpId x_grp ,  MpId x_mpid ,  AG_BOOL b_allow_unlink);


AG_EXTERN AgResult ag_clsb_unlink ( LeafGrpId x_grp, AG_U32 n_new_tag);
AG_EXTERN AgResult ag_clsb_del_prg ( MpId x_mpid/*,void** p_mp*/);/*MBINT - 22/10/2000 - second parameter removed */
AG_EXTERN AgResult ag_clsb_del_grp( LeafGrpId x_grp_id);

/* Irena 21/5/2001 New API Functions  */
AG_EXTERN AgResult ag_clsb_create_hw_mem (void *h_device /*BCM-CLS:CPU=NULL/else in CES*/  , void* p_mem_start, AG_U32 n_mem_size, AG_U32 n_cls_mode  /* 8/16 */, AgClsbMemHndl* p_mem_handle);
AG_EXTERN AgResult ag_clsb_reset_memory(AgClsbMemHndl x_mem_handle);
AG_EXTERN AgResult ag_clsb_del_hw_mem(AgClsbMemHndl x_mem_handle);
AG_EXTERN AgResult ag_clsb_activate_prg(AgComIntf n_interf, MpId n_mp_id);
AG_EXTERN AgResult ag_clsb_is_active(MpId n_mp_id, AG_BOOL *p_active);

AG_EXTERN AgResult ag_clsb_query_path_8 (MpId x_mpid, 
									   AG_U8 *p_frame, 
									   AG_U32 n_frame_len, 
									   AG_BOOL n_match_flag, 
									   AG_U32 *p_tag, 
									   AG_U32 *p_pos);

AG_EXTERN AgResult ag_clsb_query_path_16 (MpId x_mpid, 
									   AG_U16 *p_frame, 
									   AG_U32 n_frame_len, 
									   AG_BOOL n_match_flag, 
									   AG_U32 *p_tag, 
									   AG_U32 *p_pos);

AG_EXTERN AgResult ag_clsb_change_tag_8(MpId x_mpid, 
										AG_U8 *p_path,
										AG_U8 *p_opc,
										AG_U32 n_pathlen,
										AG_U32 n_old_tag,
										AG_U32 n_new_tag);
/* Irena The following two functions should not be used directly (use macros) */
AG_EXTERN AgResult cls_add_path_8 ( MpId x_mpid ,
								  AG_U8 *p_path ,  
								  AG_U8 *p_ctl , 
								  AG_U32 n_pathlen ,  
								  AG_U32 *p_tag,  
								  AG_U32 n_taglen, 
								  LeafGrpId x_grp);

AG_EXTERN AgResult cls_add_path_16 ( MpId x_mpid ,
								  AG_U16 *p_path ,  
								  AG_U8 *p_ctl , 
								  AG_U8 *p_mask,
								  AG_U32 n_pathlen ,  
								  AG_U32 *p_tag,  
								  AG_U32 n_taglen, 
								  LeafGrpId x_grp);

AG_EXTERN AgResult cls_delete_path_8 (  MpId x_mpid, 
									  AG_U8 *p_path, 
									  AG_U8 *p_ctl,  
									  AG_U32 n_pathlen, 
									  LeafGrpId x_grp, 
									  AG_U32 *p_erase_pos, 
									  AG_U16 *p_ref_count);

AG_EXTERN AgResult cls_delete_path_16 (  MpId x_mpid, 
									  AG_U16 *p_path, 
									  AG_U8 *p_ctl,  
									  AG_U8 *p_mask,  
									  AG_U32 n_pathlen, 
									  LeafGrpId x_grp, 
									  AG_U32 *p_erase_pos, 
									  AG_U16 *p_ref_count);


#ifdef __cplusplus
AG_EXTERN AgResult ag_clsb_serialize(IAgArchMedia* p_arc, MpId x_root , AgComIntf e_if);
AG_EXTERN AgResult ag_clsb_get_serialize_size(AgClsbMemHndl n_cls_mem_handle, AG_U32* p_size);
AG_EXTERN AgResult ag_clsb_load_prg(IAgArchMedia *p_arc,void* p_to);
#endif

/*AG_EXTERN AgResult ag_clsb_mph_dump(AG_U8 *p_filename); */
AG_EXTERN AgResult ag_clsb_dump (const AG_CHAR *output_file , MpId x_mpid);
AG_EXTERN AgResult ag_clsb_dump_bin (const AG_CHAR *output_file , MpId x_mpid);
AG_EXTERN AgResult ag_clsb_dump_vl (const AG_CHAR *output_file ,MpId x_mpid);
AG_EXTERN AgResult ag_clsb_load_file(const AG_CHAR *p_file_name, ClsEntry *p_prg,AG_U32 *p_prg_size,PrgRoot *p_root,AG_U32 *p_mode_of_work);
AG_EXTERN AgResult ag_clsb_mph_dump(const AG_CHAR *p_filename);
AG_EXTERN AgResult ag_clsb_cls_prg_dump(const AG_CHAR *output_file , AgClsbMemHndl x_mem_handle);
AG_EXTERN AgResult ag_clsb_get_memory_handle(MpId x_mpid, AgClsbMemHndl* p_cls_mem_handle);
AG_EXTERN AgResult ag_clsb_get_number_of_added_paths(MpId x_mpid, AG_U32* p_num);

#define ag_clsb_add_path_with_grp_8(x_mpid, p_path, p_ctl, n_pathlen, p_tag, n_taglen, x_grp)  (cls_add_path_8(x_mpid, p_path, p_ctl, n_pathlen, p_tag, n_taglen, x_grp))
#define ag_clsb_add_path_8(x_mpid, p_path, p_ctl, n_pathlen, p_tag, n_taglen)  (cls_add_path_8(x_mpid, p_path, p_ctl, n_pathlen, p_tag, n_taglen, AG_CLSB_IGNORE_GRP))
#define ag_clsb_add_path_with_grp_16(x_mpid, p_path, p_ctl, p_mask, n_pathlen, p_tag, n_taglen, x_grp)  (cls_add_path_16(x_mpid, p_path, p_ctl, p_mask, n_pathlen, p_tag, n_taglen, x_grp))
#define ag_clsb_add_path_16(x_mpid, p_path, p_ctl, p_mask, n_pathlen, p_tag, n_taglen)  (cls_add_path_16(x_mpid, p_path, p_ctl, p_mask, n_pathlen, p_tag, n_taglen, AG_CLSB_IGNORE_GRP))

#define ag_clsb_delete_path_with_grp_8(x_mpid, p_path , p_ctl, n_pathlen, x_grp, p_erase_pos, p_ref_count)   (cls_delete_path_8(x_mpid, p_path , p_ctl, n_pathlen , x_grp, p_erase_pos, p_ref_count))
#define ag_clsb_delete_path_8(x_mpid, p_path , p_ctl, n_pathlen, p_erase_pos, p_ref_count)   (cls_delete_path_8(x_mpid, p_path , p_ctl, n_pathlen , AG_CLSB_IGNORE_GRP, p_erase_pos, p_ref_count))
#define ag_clsb_delete_path_with_grp_16(x_mpid, p_path , p_ctl, p_mask, n_pathlen, x_grp, p_erase_pos, p_ref_count)   (cls_delete_path_16(x_mpid, p_path , p_ctl, p_mask, n_pathlen , x_grp, p_erase_pos, p_ref_count))
#define ag_clsb_delete_path_16(x_mpid, p_path , p_ctl, p_mask, n_pathlen, p_erase_pos, p_ref_count)   (cls_delete_path_16(x_mpid, p_path , p_ctl, p_mask, n_pathlen , AG_CLSB_IGNORE_GRP, p_erase_pos, p_ref_count))

#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#undef AG_EXTERN

#endif  /* CLSB_API_H */
