/* $Id: jer_pp_kaps_diag.c, hagayco Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

/*******************************************************************************
*
* Copyright 2011-2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in an
* Authorized License, Broadcom grants no license (express or implied), right to
* use, or waiver of any kind with respect to the Software, and Broadcom expressly
* reserves all rights in and to the Software and all intellectual property rights
* therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
* TO THE SOFTWARE. BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
* OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
*
*******************************************************************************/
#include <soc/mcm/memregs.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_diag.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/JER/JER_PP/jer_pp_diag.h>
#define _ERR_MSG_MODULE_NAME BSL_SOC_TCAM

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

#include <soc/dpp/JER/JER_PP/jer_pp_kaps_diag.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_xpt.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_entry_mgmt.h>
#include <shared/swstate/access/sw_state_access.h>
#include <shared/swstate/sw_state.h>
#include <soc/dpp/QAX/QAX_PP/qax_pp_kaps_xpt.h>
#include <soc/dpp/JER/jer_sbusdma_desc.h>

#include <soc/shared/sand_signals.h>

#define PREFIX_LEN_8 (10)

#define GET_RAND_VAL(low, high)     (uint32_t) (low + ((float)(high - low  + 1) * (rand() /(RAND_MAX+1.0) )) )

/* BIST */
#define JER_KAPS_RPB_CAM_BIST_CONTROL_REG_ADDR  0x2A
#define JER_KAPS_RPB_CAM_BIST_STATUS_REG_ADDR   0x2B

static kbp_status write_bits_to_array(uint8_t* arr,
                                      uint32_t arr_sz,
                                      uint32_t end_pos,
                                      uint32_t st_pos,
                                      uint32_t value) {
    uint32_t startByteIdx;
    uint32_t endByteIdx;
    uint32_t byte;
    uint32_t len;
    uint8_t maskValue;

    if(end_pos < st_pos) {
        kbp_printf(" \n endbit cannot be less than startbit \n\n");
        exit(1);
    }
    if(end_pos >= (arr_sz << 3) ) {
        kbp_printf(" \n endBit exceeds the arr size \n\n");
        exit(1);
    }

    len = (end_pos - st_pos + 1);
    if(len > 32) {
        kbp_printf(" \n Can not write more than 32-bit at a time ! \n\n");
        exit(1);
    }

    /* Value is unsigned 32 bit variable, so it can not be greater than ~0.*/
    if(len != 32){
        if(value > ((uint32_t) (~(~0 << len))) )
            kbp_printf(" \n Value is to big to write in the bit field! \n\n");
    }

    startByteIdx = arr_sz - ((st_pos >> 3) + 1);
    endByteIdx = arr_sz- ((end_pos >> 3) + 1);

    if(startByteIdx == endByteIdx){
        maskValue = (uint8_t)(0xFE << ((end_pos  & 7)));
        maskValue |= ((1 << (st_pos  & 7)) - 1);
        arr[startByteIdx] &= maskValue;
        arr[startByteIdx] |= (uint8_t)((value << (st_pos  & 7)));
        return 0;
    }
    if(st_pos & 7) {
        maskValue = (uint8_t)((1 << (st_pos  & 7)) - 1);
        arr[startByteIdx] &= maskValue;
        arr[startByteIdx] |= (uint8_t)((value << (st_pos & 7)));
        startByteIdx--;
        value >>= (8 - (st_pos  & 7));
    }
    for(byte = startByteIdx; byte > endByteIdx; byte--){
        arr[byte] = (uint8_t)(value);
        value >>= 8;
    }
    maskValue = (uint8_t)(0xFE << ((end_pos & 7)));
    arr[byte] &= maskValue;
    arr[byte] |= value;

    return KBP_OK;
}

const char* JER_KAPS_TABLE_NAMES[] = {"CORE_0_PRIVATE_IPV4_UC",
                                      "CORE_1_PRIVATE_IPV4_UC",
                                      "CORE_0_PRIVATE_IPV6_UC",
                                      "CORE_1_PRIVATE_IPV6_UC",
                                      "CORE_0_PRIVATE_IPV4_MC",
                                      "CORE_0_PRIVATE_IPV6_MC",
                                      "CORE_0_PUBLIC_IPV4_UC ",
                                      "CORE_1_PUBLIC_IPV4_UC ",
                                      "CORE_0_PUBLIC_IPV6_UC ",
                                      "CORE_1_PUBLIC_IPV6_UC ",
                                      "CORE_0_PUBLIC_IPV4_MC ",
                                      "CORE_0_PUBLIC_IPV6_MC "};

/* Convert the data and mask to XY format */
static void kaps_convert_dm_to_xy(uint8_t * data_d, uint8_t * data_m, uint8_t * data_x, uint8_t * data_y, uint32_t dataLenInBytes)
{
    uint32_t i;
    for (i = 0; i < dataLenInBytes; i++) {
        data_x[i] = data_d[i] & (~data_m[i]);
        data_y[i] = (~data_d[i]) & (~data_m[i]);
    }
}

/* Comparing the returned ads data with ads data which we written into ADS block */
static int kaps_compare_ads(struct kaps_ads w_ads, struct kaps_ads r_ads)
{

    if (w_ads.bpm_ad != r_ads.bpm_ad)
        return 1;

    if (w_ads.bpm_len != r_ads.bpm_len)
        return 1;

    if (w_ads.row_offset != r_ads.row_offset)
        return 1;

    if (w_ads.key_shift != r_ads.key_shift)
        return 1;

    if (w_ads.bkt_row != r_ads.bkt_row)
        return 1;

    if (w_ads.format_map_00 != r_ads.format_map_00)
        return 1;

    if (w_ads.format_map_03 != r_ads.format_map_03)
        return 1;

    if (w_ads.format_map_07 != r_ads.format_map_07)
        return 1;

    if (w_ads.format_map_11 != r_ads.format_map_11)
        return 1;

    if (w_ads.format_map_15 != r_ads.format_map_15)
        return 1;

    return 0;
}

static void kaps_construct_data(uint8_t * data, uint32_t leninbytes)
{
    uint32_t iter = 0;

    kbp_memset(data, 0, leninbytes);
    for (iter = 0; iter < leninbytes; iter++)
        data[iter] = GET_RAND_VAL(0, 256);
}

kbp_status kaps_diag_01(struct kaps_xpt *xpt)
{
    uint8_t data[KAPS_RPB_WIDTH_8];
    uint8_t mask[KAPS_RPB_WIDTH_8];
    uint8_t blk_nr, row_nr, nrows, iter, bkt_nr, i;
    kbp_status status = KBP_OK;
    uint32_t register_data;
    uint8_t func;

    kbp_printf("\n    -- KAPS DIAG TOOL --    \n\n");

    /* Reset the KAPS Blocks */
    xpt->kaps_command(xpt, KAPS_CMD_EXTENDED, KAPS_FUNC15, 0, 0, 0, NULL);

    /* Enumerate the KAPS Blocks */
    xpt->kaps_command(xpt, KAPS_CMD_EXTENDED, KAPS_FUNC14, 0, 0, 0, NULL);

    for (blk_nr = 0; blk_nr < KAPS_NUM_RPB_BLOCKS; blk_nr++) {

        KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_READ, KAPS_FUNC0, KAPS_RPB_BLOCK_START + blk_nr, 0x00000002, sizeof(register_data), (uint8_t *) &register_data ));
        write_bits_to_array((uint8_t *) &register_data, sizeof(register_data), 2, 2, 1);
        KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, KAPS_FUNC0, KAPS_RPB_BLOCK_START + blk_nr, 0x00000002, sizeof(register_data), (uint8_t *) &register_data ));

        kbp_printf(" Write/Read to RPB Block #%d\n", blk_nr + 1);
        nrows = GET_RAND_VAL(0, KAPS_RPB_BLOCK_SIZE);
        for (iter = 0; iter < nrows; iter++) {

            struct kaps_write_rpb_rqt write_rpb_key0, write_rpb_key1;
            struct kaps_read_rpb_resp read_rpb_resp;
            uint8_t req_buffer[KAPS_RPB_WIDTH_8 + 1];
            uint8_t resp_buffer[KAPS_RPB_WIDTH_8 + 1];
            int32_t i;

            kbp_memset(mask, 0x00, KAPS_RPB_WIDTH_8);
            kbp_memset(data, 0x00, KAPS_RPB_WIDTH_8);
            row_nr = GET_RAND_VAL(0, KAPS_RPB_BLOCK_SIZE);
            kaps_construct_data(data, KAPS_RPB_WIDTH_8);

            /* Configure the write rpb request payload */
            func = KAPS_FUNC1;
            write_rpb_key0.is_valid = 0x3;
            write_rpb_key1.is_valid = 0x3;

            write_rpb_key0.search_lsn = 0;
            write_rpb_key1.search_lsn = 0;

            kaps_convert_dm_to_xy(data, mask, write_rpb_key0.key, write_rpb_key1.key, KAPS_RPB_WIDTH_8);

            for ( i = 0 ; i < KAPS_RPB_WIDTH_8; i++) {
                write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, (i * 8) + 7, (i * 8), write_rpb_key1.key[i]);
            }
            write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 163, 160, write_rpb_key1.search_lsn);
            write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 165, 164, write_rpb_key1.is_valid);
            write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 167, 166, 0);
            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_RPB_BLOCK_START + blk_nr, 2 * row_nr + 1, sizeof(write_rpb_key1), req_buffer));

            for ( i = 0 ; i < KAPS_RPB_WIDTH_8; i++) {
                write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, (i * 8) + 7, (i * 8), write_rpb_key0.key[i]);
            }
            write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 163, 160, write_rpb_key0.search_lsn);
            write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 165, 164, write_rpb_key0.is_valid);
            write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 167, 166, 0);
            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_RPB_BLOCK_START + blk_nr, 2 * row_nr, sizeof(write_rpb_key0), (uint8_t *) req_buffer));

            /* Configure the read rpb request payload */
            func = KAPS_FUNC1;

            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_READ, func, KAPS_RPB_BLOCK_START + blk_nr, 2 * row_nr, sizeof(read_rpb_resp),
                                   (uint8_t *) &resp_buffer));
            if (kbp_memcmp(req_buffer, resp_buffer, KAPS_RPB_WIDTH_8) != 0) {
                kbp_printf("\tMis Match in data or valid bit is not set at Row Number: %d for RPB Blk #%d\n", row_nr,
                           blk_nr);
                return 1;
            }
        }
        kbp_printf("\tVerified #%d Rows\n", nrows);
    }
    kbp_printf(" RPB Block Testing is Passed \n\n");

    for (blk_nr = 0; blk_nr < KAPS_NUM_ADS_BLOCKS; blk_nr++) {

        kbp_printf(" Write/Read to ADS Block #%d\n", blk_nr + 1);
        nrows = GET_RAND_VAL(0, KAPS_ADS_NUW_ROWS);
        for (iter = 0; iter < nrows; iter++) {

            struct kaps_ads write_ads;
            struct kaps_ads read_ads_resp;
            uint8_t ad_value[JER_KAPS_AD_WIDTH_IN_BYTES];
            row_nr = GET_RAND_VAL(0, KAPS_ADS_NUW_ROWS);

            /* Configure the write ads request payload */
            func = KAPS_FUNC4;
            write_ads.bkt_row = row_nr % KAPS_ADS_NUW_ROWS;
            write_ads.key_shift = 0;
            write_ads.row_offset = row_nr % KAPS_ADS_NUW_ROWS;
            write_ads.bpm_len = 0;
            write_ads.reserved = 0;
            write_ads.ecc = 0;

            kaps_construct_data(ad_value, JER_KAPS_AD_WIDTH_IN_BYTES);
            for (i = 0; i < JER_KAPS_AD_WIDTH_IN_BYTES; i++) {
                write_ads.bpm_ad = (write_ads.bpm_ad << 8) | ad_value[i];
            }

            write_ads.format_map_00 = GET_RAND_VAL(0, 12);
            write_ads.format_map_03 = GET_RAND_VAL(0, 12);
            write_ads.format_map_07 = GET_RAND_VAL(0, 12);
            write_ads.format_map_11 = GET_RAND_VAL(0, 12);
            write_ads.format_map_15 = GET_RAND_VAL(0, 12);

            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_RPB_BLOCK_START + blk_nr, row_nr, sizeof(write_ads), (uint8_t *) &write_ads));

            /* Configure the read ads request payload */
            func = KAPS_FUNC4;

            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_READ, func, KAPS_RPB_BLOCK_START + blk_nr, row_nr, sizeof(read_ads_resp),
                                   (uint8_t *) &read_ads_resp));
            if (kaps_compare_ads(write_ads, read_ads_resp) != 0) {
                kbp_printf("Mis Match in data at Row Number: %d for ADS Blk Number: %d\n", row_nr, blk_nr);
                return 1;
            }
        }
        kbp_printf("\tVerified #%d Rows\n", nrows);
    }
    kbp_printf(" ADS Block Testing is Passed \n\n");

    for (bkt_nr = 0; bkt_nr < KAPS_MIN_NUM_BB; bkt_nr++) {

        kbp_printf(" Write/Read to Bucket Block #%d\n", bkt_nr + 1);
        nrows = GET_RAND_VAL(0, KAPS_BKT_NUM_ROWS);
        for (iter = 0; iter < nrows; iter++) {

            struct kaps_bkt_rqt write_bkt;
            struct kaps_bkt_rqt read_bkt_resp;

            row_nr = GET_RAND_VAL(0, KAPS_BKT_NUM_ROWS);

            /* Configure the write bucket block request payload */
            func = KAPS_FUNC2;
            kaps_construct_data(write_bkt.data, KAPS_BKT_WIDTH_8);

            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_BB_START + bkt_nr, row_nr, sizeof(write_bkt), (uint8_t *) &write_bkt));

            /* Configure the read bucket block request payload */
            func = KAPS_FUNC2;

            KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_READ, func, KAPS_BB_START + bkt_nr, row_nr, sizeof(read_bkt_resp),
                                   (uint8_t *) &read_bkt_resp));
            if (kbp_memcmp(write_bkt.data, read_bkt_resp.data, KAPS_BKT_WIDTH_8) != 0) {
                kbp_printf("Mis Match in data at Row Number: %d for BB Number: %d\n", row_nr, bkt_nr);
                return 1;
            }
        }
        kbp_printf("\tVerified #%d Rows\n", nrows);
    }
    kbp_printf(" Bucket Blocks Testing is Passed \n");
    return status;
}

