
/**
 *\file dbal_structures.h 
 * Main typedefs and enum of dbal. 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef DBAL_STRUCTURES_H_INCLUDED
#  define DBAL_STRUCTURES_H_INCLUDED

#  ifndef BCM_DNX_SUPPORT
#    error "This file is for use by DNX (JR2) family only!"
#  endif

/**************
 *  INCLUDES  *
 **************/
#  include <soc/mem.h>
#  include <soc/drv.h>
#  include <soc/dnx/dbal/dbal_defines_tables.h>
#  include <soc/dnx/dbal/dbal_defines_fields.h>
#  include <soc/dnx/dbal/dbal_defines_hw_entities.h>
#  include <shared/shrextend/shrextend_debug.h>
#  include <shared/swstate/sw_state.h>
#  include <shared/utilex/utilex_hashtable.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dnx/swstate/access/mdb_kaps_access.h>
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

/*************
 *  DEFINES  *
 *************/

/**
 *  \brief used for fields instances APIs when there is no multiple instances
 *         for the field in a specifiec table
 */
#  define INST_SINGLE                         -1

/**
 *  \brief used for fields instances APIs when all the instances of the field in current table are relevant
 */
#  define INST_ALL                           0xffff

/**
 *  \brief the size of field with type array in bytes 
 */
#  define DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES        64

/**
 *  \brief the size of field with type array in words 
 */
#  define DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS        (DBAL_FIELD_ARRAY_MAX_SIZE_IN_BYTES/sizeof(uint32))

/**
 *  \brief number of result fields, table cannot be defined with
 *         more than this size
 */
#define DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS       70

/**
 *  \brief per table the maximum different result types 
 */

#define DBAL_MAX_NUMBER_OF_RESULT_TYPES            16

/**
 *  \brief per table the minimum different result types
 */
#define DBAL_MIN_NUMBER_OF_RESULT_TYPES            1

/**
 *  \brief max dbal labels per entity
 */
#define DBAL_MAX_NOF_ENTITY_LABEL_TYPES             5

/**
 *  \brief number of key fields, table cannot be defined with
 *         more than this size
 */
#  define DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS          15

/**
 *  \brief number of enum values for enum field 
 */
#  define DBAL_FIELD_MAX_NUM_OF_ENUM_VALUES         31

/**
 *  \brief DBAL string size limitation 
 */
#  define DBAL_MAX_STRING_LENGTH                    128

/**
 *  \brief max number of registers for array of registers in
 *         H.L. direct
 */
#  define DBAL_MAX_NUMBER_OF_REGISTERS              10

/**
 *  \brief number of entries handle to preform table action
 *         with
 */
#  define DBAL_SW_NOF_ENTRY_HANDLES                 10

/**
 *  \brief max number of conditions to preform H.L access 
 */
#  define DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS  5

/**
 *  \brief number of sub-fields (destination --> fec, MC_ID...)
 *         
 */
#  define DBAL_FIELD_MAX_CHILDS_PER_FIELD           5

/**
 *  \brief max number of parent fields 
 */
#  define DBAL_FIELD_MAX_PARENTS_PER_FIELD          3

/**
 *  \brief the size in words of the key buffer in the entry
 *         handle
 */
#  define DBAL_PHYSICAL_KEY_SIZE_IN_WORDS  (20)

/**
 *  \brief the size in words of the result buffer in the entry handle 
 */
#  define DBAL_PHYSICAL_RES_SIZE_IN_WORDS  (20)

/**
 *  \brief max key size for sw directtables in bits
 */
#  define DBAL_SW_DIRECT_TABLES_MAX_KEY_SIZE_BITS  (30)

/**
 *  \brief max key size for sw hash tables in bytes
 */
#  define DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES   (8)

/**
 *  \brief max key size for sw hash tables in words
 */
#  define DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_WORDS   (DBAL_SW_HASH_TABLES_MAX_KEY_SIZE_BYTES/sizeof(uint32))

/**
 *  \brief when working with multiple result types indicate that
 *         result type was not set for the handle
 */
#  define DBAL_RESULT_TYPE_NOT_INITIALIZED    -1

/**************
 *  TYPEDEFS  *
 **************/
/**
 *  \brief List of dbal logger types 
 */
typedef enum dnx_dbal_logger_type_t
{
    DNX_DBAL_LOGGER_TYPE_API,
    DNX_DBAL_LOGGER_TYPE_ACCESS,
    DNX_DBAL_LOGGER_TYPE_LAST,
} dnx_dbal_logger_type_e;

/**
 *  \brief List of physical tables according to the XML definitions from the MDB 
 */
