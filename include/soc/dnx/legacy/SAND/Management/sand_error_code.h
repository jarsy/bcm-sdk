/* $Id: sand_error_code.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_SAND_ERROR_CODE_H_INCLUDED__
/* { */
#define __DNX_SAND_ERROR_CODE_H_INCLUDED__
#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>

/* $Id: sand_error_code.h,v 1.13 Broadcom SDK $
 * Debug Macros {
 */
/*
 * Macros to be used throught the code in order to inform
 * the user inmformation.
 */

#define DEBUG_PRINTS 1
#if DEBUG_PRINTS
#define DNX_SAND_INFO_PRINTF(x)  \
  { \
    dnx_sand_os_printf("++dnx_sand info: %s:%s\r\n", __FILE__, __LINE__); \
    dnx_sand_os_printf(x); \
  }
#define DNX_SAND_ERROR_PRINTF(x)  \
  { \
    dnx_sand_os_printf("**dnx_sand error: %s:%s\r\n", __FILE__, __LINE__); \
    dnx_sand_os_printf(x); \
  }
#else
#define DNX_SAND_INFO_PRINTF(x)
#define DNX_SAND_ERROR_PRINTF(x)
#endif
/*
 * } END Debug Macros
 */
/*
 * System-wide error handling
 * {
 */
/*
 * Severity classification of events:
 * just message, warning, error, fatal error (i.e., unrecoverable error)
 */
#define DNX_SAND_SVR_MSG 0
#define DNX_SAND_SVR_WRN 1
#define DNX_SAND_SVR_ERR 2
#define DNX_SAND_SVR_FTL 3
/*
 * Structure of one element in array of all-system errors.
 * This array is kept sorted in malloc'ed area.
 */
typedef struct
{
  uint32  err_id ;
  char           *err_name ;
  char           *err_text ;
  unsigned short severity ;
    /*
     * Flag. If set then driver is required to not report this
     * event via 'dnx_sand_error_handler()'.
     */
  unsigned short keep_internal ;
} DNX_ERROR_DESC_ELEMENT ;
/*
 * }
 */
/*
 * ERROR RETURN VALUES
 * {
 */
#ifdef DNX_SAND_ERROR_CODE_OLD_MODE

/*
 * Error return value is uint32 composed of the
 * fields specified below.
 * Error code -
 *   Identifier of the error. Up to 64K error codes are allowed.
 *   BITS 0 -> 15
 * Procedure id -
 *   Identifier of procedure returning error code. Up to 8k procedures
 *   are allowed.
 *   BITS 16 -> 26
 * Module id -
 *   Identifier of Module returning error code. Up to 8 modules
 *   are allowed.
 *   BITS 27 -> 29
 * Error severity -
 *   Flag. Indicator on whether error level was such that procedure
 *   could not carry out its function or whether it was just a
 *   warning and procedure could carry out its function. One bit.
 *   DNX_SAND_BIT 30
 * Callback Procedure -
 *   Flag. Indicator on whether procedure returning error value
 *   is a callback procedure or direct-return procedure. One bit.
 *   DNX_SAND_BIT 31
 */
#define DNX_SAND_ERROR_CODE_MS_BIT     15
#define DNX_SAND_ERROR_CODE_NUM_BITS   16
#define DNX_SAND_ERROR_CODE_LS_BIT     (DNX_SAND_ERROR_CODE_MS_BIT + 1 - DNX_SAND_ERROR_CODE_NUM_BITS)
#define DNX_SAND_ERROR_CODE_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_ERROR_CODE_MS_BIT) - DNX_SAND_BIT(DNX_SAND_ERROR_CODE_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_ERROR_CODE_MS_BIT))
#define DNX_SAND_ERROR_CODE_SHIFT      DNX_SAND_ERROR_CODE_LS_BIT
#define DNX_SAND_ERROR_CODE_MAX_VAL    (DNX_SAND_BIT(DNX_SAND_ERROR_CODE_NUM_BITS) - 1)
#define DNX_SAND_ERROR_CODE_MAX_MASK   (DNX_SAND_BIT(DNX_SAND_ERROR_CODE_NUM_BITS) - 1)

#define DNX_PROC_ID_MS_BIT        29
#define DNX_PROC_ID_NUM_BITS      14
#define DNX_PROC_ID_LS_BIT        (DNX_PROC_ID_MS_BIT + 1 - DNX_PROC_ID_NUM_BITS)
#define DNX_PROC_ID_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_PROC_ID_MS_BIT) - DNX_SAND_BIT(DNX_PROC_ID_LS_BIT)) + DNX_SAND_BIT(DNX_PROC_ID_MS_BIT))
#define DNX_PROC_ID_SHIFT         DNX_PROC_ID_LS_BIT
#define DNX_PROC_ID_MAX_VAL       (DNX_SAND_BIT(DNX_PROC_ID_NUM_BITS) - 1)
#define DNX_PROC_ID_MAX_MASK      (DNX_SAND_BIT(DNX_PROC_ID_NUM_BITS) - 1)

#define DNX_SAND_ERR_SEVERE_MS_BIT         30
#define DNX_SAND_ERR_SEVERE_NUM_BITS       1
#define DNX_SAND_ERR_SEVERE_LS_BIT         (DNX_SAND_ERR_SEVERE_MS_BIT + 1 - DNX_SAND_ERR_SEVERE_NUM_BITS)
#define DNX_SAND_ERR_SEVERE_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_MS_BIT) - DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_MS_BIT))
#define DNX_SAND_ERR_SEVERE_SHIFT          DNX_SAND_ERR_SEVERE_LS_BIT
#define DNX_SAND_ERR_SEVERE_MAX_VAL        (DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_NUM_BITS) - 1)

#define DNX_SAND_CALLBACK_PROC_MS_BIT      31
#define DNX_SAND_CALLBACK_PROC_NUM_BITS    1
#define DNX_SAND_CALLBACK_PROC_LS_BIT      (DNX_SAND_CALLBACK_PROC_MS_BIT + 1 - DNX_SAND_CALLBACK_PROC_NUM_BITS)
#define DNX_SAND_CALLBACK_PROC_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_MS_BIT) - DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_MS_BIT))
#define DNX_SAND_CALLBACK_PROC_SHIFT       DNX_SAND_CALLBACK_PROC_LS_BIT
#define DNX_SAND_CALLBACK_PROC_MAX_VAL     (DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_NUM_BITS) - 1)

#define DNX_SAND_MODULE_ID_MS_BIT      29
#define DNX_SAND_MODULE_ID_NUM_BITS    4
#define DNX_SAND_MODULE_ID_LS_BIT      (DNX_SAND_MODULE_ID_MS_BIT + 1 - DNX_SAND_MODULE_ID_NUM_BITS)
#define DNX_SAND_MODULE_ID_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_MODULE_ID_MS_BIT) - DNX_SAND_BIT(DNX_SAND_MODULE_ID_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_MODULE_ID_MS_BIT))
#define DNX_SAND_MODULE_ID_SHIFT       DNX_SAND_MODULE_ID_LS_BIT
#define DNX_SAND_MODULE_ID_MAX_VAL     (DNX_SAND_BIT(DNX_SAND_MODULE_ID_NUM_BITS) - 1)

#else /*#ifdef DNX_SAND_ERROR_CODE_OLD_MODE*/

/*
 * Error return value is uint32 composed of the
 * fields specified below.
 * Error code -
 *   Identifier of the error. Up to 4K error codes are allowed.
 *   BITS 0 -> 11
 * Procedure id -
 *   Identifier of procedure returning error code. Up to 8k procedures
 *   are allowed, per module type
 *   BITS 12 -> 24
 * Module id -
 *   Identifier of Module returning error code. Up to 32 modules
 *   are allowed.
 *   BITS 25 -> 29
 * Error severity -
 *   Flag. Indicator on whether error level was such that procedure
 *   could not carry out its function or whether it was just a
 *   warning and procedure could carry out its function. One bit.
 *   DNX_SAND_BIT 30
 * Callback Procedure -
 *   Flag. Indicator on whether procedure returning error value
 *   is a callback procedure or direct-return procedure. One bit.
 *   DNX_SAND_BIT 31
 */
#define DNX_SAND_ERROR_CODE_MS_BIT     12
#define DNX_SAND_ERROR_CODE_NUM_BITS   13
#define DNX_SAND_ERROR_CODE_LS_BIT     (DNX_SAND_ERROR_CODE_MS_BIT + 1 - DNX_SAND_ERROR_CODE_NUM_BITS)
#define DNX_SAND_ERROR_CODE_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_ERROR_CODE_MS_BIT) - DNX_SAND_BIT(DNX_SAND_ERROR_CODE_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_ERROR_CODE_MS_BIT))
#define DNX_SAND_ERROR_CODE_SHIFT      DNX_SAND_ERROR_CODE_LS_BIT
#define DNX_SAND_ERROR_CODE_MAX_VAL    (DNX_SAND_BIT(DNX_SAND_ERROR_CODE_NUM_BITS) - 1)
#define DNX_SAND_ERROR_CODE_MAX_MASK   (DNX_SAND_BIT(DNX_SAND_ERROR_CODE_NUM_BITS) - 1)