static void kaps_print_entry(uint8_t *data, uint32_t LenInBytes)
{
    int32_t i;

    for (i = 0;i < LenInBytes; i++)
        kbp_printf("%02x ",data[i]);
    kbp_printf("\n");

}

static kbp_status kaps_write_to_rpb(struct kaps_xpt *xpt, uint8_t *search_key, uint32_t blk_num, uint32_t row_num, uint32_t rpb_key_len_8)
{
    uint8_t rpb_data[KAPS_RPB_WIDTH_8] = { 0x00, };
    uint8_t mask[KAPS_RPB_WIDTH_8];
    uint8_t req_buffer[KAPS_RPB_WIDTH_8 + 1];
    uint8_t  i;
    struct kaps_write_rpb_rqt write_rpb_key0, write_rpb_key1;
    kbp_status status = KBP_OK;
    uint8_t func;

    for (i = 0; i < rpb_key_len_8; i++) {
        rpb_data[i] = search_key[i];
        mask[i] = 0x00;
    }

    for (i = rpb_key_len_8; i < KAPS_RPB_WIDTH_8; i++) {
        mask[i] = rpb_data[i] = 0xFF;
    }

    func = KAPS_FUNC1;
    write_rpb_key0.is_valid = 0x3;
    write_rpb_key1.is_valid = 0x3;

    write_rpb_key0.search_lsn = 0;
    write_rpb_key1.search_lsn = 0;

    kaps_convert_dm_to_xy(rpb_data, mask, write_rpb_key0.key, write_rpb_key1.key, KAPS_RPB_WIDTH_8);

    for ( i = 0 ; i < KAPS_RPB_WIDTH_8; i++) {
        write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, ( (KAPS_RPB_WIDTH_8 - i) * 8) + 5, ((KAPS_RPB_WIDTH_8 - i) * 8) - 2, write_rpb_key1.key[i]);
    }
    write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 5, 2, write_rpb_key1.search_lsn);
    write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 1, 0, write_rpb_key1.is_valid);
    write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 167, 166, 0);

    KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_RPB_BLOCK_START + blk_num, 2 * row_num + 1, sizeof(write_rpb_key1), req_buffer));

    for ( i = 0 ; i < KAPS_RPB_WIDTH_8; i++) {
        write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, ( (KAPS_RPB_WIDTH_8 - i) * 8) + 5, ((KAPS_RPB_WIDTH_8 - i) * 8) - 2, write_rpb_key0.key[i]);
    }
    write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 5, 2, write_rpb_key0.search_lsn);
    write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 1, 0, write_rpb_key0.is_valid);
    write_bits_to_array(req_buffer,KAPS_RPB_WIDTH_8 + 1, 167, 166, 0);
    KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_RPB_BLOCK_START + blk_num, 2 * row_num, sizeof(write_rpb_key0), req_buffer));
    return status;

}

static kbp_status kaps_write_to_ads(struct kaps_xpt *xpt,struct kaps_ads ads, uint32_t blk_num, uint32_t row_num)
{

    kbp_status status = KBP_OK;
    uint8_t req_buffer[16];

    write_bits_to_array(req_buffer, 16, 19,0, ads.bpm_ad);
    write_bits_to_array(req_buffer, 16, 27, 20, ads.bpm_len);
    write_bits_to_array(req_buffer, 16, 31, 28, ads.row_offset);

    write_bits_to_array(req_buffer, 16, 35, 32, ads.format_map_00);
    write_bits_to_array(req_buffer, 16, 39, 36, ads.format_map_01);
    write_bits_to_array(req_buffer, 16, 43, 40, ads.format_map_02);
    write_bits_to_array(req_buffer, 16, 47, 44, ads.format_map_03);
    write_bits_to_array(req_buffer, 16, 51, 48, ads.format_map_04);
    write_bits_to_array(req_buffer, 16, 55, 52, ads.format_map_05);
    write_bits_to_array(req_buffer, 16, 59, 56, ads.format_map_06);
    write_bits_to_array(req_buffer, 16, 63, 60, ads.format_map_07);
    write_bits_to_array(req_buffer, 16, 67, 64, ads.format_map_08);
    write_bits_to_array(req_buffer, 16, 71, 68, ads.format_map_09);
    write_bits_to_array(req_buffer, 16, 75, 72, ads.format_map_10);
    write_bits_to_array(req_buffer, 16, 79, 76, ads.format_map_11);
    write_bits_to_array(req_buffer, 16, 83, 80, ads.format_map_12);
    write_bits_to_array(req_buffer, 16, 87, 84, ads.format_map_13);
    write_bits_to_array(req_buffer, 16, 91, 88, ads.format_map_14);
    write_bits_to_array(req_buffer, 16, 95, 92, ads.format_map_15);

    write_bits_to_array(req_buffer, 16, 106, 96, ads.bkt_row);
    write_bits_to_array(req_buffer, 16, 111, 107, ads.reserved);
    write_bits_to_array(req_buffer, 16, 119, 112, ads.key_shift);
    write_bits_to_array(req_buffer, 16, 127, 120, ads.ecc);

    status = xpt->kaps_command(xpt, KAPS_CMD_WRITE, KAPS_FUNC4, KAPS_RPB_BLOCK_START + blk_num, row_num, sizeof(ads), (uint8_t *) &req_buffer);
    return status;
}

static kbp_status kaps_write_to_bb(struct kaps_xpt *xpt, uint8_t *bktdata,uint32_t bkt_num, uint32_t bkt_row_num)
{
    struct kaps_bkt_rqt write_bkt;
    kbp_status status = KBP_OK;
    uint8_t func;

    func = KAPS_FUNC2;

    kbp_memset(write_bkt.data, 0, KAPS_BKT_WIDTH_8);
    kbp_memcpy(write_bkt.data, bktdata, KAPS_BKT_WIDTH_8);

    status = xpt->kaps_command(xpt, KAPS_CMD_WRITE, func, KAPS_BB_START + bkt_num * 2, bkt_row_num, sizeof(write_bkt), (uint8_t *) &write_bkt);
    return status;

}

