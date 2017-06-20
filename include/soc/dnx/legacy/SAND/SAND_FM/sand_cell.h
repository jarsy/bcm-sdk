/* $Id: sand_cell.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef DNX_SAND_CELL_H_INCLUDED
#define DNX_SAND_CELL_H_INCLUDED

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>


/* $Id: sand_cell.h,v 1.7 Broadcom SDK $
 * DNX_SAND data cell types.
 */
#define DATA_CELL_TYPE_SOURCE_ROUTED      1
#define DATA_CELL_TYPE_DESTINATION_ROUTED 0


/*
 * DNX_SAND cells byte size.
 */
#define DNX_SAND_DATA_CELL_BYTE_SIZE 40
#define DNX_SAND_DATA_CELL_UINT32_SIZE (DNX_SAND_DATA_CELL_BYTE_SIZE/sizeof(uint32))
#define DNX_SAND_CTRL_CELL_BYTE_SIZE 10

/*
 * Definitions related to 'paths' element.
 * {
 */

#define DNX_SAND_PATHS_RX_SRC_LVL_MS_BIT      10
#define DNX_SAND_PATHS_RX_SRC_LVL_NUM_BITS    3
#define DNX_SAND_PATHS_RX_SRC_LVL_LS_BIT      (DNX_SAND_PATHS_RX_SRC_LVL_MS_BIT + 1 - DNX_SAND_PATHS_RX_SRC_LVL_NUM_BITS)
#define DNX_SAND_PATHS_RX_SRC_LVL_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_PATHS_RX_SRC_LVL_MS_BIT) - DNX_SAND_BIT(DNX_SAND_PATHS_RX_SRC_LVL_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_PATHS_RX_SRC_LVL_MS_BIT))
#define DNX_SAND_PATHS_RX_SRC_LVL_SHIFT       DNX_SAND_PATHS_RX_SRC_LVL_LS_BIT

#define DNX_SAND_PATHS_RX_SRC_ID_MS_BIT       21
#define DNX_SAND_PATHS_RX_SRC_ID_NUM_BITS     11
#define DNX_SAND_PATHS_RX_SRC_ID_LS_BIT     (DNX_SAND_PATHS_RX_SRC_ID_MS_BIT + 1 - DNX_SAND_PATHS_RX_SRC_ID_NUM_BITS)
#define DNX_SAND_PATHS_RX_SRC_ID_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_PATHS_RX_SRC_ID_MS_BIT) - DNX_SAND_BIT(DNX_SAND_PATHS_RX_SRC_ID_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_PATHS_RX_SRC_ID_MS_BIT))
#define DNX_SAND_PATHS_RX_SRC_ID_SHIFT        DNX_SAND_PATHS_RX_SRC_ID_LS_BIT

/*
 * }
 */

/*
 * Definitions related to 'last_tx' element.
 * {
 */
#define DNX_SAND_LAST_TX_MS_BIT                31
#define DNX_SAND_LAST_TX_NUM_BITS              24
#define DNX_SAND_LAST_TX_LS_BIT                (DNX_SAND_LAST_TX_MS_BIT + 1 - DNX_SAND_LAST_TX_NUM_BITS)
#define DNX_SAND_LAST_TX_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_LAST_TX_MS_BIT) - DNX_SAND_BIT(DNX_SAND_LAST_TX_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_LAST_TX_MS_BIT))
#define DNX_SAND_LAST_TX_SHIFT                 DNX_SAND_LAST_TX_LS_BIT

/*
 * }
 */
/*
 * Definitions related to 'switches' element.
 * {
 */
#define DNX_SAND_SWITCHES_TX_FIRST_BYTE_MS_BIT     7
#define DNX_SAND_SWITCHES_TX_FIRST_BYTE_NUM_BITS   8
#define DNX_SAND_SWITCHES_TX_FIRST_BYTE_LS_BIT     (DNX_SAND_SWITCHES_TX_FIRST_BYTE_MS_BIT + 1 - DNX_SAND_SWITCHES_TX_FIRST_BYTE_NUM_BITS)
#define DNX_SAND_SWITCHES_TX_FIRST_BYTE_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FIRST_BYTE_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FIRST_BYTE_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FIRST_BYTE_MS_BIT))
#define DNX_SAND_SWITCHES_TX_FIRST_BYTE_SHIFT      DNX_SAND_SWITCHES_TX_FIRST_BYTE_LS_BIT


