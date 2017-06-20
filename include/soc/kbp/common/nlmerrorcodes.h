/*
 * $Id: nlmerrorcodes.h,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 #ifndef INCLUDED_NLMERRORCODES_H
#define INCLUDED_NLMERRORCODES_H

/* Error code definitions */
#define NLMERR_OK           (0)
#define NLMERR_FAIL         (1)
#define NLMERR_NULL_PTR     (2)
#define NLMERR_OPERATION_NO_SUPPORT (4)

/* Reason code definitions */
/* NOTE: do not add reason codes in-between within a specific module. Always add it at the end.
  * For e.g., if you want to add a reason code for DevMgr, add it at the end of DevMgr
  * reason codes. The starting value of reason code of a module is 20 more than that of
  * predecessor module.
  */
typedef enum NlmReasonCode_t
{
    NLMRSC_REASON_OK                    = 0,
    NLMRSC_LOW_MEMORY,
    NLMRSC_INVALID_MEMALLOC_PTR,
    NLMRSC_INTERNAL_ERROR,

    /* Simxpt, Xpt specific */
    NLMRSC_INVALID_XPT_PTR              = 21,
    NLMRSC_INVALID_XPT_RQT_PTR,
    NLMRSC_INVALID_XPT_RSLT_PTR,
    NLMRSC_NOFREE_RQST,
    NLMRSC_NORQST_AVBL,
    NLMRSC_NORSLT_AVBL,
    NLMRSC_OPR_FAILURE,

    /* Device Manager specific */
    NLMRSC_INVALID_DEV_PTR              = 41,
    NLMRSC_INVALID_DEVMGR_PTR,
    NLMRSC_INVALID_SHADOWDEV_PTR,
    NLMRSC_INVALID_KEY_PTR,
    NLMRSC_INVALID_SRCH_RSLT_PTR        = 45,
    NLMRSC_INVALID_POINTER,
    NLMRSC_INVALID_OUTPUT,
    NLMRSC_INVALID_INPUT,
    NLMRSC_INVALID_REG_ADDRESS,
    NLMRSC_INVALID_DB_ADDRESS           = 50,
    NLMRSC_INVALID_CB_ADDRESS,
    NLMRSC_INVALID_RANGE_ADDRESS,
    NLMRSC_INVALID_DATA,
    NLMRSC_INVALID_LTR_NUM,
    NLMRSC_INVALID_AB_NUM               = 55,
    NLMRSC_INVALID_BLK_WIDTH,
    NLMRSC_INVALID_AB_INDEX,
    NLMRSC_DUPLICATE_DEVICE,
    NLMRSC_INVALID_PARAM,
    NLMRSC_DEV_MGR_CONFIG_LOCKED        = 60,
    NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED,
    NLMRSC_CASCADE_NOT_EXIST,
    NLMRSC_INVALID_PARENT,
    NLMRSC_INVALID_DEVICE_TYPE,
    NLMRSC_INVALID_NUM_OF_DEVICES       = 65,
    NLMRSC_INVALID_DATA_LENGTH,
    NLMRSC_DATA_LENGTH_ADDRESS_MISMATCH,
    NLMRSC_READONLY_REGISTER,
    NLMRSC_INVALID_DEVICE_ID,
    NLMRSC_INVALID_OPER_MODE,
    NLMRSC_INVALID_BANK,


    /* Generic Table Manager related */
    NLMRSC_INVALID_GENERIC_TM           = 81,
    NLMRSC_INVALID_9K_TM,
    NLMRSC_INVALID_GENERIC_TABLE,
    NLMRSC_INVALID_GTM_BLKS_RANGE,
    NLMRSC_INVALID_APP_CALLBACK         = 85,
    NLMRSC_INVALID_TABLEID,
    NLMRSC_INVALID_SEARCH_ATTRIBUTES,
    NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES,
    NLMRSC_CONFIGURATION_LOCKED,
    NLMRSC_EXCESSIVE_NUM_TABLE_PAR_LTR  = 90,
    NLMRSC_NO_LTR_CONFIGURED,
    NLMRSC_INVALID_BMR,
    NLMRSC_INVALID_RECORD,
    NLMRSC_DEVICE_DATABASE_FULL,
    NLMRSC_TABLE_FULL                   = 95, /*No more entries can be inserted
                                               for the table. There may be free space on the chip */
    NLMRSC_TABLE_LIMIT_EXCEEDED,             /*The number of records in table
                                               is equal to the table size */
    NLMRSC_RECORD_NOTFOUND,
    NLMRSC_INVALID_RANGE_ATTRIBUTES,
    NLMRSC_INVALID_KEY_NUM,
    NLMRSC_INVALID_RESULT_SEQ_NUM       = 100,
    NLMRSC_INVALID_TABLE_WIDTH,
    NLMRSC_INVALID_TABLE_ASSO_WIDTH,
    NLMRSC_SEARCHATTRIB_NOT_DELETED,
    /* nlm12k GTM reason codes */
    NLMRSC_INVALID_BANK_NUM,
    NLMRSC_INVALID_GTM_SB_BLKS_RANGE,

    /* Range Manager Related */
    NLMRSC_INVALID_RANGE_MGR_ATTR       = 121,   /* Indicates invalid
                                                   Range Manager Attributes */
    NLMRSC_INVALID_RANGE_MGR,                    /* Indicates invalid Range Manager,
                                                    either this Range Mgr does not exist or it is NULL*/
    NLMRSC_INVALID_DATABASE,                     /* Indicates invalid Database,
                                                    either this Database does not exist or it is NULL*/
    NLMRSC_DUPLICATE_DATABASE_ID,
    NLMRSC_INVALID_RANGE_DB_ATTR        = 125,   /* Indicates invalid Range
                                                    Database Attributes */
    NLMRSC_INVALID_RANGE,                        /* Indicates Range pointer is NULL */
    NLMRSC_INVALID_MCOR_CONFIG,
    NLMRSC_INVALID_OUTPUT_RNG_PTR,               /* Indicates NULL pointer provided by the
                                                    user to get the range-result */
    NLMRSC_INVALID_OUTPUT_NUM_PTR,               /* Indicates NULL pointer provided by the user
                                                    to get the number of ranges matched in the result.*/
    NLMRSC_DUPLICATE_RANGE_ID           = 130,
    NLMRSC_INVALID_RANGE_SELECTION,
    NLMRSC_RANGE_DATABASE_FULL,
    NLMRSC_NO_SUPPORT_FOR_THIS_COMB,
    NLMRSC_NO_SUPPORT_FOR_SEARCHDB,

    /* FIB Table Manager related */
    NLMRSC_INVALID_TBLID_LEN            = 161,
    NLMRSC_INVALID_FIB_MGR,
    NLMRSC_INVALID_FIB_TBL,
    NLMRSC_DUPLICATE_FIB_TBL_ID,
    NLMRSC_INVALID_FIB_BLKS_RANGE       = 165,
    NLMRSC_INVALID_FIB_TBL_INDEX_RANGE,
    NLMRSC_INVALID_FIB_MAX_PREFIX_LENGTH,
    NLMRSC_INVALID_FIB_START_BIT_IN_KEY,
    NLMRSC_INVALID_PREFIX,
    NLMRSC_INVALID_IDX_SPACE_MGR        = 170,
    NLMRSC_INVALID_IDX_RANGE_PTR,
    NLMRSC_IDX_RANGE_FULL,
    NLMRSC_INVALID_IDX_RANGE_GROW_FAILED,
    NLMRSC_MLP_GROW_FAILED,
    NLMRSC_LOCATE_NODE_FAILED           = 175,
    NLMRSC_DUPLICATE_PREFIX,             
    NLMRSC_PREFIX_NOT_FOUND,                    /* Prefix you are trying to delete is not found */
    NLMRSC_INVALID_PREFIX_LEN,
    NLMRSC_RESOURCE_ALLOC_FAILED,
    NLMRSC_TMAX_EXCEEDED,                       /* Prefix count exceeds tmax value,
                                                   Cynapse internal reason code */

    /* 12k reason codes*/
    NLMRSC_INVALID_IBF_LOCATION         = 201,
    NLMRSC_INVALID_CLK_CYCLE,
    NLMRSC_INVALID_OPERATION,
    
    NLMRSC_INCORRECT_INST_TYPE          = 205,
    NLMRSC_INSTRUCTION_OVERFLOW,
    NLMRSC_INSTRUCTION_ALREADY_SCHEDULED,
    NLMRSC_INVALID_NIP_VALUE,
    NLMRSC_INVALID_RETURN_AD_VALUE,

    NLMRSC_INVALID_RBF_LOC              = 210,
    NLMRSC_INVALID_CAB_LOC,
    NLMRSC_INVALID_SRAM_SHIFT_VALUE,
    NLMRSC_INVALID_SRAM_SHIFT_DIR,
    NLMRSC_INVALID_JUMP_ON_AD,
    
    NLMRSC_INVALID_KEY_SUPERBLK_MAPPING = 215,
    NLMRSC_INVALID_BLK_RESULT_MAPPING,
    NLMRSC_COUNTER_WIDTH_NOT_SUPPORTED,
    NLMRSC_INVALID_COUNTER_CONFIG,
    NLMRSC_INVALID_COUNTER_COMMAND,
    
    NLMRSC_INVALID_DUA_ADDRESS,
    NLMRSC_INVALID_DUA_COMMAND,
    
    NLMRSC_INVALID_AD_FIELD_SELECT = 222,
    NLMRSC_INVALID_BRANCH_WAY,
    NLMRSC_INVALID_LATENCY_CONFIGURATION,

    NLMRSC_INVALID_ADDRESS,

    /* MT reason codes */
    NLMRSC_MT_SPINLOCK_INIT_FAILED = 226,
    NLMRSC_MT_MUTEX_INIT_FAILED,
    NLMRSC_MT_SEM_INIT_FAILED,
    NLMRSC_MT_SPINLOCK_DESTROY_FAILED,
    NLMRSC_MT_MUTEX_DESTROY_FAILED,
    NLMRSC_MT_SEM_DESTROY_FAILED,
    NLMRSC_MT_TRIENODE_DOESNOT_EXIST,

    NLMRSC_MT_INVALID_THREAD_ID,
    NLMRSC_MT_THREAD_ALREADY_REGISTERED,


    /*MAC table related reason codes */
    NLMRSC_INVALID_MAC_TM,
    NLMRSC_INVALID_MAC_SB_BLKS_RANGE,
    NLMRSC_INVALID_MAC_TBL,
    NLMRSC_INVALID_MAC_TBL_TYPE,
    NLMRSC_INVALID_MAC_START_BYTE_IN_KEY,
    NLMRSC_INVALID_MAC_AD_WIDTH,
    NLMRSC_INVALID_MAC_AD,

    /* Port/SMT related reason codes */
    NLMRSC_INVALID_PORT_NUM,
    NLMRSC_INVALID_SMT_NUM,
    NLMRSC_INVALID_INST_TYPE,

    /* vertical partitioning codes */
    NLMRSC_INVALID_AC2BANK_MAPPING,
    NLMRSC_INVALID_P1CTXID_SHIFT_VALUE,

    /* invalid UDA SB Range */
    NLMRSC_INVALID_UDA_SB_BLKS_RANGE,

    /*Other reason codes */
    NLMRSC_REASON_UNKNOWN,
    NLMRSC_API_NOT_SUPPORTED,
    NLMRSC_NO_RECORDS_FOUND,
    NLMRSC_REASON_END

}NlmReasonCode;

#endif
/* */
