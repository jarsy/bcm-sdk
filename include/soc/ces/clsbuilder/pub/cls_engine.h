
/*******************************************************************

Copyright Redux Communications Ltd.

Module Name: 

File Name: 
    cls_engine.h

File Description: 

  $Revision: 1.1.2.1 $  - Visual SourceSafe revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    3/26/01
*******************************************************************/
#ifndef CLS_ENGINE_H 
#define CLS_ENGINE_H 

#include "clsb_types_priv.h"

#ifdef CLS_STD_ENGINE_C
	#define CLS_EXTERN
#else
	#define CLS_EXTERN extern
#endif

#include "ag_common.h"

#ifndef BCM_CES_SDK /*Needed bcm sal?*/
#include "bcm_ces_sal.h"
#include "utils/gen_net.h"
#include "utils/gen_reg_acce.h"
#endif  /*BCM_CES_SDK*/

#include "classification/clsb_types.h"
#include "classification/cls_results.h"
#include "cls_stack.h"
#include "cls_mp_handler.h"


#ifdef __cplusplus
extern "C"
{
#endif


/* Externs*/
CLS_EXTERN AgResult add_path_8 (AG_U8 *buf, AG_U8 *ctl_buf ,AG_U32 buf_len, AG_U32 *tag_buf,
                                AG_U32 n_taglen, StdPrg *p_stdprg, ClsbCurrState *curr_state);

CLS_EXTERN AgResult add_path_16( AG_U16 *buf,  AG_U8 *ctl_buf , AG_U8 *msk_buf, 
                                AG_U32 buf_len, AG_U32 *tag_buf,AG_U32 n_taglen,  
                                StdPrg *p_stdprg ,  ClsbCurrState *curr_state);

CLS_EXTERN AgResult validate_delete_path_8 (AG_U8 *buf, AG_U8 *ctl_buf, AG_U32 buf_len , 
                            StdPrg *p_stdprg, Leaf* p_leaf, ClsbDelPoint *p_del_point);
CLS_EXTERN void delete_path_8(AG_U8 *buf, AG_U8 *ctl_buf, AG_U32 buf_len, 
                             StdPrg *p_stdprg , ClsbDelPoint *p_del_point);

CLS_EXTERN AgResult delete_path_16( AG_U16 *buf,  AG_U8 *ctl_buf, AG_U8 *msk_buf, AG_U32 buf_len,  
                                   StdPrg *p_stdprg , ClsbCurrState *p_currstate, 
                                   AG_U32 *p_erase_pos, AG_U16 *p_ref_count);

CLS_EXTERN void cls_get_leaf_tag_8 (Leaf *p_leaf, AG_U32 *p_tag, AgClsbMem *p_mem_handle); 
CLS_EXTERN void cls_get_leaf_tag_16 (Leaf *p_leaf, AG_U32 *p_tag, AgClsbMem *p_mem_handle);

CLS_EXTERN void cls_set_leaf_tag_8 ( Leaf *p_leaf ,  AG_U32 n_tag, AgClsbMem *p_mem_handle);
CLS_EXTERN void cls_set_leaf_tag_16 ( Leaf *p_leaf ,  AG_U32 n_tag, AgClsbMem *p_mem_handle);

CLS_EXTERN void cls_set_link_8	( Leaf *p_leaf, StdPrg *p_mp, AG_U8 n_opcode);
CLS_EXTERN void cls_set_link_16( Leaf *p_leaf, StdPrg *p_mp, AG_U8 n_opcode);

CLS_EXTERN AG_BOOL clsb_is_valid_to_link( Leaf *p_leaf, AgClsbMem *p_mem, AG_U8 n_opcode);

CLS_EXTERN AgResult dump_8 (const AG_CHAR *output_file , StdPrg *x_std_prg);
CLS_EXTERN AgResult dump_bin_8 (const AG_CHAR *output_file , StdPrg *x_std_prg);
CLS_EXTERN AgResult dump_vl_8 (const AG_CHAR *output_file ,StdPrg *x_std_prg);

CLS_EXTERN AgResult dump_16(const AG_CHAR *output_file , StdPrg *x_std_prg);
CLS_EXTERN AgResult dump_bin_16(const AG_CHAR *output_file , StdPrg *x_std_prg);
CLS_EXTERN AgResult dump_vl_16 (const AG_CHAR *output_file ,StdPrg *x_std_prg);

CLS_EXTERN AgResult find_next_ent_8 (AG_U8 br, AG_U8 n_skip, ClsbCurrState *curr_state, AgClsbMem *p_mem_handle);
CLS_EXTERN AgResult find_next_ent_16 (AG_U16 wr, ClsbCurrState *curr_state, AgClsbMem *p_mem_handle);

CLS_EXTERN void clsb_eng_del_mp_8(StdPrg* p_prg);
CLS_EXTERN void clsb_eng_del_mp_16(StdPrg* p_prg);

CLS_EXTERN void cls_write_to_memory_16( ClsbStack *p_stack, AgClsbMem *p_mem_handle);

CLS_EXTERN void cls_eng_write_leaf_to_memory_8 (ClsbStack *p_stack,  AgClsbMem *p_mem_handle);
CLS_EXTERN void cls_eng_write_until_empty_ent_to_memory_8 (ClsbStack *p_stack,  AgClsbMem *p_mem_handle);
CLS_EXTERN void cls_eng_write_empty_ent_to_memory_8 (ClsbStack *p_stack,  AgClsbMem *p_mem_handle);

CLS_EXTERN AgResult cls_reset_stack_8( ClsbStack *p_stack, StdPrg *p_stdprg);
CLS_EXTERN AgResult cls_reset_stack_16( ClsbStack *p_stack, StdPrg *p_stdprg);

CLS_EXTERN AgResult cls_undo_write_to_mem_8(ClsbStack *p_stack, AG_S32 n_org_top, StdPrg *p_stdprg);

CLS_EXTERN AgResult query_path_8 (StdPrg *p_stdprg, AG_U8 *p_frame, AG_U32 n_frame_len, 
                                  AG_BOOL n_match_flag, AG_U32 *p_tag, AG_U32 *p_pos);
CLS_EXTERN AgResult query_path_16 (StdPrg *p_stdprg, AG_U16 *p_frame, AG_U32 n_frame_len, 
                                   AG_BOOL n_match_flag, AG_U32 *p_tag, AG_U32 *p_pos);

CLS_EXTERN 	AgResult find_empty_rec_8 (ClsbCurrState *curr_state, AgClsbMem *p_mem_handle);
CLS_EXTERN 	AgResult find_empty_rec_16 (ClsbCurrState *curr_state, AgClsbMem *p_mem_handle);
CLS_EXTERN  void clsb_eng_utilization (AgClsbMem *p_mem_handle, 
						   AG_U32 *p_empty_rec, 
						   AG_U32 *p_non_empty_rec, 
						   AG_U32 *p_compr_ratio,
						   AG_U32 *p_def_comp_ratio,
						   AG_U32 *p_total_util);

CLS_EXTERN  void clsb_eng_usage (AgClsbMem *p_mem_handle, 
							AG_U32 *p_empty_rec, 
							AG_U32 *p_non_empty_rec, 
							AG_U32 *p_non_empty_entry_pri,
							AG_U32 *p_non_empty_entry_def);

CLS_EXTERN  AgResult clsb_eng_rec_dump_bin_8 (const AG_CHAR *p_file_name, AgClsbMem *p_mem_handle);
CLS_EXTERN  AgResult clsb_eng_aux_mem_dump_bin_8 (const AG_CHAR *p_file_name, AgClsbMem *p_mem_handle);

CLS_EXTERN  AG_U16 clsb_get_ref_count(AgClsbMem* p_mem, AG_U32 n_rec, AG_U32 n_entry);


#ifdef __cplusplus
} /*end of extern "C"*/
#endif


