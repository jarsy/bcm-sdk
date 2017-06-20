/* $Id: jer2_arad_cell.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_CELL_INCLUDED__
/* { */
#define __JER2_ARAD_CELL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_cell.h>
#include <soc/dnx/legacy/ARAD/arad_framework.h>
#include <soc/dnxc/legacy/vsc256_fabric_cell.h>
#include <soc/dnxc/legacy/dnxc_fabric_cell.h>
#include <soc/dnxc/legacy/dnxc_fabric_source_routed_cell.h>

#include <soc/dnx/legacy/TMC/tmc_api_cell.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_DNX_JER2_ARAD_NUM_OF_ROUTE_GROUPS    (32)
#define SOC_DNX_JER2_ARAD_NUM_OF_ENTRIES         (2048)

#define JER2_ARAD_DATA_CELL_BYTE_SIZE           (139)
#define JER2_ARAD_DATA_CELL_U32_SIZE            (35)

#define JER2_ARAD_DATA_CELL_RECEIVED_BYTE_SIZE                 (73)
#define JER2_ARAD_DATA_CELL_RECEIVED_UINT32_SIZE                 (19)
#define JER2_ARAD_RECEIVED_DATA_CELL_HEADER_START              (520)
#define JER2_ARAD_RECEIVED_DATA_CELL_HEADER_OFFSET_VSC256      (16)
#define JER2_ARAD_RECEIVED_DATA_CELL_TYPE_START                (583)
#define JER2_ARAD_RECEIVED_DATA_CELL_SIZE_IN_BITS              (512)
#define JER2_ARAD_RECUEVED_DATA_CELL_CELL_SIZE_LENGTH_IN_BITS  (8)

#define JER2_ARAD_RECEIVED_DATA_CELL_HEADER_SIZE_IN_BITS_VSC128 (56)

#define JER2_ARAD_DATA_CELL_NOT_INBAND_BYTE_SIZE     (64)
#define JER2_ARAD_MIN_PAYLOAD_BYTE_SIZE              (64)

#define JER2_ARAD_PARSING_CELL_TYPE_START            (512 + 63)
#define JER2_ARAD_PARSING_CELL_TYPE_LENGTH           (2)

#define JER2_ARAD_SR_DATA_CELL_PAYLOAD_START         (0)
#define JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH        (1024)
#define JER2_ARAD_SR_DATA_CELL_PAYLOAD_LENGTH_U32    (32)

#define JER2_ARAD_INBAND_PAYLOAD_CELL_OFFSET         (512)

#define JER2_ARAD_CELL_NOF_LINKS_IN_PATH_LINKS            (4)
#define JER2_ARAD_CELL_NOF_BYTES_IN_UINT32                  (4)
#define JER2_ARAD_CELL_NOF_DATA_WORDS_IN_INDIRECT_CELL    (3)

#define JER2_ARAD_IN_BAND_TIMEOUT_ATTEMPTS_CFG       (10)
/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define JER2_ARAD_DO_NOTHING_AND_EXIT    goto exit

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct
{
  /*
   *  The cell ident of the source routed cell
   */
  uint16  current_cell_ident;
}  JER2_ARAD_CELL;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

int soc_dnx_jer2_arad_fabric_inband_is_valid(int unit, int id, uint32 *is_valid);/* route or group*/
int soc_dnx_jer2_arad_fabric_inband_route_group_set(int unit, int group_id, int flags, int route_count, int *route_ids);
int soc_dnx_jer2_arad_fabric_inband_route_set(int unit, int route_id, dnxc_soc_fabric_inband_route_t *route);
int soc_dnx_jer2_arad_fabric_inband_route_group_get(int unit, int group_id, int flags, int route_count_max, int *route_count, int *route_ids);
int soc_dnx_jer2_arad_fabric_inband_route_get(int unit, int route_id, dnxc_soc_fabric_inband_route_t *route);

soc_error_t
soc_jer2_arad_parse_cell(int unit, uint8 is_inband, uint32* buf, dnxc_vsc256_sr_cell_t* cell);

uint32
  jer2_arad_sr_send_cell(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint32       is_fe1600,
    DNX_SAND_IN  uint32        route_group_id,
    DNX_SAND_IN  uint8       is_group,
    DNX_SAND_IN  DNX_SAND_DATA_CELL    *data_cell_sent_old_format,
    DNX_SAND_IN  dnxc_vsc256_sr_cell_t  *data_cell_sent
  );

uint32
  jer2_arad_cell_ack_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint8                      is_inband,
    DNX_SAND_OUT uint32                       *ack,
    DNX_SAND_OUT uint8                      *success
  );

