/* $Id: sand_error_code.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/




#include <shared/bsl.h>

#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>

#ifdef _MSC_VER
/* $Id: sand_error_code.c,v 1.19 Broadcom SDK $
 * In Microsoft:
 * cancel the warning of internal int to unsigned
 * The S/GET_FLD_IN_PLACE cause it
 */
#pragma warning(disable:4308)
#endif /* _MSC_VER */

/* After adding error/proc pool, go over it again and search for duplicates */
#define DNX_SAND_ERROR_CODE_FIND_DUPLICATES 1

/*
 * List of DNX_SAND procedures. To be copied and sorted
 * using dnx_sand_add_proc_id_pool()
 */
CONST static
  DNX_PROCEDURE_DESC_ELEMENT
    Dnx_soc_sand_procedure_desc_element[]=
{
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MODULE_OPEN),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MODULE_CLOSE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PHYSICAL_READ_FROM_CHIP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PHYSICAL_WRITE_TO_CHIP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_READ_MODIFY_WRITE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DEVICE_REGISTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DEVICE_UNREGISTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DEVICE_MNGMNT_LOAD_CHIP_VER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HANDLES_INIT_HANDLES),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HANDLES_SHUT_DOWN_HANDLES),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HANDLES_REGISTER_HANDLE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HANDLES_UNREGISTER_HANDLE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HANDLES_UNREGISTER_ALL_DEVICE_HANDLES),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_CLEAR_CHIP_DESCRIPTOR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INIT_CHIP_DESCRIPTORS),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELETE_CHIP_DESCRIPTORS),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_REMOVE_CHIP_DESCRIPTOR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ADD_CHIP_DESCRIPTOR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_REGISTER_EVENT_CALLBACK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_UNREGISTER_EVENT_CALLBACK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_COMBINE_2_EVENT_CALLBACK_HANDLES),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_MUTEX_OWNER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SET_CHIP_DESCRIPTOR_MUTEX_OWNER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_MUTEX_COUNTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SET_CHIP_DESCRIPTOR_MUTEX_COUNTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INC_CHIP_DESCRIPTOR_MUTEX_COUNTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DEC_CHIP_DESCRIPTOR_MUTEX_COUNTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TAKE_CHIP_DESCRIPTOR_MUTEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GIVE_CHIP_DESCRIPTOR_MUTEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_BASE_ADDR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SET_CHIP_DESCRIPTOR_VER_INFO),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_MEMORY_SIZE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_INTERRUPT_CALLBACK_ARRAY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_UNMASK_FUNC),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DEC_CHIP_DESCRIPTOR_INT_MASK_COUNTER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_IS_CHIP_DESCRIPTOR_CHIP_VER_BIGGER_EQ),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_CHIP_VER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_DBG_VER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_DECREASE_TIME_FROM_SECOND_ITEM),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DEVICE_MNGMNT_GET_DEVICE_TYPE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SSR_GET_VER_FROM_HEADER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SSR_GET_SIZE_FROM_HEADER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_CHIP_TYPE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_IS_CHIP_DESCRIPTOR_VALID),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_MAGIC),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GENERAL_SET_MAX_NUM_DEVICES),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MODULE_INIT_ALL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MODULE_END_ALL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_SEND_MESSAGE_TO_Q_FROM_TASK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_SEND_MESSAGE_TO_Q_FROM_INT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_CALLBACK_ENGINE_START),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_REGISTER_POLLING_CALLBACK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_UNREGISTER_POLLING_CALLBACK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_USER_CALLBACK_UNREGISTER_DEVICE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_UNPACK_RX_SR_DATA_CELL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PACK_TX_SR_DATA_CELL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_SET_INFO),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_CLEAR_INFO),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_CLEAR_INFO_ALL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_CHECK_REQUEST_LEGAL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_VERIFY_TRIGGER_0),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_WRITE_ADDRESS),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_WRITE_VALUE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_ASSERT_TRIGGER_1),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_READ_RESULT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MEM_INTERRUPT_MASK_ADDRESS_CLEAR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MEM_READ),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MEM_WRITE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TRIGGER_VERIFY_0),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TRIGGER_ASSERT_1),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_TAKE_MUTEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_GIVE_MUTEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_INSERT_D),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_DECREASE_TIME_FROM_HEAD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELTA_LIST_REMOVE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INIT_ERRORS_QUEUE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SET_USER_ERROR_HANDLER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_LOAD_ERRORS_QUEUE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DELETE_ERRORS_QUEUE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ERROR_HANDLER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_UNLOAD_ERRORS_QUEUE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_CALLBACK_ENGINE_MAIN),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MEM_READ_CALLBACK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_READ_FROM_CHIP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_WRITE_TO_CHIP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_READ_FIELD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_WRITE_FIELD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GET_CHIP_DESCRIPTOR_DEVICE_AT_INIT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SET_CHIP_DESCRIPTOR_DEVICE_AT_INIT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TCM_CALLBACK_DELTA_LIST_TAKE_MUTEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_BITSTREAM_SET_ANY_FIELD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_BITSTREAM_GET_ANY_FIELD),

  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_IP_ADDR_NUMERIC_TO_STRING),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_IP_ADDR_STRING_TO_NUMERIC),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_WRITE_IND_INFO_LOW),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_INDIRECT_READ_IND_INFO_LOW),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TBL_READ),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TBL_READ_UNSAFE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TBL_WRITE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_TBL_WRITE_UNSAFE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_CLEAR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_CREATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_GET_NEXT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_ALLOC_NEXT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_PRINT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_GET_NEXT_HELPER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_OCCUP_STATUS_SET),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_OCCUP_STATUS_SET_HELPER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_OCC_BM_INIT),
 /*
  * hash table
  */
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_CREATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_ENTRY_LOOKUP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_ENTRY_ADD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_ENTRY_REMOVE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_ENTRY_REMOVE_BY_INDEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_GET_NEXT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_PRINT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_HASH_TABLE_FIND_ENTRY),
 /*
  * Group Member Linked List
  */
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_CREATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_CLEAR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBERS_SET),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBER_ADD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBER_REMOVE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBERS_GET),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBER_GET_GROUP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBER_CLEAR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_MEMBER_PRINT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_GROUP_MEM_LL_FUNC_RUN),
/*
 * memory management
 */
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_CREATE),

  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_FREE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_READ),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_WRITE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_READ_BLOCK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_CLEAR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_ARR_MEM_ALLOCATOR_BLOCK_SIZE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_IPV4_SUBNET_VERIFY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_STRUCT_TO_LONG),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_LONG_TO_STRUCT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_STRING_PARSE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_INC),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_ADD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_SUB),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_REVERSE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PP_MAC_ADDRESS_ARE_EQUAL),
 /*
  * Multi Set
  */
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_CREATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_MEMBER_LOOKUP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_MEMBER_ADD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_MEMBER_REMOVE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_MEMBER_REMOVE_BY_INDEX),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_GET_NEXT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_MULTI_SET_PRINT),
 /*
  * Sorted List
  */
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_CREATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_LOOKUP),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_ADD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_UPDATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_REMOVE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_GET_NEXT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_GET_PREV),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_PRINT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_FIND_MATCH_ENTRY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_ADD_BY_ITER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_REMOVE_BY_ITER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_ENTRY_VALUE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_SORTED_LIST_GET_FOLLOW),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_CREATE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_DESTROY),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_CLEAR),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_NODE_ADD),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_NODE_REMOVE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_LPM_GET),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_GET_BLOCK),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_GET_SIZE),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PAT_TREE_PRINT),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PACK_DEST_ROUTED_DATA_CELL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_UNPACK_DEST_ROUTED_DATA_CELL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_PACK_SOURCE_ROUTED_DATA_CELL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_UNPACK_SOURCE_ROUTED_DATA_CELL),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_DATA_CELL_TO_BUFFER),
  DNX_PROCEDURE_DESC_ELEMENT_DEF(DNX_SAND_BUFFER_TO_DATA_CELL),
  
  /* Last element. Do not touch. */
  DNX_PROCEDURE_DESC_ELEMENT_DEF_LAST
} ;

/*
 * List of DNX_SAND errors. To be copied and sorted
 * using dnx_sand_add_error_pool()
 */
