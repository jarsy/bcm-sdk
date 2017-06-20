/*
 * $Id: etu_fifo.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    etu.c
 * Purpose: Caladan3 External TCAM driver Read/Write interface manager
 *          For CLI and NL XPT driver for Caladan3 ETU
 *
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <shared/util.h>
#include <soc/sbx/caladan3/etu.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/debug.h>


int etu_debug = 0;
int etu_mode = 0;
static int etu_flush_count[SOC_MAX_NUM_DEVICES];
static etu_cp_fifo_entry_t *cp_entries[SOC_MAX_NUM_DEVICES];

#define KEY_WIDTH_IN_64B_WORDS(w) (((w)+63) >> 6)
#define KEY_WIDTH_IN_BYTES(w) (((w)+7) >> 3)

#define PACK_ADDR(m, a)  \
         (!(m) ? (a) : \
        (((a) >> 24) | ((((a) >> 16) & 0xff) << 8) | ((((a) >> 8) & 0xff) << 16) | (((a) & 0xff) << 24)))


#define PACK_WRMODE(m, a)  \
         (!(m) ? ((a) << 30) : ((a) << 6))

#define PACK_VBIT(m, a)  \
         (!(m) ? ((a) << 31) : ((a) << 7))

#define PACK_WORD(m ,b0, b1, b2, b3) \
         (!(m) ? \
           (((b3) << 24) | ((b2) << 16) | ((b1) << 8) | b0) : \
           (((b0) << 24) | ((b1) << 16) | ((b2) << 8) | b3))

/*
 * Address Format: 
 *   [31------30------------25------24--------------------------------------0]
 *   [wrmode][vbit][0000][op][devid(1)][devid(0)][000][blk(7:0)][index(11:0)] 
 *   [   w  ][ v  ][0000][op][  d(1)  ][  d(0)  ][000][   r(19:0)           ] 
 */
#define ENCODE_ADDR(op, d, r, w, v)             \
        (((((d) & 1) << 23) | (r)) |              \
             ((op << 25) | (((d >> 1) & 1) << 24)) |  \
             (((w) & 1) << 31)                     |  \
             (((v) & 1) << 30))                       \

void
soc_sbx_caladan3_etu_debug_set(int unit, int level)
{
    etu_debug = level;
}

/**
 **
 **    Program Memory Setup
 **       512 Entries are programmed to allow LRP to execute any valid operation
 **       ndw = num of dw, app0 = CB, app1:app2 = opcode
 **       ns = Notsearch and ut = UseTag
 **
 */
int
soc_sbx_caladan3_etu_prog_memory_set(int unit, int idx, int keylen, int ns, int ut,
                                     uint32 app0, uint32 app1, uint32 app2)
{
    prog_mem_entry_t data;

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, PROG_MEMm,
                                     MEM_BLOCK_ANY, idx, &data));
    soc_mem_field_set(unit, PROG_MEMm, &data.entry_data[0], USE_TAGIDf, (uint32*)&ut);
    soc_mem_field_set(unit, PROG_MEMm, &data.entry_data[0], NOT_SEARCHf, (uint32*)&ns);
    soc_mem_field32_set(unit, PROG_MEMm, &data.entry_data[0], IL_EOPf, 8);
    soc_mem_field32_set(unit, PROG_MEMm, &data.entry_data[0], IL_CHAN_NUMf, 0);
    soc_mem_field32_set(unit, PROG_MEMm, &data.entry_data[0], FC_CHAN_1f, 0);
    soc_mem_field32_set(unit, PROG_MEMm, &data.entry_data[0], DW_REQUIREDf, (keylen >> 6));
    soc_mem_field_set(unit, PROG_MEMm, &data.entry_data[0], APP2f, &app2);
    soc_mem_field_set(unit, PROG_MEMm, &data.entry_data[0], APP1f, &app1);
    soc_mem_field_set(unit, PROG_MEMm, &data.entry_data[0], APP0f, &app0);

    SOC_IF_ERROR_RETURN(soc_mem_write(unit, PROG_MEMm,
                                     MEM_BLOCK_ANY, idx, &data));
    return SOC_E_NONE;
}


/**
 **
 **    FIFO Management
 **
 */

/*
 * Function:
 *    soc_sbx_caladan3_etu_fifo_init
 * Purpose:
 *    Initialize the ETU CP FIFO
 */