#define DNX_PROC_ID_MS_BIT        29
#define DNX_PROC_ID_NUM_BITS      17
#define DNX_PROC_ID_LS_BIT        (DNX_PROC_ID_MS_BIT + 1 - DNX_PROC_ID_NUM_BITS)
#define DNX_PROC_ID_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_PROC_ID_MS_BIT) - DNX_SAND_BIT(DNX_PROC_ID_LS_BIT)) + DNX_SAND_BIT(DNX_PROC_ID_MS_BIT))
#define DNX_PROC_ID_SHIFT         DNX_PROC_ID_LS_BIT
#define DNX_PROC_ID_MAX_VAL       (DNX_SAND_BIT(DNX_PROC_ID_NUM_BITS) - 1)
#define DNX_PROC_ID_MAX_MASK      (DNX_SAND_BIT(DNX_PROC_ID_NUM_BITS) - 1)

#define DNX_SAND_ERR_SEVERE_MS_BIT         30
#define DNX_SAND_ERR_SEVERE_NUM_BITS       1
#define DNX_SAND_ERR_SEVERE_LS_BIT         (DNX_SAND_ERR_SEVERE_MS_BIT + 1 - DNX_SAND_ERR_SEVERE_NUM_BITS)
#define DNX_SAND_ERR_SEVERE_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_MS_BIT) - DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_MS_BIT))
#define DNX_SAND_ERR_SEVERE_SHIFT          DNX_SAND_ERR_SEVERE_LS_BIT
#define DNX_SAND_ERR_SEVERE_MAX_VAL        (DNX_SAND_BIT(DNX_SAND_ERR_SEVERE_NUM_BITS) - 1)

#define DNX_SAND_CALLBACK_PROC_MS_BIT      31
#define DNX_SAND_CALLBACK_PROC_NUM_BITS    1
#define DNX_SAND_CALLBACK_PROC_LS_BIT      (DNX_SAND_CALLBACK_PROC_MS_BIT + 1 - DNX_SAND_CALLBACK_PROC_NUM_BITS)
#define DNX_SAND_CALLBACK_PROC_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_MS_BIT) - DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_MS_BIT))
#define DNX_SAND_CALLBACK_PROC_SHIFT       DNX_SAND_CALLBACK_PROC_LS_BIT
#define DNX_SAND_CALLBACK_PROC_MAX_VAL     (DNX_SAND_BIT(DNX_SAND_CALLBACK_PROC_NUM_BITS) - 1)

/* 
 * Note: this field is part of the DNX_PROC_ID field,
 */
#define DNX_SAND_MODULE_ID_MS_BIT      29
#define DNX_SAND_MODULE_ID_NUM_BITS    5
#define DNX_SAND_MODULE_ID_LS_BIT      (DNX_SAND_MODULE_ID_MS_BIT + 1 - DNX_SAND_MODULE_ID_NUM_BITS)
#define DNX_SAND_MODULE_ID_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_MODULE_ID_MS_BIT) - DNX_SAND_BIT(DNX_SAND_MODULE_ID_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_MODULE_ID_MS_BIT))
#define DNX_SAND_MODULE_ID_SHIFT       DNX_SAND_MODULE_ID_LS_BIT
#define DNX_SAND_MODULE_ID_MAX_VAL     (DNX_SAND_BIT(DNX_SAND_MODULE_ID_NUM_BITS) - 1)

#endif /*#ifdef DNX_SAND_ERROR_CODE_OLD_MODE*/

/*
 * }
 */
/*
 * System-wide procedure identification handling
 * {
 */
/*
 * Structure of one element in array of all-system procedure
 * descriptors.
 * This array is kept sorted in malloc'ed area.
 */
typedef struct
{
  uint32  proc_id ;
  char           *proc_name ;
} DNX_PROCEDURE_DESC_ELEMENT ;

/* To be used to init DNX_PROCEDURE_DESC_ELEMENT with single argument, when one wants
 * to use a macro for both proc_id and proc_name
 */
#define DNX_PROCEDURE_DESC_ELEMENT_DEF(proc_desc) {proc_desc, #proc_desc}
#define DNX_PROCEDURE_DESC_ELEMENT_DEF_LAST DNX_PROCEDURE_DESC_ELEMENT_DEF((uint32)DNX_SAND_END_PROC_LIST)

#define DNX_SAND_ERR_DESC_ELEMENT_DEF_LAST \
  { \
    (uint32)DNX_SAND_END_ERR_LIST, \
    "", \
    "", \
    DNX_SAND_SVR_FTL, \
    FALSE \
  }

#define DNX_SAND_PROC_BITS_SET(_module_id)  \
  (_module_id    << (DNX_PROC_ID_NUM_BITS - DNX_SAND_MODULE_ID_NUM_BITS))

/*
 * The following bit masks are or'ed with procedure id's within the various
 * modules, to identify the owner module
 */
#define DNX_SAND_MODULE_IDENTIFIER_DNX          1
#define DNX_SAND_PROC_BITS                 DNX_SAND_PROC_BITS_SET(DNX_SAND_MODULE_IDENTIFIER_DNX)

#define FE200_MODULE_IDENTIFIER_DNX					2

#define FAP10M_MODULE_IDENTIFIER_DNX				3

#define FMF_MODULE_IDENTIFIER_DNX						4

#define FAP20V_MODULE_IDENTIFIER_DNX				5


#define TIMNA_MODULE_IDENTIFIER_DNX					7


#define FAP20M_MODULE_IDENTIFIER_DNX				9

#define SOC_PETRA_MODULE_IDENTIFIER_DNX					10
#define DNX_PETRA_PROC_BITS DNX_SAND_PROC_BITS_SET(SOC_PETRA_MODULE_IDENTIFIER_DNX)

#define FAP21V_MODULE_IDENTIFIER_DNX				11

#define SOC_PETRA_PP_MODULE_IDENTIFIER_DNX			12

#define FE600_MODULE_IDENTIFIER_DNX					13

#define T20E_MODULE_IDENTIFIER_DNX					14

#define SOC_PB_MODULE_IDENTIFIER_DNX						15
#define DNX_SAND_PB_PROC_BITS DNX_SAND_PROC_BITS_SET(DNX_SAND_PB_MODULE_IDENTIFIER_DNX)

#define SOC_PPD_MODULE_IDENTIFIER_DNX						16
#define DNX_PPD_PROC_BITS DNX_SAND_PROC_BITS_SET(SOC_PPD_MODULE_IDENTIFIER_DNX)

#define PPH_MODULE_IDENTIFIER_DNX						17

#define OAM_MODULE_IDENTIFIER_DNX						18

#define SOC_PB_PP_MODULE_IDENTIFIER_DNX					19
#define DNX_SAND_PB_PP_PROC_BITS DNX_SAND_PROC_BITS_SET(DNX_SAND_PB_PP_MODULE_IDENTIFIER_DNX)

#define TMD_MODULE_IDENTIFIER_DNX						20

#define DNX_TMC_MODULE_IDENTIFIER_DNX						21
#define DNX_TMC_PROC_BITS DNX_SAND_PROC_BITS_SET(DNX_TMC_MODULE_IDENTIFIER_DNX)

#define PCP_MODULE_IDENTIFIER_DNX						22
#define DNX_PCP_PROC_BITS DNX_SAND_PROC_BITS_SET(PCP_MODULE_IDENTIFIER_DNX)

#define JER2_ARAD_MODULE_IDENTIFIER_DNX					23
#define JER2_ARAD_PROC_BITS DNX_SAND_PROC_BITS_SET(JER2_ARAD_MODULE_IDENTIFIER_DNX)

#define JER2_ARAD_PP_MODULE_IDENTIFIER_DNX					25
#define JER2_ARAD_PP_PROC_BITS DNX_SAND_PROC_BITS_SET(JER2_ARAD_PP_MODULE_IDENTIFIER_DNX)


/*
 * }
 */
/*
 * List of DNX_SAND procedure identifiers.
 * {
 */
/*
 * All dnx_sand procedures have DNX_SAND_PROC_BIT set. This
 * distinguishes dnx_sand procedures from upper layer procedures.
 */