CONST static
  DNX_ERROR_DESC_ELEMENT
    Dnx_soc_sand_error_desc_element[] =
{
  {
    DNX_SAND_NO_ERR,
    "DNX_SAND_NO_ERR",
    "No error. OK indication for all modules.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_GEN_ERR,
    "DNX_SAND_GEN_ERR",
    "General error code (unspecified).",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MALLOC_FAIL,
    "DNX_SAND_MALLOC_FAIL",
    "Failure trying to do memory allocation.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MUTEX_CREATE_FAIL,
    "DNX_SAND_MUTEX_CREATE_FAIL",
    "Failure trying to do mutex allocation.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_NULL_POINTER_ERR,
    "DNX_SAND_NULL_POINTER_ERR",
    "NULL pointer was given in a forbidden context.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_NULL_USER_CALLBACK_FUNC,
    "DNX_SAND_NULL_USER_CALLBACK_FUNC",
    "One of the driver-usr callback function that was given\n\r"
    "to the driver was NULL function.\n\r"
    "E.g., 'fap10m_register_device()' reset_device_ptr was set to NULL\n\r"
    " and 'fap10m_reset_device()' was called.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR,
    "DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR",
    "Tried to set or illegal/unknown device identifier.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_DRIVER_NOT_STARTED,
    "DNX_SAND_DRIVER_NOT_STARTED",
    "Driver has not been started. Can not activate "
    "driver services.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_DRIVER_ALREADY_STARTED,
    "DNX_SAND_DRIVER_ALREADY_STARTED",
    "Driver has already been started. Can not activate "
    "driver again.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_DRIVER_BUSY,
    "DNX_SAND_DRIVER_BUSY",
    "Just a message: Driver is currently busy. Try\r\n"
    "again, after timeout, until driver gets free.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ILLEGAL_DEVICE_ID,
    "DNX_SAND_ILLEGAL_DEVICE_ID",
    "Illegal/unknown device identifier as input parameter",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ILLEGAL_CHIP_DESCRIPTOR,
    "DNX_SAND_ILLEGAL_CHIP_DESCRIPTOR",
    "Illegal chip descriptor for specified device handle.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TRYING_TO_ACCESS_DELETED_DEVICE,
    "DNX_SAND_TRYING_TO_ACCESS_DELETED_DEVICE",
    "Illegal chip descriptor for specified device handle\r\n"
    "because it has been deleted.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SYSTEM_TICK_ERR_01,
    "DNX_SAND_SYSTEM_TICK_ERR_01",
    "currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TCM_TASK_PRIORITY_ERR_01,
    "DNX_SAND_TCM_TASK_PRIORITY_ERR_01",
    "currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MIN_TIME_BETWEEN_POLLS_ERR_01,
    "DNX_SAND_MIN_TIME_BETWEEN_POLLS_ERR_01",
    "currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TCM_MOCKUP_INTERRUPTS_ERR_01,
    "DNX_SAND_TCM_MOCKUP_INTERRUPTS_ERR_01",
    "currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_USER_ADD_SAND_ERRORS_ERR_01,
    "DNX_SAND_USER_ADD_SAND_ERRORS_ERR_01",
    "Procedure dnx_sand_add_sand_errors() returned with\r\n"
    "error indication in dnx_sand_module_open().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_USER_ADD_SAND_PROCEDURES_ERR_01,
    "DNX_SAND_USER_ADD_SAND_PROCEDURES_ERR_01",
    "Procedure dnx_sand_add_sand_procedures() returned with\r\n"
    "error indication in dnx_sand_module_open().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_USER_ERROR_HANDLER_ERR_01,
    "DNX_SAND_USER_ERROR_HANDLER_ERR_01",
    "Procedure dnx_sand_set_user_error_handler() returned with\r\n"
    "error indication in dnx_sand_module_open().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MODULE_END_ALL_ERR_01,
    "DNX_SAND_MODULE_END_ALL_ERR_01",
    "Procedure dnx_sand_module_end_all() returned with\r\n"
    "error indication in dnx_sand_module_open().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MODULE_INIT_ALL_ERR_01,
    "DNX_SAND_MODULE_INIT_ALL_ERR_01",
    "Procedure dnx_sand_module_init_all() returned with\r\n"
    "error indication in dnx_sand_module_open().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_INTERRUPT_CLEAR_ALL_ERR_01,
    "DNX_SAND_INTERRUPT_CLEAR_ALL_ERR_01",
    "Procedure dnx_sand_mem_interrupt_mask_address_clear_all()\r\n"
    "returned with error indication in dnx_sand_module_open().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_INIT_CHIP_DESCRIPTORS_ERR_01,
    "DNX_SAND_INIT_CHIP_DESCRIPTORS_ERR_01",
    "dnx_sand_init_chip_descriptors failed.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_HANDLES_INIT_ERR_01,
    "DNX_SAND_HANDLES_INIT_ERR_01",
    "dnx_sand_handles_init_handles failed.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TCM_CALLBACK_ENGINE_START_ERR_01,
    "DNX_SAND_TCM_CALLBACK_ENGINE_START_ERR_01",
    "dnx_sand_tcm_callback_engine_start failed.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_INDIRECT_SET_INFO_ERR_01,
    "DNX_SAND_INDIRECT_SET_INFO_ERR_01",
    "dnx_sand_indirect_clear_info_all failed.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_INDIRECT_MODULE_NOT_EXIST,
    "DNX_SAND_INDIRECT_MODULE_NOT_EXIST",
    "Tried to access not exist module in the indirect zone.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_REGISTER_DEVICE_001,
    "DNX_SAND_REGISTER_DEVICE_001",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "Maximal number of devices has already been\r\n"
    "registered. Can not register more.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_DEPRECATED_ERROR_00001,
    "DNX_SAND_DEPRECATED_ERROR_00001",
    "Deprecated Error - do not use.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_REGISTER_DEVICE_003,
    "DNX_SAND_REGISTER_DEVICE_003",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "No live device has been found trying to access specified memory.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_REGISTER_DEVICE_004,
    "DNX_SAND_REGISTER_DEVICE_004",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "Input parameter 'base_address' is not long-aligned.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_REGISTER_DEVICE_005,
    "DNX_SAND_REGISTER_DEVICE_005",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "Chip type as reported by chip itself does not match registered chip type.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_REGISTER_DEVICE_NULL_BUFFER,
    "DNX_SAND_REGISTER_DEVICE_NULL_BUFFER",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "User supplied buffer is NULL.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_DESC_ARRAY_NOT_INIT,
    "DNX_SAND_DESC_ARRAY_NOT_INIT",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "Descriptors array / mutex are not initialized.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ADD_CHIP_DESCRIPTOR_ERR,
    "DNX_SAND_ADD_CHIP_DESCRIPTOR_ERR",
    "Procedure dnx_sand_register_device() reports:\r\n"
    "Unknown error return value from dnx_sand_add_chip_descriptor().",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SEM_CREATE_FAIL,
    "DNX_SAND_SEM_CREATE_FAIL",
    "Failure trying to create semaphore.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SEM_DELETE_FAIL,
    "DNX_SAND_SEM_DELETE_FAIL",
    "Failure trying to delete semaphore.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SEM_TAKE_FAIL,
    "DNX_SAND_SEM_TAKE_FAIL",
    "Failure trying to take semaphore.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_CHIP_VER_IS_NOT_A,
    "DNX_SAND_CHIP_VER_IS_NOT_A",
    "Failure trying to call RevA only function while running over RevB+ chip.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_CHIP_VER_IS_A,
    "DNX_SAND_CHIP_VER_IS_A",
    "Failure trying to call RevB+ function while running over RevA chip.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TCM_CALLBACK_DELTA_LIST_SEM_TAKE_FAIL,
    "DNX_SAND_TCM_CALLBACK_DELTA_LIST_SEM_TAKE_FAIL",
    "ERROR at dnx_sand_tcm_callback_delta_list_take_mutex:\n\r"
    "current task id cannot take the tcm callback delta\n\r"
    "list semaphore while already having the chip descriptors semaphore.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_CHIP_DESCRIPTORS_SEM_TAKE_ERR_0,
    "DNX_SAND_CHIP_DESCRIPTORS_SEM_TAKE_ERR_0",
    "ERROR at dnx_sand_take_chip_descriptor_mutex:\n\r"
    "current task id cannot take the chip descriptor mutex\n\r"
    "list semaphore while already having the handles delta list semaphore.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SEM_GIVE_FAIL,
    "DNX_SAND_SEM_GIVE_FAIL",
    "Failure trying to give back semaphore.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SEM_CHANGED_ON_THE_FLY,
    "DNX_SAND_SEM_CHANGED_ON_THE_FLY",
    "Semaphore take has succeeded but it is not the one\r\n"
    "which was intended, initially (It was deleted and\r\n"
    "replaced).",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_IND_TRIGGER_TIMEOUT,
    "DNX_SAND_IND_TRIGGER_TIMEOUT",
    "This error relates to indirect access only.\r\n"
    "Timeout waiting for chip to deassert the 'trigger'\r\n"
    "bit.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_UNREGISTER_DEVICE_001,
    "DNX_SAND_UNREGISTER_DEVICE_001",
    "Procedure dnx_sand_user_callback_unregister_device() or\r\n"
    "dnx_sand_clear_all_device_peding_services() reports:\r\n"
    "Failed to delete some of the device's active handles",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_UNREGISTER_DEVICE_002,
    "DNX_SAND_UNREGISTER_DEVICE_002",
    "Procedure dnx_sand_user_callback_unregister_device() reports:\r\n"
    "Failed to clear device descriptor",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_001,
    "DNX_SAND_MEM_READ_001",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "Input parameter 'offset' is not long-aligned.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_002,
    "DNX_SAND_MEM_READ_002",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "Input parameter 'offset' is not long-aligned.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_US_001,
    "DNX_SAND_MEM_READ_US_001",
    "Procedure dnx_sand_mem_read_unsafe() reports:\r\n"
    "Input parameter 'offset' is not long-aligned.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_US_002,
    "DNX_SAND_MEM_READ_US_002",
    "Procedure dnx_sand_mem_read_unsafe() reports:\r\n"
    "Input parameter 'size' is not a multiple of"
    "four.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_US_003,
    "DNX_SAND_MEM_READ_US_003",
    "Procedure dnx_sand_mem_read_unsafe() reports:\r\n"
    "This error relates to direct access only.\r\n"
    "Specified memory range ('offset', 'size') contains\r\n"
    "some addresses which are not readable by this chip\r\n"
    "(probably interrupt sources, which are aoto cleread).",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_003,
    "DNX_SAND_MEM_READ_003",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "This error relates to direct access only.\r\n"
    "Specified memory range ('offset', 'size') contains\r\n"
    "some addresses which are not readable by this chip\r\n"
    "(probably interrupt sources, which are aoto cleread).",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_004,
    "DNX_SAND_MEM_READ_004",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "This error relates to indirect access only.\r\n"
    "Specified module (within 'offset') is illegal.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_005,
    "DNX_SAND_MEM_READ_005",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "This error relates to indirect access only.\r\n"
    "Specified memory range ('offset', 'size')\r\n"
    "contains some addresses which are not readable\r\n"
    "by this chip.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_US_005,
    "DNX_SAND_MEM_READ_US_005",
    "Procedure dnx_sand_mem_read_unsafe() reports:\r\n"
    "This error relates to indirect access only.\r\n"
    "dnx_sand_indirect_read_from_chip failed.\r\n",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_006,
    "DNX_SAND_MEM_READ_006",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "Failed to read from the chip",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_US_006,
    "DNX_SAND_MEM_READ_US_006",
    "Procedure dnx_sand_mem_read_unsafe() reports:\r\n"
    "Failed to read from the chip",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_007,
    "DNX_SAND_MEM_READ_007",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "Failed to write to chip (relevant only to indirect)",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_READ_010,
    "DNX_SAND_MEM_READ_010",
    "Procedure dnx_sand_mem_read() reports:\r\n"
    "Function - dnx_sand_mem_read_unsafe() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_001,
    "DNX_SAND_MEM_WRITE_001",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "Input parameter 'offset' is not long-aligned.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_US_001,
    "DNX_SAND_MEM_WRITE_US_001",
    "Procedure dnx_sand_mem_write_unsafe() reports:\r\n"
    "Input parameter 'offset' is not long-aligned.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_002,
    "DNX_SAND_MEM_WRITE_002",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "Input parameter 'size' is not a multiple of"
    "four.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_003,
    "DNX_SAND_MEM_WRITE_003",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "This error relates to direct access only.\r\n"
    "Specified memory range ('offset', 'size') contains\r\n"
    "some addresses which are not writable by this chip\r\n"
    "(probably interrupt sources or interrupt mask).",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_US_003,
    "DNX_SAND_MEM_WRITE_US_003",
    "Procedure dnx_sand_mem_write_unsafe() reports:\r\n"
    "This error relates to direct access only.\r\n"
    "Specified memory range ('offset', 'size') contains\r\n"
    "some addresses which are not writable by this chip\r\n"
    "(probably interrupt sources or interrupt mask).",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_004,
    "DNX_SAND_MEM_WRITE_004",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "This error relates to indirect access only.\r\n"
    "Specified module (within 'offset') is illegal.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_005,
    "DNX_SAND_MEM_WRITE_005",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "This error relates to indirect access only.\r\n"
    "Specified memory range ('offset', 'size')\r\n"
    "contains some addresses which are not writable\r\n"
    "by this chip.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_US_005,
    "DNX_SAND_MEM_WRITE_US_005",
    "Procedure dnx_sand_mem_write_unsafe() reports:\r\n"
    "This error relates to indirect access only.\r\n"
    "dnx_sand_indirect_write_from_chip failed.\r\n",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_006,
    "DNX_SAND_MEM_WRITE_006",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "Failed to write to the chip",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_US_006,
    "DNX_SAND_MEM_WRITE_US_006",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "Failed to write to the chip",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MEM_WRITE_010,
    "DNX_SAND_MEM_WRITE_010",
    "Procedure dnx_sand_mem_write() reports:\r\n"
    "Function - dnx_sand_mem_write_unsafe() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_FAIL_REQUEST_LEGAL,
    "DNX_SAND_FAIL_REQUEST_LEGAL",
    "Procedure dnx_sand_indirect_read_low() reports:\r\n"
    "Function - dnx_sand_indirect_check_request_legal() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_FAIL_VERIFY_0,
    "DNX_SAND_FAIL_VERIFY_0",
    "Procedure dnx_sand_indirect_read_low() reports:\r\n"
    "Function - dnx_sand_trigger_verify_0_inc_info() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_FAIL_WRITE_ADDRESS,
    "DNX_SAND_FAIL_WRITE_ADDRESS",
    "Procedure dnx_sand_indirect_read_low() reports:\r\n"
    "Function - dnx_sand_indirect_write_address_inc_info() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_FAIL_WRITE_VALUE,
    "DNX_SAND_FAIL_WRITE_VALUE",
    "Procedure dnx_sand_indirect_read_low() reports:\r\n"
    "Function - dnx_sand_indirect_write_value_inc_info() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_FAIL_ASSERT_1,
    "DNX_SAND_FAIL_ASSERT_1",
    "Procedure dnx_sand_indirect_read_low() reports:\r\n"
    "Function - dnx_sand_trigger_assert_1_inc_info() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_FAIL_READ_RESULT,
    "DNX_SAND_FAIL_READ_RESULT",
    "Procedure dnx_sand_indirect_read_low() reports:\r\n"
    "Function - dnx_sand_indirect_read_result_inc_info() failed",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TRG_TIMEOUT,
    "DNX_SAND_TRG_TIMEOUT",
    "Timeout on waiting for 'trigger'to be deasserted.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_CALLBACK_FUNCTION_CANT_BE_INT,
    "DNX_SAND_CALLBACK_FUNCTION_CANT_BE_INT",
    "Called to callback function that do\r\n"
    "not use interrupt with interrupt request.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_NOT_CALLBACK_FUNCTION,
    "DNX_SAND_NOT_CALLBACK_FUNCTION",
    "Asked function can not be registered as callback.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_TCM_MAIN_ERR_01,
    "DNX_SAND_TCM_MAIN_ERR_01",
    "Unrecoverable error encountered in TCM.",
    DNX_SAND_SVR_FTL,
    FALSE
  },
  {
    DNX_SAND_TCM_MAIN_ERR_02,
    "DNX_SAND_TCM_MAIN_ERR_02",
    "Removed a deferred action due to an error.",
    DNX_SAND_SVR_FTL,
    FALSE
  },
  {
    DNX_SAND_TCM_MAIN_ERR_03,
    "DNX_SAND_TCM_MAIN_ERR_03",
    "Removed a deferred action due to user registration request.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR,
    "DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR",
    "Procedure dnx_sand_indirect_check_request_legal() reports:\n\r"
    "Trying to access non-existing indirect module.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_INDIRECT_OFFSET_SIZE_MISMATCH_ERR,
    "DNX_SAND_INDIRECT_OFFSET_SIZE_MISMATCH_ERR",
    "Procedure dnx_sand_indirect_check_request_legal() reports:\n\r"
    "Trying to access non-existing offset/size in the indirect module.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_INDIRECT_SIZE_MISMATCH_ERR,
    "DNX_SAND_INDIRECT_SIZE_MISMATCH_ERR",
    "Procedure dnx_sand_indirect_check_request_legal() reports:\n\r"
    "Example to a possible error:\n\r"
    "Say specific module answer size is 4 32 bit register (128 bit, 16 byte, 4 longs)\n\r"
    "(PEC in DNX_SAND_FAP10M). User gave 5 longs as buffer size.\n\r"
    "((5/4) * 4) ==> 4 longs.\n\r"
    "4!=5 ... the result is different than we need. We find a mismatch is sizes.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_INDIRECT_CANT_GET_INFO_ERR,
    "DNX_SAND_INDIRECT_CANT_GET_INFO_ERR",
    "Internal error!!\n\r"
    "The indirect information missing offset information.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_INDIRECT_TABLES_INFO_ORDER_ERR,
    "DNX_SAND_INDIRECT_TABLES_INFO_ORDER_ERR",
    "If a tables_prefix A is a subset of tables_prefix B,\n\r"
    "The structure that contain tables_prefix A, must\n\r"
    "come before the structure that contains tables_prefix B.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_FREE_FAIL,
    "DNX_SAND_FREE_FAIL",
    "Failure trying to free memory,\n\r",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_CREDIT_SIZE_ENUM_ERR,
    "DNX_SAND_CREDIT_SIZE_ENUM_ERR",
    "User gave not valid value in DNX_SAND_CREDIT_SIZE.\n\r"
    "Please refer to DNX_SAND_CREDIT_SIZE definition.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR,
    "DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR",
    "Functions 'dnx_sand_bitstream_get_field()'/'dnx_sand_bitstream_set_field()' reports:\n\r"
    "'nof_bits' is bigger than 32 (DNX_SAND_BIT_STREAM_FIELD_SET_SIZE).\n\r"
    "Note, these functions get/set at most 32 bit fields.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_DO_NOT_SUPPORTS_INTERRUPTS_ERR,
    "DNX_SAND_DO_NOT_SUPPORTS_INTERRUPTS_ERR",
    "Function 'XXXX_register_device()' reports:\n\r"
    "User try to register device in DNX_SAND. DNX_SAND was configured to\n\r"
    "user real interrupts (via 'dnx_sand_module_open()'). This driver\n\r"
    "do not supports interrupts. Only mock-up interrupts\n\r"
    "('soc_tcmmockup_interrupts' indicator in 'dnx_sand_module_open()).",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_DIV_BY_ZERO_ERR,
    "DNX_SAND_DIV_BY_ZERO_ERR",
    "Division by zero. \n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_OVERFLOW_ERR,
    "DNX_SAND_OVERFLOW_ERR",
    "Calculation overflow. \n\r "
    ".\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERRORS_FOUND_DUPLICATES_ERR,
    "DNX_SAND_ERRORS_FOUND_DUPLICATES_ERR",
    "Different errors use the same error id. \n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PROCS_FOUND_DUPLICATES_ERR,
    "DNX_SAND_PROCS_FOUND_DUPLICATES_ERR",
    "Different procedures use the same proc_id. \n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },  
  {
    DNX_SAND_ERR_8001,
    "DNX_SAND_ERR_8001",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8002,
    "DNX_SAND_ERR_8002",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8003,
    "DNX_SAND_ERR_8003",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8004,
    "DNX_SAND_ERR_8004",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8005,
    "DNX_SAND_ERR_8005",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8006,
    "DNX_SAND_ERR_8006",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8007,
    "DNX_SAND_ERR_8007",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8008,
    "DNX_SAND_ERR_8008",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8009,
    "DNX_SAND_ERR_8009",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8010,
    "DNX_SAND_ERR_8010",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8011,
    "DNX_SAND_ERR_8011",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8012,
    "DNX_SAND_ERR_8012",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8013,
    "DNX_SAND_ERR_8013",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8014,
    "DNX_SAND_ERR_8014",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8015,
    "DNX_SAND_ERR_8015",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ERR_8016,
    "DNX_SAND_ERR_8016",
    "Currently undefined.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_CODE_HAMMING_P_BIT_WIDE_UN_MATCH_ERR,
    "DNX_SAND_CODE_HAMMING_P_BIT_WIDE_UN_MATCH_ERR",
    "Hamming-Code SW module received not correlated data-bit-wide,\n\r"
    "and p-bit-wide. The correlated number can be found from\n\r"
    "'dnx_sand_code_hamming_get_p_bit_wide()' func call.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_CODE_HAMMING_UN_SUPPORTED_DATA_BIT_WIDE_ERR,
    "DNX_SAND_CODE_HAMMING_UN_SUPPORTED_DATA_BIT_WIDE_ERR",
    "Hamming-Code SW module received un-supported data-bit-wide.\n\r"
    "The maximum-p-bit-size we support is 32. Hence,\n\r"
    "this problem is practically will no happen.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_MAGIC_NUM_ERR,
    "DNX_SAND_MAGIC_NUM_ERR",
    "When a device driver supports the magic number capability,\n\r"
    " the user has to call a _clear function, before using\n\r"
    " a structure to set information to the device driver.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_ILLEGAL_IP_FORMAT,
    "DNX_SAND_ILLEGAL_IP_FORMAT",
    " Illegal IP decimal format. Must be 16 bytes long at least,\n\r"
    " 4 decimal numbers separated by dots.",
    DNX_SAND_SVR_ERR,
    FALSE
  },

 /*
  * Hash table
  */
  {
    DNX_SAND_HASH_TABLE_SIZE_OUT_OF_RANGE_ERR,
    "DNX_SAND_HASH_TABLE_SIZE_OUT_OF_RANGE_ERR",
    "the given size of the hash table to allocate is out of range.\n\r"
    "Range is 1-256K, preferred power of 2.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_HASH_TABLE_IS_FULL_ERR,
    "DNX_SAND_HASH_TABLE_IS_FULL_ERR",
    "the hash table is full and trying to insert to the hash table.\n\r"
    "the size of the hash table as allocated in the create..\n\r"
    "and this size may not change.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_HASH_TABLE_KEY_ALREADY_EXIST_ERR,
    "DNX_SAND_HASH_TABLE_KEY_ALREADY_EXIST_ERR",
    "trying to add to the hash table key that already present in\n\r"
    "the hash table. key can be present at most once in the hash table.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_HASH_TABLE_KEY_IS_NOT_EXIST_ERR,
    "DNX_SAND_HASH_TABLE_KEY_IS_NOT_EXIST_ERR",
    "trying to update a key that is not exist in the hash table \n\r"
    "should use Add.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
 /*
  * group member list
  */
  {
    DNX_SAND_GROUP_MEM_LL_NOF_MEMBERS_OUT_OF_RANGE_ERR,
    "DNX_SAND_GROUP_MEM_LL_NOF_MEMBERS_OUT_OF_RANGE_ERR",
    "the number of elements (members) is out of range.\n\r"
    "Range is 1-",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_GROUP_MEM_LL_NOF_GROUPS_OUT_OF_RANGE_ERR,
    "DNX_SAND_GROUP_MEM_LL_NOF_GROUPS_OUT_OF_RANGE_ERR",
    "the number of groups is out of range.\n\r"
    "Range is 1-",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR,
    "DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR",
    "the member (element) id is out of range.\n\r"
    "Range according to nof_elements given in create.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR,
    "DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR",
    "the group id is out of range.\n\r"
    "Range according to nof_groups given in create.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_GROUP_MEM_LL_ILLEGAL_ADD_ERR,
    "DNX_SAND_GROUP_MEM_LL_ILLEGAL_ADD_ERR",
    "try to add element as member to group when it's already.\n\r"
    "a member in a group and auto_remove is FALSE.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_ARR_MEM_ALLOCATOR_NOF_LINES_OUT_OF_RANGE_ERR,
    "DNX_SAND_ARR_MEM_ALLOCATOR_NOF_LINES_OUT_OF_RANGE_ERR",
    "the number of lines (memory) is out of range.\n\r"
    "Range is 1-max_sand_u32",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC_SIZE_OUT_OF_RANGE_ERR,
    "DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC_SIZE_OUT_OF_RANGE_ERR",
    "malloc size is out of range.\n\r"
    "malloc can be in size of 2-size of create memory.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR,
    "DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR",
    "Free pointer is out of range..\n\r"
    "free pointer range is 0-size of create memory.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_PP_IPV4_SUBNET_PREF_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_IPV4_SUBNET_PREF_OUT_OF_RANGE_ERR",
    "the ipv4 prefix len is out of range \n\r"
    "ipv4 prefix length range 0-32.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_PP_IPV6_ADDRESS_IS_NOT_MCAST_ERR,
    "DNX_SAND_PP_IPV6_ADDRESS_IS_NOT_MCAST_ERR",
    "the ipv6 address is not ipv6 multicast address\n\r"
    "prefix is not 0xff.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_PP_MAC_ADDRESS_ILLEGAL_STRING_ERR,
    "DNX_SAND_PP_MAC_ADDRESS_ILLEGAL_STRING_ERR",
    "the mac address string is not valid \n\r"
    "one of the string chars is out of hex values."
    "range of each char is 0-f in hexadecimal.",
    DNX_SAND_SVR_MSG,
    FALSE
  },
    {
    DNX_SAND_PP_IPV4_ADDRESS_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_IPV4_ADDRESS_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_IPV4_ADDRESS' is out of range. \n\r "
    "The range is: 0 - No_max.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_VLAN_ID' is out of range. \n\r "
    "The range is: 0 - 4*1024-1.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_TC_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_TC_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_TC' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_DP_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_DP_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_DP' is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_DEI_CFI_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_DEI_CFI_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_DEI_CFI' is out of range. \n\r "
    "The range is: 0 - 1.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_PCP_UP_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_PCP_UP_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_PCP_UP' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_ETHER_TYPE_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_ETHER_TYPE_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_ETHER_TYPE' is out of range. \n\r "
    "The range is: 0 - 0xffff.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_TPID_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_TPID_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_TPID' is out of range. \n\r "
    "The range is: 0 - 0xffff.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_IP_TTL_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_IP_TTL_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_IP_TTL' is out of range. \n\r "
    "The range is: 0 - 255.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_IPV4_TOS_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_IPV4_TOS_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_IPV4_TOS' is out of range. \n\r "
    "The range is: 0 - 255.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_IPV6_TC_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_IPV6_TC_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_IPV6_TC' is out of range. \n\r "
    "The range is: 0 - 255.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_MPLS_LABEL_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_MPLS_LABEL_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_MPLS_LABEL' is out of range. \n\r "
    "The range is: 0 - (1<<20)-1.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_MPLS_EXP' is out of range. \n\r "
    "The range is: 0 - 7.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_DSCP_EXP_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR",
    "The parameter DSCP_EXP_NDX is out of range. \n\r "
    "The range is: 0 - 255.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_MPLS_DSCP_EXP_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_DSCP_EXP_OUT_OF_RANGE_ERR",
    "The parameter DSCP_EXP_NDX is out of range. \n\r "
    "For packet type MPLS \n\r "
    "The range is: 0 - 7.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_ISID_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_ISID_OUT_OF_RANGE_ERR",
    "The parameter of type 'DNX_SAND_PP_ISID' is out of range. \n\r "
    "The range is: 0 - No_max.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_PP_IN_RANGE_OUT_OF_RANGE_ERR,
    "DNX_SAND_PP_IN_RANGE_OUT_OF_RANGE_ERR",
    "The parameter IN_RANGE is out of range. \n\r "
    "The range is: 0 - 1.\n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SORTED_LIST_KEY_DATA_ALREADY_EXIST_ERR,
    "DNX_SAND_SORTED_LIST_KEY_DATA_ALREADY_EXIST_ERR",
    "adding to the sorted list entry with key \n\r"
    "and data that already exist in the list",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR,
    "DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR",
    "trying to add/remove entry using illegal iterator position",
    DNX_SAND_SVR_MSG,
    FALSE
  },
  {
    DNX_SAND_GET_ERR_TXT_ERR,
    "DNX_SAND_GET_ERR_TXT_ERR",
    "Cannot get error text for error ID. \n\r ",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_VALUE_OUT_OF_RANGE_ERR,
    "DNX_SAND_VALUE_OUT_OF_RANGE_ERR",
    " Value outside the allowed range.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_VALUE_BELOW_MIN_ERR,
    "DNX_SAND_VALUE_BELOW_MIN_ERR",
    " Value below the minimal allowed value.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_VALUE_ABOVE_MAX_ERR,
    "DNX_SAND_VALUE_ABOVE_MAX_ERR",
    " Value above the maximal allowed value.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    DNX_SAND_SOC_ERR,
    "DNX_SAND_SOC_ERR",
    "A soc error occurred.",
    DNX_SAND_SVR_ERR,
    FALSE
  },
  {
    (uint32)DNX_SAND_END_ERR_LIST,
    "",
    "",
    DNX_SAND_SVR_FTL,
    FALSE
  }
} ;

static
  uint8
    Dnx_soc_no_error_printing = FALSE;

/*
 * {
 * Error word construction utilities
 */
void
  dnx_sand_initialize_error_word(
    uint32   proc_id,
    unsigned char   callback,
    uint32   *error_word
    )
{
  dnx_sand_set_proc_id_into_error_word   (proc_id, error_word) ;
  dnx_sand_set_error_code_into_error_word(DNX_SAND_OK,  error_word) ;
  dnx_sand_set_severity_into_error_word  (0, error_word) ;
  dnx_sand_set_callback_into_error_word  (callback, error_word) ;
}
/*
 */
void
  dnx_sand_set_proc_id_into_error_word(
    uint32   proc_id,
    uint32   *error_word
  )
{
  uint32 local_proc_id = proc_id ;
  if(proc_id > DNX_PROC_ID_MAX_VAL)
  {
    goto exit ;
  }
  local_proc_id = DNX_SAND_SET_FLD_IN_PLACE(local_proc_id, DNX_PROC_ID_SHIFT, DNX_PROC_ID_MASK) ;
  *error_word &= ~DNX_PROC_ID_MASK ;
  *error_word |= local_proc_id ;
exit:
  return ;
}
/*
 */
uint32
  dnx_sand_get_proc_id_from_error_word(
    uint32   error_word
  )
{
  uint32 res ;
  res = (uint32)DNX_SAND_GET_FLD_FROM_PLACE(error_word, DNX_PROC_ID_SHIFT, DNX_PROC_ID_MASK) ;
  return res ;
}
/*
 */
void
  dnx_sand_set_error_code_into_error_word(
    unsigned short  error_code,
    uint32   *error_word
  )
{
  uint32
      local_error_code = error_code ;
  /*
   * Make sure error code is within range...
   */
  error_code &= DNX_SAND_ERROR_CODE_MAX_MASK ;


  local_error_code = DNX_SAND_SET_FLD_IN_PLACE(local_error_code, DNX_SAND_ERROR_CODE_SHIFT, DNX_SAND_ERROR_CODE_MASK) ;
  *error_word &= ~DNX_SAND_ERROR_CODE_MASK ;
  *error_word |= local_error_code ;
  goto exit;
exit:
  return ;
}


uint32
  dnx_sand_update_error_code(
    uint32   fun_error_word,
    uint32   *error_word
  )
{
  unsigned short error_code ;
  uint32
      local_error_code;
  /*
   * Make sure error code is within range...
   */
  error_code = (unsigned short)DNX_SAND_GET_FLD_FROM_PLACE(fun_error_word, DNX_SAND_ERROR_CODE_SHIFT, DNX_SAND_ERROR_CODE_MASK) ;
  local_error_code = error_code;

  error_code &= DNX_SAND_ERROR_CODE_MAX_MASK ;
  local_error_code = DNX_SAND_SET_FLD_IN_PLACE(local_error_code, DNX_SAND_ERROR_CODE_SHIFT, DNX_SAND_ERROR_CODE_MASK) ;
  *error_word &= ~DNX_SAND_ERROR_CODE_MASK ;
  *error_word |= local_error_code ;
  goto exit;
exit:
  return *error_word;
}
/*****************************************************
 * See details in dnx_sand_error_code.h
 *****************************************************/
unsigned short
  dnx_sand_get_error_code_from_error_word(
    uint32   error_word
  )
{
  unsigned short res ;
  res = (unsigned short)DNX_SAND_GET_FLD_FROM_PLACE(error_word, DNX_SAND_ERROR_CODE_SHIFT, DNX_SAND_ERROR_CODE_MASK) ;
  return res ;
}
/*
 */
void
  dnx_sand_set_severity_into_error_word(
    unsigned char severity,
    uint32 *error_word
  )
{
  uint32 local_severity = severity ;
  if(severity > DNX_SAND_ERR_SEVERE_MAX_VAL)
  {
    goto exit ;
  }
  local_severity = DNX_SAND_SET_FLD_IN_PLACE(local_severity, DNX_SAND_ERR_SEVERE_SHIFT, DNX_SAND_ERR_SEVERE_MASK) ;
  *error_word &= ~DNX_SAND_ERR_SEVERE_MASK ;
  *error_word |= local_severity ;
exit:
  return ;
}
/*
 */
unsigned char
  dnx_sand_get_severity_from_error_word(
    uint32 error_word
  )
{
  unsigned char res ;
  res = (unsigned char)DNX_SAND_GET_FLD_FROM_PLACE(error_word, DNX_SAND_ERR_SEVERE_SHIFT, DNX_SAND_ERR_SEVERE_MASK) ;
  return res ;
}
/*
 */
void
  dnx_sand_set_callback_into_error_word(
    unsigned char callback,
    uint32 *error_word
  )
{
  uint32 local_callback = callback ;
  if(callback > DNX_SAND_CALLBACK_PROC_MAX_VAL)
  {
    goto exit ;
  }
  local_callback = DNX_SAND_SET_FLD_IN_PLACE(local_callback, DNX_SAND_CALLBACK_PROC_SHIFT, DNX_SAND_CALLBACK_PROC_MASK) ;
  *error_word &= ~DNX_SAND_CALLBACK_PROC_MASK ;
  *error_word |= local_callback ;
exit:
  return ;
}
/*
 */
unsigned char
  dnx_sand_get_callback_from_error_word(
    uint32 error_word
  )
{
  unsigned char res ;
  res = (unsigned char)DNX_SAND_GET_FLD_FROM_PLACE(error_word, DNX_SAND_CALLBACK_PROC_SHIFT, DNX_SAND_CALLBACK_PROC_MASK) ;
  return res ;
}
uint32
  dnx_sand_build_error_code(
    unsigned short error_code,
    uint32  proc_id,
    unsigned char  severity,
    unsigned char  is_call_back
  )
{
  uint32
    ex = DNX_SAND_OK ;
  dnx_sand_set_error_code_into_error_word(error_code,   &ex) ;
  dnx_sand_set_proc_id_into_error_word   (proc_id,      &ex) ;
  dnx_sand_set_severity_into_error_word  (severity,     &ex) ;
  dnx_sand_set_callback_into_error_word  (is_call_back, &ex) ;
  return ex ;
}
/*
 * }
 * Error word construction utilities
 */
/*
 * Handling of all-system errors list plus descriptors.
 * {
 */
/*****************************************************
*NAME
* dnx_sand_get_errors_ptr
*TYPE:
*  PROC
*DATE:
*  17/FEB/2003
*FUNCTION:
*  Get the pointer to the list of errors of the 'dnx_sand'
*  module.
*CALLING SEQUENCE:
*  dnx_sand_get_errors_ptr()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    list of dnx_sand errors: Dnx_soc_sand_error_desc_element.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_ERROR_DESC_ELEMENT * -
*      Pointer to the static list of dnx_sand errors.
*  DNX_SAND_INDIRECT:
*    .
*REMARKS:
*  This utility is mainly for external users (to the 'dnx_sand'
*  module) such as 'fe200 module'.
*SEE ALSO:
*
*****************************************************/
CONST DNX_ERROR_DESC_ELEMENT
  *dnx_sand_get_errors_ptr(
    void
  )
{
  CONST DNX_ERROR_DESC_ELEMENT
    *ret ;
  ret = &Dnx_soc_sand_error_desc_element[0] ;
  return (ret) ;
}
/*****************************************************
*NAME
*  dnx_soc_compare_error_desc_elements
*TYPE: PROC
*DATE: 30/JAN/2003
*FUNCTION:
*  Compare two error descriptors using error id.
*  Return an integer less than, equal to, or greater
*  than zero if id of first argument (error_desc_element_1)
*  is smaller than, equal to, or greater than the id
*  of second argument (error_desc_element_2).
*CALLING SEQUENCE:
*  dnx_soc_compare_error_desc_elements(
*            error_desc_element_1,error_desc_element_2)
*INPUT:
*  DNX_SAND_DIRECT:
*    const void *error_desc_element_1 -
*      Pointer to structure of type DNX_ERROR_DESC_ELEMENT.
*      First subject to compare.
*    const void *error_desc_element_2 -
*      Pointer to structure of type DNX_ERROR_DESC_ELEMENT.
*      Second subject to compare.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int. See decsription.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
int
  dnx_soc_compare_error_desc_elements(
    void *error_desc_element_1,
    void *error_desc_element_2
  )
{
  int
    ret ;
  ret =
      (int)(((const DNX_ERROR_DESC_ELEMENT *)error_desc_element_1)->err_id) -
      (int)(((const DNX_ERROR_DESC_ELEMENT *)error_desc_element_2)->err_id) ;
  return (ret) ;
}

/*
 * Maximal number of elements in one errors pool. For
 * precaution.
 */
#define MAX_POOL_ELEMENT 10000
#define MAX_NOF_POOLS 250

/*
 * Pointer to dynamic memory allocated to hold sorted
 * memory of error descriptors.
 */
static
  DNX_ERROR_DESC_ELEMENT
    *Dnx_soc_sand_p_error_desc_element = (DNX_ERROR_DESC_ELEMENT *) 0 ;
/*
 * Current size of dynamic errors pool (number of bytes).
 */
uint32
  Dnx_soc_sand_error_pool_size = 0 ;
/*
 * Current number of element in dynamic errors pool.
 */
uint32
  Dnx_soc_sand_error_pool_num_elements = 0 ;
/*
 * Semaphore used for protection of dynamic errors pool.
 */
sal_mutex_t
  Dnx_soc_sand_error_pool_mutex = 0 ;

/*
 * List of already-added pools
 */
CONST DNX_ERROR_DESC_ELEMENT
  *Dnx_soc_sand_error_pools[MAX_NOF_POOLS] = {0};

uint32
  Dnx_soc_sand_error_pool_nof_pools = 0;

/*****************************************************
*NAME
* dnx_sand_add_error_pool
*TYPE:
*  PROC
*DATE:
*  30/JAN/2003
*FUNCTION:
*  Add a pool of unsorted error descriptors to all-system,
*  dynamic memory, sorted errors pool.
*CALLING SEQUENCE:
*  dnx_sand_add_error_pool(error_pool)
*INPUT:
*  DNX_SAND_DIRECT:
*   DNX_ERROR_DESC_ELEMENT  *error_pool  -
*     Pointer to unsorted pool of errors.
*  DNX_SAND_INDIRECT:
*   See error_pool
*   Dnx_soc_sand_error_pools,Dnx_soc_sand_p_error_desc_element,Dnx_soc_sand_error_pool_size,
*   Dnx_soc_sand_error_pool_num_elements,Dnx_soc_sand_error_pool_mutex
*OUTPUT:
*  DNX_SAND_DIRECT:
*     Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted error descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_add_error_pools(
    DNX_SAND_IN DNX_ERROR_DESC_ELEMENT  **error_id_pools,
    uint32 nof_pools
  )
{
  int
    ret,
    totol_nof_errs,
    err_i,
    *pool_sizes,
    offset;
  uint32
    pool_i;
  DNX_ERROR_DESC_ELEMENT
    *error_desc_element;
  CONST DNX_ERROR_DESC_ELEMENT
    *error_desc_element_const;

  pool_sizes = dnx_sand_os_malloc_any_size(nof_pools * sizeof(uint32), "pool_sizes");
  if (!pool_sizes)
  {
    ret = 2;
    goto exit;
  }

  totol_nof_errs = 0;
  for (pool_i = 0; pool_i < nof_pools; ++pool_i)
  {
    error_desc_element_const = error_id_pools[pool_i] ;

    for (err_i = 0 ; err_i < MAX_POOL_ELEMENT ; err_i++, error_desc_element_const++)
    {
      if (error_desc_element_const->err_id == DNX_SAND_END_ERR_LIST)
      {
        break ;
      }
    }

    if (err_i ==  MAX_POOL_ELEMENT)
    {
      /*
      * Pool size is too small -
      * we cannot accommodate all the error descriptors,
      * quit with error
      */
      ret = 5 ;
      goto exit_free ;
    }
    else
    {
      pool_sizes[pool_i] = err_i;
      totol_nof_errs += err_i;
    }
  }

  if (totol_nof_errs >  MAX_POOL_ELEMENT)
  {
    /*
    * Pool size is too small -
    * we cannot accommodate all the error descriptors,
    * quit with error
    */
    ret = 8 ;
    goto exit_free ;
  }

  ++totol_nof_errs; /* For DNX_SAND_END_ERR_LIST last element */

  error_desc_element = dnx_sand_os_malloc_any_size(totol_nof_errs * sizeof(DNX_ERROR_DESC_ELEMENT),"error_desc_element");
  if (!error_desc_element)
  {
    ret = 12;
    goto exit_free;
  }

  offset = 0;
  for (pool_i = 0; pool_i < nof_pools; ++pool_i)
  {
    dnx_sand_os_memcpy(
      &error_desc_element[offset],
      error_id_pools[pool_i],
      pool_sizes[pool_i] * sizeof(DNX_ERROR_DESC_ELEMENT)
      );
    offset += pool_sizes[pool_i];
  }

  error_desc_element[offset].err_id = (uint32)DNX_SAND_END_ERR_LIST;

  ret = dnx_sand_add_error_pool(error_desc_element);
  dnx_sand_os_free_any_size(error_desc_element);

