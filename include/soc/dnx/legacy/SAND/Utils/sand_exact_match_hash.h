/* $Id: dnx_sand_exact_match_hash.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef DNX_SAND_EXACT_MATCH_HASH_H_INCLUDED
/* { */
#define DNX_SAND_EXACT_MATCH_HASH_H_INCLUDED

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Utils/sand_exact_match.h>

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
  dnx_sand_exact_match_hash_key_to_verifier_31_19(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, 
    DNX_SAND_IN uint32 table_id, 
    DNX_SAND_OUT DNX_SAND_EXACT_MATCH_VERIFIER verifier
  );

DNX_SAND_EXACT_MATCH_HASH_VAL 
  dnx_sand_exact_match_hash_key_to_hash_31_19(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, 
    DNX_SAND_IN uint32 table_id
  );

void 
  dnx_sand_exact_match_hash_key_to_verifier_31_20(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, 
    DNX_SAND_IN uint32 table_id, 
    DNX_SAND_OUT DNX_SAND_EXACT_MATCH_VERIFIER verifier
  );

DNX_SAND_EXACT_MATCH_HASH_VAL 
  dnx_sand_exact_match_hash_key_to_hash_31_20(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, 
    DNX_SAND_IN uint32 table_id
  );

void 
  dnx_sand_exact_match_hash_key_to_verifier_25_12(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, 
    DNX_SAND_IN uint32 table_id, 
    DNX_SAND_OUT DNX_SAND_EXACT_MATCH_VERIFIER verifier
  );

DNX_SAND_EXACT_MATCH_HASH_VAL 
  dnx_sand_exact_match_hash_key_to_hash_25_13(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, 
    DNX_SAND_IN uint32 table_id
  );

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } DNX_SAND_EXACT_MATCH_HASH_H_INCLUDED*/
#endif