kbp_status kaps_diag_02(struct kaps_xpt *xpt)
{
    uint8_t srch_data[PREFIX_LEN_8] = { 0x00, };
 /*   uint8_t rpb_data[21] = { 0x00, };
    uint8_t rpb_data_1[21] = { 0x00, };*/
    uint8_t ad_value_1[JER_KAPS_AD_WIDTH_IN_BYTES];
    uint8_t ad_value_2[JER_KAPS_AD_WIDTH_IN_BYTES];
    uint8_t bktdata[KAPS_BKT_WIDTH_8] = {0x00, };
    uint32_t ad_data = 0, rpb_key_len_8 = 0, pfx_len_8 = 0;
    uint32_t bkt_len_8 = 0, i;
    uint32_t rpb_blk_num, rpb_row_num;
    uint32_t bkt_num, bkt_row_num;
    uint32_t register_data = 0;
    kbp_status status = KBP_OK;

    struct kaps_search_result kaps_result;
    struct kaps_ads ads1, ads2;

    /* Reset the KAPS Blocks */
    xpt->kaps_command(xpt, KAPS_CMD_EXTENDED, KAPS_FUNC15, 0, 0, 0, NULL);

    /* Enumerate the KAPS Blocks */
    xpt->kaps_command(xpt, KAPS_CMD_EXTENDED, KAPS_FUNC14, 0, 0, 0, NULL);

    pfx_len_8 = PREFIX_LEN_8;
    kbp_memset(&ads1 , 0, sizeof(struct kaps_ads));
    kbp_memset(&ads2 , 0, sizeof(struct kaps_ads));
    kaps_construct_data(srch_data, PREFIX_LEN_8);
    kaps_construct_data(ad_value_1, JER_KAPS_AD_WIDTH_IN_BYTES);
    kaps_construct_data(ad_value_2, JER_KAPS_AD_WIDTH_IN_BYTES);

    kbp_printf("\n -- Search - 1 -- \n\n");
    rpb_blk_num = 0;
    rpb_row_num = 1;
    bkt_num = 0;
    bkt_row_num = 2;
    rpb_key_len_8 = 7;
    bkt_len_8 = pfx_len_8 - rpb_key_len_8;

    kbp_printf(" RPB Block Number: %d RPB Row Number: %d\n", rpb_blk_num, rpb_row_num);
    kbp_printf(" Bucket Num: %d Bucket Row Num: %d\n", bkt_num, bkt_row_num);

    KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_READ, KAPS_FUNC0, KAPS_RPB_BLOCK_START + rpb_blk_num, 0x00000002, sizeof(register_data), (uint8_t *) &register_data ));
    write_bits_to_array((uint8_t *) &register_data, sizeof(register_data), 2, 2, 1);
    KBP_STRY(xpt->kaps_command(xpt, KAPS_CMD_WRITE, KAPS_FUNC0, KAPS_RPB_BLOCK_START + rpb_blk_num, 0x00000002, sizeof(register_data), (uint8_t *) &register_data ));

    /* write to RPB Block */
    kaps_write_to_rpb(xpt, srch_data, rpb_blk_num, rpb_row_num, rpb_key_len_8);

    /* Fill ADS */
    ads1.bkt_row = bkt_row_num;
    ads1.key_shift = rpb_key_len_8 * 8;
    ads1.row_offset = bkt_num;
    ads1.bpm_len = 0;
    ads1.bpm_ad = 0;

    ads1.format_map_00 = bkt_len_8 + 1;

    /* Write to ADS Block */
    kaps_write_to_ads(xpt, ads1,rpb_blk_num, rpb_row_num);

    register_data = 0;
    write_bits_to_array((uint8_t *) &register_data, sizeof(register_data), 1, 0, rpb_blk_num);
    xpt->kaps_command(xpt, KAPS_CMD_WRITE, KAPS_FUNC0, bkt_num + KAPS_BB_START, 0x00000021, sizeof(register_data), (uint8_t *)&register_data);

    /* construct Bucket Block data */
    for (i = 0; i < bkt_len_8; i++) {
        bktdata[i] = srch_data[rpb_key_len_8 + i];
    }

    /* Writing some part of key in LSN(Bucket Block)*/
    if (bkt_len_8) {
        bktdata[bkt_len_8] = 0x80;
        kbp_memcpy(&bktdata[KAPS_BKT_WIDTH_8 - JER_KAPS_AD_WIDTH_IN_BYTES], ad_value_1, JER_KAPS_AD_WIDTH_IN_BYTES);
    }

    /* Write to Bucket Block */
    kaps_write_to_bb(xpt, bktdata, bkt_num, bkt_row_num);

    for (i = 0;i < JER_KAPS_AD_WIDTH_IN_BYTES; i++)
        ad_data = (ad_data << 8) | ad_value_1[i];

    ad_data = ad_data << 4;
    ad_value_1[0] = (ad_data & 0xFF0000) >> 16;
    ad_value_1[1] = (ad_data & 0x00FF00) >> 8;
    ad_value_1[2] = (ad_data & 0x0000FF);

    kbp_printf(" Search Data: ");
    kaps_print_entry(srch_data, PREFIX_LEN_8);
    KBP_STRY(xpt->kaps_search(xpt, srch_data, rpb_blk_num, &kaps_result));

    if (kaps_result.match_len != PREFIX_LEN_8 * 8) {
        kbp_printf("\n Mis Match in match len Exp: %d, Got: %d\n", PREFIX_LEN_8 * 8, kaps_result.match_len);
        return KBP_MISMATCH;
    } else {
        kbp_printf("\n Match len = %d\n", kaps_result.match_len);
    }

    if (kbp_memcmp(ad_value_1, kaps_result.ad_value, JER_KAPS_AD_WIDTH_IN_BYTES) != 0) {
        kbp_printf("\n Mis Match in AD Value \n");
        kbp_printf("\n Exp =  ");
        kaps_print_entry(ad_value_1, JER_KAPS_AD_WIDTH_IN_BYTES);
        kbp_printf("\n Got =  ");
        kaps_print_entry(kaps_result.ad_value, JER_KAPS_AD_WIDTH_IN_BYTES);
        return KBP_MISMATCH;
    } else {
        kbp_printf("\n AD Value =  ");
        kaps_print_entry(ad_value_1, JER_KAPS_AD_WIDTH_IN_BYTES);
    }

    /* Excercising Lmpsofar search, not storing any part of key in LSN(bucket block) */
    kbp_printf("\n -- Search - 2 -- \n\n");
    kaps_construct_data(&srch_data[PREFIX_LEN_8 / 2], PREFIX_LEN_8 / 2);
    ad_data = 0;
    rpb_blk_num = 0;
    rpb_row_num = 2;
    bkt_num = 0;
    bkt_row_num = 3;
    rpb_key_len_8 = 10;
    bkt_len_8 = pfx_len_8 - rpb_key_len_8;
    kbp_memset(&kaps_result, 0, sizeof(struct kaps_search_result));
    kbp_memset(bktdata, 0, KAPS_BKT_WIDTH_8);

    kbp_printf(" RPB Block Number: %d RPB Row Number: %d\n", rpb_blk_num, rpb_row_num);
    kbp_printf(" Bucket Num: %d Bucket Row Num: %d\n", bkt_num, bkt_row_num);

    /* write to RPB Block */
    kaps_write_to_rpb(xpt, srch_data, rpb_blk_num, rpb_row_num, rpb_key_len_8);

    /* Fill ADS */
    ads2.bkt_row = bkt_row_num;
    ads2.key_shift = rpb_key_len_8 * 8;
    ads2.row_offset = bkt_num;
    ads2.bpm_len = 0;
    ads2.bpm_ad = 0;

    for (i = 0; i < JER_KAPS_AD_WIDTH_IN_BYTES; i++) {
        ads2.bpm_ad = (ads2.bpm_ad << 8) | ad_value_2[i] ;
    }
    ads2.bpm_len = pfx_len_8 * 8;
    ads2.format_map_00 = 0/*bkt_len_8 + 1*/;

    /* Write to ADS Block */
    kaps_write_to_ads(xpt, ads2,rpb_blk_num, rpb_row_num);

    register_data = 0;
    write_bits_to_array((uint8_t *) &register_data, sizeof(register_data), 1, 0, rpb_blk_num);
    xpt->kaps_command(xpt, KAPS_CMD_WRITE, KAPS_FUNC0, bkt_num + KAPS_BB_START, 0x00000021, sizeof(register_data), (uint8_t *)&register_data);


    /* Write to Bucket Block */
   /* kaps_write_to_bb(xpt, bktdata, bkt_num, bkt_row_num);*/

    for (i = 0;i < JER_KAPS_AD_WIDTH_IN_BYTES; i++)
        ad_data = (ad_data << 8) | ad_value_2[i];

    ad_data = ad_data << 4;
    ad_value_2[0] = (ad_data & 0xFF0000) >> 16;
    ad_value_2[1] = (ad_data & 0x00FF00) >> 8;
    ad_value_2[2] = (ad_data & 0x0000FF);

    kbp_printf(" Search Data: ");
    kaps_print_entry(srch_data, PREFIX_LEN_8);
    KBP_STRY(xpt->kaps_search(xpt, srch_data, rpb_blk_num, &kaps_result));

    if (kaps_result.match_len != PREFIX_LEN_8 * 8) {
        kbp_printf("\n Mis Match in match len Exp: %d, Got: %d\n", PREFIX_LEN_8 * 8, kaps_result.match_len);
        return KBP_MISMATCH;
    } else {
        kbp_printf("\n Match len = %d\n", kaps_result.match_len);
    }

    if (kbp_memcmp(ad_value_2, kaps_result.ad_value, JER_KAPS_AD_WIDTH_IN_BYTES) != 0) {
        kbp_printf("\n Mis Match in AD Value \n");
        kbp_printf("\n Exp =  ");
        kaps_print_entry(ad_value_2, JER_KAPS_AD_WIDTH_IN_BYTES);
        kbp_printf("\n Got =  ");
        kaps_print_entry(kaps_result.ad_value, JER_KAPS_AD_WIDTH_IN_BYTES);
        return KBP_MISMATCH;
    } else {
        kbp_printf("\n AD Value =  ");
        kaps_print_entry(ad_value_2, JER_KAPS_AD_WIDTH_IN_BYTES);
    }

    kbp_memset(bktdata, 0, KAPS_BKT_WIDTH_8);
    kaps_write_to_bb(xpt, bktdata, bkt_num, bkt_row_num);
    return status;
}

