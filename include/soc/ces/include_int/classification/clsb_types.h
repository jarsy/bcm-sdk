
/*******************************************************************

Copyright Redux Communications Ltd.
 
Module Name:

File Name: 
    cls_types.h

File Description:

$Revision: 1.3 $  - Visual SourceSafe automatic revision number 

*******************************************************************/

#ifndef cls_types_h
#define cls_types_h

#include "ag_common.h"
#include "classification/cls_types.h"


/* well known tags definition moved to cls_typs.h */

#define   AG_CLSB_DEFAULT_VAL          ('d')

/**********************************************
Op codes 
***********************************************/
#define NUM_OF_OPCODES_8    8      /*number of opcodes in 8 bit mode*/

/* Op codes */
#define OPC_KEEPOPC 0xFF

/* common to 8 and 16 bit.*/
#define AG_CLSB_OPC_EXT_CMD             ('e')

/*8 bits opcodes*/
#define AG_CLSB_OPC8_CLASSIFY			0 
#define AG_CLSB_OPC8_SKIP1			    1 
#define AG_CLSB_OPC8_START_MPLS			2
#define AG_CLSB_OPC8_HIGH_NIBBLE 	    3
#define AG_CLSB_OPC8_SOT_OFFSET  	    4
#define AG_CLSB_OPC8_INT_TAG			5 
#define AG_CLSB_OPC8_START_IP			6
#define AG_CLSB_OPC8_TAG		        7


/*16 bits opcodes*/
#define AG_CLSB_OPC16_CLASSIFY			0
#define AG_CLSB_OPC16_SKIP_1	        1
#define AG_CLSB_OPC16_SKIP_2     		2
#define AG_CLSB_OPC16_SKIP_3    		3
#define AG_CLSB_OPC16_SKIP_4     		4
#define AG_CLSB_OPC16_SKIP_5    		5
#define AG_CLSB_OPC16_SKIP_6    		6
#define AG_CLSB_OPC16_SKIP_7    		7
#define AG_CLSB_OPC16_SKIP_8    		8
#define AG_CLSB_OPC16_SOT_OFFSET		9
#define AG_CLSB_OPC16_START_MPLS_1		10
#define AG_CLSB_OPC16_START_MPLS_2		11
#define AG_CLSB_OPC16_INT_TAG			12
#define AG_CLSB_OPC16_START_IP_1		13
#define AG_CLSB_OPC16_START_IP_2		14
#define AG_CLSB_OPC16_TAG				15


/************************************************
mini program Id
*************************************************/
typedef AG_U32 MpId;  

#define INVALID_MP_ID ((MpId)AG_MAX_U32)

/************************************************
Leaf Group Id
*************************************************/
typedef AG_U32 LeafGrpId;  

/*Entry Type*/
typedef AG_U32 ClsEntry;

#define INVALID_GRP_ID ((LeafGrpId)AG_MAX_U32)
#define AG_CLSB_IGNORE_GRP    ((LeafGrpId)0xFFFFFFFE) 

#define AG_CLS_BIT_MODE_8  0
#define AG_CLS_BIT_MODE_16 1

/* Irena 7/6/2001 MLT PRG */
#define AG_CLS_MEM_BASE_SIZE_8    0x800    /* 2K  */
#define AG_CLS_MEM_BASE_SIZE_16   0x80000 /* 0.5M */

/* 16MB is the maximum memory area size*/
#define AG_CLS_MAX_MEM_SIZE_8    0x1000000    

/* 128MB is the maximum THEORITICAL memory area size. of course since this is the
DRAM size limit, it is practically never reached.*/
#define AG_CLS_MAX_MEM_SIZE_16   0x8000000    
 
typedef struct AgClsbHwMemUtil_S
{
   AG_U32 n_empty_rec; 
   AG_U32 n_non_empty_rec;
   AG_U32 n_pri_compr_ratio;
   AG_U32 n_def_compr_ratio;
   AG_U32 n_comp_ratio;
} AgClsbHwMemUtil;


typedef struct AgClsbHwMemUse_S
{
   AG_U32 n_empty_rec; 
   AG_U32 n_non_empty_rec;
   AG_U32 n_non_empty_entry_pri;
   AG_U32 n_non_empty_entry_def;
} AgClsbHwMemUse;

/* The types of this structure (AG_U16 and AG_U8) are assumed in the dump_bin
and load_file functions (in the case of if(!b_little_endian).*/
struct PrgRoot_S 
{
	AG_U16              def_pt;
	AG_U16              def_rec;
	AG_U8               def_mask;
	AG_U8               n_cls_def_opc; 
};

typedef struct PrgRoot_S PrgRoot;

typedef AG_U32 AgClsbMemHndl;
#define AG_CLSB_INVALID_MEM_HANDLE  0xFFFFFFFF

/*used for the ag_clsb_create_std_prg function (the n_static_flag parameter)*/
enum AgClsbPrgType_E
{
    AG_CLSB_STATIC_PRG = 0,
    AG_CLSB_DYNAMIC_PRG = 1
};
typedef enum AgClsbPrgType_E AgClsbPrgType;

/* Match Flag for "query path" function */
#define CLSB_QUERY_PATH_MATCH    0
#define CLSB_QUERY_PATH_REG      1  

#ifdef CLS_TEST_FULL_TRACE

extern FILE *full_trace;  /* The output file contains full trace of test */

#endif /* CLS_TEST_FULL_TRACE */

#endif