int
soc_sbx_caladan3_etu_fifo_init(int unit)
{
    int opcode, keylen, idx = 0, ltr;
    uint32 regval = 0;
    WRITE_ETU_CP_FIFO_INTR_ENABLEr(unit, regval);
    WRITE_ETU_CP_FIFO_INTR_CLRr(unit, regval);
    WRITE_ETU_TX_REQ_FIFO_INTR_ENABLEr(unit, regval);
    WRITE_ETU_TX_REQ_FIFO_INTR_CLRr(unit, regval);
    WRITE_ETU_TX_PIPE_CTL_FIFO_INTR_ENABLEr(unit, regval);
    WRITE_ETU_TX_PIPE_CTL_FIFO_INTR_CLRr(unit, regval);
    WRITE_ETU_RX_RSP_FIFO_INTR_ENABLEr(unit, regval);
    WRITE_ETU_RX_RSP_FIFO_INTR_CLRr(unit, regval);

    /* Setup ETU PROG_MEM for search ops
     *    ETU Prog number derivation for ucode use 
     *        Given keylen & ltr, => EPROG = (ltr * 4) + (keylen / 160)
     *        =>     640b & ltr, => EPROG = (ltr * 4) + 4
     *               480b & ltr, => EPROG = (ltr * 4) + 3
     *               320b & ltr, => EPROG = (ltr * 4) + 2
     *               160b & ltr, => EPROG = (ltr * 4) + 1
     *        Other valid sizes are 80b, 240b, 400b, 560b,
     *        This code doesnt setup all possible combinations since the prog_mem is limited.
     *    DBWrite
     *        EPROG = 0 
     */
    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, PROG_MEMm, COPYNO_ALL, FALSE));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Unit %d, C3 XPT Setting up PROG_MEM \n"), unit));

    /* DB write 80B data + 80B mask */
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Unit %d DBWrite at Index (%d)\n"), unit, idx));
    soc_sbx_caladan3_etu_prog_memory_set(unit, idx++, 160, 1,
                                         1, 0 /* use tag as CB */,
                                         SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_WRITE,
                                         0);
    for (ltr=0; ltr<64; ltr++) {
        for (keylen=160;  keylen <= 640; keylen+=160) {
            opcode = (keylen <= 320) ? SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP1 :
                                       SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP2;
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Unit %d %s keylen(%d) with LTR(%d) at Index (%d)\n"), unit,
                         (opcode == SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP2) ? "CMP2" : "CMP1",
                         keylen, ltr, idx));
            soc_sbx_caladan3_etu_prog_memory_set(unit, idx++, keylen, 0,
                                                 1, 0 /* use tag as CB */,
                                                 ltr,
                                                 opcode);
        }
    }

    return SOC_E_NONE;
}

void
soc_sbx_caladan3_etu_tcam_pack_cw_extended(uint32 *cw, int mode, uint8 opcode, 
                                           uint32 cxt_buf_addr, uint8 cmd, int capture)
{
    cw[0] = ((cxt_buf_addr << 9) | opcode);
    cw[1] = (((cmd & 0x3) << 9) | (capture << 8) | (mode << 7));
}

static  void
soc_sbx_caladan3_etu_tcam_pack_cw(uint32 *cw, int mode, uint8 opcode, uint32 cxt_buf_addr) 
{
    cw[0] = ((cxt_buf_addr << 9) | opcode);
}

void
soc_sbx_caladan3_etu_tcam_pack_dw(uint32 *dw, int mode, uint32 addr, uint8 *datax, uint8 *masky)
{
    dw[0] = PACK_ADDR(mode, addr);
    if (datax) {
        dw[1] = PACK_WORD(mode, datax[0], datax[1], datax[2], datax[3]);
        dw[2] = PACK_WORD(mode, datax[4], datax[5], datax[6], datax[7]);
        dw[3] = PACK_WORD(mode, datax[8], datax[9], 0, 0);
    }
    if (masky) {
        dw[3] |= PACK_WORD(mode, 0, 0, masky[0], masky[1]);
        dw[4] = PACK_WORD(mode, masky[2], masky[3], masky[4], masky[5]);
        dw[5] = PACK_WORD(mode, masky[6], masky[7], masky[8], masky[9]);
    }
}

