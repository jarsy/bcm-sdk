/******************************************************************************
 ******************************************************************************
 *  Revision      :   *
 *                                                                            *
 *  Description   :  Enum types used by Serdes API functions                  *
 *                                                                            *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$                                                      *
 *  No portions of this material may be reproduced in any form without        *
 *  the written permission of:                                                *
 *      Broadcom Corporation                                                  *
 *      5300 California Avenue                                                *
 *      Irvine, CA  92617                                                     *
 *                                                                            *
 *  All information contained in this document is Broadcom Corporation        *
 *  company private proprietary, and trade secret.                            *
 *                                                                            *
 ******************************************************************************
 ******************************************************************************/

/** @file falcon16_tsc_enum.h
 * Enum types used by Serdes API functions
 */

#ifndef FALCON16_TSC_API_ENUM_H
#define FALCON16_TSC_API_ENUM_H

#include "falcon16_tsc_ipconfig.h"
/*#include <stdint.h>*/
#include <phymod/phymod.h>

enum falcon16_tsc_pll_refclk_enum {
    FALCON16_TSC_PLL_REFCLK_UNKNOWN = 0, /* Refclk value to be determined by API. */
    FALCON16_TSC_PLL_REFCLK_100MHZ         = 0x00100064UL, /* 100 MHz         */
    FALCON16_TSC_PLL_REFCLK_106P25MHZ      = 0x004001A9UL, /* 106.25 MHz      */
    FALCON16_TSC_PLL_REFCLK_122P88MHZ      = 0x01900C00UL, /* 122.88 MHz      */
    FALCON16_TSC_PLL_REFCLK_125MHZ         = 0x0010007DUL, /* 125 MHz         */
    FALCON16_TSC_PLL_REFCLK_155P52MHZ      = 0x01900F30UL, /* 155.52 MHz      */
    FALCON16_TSC_PLL_REFCLK_156P25MHZ      = 0x00400271UL, /* 156.25 MHz      */
    FALCON16_TSC_PLL_REFCLK_159P375MHZ     = 0x008004FBUL, /* 159.375 MHz     */
    FALCON16_TSC_PLL_REFCLK_161MHZ         = 0x001000A1UL, /* 161    MHz      */
    FALCON16_TSC_PLL_REFCLK_161P1328125MHZ = 0x08005091UL, /* 161.1328125 MHz */
    FALCON16_TSC_PLL_REFCLK_166P67MHZ      = 0x0640411BUL, /* 166.67 MHz      */
    FALCON16_TSC_PLL_REFCLK_174P703125MHZ  = 0x04002BADUL, /* 174.703125 MHz  */
    FALCON16_TSC_PLL_REFCLK_176P45MHZ      = 0x01400DC9UL, /* 176.45 MHz      */
    FALCON16_TSC_PLL_REFCLK_212P5MHZ       = 0x002001A9UL, /* 212.5  MHz      */
    FALCON16_TSC_PLL_REFCLK_322MHZ         = 0x00100142UL, /* 322    MHz      */
    FALCON16_TSC_PLL_REFCLK_352P9MHZ       = 0x00A00DC9UL  /* 352.9  MHz      */
    };


enum falcon16_tsc_pll_div_enum {
    FALCON16_TSC_PLL_DIV_UNKNOWN = 0, /* Divide value to be determined by API. */
    FALCON16_TSC_PLL_DIV_80         =          0x00000050UL, /* Divide by 80         */
    FALCON16_TSC_PLL_DIV_96         =          0x00000060UL, /* Divide by 96         */
    FALCON16_TSC_PLL_DIV_100        =          0x00000064UL, /* Divide by 100        */
    FALCON16_TSC_PLL_DIV_120        =          0x00000078UL, /* Divide by 120        */
    FALCON16_TSC_PLL_DIV_127P401984 =          0x66E8707FUL, /* Divide by 127.401984 */
    FALCON16_TSC_PLL_DIV_128        =          0x00000080UL, /* Divide by 128        */
    FALCON16_TSC_PLL_DIV_132        =          0x00000084UL, /* Divide by 132        */
    FALCON16_TSC_PLL_DIV_140        =          0x0000008CUL, /* Divide by 140        */
    FALCON16_TSC_PLL_DIV_144        =          0x00000090UL, /* Divide by 144        */
    FALCON16_TSC_PLL_DIV_147P2      =          0x33333093UL, /* Divide by 147.2      */
    FALCON16_TSC_PLL_DIV_158P4      =          0x6666609EUL, /* Divide by 158.4      */
    FALCON16_TSC_PLL_DIV_160        =          0x000000A0UL, /* Divide by 160        */
    FALCON16_TSC_PLL_DIV_165        =          0x000000A5UL, /* Divide by 165        */
    FALCON16_TSC_PLL_DIV_168        =          0x000000A8UL, /* Divide by 168        */
    FALCON16_TSC_PLL_DIV_170        =          0x000000AAUL, /* Divide by 170        */
    FALCON16_TSC_PLL_DIV_175        =          0x000000AFUL, /* Divide by 175        */
    FALCON16_TSC_PLL_DIV_180        =          0x000000B4UL, /* Divide by 180        */
    FALCON16_TSC_PLL_DIV_184        =          0x000000B8UL, /* Divide by 184        */
    FALCON16_TSC_PLL_DIV_192        =          0x000000C0UL, /* Divide by 192        */
    FALCON16_TSC_PLL_DIV_198        =          0x000000C6UL, /* Divide by 198        */
    FALCON16_TSC_PLL_DIV_200        =          0x000000C8UL, /* Divide by 200        */
    FALCON16_TSC_PLL_DIV_224        =          0x000000E0UL, /* Divide by 224        */
    FALCON16_TSC_PLL_DIV_240        =          0x000000F0UL, /* Divide by 240        */
    FALCON16_TSC_PLL_DIV_264        =          0x00000108UL, /* Divide by 264        */
    FALCON16_TSC_PLL_DIV_280        =          0x00000118UL  /* Divide by 280        */
};

#endif