#define DNX_SAND_SWITCHES_TX_FE3SW_MS_BIT         20
#define DNX_SAND_SWITCHES_TX_FE3SW_NUM_BITS       5
#define DNX_SAND_SWITCHES_TX_FE3SW_LS_BIT         (DNX_SAND_SWITCHES_TX_FE3SW_MS_BIT + 1 - DNX_SAND_SWITCHES_TX_FE3SW_NUM_BITS)
#define DNX_SAND_SWITCHES_TX_FE3SW_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FE3SW_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FE3SW_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FE3SW_MS_BIT))
#define DNX_SAND_SWITCHES_TX_FE3SW_SHIFT          DNX_SAND_SWITCHES_TX_FE3SW_LS_BIT

#define DNX_SAND_SWITCHES_TX_FE2SW_MS_BIT         26
#define DNX_SAND_SWITCHES_TX_FE2SW_NUM_BITS       6
#define DNX_SAND_SWITCHES_TX_FE2SW_LS_BIT         (DNX_SAND_SWITCHES_TX_FE2SW_MS_BIT + 1 - DNX_SAND_SWITCHES_TX_FE2SW_NUM_BITS)
#define DNX_SAND_SWITCHES_TX_FE2SW_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FE2SW_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FE2SW_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_SWITCHES_TX_FE2SW_MS_BIT))
#define DNX_SAND_SWITCHES_TX_FE2SW_SHIFT          DNX_SAND_SWITCHES_TX_FE2SW_LS_BIT

/*
 * }
 */

/*
 * Definitions related to 'paths' element.
 * {
 */

#define DNX_SAND_PATHS_TX_DEST_LVL_MS_BIT     7
#define DNX_SAND_PATHS_TX_DEST_LVL_NUM_BITS   3
#define DNX_SAND_PATHS_TX_DEST_LVL_LS_BIT     (DNX_SAND_PATHS_TX_DEST_LVL_MS_BIT + 1 - DNX_SAND_PATHS_TX_DEST_LVL_NUM_BITS)
#define DNX_SAND_PATHS_TX_DEST_LVL_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_PATHS_TX_DEST_LVL_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_PATHS_TX_DEST_LVL_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_PATHS_TX_DEST_LVL_MS_BIT))
#define DNX_SAND_PATHS_TX_DEST_LVL_SHIFT      DNX_SAND_PATHS_TX_DEST_LVL_LS_BIT

#define DNX_SAND_PATHS_TX_SRC_LVL_MS_BIT      10
#define DNX_SAND_PATHS_TX_SRC_LVL_NUM_BITS    3
#define DNX_SAND_PATHS_TX_SRC_LVL_LS_BIT      (DNX_SAND_PATHS_TX_SRC_LVL_MS_BIT + 1 - DNX_SAND_PATHS_TX_SRC_LVL_NUM_BITS)
#define DNX_SAND_PATHS_TX_SRC_LVL_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_PATHS_TX_SRC_LVL_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_PATHS_TX_SRC_LVL_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_PATHS_TX_SRC_LVL_MS_BIT))
#define DNX_SAND_PATHS_TX_SRC_LVL_SHIFT       DNX_SAND_PATHS_TX_SRC_LVL_LS_BIT

#define DNX_SAND_PATHS_TX_SRC_ID_MS_BIT       21
#define DNX_SAND_PATHS_TX_SRC_ID_NUM_BITS     11
#define DNX_SAND_PATHS_TX_SRC_ID_LS_BIT     (DNX_SAND_PATHS_TX_SRC_ID_MS_BIT + 1 - DNX_SAND_PATHS_TX_SRC_ID_NUM_BITS)
#define DNX_SAND_PATHS_TX_SRC_ID_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_PATHS_TX_SRC_ID_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_PATHS_TX_SRC_ID_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_PATHS_TX_SRC_ID_MS_BIT))
#define DNX_SAND_PATHS_TX_SRC_ID_SHIFT        DNX_SAND_PATHS_TX_SRC_ID_LS_BIT