static  void
soc_sbx_caladan3_etu_tcam_pack_search_dw(uint32 *dw, int mode, uint8 *data, int num_dw)
{
    int i, j;
    for (i=0; i < num_dw*2; i+=2) 
    {
        j = i << 2;
        dw[i+1] = PACK_WORD(mode, data[j+0], data[j+1], data[j+2], data[j+3]);
        dw[i+0] = PACK_WORD(mode, data[j+4], data[j+5], data[j+6], data[j+7]);
    }
}

static  void
soc_sbx_caladan3_etu_tcam_unpack_rsp(uint32 *rsp, uint8 *data, uint8 *vbit, uint8 *devid)
{
    int i = 0;

    if (data) {
        data[i++] = (rsp[1] >> 24) & 0xFF;
        data[i++] = (rsp[1] >> 16) & 0xFF;
        data[i++] = (rsp[1] >>  8) & 0xFF;
        data[i++] = (rsp[1]) & 0xFF;
    
        data[i++] = (rsp[0] >> 24) & 0xFF;
        data[i++] = (rsp[0] >> 16) & 0xFF;
        data[i++] = (rsp[0] >>  8) & 0xFF;
        data[i++] = (rsp[0]) & 0xFF;

        data[i++] = (rsp[3] >> 24) & 0xFF;
        data[i++] = (rsp[3] >> 16) & 0xFF;
        data[i++] = (rsp[3] >>  8) & 0xFF;
        data[i++] = (rsp[3]) & 0xFF;

        data[i++] = (rsp[2] >> 24) & 0xFF;
        data[i++] = (rsp[2] >> 16) & 0xFF;
        data[i++] = (rsp[2] >>  8) & 0xFF;
        data[i++] = (rsp[2] ) & 0xFF;
    }
    if (vbit) {
        *vbit = (rsp[3] >> 12) & 1;
    }
    if (devid) {
        *devid = (rsp[3] >> 8) & 3;
    }
}

static  void
soc_sbx_caladan3_etu_tcam_db_rsp_xy_to_data(uint8 *data, uint8 *mask)
{
#ifdef USE_XY_DECODE_UNTESTED
    int i;
    uint8 t;
    for(i=0; i < SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX*4; i++) {
        t = ~(data[i] ^ mask[i]);
        mask[i] = t;
    }
#endif
}



static  int
soc_sbx_caladan3_etu_tcam_unpack_search_rsp(uint32 *rsp, etu_tcam_result_t *results, int num_results)
{
    int i = 0;
    if (!results || (num_results < 0) || (num_results > 4))  {
        return SOC_E_PARAM;
    }
    results[i].result = rsp[1] & 0x7FFFFF;
    results[i].hit = (( rsp[1] >> 30) & 1);
    results[i].valid = (( rsp[1] >> 31) & 1);

    if (i < num_results) i++;
    results[i].result = rsp[0] & 0x7FFFFF;
    results[i].hit = (( rsp[0] >> 30) & 1);
    results[i].valid = (( rsp[0] >> 31) & 1);

    if (i < num_results) i++;
    results[i].result = rsp[3] & 0x7FFFFF;
    results[i].hit = (( rsp[3] >> 30) & 1);
    results[i].valid = (( rsp[3] >> 31) & 1);

    if (i < num_results) i++;
    results[i].result = rsp[2] & 0x7FFFFF;
    results[i].hit = (( rsp[2] >> 30) & 1);
    results[i].valid = (( rsp[2] >> 31) & 1);

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_wait_for_fifo_response
 * Purpose:
 *    Wait for the response
 */
int
soc_sbx_caladan3_etu_wait_for_fifo_response(int unit)
{
    soc_timeout_t to;
    uint32 usecs, rval;
    int rv = SOC_E_NONE;

    if (SAL_BOOT_SIMULATION) {
        usecs = 10000000;
    } else {
        usecs = 50000;
    }
    soc_timeout_init(&to, usecs, 0);

    do {
        SOC_IF_ERROR_RETURN(READ_ETU_GLOBAL_INTR_STSr(unit, &rval));
        if (soc_reg_field_get(unit, ETU_GLOBAL_INTR_STSr, rval, 
                                          CP_FIFO_CAPTURE_DONEf)) {
            rv = SOC_E_NONE;
            break;
        }
        if (soc_timeout_check(&to)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "unit %d : ETU CP FIFO timeout\n"), unit));
            rv = SOC_E_TIMEOUT;
            break;
        }
    } while (TRUE);

    return rv;;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_wait_for_raw_response
 * Purpose:
 *    Wait for the raw response
 */
