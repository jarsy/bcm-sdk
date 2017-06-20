/*
 *         
 * $Id: phymod.xml,v 1.1.2.5 Broadcom SDK $
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *     
 */

#include <phymod/phymod.h>
#include <phymod/phymod_dispatch.h>
#include <phymod/phymod_util.h>


int merlin16_phy_media_type_tx_get(const phymod_phy_access_t* phy, phymod_media_typed_t media, phymod_tx_t* tx)
{
    switch (media) {
    case phymodMediaTypeChipToChip:
        tx->pre   = 0x4;
        tx->main  = 0x25;
        tx->post  = 0x13;
        tx->post2 = 0x0;
        break;
    case phymodMediaTypeShort:
        tx->pre   = 0x4;
        tx->main  = 0x25;
        tx->post  = 0x13;
        tx->post2 = 0x0;
        break;
    case phymodMediaTypeMid:
        tx->pre   = 0x4;
        tx->main  = 0x25;
        tx->post  = 0x13;
        tx->post2 = 0x0;
        break;
    case phymodMediaTypeLong:
        tx->pre   = 0x4;
        tx->main  = 0x25;
        tx->post  = 0x13;
        tx->post2 = 0x0;
        break;
    default:
        tx->pre   = 0x4;
        tx->main  = 0x25;
        tx->post  = 0x13;
        tx->post2 = 0x0;
        break;
    }

    return PHYMOD_E_NONE;
}