exit_free:
  dnx_sand_os_free_any_size(pool_sizes);

exit:
  return ret;
}

int
  dnx_sand_add_error_pool(
    DNX_SAND_IN DNX_ERROR_DESC_ELEMENT  *error_pool
  )
{
  int
    ret ;
  char
    *loc_error_desc_element_ch;
  unsigned
    int
      ui,
      error_pool_size,
      error_pool_num_elements;
  DNX_SAND_RET
    dnx_sand_ret ;
  DNX_ERROR_DESC_ELEMENT
    *error_desc_element ;
  CONST DNX_ERROR_DESC_ELEMENT
    *error_desc_element_const;
  ret = 0 ;
  if (error_pool == (DNX_ERROR_DESC_ELEMENT *)0)
  {
    ret = 6 ;
    goto exit ;
  }

  /*
   * Lock task switching to check whether semaphore needs
   * to be created (first time).
   */
  dnx_sand_ret = dnx_sand_os_task_lock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 1 ;
    goto exit ;
  }
  {
    /*
     * Area protected from task switching.
     */
    if (Dnx_soc_sand_error_pool_mutex == 0)
    {
      /*
       * If this is the first time then allocate semaphore
       */
      Dnx_soc_sand_error_pool_mutex = dnx_sand_os_mutex_create() ;
      if (Dnx_soc_sand_error_pool_mutex == 0)
      {
        dnx_sand_os_task_unlock() ;
        ret = 2 ;
        goto exit ;
      }
    }
  }
  dnx_sand_ret = dnx_sand_os_task_unlock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 3 ;
    goto exit ;
  }
  /*
   * Note that if semaphore is deleted (by 'dnx_sand_close_all_error_pool')
   * while the following 'wait' is active, it is supposed to return
   * with error.
   */
  dnx_sand_ret = dnx_sand_os_mutex_take(Dnx_soc_sand_error_pool_mutex,DNX_SAND_INFINITE_TIMEOUT) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 4 ;
    goto exit ;
  }
  {
    /*
     * Area protected from other users using semaphore.
     */

    for (ui = 0 ; ui < Dnx_soc_sand_error_pool_nof_pools ; ui++)
    {
      if (Dnx_soc_sand_error_pools[ui] == error_pool)
      {
        /* Pool already exist. Nothing to do */
        dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_sand_error_pool_mutex) ;
        if (dnx_sand_ret != DNX_SAND_OK)
        {
          ret = 7 ;
        }
        goto exit;
      }
    }

    /*
     * Find out number of elements in new pool to incorporate.
     */
    error_desc_element_const = error_pool ;
    for (ui = 0 ; ui < MAX_POOL_ELEMENT ; ui++, error_desc_element_const++)
    {
      if (error_desc_element_const->err_id == DNX_SAND_END_ERR_LIST)
      {
        break ;
      }
    }
    if (ui == 0)
    {
      /*
       * If pool is empty then just quit.
       */
      dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_sand_error_pool_mutex) ;
      if (dnx_sand_ret != DNX_SAND_OK)
      {
        ret = 7 ;
      }
      goto exit ;
    }
    error_pool_num_elements = ui ;
    error_pool_size = sizeof(DNX_ERROR_DESC_ELEMENT) * ui ;
    /*
     * At this point, the new pool is not empty and has not yet
     * been incorporated.
     */
    if (Dnx_soc_sand_p_error_desc_element)
    {
      /*
       * Enter if memory has already been allocated for
       * a previous pool. Free it and allocate larger memory.
       */
      DNX_ERROR_DESC_ELEMENT
        *loc_error_desc_element ;
      error_pool_size += Dnx_soc_sand_error_pool_size ;
      error_pool_num_elements += Dnx_soc_sand_error_pool_num_elements ;
      error_desc_element =
          loc_error_desc_element =
              (DNX_ERROR_DESC_ELEMENT *)dnx_sand_os_malloc(error_pool_size, "loc_error_desc_element") ;
      if (error_desc_element == (DNX_ERROR_DESC_ELEMENT *)0)
      {
        /*
         * If there is no dynamic memory then quit with error.
         */
        dnx_sand_os_mutex_give(Dnx_soc_sand_error_pool_mutex) ;
        ret = 8 ;
        goto exit ;
      }
      /*
       * Copy elements already there.
       */
      dnx_sand_os_memcpy(error_desc_element,Dnx_soc_sand_p_error_desc_element,Dnx_soc_sand_error_pool_size) ;
      /*
       * Free the old memory.
       */
      dnx_sand_os_free(Dnx_soc_sand_p_error_desc_element);
      Dnx_soc_sand_p_error_desc_element = NULL;
      /*
       * Copy new elements.
       */
      loc_error_desc_element_ch = ((char *)loc_error_desc_element);
      loc_error_desc_element_ch += Dnx_soc_sand_error_pool_size;
      loc_error_desc_element = ((DNX_ERROR_DESC_ELEMENT*)loc_error_desc_element_ch);
      dnx_sand_os_memcpy(loc_error_desc_element,error_pool,error_pool_size - Dnx_soc_sand_error_pool_size) ;
    }
    else
    {
      /*
       * Enter if memory has not yet been allocated for
       * any previous pool. Allocate for the first time.
       */
      error_desc_element = (DNX_ERROR_DESC_ELEMENT *)dnx_sand_os_malloc(error_pool_size,"error_desc_element") ;
      if (error_desc_element == (DNX_ERROR_DESC_ELEMENT *)0)
      {
        /*
         * If there is no dynamic memory then quit with error.
         */
        dnx_sand_os_mutex_give(Dnx_soc_sand_error_pool_mutex) ;
        ret = 9 ;
        goto exit ;
      }
      dnx_sand_os_memcpy(error_desc_element,error_pool,error_pool_size) ;
    }
    /*
     * At this point, 'error_desc_element' points to dynamic memory
     * with all error descriptors in it, needing sorting.
     */
    /*
     * Sort list by error id.
     */
    dnx_sand_os_qsort(
      &error_desc_element[0],error_pool_num_elements,
      sizeof(DNX_ERROR_DESC_ELEMENT),dnx_soc_compare_error_desc_elements) ;
    /*
     * Now update control variables.
     */
    Dnx_soc_sand_p_error_desc_element = error_desc_element ;
    Dnx_soc_sand_error_pool_size = error_pool_size ;
    Dnx_soc_sand_error_pool_num_elements = error_pool_num_elements ;
    Dnx_soc_sand_error_pools[Dnx_soc_sand_error_pool_nof_pools++] = error_pool;
  }