int
soc_sbx_caladan3_etu_wait_for_raw_response(int unit)
{
    soc_timeout_t to;
    uint32 usecs, rval;
    int rv = SOC_E_NONE;

    if (SAL_BOOT_SIMULATION) {
        usecs = 10000000;
    } else {
        usecs = 50000;
    }
    soc_timeout_init(&to, usecs, 0);

    do {
        SOC_IF_ERROR_RETURN(READ_ETU_GLOBAL_INTR_STSr(unit, &rval));
        if (soc_reg_field_get(unit, ETU_GLOBAL_INTR_STSr, rval, 
			      TX_RAW_REQ_DONEf)) {
            rv = SOC_E_NONE;
            break;
        }
        if (soc_timeout_check(&to)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "unit %d : ETU CP FIFO timeout\n"), unit));
            rv = SOC_E_TIMEOUT;
            break;
        }
    } while (TRUE);

    return rv;;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_fifo_access
 * Purpose:
 *    Write an entry into the ETU Fifo, read the response if interested.
 */
int
soc_sbx_caladan3_etu_fifo_access(int unit, uint32 mode, uint32 *cw,
				 uint32 *data, uint32 num_bytes, uint32 *response, uint32 write) 
{
#define ETU_MAX_CP_FIFO_CACHE 4
    int i, rv = SOC_E_NONE;
    uint8 *p;
    uint32 dw[2] = {0};
    etu_cp_fifo_entry_t *current_cp_entry;
    etu_dbg_cp_fifo_rsp_entry_t rsp_entry;
    etu_tx_raw_req_data_word_entry_t raw_entry;
    etu_dbg_rx_raw_rsp_entry_t raw_rsp_entry;
    uint32 get_rsp, eop, cmd_type;
    uint32 rval, num_dw = (num_bytes+7)>>3;
    int flush = FALSE;
    etu_cp_fifo_entry_t *cp_entry = cp_entries[unit];
    uint32 *cp_ptr = NULL;

    /* alloc dma buffer if not allocated yet, it's being reused */
    if (cp_entry == NULL) {
	cp_entries[unit] = (etu_cp_fifo_entry_t *)soc_cm_salloc(unit, ETU_MAX_CP_FIFO_CACHE * sizeof(etu_cp_fifo_entry_t),
							"etu fifo access dma buffer");
	if (cp_entries[unit] == NULL) {
	    rv = SOC_E_MEMORY;
	    return rv;
	} else {
	    cp_entry = cp_entries[unit];
	}
    }

    if (write) {
	get_rsp = (etu_flush_count[unit] == (ETU_MAX_CP_FIFO_CACHE-1))?1:0;
    } else {
	get_rsp = (response != NULL) ? 1 : 0;
    }
    eop = 0x8;

    if (etu_debug) {
	LOG_CLI((BSL_META_U(unit,
                            "Fifo Req: CW  0x%08x 0x%08x\n"), cw[0], cw[1]));
	for (i=0; i< num_dw<<1; i+=2) {
	    LOG_CLI((BSL_META_U(unit,
                                "Fifo Req: DW%d 0x%08x 0x%08x\n"), i>>1, data[i], data[i+1]));
	}
	LOG_CLI((BSL_META_U(unit,
                            "Flags   : S: %d C: %d N: %d E: %d\n"), mode, get_rsp, num_dw, eop));
    }

    /* 
     * When we are not using raw interfaces and we are not expecting response
     * we can queue multiple fifo access into single DMA to improve performance.
     * we trigger flush when detect either raw access or get_rsp is 1 or reach max cache Or 
     * as requested by caller (only db_write can accumulate or force flush now, other
     * commands are all get_rsp == 1.
     */
    if ((num_dw > 3) || (get_rsp) || (etu_flush_count[unit] == (ETU_MAX_CP_FIFO_CACHE-1))) {
	/* we have to flush the accumulated commands */
	flush = TRUE;
    } else {
	/* accumulate commands */
	flush = FALSE;
    }

    current_cp_entry = &cp_entry[etu_flush_count[unit]];

    if (num_dw > 3) {
	/* have to use raw interface */
	sal_memset(&raw_entry, 0, sizeof(raw_entry));
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW0f, &data[0]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW1f, &data[2]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW2f, &data[4]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW3f, &data[6]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW4f, &data[8]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW5f, &data[10]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW6f, &data[12]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW7f, &data[14]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW8f, &data[16]);
	soc_ETU_TX_RAW_REQ_DATA_WORDm_field_set(unit, &raw_entry, DW9f, &data[18]);    
	rv = WRITE_ETU_TX_RAW_REQ_DATA_WORDm(unit, MEM_BLOCK_ALL, 0, &raw_entry);
	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Failed to write ETU raw req data words\n")));
	    return rv;
	}

	/* clear the previous raw request */
        SOC_IF_ERROR_RETURN(READ_ETU_GLOBAL_INTR_STSr(unit, &rval));
	soc_reg_field_set(unit, ETU_GLOBAL_INTR_STSr, &rval, TX_RAW_REQ_DONEf, 0);
	SOC_IF_ERROR_RETURN(WRITE_ETU_GLOBAL_INTR_STSr(unit, rval));	

	/* issue CP_FIFO raw request */
	cmd_type = 1;
	sal_memset(current_cp_entry, 0, sizeof(etu_cp_fifo_entry_t));
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, CWf, cw);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, EOPf, &eop);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, NUM_DWf, &num_dw);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, CAPTUREf, &get_rsp);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, NOT_SEARCHf, &mode);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, DW0f, &data[0]);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, DW1f, &data[2]);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, DW2f, &data[4]);
	soc_ETU_CP_FIFOm_field_set(unit, current_cp_entry, CMD_TYPEf, &cmd_type);

	/* once we have raw request, we always flush */
	if (etu_flush_count[unit] == 0) {
	    rv = WRITE_ETU_CP_FIFOm(unit, MEM_BLOCK_ALL, 0, (uint32*)cp_entry);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Failed to write ETU fifo entry\n")));
	    }	
	} else {
            /*    coverity[negative_returns : FALSE]    */
            rv = soc_mem_write_range(unit, ETU_CP_FIFOm,
				     MEM_BLOCK_ANY, 0, etu_flush_count[unit], 
				     (uint32*)cp_entry);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Failed to write multiple ETU fifo entry\n")));
	    }	  	    
	}

	if (SOC_SUCCESS(rv) && get_rsp) {
	    rv = soc_sbx_caladan3_etu_wait_for_raw_response(unit);
	}
	if (SOC_SUCCESS(rv) && get_rsp) {
	    p = (uint8*)response;
	    sal_memset(&raw_rsp_entry, 0, sizeof(etu_dbg_rx_raw_rsp_entry_t));
	    rv = READ_ETU_DBG_RX_RAW_RSPm(unit, MEM_BLOCK_ANY, 0, &raw_rsp_entry);
	    if (SOC_SUCCESS(rv)) {
		soc_ETU_DBG_RX_RAW_RSPm_field_get(unit, &raw_rsp_entry, DW0f, dw);
		if (etu_debug) {
		    LOG_CLI((BSL_META_U(unit,
                                        "Fifo Response DW0: %x %x\n"), dw[0], dw[1]));
		}
		sal_memcpy(p, &dw[0], 4); p+=4;
		sal_memcpy(p, &dw[1], 4); p+=4;
		soc_ETU_DBG_RX_RAW_RSPm_field_get(unit, &raw_rsp_entry, DW1f, dw);
		if (etu_debug) {
		    LOG_CLI((BSL_META_U(unit,
                                        "Fifo Response DW1: %x %x\n"), dw[0], dw[1]));
		}
		sal_memcpy(p, &dw[0], 4); p+=4;
		sal_memcpy(p, &dw[1], 4); p+=4;
	    } else {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Failed reading response\n")));
	    }
	}
    } else {
	/* use the CP_FIFO */
	cmd_type = 0;
	sal_memset(current_cp_entry, 0, sizeof(etu_cp_fifo_entry_t));

	cp_ptr = (uint32 *)current_cp_entry;
	cp_ptr[0] = cw[0];
	cp_ptr[1] = cw[1];
	cp_ptr[0] &= (~(0xFF<<23));
	cp_ptr[0] |= ((eop & 0xF) << 23);
	cp_ptr[0] |= ((num_dw & 0xF) << 27);
	cp_ptr[1] &= (~(0xF<<7));
	cp_ptr[1] |= ((mode?1:0) << 7);
	cp_ptr[1] |= ((get_rsp?1:0) << 8);
	cp_ptr[1] |= ((cmd_type & 0x3) << 9);
	cp_ptr[2] &= (~(0x1FFFFF << 11));
	cp_ptr[2] |= ((data[0] & 0x1FFFFF) << 11);
	cp_ptr[3] = (((data[0] & 0xFFE00000) >> 21) | ((data[1] & 0x1FFFFF)<< 11));
	cp_ptr[4] = (((data[1] & 0xFFE00000) >> 21) | ((data[2] & 0x1FFFFF)<< 11));
	cp_ptr[5] = (((data[2] & 0xFFE00000) >> 21) | ((data[3] & 0x1FFFFF)<< 11));
	cp_ptr[6] = (((data[3] & 0xFFE00000) >> 21) | ((data[4] & 0x1FFFFF)<< 11));
	cp_ptr[7] = (((data[4] & 0xFFE00000) >> 21) | ((data[5] & 0x1FFFFF)<< 11));
	cp_ptr[8] &= (~0x7FF);
	cp_ptr[8] |= ((data[5] & 0xFFE00000) >> 21);
	
	if (flush) {
	    /* we are not accumulating, so issue commands */
	    if (etu_flush_count[unit] == 0) {
		rv = WRITE_ETU_CP_FIFOm(unit, MEM_BLOCK_ALL, 0, (uint32*)cp_entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Failed to write ETU fifo\n")));
		}	
	    } else {
                /*    coverity[negative_returns : FALSE]    */
		rv = soc_mem_write_range(unit, ETU_CP_FIFOm,
					 MEM_BLOCK_ANY, 0, etu_flush_count[unit], 
					 (uint32*)cp_entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Failed to write multiple ETU fifo entry\n")));
		}	  	    
	    }
	    
	    if (SOC_SUCCESS(rv) && get_rsp) {
		rv = soc_sbx_caladan3_etu_wait_for_fifo_response(unit);
	    }
	    
	    if (SOC_SUCCESS(rv) && get_rsp) {
		p = (uint8*)response;
		sal_memset(&rsp_entry, 0, sizeof(etu_dbg_cp_fifo_rsp_entry_t));
		rv = READ_ETU_DBG_CP_FIFO_RSPm(unit, MEM_BLOCK_ANY, 0, &rsp_entry);
		if (SOC_SUCCESS(rv)) {
		    soc_ETU_DBG_CP_FIFO_RSPm_field_get(unit, &rsp_entry, DW0f, dw);
		    if (etu_debug) {
			LOG_CLI((BSL_META_U(unit,
                                            "Fifo Response DW0: %x %x\n"), dw[0], dw[1]));
		    }
		    sal_memcpy(p, &dw[0], 4); p+=4;
		    sal_memcpy(p, &dw[1], 4); p+=4;
		    soc_ETU_DBG_CP_FIFO_RSPm_field_get(unit, &rsp_entry, DW1f, dw);
		    if (etu_debug) {
			LOG_CLI((BSL_META_U(unit,
                                            "Fifo Response DW1: %x %x\n"), dw[0], dw[1]));
		    }
		    sal_memcpy(p, &dw[0], 4); p+=4;
		    sal_memcpy(p, &dw[1], 4); p+=4;
		} else {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Failed reading response\n")));
		}
	    }
	}
    }

    /* increment flush count if didn't flush */
    if (flush) {
	etu_flush_count[unit] = 0;
    } else {
	etu_flush_count[unit]++;
    }

    return rv;
}


