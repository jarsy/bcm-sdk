/*
 *         
 * $Id: furia_pkg_cfg.h 2014/04/02 palanivk Exp $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$ 
 *         
 *     
 *
 */
#ifndef __FURIA_PKG_CFG_H__
#define __FURIA_PKG_CFG_H__
#include "furia_types.h"
/*
* Includes
*/

/*
* Defines
*/
#define FURIA_ID_82071         0x82071
#define FURIA_ID_82070         0x82070
#define FURIA_ID_82073         0x82073
#define FURIA_ID_82072         0x82072
#define FURIA_ID_82380         0x82380
#define FURIA_ID_82381         0x82381
#define FURIA_ID_82385         0x82385
#define FURIA_ID_82208         0x82208
#define FURIA_ID_82209         0x82209
#define FURIA_ID_82212         0x82212
#define FURIA_ID_82216         0x82216
#define FURIA_ID_82314         0x82314
#define FURIA_ID_82315         0x82315

#define MAX_NUM_PACKAGES       11
#define MAX_NUM_LANES          12 
#define MAX_NUM_PHYS           32 

/*
* Types
*/

/*
* Macros
*/

/*
* Global Variables
*/

extern const struct _FURIA_PKG_LANE_CFG_S *const glb_package_array[MAX_NUM_PACKAGES];

extern struct _FURIA_PHY_LIST_S glb_phy_list[MAX_NUM_PHYS];
/*
* Functions
*/
#endif /* __FURIA_PKG_CFG_H__ */