#if DNX_SAND_ERROR_CODE_FIND_DUPLICATES /* { */
  /* Validate that every error code exists one time, at most */
    {
      uint32
        nof_errors;
      uint8
        *arr,
        end_of_lst_cnt;

      nof_errors = ((1<<DNX_SAND_ERROR_CODE_NUM_BITS)-1);
      arr = dnx_sand_os_malloc_any_size(nof_errors * sizeof(*arr), "arr");

      if (!arr)
      {
        ret = 10;
        goto exit;
      }

      dnx_sand_os_memset(arr, 0, nof_errors * sizeof(*arr));

      end_of_lst_cnt = 0;
      for (ui = 0; ui < error_pool_num_elements; ++ui)
      {
        if (Dnx_soc_sand_p_error_desc_element[ui].err_id == DNX_SAND_END_ERR_LIST)
        {
          if (++end_of_lst_cnt > 1)
          {
#if (DNX_SAND_DEBUG >= DNX_SAND_DBG_LVL2)
            LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                     (BSL_META("DNX_SAND_END_ERR_LIST found twice.\n\r")));
#endif
            ret = DNX_SAND_ERRORS_FOUND_DUPLICATES_ERR;
            break;
          }
        }
        else if (Dnx_soc_sand_p_error_desc_element[ui].err_id >= nof_errors)
        {
#if (DNX_SAND_DEBUG >= DNX_SAND_DBG_LVL2)
          LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                   (BSL_META("Error ID out of range:\n\r"
"Error_name=%s, err_id=%d\n\r"),
                    Dnx_soc_sand_p_error_desc_element[ui].err_name,
                    Dnx_soc_sand_p_error_desc_element[ui].err_id
                    ));
#endif
          ret = DNX_SAND_VALUE_OUT_OF_RANGE_ERR;
          break;
        }
        else if (++arr[Dnx_soc_sand_p_error_desc_element[ui].err_id] > 1)
        {
          uint32 find_dup_i;

#if (DNX_SAND_DEBUG >= DNX_SAND_DBG_LVL2)
          LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                   (BSL_META("Duplicate found:\n\r"
"Error_name=%s, err_id=%d\n\r"),
                    Dnx_soc_sand_p_error_desc_element[ui].err_name,
                    Dnx_soc_sand_p_error_desc_element[ui].err_id
                    ));

          for (find_dup_i = 0; find_dup_i < ui; ++find_dup_i)
          {
            if (Dnx_soc_sand_p_error_desc_element[find_dup_i].err_id == Dnx_soc_sand_p_error_desc_element[ui].err_id)
            {
              LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                       (BSL_META("#2: error_name=%s, proc_id=%d\n\r"),
                        Dnx_soc_sand_p_error_desc_element[find_dup_i].err_name,
                        Dnx_soc_sand_p_error_desc_element[find_dup_i].err_id
                        ));
              /* break; */
            }
          }