/*********Constants**********/
#define CLS_ALL_ONES		0xFFFFFFFF

#define AG_CLS_NO_TAG       0xFFFFFFFF
#define AG_CLS_LATE_CLASS (AG_U32) 0x00000002

/*
tags 0,3,4 are used by the engine.
Tag 3: Unclassified. This is the initialization value for all the records 
Tag 4: pushed entry. Needed so the find_next_ent_X funtion will not find the same
       entry again and again.
Tag 0: When a node (record/path-tag) is allocated, its default is set with
       tag 0. Used to mark default entry as occupied but yet unclassified.
*/
#define EMPTY_TAG          0x00000003
#define EMPTY_ENTRY_8	   0xFFE00003  
#define PUSHED_ENTRY_8	   0xFFE00004
#define SKIPPED_ENTRY_8    0xFFE00005
#define OCCUPIED_ENTRY_8   0xFFE00000

#define EMPTY_ENTRY_16	   0xFF008003  /* Bit 15 is always 1 */
#define PUSHED_ENTRY_16	   0xFF008004
/*	#define SKIPPED_ENTRY_16   0xFF000005 */ /* no need for it in 16 bit. */
#define OCCUPIED_ENTRY_16  0xFF008000
#define EMPTY_PT_16		   0xFFFF
#define DEF_MSK_16		   0xF
#define NOT_RELEVANT_FIELD 0xFFFF
#define CARRY_MASK		   0x80000000