#define DNX_SAND_MODULE_OPEN                                    (1  | DNX_SAND_PROC_BITS)
#define DNX_SAND_MODULE_CLOSE                                   (2  | DNX_SAND_PROC_BITS)
#define DNX_SAND_PHYSICAL_READ_FROM_CHIP                        (3  | DNX_SAND_PROC_BITS)
#define DNX_SAND_PHYSICAL_WRITE_TO_CHIP                         (4  | DNX_SAND_PROC_BITS)
#define DNX_SAND_READ_MODIFY_WRITE                              (5  | DNX_SAND_PROC_BITS)
#define DNX_SAND_DEVICE_REGISTER                                (6  | DNX_SAND_PROC_BITS)
#define DNX_SAND_DEVICE_UNREGISTER                              (7  | DNX_SAND_PROC_BITS)
#define DNX_SAND_HANDLES_INIT_HANDLES                           (8  | DNX_SAND_PROC_BITS)
#define DNX_SAND_HANDLES_SHUT_DOWN_HANDLES                      (9  | DNX_SAND_PROC_BITS)
#define DNX_SAND_HANDLES_REGISTER_HANDLE                        (10 | DNX_SAND_PROC_BITS)
#define DNX_SAND_HANDLES_UNREGISTER_HANDLE                      (11 | DNX_SAND_PROC_BITS)
#define DNX_SAND_HANDLES_UNREGISTER_ALL_DEVICE_HANDLES          (12 | DNX_SAND_PROC_BITS)
#define DNX_SAND_CLEAR_CHIP_DESCRIPTOR                          (13 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INIT_CHIP_DESCRIPTORS                          (14 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELETE_CHIP_DESCRIPTORS                        (15 | DNX_SAND_PROC_BITS)
#define DNX_SAND_REMOVE_CHIP_DESCRIPTOR                         (16 | DNX_SAND_PROC_BITS)
#define DNX_SAND_ADD_CHIP_DESCRIPTOR                            (17 | DNX_SAND_PROC_BITS)
#define DNX_SAND_REGISTER_EVENT_CALLBACK                        (18 | DNX_SAND_PROC_BITS)
#define DNX_SAND_UNREGISTER_EVENT_CALLBACK                      (19 | DNX_SAND_PROC_BITS)
#define DNX_SAND_COMBINE_2_EVENT_CALLBACK_HANDLES               (20 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_MUTEX_OWNER                (21 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SET_CHIP_DESCRIPTOR_MUTEX_OWNER                (22 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_MUTEX_COUNTER              (23 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SET_CHIP_DESCRIPTOR_MUTEX_COUNTER              (24 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INC_CHIP_DESCRIPTOR_MUTEX_COUNTER              (25 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DEC_CHIP_DESCRIPTOR_MUTEX_COUNTER              (26 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TAKE_CHIP_DESCRIPTOR_MUTEX                     (27 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GIVE_CHIP_DESCRIPTOR_MUTEX                     (28 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_BASE_ADDR                  (29 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_MEMORY_SIZE                (30 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_INTERRUPT_CALLBACK_ARRAY   (31 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_UNMASK_FUNC                (32 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_IS_BIT_AC_FUNC             (33 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_IS_DEVICE_INT_MASKED_FUNC  (34 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_GET_INT_MASK_FUNC          (35 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_DEV_BETWEEN_ISR_AND_TCM    (36 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SET_CHIP_DESCRIPTOR_DEV_BETWEEN_ISR_AND_TCM    (37 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_INT_MASK_COUNTER           (38 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INC_CHIP_DESCRIPTOR_INT_MASK_COUNTER           (39 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DEC_CHIP_DESCRIPTOR_INT_MASK_COUNTER           (40 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_CHIP_TYPE                  (43 | DNX_SAND_PROC_BITS)
#define DNX_SAND_IS_CHIP_DESCRIPTOR_VALID                       (44 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_MAGIC                      (45 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GENERAL_SET_MAX_NUM_DEVICES                    (46 | DNX_SAND_PROC_BITS)
#define DNX_SAND_MODULE_INIT_ALL                                (47 | DNX_SAND_PROC_BITS)
#define DNX_SAND_MODULE_END_ALL                                 (48 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_SEND_MESSAGE_TO_Q_FROM_TASK                (49 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_SEND_MESSAGE_TO_Q_FROM_INT                 (50 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_CALLBACK_ENGINE_START                      (51 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_REGISTER_POLLING_CALLBACK                  (52 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_UNREGISTER_POLLING_CALLBACK                (53 | DNX_SAND_PROC_BITS)
#define DNX_SAND_USER_CALLBACK_UNREGISTER_DEVICE                (54 | DNX_SAND_PROC_BITS)
#define DNX_SAND_UNPACK_RX_SR_DATA_CELL                         (55 | DNX_SAND_PROC_BITS)
#define DNX_SAND_PACK_TX_SR_DATA_CELL                           (56 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_SET_INFO                              (57 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_CLEAR_INFO                            (58 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_CLEAR_INFO_ALL                        (59 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_CHECK_REQUEST_LEGAL                   (60 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_VERIFY_TRIGGER_0                      (61 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_WRITE_ADDRESS                         (62 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_WRITE_VALUE                           (63 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_ASSERT_TRIGGER_1                      (64 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_READ_RESULT                           (65 | DNX_SAND_PROC_BITS)
#define DNX_SAND_MEM_INTERRUPT_MASK_ADDRESS_CLEAR               (66 | DNX_SAND_PROC_BITS)
#define DNX_SAND_MEM_READ                                       (67 | DNX_SAND_PROC_BITS)
#define DNX_SAND_MEM_WRITE                                      (68 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TRIGGER_VERIFY_0                               (69 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TRIGGER_ASSERT_1                               (70 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_TAKE_MUTEX                          (71 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_GIVE_MUTEX                          (72 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_DESTROY                             (73 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_INSERT_D                            (74 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_DECREASE_TIME_FROM_HEAD             (75 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_REMOVE                              (76 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INIT_ERRORS_QUEUE                              (77 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SET_USER_ERROR_HANDLER                         (78 | DNX_SAND_PROC_BITS)
#define DNX_SAND_LOAD_ERRORS_QUEUE                              (79 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELETE_ERRORS_QUEUE                            (80 | DNX_SAND_PROC_BITS)
#define DNX_SAND_ERROR_HANDLER                                  (81 | DNX_SAND_PROC_BITS)
#define DNX_SAND_UNLOAD_ERRORS_QUEUE                            (82 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_CALLBACK_ENGINE_MAIN                       (83 | DNX_SAND_PROC_BITS)
#define DNX_SAND_MEM_READ_CALLBACK                              (84 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_READ_FROM_CHIP                        (85 | DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_WRITE_TO_CHIP                         (86 | DNX_SAND_PROC_BITS)
#define DNX_SAND_READ_FIELD                                     (87 | DNX_SAND_PROC_BITS)
#define DNX_SAND_WRITE_FIELD                                    (88 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_DEVICE_AT_INIT             (89 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SET_CHIP_DESCRIPTOR_DEVICE_AT_INIT             (90 | DNX_SAND_PROC_BITS)
#define DNX_SAND_TCM_CALLBACK_DELTA_LIST_TAKE_MUTEX             (91 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SET_CHIP_DESCRIPTOR_VER_INFO                   (92 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DEVICE_MNGMNT_LOAD_CHIP_VER                    (93 | DNX_SAND_PROC_BITS)
#define DNX_SAND_IS_CHIP_DESCRIPTOR_CHIP_VER_BIGGER_EQ          (94 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_CHIP_VER                   (95 | DNX_SAND_PROC_BITS)
#define DNX_SAND_GET_CHIP_DESCRIPTOR_DBG_VER                    (96 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DELTA_LIST_DECREASE_TIME_FROM_SECOND_ITEM      (97 | DNX_SAND_PROC_BITS)
#define DNX_SAND_DEVICE_MNGMNT_GET_DEVICE_TYPE                  (98 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SSR_GET_VER_FROM_HEADER                        (99 | DNX_SAND_PROC_BITS)
#define DNX_SAND_SSR_GET_SIZE_FROM_HEADER                       (100| DNX_SAND_PROC_BITS)
#define DNX_SAND_MEM_WRITE_UNSAFE                               (101| DNX_SAND_PROC_BITS)
#define DNX_SAND_MEM_READ_UNSAFE                                (102| DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_READ_LOW                              (103| DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_WRITE_LOW                             (104| DNX_SAND_PROC_BITS)
#define DNX_SAND_BITSTREAM_SET_ANY_FIELD                        (105| DNX_SAND_PROC_BITS)
#define DNX_SAND_BITSTREAM_GET_ANY_FIELD                        (106| DNX_SAND_PROC_BITS)
#define DNX_SAND_IP_ADDR_NUMERIC_TO_STRING                      (107| DNX_SAND_PROC_BITS)
#define DNX_SAND_IP_ADDR_STRING_TO_NUMERIC                      (108| DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_WRITE_IND_INFO_LOW                    (109| DNX_SAND_PROC_BITS)
#define DNX_SAND_TBL_READ                                       (110| DNX_SAND_PROC_BITS)
#define DNX_SAND_TBL_READ_UNSAFE                                (111| DNX_SAND_PROC_BITS)
#define DNX_SAND_TBL_WRITE                                      (112| DNX_SAND_PROC_BITS)
#define DNX_SAND_TBL_WRITE_UNSAFE                               (113| DNX_SAND_PROC_BITS)
#define DNX_SAND_TBL_READ_LOW                                   (114| DNX_SAND_PROC_BITS)
#define DNX_SAND_TBL_WRITE_LOW                                  (115| DNX_SAND_PROC_BITS)
#define DNX_SAND_INDIRECT_READ_IND_INFO_LOW                     (116| DNX_SAND_PROC_BITS)
/*
 * BITMAP
 */
