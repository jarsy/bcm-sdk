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

/** @file merlin16_enum.h
 * Enum types used by Serdes API functions
 */

#ifndef MERLIN16_API_ENUM_H
#define MERLIN16_API_ENUM_H

#include "merlin16_ipconfig.h"

enum merlin16_pll_refclk_enum {
    MERLIN16_PLL_REFCLK_UNKNOWN = 0, /* Refclk value to be determined by API. */
    MERLIN16_PLL_REFCLK_50MHZ          = 0x00100032UL, /* 50 MHz          */
    MERLIN16_PLL_REFCLK_125MHZ         = 0x0010007DUL, /* 125 MHz         */
    MERLIN16_PLL_REFCLK_156P25MHZ      = 0x00400271UL, /* 156.25 MHz      */
    MERLIN16_PLL_REFCLK_161P1328125MHZ = 0x08005091UL  /* 161.1328125 MHz */
    };


enum merlin16_pll_div_enum {
    MERLIN16_PLL_DIV_UNKNOWN = 0, /* Divide value to be determined by API. */
    MERLIN16_PLL_DIV_52P751515  = (int32_t)0xC0635034UL, /* Divide by 52.751515  */
    MERLIN16_PLL_DIV_54P4       =          0x66666036UL, /* Divide by 54.4       */
    MERLIN16_PLL_DIV_58P181818  =          0x2E8BA03AUL, /* Divide by 58.181818  */
    MERLIN16_PLL_DIV_60         =          0x0000003CUL, /* Divide by 60         */
    MERLIN16_PLL_DIV_62P060606  =          0x0F83E03EUL, /* Divide by 62.060606  */
    MERLIN16_PLL_DIV_64         =          0x00000040UL, /* Divide by 64         */
    MERLIN16_PLL_DIV_66         =          0x00000042UL, /* Divide by 66         */
    MERLIN16_PLL_DIV_66P460703  =          0x75F0A042UL, /* Divide by 66.460703  */
    MERLIN16_PLL_DIV_66P743079  = (int32_t)0xBE3A7042UL, /* Divide by 66.743079  */
    MERLIN16_PLL_DIV_67P878788  = (int32_t)0xE0F84043UL, /* Divide by 67.878788  */
    MERLIN16_PLL_DIV_68         =          0x00000044UL, /* Divide by 68         */
    MERLIN16_PLL_DIV_68P537598  = (int32_t)0x89A00044UL, /* Divide by 68.537598  */
    MERLIN16_PLL_DIV_68P570764  = (int32_t)0x921D9044UL, /* Divide by 68.570764  */
    MERLIN16_PLL_DIV_68P828796  = (int32_t)0xD42C0044UL, /* Divide by 68.828796  */
    MERLIN16_PLL_DIV_68P856242  = (int32_t)0xDB32B044UL, /* Divide by 68.856242  */
    MERLIN16_PLL_DIV_69P152458  =          0x27078045UL, /* Divide by 69.152458  */
    MERLIN16_PLL_DIV_69P389964  =          0x63D4B045UL, /* Divide by 69.389964  */
    MERLIN16_PLL_DIV_69P818182  = (int32_t)0xD1746045UL, /* Divide by 69.818182  */
    MERLIN16_PLL_DIV_70         =          0x00000046UL, /* Divide by 70         */
    MERLIN16_PLL_DIV_70P713596  = (int32_t)0xB6AE4046UL, /* Divide by 70.713596  */
    MERLIN16_PLL_DIV_71P008     =          0x020C5047UL, /* Divide by 71.008     */
    MERLIN16_PLL_DIV_71P112952  =          0x1CEA7047UL, /* Divide by 71.P112952 */
    MERLIN16_PLL_DIV_71P31347   =          0x503F9047UL, /* Divide by 71.31347   */
    MERLIN16_PLL_DIV_71P5584    = (int32_t)0x8EF35047UL, /* Divide by 71.5584    */
    MERLIN16_PLL_DIV_72         =          0x00000048UL, /* Divide by 72         */
    MERLIN16_PLL_DIV_73P335232  =          0x55D1C049UL, /* Divide by 73.335232  */
    MERLIN16_PLL_DIV_73P6       = (int32_t)0x9999A049UL, /* Divide by 73.6       */
    MERLIN16_PLL_DIV_75         =          0x0000004BUL, /* Divide by 75         */
    MERLIN16_PLL_DIV_80         =          0x00000050UL, /* Divide by 80         */
    MERLIN16_PLL_DIV_82P5       = (int32_t)0x80000052UL, /* Divide by 82.5       */
    MERLIN16_PLL_DIV_85P671997  = (int32_t)0xAC080055UL, /* Divide by 85.671997  */
    MERLIN16_PLL_DIV_86P036     =          0x09375056UL, /* Divide by 86.036     */
    MERLIN16_PLL_DIV_87P5       = (int32_t)0x80000057UL, /* Divide by 87.5       */
    MERLIN16_PLL_DIV_88P392     =          0x645A2058UL, /* Divide by 88.392     */
    MERLIN16_PLL_DIV_88P76      = (int32_t)0xC28F6058UL, /* Divide by 88.76      */
    MERLIN16_PLL_DIV_89P141838  =          0x244F8059UL, /* Divide by 89.141838  */
    MERLIN16_PLL_DIV_89P447998  =          0x72B00059UL, /* Divide by 89.447998  */
    MERLIN16_PLL_DIV_90         =          0x0000005AUL, /* Divide by 90         */
    MERLIN16_PLL_DIV_91P669037  = (int32_t)0xAB46005BUL, /* Divide by 91.669037  */
    MERLIN16_PLL_DIV_92         =          0x0000005CUL, /* Divide by 92         */
    MERLIN16_PLL_DIV_100        =          0x00000064UL, /* Divide by 100        */
    MERLIN16_PLL_DIV_170        =          0x000000AAUL, /* Divide by 170        */
    MERLIN16_PLL_DIV_187P5      = (int32_t)0x800000BBUL, /* Divide by 187.5      */
    MERLIN16_PLL_DIV_200        =          0x000000C8UL, /* Divide by 200        */
    MERLIN16_PLL_DIV_206P25     =          0x400000CEUL  /* Divide by 206.25     */
};

#define NUM_MICROS 2

#endif
