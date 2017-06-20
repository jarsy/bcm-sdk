/*
 * $Id: etu_tcam.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        tcam.c
 * Purpose:     external TCAM access
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/defs.h>

#ifdef BCM_CALADAN3_SUPPORT

/* Derived from esmif.c */

#include <soc/drv.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/mem.h>
#include <soc/sbx/caladan3/etu.h>
#include <soc/sbx/caladan3/etu_tcam.h>



/* Device register map */
#define NL_REG_ADDR_DEV_ID                      0x00000
#define NL_REG_ADDR_DEV_CFG                     0x00001
#define NL_REG_ADDR_ERR_STS                     0x00002
#define NL_REG_ADDR_ERR_STS_MSK                 0x00003
#define NL_REG_ADDR_DB_SOFT_ERR_FIFO            0x00005
#define NL_REG_ADDR_ADV_FEATURES_SOFT_ERR       0x00019
#define NL_REG_ADDR_LPM_ENABLE                  0x00020
#define NL_REG_ADDR_CONTEXT_BUFFER_BASE         0x08000
#define NL_REG_ADDR_SCRATCHPAD_REG0             0x80000
#define NL_REG_ADDR_SCRATCHPAD_REG1             0x80001
#define NL_REG_ADDR_RESULT_REG0                 0x80010
#define NL_REG_ADDR_RESULT_REG1                 0x80011
#define NL_REG_ADDR_RANGE_CONTROL_BASE          0x85000
#define NL_REG_ADDR_LTR_BLOCK_SEL_REG0(ltr)     (0x04000 + ((ltr) << 5))
#define NL_REG_ADDR_LTR_BLOCK_SEL_REG1(ltr)     (0x04001 + ((ltr) << 5))
#define NL_REG_ADDR_LTR_MISCELLANEOUS_REG(ltr)  (0x04009 + ((ltr) << 5))


int
soc_etu_nl_mdio_read(int      unit,   
             unsigned mdio_portid,
             unsigned mdio_dev_id, 
             unsigned mdio_addr,
             uint16   *rd_data
             )
{
    _SHR_RETURN(soc_miimc45_read(unit, mdio_portid, mdio_dev_id, mdio_addr, rd_data));
}


int
soc_etu_nl_mdio_write(int      unit,   
              unsigned mdio_portid,
              unsigned mdio_dev_id,
              unsigned mdio_addr,
              uint16   wr_data,
              unsigned verify_wr
              )
{

    SOC_IF_ERROR_RETURN(soc_miimc45_write(unit, mdio_portid, mdio_dev_id, mdio_addr, wr_data));
   
    if (verify_wr) {
        uint16 rd_data;

        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit,
                                         mdio_portid,
                                         mdio_dev_id,
                                         mdio_addr,
                                         &rd_data
                                         )
                            );

        if (rd_data != wr_data)  _SHR_RETURN(SOC_E_FAIL);
    }

    _SHR_RETURN(SOC_E_NONE);
}


int
soc_etu_nl_mdio_test_reg_access(int unit, unsigned mdio_portid)
{
    const unsigned mdio_dev_id = 2;

    unsigned errcnt = 0;
    uint16   val;
    /* Will use following mdio regs for this test:
       RX SerDes - PRBS Control and Status Registers, MDIO Device ID = 2 to 10, see Table 24
    */

    /* Verify default value for registers */

    if (SOC_FAILURE(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x12, &val))
        || val != 0xaaaa
        ) {
        ++errcnt;
    }
    if (SOC_FAILURE(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x13, &val))
        || val != 0xaaaa
        ) {
        ++errcnt;
    }

    /* Test write_verify */

    if (SOC_FAILURE(soc_etu_nl_mdio_write(unit, mdio_portid, mdio_dev_id, 0x12, 0x5555, 1))) {
        ++errcnt;
    }
    if (SOC_FAILURE(soc_etu_nl_mdio_write(unit, mdio_portid, mdio_dev_id, 0x13, 0x1234, 1))) {
        ++errcnt;
    }

    /* Make sure our above writes went to different registers */

    if (SOC_FAILURE(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x12, &val))
        || val != 0x5555
        ) {
        ++errcnt;
    }
    if (SOC_FAILURE(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x13, &val))
        || val != 0x1234
        ) {
        ++errcnt;
    }

    /* Restore default values */

    if (SOC_FAILURE(soc_etu_nl_mdio_write(unit, mdio_portid, mdio_dev_id, 0x12, 0xaaaa, 1))) {
        ++errcnt;
    }
    if (SOC_FAILURE(soc_etu_nl_mdio_write(unit, mdio_portid, mdio_dev_id, 0x13, 0xaaaa, 1))) {
        ++errcnt;
    }
    
    _SHR_RETURN(errcnt ? SOC_E_FAIL : SOC_E_NONE);
}