uint32
  jer2_arad_transaction_with_fe1600(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST         *sr_link_list,
    DNX_SAND_IN  uint32                        route_group_id,
    DNX_SAND_IN  uint8                       is_group,
    DNX_SAND_IN  soc_reg_t                       *regs,
    DNX_SAND_IN  soc_mem_t                       *mems,
    DNX_SAND_IN  int32                        *port_or_copyno,
    DNX_SAND_IN  int32                        *index,
    DNX_SAND_IN  uint32                        *data_in,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint8                       is_write,
    DNX_SAND_IN  uint8                       is_inband,
    DNX_SAND_IN  uint8                       is_memory,
    DNX_SAND_OUT soc_reg_above_64_val_t          *data_out,
    DNX_SAND_OUT uint32                        *data_out_size
  );

uint32
  jer2_arad_cpu2cpu_write_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST          *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        *data_in
  );

uint32
  jer2_arad_cpu2cpu_read_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_OUT uint32                        *data_out
  );

uint32
  jer2_arad_read_from_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST         *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_OUT uint32                        *data_out
  );

uint32
  jer2_arad_indirect_read_from_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST          *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_OUT uint32                        *data_out
  );

uint32
  jer2_arad_write_to_fe600_unsafe(
    DNX_SAND_IN  int                       unit,
    DNX_SAND_IN  DNX_TMC_SR_CELL_LINK_LIST        *sr_link_list,
    DNX_SAND_IN  uint32                        size,
    DNX_SAND_IN  uint32                        offset,
    DNX_SAND_IN  uint32                        *data_in
  );

/* 
 * common to the old format and to the new format.
 * For the new format data_cell_sent will pointer to the structure. 
 * For the old format it will be NULL                                                              .
 */
uint32
  jer2_arad_sr_send_and_wait_ack(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  dnxc_vsc256_sr_cell_t                 *data_cell_sent,
    DNX_SAND_IN  DNX_SAND_DATA_CELL                   *data_cell_sent_old,
    DNX_SAND_IN  uint32                         route_group_id,
    DNX_SAND_IN  uint8                        is_group,
    DNX_SAND_OUT dnxc_vsc256_sr_cell_t                 *data_cell_rcv,
    DNX_SAND_OUT DNX_SAND_DATA_CELL                   *data_cell_rcv_old
  );

uint32
  jer2_arad_fabric_cpu2cpu_write(    
    DNX_SAND_IN  int                                    unit,
    DNX_SAND_IN dnxc_sr_cell_link_list_t                *sr_link_list,
    DNX_SAND_IN uint32                                  data_in_size,
    DNX_SAND_IN uint32                                  *data_in
   );

uint32
  jer2_arad_fabric_cell_cpu_data_get(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_OUT uint32                  *cell_buffer
  );


#define SOC_JER2_ARAD_FABRIC_CELL_CAPTURED_SRC_FORMAT_NOF_LINES    (18)
#define SOC_JER2_ARAD_FABRIC_CELL_CAPTURED_SRC_FORMAT {\
  /*{dest_id,                                           dest_ptr,     dest_start_bit,   src_start_bit, length,  name)*/\
    {soc_dnxc_fabric_cell_dest_id_src_cell_type,        NULL,         0,                583,            2,      "CELL_TYPE"},\
    {soc_dnxc_fabric_cell_dest_id_src_src_device,       NULL,         0,                555,            11,     "SOURCE_DEVICE"},\
    {soc_dnxc_fabric_cell_dest_id_src_src_level,        NULL,         0,                552,            3,      "SOURCE_LEVEL"},\
    {soc_dnxc_fabric_cell_dest_id_src_dest_level,       NULL,         0,                549,            3,      "DEST_LEVEL"},\
    {soc_dnxc_fabric_cell_dest_id_src_fip,              NULL,         0,                544,            5,      "FIP[0:4]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fip,              NULL,         5,                527,            1,      "FIP[5]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe1,              NULL,         0,                539,            5,      "FE1[0:4]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe1,              NULL,         5,                526,            1,      "FE1[5]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe1,              NULL,         6,                569,            2,      "FE1[6:7]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe2,              NULL,         0,                533,            6,      "FE2[0:5]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe2,              NULL,         6,                525,            1,      "FE2[6]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe2,              NULL,         7,                568,            1,      "FE2[7]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe3,              NULL,         0,                528,            5,      "FE3[0:4]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe3,              NULL,         5,                524,            1,      "FE3[5]"},\
    {soc_dnxc_fabric_cell_dest_id_src_fe3,              NULL,         6,                566,            2,      "FE3[6:7]"},\
    {soc_dnxc_fabric_cell_dest_id_src_is_inband,        NULL,         0,                523,            1,      "IS_INBAND"},\
    {soc_dnxc_fabric_cell_dest_id_src_pipe_id,          NULL,         0,                571,            3,      "PIPE_ID"},\
    {soc_dnxc_fabric_cell_dest_id_src_payload,          NULL,         0,                0,              512,    "PAYLOAD"}\
    }


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_CELL_INCLUDED__*/
#endif