typedef enum
{
  /** XML indication: not used in XML */
    DBAL_PHYSICAL_TABLE_NONE,
  /** XML indication: TCAM */
    DBAL_PHYSICAL_TABLE_TCAM,
  /** XML indication: LPM_PRIVATE */
    DBAL_PHYSICAL_TABLE_LPM_PRIVATE,
  /** XML indication: LPM_PUBLIC */
    DBAL_PHYSICAL_TABLE_LPM_PUBLIC,
  /** XML indication: ISEM_1 */
    DBAL_PHYSICAL_TABLE_ISEM_1,
  /** XML indication: INLIF_1 */
    DBAL_PHYSICAL_TABLE_INLIF_1,
  /** XML indication: IVSI */
    DBAL_PHYSICAL_TABLE_IVSI,
  /** XML indication: ISEM_2 */
    DBAL_PHYSICAL_TABLE_ISEM_2,
  /** XML indication: INLIF_2 */
    DBAL_PHYSICAL_TABLE_INLIF_2,
  /** XML indication: ISEM_3 */
    DBAL_PHYSICAL_TABLE_ISEM_3,
  /** XML indication: INLIF_3 */
    DBAL_PHYSICAL_TABLE_INLIF_3,
  /** XML indication: LEM */
    DBAL_PHYSICAL_TABLE_LEM,
  /** XML indication: IOEM_0 */
    DBAL_PHYSICAL_TABLE_IOEM_0,
  /** XML indication: IOEM_1 */
    DBAL_PHYSICAL_TABLE_IOEM_1,
  /** XML indication: MAP */
    DBAL_PHYSICAL_TABLE_MAP,
  /** XML indication: FEC_1 */
    DBAL_PHYSICAL_TABLE_FEC_1,
  /** XML indication: FEC_2 */
    DBAL_PHYSICAL_TABLE_FEC_2,
  /** XML indication: FEC_3 */
    DBAL_PHYSICAL_TABLE_FEC_3,
  /** XML indication: MC_ID */
    DBAL_PHYSICAL_TABLE_MC_ID,
  /** XML indication: GLEM_0 */
    DBAL_PHYSICAL_TABLE_GLEM_0,
  /** XML indication: GLEM_1 */
    DBAL_PHYSICAL_TABLE_GLEM_1,
  /** XML indication: EEDB_1 */
    DBAL_PHYSICAL_TABLE_EEDB_1,
  /** XML indication: EEDB_2 */
    DBAL_PHYSICAL_TABLE_EEDB_2,
  /** XML indication: EEDB_3 */
    DBAL_PHYSICAL_TABLE_EEDB_3,
  /** XML indication: EEDB_4 */
    DBAL_PHYSICAL_TABLE_EEDB_4,
  /** XML indication: EOEM_0 */
    DBAL_PHYSICAL_TABLE_EOEM_0,
  /** XML indication: EOEM_1 */
    DBAL_PHYSICAL_TABLE_EOEM_1,
  /** XML indication: ESEM */
    DBAL_PHYSICAL_TABLE_ESEM,
  /** XML indication: EVSI */
    DBAL_PHYSICAL_TABLE_EVSI,
  /** XML indication: EXEM_1 */
    DBAL_PHYSICAL_TABLE_EXEM_1,
  /** XML indication: EXEM_2 */
    DBAL_PHYSICAL_TABLE_EXEM_2,
  /** XML indication: EXEM_3 */
    DBAL_PHYSICAL_TABLE_EXEM_3,
  /** XML indication: EXEM_4 */
    DBAL_PHYSICAL_TABLE_EXEM_4,
  /** XML indication: RMEP */
    DBAL_PHYSICAL_TABLE_RMEP,

    DBAL_NOF_PHYSICAL_TABLES
} dbal_physical_tables_e;

/**
 *  \brief enum field types, field type used for print output\n this enum also used to indicate the field type for
 * getting field when adding values to this enum need also to add coresponding string to: dbal_field_type_strings 
 */
typedef enum
{
    /** No field type indication
     * XML indication: not used in XML */
    DBAL_FIELD_TYPE_NONE,
    /** prints TRUE/FALSE
     *  XML indication: BOOL */
    DBAL_FIELD_TYPE_BOOL,
    /** prints positive and negative values 32bit
     *  XML indication: INT32 */
    DBAL_FIELD_TYPE_INT32,
    /** field type is uint32, prints unsigned values 32bit
     *  XML indication: UINT32 */
    DBAL_FIELD_TYPE_UINT32,
    /** prints: x.x.x.x (32bit)
     *  XML indication: IP */
    DBAL_FIELD_TYPE_IP,
    /** field type is char array, print format 0xAA:0xBB:0xCC..
     *  XML indication: ARRAY8 */
    DBAL_FIELD_TYPE_ARRAY8,
    /** field type is uint32 array, print format 0xaabbccdd 0xaabbccdd
     *  XML indication: ARRAY32 */
    DBAL_FIELD_TYPE_ARRAY32,
    /** print format 11010110...
     *  XML indication: BITMAP */
    DBAL_FIELD_TYPE_BITMAP,
    /** print enum format prints string for each valid value
     *  XML indication: ENUM  */
    DBAL_FIELD_TYPE_ENUM,

    DBAL_NOF_FIELD_TYPES
} dbal_field_type_e;


typedef enum
{
  /** un-initialized pointer type */
    DBAL_POINTER_TYPE_NONE,

  /** pointer type uint32  */
    DBAL_POINTER_TYPE_UINT32,

  /** pointer type uint8  */
    DBAL_POINTER_TYPE_UINT8,

  /** pointer type int  */
    DBAL_POINTER_TYPE_INT,

    DBAL_NOF_POINTER_TYPES
} dbal_pointer_type_e;

/**
 *  \brief enum logical tables types\n this enum also used to indicate if sw table should be allocated as hash table
 * or direct 
 */