#define DNX_SAND_OCC_BM_CLEAR                                   (118| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_CREATE                                  (119| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_DESTROY                                 (120| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_GET_NEXT                                (121| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_PRINT                                   (122| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_GET_NEXT_HELPER                         (123| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_OCCUP_STATUS_SET                        (124| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_OCCUP_STATUS_SET_HELPER                 (125| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_ALLOC_NEXT                              (126| DNX_SAND_PROC_BITS)
#define DNX_SAND_OCC_BM_INIT                                    (127| DNX_SAND_PROC_BITS)
/*
 * Hash Table
 */
#define DNX_SAND_HASH_TABLE_CREATE                              (130| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_DESTROY                             (131| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_ENTRY_LOOKUP                        (132| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_ENTRY_ADD                           (133| DNX_SAND_PROC_BITS)

#define DNX_SAND_HASH_TABLE_ENTRY_REMOVE                        (135| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_ENTRY_REMOVE_BY_INDEX               (136| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_GET_NEXT                            (137| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_PRINT                               (138| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_FIND_ENTRY                          (139| DNX_SAND_PROC_BITS)
#define DNX_SAND_HASH_TABLE_INIT                                (140| DNX_SAND_PROC_BITS)
/*
 * Group Member Linked List
 */
#define DNX_SAND_GROUP_MEM_LL_IS_GROUP_EMPTY                    (129| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_CLEAR                             (134| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_CREATE                            (140| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_DESTROY                           (141| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBERS_SET                       (142| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBER_ADD                        (143| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBER_REMOVE                     (144| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBERS_GET                       (145| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBER_GET_GROUP                  (146| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBER_CLEAR                      (147| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_MEMBER_PRINT                      (148| DNX_SAND_PROC_BITS)
#define DNX_SAND_GROUP_MEM_LL_FUNC_RUN                          (149| DNX_SAND_PROC_BITS)
/*
 * memory management
 */
#define DNX_SAND_ARR_MEM_ALLOCATOR_CREATE                       (150| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_DESTROY                      (151| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC                       (152| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_FREE                         (153| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_READ                         (154| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_WRITE                        (155| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_READ_BLOCK                   (156| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_CLEAR                        (158| DNX_SAND_PROC_BITS)
#define DNX_SAND_ARR_MEM_ALLOCATOR_BLOCK_SIZE                   (159| DNX_SAND_PROC_BITS)

/*
 * DNX_SAND PP General
 */
#define DNX_SAND_PP_IPV4_SUBNET_VERIFY                          (157| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_TRILL_DEST_VERIFY                           (158| DNX_SAND_PROC_BITS)

/*
 * DNX_SAND PP MAC
 */
#define DNX_SAND_PP_MAC_ADDRESS_STRUCT_TO_LONG                  (160| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_LONG_TO_STRUCT                  (161| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_STRING_PARSE                    (162| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_INC                             (163| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_ADD                             (164| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_SUB                             (165| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_REVERSE                         (166| DNX_SAND_PROC_BITS)
#define DNX_SAND_PP_MAC_ADDRESS_ARE_EQUAL                       (167| DNX_SAND_PROC_BITS)

/*
 * Multi set
 */
#define DNX_SAND_MULTI_SET_CREATE                              (170| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_DESTROY                             (171| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_MEMBER_LOOKUP                       (172| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_MEMBER_ADD                          (173| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_MEMBER_REMOVE_BY_INDEX              (175| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_MEMBER_REMOVE                       (176| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_GET_NEXT                            (177| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_PRINT                               (178| DNX_SAND_PROC_BITS)
#define DNX_SAND_MULTI_SET_INIT                                (179| DNX_SAND_PROC_BITS)
/*
 * sorted list
 */
#define DNX_SAND_SORTED_LIST_CREATE                            (190| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_DESTROY                           (191| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_LOOKUP                      (192| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_ADD                         (193| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_UPDATE                      (194| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_REMOVE                      (195| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_GET_NEXT                          (196| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_GET_PREV                          (197| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_PRINT                             (198| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_FIND_MATCH_ENTRY                  (199| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_ADD_BY_ITER                 (200| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_REMOVE_BY_ITER              (201| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_ENTRY_VALUE                       (202| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_GET_FOLLOW                        (203| DNX_SAND_PROC_BITS)
#define DNX_SAND_SORTED_LIST_INIT                              (204| DNX_SAND_PROC_BITS)

/*
 * PAT tree
 */
#define DNX_SAND_PAT_TREE_CREATE                               (210| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_DESTROY                              (211| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_CLEAR                                (212| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_NODE_ADD                             (213| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_NODE_REMOVE                          (214| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_LPM_GET                              (215| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_GET_BLOCK                            (216| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_GET_SIZE                             (217| DNX_SAND_PROC_BITS)
#define DNX_SAND_PAT_TREE_PRINT                                (218| DNX_SAND_PROC_BITS)

/*
* Dnx_soc_sand cell
*/
#define DNX_SAND_PACK_DEST_ROUTED_DATA_CELL                    (219| DNX_SAND_PROC_BITS)
#define DNX_SAND_UNPACK_DEST_ROUTED_DATA_CELL                  (220| DNX_SAND_PROC_BITS)
#define DNX_SAND_PACK_SOURCE_ROUTED_DATA_CELL                  (221| DNX_SAND_PROC_BITS)
#define DNX_SAND_UNPACK_SOURCE_ROUTED_DATA_CELL                (222| DNX_SAND_PROC_BITS)
#define DNX_SAND_DATA_CELL_TO_BUFFER                           (223| DNX_SAND_PROC_BITS)
#define DNX_SAND_BUFFER_TO_DATA_CELL                           (224| DNX_SAND_PROC_BITS)

/* 
 * Exact_match 
 */
#define DNX_SAND_EXACT_MATCH_MALLOC_STACK                      (225| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_FREE_STACK                        (226| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_CREATE                            (227| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_SIZE_OUT_OF_RANGE_ERR             (228| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_DESTROY                           (229| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_ENTRY_ADD                         (230| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_ENTRY_REMOVE                      (231| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_ENTRY_LOOKUP                      (232| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_CLEAR                             (233| DNX_SAND_PROC_BITS)
#define DNX_SAND_EXACT_MATCH_GET_BLOCK                         (234| DNX_SAND_PROC_BITS)

/*
 * Dnx_soc_sand error code
 */


/*
 * System wide indicator of end of procedure descriptors list.
 */
#define DNX_SAND_END_PROC_LIST                                  (-1)
/*
 * } END List of DNX_SAND procedure identifiers
 */
/*
 * List of dnx_sand error codes
 */
