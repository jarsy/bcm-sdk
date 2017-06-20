/*
 * $Id: fe3200_fabric_cell_snake_test.h,v 1.6.106.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE3200 FABRIC CELL SNAKE TEST H
 */
 
#ifndef _SOC_FE3200_FABRIC_CELL_SNAKE_TEST_H_
#define _SOC_FE3200_FABRIC_CELL_SNAKE_TEST_H_

/**********************************************************/
/*                  Includes                              */
/**********************************************************/

#include <soc/error.h>
#include <soc/types.h>

#include <soc/dfe/cmn/dfe_defs.h>
#include <soc/dfe/cmn/dfe_fabric_cell_snake_test.h>



/**********************************************************/
/*                  Defines                               */
/**********************************************************/

/**********************************************************/
/*                  Functions                             */
/**********************************************************/

int
  soc_fe3200_cell_snake_test_prepare(
    int unit, 
    uint32 flags);
    
int
  soc_fe3200_cell_snake_test_run(
    int unit, 
    uint32 flags, 
    soc_fabric_cell_snake_test_results_t* results);

#endif