typedef enum
{
    /** No table type indication
     * XML indication: not used in XML */
    DBAL_TABLE_TYPE_NONE,
    /** Exact match logical table
     *  XML indication: EM */
    DBAL_TABLE_TYPE_EM,
    /** TCAM logical table
     *  XML indication: TCAM */
    DBAL_TABLE_TYPE_TCAM,
    /** LPM logical table
     *  XML indication: LPM */
    DBAL_TABLE_TYPE_LPM,
    /** direct access logical table,
     *  entry is accessed according to key
     *  XML indication: DIRECT */
    DBAL_TABLE_TYPE_DIRECT,

    DBAL_NOF_TABLE_TYPES
} dbal_table_type_e;

/**
 *  \brief enum that represents the DBAL workmode, currently unused 
 */
typedef enum
{
    DBAL_WORK_MODE_NORMAL = 0,

    DBAL_NUM_OF_WORK_MODES
} dbal_work_modes_e;

/**
 *  \brief enum represents the handle status 
 */
typedef enum
{
    DBAL_HANDLE_STATUS_AVAILABLE,
    DBAL_HANDLE_STATUS_IN_USE,
    DBAL_HANDLE_STATUS_ACTION_PREFORMED,

    DBAL_NOF_ENTRY_HANDLE_STATUSES
} dbal_entry_handle_status_e;

/**
 *  \brief enum that represents all the encoding types for field values \n when adding values to this enum need also
 * to add coresponding string to: dbal_field_encode_type_strings 
 */
typedef enum
{
    /** no encoding, value will stay the same
     * XML indication: NONE */
    DBAL_VALUE_FIELD_ENCODE_NONE,
    /** if val !=0 will return 1.
     *  XML indication: BOOL */
    DBAL_VALUE_FIELD_ENCODE_BOOL,
    /** value shifted with prefix
     *  XML indication: PREFIX */
    DBAL_VALUE_FIELD_ENCODE_PREFIX,
    /** add suffix tp value
     *  XML indication: SUFFIX */
    DBAL_VALUE_FIELD_ENCODE_SUFFIX,
    /** value-input_parm
     *  XML indication: SUBTRACT */
    DBAL_VALUE_FIELD_ENCODE_SUBTRACT,
    /** value=input_parm
     *  XML indication: HARD_VALUE */
    DBAL_VALUE_FIELD_ENCODE_HARD_VALUE,
    /** value=transform[enum_value]
     *  XML indication: not used in XML */
    DBAL_VALUE_FIELD_ENCODE_ENUM,

    DBAL_NOF_VALUE_FIELD_ENCODE_TYPES
} dbal_value_field_encode_types_e;

/**
 *  \brief enum that represents all the calculations (encoding) types for access offsets \n when adding values to
 * this enum need also to add coresponding string to: dbal_offset_encode_type_strings 
 */
typedef enum
{
    /** no encoding, value will stay the same
     * XML indication: NONE */
    DBAL_VALUE_OFFSET_ENCODE_NONE,
    /** if val !=0 will return 1.
     * XML indication: BOOL */
    DBAL_VALUE_OFFSET_ENCODE_BOOL,
    /** value%input_parm
     *  XML indication: MODULO */
    DBAL_VALUE_OFFSET_ENCODE_MODULO,
    /** value/input_parm
     *  XML indication: DIVIDE */
    DBAL_VALUE_OFFSET_ENCODE_DIVIDE,
    /** value*input_parm
     *  XML indication: MULTIPLE */
    DBAL_VALUE_OFFSET_ENCODE_MULTIPLE,
    /** value-input_parm
     *  XML indication: SUBTRACT */
    DBAL_VALUE_OFFSET_ENCODE_SUBTRACT,
    /** key uses only one of the key fields
     * XML indication: PARTIAL_KEY */
    DBAL_VALUE_OFFSET_ENCODE_PARTIAL_KEY,
    /** value=input_parm
     * XML indication: HARD_VALUE */
    DBAL_VALUE_OFFSET_ENCODE_HARD_VALUE,

    /** (key%input_param)*value_field_size - this is use when the same field is repeted multiple times in the memory.
     * XML indication: MODULO_FIELD */
    DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP,

    DBAL_NOF_VALUE_OFFSET_ENCODE_TYPES
} dbal_value_offset_encode_types_e;

/**
 *  \brief enum that represents hard logic direct access types
 */
typedef enum
{
    /** memory access */
    DBAL_HL_ACCESS_MEMORY,
    /** register access   */
    DBAL_HL_ACCESS_REGISTER,
    /** SW access for specific field in table */
    DBAL_HL_ACCESS_SW,
    /** pemla access */
    DBAL_HL_ACCESS_PEMLA,

    DBAL_NOF_HL_ACCESS_TYPES
} dbal_hard_logic_access_types_e;

/**
 *  \brief enum that represents all label types, label are SW
 *         identification for fields and tables
 */
