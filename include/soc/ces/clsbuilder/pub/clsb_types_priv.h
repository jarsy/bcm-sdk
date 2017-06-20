
/*******************************************************************

Copyright Redux Communications Ltd.

Module Name: 

File Name: 
    clsb_types_priv.h

File Description: 

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    08/05/01
	4/11/02			Raanan Refua	p_ones_pos_in_pattern added for more efficient pattern matching.
*******************************************************************/
#ifndef CLSB_TYPES_PRIV_H 
#define CLSB_TYPES_PRIV_H 

#include "ag_common.h"
#include "classification/clsb_types.h"
#include "utils/List.h"
#ifdef BCM_CES_SDK /*Needed bcm sal?*/
#include "bcm_ces_sal.h"
#else
#include "agos/agos.h"
#endif

#ifdef AG_MNT
    #define CLS_PRINTF(_a_) (printf _a_)
#else
    #define CLS_PRINTF(_a_) 
#endif /* AG_MNT */

#ifndef CLS_8_BIT
	#define CLS_8_BIT /* set mode to 8 bit always. at this stage, 16 bit is not working correctly. */
#endif

#define CLSB_VALID_MEM_HNDL(_n_mem_handle)  ((_n_mem_handle)<nMaxMem && aClsMem[_n_mem_handle])

/*stack size should be twice the size of the maximum number of copied entries + 
the maximal path length. */
#define MAX_OLD_STACK_LEN_16 33000 /*The maximum stack size for 16 bit mode (CHECK SIZE) */
#define MAX_NEW_STACK_LEN_16 17000 /*The maximum stack size for 16 bit mode */

#define MAX_OLD_STACK_LEN_8  300 /*The maximum stack size for 8 bit mode  */
#define MAX_NEW_STACK_LEN_8  1700 /*The maximum stack size for 8 bit mode  */

#define AG_CLSB_OPC8_EXT_CMD			5 
#define AG_CLSB_OPC16_EXT_CMD			12 

#ifdef CLS_MP_HANDLER_C
#define AG_MP_EXTERN
#else
#define AG_MP_EXTERN extern
#endif

AG_MP_EXTERN AG_U32 nCurrGrpAlloc;
AG_MP_EXTERN AG_U32 nCurrMpAlloc;

#ifdef AG_MEM_MONITOR
AG_MP_EXTERN AG_U32 nCurrLeafAlloc;
AG_MP_EXTERN AG_U32 nMaxLeafAlloc;
AG_MP_EXTERN AG_U32 nMaxGrpAlloc;
AG_MP_EXTERN AG_U32 nCurrLeafPtrAlloc;
AG_MP_EXTERN AG_U32 nMaxLeafPtrAlloc;
AG_MP_EXTERN AG_U32 nCurrGrpPtrAlloc;
AG_MP_EXTERN AG_U32 nMaxGrpPtrAlloc;
AG_MP_EXTERN AG_U32 nMaxMpAlloc;
#endif 

#ifdef CLSB_API_C
#define AG_CLSB_EXTERN
#else
#define AG_CLSB_EXTERN extern
#endif
AG_CLSB_EXTERN AG_BOOL bClsbValidate;


struct ClsbDelPoint_S
{
    AG_U32 n_rec;
    AG_U32 n_pt;
    AG_U32 n_pos;
    AG_U8  n_mask;
};
typedef struct ClsbDelPoint_S ClsbDelPoint;


/***********************************************
Mini program node update structure
***********************************************/
struct MpNodeUpdate_S
{
	AG_U32 		n_old_rec;
	AG_U32	    n_old_pt;
	AG_U32 		n_new_rec;
	AG_U32	    n_new_pt;
    AG_BOOL     b_leaves; /* this node has leaves. */
    AG_BOOL     b_root;   /* this is the root node. */

};
typedef struct MpNodeUpdate_S MpNodeUpdate;

/* The Stack Element */
struct ClsbStackElement_S
{

	AG_U32 n_entry_value;/*The value to be written to the record */
	AG_U32 n_entry_num;/*Pointer to the entry in table where the value should be written */
	AG_U16 n_entry_path_tag;/*The path tag of the entry in case of 16 bit mode */
	AG_U16 n_ref_count;/*The refcount of the path tag in case of zeroing the stack */
	AG_U16 n_record;/*The record the entry must be written to */
	AG_U8  n_table;/*If is primary or default for 8 bits */

};
typedef struct ClsbStackElement_S ClsbStackElement;

/* The Stack Structure */
struct ClsbStack_S
{
    /* The stack array is allocated in the create hw mem function*/
    ClsbStackElement *a_stack;  
	AG_S32 n_size; /*The top position */
	AG_S32 n_top; /*The top position */

};
typedef struct ClsbStack_S ClsbStack;

struct AgClsbMem_S
{
	AG_U32   n_record_limit;      /* Max number of records */
	AG_U16   n_low_limit_record;  /* Low number of records. Always 0 for 8 bit */
	AG_U32   n_cls_mode;          /* Classification mode 8/16 bits */
	AG_U32   n_last_record;       /* Last record used by this cls prg */
	AG_U32   *p_record_base;     /* Array of classification records */

    ClsbStack x_old_stack;          /*stack for add path operation*/
	ClsbStack x_new_stack;          /*stack for add path operation*/
   
