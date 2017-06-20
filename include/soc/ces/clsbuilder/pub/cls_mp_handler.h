
/*******************************************************************

Copyright Redux Communications Ltd.
 
Module Name:

File Name: 
    cls_mp_handler.h

File Description:

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

*******************************************************************/

#ifndef mp_handler_h
#define mp_handler_h
#include "ag_common.h"
#include "clsb_types_priv.h"

#ifdef CLS_MP_HANDLER_C
    #define AG_EXTERN
#else
    #define AG_EXTERN extern
#endif

#ifdef __cplusplus
extern "C"
{
#endif


AG_EXTERN AgResult alloc_mp(MiniPrgDesc** p_mp);
AG_EXTERN void free_mp(MiniPrgDesc* p_mp);

AG_EXTERN AgResult mph_configure(AG_U32 n_max_mp, AG_U32 n_max_grp);

AG_EXTERN AgResult cls_create_mp( MiniPrgDesc *p_mp, MpId* p_mp_id);

AG_EXTERN AgResult cls_get_mpdesc( MpId x_mp_id,  MiniPrgDesc **p_mp);

AG_EXTERN AgResult cls_get_leaf_grp( AG_U32 n_grp_id,  LeafGrp **p_grp);

AG_EXTERN AgResult cls_create_leaf_grp(LeafGrpId* p_grp_id, AgClsbMem* p_cls_mem);

AG_EXTERN AgResult cls_add_mp_leaf(
              MiniPrgDesc* p_mp,  LeafGrp   *p_grp,
              AG_U32 n_record,  AG_U32 n_pathtag,
              TableType e_table,  AG_U16 n_data
			  );

AG_EXTERN AgResult cls_del_mp_leaf(
              MiniPrgDesc* p_mp,  LeafGrp   *p_grp,
              AG_U32 n_record,  AG_U32 n_pathtag,
              TableType e_table,  AG_U16 n_data
             );

AG_EXTERN AgResult cls_update_mp(
              MiniPrgDesc *p_mp,
              MpNodeUpdate *p_updates
             );

AG_EXTERN AgResult cls_link_leaf_grp(
              LeafGrp *p_grp,  MiniPrgDesc* p_to_mp, 
              AG_U8 n_opcode,  AG_U8 n_mask, AG_BOOL b_enable_unlink
             );

AG_EXTERN AgResult cls_unlink_leaf_grp( LeafGrp *p_grp, AG_U32 n_new_tag);

AG_EXTERN AgResult cls_del_mp( MpId x_mp_id,MiniPrgDesc** p_mpdesc, AG_BOOL b_validate,AG_BOOL b_thread_safe);

AG_EXTERN AgResult cls_del_grp( LeafGrpId x_grp,AG_BOOL b_thread_safe);

AG_EXTERN AgResult clsb_mph_reset(AgClsbMem*  p_mem_handle);

AG_EXTERN AgResult mph_dump(const AG_CHAR *p_filename);

AG_EXTERN AgResult cls_mph_engine_consistence (void);

AG_EXTERN AgResult mph_empty_table (void);


/* used for properties of MP */
#define AG_CLSB_STATIC_MP           AG_BIT(0)
#define AG_CLSB_ACTIVE_MP_LAN       AG_BIT(1)
#define AG_CLSB_ACTIVE_MP_WAN1      AG_BIT(2)
#define AG_CLSB_ACTIVE_MP_WAN2      AG_BIT(3)

AgResult clsb_set_active_mp(AgComIntf n_intf, MiniPrgDesc* p_mpdesc);

#define CLSB_SET_STATIC_MP(_p_mpdesc)   ((_p_mpdesc)->n_properties |= AG_CLSB_STATIC_MP)
#define CLSB_IS_STATIC_MP(_p_mpdesc)    ((_p_mpdesc)->n_properties & AG_CLSB_STATIC_MP)
#define CLSB_IS_ACTIVE_MP(_p_mpdesc)    ((_p_mpdesc)->n_properties & (AG_CLSB_ACTIVE_MP_LAN | AG_CLSB_ACTIVE_MP_WAN1 | AG_CLSB_ACTIVE_MP_WAN2) )
#define CLSB_IS_ACTIVE_MP_LAN(_p_mpdesc)    ((_p_mpdesc)->n_properties & AG_CLSB_ACTIVE_MP_LAN)
#define CLSB_IS_ACTIVE_MP_WAN1(_p_mpdesc)    ((_p_mpdesc)->n_properties & AG_CLSB_ACTIVE_MP_WAN1)
#define CLSB_IS_ACTIVE_MP_WAN2(_p_mpdesc)    ((_p_mpdesc)->n_properties & AG_CLSB_ACTIVE_MP_WAN2)

#ifdef __cplusplus
} /*end of extern "C"*/
#endif


#undef AG_EXTERN

#endif /*mp_handler_h */