typedef enum
{
    /** XML indication: Not used in XML */
    DBAL_LABEL_NONE,
    /** XML indication: SYSTEM */
    DBAL_LABEL_SYSTEM,
    /** XML indication: L2 */
    DBAL_LABEL_L2,
    /** XML indication: L3 */
    DBAL_LABEL_L3,
    /** XML indication: MPLS */
    DBAL_LABEL_MPLS,
    /** XML indication: FCOE */
    DBAL_LABEL_FCOE,
    /** XML indication: SNIF */
    DBAL_LABEL_SNIF,
    /** XML indication: ECGM */
    DBAL_LABEL_ECGM,
    /** XML indication: ICGM */
    DBAL_LABEL_ICGM,
    /** XML indication: SCHEDULER */
    DBAL_LABEL_SCHEDULER,
    /** XML indication: ITM */
    DBAL_LABEL_ITM,
    /** XML indication: ETM */
    DBAL_LABEL_ETM,
    /** XML indication: STACK */
    DBAL_LABEL_STACK,
    /** XML indication: LAG */
    DBAL_LABEL_LAG,
    /** XML indication: MULTICAST */
    DBAL_LABEL_MULTICAST,
    /** XML indication: FABRIC */
    DBAL_LABEL_FABRIC,
    /** XML indication: NIF */
    DBAL_LABEL_NIF,
    /** XML indication: FC */
    DBAL_LABEL_FC,
    /** XML indication: CRPS */
    DBAL_LABEL_CRPS,
    /** XML indication: MIRROR */
    DBAL_LABEL_MIRROR,
    /** XML indication: SNOOP */
    DBAL_LABEL_SNOOP,
    /** XML indication: OAM */
    DBAL_LABEL_OAM,
    /** XML indication: BFD */
    DBAL_LABEL_BFD,
    /** XML indication: PARSER */
    DBAL_LABEL_PARSER,
    /** XML indication: VSWITCH */
    DBAL_LABEL_VSWITCH,
    /** XML indication: PP_PORT */
    DBAL_LABEL_PP_PORT,
    /** XML indication: VLAN_TRANSLATION */
    DBAL_LABEL_VLAN_TRANSLATION,
    /** XML indication: VLAN */
    DBAL_LABEL_VLAN,
    /** XML indication: L2_LEARNING */
    DBAL_LABEL_L2_LEARNING,
    /** XML indication: VPLS */
    DBAL_LABEL_VPLS,
    /** XML indication: VPWS */
    DBAL_LABEL_VPWS,
    /** XML indication: EVPN */
    DBAL_LABEL_EVPN,
    /** XML indication: TRILL */
    DBAL_LABEL_TRILL,
    /** XML indication: L2GRE */
    DBAL_LABEL_L2GRE,
    /** XML indication: MIM */
    DBAL_LABEL_MIN,
    /** XML indication: VXLAN */
    DBAL_LABEL_VXLAN,
    /** XML indication: ROO */
    DBAL_LABEL_ROO,
   /** XML indication: HASHING */
    DBAL_LABEL_HASHING,
    /** XML indication: QOS_PHB */
    DBAL_LABEL_QOS_PHB,
    /** XML indication: QOS_REMARKING */
    DBAL_LABEL_QOS_REMARKING,
    /** XML indication: STG */
    DBAL_LABEL_STG,
    /** XML indication: L3_IF_RIF */
    DBAL_LABEL_L3_IF_RIF,

    DBAL_NOF_LABEL_TYPES
} dbal_labels_e;

/**
 *  \brief enum that represents the logical tables access method
 *         types
 */
typedef enum
{
    /** Physical table - Access mapping is known per table, but
     *  not per field. Spesific field(s) can be stored in SW
     *  state */
    DBAL_ACCESS_METHOD_PHY_TABLE,

    /** Hard logic - The Access mapping of the fields is known.
     *  can be either HW or SW state */
    DBAL_ACCESS_METHOD_HARD_LOGIC,

    /** SW Only - Tables which stored in SW state  */
    DBAL_ACCESS_METHOD_SW_ONLY,

    /** TTBD  */
    DBAL_ACCESS_METHOD_PEMLA,

    DBAL_NOF_ACCESS_METHODS
} dbal_access_method_e;

/**
 *  \brief enum that represents all the core modes 
 */
typedef enum
{
    /** XML indication: not used in XML */
    DBAL_CORE_NONE,
    /** dedicated per code
     *  XML indication: DPC */
    DBAL_CORE_BY_INPUT,
    /** Shared betweeen cores
     * XML indication: SBC */
    DBAL_CORE_ALL,

    DBAL_NOF_CORE_MODE_TYPES
} dbal_core_mode_e;

/**
 *  \brief this define is used in the entry handle and
 *         represents that the core field was not set.
 */
#  define DBAL_CORE_NOT_INTIATED   (-2)

/**
 *  \brief this define is used in the entry handle and
 *         represents that the core field to use is any core.
 */
#  define DBAL_CORE_ANY            (-1)

/**
 *  \brief this define is used to define the size of core fields
 *         in bits.
 */
#define DBAL_CORE_SIZE_IN_BITS      (1)

/**
 *  \brief enum that represents all the condition types \n when adding values to this enum need also to add
 * coresponding string to: dbal_condition_strings 
 */
typedef enum
{
    /** no condition, access will happen anyway
     *  XML indication: NONE */
    DBAL_CONDITION_NONE,
    /** access will happen only if key bigger than value
     *  XML indication: BIGGER_THAN */
    DBAL_CONDITION_BIGGER_THAN,
    /** access will happen only if key lower than value
     *  XML indication: LOWER_THAN */
    DBAL_CONDITION_LOWER_THAN,
    /** access will happen only if key equal to value
     *  XML indication: EQUAL_TO */
    DBAL_CONDITION_EQUAL_TO,
    /** access will happen only if key is even
     *  XML indication: IS_EVEN */
    DBAL_CONDITION_IS_EVEN,
    /** access will happen only if key is odd
     *  XML indication: IS_ODD */
    DBAL_CONDITION_IS_ODD,

    DBAL_NOF_CONDITION_TYPES
} dbal_condition_types_e;