#endif
          ret = 13;
          /* break; */
        }
      }

      dnx_sand_os_free_any_size(arr);
    }
#endif /* } DNX_SAND_ERROR_CODE_FIND_DUPLICATES */

  /*
   * Now free mutex.
   */
  dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_sand_error_pool_mutex) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 10 ;
  }
exit:
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_close_all_error_pool
*TYPE:
*  PROC
*DATE:
*  30/JAN/2003
*FUNCTION:
*  Return all resources related to system-wide errors
*  pool.
*CALLING SEQUENCE:
*  dnx_sand_close_all_error_pool()
*INPUT:
*  DNX_SAND_DIRECT:
*   None.
*  DNX_SAND_INDIRECT:
*   Dnx_soc_sand_error_pools,Dnx_soc_sand_p_error_desc_element,Dnx_soc_sand_error_pool_size,
*   Dnx_soc_sand_error_pool_num_elements,Dnx_soc_sand_error_pool_mutex
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool variables.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_close_all_error_pool(
    void
  )
{
  int
    ret ;
  DNX_SAND_RET
    dnx_sand_ret ;
  ret = 0;
 /*
   * Lock task switching to check whether semaphore exists
   * at all. If it does not, just set all variables to defaults
   * and quit.
   */
  dnx_sand_ret = dnx_sand_os_task_lock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 1 ;
    goto exit ;
  }
  {
    /*
     * Area protected from task switching.
     */
    if (Dnx_soc_sand_error_pool_mutex == 0)
    {
      /*
       * If pool has not been activated then just
       * set defaults.
       */
      Dnx_soc_sand_p_error_desc_element = (DNX_ERROR_DESC_ELEMENT *)0 ;
      Dnx_soc_sand_error_pool_nof_pools = 0;
      Dnx_soc_sand_error_pool_size =
        Dnx_soc_sand_error_pool_num_elements = 0 ;
      dnx_sand_ret = dnx_sand_os_task_unlock() ;
      if (dnx_sand_ret != DNX_SAND_OK)
      {
        ret = 2 ;
      }
      goto exit ;
    }
  }
  dnx_sand_ret = dnx_sand_os_task_unlock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 3 ;
    goto exit ;
  }
  /*
   * Note that if semaphore is deleted (by another
   *'dnx_sand_close_all_error_pool' invocation)
   * while the following 'wait' is active, it is supposed to return
   * with error.
   */
  dnx_sand_ret = dnx_sand_os_mutex_take(Dnx_soc_sand_error_pool_mutex,DNX_SAND_INFINITE_TIMEOUT) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 4 ;
    goto exit ;
  }
  {
    /*
     * Area protected from other users using semaphore.
     */
    if (Dnx_soc_sand_p_error_desc_element)
    {
      dnx_sand_os_free(Dnx_soc_sand_p_error_desc_element) ;
    }
    /*
     * Precaution: Set defaults.
     */
    Dnx_soc_sand_p_error_desc_element = (DNX_ERROR_DESC_ELEMENT *)0 ;
    Dnx_soc_sand_error_pool_nof_pools = 0;
    Dnx_soc_sand_error_pool_size =
      Dnx_soc_sand_error_pool_num_elements = 0 ;
    dnx_sand_ret = dnx_sand_os_task_lock() ;
    if (dnx_sand_ret != DNX_SAND_OK)
    {
      ret = 5 ;
    }
    dnx_sand_os_mutex_delete(Dnx_soc_sand_error_pool_mutex) ;
    Dnx_soc_sand_error_pool_mutex = 0 ;
    dnx_sand_ret = dnx_sand_os_task_unlock() ;
    if (dnx_sand_ret != DNX_SAND_OK)
    {
      ret = 6 ;
    }
  }
exit:
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_add_sand_errors
*TYPE:
*  PROC
*DATE:
*  30/JAN/2003
*FUNCTION:
*  Add the pool of DNX_SAND errors to the all-system
*  sorted pool.
*CALLING SEQUENCE:
*  dnx_sand_add_sand_errors()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted error descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_add_sand_errors(
    void
  )
{
  int
    ret ;
  ret = 0 ;
  ret = dnx_sand_add_error_pool(Dnx_soc_sand_error_desc_element) ;
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_linear_find_error
*TYPE:
*  PROC
*DATE:
*  17/FEB/2003
*FUNCTION:
*  Find indicated error code in the specified
*  non-sorted pool and point to it.
*CALLING SEQUENCE:
*  dnx_sand_linear_find_error(
*    error_code,
*    in_error_desc_element,
*    error_desc_element_ptr)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32      error_code -
*      Error code to locate.
*    DNX_ERROR_DESC_ELEMENT *in_error_desc_element -
*      Pointer to error descriptor list to search
*      (linearly).
*    DNX_ERROR_DESC_ELEMENT **error_desc_element_ptr -
*      This procedure loads pointed memory with pointer
*      to found item. If no item is found, a value
*      of '0' (null pointer) is loaded into
*      pointed memory.
*  DNX_SAND_INDIRECT:
*    in_error_desc_element
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    See error_desc_element_ptr.
*REMARKS:
*  This procedure is only to be used in case binary
*  search has failed (e.g. when dnx_sand module or some
*  other module has not been started yet).
*
*SEE ALSO:
*****************************************************/
int
  dnx_sand_linear_find_error(
    uint32      error_code,
    DNX_SAND_IN DNX_ERROR_DESC_ELEMENT *in_error_desc_element,
    CONST DNX_ERROR_DESC_ELEMENT **error_desc_element_ptr
  )
{
  int
    ret ;
  CONST DNX_ERROR_DESC_ELEMENT
    *error_desc_element ;
  unsigned
    int
      ui ;
  ret = 0 ;
  *error_desc_element_ptr = (DNX_ERROR_DESC_ELEMENT *)0 ;
  if (in_error_desc_element == (DNX_ERROR_DESC_ELEMENT *)0)
  {
    /*
     * Enter if input array is null. Just quit
     */
    ret = 1 ;
    goto exit ;
  }
  error_desc_element = in_error_desc_element ;
  /*
   * Search all elements in input unsorted pool.
   */
  for (ui = 0 ; ui < MAX_POOL_ELEMENT ; ui++, error_desc_element++)
  {
    if (error_desc_element->err_id == DNX_SAND_END_ERR_LIST)
    {
      break ;
    }
    if (error_desc_element->err_id == error_code)
    {
      *error_desc_element_ptr = error_desc_element ;
      break ;
    }
  }
exit:
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_find_error
*TYPE:
*  PROC
*DATE:
*  03/FEB/2003
*FUNCTION:
*  Find indicated error code in the all-system
*  sorted pool and point to it.
*CALLING SEQUENCE:
*  dnx_sand_find_error(error_code,error_desc_element_ptr)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32      error_code -
*      Error code to locate.
*    DNX_ERROR_DESC_ELEMENT **error_desc_element_ptr -
*      This procedure loads pointed memory with pointer
*      to found item. If no item is found, a value
*      of '0' (null pointer) is loaded into
*      pointed memory.
*  DNX_SAND_INDIRECT:
*    All-system sorted errors pool (Dnx_soc_sand_p_error_desc_element,
*    etc...).
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    See error_desc_element_ptr.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_find_error(
    uint32      error_code,
    DNX_ERROR_DESC_ELEMENT **error_desc_element_ptr
  )
{
  int
    ret ;
  void
    *void_ptr ;
  uint32
      *error_code_ptr ;
  DNX_SAND_RET
    dnx_sand_ret ;
  ret = 0 ;
  *error_desc_element_ptr = (DNX_ERROR_DESC_ELEMENT *)0 ;
  /*
   * Note that serach is protected by semaphore to make sure
   * sorted errors list does not change on-the-fly.
   */
  dnx_sand_ret =
    dnx_sand_os_mutex_take(Dnx_soc_sand_error_pool_mutex,DNX_SAND_INFINITE_TIMEOUT) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 1 ;
    goto exit ;
  }
  {
    /*
     * Area protected from other users using semaphore.
     */
    if (Dnx_soc_sand_p_error_desc_element == (DNX_ERROR_DESC_ELEMENT *)0)
    {
      ret = 2 ;
      goto exit ;
    }
    if ((Dnx_soc_sand_error_pool_size == 0) ||
          (Dnx_soc_sand_error_pool_num_elements == 0))
    {
      ret = 3 ;
      goto exit ;
    }
    error_code_ptr = &error_code ;
    void_ptr =
      dnx_sand_os_bsearch(
        (void *)error_code_ptr,
        (void *)Dnx_soc_sand_p_error_desc_element,
        (uint32)Dnx_soc_sand_error_pool_num_elements,
        sizeof(*Dnx_soc_sand_p_error_desc_element),
        dnx_soc_compare_error_desc_elements) ;
    *error_desc_element_ptr = (DNX_ERROR_DESC_ELEMENT *)void_ptr ;
  }
  /*
   * Now free mutex.
   */
  dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_sand_error_pool_mutex) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 4 ;
  }