typedef enum
{
  DNX_SAND_NO_ERR = DNX_SAND_OK,
  /*
   * First error code is 1
   */
  DNX_SAND_GEN_ERR = DNX_SAND_ERR,
  /*
   * Failure trying to do memory allocation.
   */
  DNX_SAND_MALLOC_FAIL,
  /*
   * Failure trying to do mutex allocation.
   */
  DNX_SAND_MUTEX_CREATE_FAIL,
  /*
   * NULL pointer was given, were not allowed.
   */
  DNX_SAND_NULL_POINTER_ERR,
  /*
   * One of the driver-usr callback function that was given
   * to the driver was NULL function.
   * E.g., E.g., 'fap10m_register_device()' reset_device_ptr was set to NULL
   * and 'fap10m_reset_device()' was called..
   */
  DNX_SAND_NULL_USER_CALLBACK_FUNC,
  /*
   * Tried to set or illegal/unknown device identifier.
   */
  DNX_SAND_MAX_NUM_DEVICES_OUT_OF_RANGE_ERR,
  /*
   * Driver has not been started. Can not activate
   * driver services.
   */
  DNX_SAND_DRIVER_NOT_STARTED,
  /*
   * Driver has already been started. Can not activate
   * driver again.
   */
  DNX_SAND_DRIVER_ALREADY_STARTED,
  /*
   * Just a message: Driver is currently busy. Try
   * again, after timeout, until driver gets free.
   */
  DNX_SAND_DRIVER_BUSY,
  /*
   * Illegal/unknown device identifier as input parameter
   */
  DNX_SAND_ILLEGAL_DEVICE_ID,
  /*
   * Illegal chip descriptor for specified device handle.
   */
  DNX_SAND_ILLEGAL_CHIP_DESCRIPTOR,
  /*
   * Illegal chip descriptor for specified device handle
   * because it has been deleted.
   */
  DNX_SAND_TRYING_TO_ACCESS_DELETED_DEVICE,
  /*
   * currently undefined.
   */
  DNX_SAND_SYSTEM_TICK_ERR_01,
  /*
   * currently undefined.
   */
  DNX_SAND_TCM_TASK_PRIORITY_ERR_01,
  /*
   * currently undefined.
   */
  DNX_SAND_MIN_TIME_BETWEEN_POLLS_ERR_01,
  /*
   * currently undefined.
   */
  DNX_SAND_TCM_MOCKUP_INTERRUPTS_ERR_01,
  /*
   * Procedure dnx_sand_add_sand_errors() returned with
   * error indication in dnx_sand_module_open().
   */
  DNX_SAND_USER_ADD_SAND_ERRORS_ERR_01,
  /*
   * Procedure dnx_sand_add_sand_procedures() returned with
   * error indication in dnx_sand_module_open().
   */
  DNX_SAND_USER_ADD_SAND_PROCEDURES_ERR_01,
  /*
   * Procedure dnx_sand_set_user_error_handler() returned with
   * error indication in dnx_sand_module_open().
   */
  DNX_SAND_USER_ERROR_HANDLER_ERR_01,
  /*
   * Procedure dnx_sand_module_end_all() returned with
   * error indication in dnx_sand_module_open().
   */
  DNX_SAND_MODULE_END_ALL_ERR_01,
  /*
   * Procedure dnx_sand_module_init_all() returned with
   * error indication in dnx_sand_module_open().
   */
  DNX_SAND_MODULE_INIT_ALL_ERR_01,
  /*
   * Procedure dnx_sand_mem_interrupt_mask_address_clear_all()
   * returned with error indication in dnx_sand_module_open().
   */
  DNX_SAND_INTERRUPT_CLEAR_ALL_ERR_01,
  /*
   * dnx_sand_init_chip_descriptors failed.
   */
  DNX_SAND_INIT_CHIP_DESCRIPTORS_ERR_01,
  /*
   * dnx_sand_handles_init_handles failed.
   */
  DNX_SAND_HANDLES_INIT_ERR_01,
  /*
   * dnx_sand_tcm_callback_engine_start failed.
   */
  DNX_SAND_TCM_CALLBACK_ENGINE_START_ERR_01,
  /*
   * dnx_sand_indirect_clear_info_all failed.
   */
  DNX_SAND_INDIRECT_SET_INFO_ERR_01,
  /*
   * Tried to access not exist module in the indirect zone.
   */
  DNX_SAND_INDIRECT_MODULE_NOT_EXIST,
  /*
   * Procedure dnx_sand_register_device() reports:
   * Maximal number of devices has already been
   * registered. Can not register more.
   */
  DNX_SAND_REGISTER_DEVICE_001,
  /*
   * Deprecated Error - do not use.
   */
  DNX_SAND_DEPRECATED_ERROR_00001,
  /*
   * Procedure dnx_sand_register_device() reports:
   * No live device has been found trying to access
   * specified memory.
   */
  DNX_SAND_REGISTER_DEVICE_003,
  /*
   * Procedure dnx_sand_register_device() reports:
   * Input parameter 'base_address' is not
   * long-aligned.
   */
  DNX_SAND_REGISTER_DEVICE_004,
  /*
   * Procedure dnx_sand_register_device() reports:
   * Chip type as reported by chip itself does not match
   * registered chip type.
   */
  DNX_SAND_REGISTER_DEVICE_005,
  /*
   * Procedure dnx_sand_register_device() reports:
   * User supplied buffer is NULL.
   */
  DNX_SAND_REGISTER_DEVICE_NULL_BUFFER,
  /*
   * Procedure dnx_sand_register_device() reports:
   * Descriptors array / mutex are not initialized.
   */
  DNX_SAND_DESC_ARRAY_NOT_INIT,
  /*
   * Procedure dnx_sand_register_device() reports:
   * Unknown error return value from dnx_sand_add_chip_descriptor().
   */
  DNX_SAND_ADD_CHIP_DESCRIPTOR_ERR,
  /*
   * Failure trying to create semaphore.
   */
  DNX_SAND_SEM_CREATE_FAIL,
  /*
   * Failure trying to delete semaphore.
   */
  DNX_SAND_SEM_DELETE_FAIL,
  /*
   * Failure trying to take semaphore.
   */
  DNX_SAND_SEM_TAKE_FAIL,
  /*
   * When calling RevB+ function while running over RevA chip
   */
  DNX_SAND_CHIP_VER_IS_A,
  /*
   * When calling RevA only function while running over RevB+ chip
   */
  DNX_SAND_CHIP_VER_IS_NOT_A,
  /*
   * Failure trying to take tcm callback delta list semaphore.
   */
  DNX_SAND_TCM_CALLBACK_DELTA_LIST_SEM_TAKE_FAIL,
  /*
   * Failure trying to take chip descriptors semaphore.
   */
  DNX_SAND_CHIP_DESCRIPTORS_SEM_TAKE_ERR_0,
  /*
   * Failure trying to take semaphore.
   */
  DNX_SAND_SEM_GIVE_FAIL,
  /*
   * Semaphore take has succeeded but it is not the one
   * which was intended, initially (It was deleted and
   * replaced).
   */
  DNX_SAND_SEM_CHANGED_ON_THE_FLY,
  /*
   * This error relates to indirect access only.
   * Timeout waiting for chip to deassert the 'trigger'
   * bit.
   */
  DNX_SAND_IND_TRIGGER_TIMEOUT,
  /*
   * Procedure dnx_sand_user_callback_unregister_device() or
   * dnx_sand_clear_all_device_peding_services() reports:
   * Failed to delete some of the device's active handles
   */
  DNX_SAND_UNREGISTER_DEVICE_001,
  /*
   * Procedure dnx_sand_user_callback_unregister_device() reports:
   * Failed to clear device descriptor
   */
  DNX_SAND_UNREGISTER_DEVICE_002,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * Input parameter 'offset' is not long-aligned.
   */
   DNX_SAND_MEM_READ_001,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * Input parameter 'size' is not a multiple of
   * four.
   */
   DNX_SAND_MEM_READ_002,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * This error relates to direct access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not readable
   * by this chip.
   */
   DNX_SAND_MEM_READ_003,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * This error relates to indirect access only.
   * Specified module (within 'offset') is illegal.
   */
   DNX_SAND_MEM_READ_004,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * This error relates to indirect access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not readable
   * by this chip.
   */
   DNX_SAND_MEM_READ_005,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * Failed to read from the chip
   */
   DNX_SAND_MEM_READ_006,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * Failed to write to chip (relevant only to indirect)
   */
   DNX_SAND_MEM_READ_007,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * Input parameter 'offset' is not long-aligned.
   */
   DNX_SAND_MEM_WRITE_001,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * Input parameter 'size' is not a multiple of
   * four.
   */
   DNX_SAND_MEM_WRITE_002,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * This error relates to direct access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not writable
   * by this chip.
   */
   DNX_SAND_MEM_WRITE_003,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * This error relates to indirect access only.
   * Specified module (within 'offset') is illegal.
   */
   DNX_SAND_MEM_WRITE_004,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * This error relates to indirect access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not writable
   * by this chip.
   */
   DNX_SAND_MEM_WRITE_005,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * Failed to write to the chip
   */
   DNX_SAND_MEM_WRITE_006,
   /*
    * Timeout on waiting for 'trigger'to be deasserted.
    */
   DNX_SAND_TRG_TIMEOUT,
  /*
   * Called to callback function that do
   * not use interrupt with interrupt request.
   */
  DNX_SAND_CALLBACK_FUNCTION_CANT_BE_INT,
  /*
   * Asked function can not be registered as callback.
   */
  DNX_SAND_NOT_CALLBACK_FUNCTION,
  /*
   * Unrecoverable error encountered in TCM.
   */
  DNX_SAND_TCM_MAIN_ERR_01,
  /*
   * Removed a deferred action due to an error.
   */
  DNX_SAND_TCM_MAIN_ERR_02,
  /*
   * Removed a deferred action due to user registration request.
   */
  DNX_SAND_TCM_MAIN_ERR_03,
  /*
   * Procedure dnx_sand_indirect_check_request_legal() reports:
   * Trying to access non-existing indirect module.
   */
  DNX_SAND_NO_SUCH_INDIRECT_MODULE_ERR,
  /*
   * Procedure dnx_sand_indirect_check_request_legal() reports:
   * Trying to access non-existing offset/size in the indirect module.
   */
  DNX_SAND_INDIRECT_OFFSET_SIZE_MISMATCH_ERR,
  /*
   * Procedure dnx_sand_indirect_check_request_legal() reports:
   * Example to a possible error:
   *   Say specific module answer size is 4 32 bit register (128 bit, 16 byte, 4 longs)
   *   (PEC in DNX_SAND_FAP10M). User gave 5 longs as buffer size.
   *   ((5/4) * 4) ==> 4 longs.
   *   4!=5 ... the result is different than we need. We find a mismatch is sizes.
   *
   */
  DNX_SAND_INDIRECT_SIZE_MISMATCH_ERR,
  /*
   * User gave not valid value in DNX_SAND_CREDIT_SIZE.
   * Please refer to DNX_SAND_CREDIT_SIZE definition.
   */
  DNX_SAND_CREDIT_SIZE_ENUM_ERR,
  /*
   * Functions 'dnx_sand_bitstream_get_field()'/'dnx_sand_bitstream_set_field()' reports:
   * 'nof_bits' is bigger than 32 (DNX_SAND_BIT_STREAM_FIELD_SET_SIZE).
   * Note, these functions get/set at most 32 bit fields.
   */
  DNX_SAND_BIT_STREAM_FIELD_SET_SIZE_RANGE_ERR,
  /*
   * Function 'XXXX_register_device()' reports:
   * User try to register device in DNX_SAND. DNX_SAND was configured to
   * user real interrupts (via 'dnx_sand_module_open()'). This driver
   * do not supports interrupts. Only mock-up interrupts
   * ('soc_tcmmockup_interrupts' indicator in 'dnx_sand_module_open()).
   */
  DNX_SAND_DO_NOT_SUPPORTS_INTERRUPTS_ERR,

  /*
   * Divide by Zero error
   */
  DNX_SAND_DIV_BY_ZERO_ERR,

  /*
   * Calculation overflow error
   */
  DNX_SAND_OVERFLOW_ERR,

  /*
   * Different errors use the same error id
   */
  DNX_SAND_ERRORS_FOUND_DUPLICATES_ERR,

  /*
  * Different procs use the same error id
  */
  DNX_SAND_PROCS_FOUND_DUPLICATES_ERR,

  DNX_SAND_ERR_8001, /*  taken */
  DNX_SAND_ERR_8002, /*  taken */
  DNX_SAND_ERR_8003, /*  taken */
  DNX_SAND_ERR_8004, /*  taken */
  DNX_SAND_ERR_8005, /*  taken */
  DNX_SAND_ERR_8006, /*  taken */
  DNX_SAND_ERR_8007, /*  taken */
  DNX_SAND_ERR_8008, /*  taken */
  DNX_SAND_ERR_8009, /*  taken */
  DNX_SAND_ERR_8010, /*  taken */
  DNX_SAND_ERR_8011, /*  taken */
  DNX_SAND_ERR_8012, /*  taken */
  DNX_SAND_ERR_8013, /*  taken */
  DNX_SAND_ERR_8014, /*  taken */
  DNX_SAND_ERR_8015, /*  taken */
  DNX_SAND_ERR_8016, /*  taken */

  /*
   * Internal error!!
   * The indirect information missing offset information.
   */
  DNX_SAND_INDIRECT_CANT_GET_INFO_ERR,

  /*
   * If a tables_prefix A is a subset of tables_prefix B,
   * The structure that contain tables_prefix A, must
   * come before the structure that contains tables_prefix B.
   */
  DNX_SAND_INDIRECT_TABLES_INFO_ORDER_ERR,

  /*
   * Failure trying to free memory.
   */
  DNX_SAND_FREE_FAIL,
  /*
   * Hamming-Code SW module received not correlated data-bit-wide,
   * and p-bit-wide. The correlated number can be found from
   * 'dnx_sand_code_hamming_get_p_bit_wide()' func call.
   */
  DNX_SAND_CODE_HAMMING_P_BIT_WIDE_UN_MATCH_ERR,
  /*
   * Hamming-Code SW module received un-supported data-bit-wide.
   * The maximum-p-bit-size we support is 32. Hence,
   * this problem is practically will no happen.
   */
  DNX_SAND_CODE_HAMMING_UN_SUPPORTED_DATA_BIT_WIDE_ERR,

  /*
   * Procedure dnx_sand_mem_read_unsafe() reports:
   * Input parameter 'offset' is not long-aligned.
   */
   DNX_SAND_MEM_READ_US_001,
  /*
   * Procedure dnx_sand_mem_read_unsafe() reports:
   * Input parameter 'size' is not a multiple of
   * four.
   */
   DNX_SAND_MEM_READ_US_002,
  /*
   * Procedure dnx_sand_mem_read_unsafe() reports:
   * This error relates to direct access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not readable
   * by this chip.
   */
   DNX_SAND_MEM_READ_US_003,
  /*
   * Procedure dnx_sand_mem_read_unsafe() reports:
   * This error relates to indirect access only.
   * dnx_sand_indirect_read_from_chip failed.
   */
   DNX_SAND_MEM_READ_US_005,
  /*
   * Procedure dnx_sand_mem_read_unsafe() reports:
   * Failed to read from the chip
   */
   DNX_SAND_MEM_READ_US_006,
  /*
   * Procedure dnx_sand_mem_read() reports:
   * error occurred in the dnx_sand_mem_read_unsafe() function.
   */
   DNX_SAND_MEM_READ_010,
  /*
   * Procedure dnx_sand_mem_write_unsafe() reports:
   * Input parameter 'offset' is not long-aligned.
   */
   DNX_SAND_MEM_WRITE_US_001,
  /*
   * Procedure dnx_sand_mem_write_unsafe() reports:
   * This error relates to direct access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not writable
   * by this chip.
   */
   DNX_SAND_MEM_WRITE_US_003,
  /*
   * Procedure dnx_sand_mem_write() reports:
   * This error relates to indirect access only.
   * Specified memory range ('offset', 'size')
   * contains some addresses which are not writable
   * by this chip.
   */
   DNX_SAND_MEM_WRITE_US_005,
  /*
   * Procedure dnx_sand_mem_write_unsafe() reports:
   * Failed to write to the chip
   */
   DNX_SAND_MEM_WRITE_US_006,
   /*
    * Procedure dnx_sand_mem_write() reports:
    * error occurred in the dnx_sand_mem_write_unsafe() function.
    */
    DNX_SAND_MEM_WRITE_010,
   /*
    * Failed the 'legal' function.
    */
    DNX_SAND_FAIL_REQUEST_LEGAL,
   /*
    * Failed the verify_0 function.
    */
    DNX_SAND_FAIL_VERIFY_0,
   /*
    * Failed the write_address function.
    */
    DNX_SAND_FAIL_WRITE_ADDRESS,
   /*
    * Failed the write_value function.
    */
    DNX_SAND_FAIL_WRITE_VALUE,
   /*
    * Failed the assert_1 function.
    */
    DNX_SAND_FAIL_ASSERT_1,
   /*
    * Failed the read_result Function.
    */
   DNX_SAND_FAIL_READ_RESULT,


  /*
   * Functions 'dnx_sand_set_field() reports:
   * 'ms_bit - ls_bit + 1' is bigger than 32 .
   * Note, these functions set at most 32 bit fields.
   */
  DNX_SAND_BIT_FIELD_SET_SIZE_RANGE_ERR,

  /*
   * Out of range errors
   */
  DNX_SAND_VALUE_OUT_OF_RANGE_ERR,
  DNX_SAND_VALUE_BELOW_MIN_ERR,
  DNX_SAND_VALUE_ABOVE_MAX_ERR,

  /*
   * When a device driver supports the magic number capability,
   *  the user has to call a _clear function, before using
   *  a structure to set information to the device driver.
   */
  DNX_SAND_MAGIC_NUM_ERR,
  /*
   * Illegal IP decimal format. Must be 16 bytes long at least
   * 4 decimal numbers separated by dots
   */
  DNX_SAND_ILLEGAL_IP_FORMAT,

  /*
   * An error in a SOC function occured, with a SOC error code.
   */
  DNX_SAND_SOC_ERR,



 /*
  * Function must not be called when 4k feature is enabled
  */
  DNX_SAND_ILLEGAL_FUNCTION_CALL_WHEN_4K_DISABLED,

  /*
  * Function must not be called when 4k feature is disabled
  */
  DNX_SAND_ILLEGAL_FUNCTION_CALL_WHEN_4K_ENABLED,
 /*
  * Hash table
  */
 /*
  * the given size of the hash table to allocate is out of range.
  * Range is 1-, preferred power of 2
  */
  DNX_SAND_HASH_TABLE_SIZE_OUT_OF_RANGE_ERR,
 /*
  * the hash table is full and trying to add to the hash table,
  * the size of the hash table as allocated in the create.
  * and this size may not change.
  */
  DNX_SAND_HASH_TABLE_IS_FULL_ERR,
 /*
  * trying to add to the hash table key that already present in
  * the hash table. key can be present at most once in the hash table
  */
  DNX_SAND_HASH_TABLE_KEY_ALREADY_EXIST_ERR,
 /*
  * trying to update a key that is not exist in the hash table
  * should use Add.
  */
  DNX_SAND_HASH_TABLE_KEY_IS_NOT_EXIST_ERR,

 /*
  * Group Member Linked List
  */
 /*
  * the number of elements (members) is out of range.
  * Range is 1-
  */
  DNX_SAND_GROUP_MEM_LL_NOF_MEMBERS_OUT_OF_RANGE_ERR,
 /*
  * the number of groups is out of range.
  * Range is 1-
  */
  DNX_SAND_GROUP_MEM_LL_NOF_GROUPS_OUT_OF_RANGE_ERR,
 /*
  * the member (element) id is out of range.
  * Range according to nof_elements given in create
  */
  DNX_SAND_GROUP_MEM_LL_MEMBER_ID_OUT_OF_RANGE_ERR,
 /*
  * the group id is out of range.
  * Range according to nof_groups given in create
  */
  DNX_SAND_GROUP_MEM_LL_GROUP_ID_OUT_OF_RANGE_ERR,
 /*
  * try to add element as member to group when it's already
  * a member in a group and auto_remove is FALSE
  */
  DNX_SAND_GROUP_MEM_LL_ILLEGAL_ADD_ERR,
/*
 * DNX_SAND memory management
 */
 /*
  * the number of lines (memory) is out of range.
  * Range is 1-
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_NOF_LINES_OUT_OF_RANGE_ERR,
 /*
  * malloc size is out of range.
  * malloc can be in size of 2-size of create memory
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_MALLOC_SIZE_OUT_OF_RANGE_ERR,
 /*
  * free pointer is out of range.
  * free pointer range is 0-size of create memory
  */
  DNX_SAND_ARR_MEM_ALLOCATOR_POINTER_OF_RANGE_ERR,


 /*
  * DNX_SAND PP IPV4
  */
 /*
  * the ipv4 prefix len is out of range
  * ipv4 prefix length range 0-32
  */
  DNX_SAND_PP_IPV4_SUBNET_PREF_OUT_OF_RANGE_ERR,

  /*
  * DNX_SAND PP IPV4
  */
  /*
  * the ipv6 address is not ipv6 multicast address (prefix is not 0xff)
  */
  DNX_SAND_PP_IPV6_ADDRESS_IS_NOT_MCAST_ERR,

 /*
  * DNX_SAND PP MAC
  */
 /*
  * the mac address string is not valid,
  * one of the string chars is out of hex values.
  * range of each char is 0-f in hexadecimal.
  */
  DNX_SAND_PP_MAC_ADDRESS_ILLEGAL_STRING_ERR,

 /*
  * DNX_SAND PP Out of range
  */
  DNX_SAND_PP_IPV4_ADDRESS_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_VLAN_ID_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_TC_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_DP_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_DEI_CFI_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_PCP_UP_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_ETHER_TYPE_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_TPID_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_IP_TTL_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_IPV4_TOS_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_IPV6_TC_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_MPLS_LABEL_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_ISID_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_MPLS_DSCP_EXP_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_DSCP_EXP_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_IN_RANGE_OUT_OF_RANGE_ERR,
  DNX_SAND_PP_DSCP_OUT_OF_RANGE_ERR,
  
 /*
  * SORTED list
  */
 /*
  * trying to add to the sorted list key that already present in
  * the sorted list. key can be present at most once in the sorted list
  */
  DNX_SAND_SORTED_LIST_KEY_DATA_ALREADY_EXIST_ERR,
 /*
  * trying to add/remove entry using illegal iterator position
  */
  DNX_SAND_SORTED_LIST_ILLEGAL_ITER_ERR,
  /* 
   * Cannot get error text for error id
   */
  DNX_SAND_GET_ERR_TXT_ERR,
  /*
   *
   */
  DNX_SAND_PRDS_START_ERR_NUMBER   = 200,
  DNX_SAND_PRDS_INT_WITHIN_INT,
  /*
   *
   */
#ifdef DNX_SAND_ERROR_CODE_OLD_MODE

  DNX_SAND_FE200_START_ERR_NUMBER = 2000,
  DNX_SAND_FE200_INT_WITHIN_INT,

  DNX_SAND_FAP10M_START_ERR_NUMBER = 4000,
  DNX_SAND_FAP10M_INT_WITHIN_INT,

  DNX_SAND_FAP20V_START_ERR_NUMBER = 12000,
  DNX_SAND_FAP20V_INT_WITHIN_INT,
#else  /*#ifdef DNX_SAND_ERROR_CODE_OLD_MODE*/
   DNX_SAND_FE200_START_ERR_NUMBER = 200,
   DNX_SAND_FE200_INT_WITHIN_INT,

   DNX_SAND_FAP10M_START_ERR_NUMBER = 450,
   DNX_SAND_FAP10M_INT_WITHIN_INT,

   DNX_SAND_TIMNA_START_ERR_NUMBER = 450,
   DNX_SAND_TIMNA_INT_WITHIN_INT,

   DNX_SAND_FAP20V_START_ERR_NUMBER = 550, /* 200 */
   DNX_SAND_FAP20V_INT_WITHIN_INT,

   DNX_SAND_PCP_START_ERR_NUMBER    = 750, /* 540 */
   DNX_SAND_PCP_INT_WITHIN_INT,
    
#endif /*#ifdef DNX_SAND_ERROR_CODE_OLD_MODE*/

  DNX_SAND_FAP21V_START_ERR_NUMBER    = 750, /* 2 */
  DNX_SAND_FAP21V_INT_WITHIN_INT,

  DNX_SAND_FAP20M_START_ERR_NUMBER    = 752, /* 2 */
  DNX_SAND_FAP20M_INT_WITHIN_INT,

  DNX_SAND_JER2_ARAD_START_ERR_NUMBER     = 754, /* 950 - some place */
  DNX_SAND_JER2_ARAD_INT_WITHIN_INT,

  DNX_SAND_PETRA_START_ERR_NUMBER     = 1704, /* 600 */
  DNX_SAND_PETRA_INT_WITHIN_INT,

  DNX_SAND_PETRA_PP_START_ERR_NUMBER  = 2304, /* 2 */
  DNX_SAND_PETRA_PP_INT_WITHIN_INT,

  DNX_SAND_FE600_START_ERR_NUMBER     = 2306, /* 2 */
  DNX_SAND_FE600_INT_WITHIN_INT,

  DNX_SAND_T20E_START_ERR_NUMBER      = 2308, /* 2 */
  DNX_SAND_T20E_INT_WITHIN_INT,

  DNX_SAND_PB_START_ERR_NUMBER        = 2310, /* 500 */
  DNX_SAND_PB_INT_WITHIN_INT,

  DNX_SAND_PB_PP_START_ERR_NUMBER     = 2810, /* 1170 */
  DNX_SAND_PB_PP_INT_WITHIN_INT,

  DNX_SAND_PPD_START_ERR_NUMBER       = 3980, /* 10 */
  DNX_SAND_PPD_INT_WITHIN_INT,

  DNX_SAND_OAM_START_ERR_NUMBER       = 3990, /* 50 */
  DNX_SAND_OAM_INT_WITHIN_INT,

  DNX_SAND_TMD_START_ERR_NUMBER       = 4040, /* 10 */
  DNX_SAND_TMD_INT_WITHIN_INT,

  DNX_SAND_TMC_START_ERR_NUMBER       = 4050, /* 10 */
  DNX_SAND_TMC_INT_WITHIN_INT,

	DNX_SAND_JER2_ARAD_PP_START_ERR_NUMBER     = 4060, /* 1170 */
	DNX_SAND_JER2_ARAD_PP_INT_WITHIN_INT,


  /*
   * Indication of end of list of errors. Do not remove.
   */
  DNX_SAND_END_ERR_LIST = -1
} DNX_SAND_ERROR_CODE ;
/*
 * {
 * Error word construction utilities
 */