#define PRI_TABLE 0
#define DEF_TABLE 1

#define PRG_LAN		0
#define PRG_WAN1	1
#define PRG_WAN2	2

/*16 bits constatnts*/
#define NUM_ENT_PACKAGE_16		4 /*Number of entries  one package*/
#define DEF_PACKAGE_OFFSET_16	3/* Offset of default entry from begin of package*/

/***** Configuration parameters*******/

/*Configuration Parameters - 8 bits only*/
#define MAX_ENTRY_8			256
#define MAX_TABLES_8		2
#define MAX_BIT_ENTRIES_8	8
/* #define REC_SIZE_8	(MAX_TABLES_8*MAX_ENTRY_8*ENT_SIZE) = 2048 bytes*/
#define REC_SIZE_8	AG_CLS_MEM_BASE_SIZE_8
#define TBL_SIZE_8	(MAX_ENTRY_8*ENT_SIZE)              /* 1024 bytes*/

/*Configuration Parameters - 16 bits only*/
#define MAX_ENTRY_16		131072
#define MAX_PRI_ENT_16      65536
#define MAX_DEF_ENT_16      32768
#define MAX_BIT_ENTRIES_16  2048
#define MIN_RES_SKIP_16		1
#define MAX_RES_SKIP_16		255
/* #define REC_SIZE_16     (MAX_ENTRY_16*ENT_SIZE) = 0.5M */
#define REC_SIZE_16     AG_CLS_MEM_BASE_SIZE_16

/*Configuration Parameters - 8 and 16 bits*/
#define MAX_NUM_OF_SKIPS 255
#define ENT_SIZE	sizeof(AG_U32)

/*Limit Adresses for 8 bits - Records*/
#define BASE_PRG_LAN		0
#define END_PRG_LAN			0xff 
#define BASE_PRG_WAN1		0x100
#define END_PRG_WAN1		0x1ff 
#define BASE_PRG_WAN2		0x200
#define END_PRG_WAN2		0x2ff

/*Limit Adresses for 16 bits - Records*/
#define BASE_PRG_LAN_16		0
#define END_PRG_LAN_16		15 
#define BASE_PRG_WAN1_16	16
#define END_PRG_WAN1_16		31 
#define BASE_PRG_WAN2_16	32
#define END_PRG_WAN2_16		47

/* Mask manipulators*/
#define OP_CODE_MASK_8			0xE0000000
#define PATH_TAG_MASK_8			0x1FE00000
#define NEXT_RECORD_MASK_8		0x001FFF00
#define NEXT_PATH_TAG_MASK_8	0x000000FF
#define TAG_MASK_8				0x001FFFFF