exit:
  return (ret) ;
}
/*
 * }
 */
/*
 * Handling of all-system procedures id list plus descriptors.
 * {
 */
/*****************************************************
*NAME
* dnx_sand_get_procedures_ptr
*TYPE:
*  PROC
*DATE:
*  17/FEB/2003
*FUNCTION:
*  Get the pointer to the list of procedures of the 'dnx_sand'
*  module.
*CALLING SEQUENCE:
*  dnx_sand_get_procedures_ptr()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    list of dnx_sand errors: Dnx_soc_sand_procedure_desc_element.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    DNX_PROCEDURE_DESC_ELEMENT * -
*      Pointer to the static list of dnx_sand procedures.
*  DNX_SAND_INDIRECT:
*    .
*REMARKS:
*  This utility is mainly for external users (to the 'dnx_sand'
*  module) such as 'fe200 module'.
*SEE ALSO:
*
*****************************************************/
CONST DNX_PROCEDURE_DESC_ELEMENT
  *dnx_sand_get_procedures_ptr(
    void
  )
{
  CONST DNX_PROCEDURE_DESC_ELEMENT
    *ret ;
  ret = &Dnx_soc_sand_procedure_desc_element[0] ;
  return (ret) ;
}
/*****************************************************
*NAME
*  dnx_soc_compare_proc_desc_elements
*TYPE: PROC
*DATE: 30/JAN/2003
*FUNCTION:
*  Compare two procedure descriptors using proc_id.
*  Return an integer less than, equal to, or greater
*  than zero if id of first argument (procedure_desc_element_1)
*  is smaller than, equal to, or greater than the id
*  of second argument (procedure_desc_element_2).
*CALLING SEQUENCE:
*  dnx_soc_compare_proc_desc_elements(
*            procedure_desc_element_1,
*            procedure_desc_element_2)
*INPUT:
*  DNX_SAND_DIRECT:
*    const void *procedure_desc_element_1-
*      Pointer to structure of type DNX_PROCEDURE_DESC_ELEMENT.
*      First subject to compare.
*    const void *procedure_desc_element_2 -
*      Pointer to structure of type DNX_PROCEDURE_DESC_ELEMENT.
*      Second subject to compare.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Int. See decsription.
*  DNX_SAND_INDIRECT:
*    None
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
int
  dnx_soc_compare_proc_desc_elements(
    void *procedure_desc_element_1,
    void *procedure_desc_element_2
  )
{
  int
    ret ;
  ret =
      (int)(((const DNX_PROCEDURE_DESC_ELEMENT *)procedure_desc_element_1)->proc_id) -
      (int)(((const DNX_PROCEDURE_DESC_ELEMENT *)procedure_desc_element_2)->proc_id) ;
  return (ret) ;
}
/*
 * Maximal number of elements in one proc_id pool. For
 * precaution.
 */
#define MAX_PROC_ID_POOL_ELEMENT 5000
#define MAX_NOF_PROC_ID_POOLS 250

/*
 * Pointer to dynamic memory allocated to hold sorted
 * memory of procedure descriptors.
 */
static
  DNX_PROCEDURE_DESC_ELEMENT
    *Dnx_soc_procedure_desc_element = (DNX_PROCEDURE_DESC_ELEMENT *) 0 ;
/*
 * Current size of dynamic procedure id pool (number of bytes).
 */
uint32
  Dnx_soc_proc_id_pool_size = 0 ;
/*
 * Current number of element in dynamic errors pool.
 */
uint32
  Dnx_soc_proc_id_pool_num_elements = 0 ;
/*
 * Semaphore used for protection of dynamic errors pool.
 */
sal_mutex_t
  Dnx_soc_proc_id_pool_mutex = 0 ;

/*
 * List of already-added pools
 */
CONST DNX_PROCEDURE_DESC_ELEMENT
  *Dnx_soc_sand_proc_id_pools[MAX_NOF_PROC_ID_POOLS] = {0};

uint32
  Dnx_soc_sand_proc_id_pool_nof_pools = 0;
/*****************************************************
*NAME
* dnx_sand_add_proc_id_pool
*TYPE:
*  PROC
*DATE:
*  30/JAN/2003
*FUNCTION:
*  Add a pool of unsorted procedure descriptors to
*  all-system, dynamic memory, sorted proc_id pool.
*CALLING SEQUENCE:
*  dnx_sand_add_proc_id_pool(proc_id_pool)
*INPUT:
*  DNX_SAND_DIRECT:
*   DNX_PROCEDURE_DESC_ELEMENT  *proc_id_pool  -
*     Pointer to unsorted pool of procedure descriptors.
*  DNX_SAND_INDIRECT:
*   See proc_id_pool
*   Proc_id_pools,Dnx_soc_procedure_desc_element,Dnx_soc_proc_id_pool_size,
*   Dnx_soc_proc_id_pool_num_elements,Dnx_soc_proc_id_pool_mutex
*OUTPUT:
*  DNX_SAND_DIRECT:
*     Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted procedure
*    descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_add_proc_id_pools(
    DNX_SAND_IN DNX_PROCEDURE_DESC_ELEMENT  **proc_id_pools,
    uint32 nof_pools
  )
{
  int
    ret,
    totol_nof_procs,
    proc_i,
    *pool_sizes,
    offset;
  uint32
    pool_i;
  DNX_PROCEDURE_DESC_ELEMENT
    *procedure_desc_element;
  CONST DNX_PROCEDURE_DESC_ELEMENT
    *procedure_desc_element_const;

  pool_sizes = dnx_sand_os_malloc_any_size(nof_pools * sizeof(int), "pool_sizes add_proc_id_pools");
  if (!pool_sizes)
  {
    ret = 2;
    goto exit;
  }

  totol_nof_procs = 0;
  for (pool_i = 0; pool_i < nof_pools; ++pool_i)
  {
    procedure_desc_element_const = proc_id_pools[pool_i] ;

    for (proc_i = 0 ; proc_i < MAX_PROC_ID_POOL_ELEMENT ; proc_i++, procedure_desc_element_const++)
    {
      if (procedure_desc_element_const->proc_id == DNX_SAND_END_PROC_LIST)
      {
        break ;
      }
    }

    if (proc_i ==  MAX_PROC_ID_POOL_ELEMENT)
    {
      /*
      * Pool size is too small -
      * we cannot accommodate all the error descriptors,
      * quit with error
      */
      ret = 5 ;
      goto exit_free ;
    }
    else
    {
      pool_sizes[pool_i] = proc_i;
      totol_nof_procs += proc_i;
    }
  }

  if (totol_nof_procs >  MAX_PROC_ID_POOL_ELEMENT)
  {
    /*
    * Pool size is too small -
    * we cannot accommodate all the error descriptors,
    * quit with error
    */
    ret = 8 ;
    goto exit_free ;
  }

  ++totol_nof_procs; /* Add DNX_SAND_END_PROC_LIST at the end */

  procedure_desc_element = dnx_sand_os_malloc_any_size(totol_nof_procs * sizeof(DNX_PROCEDURE_DESC_ELEMENT),"procedure_desc_element");
  if (!procedure_desc_element)
  {
    ret = 12;
    goto exit_free;
  }

  offset = 0;
  for (pool_i = 0; pool_i < nof_pools; ++pool_i)
  {
    dnx_sand_os_memcpy(
      &procedure_desc_element[offset],
      proc_id_pools[pool_i],
      pool_sizes[pool_i] * sizeof(DNX_PROCEDURE_DESC_ELEMENT)
      );

    offset += pool_sizes[pool_i];
  }

  procedure_desc_element[offset].proc_id = (uint32)DNX_SAND_END_PROC_LIST;

  ret = dnx_sand_add_proc_id_pool(procedure_desc_element);
  dnx_sand_os_free_any_size(procedure_desc_element);

exit_free:
  dnx_sand_os_free_any_size(pool_sizes);

exit:
  return ret;
}

int
  dnx_sand_add_proc_id_pool(
    DNX_SAND_IN DNX_PROCEDURE_DESC_ELEMENT  *proc_id_pool
  )
{
  int
    ret ;
  unsigned
    int
      ui,
      proc_id_pool_size,
      proc_id_pool_num_elements;
  char
    *loc_procedure_desc_element_ch;
  DNX_SAND_RET
    dnx_sand_ret ;
  DNX_PROCEDURE_DESC_ELEMENT
    *procedure_desc_element ;
  CONST DNX_PROCEDURE_DESC_ELEMENT
    *procedure_desc_element_const;
  ret = 0 ;
  if (proc_id_pool == (DNX_PROCEDURE_DESC_ELEMENT *)0)
  {
    ret = 6 ;
    goto exit ;
  }
  /*
   * Lock task switching to check whether semaphore needs
   * to be created (first time).
   */
  dnx_sand_ret = dnx_sand_os_task_lock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 1 ;
    goto exit ;
  }
  {
    /*
     * Area protected from task switching.
     */
    if (Dnx_soc_proc_id_pool_mutex == 0)
    {
      /*
       * If this is the first time then allocate semaphore
       */
      Dnx_soc_proc_id_pool_mutex = dnx_sand_os_mutex_create() ;
      if (Dnx_soc_proc_id_pool_mutex == 0)
      {
        dnx_sand_os_task_unlock() ;
        ret = 2 ;
        goto exit ;
      }
    }
  }
  dnx_sand_ret = dnx_sand_os_task_unlock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 3 ;
    goto exit ;
  }
  /*
   * Note that if semaphore is deleted (by 'dnx_sand_close_all_proc_id_pool')
   * while the following 'wait' is active, it is supposed to return
   * with error.
   */
  dnx_sand_ret = dnx_sand_os_mutex_take(Dnx_soc_proc_id_pool_mutex,DNX_SAND_INFINITE_TIMEOUT) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 4 ;
    goto exit ;
  }
  {
    /*
     * Area protected from other users using semaphore.
     */

    for (ui = 0 ; ui < Dnx_soc_sand_proc_id_pool_nof_pools ; ui++)
    {
      if (Dnx_soc_sand_proc_id_pools[ui] == proc_id_pool)
      {
        /* Pool already exist. Nothing to do */
        dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
        if (dnx_sand_ret != DNX_SAND_OK)
        {
          ret = 7 ;
        }
        goto exit;
      }
    }

    /*
     * Find out number of elements in new pool to incorporate.
     */
    procedure_desc_element_const = proc_id_pool ;
    for (ui = 0 ; ui < MAX_PROC_ID_POOL_ELEMENT ; ui++, procedure_desc_element_const++)
    {
      if (procedure_desc_element_const->proc_id == DNX_SAND_END_PROC_LIST)
      {
        break ;
      }
    }
    if (ui == 0)
    {
      /*
       * If pool is empty then just quit.
       */
      dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
      if (dnx_sand_ret != DNX_SAND_OK)
      {
        ret = 7 ;
      }
      goto exit ;
    }
    else if (ui ==  MAX_PROC_ID_POOL_ELEMENT)
    {
      /*
       * Pool size is too small -
       * we cannot accommodate all the error descriptors,
       * quit with error
       */
      dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
      ret = 20 ;
      goto exit ;
    }
    proc_id_pool_num_elements = ui ;
    proc_id_pool_size = sizeof(DNX_PROCEDURE_DESC_ELEMENT) * ui ;
    /*
     * At this point, the new pool is not empty and has not yet
     * been incorporated.
     */
    if (Dnx_soc_procedure_desc_element)
    {
      /*
       * Enter if memory has already been allocated for
       * a previous pool. Free it and allocate larger memory.
       */
      DNX_PROCEDURE_DESC_ELEMENT
        *loc_procedure_desc_element ;
      proc_id_pool_size += Dnx_soc_proc_id_pool_size ;
      proc_id_pool_num_elements += Dnx_soc_proc_id_pool_num_elements ;
      procedure_desc_element =
          loc_procedure_desc_element =
              (DNX_PROCEDURE_DESC_ELEMENT *)dnx_sand_os_malloc(proc_id_pool_size, "loc_procedure_desc_element") ;
      if (procedure_desc_element == (DNX_PROCEDURE_DESC_ELEMENT *)0)
      {
        /*
         * If there is no dynamic memory then quit with error.
         */
        dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
        ret = 8 ;
        goto exit ;
      }
      /*
       * Copy elements already there.
       */
      dnx_sand_os_memcpy(procedure_desc_element,
                        Dnx_soc_procedure_desc_element,Dnx_soc_proc_id_pool_size) ;
      /*
       * Free the old memory.
       */
      dnx_sand_os_free(Dnx_soc_procedure_desc_element);
      Dnx_soc_procedure_desc_element = NULL;
      /*
       * Copy new elements.
       */
      loc_procedure_desc_element_ch = ((char *)loc_procedure_desc_element);
      loc_procedure_desc_element_ch += Dnx_soc_proc_id_pool_size ;
      loc_procedure_desc_element = ((DNX_PROCEDURE_DESC_ELEMENT*)loc_procedure_desc_element_ch);

      dnx_sand_os_memcpy(loc_procedure_desc_element,proc_id_pool,
                        proc_id_pool_size - Dnx_soc_proc_id_pool_size) ;
    }
    else
    {
      /*
       * Enter if memory has not yet been allocated for
       * any previous pool. Allocate for the first time.
       */
      procedure_desc_element = (DNX_PROCEDURE_DESC_ELEMENT *)dnx_sand_os_malloc(proc_id_pool_size, "procedure_desc_element") ;
      if (procedure_desc_element == (DNX_PROCEDURE_DESC_ELEMENT *)0)
      {
        /*
         * If there is no dynamic memory then quit with error.
         */
        dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
        ret = 9 ;
        goto exit ;
      }
      dnx_sand_os_memcpy(procedure_desc_element,proc_id_pool,proc_id_pool_size) ;
    }
    /*
     * At this point, 'procedure_desc_element' points to dynamic memory
     * with all procedure descriptors in it, needing sorting.
     */
    /*
     * Sort list by procedure id.
     */
    dnx_sand_os_qsort(
      &procedure_desc_element[0],proc_id_pool_num_elements,
      sizeof(DNX_PROCEDURE_DESC_ELEMENT),dnx_soc_compare_proc_desc_elements) ;
    /*
     * Now update control variables.
     */
    Dnx_soc_procedure_desc_element = procedure_desc_element ;
    Dnx_soc_proc_id_pool_size = proc_id_pool_size ;
    Dnx_soc_proc_id_pool_num_elements = proc_id_pool_num_elements ;
    Dnx_soc_sand_proc_id_pools[Dnx_soc_sand_proc_id_pool_nof_pools++] = proc_id_pool;

#if DNX_SAND_ERROR_CODE_FIND_DUPLICATES /* { */
    /* Validate that every error code exists one time, at most */
    {
      uint32
        nof_errors;
      uint8
        *arr,
        end_of_lst_cnt;

      nof_errors = ((1<<DNX_PROC_ID_NUM_BITS)-1);
      arr = dnx_sand_os_malloc_any_size(nof_errors * sizeof(*arr), "arr dnx_sand_add_proc_id_pool");

      if (!arr)
      {
        ret = 10;
        goto exit;
      }

      dnx_sand_os_memset(arr, 0, nof_errors * sizeof(*arr));

      end_of_lst_cnt = 0;
      for (ui = 0; ui < Dnx_soc_proc_id_pool_num_elements; ++ui)
      {
        if (Dnx_soc_procedure_desc_element[ui].proc_id == DNX_SAND_END_PROC_LIST)
        {
          if (++end_of_lst_cnt > 1)
          {
#if (DNX_SAND_DEBUG >= DNX_SAND_DBG_LVL2)
            LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                     (BSL_META("DNX_SAND_END_PROC_LIST found twice.\n\r")));
#endif
            ret = DNX_SAND_PROCS_FOUND_DUPLICATES_ERR;
            break;
          }
        }
        else if (Dnx_soc_procedure_desc_element[ui].proc_id >= nof_errors)
        {
          ret = 12;
          break;
        }
        else if (++arr[Dnx_soc_procedure_desc_element[ui].proc_id] > 1)
        {
#if (DNX_SAND_DEBUG >= DNX_SAND_DBG_LVL2)
          uint32 find_dup_i;

          LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                   (BSL_META("Duplicate found:\n\r")));
          LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                   (BSL_META("#1: proc_name=%s, proc_id=%d\n\r"),
                    Dnx_soc_procedure_desc_element[ui].proc_name,
                    Dnx_soc_procedure_desc_element[ui].proc_id
                    ));

          for (find_dup_i = 0; find_dup_i < ui; ++find_dup_i)
          {
            if (Dnx_soc_procedure_desc_element[find_dup_i].proc_id == Dnx_soc_procedure_desc_element[ui].proc_id)
            {
              LOG_INFO(BSL_LS_SOC_MANAGEMENT,
                       (BSL_META("#2: proc_name=%s, proc_id=%d\n\r"),
                        Dnx_soc_procedure_desc_element[find_dup_i].proc_name,
                        Dnx_soc_procedure_desc_element[find_dup_i].proc_id
                        ));
              break;
            }
          }
#endif          
          ret = 13;
        }
      }

      dnx_sand_os_free_any_size(arr);
    }