void
  dnx_sand_initialize_error_word(
    uint32   proc_id,
    unsigned char   callback,
    uint32   *error_word) ;
void
  dnx_sand_set_proc_id_into_error_word(uint32   proc_id,
    uint32   *error_word) ;
/*****************************************************
*NAME
*  dnx_sand_get_proc_id_from_error_word
*TYPE:
*  PROC
*DATE:
*  25/SEP/2002
*FUNCTION:
*  Extract procedure id from the standard output error
*  word.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 error_word -
*      Error code constructed as described under
*      ERROR RETURN VALUES (in dnx_sand_error_code.h).
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      Procedure id.
*      See formatting rules in ERROR RETURN VALUES in
*      dnx_sand_error_code.h.
*  DNX_SAND_INDIRECT:
*    formatting rules of ERROR RETURN VALUES
*REMARKS:
*  None
*SEE ALSO:
*****************************************************/
uint32
    dnx_sand_get_proc_id_from_error_word(
      uint32   error_word
    ) ;
void
  dnx_sand_set_error_code_into_error_word(
    unsigned short  error_code,
    uint32   *error_word) ;

uint32
  dnx_sand_update_error_code(
    uint32   fun_error_word,
    uint32   *error_word
  );

/*****************************************************
*NAME
*  dnx_sand_get_error_code_from_error_word
*TYPE:
*  PROC
*DATE:
*  25/SEP/2002
*FUNCTION:
*  Extract error code from the standard output error
*  word.
*INPUT:
*  DNX_SAND_DIRECT:
*    uint32 error_word -
*      Error code constructed as described under
*      ERROR RETURN VALUES (in dnx_sand_error_code.h).
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    unsigned short -
*      error code.
*      See formatting rules in ERROR RETURN VALUES in
*      dnx_sand_error_code.h.
*  DNX_SAND_INDIRECT:
*    formatting rules of ERROR RETURN VALUES
*REMARKS:
*  None
*SEE ALSO:
*****************************************************/
unsigned
  short
    dnx_sand_get_error_code_from_error_word(
      uint32 error_word) ;