#define OP_CODE_MASK_16			0xF0000000
#define PATH_TAG_0_MASK_16		0x0000FFFF
#define PATH_TAG_1_MASK_16		0xFFFF0000
#define NEXT_RECORD_MASK_16		0x00FF0000
#define NEXT_PATH_TAG_MASK_16	0x00007FFF
#define TAG_MASK_16				0x00FF7FFF
#define TAG_MASK0_16			0x00FF0000/*The tag is divided with an empty bit:16*/
#define TAG_MASK1_16			0x00007FFF
#define MSK_MASK_16				0x0F000000


/*DRAM access macros*/ /*MBREC-Every place where the macro was changed the old record direction was saved*/
#define CLS_RECORDS_8(n_base,n_ent,n_tab,n_rec) (*(AG_U32*)(((AG_U32)(n_base))+((n_rec)*REC_SIZE_8)+((n_tab)*TBL_SIZE_8)+((n_ent)*ENT_SIZE)))
#define CLS_RECORDS_16(n_base,n_ent,n_rec)    (*(AG_U32*)(((AG_U32)(n_base))+((n_rec)*REC_SIZE_16)+((n_ent)*ENT_SIZE)))

/* Change memory access by macros */
#define CLS_IS_EXT(n_base,n_ent,n_rec,n_rec_limit)    (*(AG_U32*)(((AG_U32)(n_base))+((n_rec)*sizeof(AG_U32))+(BIT_TABLE_ENTRY(n_ent)*(n_rec_limit)*sizeof(AG_U32))))

/*ref count extention computation: Base + rec * (MAX_ENTRY_8 bits) + (entry/32) * sizeof(AG_U32) */
#define CLS_REF_COUNT_EXT8(n_base,n_ent,n_rec)    (*(AG_U32*)( ((AG_U32)(n_base)) + (n_rec)*BITS_TO_BYTES(MAX_ENTRY_8) + BIT_TABLE_ENTRY(n_ent) * sizeof(AG_U32)))
#define CLS_REF_COUNT_8(n_base,n_ent,n_rec)  (*(AG_U8*)(((AG_U32)(n_base))+((n_rec)*MAX_ENTRY_8)+(n_ent)))

#define CLS_REF_COUNT_16(n_base,n_ent,n_rec,n_rec_limit) (*(AG_U16*)(((AG_U32)(n_base))+((n_rec)*sizeof(AG_U16))+((n_ent)*(n_rec_limit)*sizeof(AG_U16))))

/* the reason that the BIT_TABLE_ENTRY is not applied is that it is applied when passing the parameter */
/* the n_ent parameter in CLS_REC_IMAGE_X macro is already divided by 32. */
#define CLS_REC_IMAGE_8(n_base,n_ent,n_rec) (*(AG_U32*)( ((AG_U32)(n_base)) + (n_rec)*BITS_TO_BYTES(MAX_ENTRY_8) + (n_ent) * sizeof(AG_U32)))
#define CLS_REC_IMAGE_16(n_base,n_ent,n_rec,n_rec_limit) (*(AG_U32*)(((AG_U32)(n_base))+((n_rec)*sizeof(AG_U32))+((n_ent)*(n_rec_limit)*sizeof(AG_U32))))

/*Manipulation Macros*/
#define OPCODE_8(op)	(((AG_U32)(op)) >> 29)
#define PT_8(pt)		(((AG_U32)(pt) & PATH_TAG_MASK_8) >> 21) 
#define NR_8(nr)		(((AG_U32)(nr) & NEXT_RECORD_MASK_8) >> 8)
#define NPT_8(npt)		((AG_U32)(npt) & NEXT_PATH_TAG_MASK_8)

#define OPCODE_16(op)   (((AG_U32)(op)) >> 28)
#define MASK_16(msk)	(((AG_U32)(msk) & MSK_MASK_16)>>24)
#define PT0_16(pt)		((AG_U32)(pt) & PATH_TAG_0_MASK_16) 
#define PT1_16(pt)		(((AG_U32)(pt) & PATH_TAG_1_MASK_16) >> 16) 
#define NR_16(nr)		(((AG_U32)(nr) & NEXT_RECORD_MASK_16) >> 16)
#define NPT_16(npt)		((AG_U32)(npt) & NEXT_PATH_TAG_MASK_16)
#define TG_16(tag)      ((((AG_U32)(tag) & TAG_MASK0_16)>>1) | ((AG_U32)(tag) & TAG_MASK1_16))