soc_error_t soc_jer_pp_diag_kaps_lkup_info_get(
   SOC_SAND_IN int unit,
   SOC_SAND_IN int core_id
   )
{
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;
    int flp_prog_id, num_of_flp_progs;
    int flp_invoked = 0;
    uint32 flp_lpm_lookups[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];
    uint32 rv = SOC_SAND_OK;
    uint32 dbal_table_id = 0;
    SOC_DPP_DBAL_TABLE_INFO dbal_table;
    /*0:RPF private, 1:RPF public, 2:FWD private, 3:FWD public */
    uint32 search_keys[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES][BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES)];
    uint8 search_keys_8[JER_KAPS_KEY_BUFFER_NOF_BYTES];
    uint32 ad_array[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];
    uint8 ad_array_8[(BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS))];
    uint32 status[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];
    uint32 prio_len[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];

    uint32 i,j;

    char* search_names[] = {"KAPS First Private (default: RPF vrf!=0)",
                            "KAPS First Public (default: RPF vrf==0)",
                            "KAPS Second Private (default: FWD vrf!=0)",
                            "KAPS Second Public (default: FWD vrf==0)"
                            };

    SOCDNX_INIT_FUNC_DEFS;

    sal_memset(flp_lpm_lookups, 0, sizeof(uint32) * JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES);

    for (i=0; i<JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES; i++) {
        for (j=0; j<BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES); j++) {
            search_keys[i][j] = 0;
        }
        ad_array[i] = 0;
        status[i] = 0;
    }

    /*first : RPF, second : forwarding, 0 : private, 1 : public*/
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "FLP", "LPM", "LPM_1st_A_Lookup_Key", search_keys[0], BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES)));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "FLP", "LPM", "LPM_1st_B_Lookup_Key", search_keys[1], BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES)));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "FLP", "LPM", "LPM_2nd_A_Lookup_Key", search_keys[2], BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES)));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "FLP", "LPM", "LPM_2nd_B_Lookup_Key", search_keys[3], BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES)));

    /* Payload */
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_1st_A_Lookup_Result", &ad_array[0], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_1st_B_Lookup_Result", &ad_array[1], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_2nd_A_Lookup_Result", &ad_array[2], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_2nd_B_Lookup_Result", &ad_array[3], 1));

    /*status*/
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_1st_A_Lookup_Status", &status[0], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_1st_B_Lookup_Status", &status[1], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_2nd_A_Lookup_Status", &status[2], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_2nd_B_Lookup_Status", &status[3], 1));

    /* length */
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_1st_A_Lookup_Length", &prio_len[0], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_1st_B_Lookup_Length", &prio_len[1], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_2nd_A_Lookup_Length", &prio_len[2], 1));
    SOCDNX_SAND_IF_ERR_EXIT(dpp_dsig_read(unit, core_id, "IRPP", "LPM", "FLP", "LPM_2nd_B_Lookup_Length", &prio_len[3], 1));

    /* Get the latest FLP program used in order to print only relevant lookup items */
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_flp_access_print_last_programs_data(unit, core_id, 1/*to_print*/, &flp_prog_id, &num_of_flp_progs));

    /* If an FLP program was invoked, get relevant LPM lookups */
    if (num_of_flp_progs != 0 ) {
        rv = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_prog_id, &flp_lookups_tbl);
        SOCDNX_SAND_IF_ERR_EXIT(rv);

        flp_lpm_lookups[0] = flp_lookups_tbl.lpm_1st_lkp_valid;
        flp_lpm_lookups[1] = flp_lookups_tbl.lpm_public_1st_lkp_valid;
        flp_lpm_lookups[2] = flp_lookups_tbl.lpm_2nd_lkp_valid;
        flp_lpm_lookups[3] = flp_lookups_tbl.lpm_public_2nd_lkp_valid;
        flp_invoked = 1;
    }
    cli_out("\n");

    /*Print all of the keys and AD*/
    for (i=0; i < JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES; i++) {
        /* If we cannot detect the previously invoked FLP program, we print all entries */
        if (flp_invoked == 0) {
            cli_out("Last invoked FLP Program was not detected, printing all KAPS searches. \n");
        } else if ((flp_lpm_lookups[0]==0) && (flp_lpm_lookups[1]==0) && (flp_lpm_lookups[2]==0) && (flp_lpm_lookups[3]==0)) {
            cli_out("Last invoked FLP Program did not include KAPS (LPM) searches.\n");
            break;
        }

        /* If current LPM lookup is not valid but last invoked FLP program was detected, skip to next entry */
        if (flp_lpm_lookups[i] == 0 && flp_invoked == 1) {
            continue;
        }

        cli_out("%s: \n", search_names[i]);

        for (j=0; j<JER_KAPS_KEY_BUFFER_NOF_BYTES; j++) {
            search_keys_8[j] = (search_keys[i][BYTES2WORDS(JER_KAPS_KEY_BUFFER_NOF_BYTES)-1-j/WORDS2BYTES(1)] >> BYTES2BITS(WORDS2BYTES(1) - (j % WORDS2BYTES(1)) - 1)) & SOC_SAND_U8_MAX;
        }

        ad_array_8[0] = (ad_array[i] >> 12) & SOC_SAND_U8_MAX;
        ad_array_8[1] = (ad_array[i] >> 4) & SOC_SAND_U8_MAX;
        ad_array_8[2] = (ad_array[i] << 4) & SOC_SAND_U8_MAX;

        for (dbal_table_id = 0; dbal_table_id < SOC_DPP_DBAL_SW_NOF_TABLES; dbal_table_id++) {
            SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, dbal_table_id, &dbal_table));

            if ((dbal_table.physical_db_type == SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS) && ((search_keys_8[0] >> (SOC_SAND_NOF_BITS_IN_BYTE - dbal_table.db_prefix_len)) == dbal_table.db_prefix)) {
                break;
            }
        }

        SOCDNX_IF_ERR_EXIT(jer_kaps_parse_print_entry(unit, dbal_table_id, prio_len[i], search_keys_8, ad_array_8, 1 /*header_flag*/));
    }

exit:
  SOCDNX_FUNC_RETURN
}

/*
 *  Test functions
 *  return_cmp_rslt - only updates hit_or_miss and assoc_data
 */
int jer_kaps_search_generic(int unit, uint32 search_id, uint8 key[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES][JER_KAPS_KEY_BUFFER_NOF_BYTES],
                            uint32 *return_is_matched, uint32 *return_prefix_len, uint8  return_payload[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES][BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS)],
                            struct kbp_search_result *return_cmp_rslt)
{
    uint32 rv;
    uint32 soc_sand_rv;
    struct kbp_search_result cmp_rslt, *cmp_rslt_p;
    int32 i, j=0, is_matched[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES], prefix_len[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];
    uint8 payload[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES][BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS)];
    JER_KAPS_SEARCH_CONFIG search_cfg, search_cfg_get_tbl;
    JER_KAPS_IP_TBL_ID *table_id;
    uint32 key_offset;
    uint8 master_key[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES * JER_KAPS_KEY_BUFFER_NOF_BYTES];

    jer_kaps_search_config_get(unit, search_id, &search_cfg_get_tbl);
    table_id = search_cfg_get_tbl.tbl_id;
    memset(&cmp_rslt, 0, sizeof(cmp_rslt));
    memset(&master_key, 0, sizeof(uint8) * JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES * JER_KAPS_KEY_BUFFER_NOF_BYTES);

    jer_kaps_search_config_get(unit, j, &search_cfg);

    for(i = 0; i<search_cfg.valid_tables_num; i++) {
        key_offset = (i * search_cfg.max_tables_num) / search_cfg.valid_tables_num;
        is_matched[i] = 0;
        prefix_len[i] = 0;

        /*copy the key*/
        for (j=0; j < JER_KAPS_KEY_BUFFER_NOF_BYTES; j++){
            master_key[key_offset*JER_KAPS_KEY_BUFFER_NOF_BYTES + j] = key[i][j];
        }
        /*zero the payload*/
        for (j=0; j < BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS); j++){
            payload[i][j] = 0;
        }
    }

    cli_out("\n\rSW search\n\r---------\n\r");

    for(i = 0; i<search_cfg.valid_tables_num; i++) {
        soc_sand_rv = jer_kaps_sw_search_test(unit, table_id[i], key[i], payload[i], &prefix_len[i], &is_matched[i]);
        rv = handle_sand_result(soc_sand_rv);
        if (BCM_FAILURE(rv)) {
            cli_out("Error: jer_kaps_sw_search_test(%d)\n", unit);
            return rv;
        }

        if (return_is_matched != NULL) {
            return_is_matched[i] = is_matched[i];
        }
        if (return_prefix_len != NULL) {
            return_prefix_len[i] = prefix_len[i];
        }
        if (return_payload != NULL) {
            for (j = 0; j < BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS); j++) {
                return_payload[i][j] =  payload[i][j];
            }
        }

        if (is_matched[i]) {
            cli_out("SW search %d in %s: matched!, payload:0x", i, JER_KAPS_TABLE_NAMES[table_id[i]]);
            for (j = 0; j < BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS-7); j++) {
                cli_out("%02x", payload[i][j]);
            }
            /*print the last 4 bits only*/
            cli_out("%x", payload[i][j] >> (BYTES2BITS(BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS))-JER_KAPS_AD_WIDTH_IN_BITS));
            cli_out(", prefix_len:%d\n", prefix_len[i]);
        }
        else {
            cli_out("SW search %d in %s: no match\n", i, JER_KAPS_TABLE_NAMES[table_id[i]]);
        }
    }

    cli_out("\n\rHW search\n\r---------\n\r");

    /* If we return the hw compare result, use the passed struct instead */
    if (return_cmp_rslt != NULL) {
        cmp_rslt_p = return_cmp_rslt;
    } else {
        cmp_rslt_p = &cmp_rslt;
    }

    soc_sand_rv = jer_kaps_hw_search_test(unit, search_id, master_key, cmp_rslt_p);
    rv = handle_sand_result(soc_sand_rv);
    if (BCM_FAILURE(rv)) {
        cli_out("Error: jer_kaps_hw_search_test(%d)\n", unit);
        return rv;
    }

    for(i = 0; i<search_cfg.valid_tables_num; i++) {
        key_offset = (i * search_cfg.max_tables_num) / search_cfg.valid_tables_num;
        /* Return the result in the order we expect */
        if ((key_offset != i) && (return_cmp_rslt != NULL)) {
            sal_memcpy(return_cmp_rslt->assoc_data[i], cmp_rslt_p->assoc_data[key_offset], sizeof(uint8) * KBP_INSTRUCTION_MAX_AD_BYTES);
            return_cmp_rslt->hit_or_miss[i] = cmp_rslt_p->hit_or_miss[key_offset];
        }
        if (cmp_rslt_p->hit_or_miss[key_offset]) {
            cli_out("HW search %d in %s: matched!, payload:0x", i, JER_KAPS_TABLE_NAMES[table_id[i]]);
            for (j = 0; j < BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS-7); j++) {
                cli_out("%02x", cmp_rslt_p->assoc_data[key_offset][j]);
            }
            /*print the last 4 bits only*/
            cli_out("%x", cmp_rslt_p->assoc_data[key_offset][j] >> (BYTES2BITS(BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS))-JER_KAPS_AD_WIDTH_IN_BITS));
            cli_out("\n");
        }
        else {
            cli_out("HW search %d in %s: no match\n", i, JER_KAPS_TABLE_NAMES[table_id[i]]);
        }
    }

    return rv;
}