/**
 *  \brief enum that represents the maturity level of a table
 *         according to regressions
 */
typedef enum
{
    /** Low
     *  Do not add thee table to the DB, table is invalid
     *  XML indication - 0 */
    DBAL_MATURITY_LOW,
    /** Medium
     *  Add the table to the Db so basic test can be tested
     *  table is not part of the regression
     *  XML indication - 1 */
    DBAL_MATURITY_MEDIUM,
    /** High
     *  Table is ready to be part of the regression
     *  XML indication - 2 */
    DBAL_MATURITY_HIGH,

    DBAL_NOF_MATURITY_LEVELS
} dbal_maturity_level_e;

typedef enum
{
    /**
     * will return all entries including "0" entries and default entries 
     * entries
     */
    DBAL_ITER_MODE_ALL,

    /**
     * will return all entries without entries that equals to default value if not defined default value "0" is used 
     */
    DBAL_ITER_MODE_GET_ALL_BUT_DEFAULT_ENTRIES,

    DBAL_NOF_ITER_TYPES
} dbal_iterator_mode_e;

/**
 *  \brief enum that represents all the commit types available in: \n dbal_entry_get() dbal_entry_commit()
 * dbal_entry_delete() \n multiple flags can be used by "or" operation \n 
 */
typedef enum
{
    /** release the handle after commiting, preform read modify
     *  write only field that was set will be updatd */
    DBAL_COMMIT_NORMAL = SAL_BIT(0),

    /** saves the handle after commiting  to future use, user
     *  should return the handle implictly when using this option
     *  handle should be initiated to DBAL_SW_NOF_ENTRY_HANDLES
     *  and the handle release should be after the exit label.  */
    DBAL_COMMIT_KEEP_HANDLE = SAL_BIT(1),

    /** 
     *  NOT preforming read before write - not implemented yet */
    DBAL_COMMIT_OVERRUN_ENTERY = SAL_BIT(2),

    /** saves the handle after getting all the fields user
     *  should return the handle implictly when using this option after calling this API user can
     *  retrive the field values by using the APIs dbal_entry_handle_value_ */
    DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE = SAL_BIT(3),

    /** will not preform action prints used mainly to internal
     *  DBAL actions */
    DBAL_COMMIT_DISABLE_ACTION_PRINTS = SAL_BIT(4),

    DBAL_COMMIT_NOF_OPCODES
} dbal_entry_action_flags_e;

/**
 *  \brief struct that holds the fields ID and the fields value
 */
typedef struct
{
    dbal_fields_e field_id;
    uint32 field_val[DBAL_FIELD_ARRAY_MAX_SIZE_IN_WORDS];

} dbal_field_data_t;

/**
 *  \brief struct that centrelized offset information, with the
 *         following info the offset is calculated
 */
typedef struct
{
    dbal_fields_e field_id;
    dbal_value_offset_encode_types_e encode_mode;
    uint32 input_param;
    uint32 internal_inparam;/** for DBAL_VALUE_OFFSET_ENCODE_MODULO_FIELD_DEP used with the field size in bits*/
} dbal_offset_encode_info_t;

/**
 *  \brief struct that centrelized field encode information, with the following info logical value to physical value
 * is calculated 
 */
typedef struct
{
    dbal_value_field_encode_types_e encode_mode;
    uint32 input_param;
} dbal_field_encode_info_t;

/**
 *  \brief struct that centrelized sub-field encoding information, with the following info the transformation from
 * sub-field to parent field is preformed 
 */
typedef struct
{
    dbal_fields_e sub_field_id;
    dbal_field_encode_info_t encode_info;
} dbal_sub_field_info_t;

/**
 *  \brief struct that centrelized sub-field encoding information, with the following info the transformation from
 * sub-field to parent field is preformed 
 */
typedef struct
{
    uint32 value;
    char name[DBAL_MAX_STRING_LENGTH];
} dbal_enum_decoding_info_t;

/**
 *  \brief struct that centrelized condition information, with
 *         the following info the condition is calculated
 */
typedef struct
{
    dbal_condition_types_e type;
    uint32 value;
    dbal_fields_e field_id;
} dbal_access_condition_info_t;

/**
 *  \brief fields basic information 
 */
typedef struct
{
    char name[DBAL_MAX_STRING_LENGTH];
    uint32 max_size; /** max size in bits*/
    dbal_field_type_e type;
    int instances_support;
    dbal_labels_e labels[DBAL_MAX_NOF_ENTITY_LABEL_TYPES];
    uint8 is_default_value_valid;
    uint32 default_value;
    uint32 max_value;
    dbal_fields_e parent_field_id[DBAL_FIELD_MAX_PARENTS_PER_FIELD];

    int nof_child_fields;
    dbal_sub_field_info_t *sub_field_info;

    int nof_enum_values;
    dbal_enum_decoding_info_t *enum_val_info;

    dbal_field_encode_info_t encode_info;

} dbal_field_basic_info_t;

/**
 *  \brief logical 2 physical direct access field info 
 */
