/* $Id: ppc_api_frwrd_fcf.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_FCF.h
*
* MODULE PREFIX:  soc_ppc_frwrd
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_PPC_API_FRWRD_FCF_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_FCF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define SOC_PPC_FRWRD_FCF_ZONING_ACT_FRWRD      0x1
/* for TRAP and drop */
#define SOC_PPC_FRWRD_FCF_ZONING_ACT_REDIRECT   0x2


/* local domain: add to SEM */
#define SOC_PPC_FRWRD_FCF_ROUTE_LOCAL_DOMAIN    0x1
/* add domain to LPM*/
#define SOC_PPC_FRWRD_FCF_ROUTE_DOMAIN          0x2
/* host add to LEM table (24 bits) */
#define SOC_PPC_FRWRD_FCF_ROUTE_HOST            0x4
/* add host to LEM nport (24 bits)*/
#define SOC_PPC_FRWRD_FCF_ROUTE_HOST_NPORT      0x8
/* add domain to LPM nport (0-8 bits)*/
#define SOC_PPC_FRWRD_FCF_ROUTE_DOMAIN_NPORT    0x10



/* flags for fcf_zoning_table_clear */
/* remove entries match VFT */
#define SOC_PPC_FRWRD_FCF_ZONE_REMOVE_BY_VFT    0x1
/* remove entries match D_ID */
#define SOC_PPC_FRWRD_FCF_ZONE_REMOVE_BY_D_ID   0x2
/* remove entries match S_ID*/
#define SOC_PPC_FRWRD_FCF_ZONE_REMOVE_BY_S_ID   0x4



/* Virtual Fabric Identifier */
typedef uint32 SOC_PPC_FRWRD_FCF_VFI;

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
   *  The route exists in both sofwate cache and in device.
   */
  SOC_PPC_FRWRD_FCF_ROUTE_STATUS_COMMITED = 1,
  /*
   *  The route exists in software cache and is pending to be
   *  commited (added) into device.
   */
  SOC_PPC_FRWRD_FCF_ROUTE_STATUS_PEND_ADD = 2,
  /*
   *  The route was removed from software cache and is pending
   *  to be commited (removed) into device.
   */
  SOC_PPC_FRWRD_FCF_ROUTE_STATUS_PEND_REMOVE = 4,
  /*
   *  entry was accessed by traffic lookup
   */
  SOC_PPC_FRWRD_FCF_ROUTE_STATUS_ACCESSED = 8,
  /*
   *  Number of types in SOC_PPC_FRWRD_IP_ROUTE_STATUS
   */
  SOC_PPC_NOF_FRWRD_FCF_ROUTE_STATUSS = 4
}SOC_PPC_FRWRD_FCF_ROUTE_STATUS;



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* enable FC functionality, load relevant programs */
  uint8 enable;
} SOC_PPC_FRWRD_FCF_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  route flags see SOC_PPC_FRWRD_FCF_ROUTE
   */
  uint32 flags;
  /*
   *  Virtual Fabric Identifier
   */
  SOC_PPC_FRWRD_FCF_VFI vfi;
  /*
   *  Destination ID
   */
  uint32 d_id;
  /*
   *  Number of bits to consider in the IPD_ID address starting 
   *  from the msb. Range: 0 - 32.                             
   */
  uint8 prefix_len;


} SOC_PPC_FRWRD_FCF_ROUTE_KEY;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  destination inforrmation.
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_decision;
} SOC_PPC_FRWRD_FCF_ROUTE_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Virtual Fabric Identifier
   */
  SOC_PPC_FRWRD_FCF_VFI vfi;
  /*
   *  Destination ID
   */
  uint32 d_id;
  /*
   *  sourec ID
   */
  uint32 s_id;

} SOC_PPC_FRWRD_FCF_ZONING_KEY;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Zoning actions flags.
   *  see SOC_PPC_FRWRD_FCF_ZONING
   */
  uint32 flags;
  /*
   *  destination inforrmation.
   */
  SOC_PPC_FRWRD_DECISION_INFO frwrd_decision;

} SOC_PPC_FRWRD_FCF_ZONING_INFO;


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


void
  SOC_PPC_FRWRD_FCF_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_FCF_ROUTE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY *info
  );
void
  SOC_PPC_FRWRD_FCF_ROUTE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO *info
  );


void
  SOC_PPC_FRWRD_FCF_ZONING_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_KEY *info
  );

void
  SOC_PPC_FRWRD_FCF_ZONING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO *info
  );


#if SOC_PPC_DEBUG_IS_LVL1


const char*
  SOC_PPC_FRWRD_FCF_ROUTE_STATUS_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_STATUS enum_val
  );

void
  SOC_PPC_FRWRD_FCF_GLBL_INFO_print(
    SOC_SAND_IN SOC_PPC_FRWRD_FCF_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_FCF_ROUTE_KEY_print(
    SOC_SAND_IN SOC_PPC_FRWRD_FCF_ROUTE_KEY *info
  );

void
  SOC_PPC_FRWRD_FCF_ROUTE_INFO_print(
    SOC_SAND_IN SOC_PPC_FRWRD_FCF_ROUTE_INFO *info
  );


void
  SOC_PPC_FRWRD_FCF_ZONING_KEY_print(
    SOC_SAND_IN SOC_PPC_FRWRD_FCF_ZONING_KEY *info
  );

void
  SOC_PPC_FRWRD_FCF_ZONING_INFO_print(
    SOC_SAND_IN SOC_PPC_FRWRD_FCF_ZONING_INFO *info
  );


#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_FCF_INCLUDED__*/
#endif