static int
nl_mdio_chk_serdes_reset_seq_done(int unit, unsigned mdio_portid)
{
    uint16 val;

    SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit, mdio_portid, 1, 0x8183, &val));
    return (val & (1 << 3) ? SOC_E_NONE : SOC_E_BUSY);
}


static int
nl_mdio_poll_serdes_reset_seq_done(int unit, unsigned mdio_portid)
{
    unsigned n;

    for (n = 1000; n; --n) {
        int errcode = nl_mdio_chk_serdes_reset_seq_done(unit, mdio_portid);

        if (errcode == SOC_E_BUSY) {
            sal_usleep(1000);       /* Wait 1 ms */
            continue;
        }

        _SHR_RETURN(errcode);
    }

    _SHR_RETURN(SOC_E_TIMEOUT);
}


static int
nl_mdio_clr_csm_status_regs(int unit, unsigned mdio_portid)
{
    static const uint16 mdio_regs[] = {
        0x8180,
        0x8183,
        0x8184,
        0x8185,
        0x8186,
        0x8187,
        0x8188,
        0x8189,
        0x818a        
    };

    const unsigned mdio_dev_id = 1;

    unsigned i;

    for (i = 0; i < COUNTOF(mdio_regs); ++i) {
        uint16 dummy;

        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, mdio_regs[i], &dummy));
    }

    _SHR_RETURN(SOC_E_NONE);
}


static int
nl_mdio_chk_csm_status_regs(int unit, unsigned mdio_portid, unsigned chk_crx)
{
    static const struct {
        uint8  crxf;
        uint16 reg_addr;
        uint16 exp_val;
    } regvals[] = {
        { 0, 0x8180,   0 },
        { 0, 0x8184,   0 },
        { 1, 0x8185,   0 },
        { 0, 0x8186,   0 },
        { 0, 0x8187,   0 },
        { 1, 0x8188,   0 },
        { 0, 0x8189,   0 },
        { 0, 0x818a,   0 }
    };

    const unsigned mdio_dev_id = 1;

    unsigned i;
    uint16   regval;
    int      rv = 0;

    for (i = 0; i < COUNTOF(regvals); ++i) {
        if (regvals[i].crxf && !chk_crx) {
            continue;
        }
        
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, regvals[i].reg_addr, &regval));
        if (regval != regvals[i].exp_val) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit: %d Dev: %d CSM Status error reg %x val %x\n"),
                       unit, mdio_dev_id, regvals[i].reg_addr, regval));
            _SHR_RETURN(SOC_E_FAIL);
        }
    }

    for (i=0; i<10; i++) {
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8183, &regval));
        if (regval != (chk_crx ? 0xf : 0xe)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit: %d Dev: %d CSM Status error reg %x val %x\n"),
                       unit, mdio_dev_id, 0x8183, regval));
            rv = SOC_E_FAIL;
        } else {
            rv = SOC_E_NONE;
        }
    }

    _SHR_RETURN(rv);
}