int jer_kaps_sw_search_test(int unit, uint32 tbl_id, uint8 *key, uint8 *payload, int32 *prefix_len, int32 *is_matched)
{
    struct kbp_ad_db *ad_db_p;
    struct kbp_entry *entry_p = NULL;
    struct kbp_ad *ad_entry_p = NULL;
    int32 index, clone_db, res;
    JER_KAPS_TABLE_CONFIG table_cfg_p;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    jer_kaps_table_config_get(unit, tbl_id, &table_cfg_p);

    clone_db = jer_kaps_clone_of_db_id_get(unit, table_cfg_p.db_id);

    if (clone_db == JER_KAPS_IP_NOF_DB) {
        /* not a clone */
        jer_kaps_ad_db_by_db_id_get(unit, table_cfg_p.db_id, &ad_db_p);
    }
    else {
        /* DB is a clone. Get AD DB of the original DB. */
        jer_kaps_ad_db_by_db_id_get(unit, clone_db, &ad_db_p);
    }

    res = kbp_db_search(table_cfg_p.tbl_p, key, &entry_p, &index, prefix_len);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_db_search failed with : %s!\n"),
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));

        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

    if (entry_p) {
        *is_matched = TRUE;
        kbp_entry_get_ad(table_cfg_p.tbl_p, entry_p, &ad_entry_p);
        kbp_ad_db_get(ad_db_p, ad_entry_p, payload);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_kaps_search_test()", 0, 0);
}

int jer_kaps_hw_search_test(int unit, uint32 search_id, uint8 *master_key, struct kbp_search_result *cmp_rslt)
{
    int32 res;
    JER_KAPS_SEARCH_CONFIG search_cfg;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    jer_kaps_search_config_get(unit, search_id, &search_cfg);
    res = kbp_instruction_search(search_cfg.inst_p, master_key, 0, cmp_rslt);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_instruction_search failed with : %s!\n"),
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));

        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_kaps_search_test()", 0, 0);
}

int jer_kaps_parse_print_entry(int unit, uint32 dbal_table_id, uint32 prio_len, uint8 *data_bytes, uint8 *ad_8, uint8 header_flag) {
    uint32 res;
    uint32 qual_print_len = 0;
    uint32 i;
    uint32 temp_prio_len;

    uint32 qual_prio_len;
    uint32 qual_max_size; /* The maximal number of bits of a qual in a key */
    uint32 temp_zero_padding, zero_padding; /*used to calculate the number of bit padding added to key */
    uint32 qual_tbl_index, qual_type_tbl_index = 0;
    SOC_DPP_DBAL_TABLE_INFO dbal_table;

    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    char qual_name[20];

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, dbal_table_id, &dbal_table);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_dbal_kbp_buffer_to_entry_key(unit, &dbal_table, prio_len, data_bytes, qual_vals);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /*cli_out(header_flag ? "|Prefix_len:%03d " : "|           %03d ", prio_len);*/
    cli_out(header_flag? "|Raw: 0x" : "|     0x");
    for (i = 0; i < JER_KAPS_KEY_BUFFER_NOF_BYTES; i++) {
        cli_out("%02x", data_bytes[i]);
    }
    cli_out("/%03d", prio_len);
    cli_out(header_flag ? " |TBL:0x%*x/%01d/%01d" : " |    0x%*x/%01d/%01d", (dbal_table.db_prefix_len + 3)/4, dbal_table.db_prefix, dbal_table.db_prefix_len, dbal_table.db_prefix_len);

    prio_len -= dbal_table.db_prefix_len;
    /* Remove the zero padding from the prio_len */
    temp_prio_len = prio_len;
    zero_padding = 0;
    temp_zero_padding = 0;
    qual_tbl_index = dbal_table.nof_qualifiers - 1; /* quals are listed in the table qual_info from LSB to MSB*/
    while (qual_tbl_index != -1) {
        if (dbal_table.qual_info[qual_tbl_index].qual_type == SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES) {
             temp_zero_padding += dbal_table.qual_info[qual_tbl_index].qual_nof_bits;
        } else { /* Only count the zeros within the key */
            if (temp_prio_len > 0 && temp_prio_len <= prio_len) {
                zero_padding += temp_zero_padding;
                temp_zero_padding = 0;
            } else {
                break;
            }
        }
        temp_prio_len -= dbal_table.qual_info[qual_tbl_index].qual_nof_bits;

        qual_tbl_index--;
    }
    prio_len -= zero_padding;

    for (i=0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
        if (((qual_vals[i].type != SOC_PPC_NOF_FP_QUAL_TYPES) && (qual_vals[i].type != SOC_PPC_FP_QUAL_IRPP_ALL_ONES)
            && (qual_vals[i].type != SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES)) && (qual_vals[i].type != BCM_FIELD_ENTRY_INVALID)) { /*Print all fields except ones/zeroes*/

            if (qual_vals[i].type > SOC_PPC_NOF_FP_QUAL_TYPES) {
                cli_out("\nUnexpected qual type!\n");
                ARAD_DO_NOTHING_AND_EXIT;
            }

            switch (qual_vals[i].type) {/*Pretty print basic common qualifiers - shorter than SOC_PPC_FP_QUAL_TYPE_to_string*/
            case SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP:
                sal_strcpy(qual_name, "DIPv4");
                qual_print_len = 8;
                break;
            case SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP:
                sal_strcpy(qual_name, "SIPv4");
                qual_print_len = 8;
                break;
            case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW:
                sal_strcpy(qual_name, "DIPv6_L");
                qual_print_len = 8;
                break;
            case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH:
                sal_strcpy(qual_name, "DIPv6_H");
                qual_print_len = 8;
                break;
            case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW:
                sal_strcpy(qual_name, "SIPv6_L");
                qual_print_len = 8;
                break;
            case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH:
                sal_strcpy(qual_name, "SIPv6_H");
                qual_print_len = 8;
                break;
            case SOC_PPC_FP_QUAL_IRPP_VRF:
                sal_strcpy(qual_name, "VRF");
                qual_print_len = 4;
                break;
            case SOC_PPC_FP_QUAL_IRPP_IN_RIF:
                sal_strcpy(qual_name, "INRIF");
                qual_print_len = 4;
                break;
            default:
                /* Attempt to retrieve the qual name string from function */
                if (sal_strcmp(" Unknown", SOC_PPC_FP_QUAL_TYPE_to_string(qual_vals[i].type))) {
                    sal_strcpy(qual_name, SOC_PPC_FP_QUAL_TYPE_to_string(qual_vals[i].type));
                } else {
                    sal_sprintf(qual_name, "Qual-%03d", qual_vals[i].type);
                }
                qual_print_len = 8;/*over 32 bits are printed as 64bit*/
            }
            header_flag ? cli_out(" |%s: 0x", qual_name) : cli_out(" |%*s  0x", (int)strlen(qual_name), "");

            qual_tbl_index = 0;
            qual_max_size = 0;
            while (qual_tbl_index != SOC_PPC_FP_NOF_QUALS_PER_DB_MAX) {
                if (dbal_table.qual_info[qual_tbl_index].qual_type == qual_vals[i].type) {
                    qual_max_size += dbal_table.qual_info[qual_tbl_index].qual_nof_bits;
                    qual_type_tbl_index = qual_tbl_index;
                }
                qual_tbl_index++;
            }

            /*In case this qualifier is longer than 32bits, print it all*/
            if (dbal_table.qual_info[qual_type_tbl_index].qual_full_size > SOC_SAND_NOF_BITS_IN_UINT32) {
                cli_out("%0*x", 8, qual_vals[i].val.arr[1]);
            }

            /* Calculate the qual prefix length */
            qual_prio_len = prio_len > qual_max_size ? qual_max_size : prio_len;
            prio_len -= qual_prio_len;
            cli_out("%0*x/%02d/%02d", qual_print_len, qual_vals[i].val.arr[0], qual_prio_len, qual_max_size);
        }
    }

    cli_out(header_flag ? " |Result: 0x" : " |        0x");

    for (i=0; i < BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS - 7); i++ ) {
        cli_out("%02x", ad_8[i]);
    }
    /*print the last 4 bits only*/
    cli_out("%01x/%02d", ad_8[i] >> (BYTES2BITS(BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS))-JER_KAPS_AD_WIDTH_IN_BITS), JER_KAPS_AD_WIDTH_IN_BITS);

    cli_out("|\n\r");

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_kaps_parse_print_entry()", 0, 0);
}