typedef struct
{
    dbal_fields_e field_id;

    /** 
     *  access_nof_bits and access_offset are used only in the
     *  parsing process. the real offset and size are located in
     *  nof_bits_in_multiple_result, offset_in_multiple_result
     */
    uint32 access_nof_bits;
    uint32 access_offset;

    /** field index in interface structure  */
    int field_pos_in_interface;

    /** field length (bits) in interface structure  */
    uint32 nof_bits_in_interface;

    /** field offset (bits) in interface structure  */
    uint32 offset_in_interface;

    /** hw entities of mapping mem/register, field   */
    soc_mem_t memory;
    soc_reg_t reg[DBAL_MAX_NUMBER_OF_REGISTERS];
    soc_field_t hw_field;

    /** 
     *  array index of mem/reg
     *  Default is wirting to all array's elements
     */
    dbal_offset_encode_info_t array_offset_info;

    /** 
     *  entry index of mem, invalid for register
     *  Default is entire key
     */
    dbal_offset_encode_info_t entry_offset_info;

    /** 
     *  Data offset in HW entity - in bits If hw field is set, the
     *  offset is inside the hw field, if not the offset is iside
     *  the entry
     */
    dbal_offset_encode_info_t data_offset_info;

    /** 
     *  entry index of mem, invalid for register
     *  Default is entire key
     */
    dbal_offset_encode_info_t block_index_info;

    /** 
     *  If set, the data is written to HW using this alias memory.
     */
    soc_mem_t alias_memory;

    /** 
     *  If set, the data is written to HW using this alias register.
     */
    soc_reg_t alias_reg[DBAL_MAX_NUMBER_OF_REGISTERS];

    /** 
     *  Offset calculation for alias memory/register
     */
    dbal_offset_encode_info_t alias_data_offset_info;

    /**
     * Condition for mapping. 
     * Mapping is done only if condition is true. 
     */
    dbal_access_condition_info_t mapping_condition[DBAL_DIRECT_ACCESS_MAX_NUM_OF_CONDITIONS];

} dbal_direct_l2p_field_info_t;

typedef struct
{
    int num_of_access_fields;
    dbal_direct_l2p_field_info_t *l2p_fields_info;
} dbal_direct_l2p_info_t;

typedef struct
{
    dbal_direct_l2p_info_t l2p_direct_info[DBAL_NOF_HL_ACCESS_TYPES];
} dbal_hl_mapping_multi_res_t;

typedef struct
{
    uint32 *key_fields_mapping;
    uint32 *result_fields_mapping;
} dbal_pemla_db_mapping_info_t;

/**
 *  generic field info for table 
 */
typedef struct
{
    /**
     * the field id
     */
    dbal_fields_e field_id;

    /** 
     *  full field size in table.
     *  notice: access layer can define a different bit size
     */
    int field_nof_bits;

    /**
     * offset to take in logical field 
     * the first valid bit to be used in table
     */
    int offset_in_logical_field;

    /** 
     *  location (bit offset) in the handle buffer
     */
    int bits_offset_in_buffer;
    /** 
     *  nof_instances in the table
     */
    int nof_instances;

    /**
     * Indication for field's encoding
     */
    uint8 is_field_encoded;

    uint8 is_sw_field;
    int bytes_offset_in_sw_buffer;

} dbal_table_field_info_t;

typedef struct
{
    uint32 key_size;
    uint32 key[DBAL_PHYSICAL_KEY_SIZE_IN_WORDS];
    uint32 k_mask[DBAL_PHYSICAL_KEY_SIZE_IN_WORDS];
    uint32 payload_size;        /* max payload size for the current logical table (used in MDB) */
    uint32 payload[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];
    uint32 p_mask[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];
    uint8 hitbit;
} dbal_physical_entry_t;

typedef struct
{
    uint32 mdb_entry_index;     /* next entry index to be retrieved (direct table) */
    int mdb_entry_capacity;     /* The table capacity, optimization for direct table */
    uint32 payload_basic_size;  /* The payload basic size associated with direct table (except EEDB/IN_LIF) */

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    mdb_kaps_db_t_p mdb_lpm_db_p;       /* KBPSDK DB pointer */
    mdb_kaps_ad_db_t_p mdb_lpm_ad_db_p; /* KBPSDK AD DB pointer */
    struct kbp_entry_iter *mdb_lpm_iter;        /* KBPSDK iterator pointer */
#endif                          /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
} dbal_physical_entry_iterator_t;

typedef struct
{
    /** Indicates the key size in words (uint32s)  */
    uint32 key_size_in_words;

    /** indicates for done iterating, but current entry is valid */
    uint8 iterated_all_entries;

    /** Last entry to iterated */
    uint32 max_num_of_iterations[DBAL_PHYSICAL_KEY_SIZE_IN_WORDS];

    /** default entry of table */
    uint32 default_entry[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];

} dbal_direct_table_iterator_t;

typedef struct
{
    UTILEX_HASH_TABLE_PTR hash_table_id;
    UTILEX_HASH_TABLE_ITER hash_entry_index;
    uint32 direct_max_index;
    uint8 direct_iterated_all_entries;
    uint32 direct_default_entry[DBAL_PHYSICAL_RES_SIZE_IN_WORDS];

} dbal_sw_table_iterator_t;

typedef struct
{
    dbal_field_type_e type;
    void *returned_pointer;
} dbal_user_output_info_t;