	/* Auxiliary arrays */
	void     *p_ref_count;        /* Array of reference counts (AG_U8* in 8 bit mode and
                                     AG_U16* in 16 bit mode*/
    AG_U32   *p_ref_ext;          /* extension of 1 bit for the ref count.*/
	AG_U32   *p_extend;           /* Extended commands array */
	AG_U32   *p_bit_impression;   /* Array of entries impression to be copy */
	/* Raanan Refua 4/11/2002*/
	AG_U32	 *p_ones_pos_in_pattern; /*Equivalent compact presentation of data held by p_bit_impression*/
	AG_U32   *p_record_image;     /* Array of record images */
	AG_U32   *p_static_rec_image; /* Bit mask of records */
	AG_U32   *p_entry_to_copy;    /* Used by copy entries */

	void     *p_empty_entries;    /* Number of empty entries in record, 
								     (AG_U16* in 8 bit mode and AG_U32* in 16 bit mode*/

	MpNodeUpdate x_node_update;
	AG_U32	     n_update_size;

	AgClsbMemHndl x_mem_hndl;    /* Cls Prg memory handle */

	AgosBiSemaphoreInfo  x_semaphore;  /* Semaphore to synchronize access to program memory */
    /*BCM-CLS: add Device of this handel*/
    void *h_device; /*if value is CLS_MEM_HANDEL_CPU, not in CES memory*/
};

typedef struct AgClsbMem_S AgClsbMem;


struct StdPrg_S 
{
	AgClsbMem   *p_mem_handle;
	PrgRoot     x_root;
/* 	AG_BOOL     str_prg; */

};
typedef struct StdPrg_S StdPrg;

struct ClsbCurrState_S {
	AG_U16 curr_rec;
    AG_U16 next_rec ;
	AG_U16 curr_pt;
    AG_U16 next_pt  ;
	AG_U16 def_pt;
	AG_U16 def_rec ;
	AG_U8  def_mask;    
	AG_U8 curr_table; 
	AG_U16	curr_data ;
	AG_U32 curr_entry_num;  /* used by copy entries to push root entry into stack */
	AG_U32 next_entry_num;  /* used by copy entries  */
	AG_BOOL b_is_extended;  
	AG_U32 n_num_of_ent_to_copy; /* used by copy entries */

};
typedef struct ClsbCurrState_S ClsbCurrState;

/************************************************
Table Type 
*************************************************/
enum TableType_E
{
	CLS_PRI_TBL = 0,	/* primary table */
	CLS_DEF_TBL	= 1 	/* default table */
};
typedef enum TableType_E TableType;


/************************************************
 The Mini Program full description Structure
*************************************************/
struct MiniPrgDesc_S
{
	/* The Mini Prog object */
	StdPrg 	    x_stdprg;

    AG_U32      n_properties;

	/* List of Leaves */
	List		x_leaves;

	/* List of leaf groups that are linked to this mini-program.*/
	List		x_linked_grps;

};
typedef struct MiniPrgDesc_S MiniPrgDesc;

struct LeafGrp_S;

/***********************************************
The leaf structure
***********************************************/
struct Leaf_S
{
	MiniPrgDesc*        p_mp; /* the MP it belongs to */
	struct LeafGrp_S*	p_grp; /* the leaf group it belongs to */
	AG_U32 	            n_rec;
	AG_U32	            n_pt;
	AG_U16	            n_data;
	TableType	        e_table;
/*	AG_U32	            n_cls_tag; */

};
typedef struct Leaf_S Leaf;

/***********************************************
The LeafPtrEl (Leaf Pointer Element) structure:
used for linking leaves in Lists.
pointer is used because each leaf is linked
in 2 lists (MiniPrgDesc and LeafGroup)
***********************************************/
struct LeafPtrEl_S
{
    /* The ListElement MUST be the first element in the struct */
	/* casting from ListElement* to LeafPtrEl* depends on it. */
	ListElement x_element;
    
	Leaf        *p_leaf;
};
typedef struct LeafPtrEl_S LeafPtrEl;

/***********************************************
 The leaf group Structure
***********************************************/
struct LeafGrp_S
{
    /* the id of this group */
/*    AG_U32    x_grp_id; */

    /* the id of this group */
/*    AG_U32    n_table_entry; */

    /* the MP this group is linked to */
    MiniPrgDesc	*p_linked_mp;

	/* list of Leaves */
    List	x_leaves;
    
    /* The memory area of the program */
	AgClsbMem*  p_mem_handle;

    /* TRUE if the gruop is linked. FALSE otherwise */
    AG_U8	b_linked;

    /* enable unlink operation if possible */
    AG_U8    b_enable_unlink;

	/* the opcode that is used for the link */
    AG_U8      n_opcode;

    AG_U8      n_ref_count;

};
typedef struct LeafGrp_S LeafGrp;


/***********************************************
The LeafGrpPtrEl (LeafGroup Pointer Element) structure:
used for linking leafGroups in Lists.
***********************************************/
struct LeafGrpPtrEl_S
{
    /* The ListElement MUST be the first element in the struct */
	/* casting from ListElement* to LeafGrpPtrEl* depends on it. */
	ListElement x_element;
    
	LeafGrp*   p_grp;
};
typedef struct LeafGrpPtrEl_S LeafGrpPtrEl;

AG_CLSB_EXTERN AgClsbMem **aClsMem; /*array of AgClsbMem pointers*/
AG_CLSB_EXTERN AG_U32 nMaxMem;  /*The maximal number of memory areas that can exist in the
                       system concurrently*/




#undef AG_MP_EXTERN
#undef AG_CLSB_EXTERN

#endif  /* CLSB_TYPES_PRIV_H */