/* Prints the contents of the kaps table */
int jer_kaps_show_table(int unit, uint32 dbal_table_id, uint32 print_entries) {
    struct kbp_entry_iter *iter;
    struct kbp_entry *kpb_e;
    struct kbp_entry_info kpb_e_info;
    struct kbp_ad_db *ad_db_p;

    JER_KAPS_TABLE_CONFIG table_cfg_p, table_cfg_p_iter;
    SOC_DPP_DBAL_TABLE_INFO dbal_table;

    JER_KAPS_DB *tbl_p;
    uint32 j, i;
    uint32 res;

    uint8 db_id = 0;

    uint8 ad_8[BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS)];
    uint32 tbl_idx;

    uint32 num_of_entries = 0;

    uint8 header_flag = 1;

    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    DBAL_QUAL_VALS_CLEAR(qual_vals);

    res = sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, dbal_table_id, &dbal_table);
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

    if (dbal_table.physical_db_type != SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS) {
        cli_out("Provided DBAL_TBL_ID is not associated with KAPS.\n");
        ARAD_DO_NOTHING_AND_EXIT;
    }

    /*Print from both private and public tables*/
    for (j=0; j<2; j++) {
        if (!JER_KAPS_ENABLE_PUBLIC_DB(unit) && j==0) {
            continue;
        }
        if (!JER_KAPS_ENABLE_PRIVATE_DB(unit) && j==1) {
            continue;
        }

        res = jer_pp_kaps_dbal_table_id_translate(unit, dbal_table_id, NULL/*qual_vals*/, j/*private_table*/, &tbl_idx/*kaps*/);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        jer_kaps_table_config_get(unit, tbl_idx, &table_cfg_p);

        if ((tbl_idx < JER_KAPS_IP_NOF_TABLES) && (tbl_idx >= 0)) {
            tbl_p = table_cfg_p.tbl_p;
            db_id = table_cfg_p.db_id;
        } else {
            tbl_p = NULL;
        }


        /* Only print if the table is allocated and not a clone */
        if ((table_cfg_p.clone_of_tbl_id == JER_KAPS_IP_NOF_TABLES)
            || ((table_cfg_p.clone_of_tbl_id == JER_KAPS_TABLE_USE_DB_HANDLE) &&
                (jer_kaps_clone_of_db_id_get(unit, table_cfg_p.db_id) == JER_KAPS_IP_NOF_DB))) {

            /* Calculate total number of entries */

            /*initialize iterator*/
            KBP_TRY(kbp_db_entry_iter_init(tbl_p, &iter));
            num_of_entries = 0;
            /*traverse entries*/
            do {
                KBP_TRY(kbp_db_entry_iter_next(tbl_p, iter, &kpb_e));

                if (kpb_e == NULL)
                    break;

                KBP_TRY(kbp_entry_get_info(tbl_p, kpb_e, &kpb_e_info));
                /* Verify that the table prefix is correct - dynamic tables share the KAPS table but do not necessarily share the dbal prefix*/
                if (kpb_e_info.data[0] >> (SOC_SAND_NOF_BITS_IN_BYTE - dbal_table.db_prefix_len) != dbal_table.db_prefix) {
                    continue;
                }

                num_of_entries += 1;

            } while (1);

            /*reclaim iterator memory*/
            KBP_TRY(kbp_db_entry_iter_destroy(tbl_p, iter));


            if (tbl_idx < JER_KAPS_IP_PUBLIC_INDEX) {
                cli_out("\n\rDBAL Table ID %d: %s - Private DB, number of entries: %d."
                        "\n\r---------------------------------------------------------------"
                        "-------------------------------------------------------------------"
                        "--------------------------------------------------------------------\n\r",
                        dbal_table_id, dbal_table.table_name, num_of_entries);
            } else {
                cli_out("\n\rDBAL Table ID %d: %s - Public DB, number of entries: %d."
                        "\n\r---------------------------------------------------------------"
                        "-------------------------------------------------------------------"
                        "--------------------------------------------------------------------\n\r",
                        dbal_table_id, dbal_table.table_name, num_of_entries);
            }

            header_flag = 1;

            if (print_entries) {
                /*initialize iterator*/
                KBP_TRY(kbp_db_entry_iter_init(tbl_p, &iter));

                /*traverse entries*/
                do {
                    KBP_TRY(kbp_db_entry_iter_next(tbl_p, iter, &kpb_e));

                    if (kpb_e == NULL)
                        break;

                    KBP_TRY(kbp_entry_get_info(tbl_p, kpb_e, &kpb_e_info));

                    /* Verify that the table prefix is correct - dynamic tables share the KAPS table but do not necessarily share the dbal prefix*/
                    if (kpb_e_info.data[0] >> (SOC_SAND_NOF_BITS_IN_BYTE - dbal_table.db_prefix_len) != dbal_table.db_prefix) {
                        continue;
                    }

                    jer_kaps_ad_db_by_db_id_get(unit, db_id, &ad_db_p);

                    if ((ad_db_p != NULL) && (kpb_e_info.ad_handle != NULL)) {
                            KBP_TRY(kbp_ad_db_get(ad_db_p, kpb_e_info.ad_handle, ad_8));
                    }

                    res = jer_kaps_parse_print_entry(unit, dbal_table_id, kpb_e_info.prio_len, kpb_e_info.data, ad_8, header_flag);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

                    header_flag = 0;
                } while (1);

                /*reclaim iterator memory*/
                KBP_TRY(kbp_db_entry_iter_destroy(tbl_p, iter));
            }
        } else { /* Table is a clone */
            SOC_DPP_DBAL_SW_TABLE_IDS orig_dbal_table_id;
            SOC_DPP_DBAL_TABLE_INFO orig_dbal_table;

            if (table_cfg_p.clone_of_tbl_id == JER_KAPS_TABLE_USE_DB_HANDLE) {
                /* This table uses the handle of a clone db, find the first table that uses the orig db */
                uint32 orig_dbal_id = jer_kaps_clone_of_db_id_get(unit, table_cfg_p.db_id);
                for (i=0; i<JER_KAPS_IP_NOF_TABLES ; i++) {
                    jer_kaps_table_config_get(unit, i, &table_cfg_p_iter);
                    if ((table_cfg_p_iter.db_id == orig_dbal_id) && (table_cfg_p_iter.clone_of_tbl_id == JER_KAPS_TABLE_USE_DB_HANDLE)) {
                        break;
                    }
                }

                res = jer_pp_kaps_table_id_to_dbal_translate(unit, i, &orig_dbal_table_id);
                SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
            } else {
                res = jer_pp_kaps_table_id_to_dbal_translate(unit, table_cfg_p.clone_of_tbl_id, &orig_dbal_table_id);
                SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
            }

            res = sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, orig_dbal_table_id, &orig_dbal_table);
            SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);

            if (tbl_idx < JER_KAPS_IP_PUBLIC_INDEX) {
                cli_out("\n\rDBAL Table ID %d: %s is clone of DBAL Table ID %d: %s - Private DB"
                        "\n\r---------------------------------------------------------------"
                        "-------------------------------------------------------------------"
                        "--------------------------------------------------------------------\n\r",
                        dbal_table_id, dbal_table.table_name, orig_dbal_table_id, orig_dbal_table.table_name);
            } else {
                cli_out("\n\rDBAL Table ID %d: %s is clone of DBAL Table ID %d: %s - Public DB"
                        "\n\r---------------------------------------------------------------"
                        "-------------------------------------------------------------------"
                        "--------------------------------------------------------------------\n\r",
                        dbal_table_id, dbal_table.table_name, orig_dbal_table_id, orig_dbal_table.table_name);
            }
        }
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_kaps_show_table()", 0, 0);
}


/* Prints the stats of the kaps table */
int jer_kaps_show_db_stats( int unit, uint32 dbal_table_id ) {
    struct kbp_db_stats stats;

    SOC_DPP_DBAL_TABLE_INFO dbal_table;

    JER_KAPS_DB *tbl_p;
    JER_KAPS_TABLE_CONFIG table_cfg_p;
    uint32 i, j;
    uint32 res;

    uint32 tbl_idx;

    uint32 db_size;
    uint32 db_width;

    SOC_SAND_INIT_ERROR_DEFINITIONS( 0 );

    res = sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get( unit, dbal_table_id, &dbal_table );
    SOC_SAND_CHECK_FUNC_RESULT( res, 12, exit );

    if (dbal_table.physical_db_type != SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS) {
        cli_out("Provided DBAL_TBL_ID is not associated with KAPS.\n");
        ARAD_DO_NOTHING_AND_EXIT;
    }

    /*Print from both private and public tables*/
    for ( j = 0; j < 2 ; j++ ) {
        res = jer_pp_kaps_dbal_table_id_translate( unit, dbal_table_id, NULL/*qual_vals*/, j/*private_table*/, &tbl_idx/*kaps*/);
        SOC_SAND_CHECK_FUNC_RESULT( res, 10, exit );

        if ( ( tbl_idx < JER_KAPS_IP_NOF_TABLES ) && ( tbl_idx >= 0 ) ) {
            jer_kaps_table_config_get(unit, tbl_idx, &table_cfg_p);
            tbl_p = table_cfg_p.tbl_p;

            if ((table_cfg_p.db_id == JER_KAPS_IP_CORE_0_PRIVATE_DB_ID) || (table_cfg_p.db_id == JER_KAPS_IP_CORE_1_PRIVATE_DB_ID))
            {
                db_size = soc_property_get(unit, spn_PRIVATE_IP_FRWRD_TABLE_SIZE, 0);
            }
            else
            {
                db_size = soc_property_get(unit, spn_PUBLIC_IP_FRWRD_TABLE_SIZE, 0);
            }

            db_width = dbal_table.db_prefix_len;
            /* Only count non-zero qualifiers in the key-width, otherwise it would be a constant 160bit */
            for (i = 0; i < dbal_table.nof_qualifiers; i++) {
                if (dbal_table.qual_info[i].qual_type != SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES) {
                    db_width += dbal_table.qual_info[i].qual_nof_bits;
                }
            }
        } else {
            tbl_p = NULL;
        }

        /*Only print if the table is allocated*/
        if ( ( tbl_p != NULL ) && ( dbal_table.is_table_initiated ) ) {
            cli_out( " %-3d", dbal_table_id);
            cli_out( "%-10s", j ? "- Private" : "- Public" );
            cli_out( " %-23s", dbal_table.table_name );
            cli_out( " %-10d", db_size );
            cli_out( " %-13d", db_width );
            cli_out( " %-10d", JER_KAPS_AD_WIDTH_IN_BITS );
            res = kbp_db_stats( tbl_p, &stats );
            if ( ARAD_KBP_TO_SOC_RESULT( res ) != SOC_SAND_OK ) {
                LOG_ERROR( BSL_LS_SOC_TCAM, ( BSL_META_U( unit, "Error in %s(): DB: kbp_db_stats with failed: %s!\n" ), FUNCTION_NAME(), kbp_get_status_string( res ) ) );
            }
            cli_out( " %-13d", stats.num_entries );
            cli_out( " %-10d\n", stats.capacity_estimate );
        } else {
            cli_out( " %-10d", dbal_table_id );
            cli_out( " %-23s", dbal_table.table_name );
            cli_out( "not allocated\n" );
        }
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_kaps_show_db_stats()", 0, 0 );
}

/* kaps write/read test diagnostic tool */
int jer_pp_kaps_diag_01(int unit)
{
    int32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = kaps_diag_01(jer_kaps_kaps_xpt_p_get(unit));
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "\n Overall Result: Failed, Reason Code: %s\n"),
                  kbp_get_status_string(res)));

       SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }
    else
    {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "\n Overall Result: Passed\n")));
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_pp_kaps_diag_01()", 0, 0);
}

/* kaps search test diagnostic tool */
int jer_pp_kaps_diag_02(int unit)
{
    int32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = kaps_diag_02(jer_kaps_kaps_xpt_p_get(unit));
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "\n Overall Result: Failed, Reason Code: %s\n"),
                  kbp_get_status_string(res)));

       SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }
    else
    {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "\n Overall Result: Passed\n")));
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_pp_kaps_diag_02()", 0, 0);
}

/* KAPS RPB TCAM built in self test */
int jer_pp_kaps_tcam_bist(int unit)
{
    int32 res;
    uint32 blk_id, cam_index, cam_index_max, nbytes = sizeof(uint32);
    JER_KAPS_XPT *xpt_p;
    uint8 data[4]={0}, res_data[4]={0}, expected_data[4]={0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* set the expected status */
    expected_data[0] = 0;
    expected_data[1] = 0;
    expected_data[2] = 0;
    expected_data[3] = 3;

    cam_index_max = SOC_IS_JERICHO_PLUS(unit) ? 1 : 2;
    xpt_p = (JER_KAPS_XPT *)jer_kaps_kaps_xpt_p_get(unit);

    for (blk_id = JER_KAPS_RPB_BLOCK_INDEX_START; blk_id <= JER_KAPS_RPB_BLOCK_INDEX_END; blk_id++) {
        for(cam_index = 0; cam_index < cam_index_max; cam_index++) {
            sal_memset(data, 0, sizeof(uint32));
            sal_memset(res_data, 0, sizeof(uint32));
            data[3] = cam_index + 1;

            res = xpt_p->jer_kaps_xpt.kaps_command(xpt_p,
                                               KAPS_CMD_WRITE,
                                               KAPS_FUNC0,
                                               blk_id,
                                               JER_KAPS_RPB_CAM_BIST_CONTROL_REG_ADDR,
                                               nbytes,
                                               data);

            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "\n jer_pp_kaps_write_command Failed, Reason Code: %s\n"),
                          kbp_get_status_string(res)));

               SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }

            if (SOC_IS_JERICHO_PLUS(unit)) {
                res = arad_polling(
                            unit,
                            ARAD_TIMEOUT,
                            ARAD_MIN_POLLS,
                            KAPS_RPB_CAM_BIST_STATUSr,
                            REG_PORT_ANY,
                            (blk_id - JER_KAPS_RPB_BLOCK_INDEX_START),
                            RPB_BIST_STATUS_Nf,
                            0x3
                           );
                SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
            }

            res = xpt_p->jer_kaps_xpt.kaps_command(xpt_p,
                                               KAPS_CMD_READ,
                                               KAPS_FUNC0,
                                               blk_id,
                                               JER_KAPS_RPB_CAM_BIST_STATUS_REG_ADDR,
                                               nbytes,
                                               res_data);

            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "\n jer_pp_kaps_read_command Failed, Reason Code: %s\n"),
                          kbp_get_status_string(res)));

               SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }

            if (sal_memcmp(res_data, expected_data, 4) != 0) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "\n unexpected regitser value \n")));

               SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }
        }
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_pp_kaps_tcam_bist()", 0, 0);
}

