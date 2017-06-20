/**
 * \file algo_l3.h Internal DNX L3 Managment APIs
PIs $Copyright: (c) 2016 Broadcom.
PIs Broadcom Proprietary and Confidential. All rights reserved.$ 
 */

#ifndef SOURCE_ADDRESS_TABLE_ALLOCATION_H_INCLUDED
/*
 * { 
 */
#define SOURCE_ADDRESS_TABLE_ALLOCATION_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <bcm/types.h>

/**
 * \brief Source address table allocation management. 
 *  
 * The source address table is used for adding source addresses to packets.  It can hold either mac, 
 *  IPv4 or IPv6 entries. 
 * The algorithm manages the profile ID according to profile data which present  the requirements from 
 *  the source address table. 
 * The profile returned represents the access key for this table.
 */

/*
 * { 
 */

/** 
 * \brief Source address table supported types under one union structure to ease on the algorithm management.
 */
typedef union
{
    /*
     * MAC address.
     */
    bcm_mac_t mac_address;
    /*
     * IPv4 address.
     */
    bcm_ip_t ipv4_address;
    /*
     * IPv6 address.
     */
    bcm_ip6_t ipv6_address;
} source_address_t;

/** 
 * \brief Identifier of the entry type.
 */
typedef enum
{
    source_address_type_invalid = -1,
    source_address_type_mac,
    source_address_type_ipv4,
    source_address_type_ipv6,
    source_address_type_count
} source_address_type_e;

/** 
 * \brief The data to be stored in the source address table template. 
 *  
 * This data is composed of the address, and the address type.
 */
typedef struct
{
    /*
     * Address type. Used to identify the entry.
     */
    source_address_type_e address_type;
    /*
     * Address.
     */
    source_address_t address;
} source_address_entry_t;

/*
 * } 
 */

/**
* \brief
*   Intialize the source address table algorithm.
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
shr_error_e dnx_algo_l3_source_address_table_init(
    int unit);

/**
* \brief
*   Dentialize the source address table algorithm.
*   Currently doens't do anything, since template manager doesn't require
*    deinitialization per template, and the sw state is deinitialized in
*    dnx_algo_l3_deinit.
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
shr_error_e dnx_algo_l3_source_address_table_deinit(
    int unit);

/*
 * } 
 */
#endif /* SOURCE_ADDRESS_TABLE_ALLOCATION_H_INCLUDED */