void
  dnx_sand_set_severity_into_error_word(
    unsigned char severity,
    uint32 *error_word) ;
unsigned
  char
    dnx_sand_get_severity_from_error_word(
      uint32 error_word) ;
void
  dnx_sand_set_callback_into_error_word(
    unsigned char callback,
    uint32 *error_word) ;
unsigned
  char dnx_sand_get_callback_from_error_word(
    uint32 error_word) ;
uint32
    dnx_sand_build_error_code(
      unsigned short error_code,
      uint32  proc_id,
      unsigned char  severity,
      unsigned char  is_call_back) ;
int
  dnx_sand_error_code_to_string(
    DNX_SAND_IN  uint32 in_error_code,
    DNX_SAND_OUT char    **out_error_name,
    DNX_SAND_OUT char    **out_error_description
  ) ;
int
  dnx_sand_linear_find_error(
    uint32      error_code,
    DNX_SAND_IN DNX_ERROR_DESC_ELEMENT *in_error_desc_element,
    CONST DNX_ERROR_DESC_ELEMENT **error_desc_element_ptr
  ) ;
/*
 * }
 * Error word construction utilities
 */
/*
 * Handling of all-system errors list plus descriptors.
 * {
 */
int
  dnx_sand_add_error_pool(
    DNX_SAND_IN DNX_ERROR_DESC_ELEMENT  *error_pool
  ) ;