#endif /* } DNX_SAND_ERROR_CODE_FIND_DUPLICATES */

  }
  /*
   * Now free mutex.
   */
  dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 13 ;
  }
exit:
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_close_all_proc_id_pool
*TYPE:
*  PROC
*DATE:
*  30/JAN/2003
*FUNCTION:
*  Return all resources related to system-wide procedure
*  descriptors pool.
*CALLING SEQUENCE:
*  dnx_sand_close_all_proc_id_pool()
*INPUT:
*  DNX_SAND_DIRECT:
*   None.
*  DNX_SAND_INDIRECT:
*   Proc_id_pools,Dnx_soc_procedure_desc_element,Dnx_soc_proc_id_pool_size,
*   Dnx_soc_proc_id_pool_num_elements,Dnx_soc_proc_id_pool_mutex
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool variables.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_close_all_proc_id_pool(
    void
  )
{
  int
    ret ;
  DNX_SAND_RET
    dnx_sand_ret ;
  /*
   */
  ret = 0;
  /*
   * Lock task switching to check whether semaphore exists
   * at all. If it does not, just set all variables to defaults
   * and quit.
   */
  dnx_sand_ret = dnx_sand_os_task_lock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 1 ;
    goto exit ;
  }
  {
    /*
     * Area protected from task switching.
     */
    if (Dnx_soc_proc_id_pool_mutex == 0)
    {
      /*
       * If pool has not been activated then just
       * set defaults.
       */
      Dnx_soc_procedure_desc_element = (DNX_PROCEDURE_DESC_ELEMENT *)0 ;
      Dnx_soc_sand_proc_id_pool_nof_pools = 0;
      Dnx_soc_proc_id_pool_size =
        Dnx_soc_proc_id_pool_num_elements = 0 ;
      dnx_sand_ret = dnx_sand_os_task_unlock() ;
      if (dnx_sand_ret != DNX_SAND_OK)
      {
        ret = 2 ;
      }
      goto exit ;
    }
  }
  dnx_sand_ret = dnx_sand_os_task_unlock() ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 3 ;
    goto exit ;
  }
  /*
   * Note that if semaphore is deleted (by another
   *'dnx_sand_close_all_proc_id_pool' invocation)
   * while the following 'wait' is active, it is supposed to return
   * with error.
   */
  dnx_sand_ret = dnx_sand_os_mutex_take(Dnx_soc_proc_id_pool_mutex,DNX_SAND_INFINITE_TIMEOUT) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 4 ;
    goto exit ;
  }
  {
    /*
     * Area protected from other users using semaphore.
     */
    if (Dnx_soc_procedure_desc_element)
    {
      dnx_sand_os_free(Dnx_soc_procedure_desc_element) ;
    }
    /*
     * Precaution: Set defaults.
     */
    Dnx_soc_procedure_desc_element = (DNX_PROCEDURE_DESC_ELEMENT *)0 ;
    Dnx_soc_sand_proc_id_pool_nof_pools = 0;
    Dnx_soc_proc_id_pool_size =
      Dnx_soc_proc_id_pool_num_elements = 0 ;
    dnx_sand_ret = dnx_sand_os_task_lock() ;
    if (dnx_sand_ret != DNX_SAND_OK)
    {
      ret = 5 ;
    }
    dnx_sand_os_mutex_delete(Dnx_soc_proc_id_pool_mutex) ;
    Dnx_soc_proc_id_pool_mutex = 0 ;
    dnx_sand_ret = dnx_sand_os_task_unlock() ;
    if (dnx_sand_ret != DNX_SAND_OK)
    {
      ret = 6 ;
    }
  }
exit:
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_add_sand_procedures
*TYPE:
*  PROC
*DATE:
*  30/JAN/2003
*FUNCTION:
*  Add the pool of DNX_SAND procedures to the all-system
*  sorted pool.
*CALLING SEQUENCE:
*  dnx_sand_add_sand_procedures()
*INPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    Updated all-system pool of sorted procedure
*    descriptors.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_add_sand_procedures(
    void
  )
{
  int
    ret ;
  ret = 0 ;
  ret = dnx_sand_add_proc_id_pool(Dnx_soc_sand_procedure_desc_element) ;
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_find_procedure
*TYPE:
*  PROC
*DATE:
*  03/FEB/2003
*FUNCTION:
*  Find indicated procedure id in the all-system
*  sorted pool and point to it.
*CALLING SEQUENCE:
*  dnx_sand_find_procedure(
*        proc_id,procedure_desc_element_ptr)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32      proc_id -
*      Procedure id to locate.
*    DNX_ERROR_DESC_ELEMENT **procedure_desc_element_ptr -
*      This procedure loads pointed memory with pointer
*      to found item. If no item is found, a value
*      of '0' (null pointer) is loaded into
*      pointed memory.
*  DNX_SAND_INDIRECT:
*    All-system sorted procedure descriptors pool
*   (Dnx_soc_procedure_desc_element, etc...).
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    See procedure_desc_element_ptr.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_find_procedure(
    uint32          proc_id,
    DNX_PROCEDURE_DESC_ELEMENT **procedure_desc_element_ptr
  )
{
  int
    ret ;
  void
    *void_ptr ;
  uint32
      *proc_id_ptr ;
  DNX_SAND_RET
    dnx_sand_ret ;
  ret = 0 ;
  *procedure_desc_element_ptr = (DNX_PROCEDURE_DESC_ELEMENT *)0 ;
  /*
   * Note that serach is protected by semaphore to make sure
   * sorted procedure descriptors list does not change on-the-fly.
   */
  dnx_sand_ret =
    dnx_sand_os_mutex_take(Dnx_soc_proc_id_pool_mutex,DNX_SAND_INFINITE_TIMEOUT) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 1 ;
    goto exit ;
  }
  {
    /*
     * Area protected from other users using semaphore.
     */
    if (Dnx_soc_procedure_desc_element == (DNX_PROCEDURE_DESC_ELEMENT *)0)
    {
      ret = 2 ;
      goto exit ;
    }
    if ((Dnx_soc_proc_id_pool_size == 0) ||
          (Dnx_soc_proc_id_pool_num_elements == 0))
    {
      ret = 3 ;
      goto exit ;
    }
    proc_id_ptr = &proc_id ;
    void_ptr =
      dnx_sand_os_bsearch(
        (void *)proc_id_ptr,
        (void *)Dnx_soc_procedure_desc_element,
        (uint32)Dnx_soc_proc_id_pool_num_elements,
        sizeof(*Dnx_soc_procedure_desc_element),
        dnx_soc_compare_proc_desc_elements) ;
    *procedure_desc_element_ptr = (DNX_PROCEDURE_DESC_ELEMENT *)void_ptr ;
  }
  /*
   * Now free mutex.
   */
  dnx_sand_ret = dnx_sand_os_mutex_give(Dnx_soc_proc_id_pool_mutex) ;
  if (dnx_sand_ret != DNX_SAND_OK)
  {
    ret = 4 ;
  }
exit:
  return (ret) ;
}
/*****************************************************
*NAME
* dnx_sand_linear_find_procedure
*TYPE:
*  PROC
*DATE:
*  17/FEB/2003
*FUNCTION:
*  Find indicated procedure description in the specified
*  non-sorted pool and point to it.
*CALLING SEQUENCE:
*  dnx_sand_linear_find_procedure(
*    error_code,
*    in_procedure_desc_element,
*    procedure_desc_element_ptr)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32          proc_id -
*      Error code to locate.
*    DNX_PROCEDURE_DESC_ELEMENT *in_procedure_desc_element -
*      Pointer to error descriptor list to search
*      (linearly).
*    DNX_PROCEDURE_DESC_ELEMENT **procedure_desc_element_ptr -
*      This procedure loads pointed memory with pointer
*      to found item. If no item is found, a value
*      of '0' (null pointer) is loaded into
*      pointed memory.
*  DNX_SAND_INDIRECT:
*    in_procedure_desc_element
*OUTPUT:
*  DNX_SAND_DIRECT:
*    Non-Zero in case of an error
*  DNX_SAND_INDIRECT:
*    See procedure_desc_element_ptr.
*REMARKS:
*  This procedure is only to be used in case binary
*  search has failed (e.g. when dnx_sand module or some
*  other module has not been started yet.
*
*SEE ALSO:
*****************************************************/
int
  dnx_sand_linear_find_procedure(
    uint32          proc_id,
    DNX_SAND_IN DNX_PROCEDURE_DESC_ELEMENT *in_procedure_desc_element,
    CONST DNX_PROCEDURE_DESC_ELEMENT **procedure_desc_element_ptr
  )
{
  int
    ret ;
  CONST DNX_PROCEDURE_DESC_ELEMENT
    *procedure_desc_element ;
  unsigned
    int
      ui ;
  ret = 0 ;
  *procedure_desc_element_ptr = (DNX_PROCEDURE_DESC_ELEMENT *)0 ;
  if (in_procedure_desc_element == (DNX_PROCEDURE_DESC_ELEMENT *)0)
  {
    /*
     * Enter if input array is null. Just quit
     */
    ret = 1 ;
    goto exit ;
  }
  procedure_desc_element = in_procedure_desc_element ;
  /*
   * Search all elements in input unsorted pool.
   */
  for (ui = 0 ; ui < MAX_PROC_ID_POOL_ELEMENT ; ui++, procedure_desc_element++)
  {
    if (procedure_desc_element->proc_id == DNX_SAND_END_PROC_LIST)
    {
      break ;
    }
    if (procedure_desc_element->proc_id == proc_id)
    {
      *procedure_desc_element_ptr = procedure_desc_element ;
      break ;
    }
  }
exit:
  return (ret) ;
}
/*
 * }
 */