static int
nl_mdio_chk_csm_error_counters(int unit, unsigned mdio_portid, unsigned chk_crx)
{
    static const uint16 regaddrs[] = {
        0x8203,
        0x8204,
        0x8205,
        0x8284,
        0x8285,
        0x8288,
        0x8289,
        0x828a,
        0x828b,
        0x8290,
        0x8291,
        0x8292,
        0x8293        
    };
    char *msg[] = {
        "Rx Fifo Full CRX port",
        "Rx Fifo full 16-23",
        "Rx fifo full 00-15",
        "Rx Err packet count lsw",
        "Rx Err packet count msw",
        "Rx Crc err packet count lsw",
        "Rx Crc err packet count msw",
        "Num of ui_violations lsw",
        "Num of ui_violations msw",
        "Num rsp packets with err lsw",
        "Num rsp packets with err msw",        
        "Num rsp packets with Crc err lsw",
        "Num rsp packets with Crc err msw"        
    };


    const unsigned mdio_dev_id = 1;

    unsigned i;
    uint16   regval;

    for (i = 0; i < COUNTOF(regaddrs); ++i) {
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, regaddrs[i], &regval));
        if (regval != 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit: %d Dev: %d %s \n"), unit, mdio_dev_id, msg[i]));
            _SHR_RETURN(SOC_E_FAIL);
        }
    }

    _SHR_RETURN(SOC_E_NONE);
}


static int
nl_mdio_chk_pcs_error_status(int unit, unsigned mdio_portid, unsigned chk_crx)
{

    char *msg[] = {
        "RX PCS Status lock failed",
        "RX PCS Word Alignment Status",
        "RX PCS Word Error",
        "RX PCS Block Type Error",
        "RX PCS Metaframe",
        "RX PCS Descrambler Sync Loss Error",
        "RX PCS Descrambler Single Error",
        "RX PCS eFifo Error",
        "RX PCS Crc32 ERROR",
    };
    const unsigned min_mdio_addr   = 0x8300;
    const unsigned max_mdio_addr   = 0x8308;
    const unsigned min_mdio_dev_id = 2;
    const unsigned max_mdio_dev_id = chk_crx ? 10 : 7;

    int rv = 0;
    unsigned mdio_dev_id, mdio_addr;
    uint16   exp_val, obs_val;
    int rng = 0;

    for (mdio_dev_id = min_mdio_dev_id; mdio_dev_id <= max_mdio_dev_id; ++mdio_dev_id) {
        for (mdio_addr = min_mdio_addr; mdio_addr <= max_mdio_addr; ++mdio_addr) {
            exp_val = mdio_addr == 0x8300 ? 0xff0 : 0;
            rv  = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, mdio_addr, &obs_val);
            if (SOC_SUCCESS(rv) && (exp_val != obs_val)) {
                rng = mdio_addr-min_mdio_addr;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit: %d Dev:%d %s lanes %d-%d Reg %x Val:%x\n"), 
                           unit, mdio_dev_id, msg[rng], 
                           rng, rng+4, mdio_addr, obs_val));
                rv = SOC_E_FAIL;
            }
        }
    }

    return rv;
}


static int
nl_mdio_chk_pcs_error_counters(int unit, unsigned mdio_portid, unsigned chk_crx)
{
    const unsigned min_mdio_dev_id = 2,
        max_mdio_dev_id = chk_crx ? 10 : 7,
        min_mdio_addr   = 0x8380,
        max_mdio_addr   = 0x83a7;

    int      errcode = SOC_E_NONE;
    unsigned mdio_dev_id, mdio_addr;

    for (mdio_dev_id = min_mdio_dev_id; mdio_dev_id <= max_mdio_dev_id; ++mdio_dev_id) {
        for (mdio_addr = min_mdio_addr; mdio_addr <= max_mdio_addr; ++mdio_addr) {
            uint16 regval;

            SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, mdio_addr, &regval));
            if (regval != 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit: %d Dev:%d PCS error Ctr: %x Val:%x\n"),
                           unit, mdio_dev_id, mdio_addr, regval));

                errcode = SOC_E_FAIL;
            }
        }
    }

    _SHR_RETURN(errcode);
}