int jer_pp_kbp_sdk_ver_test(int unit)
{
    int32 res;
    uint32 regval, i, array_index = 0;
    /* The following three arrays hold the compatible kbp_sdk_ver and sw_sdk_ver.
     * The first sw_sdk_ver that supports the corresponding kbp_sdk_ver is listed in the same array index.
     * We need to maintain in parallel the compatible version for 6_4 and 6_5 branches.
     */
    uint32 number_of_elements;
    const char* kbp_sdk_ver_arr[] =       {"KBP SDK 1.3.11", "KBP SDK 1.4.4", "KBP SDK 1.4.6", "KBP SDK 1.4.7", "KBP SDK 1.4.8", "KBP SDK 1.4.9"};
    const uint32 sw_sdk_ver_arr_6_5[]  = {0x65100000, 0x65300000, 0x65400000, 0x65500000, 0x65600000, 0x65700000};
    const uint32 sw_sdk_ver_arr_6_4[]  = {0x64700000, 0x64b00000, 0x64b00000, 0x64b00000, 0x64b00000, 0x64b00000};

    const uint32 *relevant_array;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = soc_reg32_get(unit, ECI_SW_VERSIONr, REG_PORT_ANY,  0, &regval);
    if (res != SOC_SAND_OK) {
        cli_out("Error: jer_pp_kbp_sdk_ver_test(%d), soc_reg32_get\n", unit);
        return res;
    }

    /* Find if we are on the 6_4 branch or 6_5 */
    if (regval < sw_sdk_ver_arr_6_5[0]) {
        relevant_array = sw_sdk_ver_arr_6_4;
    } else {
        relevant_array = sw_sdk_ver_arr_6_5;
    }

    /* Find the closest relevant sw_sdk_ver in the array */
    number_of_elements = sizeof(sw_sdk_ver_arr_6_5)/sizeof(sw_sdk_ver_arr_6_5[0]);
    for (i=0; i<number_of_elements; i++) {
        if (regval >= relevant_array[i]) {
            array_index = i;
        }
    }

    if (sal_strcmp(kbp_sdk_ver_arr[array_index], kbp_device_get_sdk_version())) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "\n kbp_sdk_ver and sw_sdk_ver do not match. Expected: %s. Current: %s.\n"),
                  kbp_sdk_ver_arr[array_index], kbp_device_get_sdk_version()));

       SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_pp_kbp_sdk_ver_test()", 0, 0);
}