int
  dnx_sand_add_error_pools(
    DNX_SAND_IN DNX_ERROR_DESC_ELEMENT  **error_id_pools,
    uint32 nof_pools
  ) ;
int
  dnx_sand_close_all_error_pool(
    void
  ) ;
int
  dnx_sand_add_sand_errors(
    void
  ) ;
CONST DNX_ERROR_DESC_ELEMENT
  *dnx_sand_get_errors_ptr(
    void
  ) ;
/*
 * }
 */
/*
 * Handling of all-system procedures list plus descriptors.
 * {
 */
int
  dnx_sand_add_proc_id_pool(
    DNX_SAND_IN DNX_PROCEDURE_DESC_ELEMENT  *proc_id_pool
  ) ;
int
  dnx_sand_add_proc_id_pools(
    DNX_SAND_IN DNX_PROCEDURE_DESC_ELEMENT  **proc_id_pool,
    uint32 nof_pools
  ) ;
int
  dnx_sand_add_sand_procedures(
    void
  ) ;
int
  dnx_sand_close_all_proc_id_pool(
    void
  ) ;
int
  dnx_sand_proc_id_to_string(
    DNX_SAND_IN  uint32 in_proc_id,
    DNX_SAND_OUT char          **out_module_name,
    DNX_SAND_OUT char    **out_proc_name
  ) ;
CONST DNX_PROCEDURE_DESC_ELEMENT
  *dnx_sand_get_procedures_ptr(
    void
  ) ;
int
  dnx_sand_linear_find_procedure(
    uint32          proc_id,
    DNX_SAND_IN DNX_PROCEDURE_DESC_ELEMENT *in_procedure_desc_element,
    CONST DNX_PROCEDURE_DESC_ELEMENT **procedure_desc_element_ptr
  ) ;
/*
 * }
 */
DNX_SAND_RET
  dnx_sand_init_errors_queue(
    void
  ) ;
DNX_SAND_RET
  dnx_sand_delete_errors_queue(
    void
  ) ;
DNX_SAND_RET
  dnx_sand_load_errors_queue(
    DNX_SAND_ERRORS_QUEUE_MESSAGE *errors_queue_message
  ) ;
DNX_SAND_RET
  dnx_sand_unload_errors_queue(
    void
  ) ;
int
  dnx_sand_is_errors_queue_msg(
    uint32 msg
  ) ;
void
  dnx_sand_error_code_handler(
    DNX_SAND_IN DNX_SAND_RET error_code,
    DNX_SAND_IN char*    error_file,
    DNX_SAND_IN int      error_line,
    DNX_SAND_IN char*    error_func_name,
    DNX_SAND_IN char*    error_msg
  ) ;
/*
 * If ACTIVATE_DBG_ERROR_REPORT is asserted then built-in compiler-dependent
 * error report is activated at exit of DNX_SAND procedures. This is a debugging
 * tool and should be shut down on operational code.
 */
#define ACTIVATE_DBG_ERROR_REPORT 0

#if ACTIVATE_DBG_ERROR_REPORT
/* { */
#ifdef __GNUC__
/* { */
#define DNX_SAND_DBG_REPORT(dnx_sand_error_code, error_msg) \
          dnx_sand_error_code_handler(  \
            dnx_sand_error_code,        \
            __FILE__,               \
            __LINE__,               \
            __PRETTY_FUNCTION__ ,   \
            error_msg               \
          )
/* } */
#else
/* { */
#define DNX_SAND_DBG_REPORT(dnx_sand_error_code, error_msg) \
          dnx_sand_error_code_handler(  \
            dnx_sand_error_code,        \
            __FILE__,               \
            __LINE__,               \
            NULL,                   \
            error_msg               \
          )
/* } */
#endif
/* } */
#else
/* { */
#define DNX_SAND_DBG_REPORT(dnx_sand_error_code, error_msg)
/* } */
#endif

#ifdef BROADCOM_DEBUG

/* If expr is false, then an error will be thrown and the error message will be the expression failed. */
/* This is a version of assert without exit. */
# define DNX_SAND_VERIFY(expr) { if (!(expr)) { DNX_SAND_SET_ERROR_MSG((_ERR_MSG_MODULE_NAME, unit, "ASSERTION FAILED (%s:%d): %s\n", __FILE__, __LINE__, #expr)); } }
#else
# define DNX_SAND_VERIFY(expr) ((void)0)
#endif /* BROADCOM_DEBUG */

/*
 * Macro to activate at exit of all DNX_SAND procedures.
 * Note that each procedure reports itself but may report an error
 * brought in from other (invoked) procedure.
 * A few procedures may report the same error (but each with
 * its procedure id).
 */
#define DNX_SAND_ERROR_REPORT(error_code,error_msg,callback,svr,proc_id,err_text,p1,p2,p3,p4,p5,p6) \
  if (error_code != DNX_SAND_OK)  \
  {               \
    uint32      \
        err_id ;  \
    dnx_sand_initialize_error_word(proc_id,callback,&err_id) ;     \
    dnx_sand_set_error_code_into_error_word(error_code,&err_id) ;  \
    dnx_sand_set_severity_into_error_word(svr,&err_id) ;           \
    dnx_sand_error_handler(err_id,err_text,p1,p2,p3,p4,p5,p6) ;    \
    DNX_SAND_DBG_REPORT(error_code,error_msg) ;               \
  }

/*****************************************************
 * See details in dnx_sand_error_code.c
 *****************************************************/
DNX_SAND_RET
  dnx_sand_check_CREDIT_WORTH(
    DNX_SAND_IN  DNX_SAND_CREDIT_SIZE credit_worth
  );



#if DNX_SAND_DEBUG
/* { */
/*
 */

/*****************************************************
 * See details in dnx_sand_error_code.c
 *****************************************************/
int
  dnx_sand_disp_result(
    DNX_SAND_IN uint32 driver_api_result
  );
int
  dnx_sand_disp_result_proc(
    DNX_SAND_IN uint32 driver_api_result,
    DNX_SAND_IN char              *proc_name
  );

/*****************************************************
 * See details in dnx_sand_error_code.c
 *****************************************************/
void
  dnx_sand_no_error_printing_set(
    uint8   no_error_printing
  );

uint8
  dnx_sand_no_error_printing_get(
       void
    );


/*
 * }
 */
#endif


#ifdef  __cplusplus
}
#endif
/* } __DNX_SAND_ERROR_CODE_H_INCLUDED__*/
#endif