static int
nl_mdio_chk_csm_counters(int unit, unsigned mdio_portid, unsigned chk_crx)
{
    return (SOC_E_NONE);
}


int
soc_etu_nl_mdio_chk_error_counters_status(int unit, unsigned dev_id, unsigned chk_crx)
{
    const unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

    unsigned errcnt = 0;
    
    if (SOC_FAILURE(nl_mdio_chk_csm_status_regs(unit, mdio_portid, chk_crx))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "TCAM CSM Status check failed \n")));
        ++errcnt;
    }
    if (SOC_FAILURE(nl_mdio_chk_csm_error_counters(unit, mdio_portid, chk_crx))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "TCAM CSM errors detected\n")));
        ++errcnt;
    }
    if (SOC_FAILURE(nl_mdio_chk_csm_counters(unit, mdio_portid, chk_crx))) {
        ++errcnt;
    }
    if (SOC_FAILURE(nl_mdio_chk_pcs_error_status(unit, mdio_portid, chk_crx))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "TCAM PCS errors detected\n")));
        ++errcnt;
    }
    if (SOC_FAILURE(nl_mdio_chk_pcs_error_counters(unit, mdio_portid, chk_crx))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "TCAM PCS errors detected\n")));
        ++errcnt;
    }

    _SHR_RETURN(errcnt ? SOC_E_FAIL : SOC_E_NONE);
}

int
soc_etu_nl_mdio_print_csm_status(int unit, unsigned dev_id)
{
    int rv = SOC_E_NONE;
    uint16 regval = 0;
    uint16 mdio_dev_id = 1;
    int i;
    char *errmsg[] = {
                    "CRC 24 Error",
                    "SOP Error",
                    "EOP Error",
                    "No Data in the packet / Burst short violation",
                    "Burst Max Error",
                    "Alignment error",
                    "Framing Control Word Error",
                    "RxNMAC FIFO Parity Error",
                    "Instruction burst error",
                    "Protocol Error",
                    "Channel no Error",
                    "Burst Control Word Error",
                    "EFIFO Error",
                    "CRX UI Violation",
                    "RX UI Violation",
                    "TxNMAC FIFO Parity Error",
    };
    char *statusmsg[] = {
                    "CRX PCS Ready",
                    "RX PCS Ready",
                    "Core Init done",
                    "Serdes Reset Seq done"
    };

    const unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8183, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        for (i=0; i < 4; i++) {
            if (regval & (1<<i)) {
                LOG_CLI((BSL_META_U(unit,
                                    "CSM Status %s\n"), statusmsg[i]));
            }
        }
    }
    
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8180, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        for (i=0; i < 16; i++) {
            if (regval & (1<<i)) {
                LOG_CLI((BSL_META_U(unit,
                                    "CSM Error Status %s\n"), errmsg[i]));
            }
        }
    }
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8185, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        LOG_CLI((BSL_META_U(unit,
                            "CSM Status IDLE crc24 error CRX[11:0] = %x\n"), regval));
    }
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8186, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        LOG_CLI((BSL_META_U(unit,
                            "CSM Status IDLE crc24 error RX[23:16] = %x\n"), regval));
    }
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8187, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        LOG_CLI((BSL_META_U(unit,
                            "CSM Status IDLE crc24 error RX[15:0] = %x\n"), regval));
    }
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8188, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        LOG_CLI((BSL_META_U(unit,
                            "CSM Status Interface error CRX[11:0] = %x\n"), regval));
    }
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x8189, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        LOG_CLI((BSL_META_U(unit,
                            "CSM Status Interface error RX[23:16] = %x\n"), regval));
    }
    rv = soc_etu_nl_mdio_read(unit, mdio_portid, mdio_dev_id, 0x818A, &regval);
    if (SOC_SUCCESS(rv) && (regval != 0)) {
        LOG_CLI((BSL_META_U(unit,
                            "CSM Status Interface error RX[15:0] = %x\n"), regval));
    }

    return rv;
}