typedef struct
{
    uint32 result_type_hw_value;
    /**
     * result type name. 
     * used for diagnostics and logging 
     */
    char result_type_name[DBAL_MAX_STRING_LENGTH];

    /**
     * check if necesseray
     */
    int entry_payload_size;

    /**
     * Zero padding in buffer, used in MDB multiple result tables
     */
    int zero_padding;

    int sw_payload_length_bytes;
    /** 
     * result field information
     */
    int nof_result_fields;
    dbal_table_field_info_t *results_info;
} multi_res_info_t;

/**
 *  \brief Logical table structure definition 
 */
typedef struct
{
    /*
     * General table parameters
     */
    char table_name[DBAL_MAX_STRING_LENGTH];
    uint32 is_table_valid;
    uint8 is_table_initiated;
    dbal_maturity_level_e maturity_level;
    dbal_labels_e labels[DBAL_MAX_NOF_ENTITY_LABEL_TYPES];
    dbal_table_type_e table_type;
    int nof_entries;
    int min_index;
    int max_capacity;

    /*
     * Interface parameters 
     * information regarding the key and result fields 
     */
    int nof_key_fields;
    dbal_table_field_info_t *keys_info;
    uint32 key_size;

    int nof_result_types;
    multi_res_info_t *multi_res_info;
    int sw_payload_length_bytes;
    int max_payload_size;
    int max_nof_result_fields;

    /*
     * logical to physical information
     */
    dbal_core_mode_e core_mode;
    dbal_access_method_e access_method;

    /*
     * MDB info
     */
    uint32 app_id;
    dbal_physical_tables_e physical_db_id;

    /*
     * Hard logic (or HL+SW) 
     * Structure is allocatesd staticaly because its index is corresponding to the access_typ enum 
     * the unify_res_mapping field indicates that all result types are mapped to the same HW place 
     */
    uint8 unify_res_mapping;
    dbal_hl_mapping_multi_res_t *hl_mapping_multi_res;

    /*
     * Pemla mapping DB
     */
    uint32 pemla_db_id;
    dbal_pemla_db_mapping_info_t pemla_mapping;

} dbal_logical_table_t;

/** 
 * \brief 
 *  Error information related to field_set/ field_get operations.
 */
typedef struct
{

    /**
     * True if error occured
     */
    uint8 error_exists;

    /**
     * The field ID related to the error
     */
    dbal_fields_e field_id;
} field_error_info_t;

typedef struct
{
    dbal_tables_e table_id;

    /** 
     *  pointer to the corresponding table
     */
    dbal_logical_table_t *table;

    /** 
     *  all key fields added to the table according to thier position in the table
     *  it saves the actual field ID in this position, to support subfields
     */
    dbal_fields_e key_field_ids[DBAL_TABLE_MAX_NUM_OF_KEY_FIELDS];
    uint8 nof_key_fields;

    /** 
     *  all value fields added to the table according to thier position in the table
     *  it saves the actual field ID in this position, to support subfields
     */
    dbal_fields_e value_field_ids[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS];
    uint8 nof_result_fields;

    /** 
     *  Total number of fields
     */
    uint8 num_of_fields;

    /** 
     *  Not in used, multiple transcation support
     */
    uint8 trans_id;

    /** 
     *  Handle status according to enum
     */
    dbal_entry_handle_status_e handle_status;

    /** 
     *  buffers that contains all the fields values
     */
    dbal_physical_entry_t phy_entry;

    /**
     * core_id holds the core to preform the action, 
     * in table that works in core_mode by input if the core ID was 
     * not added we use the default value 
     */
    int core_id;

    int nof_result_types;

    /**
     * the current result type value init with -1 valid values can 
     * be 0 - DBAL_MAX_NUMBER_OF_RESULT_TYPES
     */
    int cur_res_type;

    /** 
     *  information about get fields that will be updated after
     *  entry get
     */
    dbal_user_output_info_t user_output_info[DBAL_TABLE_MAX_NUM_OF_RESULT_FIELDS];

    /** 
     *  error information that is saved in the entry handle during
     *  field set/get procedure call, this info is returned to user
     *  only after calling entry commit/get/delete
     */
    field_error_info_t error_info;

    /** 
     *  indicate that all fields are requested.
     */
    uint8 get_all_fields;
} dbal_entry_handle_t;

typedef struct
{
    dbal_fields_e field_id;
    uint32 field_val;
    dbal_condition_types_e condition;

} dbal_iterator_rule_info_t;

typedef struct
{
    dbal_fields_e field_id;
    uint32 *field_val;

} dbal_returned_field_t;

typedef struct
{
    /** DBAL table ID */
    dbal_tables_e table_id;

    /** entry handle id*/
    uint32 entry_handle_id;

    /** entry handle for iterator */
    dbal_entry_handle_t *entry_handle;

    /** entries found counter */
    uint32 entries_counter;

    /** indicates for done iterating, Current entry is not valid */
    uint8 is_end;

    /** Direct tables (H.L and SW direct) iterator data */
    dbal_direct_table_iterator_t direct_iterator;

    /** SW HASH iterator data */
    dbal_sw_table_iterator_t sw_iterator;

    /** MDB iterator data */
    dbal_physical_entry_iterator_t mdb_iterator;

    /** which entries will be chosen - TBD */
    dbal_iterator_rule_info_t rule;
    dbal_iterator_mode_e mode;

    /** returned info by iterator */
    int nof_key_fields;
    dbal_field_data_t *keys_info;
    int nof_result_fields;
    dbal_field_data_t *results_info;

} dbal_iterator_info_t;