#define MAKE_OPCODE_8(op)	((((AG_U32)(op)) << 29) | ~OP_CODE_MASK_8)
#define MAKE_PT_8(pt)		((((AG_U32)(pt)) << 21) | ~PATH_TAG_MASK_8)
#define MAKE_NR_8(nr)		((((AG_U32)(nr)) << 8) | ~NEXT_RECORD_MASK_8)
#define MAKE_NPT_8(npt)		(((AG_U32)(npt)) | ~NEXT_PATH_TAG_MASK_8)
#define MAKE_TAG_8(tag)		((AG_U32)(tag) | ~TAG_MASK_8)

#define MAKE_OPCODE_16(op)	((((AG_U32)(op)) << 28) | ~OP_CODE_MASK_16)
#define MAKE_MASK_16(msk)		((((AG_U32)(msk)) << 24) | ~MSK_MASK_16)
#define MAKE_PT0_16(pt)		((AG_U32)(pt)) 
#define MAKE_PT1_16(pt)		(((AG_U32)(pt)) << 16)
#define MAKE_NR_16(nr)		((((AG_U32)(nr)) << 16) | ~NEXT_RECORD_MASK_16)
#define MAKE_NPT_16(npt)	(((AG_U32)(npt)) | ~NEXT_PATH_TAG_MASK_16)
#define MAKE_TAG_16(tag)	((((AG_U32)(tag) <<1)| ~TAG_MASK0_16) & ((AG_U32)(tag)|~TAG_MASK1_16))

#define ENTRY(x,y) ((AG_U8)((x) + (y)))


/*MB16 - addres calculation for 16 bits*/
#define DEF_ENTRY_NUM_16(pt)	((AG_U32)(pt)*NUM_ENT_PACKAGE_16+DEF_PACKAGE_OFFSET_16)
#define DEF_PT(ent_num)
#define PRI_ENTRY_NUM_16(wr,pt) ((((AG_U32)((wr+pt)/2)*NUM_ENT_PACKAGE_16)%MAX_ENTRY_16+((wr+pt)%2+1)))
#define PRI_PT_16(wr,ent_num)		((ent_num/NUM_ENT_PACKAGE_16)*2+(ent_num+1)%2-wr)
#define PRI_WR_16(pt,ent_num)		((ent_num/NUM_ENT_PACKAGE_16)*2+(ent_num+1)%2-pt)


/*MBEXT - for keep track of bitwise tableds*/
#define BITS_TO_BYTES(x)    ((AG_U32)(x)>>3)
#define BIT_TABLE_ENTRY(x)	((AG_U32)((x)>>5))
#define BIT_ENTRY_POS(x)	((AG_U32)(1<<((x) & 0x1f)))
#define CALC_CARRY(x)		((AG_U32)(((x) & CARRY_MASK)>>31))
#define IS_EXTENDED_8(n_base,ent,rec,n_rec_limit)	(((AG_U32)CLS_IS_EXT(n_base,ent,rec,n_rec_limit))&(BIT_ENTRY_POS(ent)))
#define IS_EXTENDED_16(n_base,ent,rec,n_rec_limit) (((AG_U32)CLS_IS_EXT(n_base,(ent/2),rec,n_rec_limit))&(BIT_ENTRY_POS(ent/2)))

/* Memory size to allocation */
#define BIT_TABLE_ENTRY_GEN(x)  (((x)&0x1F)?(((x)>>5)+1):((x)>>5))  

/*the size of a bit array with record_limit * MAX_ENTRY_8 bits */
#define AG_CLSB_BIT_ARRAY_SIZE_8(p_mem) (sizeof(AG_U32) * BIT_TABLE_ENTRY(MAX_ENTRY_8) * (p_mem)->n_record_limit)

#endif  /* CLS_ENGINE_H */