#define DNX_SAND_PATHS_TX_COLN_MS_BIT         29
#define DNX_SAND_PATHS_TX_COLN_NUM_BITS       6
#define DNX_SAND_PATHS_TX_COLN_LS_BIT         (DNX_SAND_PATHS_TX_COLN_MS_BIT + 1 - DNX_SAND_PATHS_TX_COLN_NUM_BITS)
#define DNX_SAND_PATHS_TX_COLN_MASK   \
    (((uint32)DNX_SAND_BIT(DNX_SAND_PATHS_TX_COLN_MS_BIT) - \
    DNX_SAND_BIT(DNX_SAND_PATHS_TX_COLN_LS_BIT)) + DNX_SAND_BIT(DNX_SAND_PATHS_TX_COLN_MS_BIT))
#define DNX_SAND_PATHS_TX_COLN_SHIFT          DNX_SAND_PATHS_TX_COLN_LS_BIT



#define DNX_SAND_DATA_CELL_CELL_TYPE_START          (318)
#define DNX_SAND_DATA_CELL_CELL_TYPE_LENGTH         (2  )
#define DNX_SAND_DATA_CELL_SOURCE_ID_START          (307)
#define DNX_SAND_DATA_CELL_SOURCE_ID_LENGTH         (11 )
#define DNX_SAND_DATA_CELL_SRC_LEVEL_START          (304)
#define DNX_SAND_DATA_CELL_SRC_LEVEL_LENGTH         (3  )
#define DNX_SAND_DATA_CELL_DEST_LEVEL_START         (301)
#define DNX_SAND_DATA_CELL_DEST_LEVEL_LENGTH        (3  )
#define DNX_SAND_DATA_CELL_FIP_SWITCH_START         (296)
#define DNX_SAND_DATA_CELL_FIP_SWITCH_LENGTH        (5  )
#define DNX_SAND_DATA_CELL_FE1_SWITCH_START         (291)
#define DNX_SAND_DATA_CELL_FE1_SWITCH_LENGTH        (5  )
#define DNX_SAND_DATA_CELL_FE2_SWITCH_START         (285)
#define DNX_SAND_DATA_CELL_FE2_SWITCH_LENGTH        (6  )
#define DNX_SAND_DATA_CELL_FE3_SWITCH_START         (280)
#define DNX_SAND_DATA_CELL_FE3_SWITCH_LENGTH        (5  )
#define DNX_SAND_DATA_CELL_DEST_ID_START            (296 )
#define DNX_SAND_DATA_CELL_DEST_ID_LENGTH           (11 )
#define DNX_SAND_DATA_CELL_ORIGIN_TIME_START        (281 )
#define DNX_SAND_DATA_CELL_ORIGIN_TIME_LENGTH       (15 )
#define DNX_SAND_DATA_CELL_FRAG_NUMBER_START        (272 )
#define DNX_SAND_DATA_CELL_FRAG_NUMBER_LENGTH       (9  )
#define DNX_SAND_DATA_CELL_PAYLOAD_START            (16 )
#define DNX_SAND_DATA_CELL_PAYLOAD_IN_UINT32S         (8  )
#define DNX_SAND_DATA_CELL_PAYLOAD_IN_BYTES         (32 )