STATIC
int jer_pp_kaps_verify_search_results(int unit, uint32 is_matched[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES], uint32 expected_is_matched[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES],
                                      uint32 search_payload_32[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES],
                                      uint32 expected_search_payload_32[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES],
                                      uint32 prefix_len_array[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES], uint32 expected_prefix_len_array[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES],
                                      uint32 num_of_valid_tables)
{
    uint32 i;

    SOCDNX_INIT_FUNC_DEFS;

    for (i=0; i < num_of_valid_tables; i++) {
        if (is_matched[i] == expected_is_matched[i]) {
            if (search_payload_32[i] == expected_search_payload_32[i]) {
                if ((prefix_len_array != NULL) && (prefix_len_array[i] != expected_prefix_len_array[i])) {
                    cli_out("search verification failed, wrong prefix length!\nDetails: Search id: %d Prefix_len: %d Expected_prefix_len: %d \n",
                            i, prefix_len_array[i], expected_prefix_len_array[i]);
                    SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                } else{
                    cli_out("Search %d verified!\n", i);
                }
            } else {
                cli_out("Search verification failed, wrong payloads! \nDetails: Search id: %d Payload: 0x%x Expected_payload: 0x%x \n",
                            i, search_payload_32[i], expected_search_payload_32[i]);
                SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
            }
        } else {
            cli_out("Search verification failed, hit-match mismatch! \nDetails: Search id: %d Match: %d Expected_match: %d \n",
                        i, is_matched[i], expected_is_matched[i]);
            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* KAPS RPB TCAM built in self test */
int jer_pp_kaps_search_test(int unit, uint32 add_entries, uint32 search_entries, uint32 delete_entries, uint32 loop_test, uint32 cache_test)
{
    uint32 res;

    SOC_SAND_PP_IPV6_ADDRESS dip_v6_32,
           sip_v6_32,
        mc_v6_32;
    uint32 dip_v4 = 0, sip_v4 = 0, mc_v4 = 0, vrf = 0, inrif = 0;

    uint32 i,j,k;
    uint8 key[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES][JER_KAPS_KEY_BUFFER_NOF_BYTES];

    uint32 payload;

    JER_KAPS_SEARCH_CONFIG search_cfg;

    SOC_SAND_SUCCESS_FAILURE success;

    uint32  prefix_len;
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 mask[SOC_SAND_PP_IPV6_ADDRESS_NOF_UINT32S] = {SOC_SAND_U32_MAX, SOC_SAND_U32_MAX, SOC_SAND_U32_MAX, SOC_SAND_U32_MAX};
    uint32 partial_mask[SOC_SAND_PP_IPV6_ADDRESS_NOF_UINT32S] = {0, 0, SOC_SAND_U32_MAX, SOC_SAND_U32_MAX};
    SOC_DPP_DBAL_SW_TABLE_IDS table_id;
    SOC_DPP_DBAL_TABLE_INFO table;

    struct kbp_search_result cmp_rslt;
    uint32 is_matched[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES], prefix_len_array[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];
    uint32 expected_is_matched[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES], expected_prefix_len_array[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];
    uint8 search_payload[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES][BITS2BYTES(JER_KAPS_AD_WIDTH_IN_BITS)];
    uint32 expected_search_payload[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];

    uint32 search_payload_32[JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES];

    uint32 desc_dma_init_state = 0;

    SOCDNX_INIT_FUNC_DEFS;

    dip_v6_32.address[0] = 0x12345678;
    dip_v6_32.address[1] = 0x9abcdef0;
    dip_v6_32.address[2] = 0xfedcba98;
    dip_v6_32.address[3] = 0x76543210;

    sip_v6_32.address[0] = 0x01020304;
    sip_v6_32.address[1] = 0x05060708;
    sip_v6_32.address[2] = 0x090a0b0c;
    sip_v6_32.address[3] = 0x0d0e0f00;

    mc_v6_32.address[0] = 0x01020304;
    mc_v6_32.address[1] = 0x05060708;
    mc_v6_32.address[2] = 0x090a0b0c;
    mc_v6_32.address[3] = 0x0d0e0f00;

    dip_v4 = 0xa1b2c3d4;
    sip_v4 = 0xf9e8d7c6;
    mc_v4 =  0xf13579bd;

    vrf = 5;
    inrif = 3;

    #define IPV4_DIP_PAYLOAD 0x12345
    #define IPV4_SIP_PAYLOAD 0x54321
    #define IPV4_MC_PAYLOAD  0xabcde

    #define IPV6_DIP_PAYLOAD 0x67890
    #define IPV6_SIP_PAYLOAD 0x98765
    #define IPV6_MC_PAYLOAD  0xfedcb

    if (cache_test) {
        if (SOC_IS_JERICHO_PLUS(unit)) {
            desc_dma_init_state = jer_sbusdma_desc_is_enabled(unit);
            if (!desc_dma_init_state) {
                SOCDNX_IF_ERR_EXIT(jer_sbusdma_desc_init(unit, 0x100/*desc_num_max*/, 0x1000/*mem_buff_size*/, 10000/*timeout*/));
            }
        } else {
            cli_out("Error: KAPS caching is only available in QAX and above\n");
            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_UNAVAIL);
        }
    }

    do {
        loop_test--;
        for (k=0; k < 2; k++) { /* Second pass runs with all expected zero after a table clear*/
            /* Initialize qual_vals to include all the possible quals in the searches*/
            DBAL_QUAL_VALS_CLEAR(qual_vals);
            DBAL_QUAL_VAL_ENCODE_FWD_IPV4_SIP(&qual_vals[0], sip_v4, SOC_SAND_NOF_BITS_IN_UINT32);
            DBAL_QUAL_VAL_ENCODE_IPV6_SIP_LOW(&qual_vals[1], sip_v6_32.address, mask);
            DBAL_QUAL_VAL_ENCODE_IPV6_SIP_HIGH(&qual_vals[2], sip_v6_32.address, mask);
            DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], dip_v4, SOC_SAND_NOF_BITS_IN_UINT32);
            DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], dip_v6_32.address, mask);
            DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], dip_v6_32.address, mask);
            DBAL_QUAL_VAL_ENCODE_IN_RIF(&qual_vals[6], inrif, SOC_SAND_U32_MAX);
            DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], vrf, SOC_SAND_U32_MAX);

            if (add_entries && (k==0)) {
                /* Add the ipv4 dip into the ipv4-uc table, fully masked, private entry */
                payload = IPV4_DIP_PAYLOAD;
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, qual_vals, 0/*priority*/,  &payload, &success));

                /* Add the ipv4 sip into the ipv4-uc table, partially masked, public entry */
                payload = IPV4_SIP_PAYLOAD;
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], sip_v4, SOC_SAND_NOF_BITS_IN_UINT32/2);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, qual_vals, 0/*priority*/,  &payload, &success));

                /* Add the ipv4 mc into the ipv4-mc table, fully-masked, private entry */
                payload = IPV4_MC_PAYLOAD;
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], vrf, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], mc_v4, SOC_SAND_NOF_BITS_IN_UINT32);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, qual_vals, 0/*priority*/,  &payload, &success));

                /* Add the ipv6 dip into the ipv6-uc table, fully masked, public entry */
                payload = IPV6_DIP_PAYLOAD;
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, qual_vals, 0/*priority*/,  &payload, &success));

                /* Add the ipv6 sip into the ipv6-uc table, partially masked, private entry */
                payload = IPV6_SIP_PAYLOAD;
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], vrf, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], sip_v6_32.address, partial_mask);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], sip_v6_32.address, partial_mask);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, qual_vals, 0/*priority*/,  &payload, &success));

                /* Add the ipv6 mc into the ipv6-mc table, fully-masked, public entry */
                payload = IPV6_MC_PAYLOAD;
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], mc_v6_32.address, mask);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], mc_v6_32.address, mask);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_add(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, qual_vals, 0/*priority*/,  &payload, &success));
            }

            if (delete_entries && (k==1)) {
                cli_out("\n-----------------------------\nClearing Test Entries.\n-----------------------------\n");
                /* Delete the ipv4 dip from the ipv4-uc table, fully masked, private entry */
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, qual_vals, &success));

                /* Delete the ipv4 sip from the ipv4-uc table, partially masked, public entry */
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], sip_v4, SOC_SAND_NOF_BITS_IN_UINT32/2);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, qual_vals, &success));

                /* Delete the ipv4 mc from the ipv4-mc table, fully-masked, private entry */
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], vrf, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], mc_v4, SOC_SAND_NOF_BITS_IN_UINT32);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, qual_vals, &success));

                /* Delete the ipv6 dip from the ipv6-uc table, fully masked, public entry */
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, qual_vals, &success));

                /* Delete the ipv6 sip from the ipv6-uc table, partially masked, private entry */
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], vrf, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], sip_v6_32.address, partial_mask);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], sip_v6_32.address, partial_mask);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, qual_vals, &success));

                /* Delete the ipv6 mc from the ipv6-mc table, fully-masked, public entry */
                DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], mc_v6_32.address, mask);
                DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], mc_v6_32.address, mask);
                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_entry_delete(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, qual_vals, &success));
            }

            if (SOC_IS_JERICHO_PLUS(unit) && jer_sbusdma_desc_is_enabled(unit)) {
                SOCDNX_IF_ERR_EXIT(jer_sbusdma_desc_wait_done(unit));
            } else {
                SOCDNX_IF_ERR_EXIT(jer_pp_xpt_wait_dma_done(unit));
            }
            sal_memset(expected_is_matched, 0, sizeof(uint32)*JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES);
            sal_memset(expected_search_payload, 0, sizeof(uint32)*JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES);
            sal_memset(expected_prefix_len_array, 0, sizeof(uint32)*JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES);
            if (search_entries && ((k==0 && add_entries) || (k==1 && delete_entries))) {
                for (j=0; j < JER_KAPS_NOF_SEARCHES; j++) {
                    jer_kaps_search_config_get(unit, j, &search_cfg);
                    if (!search_cfg.valid_tables_num) {
                        LOG_ERROR(BSL_LS_SOC_TCAM,
                             (BSL_META_U(unit,
                                         "\n Search config %d disabled.\n"), j));
                        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                    }

                    cli_out("\n-----------------------------\nTesting KAPS search config %d - Tables %s.\n-----------------------------\n", j, k ? "cleared" : "filled");

                    for (i=0; i < search_cfg.valid_tables_num; i++) {
                        /*vrf!=0 only for private*/
                        if (search_cfg.tbl_id[i] < JER_KAPS_IP_PUBLIC_INDEX) {
                            DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], vrf, SOC_SAND_U32_MAX);
                        } else{
                            DBAL_QUAL_VAL_ENCODE_VRF(&qual_vals[7], 0, SOC_SAND_U32_MAX);
                        }

                        res = jer_pp_kaps_table_id_to_dbal_translate(unit, search_cfg.tbl_id[i], &table_id);
                        if (res != SOC_SAND_OK) {
                            cli_out("Error: jer_pp_kaps_search_test(%d), table translation failed\n", unit);
                            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                        }

                        res = sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, table_id, &table);
                        if (res != SOC_SAND_OK) {
                            cli_out("Error: jer_pp_kaps_search_test(%d), get dbal table failed\n", unit);
                            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                        }

                        /* MC entries utilize the dip qual, need to change it per ipv4/ipv6 table */
                        if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS) {
                            DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], dip_v4, SOC_SAND_NOF_BITS_IN_UINT32);
                        } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS) {
                            DBAL_QUAL_VAL_ENCODE_FWD_IPV4_DIP(&qual_vals[3], mc_v4, SOC_SAND_NOF_BITS_IN_UINT32);
                        } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE) {
                            DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], dip_v6_32.address, mask);
                            DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], dip_v6_32.address, mask);
                        } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC) {
                            DBAL_QUAL_VAL_ENCODE_IPV6_DIP_LOW(&qual_vals[4], mc_v6_32.address, mask);
                            DBAL_QUAL_VAL_ENCODE_IPV6_DIP_HIGH(&qual_vals[5], mc_v6_32.address, mask);
                        }

                        res = arad_pp_dbal_entry_key_to_kbp_buffer(unit, &table, JER_KAPS_KEY_BUFFER_NOF_BYTES, qual_vals, &prefix_len, key[i]);
                        if (res != SOC_SAND_OK) {
                            cli_out("Error: jer_pp_kaps_search_test(%d), key buffer construction failed\n", unit);
                            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                        }

                        /* Configure expected search results */
                        if (k == 0) { /* Second pass after the tables clear should expect all 0 */
                            SOC_DPP_DBAL_TABLE_INFO dbal_table;
                            res = sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, &dbal_table);
                            if (res != SOC_SAND_OK) {
                                cli_out("Error: sw_state_access(%d), Get DBAL table info failed\n", unit);
                                SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                            }
                            switch (search_cfg.tbl_id[i]) {
                                case JER_KAPS_IP_CORE_1_PUBLIC_IPV4_UC_TBL_ID:
                                    expected_is_matched[i] = 1; expected_search_payload[i] = IPV4_SIP_PAYLOAD;
                                    /* Support dynamic tables */
                                    expected_prefix_len_array[i] = 30 + (dbal_table.db_prefix_len > JER_KAPS_TABLE_PREFIX_LENGTH ? 6 : 2);
                                    break;
                                case JER_KAPS_IP_CORE_0_PUBLIC_IPV6_UC_TBL_ID:
                                    expected_is_matched[i] = 1; expected_search_payload[i] = IPV6_DIP_PAYLOAD; expected_prefix_len_array[i] = 144;
                                    break;
                                case JER_KAPS_IP_CORE_0_PUBLIC_IPV6_MC_TBL_ID:
                                    expected_is_matched[i] = 1; expected_search_payload[i] = IPV6_MC_PAYLOAD; expected_prefix_len_array[i] = 159;
                                    break;
                                case JER_KAPS_IP_CORE_0_PRIVATE_IPV4_UC_TBL_ID:
                                    expected_is_matched[i] = 1; expected_search_payload[i] = IPV4_DIP_PAYLOAD;
                                    /* Support dynamic tables */
                                    expected_prefix_len_array[i] = 46 + (dbal_table.db_prefix_len > JER_KAPS_TABLE_PREFIX_LENGTH ? 6 : 2);
                                    break;
                                case JER_KAPS_IP_CORE_0_PRIVATE_IPV4_MC_TBL_ID:
                                    expected_is_matched[i] = 1; expected_search_payload[i] = IPV4_MC_PAYLOAD; expected_prefix_len_array[i] = 92;
                                    break;
                                case JER_KAPS_IP_CORE_1_PRIVATE_IPV6_UC_TBL_ID:
                                    expected_is_matched[i] = 1; expected_search_payload[i] = IPV6_SIP_PAYLOAD; expected_prefix_len_array[i] = 80;
                                    break;
                                case JER_KAPS_IP_CORE_0_PUBLIC_IPV4_UC_TBL_ID:
                                case JER_KAPS_IP_CORE_0_PUBLIC_IPV4_MC_TBL_ID:
                                case JER_KAPS_IP_CORE_1_PUBLIC_IPV6_UC_TBL_ID:
                                case JER_KAPS_IP_CORE_1_PRIVATE_IPV4_UC_TBL_ID:
                                case JER_KAPS_IP_CORE_0_PRIVATE_IPV6_UC_TBL_ID:
                                case JER_KAPS_IP_CORE_0_PRIVATE_IPV6_MC_TBL_ID:
                                    expected_is_matched[i] = 0; expected_search_payload[i] = 0; expected_prefix_len_array[i] = 0;
                                    break;
                                default:
                                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("jer_pp_kaps_search_test - invalid kaps table_id: %d \n"), search_cfg.tbl_id[i]));
                            }
                        }
                    }

                    res = jer_kaps_search_generic(unit, j, key, is_matched, prefix_len_array, search_payload, &cmp_rslt);
                    if (res != SOC_SAND_OK) {
                        cli_out("Error: jer_kaps_search_generic(%d)\n", unit);
                        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                    }

                    /* Verify expected search results */
                    cli_out("\nVerifying SW search config %d: \n", j);
                    for (i=0; i < search_cfg.valid_tables_num; i++) {
                        SOCDNX_SAND_IF_ERR_EXIT(jer_pp_kaps_route_to_kaps_payload_buffer_decode(unit, search_payload[i], &search_payload_32[i]));
                    }
                    SOCDNX_IF_ERR_EXIT(jer_pp_kaps_verify_search_results(unit, is_matched, expected_is_matched, search_payload_32, expected_search_payload, prefix_len_array, expected_prefix_len_array, search_cfg.valid_tables_num));

                    cli_out("\nVerifying HW search config %d: \n", j);
                    for (i=0; i < search_cfg.valid_tables_num; i++) {
                        SOCDNX_SAND_IF_ERR_EXIT(jer_pp_kaps_route_to_kaps_payload_buffer_decode(unit, cmp_rslt.assoc_data[i], &search_payload_32[i]));
                    }
                    SOCDNX_IF_ERR_EXIT(jer_pp_kaps_verify_search_results(unit, cmp_rslt.hit_or_miss, expected_is_matched, search_payload_32, expected_search_payload, NULL /*prefix*/, NULL, search_cfg.valid_tables_num));

                }
            }

            if (delete_entries == 0) {
                break;
            }
        }
    } while (loop_test);

    if (SOC_IS_JERICHO_PLUS(unit) && cache_test && desc_dma_init_state == 0) {
        SOCDNX_IF_ERR_EXIT(jer_sbusdma_desc_deinit(unit));
    }

exit:
  SOCDNX_FUNC_RETURN;
}


#else /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */
soc_error_t soc_jer_pp_diag_kaps_lkup_info_get(
   SOC_SAND_IN int unit,
    SOC_SAND_IN int core_id
   )
{
    SOCDNX_INIT_FUNC_DEFS;
    cli_out("KAPS unsupported\n");
    SOCDNX_FUNC_RETURN
}

#endif /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */
