/**
 * \file algo_l3.h Internal DNX L3 Managment APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$ 
 */

#ifndef ALGO_L3_H_INCLUDED
/*
 * { 
 */
#define ALGO_L3_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/**
* \brief
*   Intialize l3 algorithms.
*  
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*  \par INDIRECT INPUT:
*    - DBAL table sizes, used to initialize source address table template.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \par INDIRECT OUTPUT:
*      None
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_l3_init(
    int unit);

/**
* \brief
*   Deintialize l3 algorithms.
*  
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*  \par INDIRECT INPUT:
*    - DBAL table sizes, used to initialize source address table template.
*  \par DIRECT OUTPUT:
*    shr_error_e - 
*      Error return value
*  \par INDIRECT OUTPUT:
*      None
*  \remark
*    None
*  \see
*    shr_error_e
*****************************************************/
shr_error_e dnx_algo_l3_deinit(
    int unit);

/*
 * } 
 */
#endif/*_ALGO_L3_API_INCLUDED__*/
