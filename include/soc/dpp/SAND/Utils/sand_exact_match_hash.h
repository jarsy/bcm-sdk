/* $Id: soc_sand_exact_match_hash.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef SOC_SAND_EXACT_MATCH_HASH_H_INCLUDED
/* { */
#define SOC_SAND_EXACT_MATCH_HASH_H_INCLUDED

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_exact_match.h>

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
  soc_sand_exact_match_hash_key_to_verifier_31_19(
    SOC_SAND_IN SOC_SAND_EXACT_MATCH_KEY key, 
    SOC_SAND_IN uint32 table_id, 
    SOC_SAND_OUT SOC_SAND_EXACT_MATCH_VERIFIER verifier
  );

SOC_SAND_EXACT_MATCH_HASH_VAL 
  soc_sand_exact_match_hash_key_to_hash_31_19(
    SOC_SAND_IN SOC_SAND_EXACT_MATCH_KEY key, 
    SOC_SAND_IN uint32 table_id
  );

void 
  soc_sand_exact_match_hash_key_to_verifier_31_20(
    SOC_SAND_IN SOC_SAND_EXACT_MATCH_KEY key, 
    SOC_SAND_IN uint32 table_id, 
    SOC_SAND_OUT SOC_SAND_EXACT_MATCH_VERIFIER verifier
  );

SOC_SAND_EXACT_MATCH_HASH_VAL 
  soc_sand_exact_match_hash_key_to_hash_31_20(
    SOC_SAND_IN SOC_SAND_EXACT_MATCH_KEY key, 
    SOC_SAND_IN uint32 table_id
  );

void 
  soc_sand_exact_match_hash_key_to_verifier_25_12(
    SOC_SAND_IN SOC_SAND_EXACT_MATCH_KEY key, 
    SOC_SAND_IN uint32 table_id, 
    SOC_SAND_OUT SOC_SAND_EXACT_MATCH_VERIFIER verifier
  );

SOC_SAND_EXACT_MATCH_HASH_VAL 
  soc_sand_exact_match_hash_key_to_hash_25_13(
    SOC_SAND_IN SOC_SAND_EXACT_MATCH_KEY key, 
    SOC_SAND_IN uint32 table_id
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } SOC_SAND_EXACT_MATCH_HASH_H_INCLUDED*/
#endif