/**
 **
 ** TCAM Fifo read write accessor
 **
 **/

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_reg_read
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_reg_read(int unit, int devid, int reg, uint8 *data)
{
    int rv = SOC_E_NONE, address = 0, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;

    if (!data) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_reg_read: Invalid parameter data\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Reg Read operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    address = ENCODE_ADDR(SOC_SBX_CALADAN3_ETU_TCAM_REG_ACCESS, devid, reg, 0, 0);
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_REG_READ;
    mode = SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP;
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
    soc_sbx_caladan3_etu_tcam_pack_dw(dw, mode, address, 0, 0);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 16, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, data, 0 ,0);
    }

    return rv;   
    
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_reg_write
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_reg_write(int unit, int devid, int reg, uint8 *data, uint8 *response)
{
    int rv = SOC_E_NONE, address = 0, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;

    if (!data) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_reg_write: Invalid parameter data\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Reg write operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    address = ENCODE_ADDR(SOC_SBX_CALADAN3_ETU_TCAM_REG_ACCESS, devid, reg, 0, 0);
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_REG_WRITE;
    mode = SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP;
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
    soc_sbx_caladan3_etu_tcam_pack_dw(dw, mode, address, data, 0);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 24, rsp, FALSE);
    if (SOC_SUCCESS(rv) && response) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, response, 0, 0);
    }
 
    return rv;   
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_db_read_datax
 * Purpose:
 *    Read a TCAM data entry
 */