/*
 * Diag access to mdio space
 */
int
soc_etu_nk_mdio_register_access(int unit, int op, int portid, uint16 dev_id, uint16 regaddr, uint16 *regval)
{
    int rv = SOC_E_NONE;
    if (op) {
        rv = soc_etu_nl_mdio_read(unit, portid, dev_id, regaddr, regval);
    } else {
        rv = soc_etu_nl_mdio_write(unit, portid, dev_id, regaddr, *regval, 1);
    }
    return rv;
}

/*
 * Function: soc_etu_nl_mdio_init_ready
 * Purpose: Enable the TCAM serdes
 */
int
soc_etu_nl_mdio_init_ready(int unit, unsigned num_nl)
{
    const unsigned mdio_dev_id = 1;

    unsigned dev_id;
    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);
            
        /* Toggle "Reset RX Satellite Sticky Registers" bit */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x811b,
                                          0x40,
                                          1));
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x811b,
                                          0,
                                          1));
    }
        
    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned chk_crx     = (dev_id != 0);
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);
        SOC_IF_ERROR_RETURN(nl_mdio_clr_csm_status_regs(unit, mdio_portid));
        SOC_IF_ERROR_RETURN(nl_mdio_chk_csm_status_regs(unit, mdio_portid, chk_crx));
        SOC_IF_ERROR_RETURN(nl_mdio_chk_csm_error_counters(unit, mdio_portid, chk_crx));
        SOC_IF_ERROR_RETURN(nl_mdio_chk_pcs_error_status(unit, mdio_portid, chk_crx));
        SOC_IF_ERROR_RETURN(nl_mdio_chk_pcs_error_counters(unit, mdio_portid, chk_crx));
    }

    _SHR_RETURN(SOC_E_NONE);
}

/*
 * Function: soc_etu_nl_mdio_init_seq
 * Purpose: TCAM mdio init sequence
 *          Must be called with SRST_L asserted and CRST_L deasserted
 */