/*****************************************************
*NAME
* dnx_sand_proc_id_to_string
*TYPE:
*  PROC
*DATE:
*  03/FEB/2003
*FUNCTION:
*  Get ASCII names of module and procedure from input
*  procedure id.
*CALLING SEQUENCE:
*  dnx_sand_proc_id_to_string(
*        proc_id,out_module_name,out_proc_name)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  proc_id -
*      Procedure id to locate namde and module of.
*    char           **out_module_name -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      the name of the module.
*    char           **out_proc_name -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      the name of the procedure.
*  DNX_SAND_INDIRECT:
*    All-system sorted procedure descriptors pool
*    (Dnx_soc_procedure_desc_element).
*OUTPUT:
*  DNX_SAND_DIRECT:
*    int -
*      If non-zero then some error has occurred and
*      procedure string has not been located.
*  DNX_SAND_INDIRECT:
*    See out_module_name, out_module_name.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_proc_id_to_string(
    DNX_SAND_IN  uint32 in_proc_id,
    DNX_SAND_OUT char    **out_module_name,
    DNX_SAND_OUT char    **out_proc_name
  )
{
  int
    err ;
  DNX_PROCEDURE_DESC_ELEMENT
    *procedure_desc_element ;
  unsigned
    int
      module_id ;
  module_id = in_proc_id >> (DNX_PROC_ID_NUM_BITS - DNX_SAND_MODULE_ID_NUM_BITS) ;
  module_id &= (DNX_SAND_BIT(DNX_SAND_MODULE_ID_NUM_BITS) - 1) ;
  switch (module_id)
  {
    case DNX_SAND_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "DNX_SAND module" ;
      break ;
    }
    case FE200_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "DNX_SAND_FE200 module" ;
      break ;
    }
    case FE600_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "DNX_SAND_FE600 module" ;
      break ;
    }
    case FAP10M_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "DNX_SAND_FAP10M module" ;
      break ;
    }
    case FAP20V_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "DNX_SAND_FAP20V module" ;
      break ;
    }
    case FAP21V_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "DNX_SAND_FAP21V module" ;
      break ;
    }
    case SOC_PETRA_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "SOC_PETRA module" ;
      break ;
    }
    case SOC_PETRA_PP_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "SOC_PETRA PP module" ;
      break ;
    }
    case SOC_PB_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "SOC_PETRA-B module" ;
      break ;
    }
    case SOC_PB_PP_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "SOC_PETRA-B PP module" ;
      break ;
    }
    case DNX_TMC_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "TMC module" ;
      break ;
    }
    case TMD_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "TMD module" ;
      break ;
    }    
    case SOC_PPD_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "PPD module" ;
      break ;
    }
    case JER2_ARAD_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "JER2_ARAD module" ;
      break ;
    }
    case JER2_ARAD_PP_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "JER2_ARAD PP module" ;
      break ;
    }
    case T20E_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "T20E module" ;
      break ;
    }
    case FMF_MODULE_IDENTIFIER_DNX:
    {
      *out_module_name = "FMF or PRDS module" ;
      break ;
    }
    default:
    {
      *out_module_name = "Unknown_module" ;
      *out_proc_name = "Unknown_procedure" ;
      err = 1 ;
      goto exit;
    }
  }
  err = dnx_sand_find_procedure(in_proc_id,&procedure_desc_element) ;
  if (err || (procedure_desc_element == (DNX_PROCEDURE_DESC_ELEMENT *)0))
  {
    *out_proc_name = "Unknown_procedure" ;
    err = 1 ;
  }
  else
  {
    /*
     * Copy module_name and procedure_name pointers into user
     * pointed memory
     */
    *out_proc_name = (char *)procedure_desc_element->proc_name ;
    err = 0 ;
  }
exit:
  return (err) ;
}
/*****************************************************
*NAME
* dnx_sand_error_code_to_string
*TYPE:
*  PROC
*DATE:
*  17/FEB/2003
*FUNCTION:
*  Get ASCII names of error and its description from
*  input error id.
*CALLING SEQUENCE:
*  dnx_sand_error_code_to_string(
*        in_error_code,out_error_name,
*        out_error_description)
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32  proc_id -
*      Procedure id to locate namde and module of.
*    char           **out_error_name -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      the name of the error/event.
*    char           **out_error_description -
*      This procedure loads pointed memory with
*      pointer to null terminated string containing
*      the description of the error.
*  DNX_SAND_INDIRECT:
*    All-system sorted errors descriptors pool
*    (Dnx_soc_sand_p_error_desc_element).
*OUTPUT:
*  DNX_SAND_DIRECT:
*    None.
*  DNX_SAND_INDIRECT:
*    See out_error_name, out_error_description.
*REMARKS:
*SEE ALSO:
*****************************************************/
int
  dnx_sand_error_code_to_string(
    DNX_SAND_IN  uint32 in_error_code,
    DNX_SAND_OUT char    **out_error_name,
    DNX_SAND_OUT char    **out_error_description
  )
{
  int
    err ;
  DNX_ERROR_DESC_ELEMENT
    *error_desc_element ;
  err = dnx_sand_find_error(in_error_code,&error_desc_element) ;
  if (err || (error_desc_element == (DNX_ERROR_DESC_ELEMENT *)0) || (error_desc_element->err_name  == (char *)0))
  {
    *out_error_name = "Unknown_error_code" ;
    *out_error_description = "Unknown error description" ;
    err = 1 ;
  }
  else
  {
    /*
     * Copy error_name and error_description pointers into user
     * pointed memory
     */
    *out_error_name = error_desc_element->err_name ;
    *out_error_description = error_desc_element->err_text ;
    err = 0 ;
  }
  return (err) ;
}
void
  dnx_sand_error_code_handler(
    DNX_SAND_IN DNX_SAND_RET error_code,
    DNX_SAND_IN char*    error_file,
    DNX_SAND_IN int      error_line,
    DNX_SAND_IN char*    error_func_name,
    DNX_SAND_IN char*    error_msg
  )
{
  char* error_name ;
  char* error_description ;
  char  msg[DNX_SAND_CALLBACK_BUF_SIZE] ;
  uint32 msg_index ;
  uint32 str_size ;

  if (DNX_SAND_OK == error_code)
  {
    /*
     * NO Error occurred
     */
    goto exit ;
  }

  /*
   * Error occurred
   */

  error_name = NULL ;
  error_description = NULL ;

  dnx_sand_error_code_to_string(error_code, &error_name, &error_description) ;

  msg[0] = '\n' ;
  msg[1] = '\r' ;
  msg[2] = '\0' ;
  msg_index = 2 ;
  if (NULL != error_file)
  {
    str_size = dnx_sand_os_strlen(error_file) ;
    if (str_size + msg_index  + 5 < DNX_SAND_CALLBACK_BUF_SIZE-1)
    {
      if (!(Dnx_soc_no_error_printing))
      {
        sal_sprintf(msg + msg_index, "File:%s:%d", error_file, error_line) ;
      }
      msg_index = dnx_sand_os_strlen(msg) ;
    }
  }
  if (NULL != error_func_name)
  {
    str_size = dnx_sand_os_strlen(error_func_name) ;
    if (str_size + msg_index < DNX_SAND_CALLBACK_BUF_SIZE-1)
    {
      if (!(Dnx_soc_no_error_printing))
      {
        sal_sprintf(msg + msg_index, "\r\nFunc Name:%s", error_func_name) ;
      }
      msg_index = dnx_sand_os_strlen(msg) ;
    }
  }
  if (NULL != error_name)
  {
    str_size = dnx_sand_os_strlen(error_name) ;
    if (str_size + msg_index < DNX_SAND_CALLBACK_BUF_SIZE-1)
    {
      if (!(Dnx_soc_no_error_printing))
      {
        sal_sprintf(msg + msg_index, "\r\nError Name:%s", error_name) ;
      }
      msg_index = dnx_sand_os_strlen(msg) ;
    }
  }
  if (NULL != error_description)
  {
    str_size = dnx_sand_os_strlen(error_description) ;
    if (str_size + msg_index < DNX_SAND_CALLBACK_BUF_SIZE-1)
    {
      if (!(Dnx_soc_no_error_printing))
      {
        sal_sprintf(msg + msg_index, "\r\nError Description:%s", error_description) ;
      }
      msg_index = dnx_sand_os_strlen(msg) ;
    }
  }
  if (NULL != error_msg)
  {
    str_size = dnx_sand_os_strlen(error_msg) ;
    if (str_size + msg_index < DNX_SAND_CALLBACK_BUF_SIZE-1)
    {
      if (!(Dnx_soc_no_error_printing))
      {
        sal_sprintf(msg + msg_index, "\r\nError Message:%s", error_msg) ;
      }
      msg_index = dnx_sand_os_strlen(msg) ;
    }
  }
  dnx_sand_error_handler(error_code, msg, 0, 0, 0, 0, 0, 0) ;
exit:
  return ;
}


/*****************************************************
*NAME
*  dnx_sand_check_CREDIT_WORTH
*TYPE:
*  PROC
*DATE:
*  21-Nov-02
*FUNCTION:
*  Check that input 'credit_worth' is one
*   of the 4 possibilities.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN  DNX_SAND_CREDIT_SIZE credit_worth -
*       number to check.
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 -
*      See formatting rules in ERROR RETURN VALUES above.
*      If error code is not FAP10M_NO_ERR then
*        specific error codes:
*          FAP10M_GEN_ERR.
*      Otherwise, no error has been detected and device
*        has been written.
*  DNX_SAND_INDIRECT:
*    NON
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
DNX_SAND_RET
  dnx_sand_check_CREDIT_WORTH(
    DNX_SAND_IN  DNX_SAND_CREDIT_SIZE credit_worth
  )
{
  DNX_SAND_RET
    dnx_sand_ret;

  if( (credit_worth == DNX_SAND_CR_256) ||
      (credit_worth == DNX_SAND_CR_512) ||
      (credit_worth == DNX_SAND_CR_1024) ||
      (credit_worth == DNX_SAND_CR_2048) ||
      (credit_worth == DNX_SAND_CR_4096) ||
      (credit_worth == DNX_SAND_CR_8192)
   )
  {
    dnx_sand_ret = DNX_SAND_OK;
  }
  else
  {
    dnx_sand_ret = DNX_SAND_CREDIT_SIZE_ENUM_ERR;
  }
  return dnx_sand_ret;
}

#if DNX_SAND_DEBUG
/* { */
/*
 */

/*****************************************************
*NAME
*  dnx_sand_disp_result
*TYPE: PROC
*DATE: 18/FEB/2003
*FUNCTION:
*  Display return value and related information for
*  DNX_SAND driver services.
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_IN uint32 driver_api_result -
*      Return value of any DNX_SAND driver service
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    int -
*      If non zero then some error has occurred.
*  DNX_SAND_INDIRECT:
*    Processing results.
*REMARKS:
*  None.
*SEE ALSO:
*****************************************************/
int
  dnx_sand_disp_result(
    DNX_SAND_IN uint32 driver_api_result
  )
{
  int
    ret ;
  uint32
      proc_id ;
  int
      err_id ;
  char
    *err_name,
    *err_text,
    *dnx_sand_proc_name,
    *dnx_sand_module_name ;
  unsigned short
    error_code;

  ret = 0 ;
  error_code = dnx_sand_get_error_code_from_error_word( driver_api_result ) ;
  err_id = dnx_sand_error_code_to_string(error_code, &err_name,&err_text) ;
  /*
   * Let driver print whatever it wishes before going ahead here.
   */
  if (dnx_sand_get_error_code_from_error_word(err_id) != DNX_SAND_OK)
  {
    err_text = "No error code description (or procedure id) found" ;
  }

  proc_id = dnx_sand_get_proc_id_from_error_word(driver_api_result) ;
  dnx_sand_proc_id_to_string(proc_id,&dnx_sand_module_name,&dnx_sand_proc_name)  ;
  if (error_code != DNX_SAND_OK)
  {
    if (!(Dnx_soc_no_error_printing))
    {
      LOG_CLI((BSL_META("  Code 0x%X (fail):\r\n"
                        "Text        : %s\n\r" /*Error name*/
                        "%s\r\n"               /*Error description*/
                        "Procedure id: 0x%04X (Mod: %s, Proc: %s)\n\r"),
               driver_api_result, err_name, err_text,
               proc_id,
               dnx_sand_module_name,
               dnx_sand_proc_name
               ));
    }
  }
  else
  {
    /*
     * No print on success
     */
  }
  return (ret) ;
}
int
  dnx_sand_disp_result_proc(
    DNX_SAND_IN uint32 driver_api_result,
    DNX_SAND_IN char              *proc_name
  )
{
  int
    ret ;
  unsigned short
    error_code;

  ret = 0 ;
  error_code = dnx_sand_get_error_code_from_error_word( driver_api_result ) ;
  if (error_code != DNX_SAND_OK)
  {
    if (!(Dnx_soc_no_error_printing))
    {
      /*
       *	Print the proc name
       */
      LOG_CLI((BSL_META("\n\r"
                        "**>\'%s\' -\n\r"),
               proc_name));
    }
  }
  ret = dnx_sand_disp_result(driver_api_result);

  return (ret) ;
}

/*
 *  ERROR printing get and set
 */
void
  dnx_sand_no_error_printing_set(
    uint8   no_error_printing
    )
{
  Dnx_soc_no_error_printing = no_error_printing ;
}

uint8
  dnx_sand_no_error_printing_get(
       void
    )
{
  return Dnx_soc_no_error_printing;
}


/*
 * }
 */
#endif