int
soc_sbx_caladan3_etu_tcam_db_read_datax(int unit, int devid, int index, uint8 *data, uint8 *valid)
{
    int rv = SOC_E_NONE, address = 0, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;

    if (!data) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_db_read_datax: Invalid parameter data\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM DB data read operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    address = ENCODE_ADDR(SOC_SBX_CALADAN3_ETU_TCAM_DB_ACCESS, devid, index, 0, 0);
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_DATA_OR_X_READ;
    mode = SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP;
    mode = etu_mode;
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
    soc_sbx_caladan3_etu_tcam_pack_dw(dw, mode, address, 0, 0);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 16, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, data, valid, 0);
    }

    return rv;   
}


/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_db_read_masky
 * Purpose:
 *    Read a TCAM mask entry
 */
int
soc_sbx_caladan3_etu_tcam_db_read_masky(int unit, int devid, int index, uint8 *mask)
{
    int rv = SOC_E_NONE, address = 0, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;

    if (!mask) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_db_read_masky: Invalid parameter mask\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "Tcam Mask read operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    address = ENCODE_ADDR(SOC_SBX_CALADAN3_ETU_TCAM_DB_ACCESS, devid, index, 0, 0);
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_MASK_OR_Y_READ;
    mode = SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP;
    mode = etu_mode;
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
    soc_sbx_caladan3_etu_tcam_pack_dw(dw, mode, address, 0, 0);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 16, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, mask, 0, 0);
    }

    return rv;   
}


