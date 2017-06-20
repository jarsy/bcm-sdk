
/*******************************************************************

Copyright Redux Communications Ltd.

Module Name: 

File Name: 
    cls_tbl_api.h

File Description: 

  $Revision: 1.1.2.1 $  - Visual SourceSafe revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    10/05/01
	12/08/02		Raanan Refua	addition of the functions
									ag_ct_get_hip_pointer(n_tag,p_val)
									ag_ct_get_hip_len(n_tag,p_val)
	18/08/02		Raanan Refua	addition of the function
									ag_ct_get_sot(n_tag, p_val)
*******************************************************************/
#ifndef CLS_TBL_API_H 
#define CLS_TBL_API_H 

#include "ag_common.h"
#include "utils/gen_reg_acce.h"
#include "classification/cls_types.h"
#include "flow/flow_types.h"
#include "drivers/gpc_drv.h"

#ifdef CLS_TBL_API_C
    #define AG_EXTERN 
#else
    #define AG_EXTERN extern
#endif

#define   AG_CT_MAX_SIZE        0x200000
#define   AG_CT_MIN_SIZE        (AG_CLSB_FIRST_FREE_TAG+1)



#ifdef __cplusplus
extern "C"
{
#endif

AG_EXTERN AgClsTblEntry *pCtBase;
AG_EXTERN AG_U32 nCurrFlowAlloc;
#if defined AG_MEM_MONITOR
AG_EXTERN AG_U32 nMaxFlowAlloc;
#endif

AG_EXTERN AgResult ag_alloc_flow_struct(AgFlow** p_flow);
AG_EXTERN void ag_free_flow_struct(AgFlow* p_flow);

AG_EXTERN AgResult ag_ct_config(void* p_cls_tbl_base, AG_U32 n_num_of_entries);
AG_EXTERN AgResult ag_ct_get_config(void** p_cls_tbl_base, AG_U32* p_num_of_entries);
AG_EXTERN AgResult ag_ct_set_entries(AG_U32 n_start_tag, AG_U32 num_of_entries, AgClsTblEntry* p_entry_val);
AG_EXTERN AgResult ag_ct_set_hw_act(AG_U32 n_tag, const AgClsTblEntry* p_entry_val);
AG_EXTERN AgResult ag_ct_init_entry(AgCtEntryType e_entry_type, AgClsTblEntry* p_entry);

AG_EXTERN AgResult ag_ct_get_queue(AG_U32 n_tag, AG_U8* p_val);
AG_EXTERN AgResult ag_ct_set_queue(AG_U32 n_tag, AG_U8 n_val);

AG_EXTERN AgResult ag_ct_set_gpc_act(AG_U32 n_tag, AgGpcCntGrp e_group_num, AgClsGpcAct* p_gpc_act);
AG_EXTERN AgResult ag_ct_get_gpc_act(AG_U32 n_tag, AgGpcCntGrp e_group_num, AgClsGpcAct* p_gpc_act);

AG_EXTERN AgResult ag_ct_set_sw_act(AG_U32 n_tag, const AgClsSwAct* p_sw_act);
AG_EXTERN AgResult ag_ct_get_sw_act(AG_U32 n_tag, AgClsSwAct* p_sw_act);
AG_EXTERN AgResult ag_ct_del_sw_act(AG_U32 n_tag, AgClsSwAct* p_sw_act);
#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#define ag_ct_get_sw(n_tag)         (pCtBase[n_tag].p_sw_act)
#define ag_ct_set_sw(n_tag,n_val)   (pCtBase[n_tag].p_sw_act = n_val)


/*//////////////////////////////////////////////////////////////////////// */
/* classification table macros which are not dependant on the TAG. They get as a parameter */
/* a pointer to an array of U32 which is the begining of the cls table entry format. */
/*//////////////////////////////////////////////////////////////////////// */

/* queue */
#define ag_ct_get_que(p_entry)   ((AG_U8)(((AG_U32*)(p_entry))[0]) )
#define ag_ct_set_que(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[0], n_val, 0, 8)

/* discard */
#define ag_ct_get_discard(p_entry)		(AG_READ_BIT_VALUE(&((AG_U32*)(p_entry))[0],8))
#define ag_ct_set_discard(p_entry,n_val)   ag_set_bit_value(&((AG_U32*)(p_entry))[0], n_val, 8)

/* Tx& Fwd */
#define ag_ct_get_tx_and_fwd(p_entry)		 (AG_READ_BIT_VALUE(&((AG_U32*)(p_entry))[1],0))
#define ag_ct_set_tx_and_fwd(p_entry,n_val)   ag_set_bit_value(&((AG_U32*)(p_entry))[1], n_val, 0)

/* tos */
#define ag_ct_get_new_tos(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[0], 10, p_val, 6)
#define ag_ct_set_new_tos(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[0], n_val, 10, 6)

/* tos enable */
#define ag_ct_get_tos_enable(p_entry)		(AG_READ_BIT_VALUE(&((AG_U32*)(p_entry))[1],10))
#define ag_ct_set_tos_enable(p_entry,n_val)   ag_set_bit_value(&((AG_U32*)(p_entry))[1], n_val, 10)

/* decrement ttl */
#define ag_ct_get_dec_ttl(p_entry)   (AG_READ_BIT_VALUE(&((AG_U32*)(p_entry))[1],3))
#define ag_ct_set_dec_ttl(p_entry,n_val)   ag_set_bit_value(&((AG_U32*)(p_entry))[1], n_val, 3)

/* ttl check */
#define ag_ct_get_ttl_check(p_entry)		(AG_READ_BIT_VALUE(&((AG_U32*)(p_entry))[0],9))
#define ag_ct_set_ttl_check(p_entry,n_val)   ag_set_bit_value(&((AG_U32*)(p_entry))[0], n_val, 9)

/* napt ip */
#define ag_ct_get_napt_ip(p_entry)			(((AG_U32*)(p_entry))[2]) 
#define ag_ct_set_napt_ip(p_entry,n_val)    ( ((AG_U32*)(p_entry))[2] = n_val )

/* napt port */
#define ag_ct_get_napt_port(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[0], 16, p_val, 16)
#define ag_ct_set_napt_port(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[0], n_val, 16, 16)

/* nat mode */
#define ag_ct_get_nat_mode(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[1], 1, p_val, 2)
#define ag_ct_set_nat_mode(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[1], n_val, 1, 2)

/* header program offset */
#define ag_ct_get_hp_offset(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[1], 16, p_val, 16)
#define ag_ct_set_hp_offset(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[1], n_val, 16, 16)

/* header program length  */
#define ag_ct_get_hp_len(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[1], 11, p_val, 5)
#define ag_ct_set_hp_len(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[1], n_val, 11, 5)

/* sot */
#define ag_ct_get_start_of_transmit(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[1], 4, p_val, 6)
#define ag_ct_set_start_of_transmit(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[1], n_val, 4, 6)

/* gpc grp1 control */
#define ag_ct_get_grp1_control(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 13, p_val, 3)
#define ag_ct_set_grp1_control(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 13, 3)

/* gpc grp2 control */
#define ag_ct_get_grp2_control(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 10, p_val, 3)
#define ag_ct_set_grp2_control(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 10, 3)

/* gpc grp3 control */
#define ag_ct_get_grp3_control(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 7, p_val, 3)
#define ag_ct_set_grp3_control(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 7, 3)

/* gpc grp4 control */
#define ag_ct_get_grp4_control(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 4, p_val, 3)
#define ag_ct_set_grp4_control(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 4, 3)

/* gpc grp5 control */
#define ag_ct_get_grp5_control(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 2, p_val, 2)
#define ag_ct_set_grp5_control(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 2, 2)

/* gpc grp6 control */
#define ag_ct_get_grp6_control(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 0, p_val, 2)
#define ag_ct_set_grp6_control(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 0, 2)

/* gpc grp1 index */
#define ag_ct_get_grp1_indx(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[4], 11, p_val, 21)
#define ag_ct_set_grp1_indx(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[4], n_val, 11, 21)

/* gpc grp2 index */
#define ag_ct_get_grp2_indx(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[4], 0, p_val, 11)
#define ag_ct_set_grp2_indx(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[4], n_val, 0, 11)

/* gpc grp3 index */
#define ag_ct_get_grp3_indx(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[5], 16, p_val, 16)
#define ag_ct_set_grp3_indx(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[5], n_val, 16, 16)

/* gpc grp4 index */
#define ag_ct_get_grp4_indx(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[6], 20, p_val, 12)
#define ag_ct_set_grp4_indx(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[6], n_val, 20, 12)

/* gpc grp5 indx */
#define ag_ct_get_grp5_indx(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[6], 10, p_val, 10)
#define ag_ct_set_grp5_indx(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[6], n_val, 10, 10)

/* gpc grp6 indx */
#define ag_ct_get_grp6_indx(p_entry,p_val)   ag_read_range_value(&((AG_U32*)(p_entry))[6], 0, p_val, 10)
#define ag_ct_set_grp6_indx(p_entry,n_val)   ag_set_range_value(&((AG_U32*)(p_entry))[6], n_val, 0, 10)


/*/////////////////////////////////////////////////////////////////////////////////////////// */
/* obsoleted versions of the classification table MACROS */
/*/////////////////////////////////////////////////////////////////////////////////////////// */

/*set queue in the entry that p_entry points to.*/
#define ag_ct_get_q(p_entry,p_val)   ( *(p_val) = (AG_U8)((p_entry)->a_val[0]) )
#define ag_ct_set_q(p_entry,n_val)   ag_set_range_value((AG_U32*)&(p_entry)->a_val[0], n_val, 0, 8)

/*#define ag_ct_get_disc(n_tag,p_bit_val)   (*(p_bit_val) = AG_READ_BIT_VALUE((AG_U32*)&pCtBase[n_tag].a_val[0],8)) */
#define ag_ct_set_disc(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[0], n_bit_val, 8)

/*#define ag_ct_get_ttl_chk(n_tag,p_bit_val)   (*(p_bit_val) = AG_READ_BIT_VALUE((AG_U32*)&pCtBase[n_tag].a_val[0],9)) */
#define ag_ct_set_ttl_chk(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[0], n_bit_val, 9)

#define ag_ct_get_tos(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[0], 10, p_val, 6)
#define ag_ct_set_tos(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[0], n_val, 10, 6)

/*#define ag_ct_get_port(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[0], 16, p_val, 16) */
#define ag_ct_set_port(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[0], n_val, 16, 16)

#define ag_ct_get_tx_fwd(n_tag,p_bit_val)   (*(p_bit_val) = AG_READ_BIT_VALUE(&pCtBase[n_tag].a_val[1],0))
#define ag_ct_set_tx_fwd(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_bit_val, 0)
#define ag_ct_set_nat_en(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_bit_val, 1)
#define ag_ct_set_napt_en(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_bit_val, 2)
#define ag_ct_set_ttl_en(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_bit_val, 3)

/*Raanan Refua 18/08/02*/
#define ag_ct_get_sot(n_tag, p_val)			ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[1],4,p_val,6)
#define ag_ct_set_sot(n_tag, n_val)         ag_set_range_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_val, 4, 6)
#define ag_ct_set_tos_en(n_tag,n_bit_val)   ag_set_bit_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_bit_val, 10)

/*Raanan Refua 12/08/02*/
#define ag_ct_get_hip_pointer(n_tag, p_val) ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[1], 16,p_val,16)
#define ag_ct_set_hip_pointer(n_tag, n_val) ag_set_range_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_val, 16, 16)

/*Raanan Refua 12/08/02*/
#define ag_ct_get_hip_len(n_tag, p_val)     ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[1], 11, p_val, 5)
#define ag_ct_set_hip_len(n_tag, n_val)     ag_set_range_value((AG_U32*)&pCtBase[n_tag].a_val[1], n_val, 11, 5)

#define ag_ct_set_ip(n_tag,n_val)   (pCtBase[n_tag].a_val[2] = (AG_U32)n_val)

#define ag_ct_set_ip_delta(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[3], n_val, 16, 16)
#define ag_ct_set_tui_delta(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[3], n_val, 0, 16)

#define ag_ct_get_grp1(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[4], 11, p_val, 21)
/*#define ag_ct_set_grp1(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[4], n_val, 11, 21) */

/*#define ag_ct_get_grp2(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[4], 0, p_val, 11) */
/*#define ag_ct_set_grp2(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[4], n_val, 0, 11) */

/*#define ag_ct_get_grp3(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5], 16, p_val, 16) */
/*#define ag_ct_set_grp3(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 16, 16) */

/*#define ag_ct_get_grp4(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[6], 20, p_val, 12) */
/*#define ag_ct_set_grp4(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[6], n_val, 20, 12) */

/*#define ag_ct_get_grp5(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[6], 10, p_val, 10) */
/*#define ag_ct_set_grp5(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[6], n_val, 10, 10) */

/*#define ag_ct_get_grp6(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[6], 0, p_val, 10) */
/*#define ag_ct_set_grp6(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[6], n_val, 0, 10) */

/*
Usage example for GPC control:
The below macro enables byte count and frame count of group 1. and disables time stamp
  ag_ct_set_grp1_ctl(n_my_tag , AG_CT_BYTE_CNT_EN | AG_CT_FRM_CNT_EN);
Assuming we do not know the timestamp bit value and we want to preserve it then we must
read the value before writing it back as follows:
  ag_ct_get_grp1_ctl(n_my_tag , &n_current_val);
  ag_ct_set_grp1_ctl(n_my_tag , n_current_val | AG_CT_BYTE_CNT_EN | AG_CT_FRM_CNT_EN);
*/
/*#define ag_ct_get_grp1_ctl(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5], 13, p_val, 3) */
/*#define ag_ct_set_grp1_ctl(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 13, 3) */

/*#define ag_ct_get_grp2_ctl(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5], 10, p_val, 3) */
/*#define ag_ct_set_grp2_ctl(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 10, 3) */

/*#define ag_ct_get_grp3_ctl(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5], 7, p_val, 3) */
/*#define ag_ct_set_grp3_ctl(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 7, 3) */

/*#define ag_ct_get_grp4_ctl(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5],4, p_val, 3) */
/*#define ag_ct_set_grp4_ctl(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 4, 3) */

/*#define ag_ct_get_grp5_ctl(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5], 2, p_val, 2) */
/*#define ag_ct_set_grp5_ctl(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 2, 2) */

/*#define ag_ct_get_grp6_ctl(n_tag,p_val)   ag_read_range_value((AG_U32*)&pCtBase[n_tag].a_val[5], 0, p_val, 2) */
/*#define ag_ct_set_grp6_ctl(n_tag,n_val)   ag_set_range_value ((AG_U32*)&pCtBase[n_tag].a_val[5], n_val, 0, 2) */








#undef AG_EXTERN

#endif  /* CLS_TBL_API_H */