int
soc_etu_nl_mdio_init_seq(int unit,
                         unsigned num_nl,
                         unsigned rx_fifo_thr,
                         unsigned tx_fifo_thr,
                         unsigned rx_swap,
                         unsigned tx_swap,
                         unsigned tx_burst_short_16b)
{
    const unsigned mdio_dev_id = 1;

    unsigned dev_id;

    /* Device configuration using mdio (section 6.7)
       
       CSM - Config Regs, MDIO Devid=1, see Table 18 in NL spec
       - All regs are 16b wide
       NL's init seq: 4 Rx lanes, 2 Tx lanes
       See Table 17 in NL spec, it shows mapping of MDIO Device ID to SerDes lanes within NL
       This is must to understand when we want to enable prbs generators,
       checkers
    */
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "soc_etu_nl_mdio_init_seq: configuring %d tcams\n"), num_nl));
    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        /* TX_SerDes_11_0_squelch (Active high squelch for TX SerDes 11:0) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8117,
                                          0xfff,
                                          1));
        /* CTX_SerDes_15_0_squelch (Active high squelch for CTX SerDes 15:0) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8118,
                                          0xffff,
                                          1));
        /* CTX_SerDes_23_16_squelch (Active high squelch for CTX SerDes 23:16) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8119,
                                          0xff,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);
        
        /* Global RX / TX Enable (PCS enables), Bit[0] = RX PCS Enable, Bit[1] =
           TX PCS Enable
        */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x810c,
                                          0,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);
    
        /* Global SW Reset - ASSERT (only bits 2:0 are defined)
           Bit[2] SerDes Initialization Sequence Trigger,
           Bit[1] Reset Core Logic,
           Bit[0] Reset Core PLL
        */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x811c,
                                          7,
                                          1));
    }

    


    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);
        uint16 swap = 0;

        /* Lane Swap
           Bit[0] for RX lanes swap, Bit[0]=1 means Lane 0 and Lane 23, Lane 1 and Lane 22,
           ...  Lane 11 and Lane 12 are swapped
           Bit[1] for TX lanes swap, Bit[1]=1 means Lane 0 and Lane 11, Lane 1 and Lane 10,
           ...  Lane 5 and Lane 6 are swapped
        */
        swap = (((tx_swap > 0) ? 0x2 : 0x0) | ((rx_swap > 0) ? 0x1 : 0x0));
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8108,
                                          swap,
                                          1));
        /* RX Metaframe Length */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8106,
                                          SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_METAFRAME_LEN,
                                          1));
        /* TX Metaframe Length */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8107,
                                          SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_METAFRAME_LEN,
                                          1));
        /* RX (req) FIFO Threshold */
        if (rx_fifo_thr > 0) {
            SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                              mdio_portid,
                                              mdio_dev_id,
                                              0x8109,
                                              rx_fifo_thr,
                                              1));
        }
        /* TX (rsp) FIFO Threshold */
        if (tx_fifo_thr > 0) {
            SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                              mdio_portid,
                                              mdio_dev_id,
                                              0x810a,
                                              tx_fifo_thr,
                                              1));
        }
        /* TX Burst Short (0 means 8 bytes, 1 means 16 bytes) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x810b,
                                          tx_burst_short_16b ? 1 : 0,
                                          1));
        /* Speed mode select, bits 2:0 select speed for rx serdes, bits 6:4
           select speed for tx serdes  
           3'b100 means 6.250 Gbps, 3'b000 means 3.125 Gbps
        */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x811d,
                                          0x44,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        /* RX Lane Enable 0 (RX lane enable for lanes 15 to 0) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8100,
                                          0xffff,
                                          1));
        /* RX Lane Enable 1 (RX lane enable for lanes 23 to 16) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8101,
                                          0xff,
                                          1));
        /* TX Lane Enable 0 (TX lane enable for lanes 11 to 0) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8102,
                                          0xfff,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8104,
                                          dev_id == (num_nl - 1) ? 0 : 0xffff,
                                          1));
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8105,
                                          dev_id == (num_nl - 1) ? 0 : 0xff,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8103,
                                          dev_id ? 0xfff : 0,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        /* Global SW Reset - Deassert (only bits 2:0 are defined)
           Bit[2] SerDes Initialization Sequence Trigger,
           Bit[1] Reset Core Logic,
           Bit[0] Reset Core PLL
        */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x811c,
                                          0,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);
        
        SOC_IF_ERROR_RETURN(nl_mdio_poll_serdes_reset_seq_done(unit, mdio_portid));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        /* Global RX / TX Enable (PCS enables), Bit[0] = RX PCS Enable, Bit[1] =
           TX PCS Enable 
        */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x810c,
                                          3,
                                          1));
    }

    for (dev_id = 0; dev_id < num_nl; ++dev_id) {
        unsigned mdio_portid = soc_sbx_caladan3_mdio_portid_get(unit, dev_id);

        /* TX_SerDes_11_0_squelch (Active high squelch for TX SerDes 11:0) */
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8117,
                                          0,
                                          1));
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8118,
                                          dev_id == (num_nl - 1) ? 0xffff : 0,
                                          1));
        SOC_IF_ERROR_RETURN(soc_etu_nl_mdio_write(unit,
                                          mdio_portid,
                                          mdio_dev_id,
                                          0x8119,
                                          dev_id == (num_nl - 1) ? 0xff : 0,
                                          1));
    }

    _SHR_RETURN(SOC_E_NONE);
}


/**
 **
 ** TCAM register access through Serdes Interface
 **
 **/



#ifndef BIT
#define BIT(n)  (1U << (n))
#endif
#ifndef BITS
#define BITS(n)  ((n) == 32 ? ~0 : BIT(n) - 1)
#endif
#ifndef MIN
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#endif