/*****************************************************PHYSICAL TABLE DEFENITIONS***************************************************************/
typedef shr_error_e(
    *PHYSICAL_TABLE_ENTRY_ADD) (
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

typedef shr_error_e(
    *PHYSICAL_TABLE_ENTRY_GET) (
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

typedef shr_error_e(
    *PHYSICAL_TABLE_ENTRY_DELETE) (
    int unit,
    dbal_physical_tables_e physical_table,
    uint32 app_id,
    dbal_physical_entry_t * entry);

typedef shr_error_e(
    *PHYSICAL_TABLE_CLEAR) (
    int unit);

typedef shr_error_e(
    *PHYSICAL_TABLE_DEFAULT_VALUES_SET) (
    int unit);

typedef shr_error_e(
    *PHYSICAL_TABLE_INIT) (
    int unit);

typedef shr_error_e(
    *PHYSICAL_TABLE_DEINIT) (
    int unit);

typedef shr_error_e(
    *PHYSICAL_TABLE_ITERATOR_INIT) (
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator);

typedef shr_error_e(
    *PHYSICAL_TABLE_ITERATOR_GET_NEXT) (
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator,
    dbal_physical_entry_t * entry,
    uint8 * is_end);

typedef shr_error_e(
    *PHYSICAL_TABLE_ITERATOR_DEINIT) (
    int unit,
    dbal_physical_tables_e dbal_physical_table_id,
    uint32 app_id,
    dbal_physical_entry_iterator_t * physical_entry_iterator);

typedef struct
{
    /**************************************** Database information ******************************************/
    char physical_name[DBAL_MAX_STRING_LENGTH];
    dbal_physical_tables_e physical_db_type;
    int max_capacity;
    int nof_entries;

    /**************************************** Database operations ********************************************/
    PHYSICAL_TABLE_ENTRY_ADD entry_add;
    PHYSICAL_TABLE_ENTRY_GET entry_get;
    PHYSICAL_TABLE_ENTRY_DELETE entry_delete;
    PHYSICAL_TABLE_CLEAR table_clear;
    PHYSICAL_TABLE_DEFAULT_VALUES_SET table_default_values_set;
    PHYSICAL_TABLE_INIT table_init;
    PHYSICAL_TABLE_DEINIT table_deinit;
    PHYSICAL_TABLE_ITERATOR_INIT iterator_init;
    PHYSICAL_TABLE_ITERATOR_GET_NEXT iterator_get_next;
    PHYSICAL_TABLE_ITERATOR_DEINIT iterator_deinit;

} dbal_physical_table_def_t;

shr_error_e dbal_physical_table_get(
    int unit,
    dbal_physical_tables_e physical_table_id,
    dbal_physical_table_def_t ** physical_table);

shr_error_e dbal_physical_table_init(
    int unit);

shr_error_e dbal_physical_table_deinit(
    int unit);

const char *dbal_physical_table_name_get(
    int unit,
    dbal_physical_tables_e physical_table_id);

typedef struct
{
    dbal_physical_table_def_t physical_tables[DBAL_NOF_PHYSICAL_TABLES];

} dbal_physical_mngr_info_t;

typedef struct
{
    dbal_logical_table_t logical_tables[DBAL_NOF_TABLES];

} dbal_logical_tables_info_t;

typedef struct
{
    dbal_entry_handle_t entry_handles_pool[DBAL_SW_NOF_ENTRY_HANDLES];
    dbal_entry_handle_t entry_handle_for_default_entry;
    uint8 num_of_entry_handles_used;
    dbal_work_modes_e work_mode;
    uint8 is_intiated;
} dbal_mngr_info_t;

/*
 ******************
 *****SW STATE*****
 ****************** */
/**
 * \brief 
 * enum that represent the sw state table type for dbal tables
 */
typedef enum
{
    /** direct table - the entry is set by the key
     *  used for direct logical tables */
    DBAL_SW_TABLE_DIRECT = 0,

    /** hash table - the entry is decided by hash table mechanism
     *  used for non-direct logical tables (EM,TCAM,LPM) */
    DBAL_SW_TABLE_HASH,

    DBAL_NOF_SW_TABLES_TYPES
} dbal_sw_state_table_type_e;

/**
 * \brief 
 * an entry in dbal sw table. implemented by sw state buffer. 
 * allocated dynamically according to the payload size of the 
 * table 
 */
typedef struct
{
    SW_STATE_BUFF *entry_buffer;
} dbal_sw_state_entry_in_table_t;

/**
 * \brief 
 * dbal sw table. hash table will be allocated accotding to 
 * table type. 
 * All tables type (direct,hash) will store the payload in the 
 * entries
 */
typedef struct
{
    /** the table type */
    dbal_sw_state_table_type_e table_type;

    /** the entries - where the table data is stored.
     *  allocated dynamically according to key size in direct
     *  table or user input in hash tables */
    PARSER_HINT_ARR dbal_sw_state_entry_in_table_t *entries;

    /** hash table ID - valid if table type is hash table */
    UTILEX_HASH_TABLE_PTR hash_table_id;
} dbal_sw_state_table_t;

#endif /* DBAL_STRUCTURES_H_INCLUDED */