/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_db_read
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_db_read(int unit, int devid, int index, uint8 *data, uint8 *mask, uint8 *valid)
{
    int rv = SOC_E_NONE, address = 0, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;

    if (!data) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_db_read: Invalid parameter data\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM DB read operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    address = ENCODE_ADDR(SOC_SBX_CALADAN3_ETU_TCAM_DB_ACCESS, devid, index, 0, 0);
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_DATA_OR_X_READ;
    mode = SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP;
    mode = etu_mode;
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
    soc_sbx_caladan3_etu_tcam_pack_dw(dw, mode, address, 0, 0);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 16, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, data, valid, 0);

        /* Get the mask */
        opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_MASK_OR_Y_READ;
        soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
        rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 16, rsp, FALSE);
        if (SOC_SUCCESS(rv)) {
            soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, mask, 0, 0);
            soc_sbx_caladan3_etu_tcam_db_rsp_xy_to_data(data, mask);
        }
    }

    return rv;   
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_db_write
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_db_write(int unit, int devid, int index, uint8 *data, uint8 *mask,
				   int valid, int wr_mode, uint8 *response)
{
    int rv = SOC_E_NONE, address = 0, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;

    if (!data) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_reg_read: Invalid parameter data\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM DB write operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    address = ENCODE_ADDR(SOC_SBX_CALADAN3_ETU_TCAM_DB_ACCESS, devid, index, wr_mode, valid);
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_WRITE;
    mode = SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP;
    mode = etu_mode;
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, 0);
    soc_sbx_caladan3_etu_tcam_pack_dw(dw, mode, address, data, mask);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, 24, response?rsp:NULL, TRUE);

    if (SOC_SUCCESS(rv) && response) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, response, 0, 0);
    }
    return rv;   
    
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_db_lookup
 * Purpose:
 *    Read a TCAM register
 */