static uint32
nl_reg_buf_bitfield_get(nl_reg_buf_t buf, unsigned ofs, unsigned width)
{
    unsigned sh = ofs & BITS(3), r = 8 - sh, n, rsh;
    uint8    *p, m;
    uint32   result;

    /* assert((ofs + width) <= COUNTOF(buf) * sizeof(buf[0])); */
    /* assert(width <= 32); */

    for (rsh = result = 0, p = &buf[ofs >> 3]; width; width -= n, ++p) {
        n = MIN(width, 8);
        m = BITS(n);
        
        result |= ((p[0] >> sh) & m) << rsh;
        rsh += r;
        if (n > r) {
            result |= (p[1] & (m >> r)) << rsh;
            rsh += n - r;
        }
    }

    return (result);
}

static void
nl_reg_buf_bitfield_set(nl_reg_buf_t buf, unsigned ofs, unsigned width, uint32 val)
{
    unsigned sh = ofs & BITS(3), r = 8 - sh, n;
    uint8    *p, m, u;

    /* assert((ofs + width) <= COUNTOF(buf) * sizeof(buf[0])); */
    /* assert(width <= 32); */
    /* assert(val <= BITS(width)); */

    for (p = &buf[ofs >> 3]; width; width -= n, ++p, val >>= 8) {
        n = MIN(width, 8);
        m = BITS(n);
        u = val & m;

        p[0] = (p[0] & ~(m << sh)) | (u << sh);
        if (n > r) {
            p[1] = (p[1] & ~(m >> r)) | (u >> r);
        }
    }
}

static unsigned
nl_reg_buf_is_zero(nl_reg_buf_t buf)
{
    uint8    *p;
    unsigned n;

    for (p = buf, n = NL_REG_BUF_SIZE; n; --n, ++p) {
        if (*p != 0)  return (0);
    }
    return (1);
}


static int
soc_etu_nl_reg_read(int unit, unsigned dev_id, uint32 reg_addr, uint8 *rd_data)
{
    int rv = SOC_E_NONE;
    rv = soc_sbx_caladan3_etu_tcam_reg_read(unit, dev_id, reg_addr, rd_data);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_etu_nl_reg_read: unit %d dev %d reg %x read failed\n"),
                   unit, dev_id, reg_addr));
    }
    return rv;
}


static int
soc_etu_nl_reg_write(int unit, unsigned dev_id, uint32 reg_addr, nl_reg_buf_t wr_data)
{
    int rv = SOC_E_NONE;
    rv = soc_sbx_caladan3_etu_tcam_reg_write(unit, dev_id, reg_addr, wr_data ,0);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_etu_nl_reg_read: unit %d dev %d reg %x read failed\n"),
                   unit, dev_id, reg_addr));
    }
    return rv;
}



int
soc_etu_nl_prog_err_status_mask_reg(int unit, unsigned dev_id)
{
    nl_reg_buf_t buf;
    nl_reg_buf_t rd_buf;

    sal_memset(buf, 0, sizeof(buf));
    nl_reg_buf_bitfield_set(buf, 0,   3,    0x7);
    nl_reg_buf_bitfield_set(buf, 5,   1,      1);
    nl_reg_buf_bitfield_set(buf, 16, 16, 0xbfff);
    nl_reg_buf_bitfield_set(buf, 48,  3,    0x7);
    nl_reg_buf_bitfield_set(buf, 79,  1,      1);
    SOC_IF_ERROR_RETURN(soc_etu_nl_reg_write(unit, dev_id, NL_REG_ADDR_ERR_STS_MSK, buf));

    /* Read back to verify */
    sal_memset(rd_buf, 0, sizeof(rd_buf));
    SOC_IF_ERROR_RETURN(soc_etu_nl_reg_read(unit, dev_id, NL_REG_ADDR_ERR_STS_MSK, rd_buf));
    if (sal_memcmp(rd_buf, buf, sizeof(buf)) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed setting interrupt mask\n")));
        return SOC_E_FAIL;
    }
    return SOC_E_NONE;
}

void
soc_etu_nl_print_register(int unit, char *regname,  nl_reg_buf_t buf) 
{
    LOG_CLI((BSL_META_U(unit,
                        "Unit %d Tcam "), unit));
    if (regname) {
        LOG_CLI((BSL_META_U(unit,
                            "%s Reg:"), regname));
    }
    LOG_CLI((BSL_META_U(unit,
                        "0x%08x "), nl_reg_buf_bitfield_get(buf, 64, 16)));
    LOG_CLI((BSL_META_U(unit,
                        "0x%08x "), nl_reg_buf_bitfield_get(buf, 32, 32)));
    LOG_CLI((BSL_META_U(unit,
                        "0x%08x "), nl_reg_buf_bitfield_get(buf, 0, 32)));
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
}

int
soc_etu_nl_chk_err_status_reg(int unit, unsigned dev_id)
{
    nl_reg_buf_t nl_reg_err_sts_buf ;
    int rv;

    sal_memset(nl_reg_err_sts_buf, 0, sizeof(nl_reg_err_sts_buf));
    rv = soc_etu_nl_reg_read(unit, dev_id, NL_REG_ADDR_ERR_STS, nl_reg_err_sts_buf);
    if (SOC_SUCCESS(rv)) {
        soc_etu_nl_print_register(unit, "ErrorStatus", nl_reg_err_sts_buf);
        if (!nl_reg_buf_is_zero(nl_reg_err_sts_buf)) {
            rv = SOC_E_FAIL;
        }
    }
    return rv;
}

int
nl_chk_db_err_get(int unit, unsigned dev_id, uint32 *err_addr)
{
    nl_reg_buf_t buf ;
    uint8 fifo_status;
    int rv = SOC_E_NONE;

    if (!err_addr) {
        return SOC_E_PARAM;
    }
    sal_memset(buf, 0, sizeof(buf));
    rv = soc_etu_nl_reg_read(unit, dev_id, NL_REG_ADDR_ERR_STS, buf);
    if (SOC_SUCCESS(rv)) {
        fifo_status = buf[0];
        /* Check for DB error bit */
        if (fifo_status & 0x20) {
            sal_memset(buf, 0, sizeof(buf));
            rv = soc_etu_nl_reg_read(unit, dev_id, NL_REG_ADDR_DB_SOFT_ERR_FIFO, buf);
            if (SOC_SUCCESS(rv) && (nl_reg_buf_bitfield_get(buf, 23, 1))) {
                *err_addr = nl_reg_buf_bitfield_get(buf, 0, 23);
                return SOC_E_NONE;
            } else {
                *err_addr = 0;
                return SOC_E_FAIL;
            }
        }
    }
    return rv;
}

int
soc_etu_nl_device_id_print(int unit, unsigned dev_id) 
{
    nl_reg_buf_t buf ;
    int rv = SOC_E_NONE;
    char str[64] = {0};
    int devid = 0;

    sal_memset(buf, 0, sizeof(buf));
    rv = soc_etu_nl_reg_read(unit, dev_id, NL_REG_ADDR_DEV_ID, buf);
    if (SOC_SUCCESS(rv)) {
        sal_sprintf(str, "%d Device Id", dev_id);
        soc_etu_nl_print_register(unit, str, buf);
        devid = nl_reg_buf_bitfield_get(buf, 0, 32);
	/* coverity[bad_printf_format_string : FALSE] */
        LOG_CLI((BSL_META_U(unit,
                            "Detected NL TCAM: Revision R%c-%02d "), (char)('A' + ((devid >> 3) & 0x7)), (devid & 0x7)));
        switch (devid >> 6) {
            case 2:
                LOG_CLI((BSL_META_U(unit,
                                    "512k records")));
                break;
            case 3:
                LOG_CLI((BSL_META_U(unit,
                                    "1024k records")));
                break;
            default:
                LOG_CLI((BSL_META_U(unit,
                                    "unknown size"))); 
                break;
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
    }
    return rv;
}


#endif /* BCM_CALADAN3_SUPPORT */