#define DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD  (5  )
#define DNX_SAND_SR_DATA_CELL_WORD_LENGTH           (49 )
#define DNX_SAND_SR_DATA_CELL_RW_START              (27 )
#define DNX_SAND_SR_DATA_CELL_WRITE_LENGTH          (32 )
#define DNX_SAND_SR_DATA_CELL_ADDRESS_START         (59 )
#define DNX_SAND_SR_DATA_CELL_ADDRESS_LENGTH        (16 )
#define DNX_SAND_SR_DATA_CELL_VALID_START           (75 )
#define DNX_SAND_SR_DATA_CELL_CELL_IDENT_LENGTH     (9  )
#define DNX_SAND_SR_DATA_CELL_CELL_IDENT_START      (18)
#define DNX_SAND_SR_DATA_CELL_CELL_FORMAT_LENGTH    (2  )
#define DNX_SAND_SR_DATA_CELL_CELL_FORMAT_START     (16)
#define DNX_SAND_SR_DATA_CELL_FIP_SWITCH_START      (5  )
#define DNX_SAND_SR_DATA_CELL_FIP_SWITCH_LENGTH     (1  )
#define DNX_SAND_SR_DATA_CELL_FIP_SWITCH_POSITION   (279)
#define DNX_SAND_SR_DATA_CELL_FE1_SWITCH_START      (5  )
#define DNX_SAND_SR_DATA_CELL_FE1_SWITCH_LENGTH     (1  )
#define DNX_SAND_SR_DATA_CELL_FE1_SWITCH_POSITION   (278)
#define DNX_SAND_SR_DATA_CELL_FE2_SWITCH_START      (6  )
#define DNX_SAND_SR_DATA_CELL_FE2_SWITCH_LENGTH     (1  )
#define DNX_SAND_SR_DATA_CELL_FE2_SWITCH_POSITION   (277)
#define DNX_SAND_SR_DATA_CELL_FE3_SWITCH_START      (5  )
#define DNX_SAND_SR_DATA_CELL_FE3_SWITCH_LENGTH     (1  )
#define DNX_SAND_SR_DATA_CELL_FE3_SWITCH_POSITION   (276)
#define DNX_SAND_SR_DATA_CELL_INBAND_CELL_LENGTH    (1  )
#define DNX_SAND_SR_DATA_CELL_INBAND_CELL_POSITION  (275)
#define DNX_SAND_SR_DATA_CELL_ACK_LENGTH            (1  )
#define DNX_SAND_SR_DATA_CELL_ACK_POSITION          (274)
#define DNX_SAND_SR_DATA_CELL_INDIRECT_LENGTH       (1  )
#define DNX_SAND_SR_DATA_CELL_INDIRECT_POSITION     (273)
#define DNX_SAND_SR_DATA_CELL_RW_LENGTH             (1  )
#define DNX_SAND_SR_DATA_CELL_RW_POSITION           (272)
#define DNX_SAND_SR_DATA_CELL_NOT_COMMON_LENGTH     (307)

#define DNX_SAND_SR_DATA_CELL_NOT_INBAND_ADDRESS_START         (32 )
#define DNX_SAND_SR_DATA_CELL_NOT_INBAND_RW_START              (112 )
#define DNX_SAND_SR_DATA_CELL_NOT_INBAND_CELL_IDENT_LENGTH     (16 )
#define DNX_SAND_SR_DATA_CELL_NOT_INBAND_CELL_IDENT_START      (16)

/*
 * }
 */

typedef enum
{
  DNX_SAND_CONTROL_CELL_TYPE_FLOW_STATUS = 0,

  DNX_SAND_CONTROL_CELL_NOF_TYPES
} DNX_SAND_CONTROL_CELL_TYPE;

typedef enum
{
  DNX_SAND_SCHEDULER_FLOW_STATUS_OFF = 0,
  DNX_SAND_SCHEDULER_FLOW_STATUS_ON  = 2,

  DNX_SAND_SCHEDULER_FLOW_NUM_STATUS
} DNX_SAND_SCHEDULER_FLOW_STATUS ;


typedef struct
{
  /*
   * flow status specific fields
   */
  uint32               sched_flow_id; /* 14 bits                 */
  uint32               input_q_number;/* 14 bits                 */
  DNX_SAND_SCHEDULER_FLOW_STATUS flow_status;   /* 2  bits   */
} DNX_SAND_CONTROL_CELL_FLOW_STATUS;

/*
 * the super set of all fields acceptable within a control cell
 */