int
soc_sbx_caladan3_etu_tcam_db_lookup(int unit, etu_tcam_lookup_t *lkup, etu_tcam_result_t *results)
{
    int rv = SOC_E_NONE, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 cxt_buffer_addr;
    uint32 mode = 0;
    int num_dw, num_bytes;

    opcode = lkup->key_width_in_bits < 320 ? 
                  SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP1 : 
                  SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP2;
    opcode = ((opcode << 6) | (lkup->ltr & 0x3F));
    cxt_buffer_addr = SOC_SBX_CALADAN3_ETU_LOOKUP_CXT_BUFFER_ADDR;
    mode = SOC_SBX_CALADAN3_ETU_NO_HW_BYTE_SWAP;
    num_dw =  KEY_WIDTH_IN_64B_WORDS(lkup->key_width_in_bits);
    num_bytes =  KEY_WIDTH_IN_BYTES(lkup->key_width_in_bits);
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM lookup operation\n")));
    }

    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, cxt_buffer_addr);
    soc_sbx_caladan3_etu_tcam_pack_search_dw(dw, mode, lkup->key, num_dw);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, num_bytes, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_search_rsp(rsp, results, lkup->num_results);
    }

    return rv;   
    
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_compare
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_compare(int unit, uint8 ltr, int cxt_buffer_addr,
				  uint8 *key, uint8 cmp1, int sz,
				  uint8 *response)
{
    int rv = SOC_E_NONE, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;
    int num_dw, num_bytes;

    if (!key) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_compare: Invalid parameter key\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM compare operation\n")));
    }

    opcode = (cmp1) ? 
	SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP1 : 
	SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP2;
    opcode = ((opcode << 6) | (ltr & 0x3F));

    mode = SOC_SBX_CALADAN3_ETU_NO_HW_BYTE_SWAP;
    num_dw = KEY_WIDTH_IN_64B_WORDS(sz);
    num_bytes = KEY_WIDTH_IN_BYTES(sz);

    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, cxt_buffer_addr);
    soc_sbx_caladan3_etu_tcam_pack_search_dw(dw, mode, key, num_dw); 

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, num_bytes, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, response, 0 ,0);
    }
    return rv; 
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_cxtbuf_write
 * Purpose:
 *    Nop
 */
int
soc_sbx_caladan3_etu_tcam_cxtbuf_write(int unit, uint32 cxtaddr, 
                                       uint8 *data, int keysize)
{
    int rv = SOC_E_NONE, opcode = 0;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};
    uint32 mode = 0;
    int num_dw, num_bytes;

    if (!data) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_etu_tcam_cb_write: Invalid parameter data\n")));
        return SOC_E_PARAM;
    }
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM CBuf Write operation\n")));
    }

    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    opcode = SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CXTBUF_WRITE;
    mode = SOC_SBX_CALADAN3_ETU_NO_HW_BYTE_SWAP;
    num_dw = KEY_WIDTH_IN_64B_WORDS(keysize);
    num_bytes = KEY_WIDTH_IN_BYTES(keysize);
    soc_sbx_caladan3_etu_tcam_pack_cw(cw, mode, opcode, cxtaddr);
    soc_sbx_caladan3_etu_tcam_pack_search_dw(dw, mode, data, num_dw);

    rv = soc_sbx_caladan3_etu_fifo_access(unit, mode, cw, dw, num_bytes, rsp, FALSE);

    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_nop
 * Purpose:
 *    Nop
 */
int
soc_sbx_caladan3_etu_tcam_nop(int unit, uint8 *response)
{
    int rv = SOC_E_NONE;
    uint32 cw[SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX] = {0};
    uint32 dw[SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX] = {0};
    uint32 rsp[SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX] = {0};

    sal_memset(cw, 0, sizeof(cw));
    sal_memset(dw, 0, sizeof(dw));
    sal_memset(rsp, 0, sizeof(rsp));
    if (etu_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "TCAM Nop operation\n")));
    }

    rv = soc_sbx_caladan3_etu_fifo_access(unit, 0, cw, dw, 16, rsp, FALSE);

    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_tcam_unpack_rsp(rsp, response, 0 ,0);
    }
    return rv;
}




/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_raw_reg_read
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_raw_reg_read(int unit, int devid, int reg, uint8 *data)
{
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_raw_reg_write
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_raw_reg_write(int unit, int devid, int reg, uint8 *data)
{
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_raw_db_read
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_raw_db_read(int unit, int devid, uint32 *data, uint32 *mask, int *valid)
{
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_raw_db_write
 * Purpose:
 *    Read a TCAM register
 */
int
soc_sbx_caladan3_etu_tcam_raw_db_write(int unit, int devid, uint32 *data, uint32 *mask, int valid)
{
    return SOC_E_UNAVAIL;
}




/**
 ** 
 **  FIFO manager
 **
 **/

#endif  /* BCM_CALADAN3_SUPPORT */
