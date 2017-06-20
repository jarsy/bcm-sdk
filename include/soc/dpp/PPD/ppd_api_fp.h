/* $Id: ppd_api_fp.h,v 1.59 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __SOC_PPD_API_FP_INCLUDED__
/* { */
#define __SOC_PPD_API_FP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_fp.h>

#include <soc/dpp/PPD/ppd_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPD_FP_PACKET_FORMAT_GROUP_SET = SOC_PPD_PROC_DESC_BASE_FP_FIRST,
  SOC_PPD_FP_PACKET_FORMAT_GROUP_SET_PRINT,
  SOC_PPD_FP_PACKET_FORMAT_GROUP_GET,
  SOC_PPD_FP_PACKET_FORMAT_GROUP_GET_PRINT,
  SOC_PPD_FP_DATABASE_CREATE,
  SOC_PPD_FP_DATABASE_CREATE_PRINT,
  SOC_PPD_FP_DATABASE_GET,
  SOC_PPD_FP_DATABASE_GET_PRINT,
  SOC_PPD_FP_DATABASE_DESTROY,
  SOC_PPD_FP_DATABASE_DESTROY_PRINT,
  SOC_PPD_FP_ENTRY_ADD,
  SOC_PPD_FP_ENTRY_ADD_PRINT,
  SOC_PPD_FP_ENTRY_GET,
  SOC_PPD_FP_ENTRY_GET_PRINT,
  SOC_PPD_FP_ENTRY_REMOVE,
  SOC_PPD_FP_ENTRY_REMOVE_PRINT,
  SOC_PPD_FP_DATABASE_ENTRIES_GET_BLOCK,
  SOC_PPD_FP_DATABASE_ENTRIES_GET_BLOCK_PRINT,
  SOC_PPD_FP_DIRECT_EXTRACTION_ENTRY_ADD,
  SOC_PPD_FP_DIRECT_EXTRACTION_ENTRY_ADD_PRINT,
  SOC_PPD_FP_DIRECT_EXTRACTION_ENTRY_GET,
  SOC_PPD_FP_DIRECT_EXTRACTION_ENTRY_GET_PRINT,
  SOC_PPD_FP_DIRECT_EXTRACTION_ENTRY_REMOVE,
  SOC_PPD_FP_DIRECT_EXTRACTION_ENTRY_REMOVE_PRINT,
  SOC_PPD_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET,
  SOC_PPD_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET_PRINT,
  SOC_PPD_FP_CONTROL_SET,
  SOC_PPD_FP_CONTROL_SET_PRINT,
  SOC_PPD_FP_CONTROL_GET,
  SOC_PPD_FP_CONTROL_GET_PRINT,
  SOC_PPD_FP_EGR_DB_MAP_SET,
  SOC_PPD_FP_EGR_DB_MAP_SET_PRINT,
  SOC_PPD_FP_EGR_DB_MAP_GET,
  SOC_PPD_FP_EGR_DB_MAP_GET_PRINT,
  SOC_PPD_FP_PACKET_DIAG_GET,
  SOC_PPD_FP_PACKET_DIAG_GET_PRINT,
  SOC_PPD_FP_RESOURCE_DIAG_GET,
  SOC_PPD_FP_RESOURCE_DIAG_GET_PRINT,
  SOC_PPD_FP_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FP_PROCEDURE_DESC_LAST
} SOC_PPD_FP_PROCEDURE_DESC;

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

/*********************************************************************
* NAME:
 *   soc_ppd_fp_packet_format_group_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a Packet Format Group (PFG). The packet format group
 *   defines the supported Packet formats. The user must
 *   indicate for each Database which Packet format(s) are
 *   associated with this Database. E.g.: A Packet Format
 *   Group including only IPv6 packets can be defined to use
 *   Databases with IPv6 Destination-IP qualifiers.
 * INPUT:
 *   SOC_SAND_IN  int                  unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                  pfg_ndx -
 *     Packet Format Group index. Range: 0 - 4.
 *   SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO            *info -
 *     Packet-Format-Group parameters.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE       *success -
 *     Indicate if the database is created successfully.
 * REMARKS:
 *   The user should set a minimal number of Packet Format
 *   Groups since each one uses many Hardware resources.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_packet_format_group_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  pfg_ndx,
    SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO            *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE       *success
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_fp_packet_format_group_set" API.
 *     Refer to "soc_ppd_fp_packet_format_group_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_fp_packet_format_group_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  pfg_ndx,
    SOC_SAND_OUT SOC_PPC_PMF_PFG_INFO            *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_database_create
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Create a database. Each database specifies the action
 *   types to perform and the qualifier fields for this
 *   Database. Entries in the database specify the specific
 *   actions to be taken upon specific values of the
 *   packet. E.g.: Policy Based Routing database update the
 *   FEC value according to DSCP DIP and In-RIF. An entry in
 *   the database may set the FEC of a packet with DIP
 *   1.2.2.3, DSCP value 7 and In-RIF 3 to be 9.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO                    *info -
 *     Database parameters.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Indicate if the database is created successfully.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_database_create(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_INOUT  SOC_PPC_FP_DATABASE_INFO                    *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_database_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the database parameters.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_OUT SOC_PPC_FP_DATABASE_INFO                    *info -
 *     Database parameters.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_database_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_OUT SOC_PPC_FP_DATABASE_INFO                    *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_database_destroy
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Destroy the database: all its entries are suppressed and
 *   the Database-ID is freed.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_database_destroy(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to the Database. The database entry is
 *   selected if the entire relevant packet field values are
 *   matched to the database entry qualifiers values. When
 *   the packet is qualified to several entries, the entry
 *   with the strongest priority is chosen.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  uint32                               entry_id_ndx -
 *     Entry handle ID. The entry index is a SW handle, to
 *     enable retrieving the entry attributes by the
 *     soc_ppd_fp_entry_get() function, and remove it by the
 *     soc_ppd_fp_entry_remove() function. The actual location of
 *     the entry in the database is selected according to the
 *     entry's priority. Range: 0 - 4K-1.
 *   SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO                       *info -
 *     Entry parameters.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Indicate if the database entry is created successfully.
 * REMARKS:
 *   1. The database must be created before the insertion of
 *   an entry. To create a Database, call the
 *   soc_ppd_fp_database_create API.2. The database must be NOT
 *   of type 'direct extraction'3. For a Database of type
 *   'Direct Table', the entry qualifier value must be not
 *   masked.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_entry_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_IN  uint32                               entry_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO                       *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry from the Database.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  uint32                               entry_id_ndx -
 *     Entry-ID. Range: 0 - 4K-1.
 *   SOC_SAND_OUT uint8                               *is_found -
 *     If True, then the entry is found and the entry
 *     parameters are returned in the 'info' structure.
 *     Otherwise, the entry is not present in the Database.
 *   SOC_SAND_OUT SOC_PPC_FP_ENTRY_INFO                       *info -
 *     Entry parameters. Meaningful only the entry is found.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_IN  uint32                               entry_id_ndx,
    SOC_SAND_OUT uint8                                *is_found,
    SOC_SAND_INOUT SOC_PPC_FP_ENTRY_INFO              *info
  );

uint32
  soc_ppd_fp_entry_remove_by_key(
    SOC_SAND_IN     int                               unit,
    SOC_SAND_IN     uint32                               db_id_ndx,
    SOC_SAND_INOUT  SOC_PPC_FP_ENTRY_INFO                *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_entry_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry from the Database.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  uint32                               entry_id_ndx -
 *     Entry-ID. Range: 0 - 4K-1.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_entry_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_IN  uint32                               entry_id_ndx,
    SOC_SAND_IN  uint32                               is_sw_remove_only
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_database_entries_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the Database entries. The function returns list of
 *   entries that were added to a database with database ID
 *   'db_id_ndx'.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Block range to get entries in this Database.
 *   SOC_SAND_OUT SOC_PPC_FP_ENTRY_INFO                       *entries -
 *     Database entries.
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of valid entries.
 * REMARKS:
 *   This API can be called only if the Database is NOT of
 *   type 'direct extraction'.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_database_entries_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_PPC_FP_ENTRY_INFO                       *entries,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_direct_extraction_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to the Database. The database entry is
 *   selected if all the Packet Qualifier field values are in
 *   the Database entry range.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  uint32                               entry_id_ndx -
 *     Entry-ID. Range: 0 - 15.
 *   SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info -
 *     Entry parameters.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Indicate if the database entry is created successfully.
 * REMARKS:
 *   1. The database must be created before the insertion of
 *   an entry. To create a Database, call the
 *   soc_ppd_fp_database_create API.2. The database must be of
 *   type 'direct extraction'3. The priority enables
 *   selection between two database entries with a
 *   superposition in the Qualifier field ranges.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_direct_extraction_entry_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_IN  uint32                               entry_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_direct_extraction_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry from the Database.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  uint32                               entry_id_ndx -
 *     Entry-ID. Range: 0 - 15.
 *   SOC_SAND_OUT uint8                               *is_found -
 *     If True, then the entry is found and the entry
 *     parameters are returned in the 'info' structure.
 *     Otherwise, the entry is not present in the Database.
 *   SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info -
 *     Entry parameters. Meaningful only the entry is found.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_direct_extraction_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_IN  uint32                               entry_id_ndx,
    SOC_SAND_OUT uint8                               *is_found,
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_direct_extraction_entry_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry from the Database.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  uint32                               entry_id_ndx -
 *     Entry-ID. Range: 0 - 15.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_direct_extraction_entry_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               db_id_ndx,
    SOC_SAND_IN  uint32                               entry_id_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_direct_extraction_db_entries_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the Database entries.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range -
 *     Block range to get entries in this Database.
 *   SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *entries -
 *     Database entries.
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of valid entries.
 * REMARKS:
 *   This API can be called only if the Database is of type
 *   'direct extraction'.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_direct_extraction_db_entries_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  db_id_ndx,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE     *block_range,
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *entries,
    SOC_SAND_OUT uint32                   *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_control_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set one of the control options.
 * INPUT:
 *   SOC_SAND_IN  int                  unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX      *control_ndx -
 *     Index for the control set API.
 *   SOC_SAND_IN  SOC_PPC_FP_CONTROL_INFO        *info -
 *     Type and Values of selected control option.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE       *success -
 *     Indicate if the operation has succeeded.
 * REMARKS:
 *   The exact semantics of info are determined by the
 *   control option specified by type.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_control_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  int                  core_id,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX      *control_ndx,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INFO        *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE       *success
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_fp_control_set"
 *     API.
 *     Refer to "soc_ppd_fp_control_set" API for details.
*********************************************************************/
uint32
  soc_ppd_fp_control_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  int                  core_id,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX       *control_ndx,
    SOC_SAND_OUT SOC_PPC_FP_CONTROL_INFO        *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_egr_db_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the mapping between the Packet forward type and the
 *   Port profile to the Database-ID.
 * INPUT:
 *   SOC_SAND_IN  int                  unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx -
 *     Packet forward type.
 *   SOC_SAND_IN  uint32                  port_profile_ndx -
 *     PP-Port profile. Range: 0 - 3.
 *   SOC_SAND_IN  uint32                   db_id -
 *     Database-Id to use for these packets. Range: 0 - 63.
 * REMARKS:
 *   1. The mapping between PP-Port and PP-Port profile is
 *   set via the soc_ppd_fp_control_set API with type
 *   SOC_PPC_FP_CONTROL_TYPE_PP_PORT_PROFILE.2. The Database with
 *   this Database-Id must exist and correspond to an Egress
 *   ACL Database
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_egr_db_map_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx,
    SOC_SAND_IN  uint32                  port_profile_ndx,
    SOC_SAND_IN  uint32                   db_id
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_fp_egr_db_map_set" API.
 *     Refer to "soc_ppd_fp_egr_db_map_set" API for details.
*********************************************************************/
uint32
  soc_ppd_fp_egr_db_map_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx,
    SOC_SAND_IN  uint32                  port_profile_ndx,
    SOC_SAND_OUT uint32                   *db_id
  );

/*********************************************************************
*     Compress a TCAM Database: compress the entries to minimum
*     number of banks.
*********************************************************************/
uint32
  soc_ppd_fp_database_compress(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  db_id_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_fp_packet_diag_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the Field Processing of the last packets.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_INFO *info -
 *     Field Processor specifications for this packet.
 * REMARKS:
 *   This API must be called during a continuous stream of
 *   the identical packets coming from the same source.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_fp_packet_diag_get(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  int               core_id,
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_INFO *info
  );


uint32
  soc_ppd_fp_resource_diag_get(
    SOC_SAND_IN 	int               		unit,
	SOC_SAND_IN	 	SOC_PPC_FP_RESOURCE_DIAG_MODE	mode,
    SOC_SAND_OUT 	SOC_PPC_FP_RESOURCE_DIAG_INFO	*info
  );

uint32
  soc_ppd_fp_dbs_action_info_show(
    SOC_SAND_IN 	int unit
  ) ;

uint32
  soc_ppd_fp_action_info_show(
    SOC_SAND_IN 	int unit
  ) ;

uint32
  soc_ppd_fp_print_all_fems_for_stage(
    SOC_SAND_IN   int                    unit,
    SOC_SAND_IN   uint32                 stage,
    SOC_SAND_IN   uint8                  is_for_tm
  ) ;

uint32
  soc_ppd_fp_print_fes_info_for_stage(
    SOC_SAND_IN   int                    unit,
    SOC_SAND_IN   uint32                 stage,
    SOC_SAND_IN   uint32                 pmf_pgm_ndx
  ) ;

uint32
    soc_ppd_fp_ire_traffic_send(
        SOC_SAND_IN int          unit,
        SOC_SAND_IN SOC_PPC_FP_PACKET  *packet,
        SOC_SAND_IN int          tx_count,
        SOC_SAND_IN int          core
    );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FP_INCLUDED__*/
#endif