typedef struct
{
  DNX_SAND_CONTROL_CELL_TYPE type;      /* 3  bits                 */
  uint32           dest_id;   /* 11 bits                 */
  uint32           source_id; /* 11 bits                 */

  union
  {
    DNX_SAND_CONTROL_CELL_FLOW_STATUS  flow_status_info;
  } u;

} DNX_SAND_CONTROL_CELL;



/*
 * Definitions related to source routed data cells
 * {
 */
/*
 * Structure of all elements composing a source routed data cell
 * to transmit.
 */
#define DNX_SAND_SR_DATA_NOF_DATA_UINT32S (8)

typedef struct
{
    /*
     * Eight uint32s (256 bits) of general data to carry
     * on cell. Payload.
     */
  uint32  body_tx[DNX_SAND_SR_DATA_NOF_DATA_UINT32S] ;
    /*
     * If there is an FE3 entity on the way, NOT including
     * entity which initiates this transmission, this entry
     * instructs it on which link to use when passing the
     * cell through.
     * Range: 0-31
     */
  uint32   fe3_link ;
    /*
     * If there is an FE2 entity on the way, NOT including
     * entity which initiates this transmission, this entry
     * instructs it on which link to use when passing the
     * cell through.
     * Range: 0-63
     */
  uint32   fe2_link ;
    /*
     * If there is an FE1 entity on the way, this entry
     * instructs it on which link to use when passing the
     * cell through. (relevant only to sr_cells routed
     * through FAP, therefore there is no use for this field
     * in the context of FE driver).
     * Range: 0-31
     */
  uint32   fe1_link ;
    /*
     * Type of entity to send cell to.
     * Allowed values are DNX_SAND_FE1_ENTITY, DNX_SAND_FE2_ENTITY,
     * DNX_SAND_FE3_ENTITY, DNX_SAND_FOP_ENTITY.
     * See 'Device types on the fabric'.
     */
  DNX_SAND_DEVICE_ENTITY   destination_entity_type ;
    /*
     * Type of entity sending this cell. Allowed values
     * are FE2_ENTITY, FE3_ENTITY, FE1_ENTITY, DNX_SAND_FIP_ENTITY,
     * DONT_CARE_ENTITY. In the latter case, system
     * will use value currently stored in 'configuration'
     * internal register.
     * See Device types on the fabric above.
     */
  DNX_SAND_DEVICE_ENTITY   source_entity_type ;
    /*
     * Id of device to send cell from. If value
     * is -1 (all one's) then system will use value
     * currently stored in 'configuration' internal
     * register.
     * Range: 0-2047
     */
  uint32   source_chip_id ;
    /*
     * Output link to use at the originator chip of
     * this message.
     * Range 0-63 (on DNX_SAND_FE200) or 0-8 (on DNX_SAND_FAP10M).
     * For originating chip configured as FE13, if link
     * number is in the range 32-63 then cell is sent
     * via FE1 links, otherwise, it is sent via FE3 links.
     */
  uint32   initial_output_link ;
} DNX_SAND_TX_SR_DATA_CELL  ;
/*
 * Structure of all elements composing a received source routed
 * data cell. Essentially this is an exact copy of
 * DNX_SAND_TX_SR_DATA_CELL
 */
typedef struct
{
    /*
     * Eight uint32s (256 bits) of general data received
     * payload.
     */
  uint32       body_tx[DNX_SAND_SR_DATA_NOF_DATA_UINT32S] ;
    /*
     * Type of entity that has sent this cell. See
     * 'Device types on the fabric' above. Allowed values
     * are FE1_ENTITY, FE2_ENTITY, FE3_ENTITY, FIP_ENTITY.
     */
  DNX_SAND_DEVICE_ENTITY  source_entity_type ;
    /*
     * Id of device that has sent this cell.
     * Range: 0-2047
     */
  uint32        source_chip_id ;
} DNX_SAND_RX_SR_DATA_CELL  ;
/*
 * }
 */
/*
 * packs a structured control cell to 10 bytes
 */
DNX_SAND_RET
  dnx_sand_pack_control_cell(
    DNX_SAND_IN     DNX_SAND_CONTROL_CELL control_cell,
    DNX_SAND_INOUT  unsigned char     *packed_control_cell
  );
/*
 * unpacks 10 bytes to a structured control cell
 */
DNX_SAND_RET
  dnx_sand_unpack_control_cell(
    DNX_SAND_IN     unsigned char     *packed_control_cell,
    DNX_SAND_INOUT  DNX_SAND_CONTROL_CELL *control_cell
  );
/*
 * The specific fields for a destination routed cell
 */
typedef struct
{
  /*
   * The first field is the destination id or the multicast id
   */
  uint16   dest_or_mc_id;                              /* 11 bits                      */
  uint16   origin_time;                                /* 15 bits - also for multicast */
  uint16   frag_number;                                /* 9  bits - also for multicast */
  uint8    cell_data[DNX_SAND_DATA_CELL_PAYLOAD_IN_BYTES]; /* 256 bits                */

}__ATTRIBUTE_PACKED__ DNX_SAND_DESTINATION_ROUTED_CELL;
/*
 * The specific fields for a source routed cell
 */
typedef struct
{
  /*
   * source routed (cpu2cpu) data cell specific fields
   */
  uint8   src_level;      /* 3 bits                  */
  uint8   dest_level;     /* 3 bits                  */
  uint8   fip_switch;     /* 6 bits                  */
  uint8   fe1_switch;     /* 6 bits                  */
  uint8   fe2_switch;     /* 7 bits                  */
  uint8   fe3_switch;     /* 6 bits                  */
  uint8   inband_cell;    /* 1 bit                   */
  uint8   ack;            /* 1 bit                   */
  uint8   indirect;       /* 1 bit                   */
  uint8   read_or_write;  /* 1 bit                   */
  /*
   * The fields for building the payload, composed of
   * five write_cells with data and an address
   */
  uint32   data_wr_cell[DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD];  /* 160 bits                 */
  uint32   add_wr_cell[DNX_SAND_SR_DATA_CELL_NOF_WORDS_IN_PAYLOAD];   /*  80 bits                 */
  uint16   cell_ident;                                            /*   9 bits                 */
  uint8    cell_format;                                           /*   2 bits                 */

}__ATTRIBUTE_PACKED__ DNX_SAND_SOURCE_ROUTED_CELL;
/*
 * the super set of all fields acceptable within a data cell
 */

typedef struct
{
  uint8    cell_type;                                  /* 2  bits                 */
  uint16   source_id;                                  /* 11 bits                 */

  union
  {
    DNX_SAND_DESTINATION_ROUTED_CELL dest_routed;
    DNX_SAND_SOURCE_ROUTED_CELL source_routed;

  }data_cell;
}__ATTRIBUTE_PACKED__  DNX_SAND_DATA_CELL;

/*
 * Converts a Dnx_soc_sand_data_cell to the appropriate buffer according to its type
 */
uint32
  dnx_sand_data_cell_to_buffer(
    DNX_SAND_IN  DNX_SAND_DATA_CELL   *data_cell,
    DNX_SAND_OUT uint32         *packed_data_cell
  );
/*
 * Converts a buffer to the appropriate Dnx_soc_sand_data_cell according to its type
 */
uint32
  dnx_sand_buffer_to_data_cell(
    DNX_SAND_IN  uint32        *packed_data_cell,
    DNX_SAND_IN  uint8       is_fe600,
    DNX_SAND_OUT DNX_SAND_DATA_CELL  *data_cell
  );


/*
 * Translates from the entity type to the three bit field in the cell
 */
uint32
  dnx_sand_actual_entity_value(
    DNX_SAND_IN DNX_SAND_DEVICE_ENTITY device_entity
  );
/*
 * Translates from the three bit field in the cell to the entity type
 */
DNX_SAND_DEVICE_ENTITY
  dnx_sand_real_entity_value(
    DNX_SAND_IN DNX_SAND_ENTITY_LEVEL_TYPE device_entity_3b
  );

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif
