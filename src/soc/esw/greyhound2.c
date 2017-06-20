/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        greyhound2.c
 * Purpose:
 * Requires:
 */


#include <sal/core/boot.h>

#include <soc/firebolt.h>
#include <soc/bradley.h>
#include <soc/greyhound.h>
#include <soc/hurricane3.h>
#include <soc/greyhound2.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/hash.h>
#include <soc/lpm.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/er_tcam.h>
#include <soc/memtune.h>
#include <soc/devids.h>
#include <soc/defs.h>
#include <soc/bondoptions.h>

#include <shared/util.h>
#include <shared/l3.h>
#include <shared/bsl.h>

#ifdef BCM_GREYHOUND2_SUPPORT

#define SOC_MAX_PHY_PORTS            90
#define SOC_MAX_MMU_PORTS            66

/* Port config related : p2l, max_speed and TDM */
/* 36P 1G + 12P 10G + 8P 10G + 8P 10G */
static const int p2l_mapping_op1_0[] = {
     0, -1,
    /* TSC4L 0~5 */
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    /* TSC4Q 0~1 */
     2,  3,  4,  5,  6,  7,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25,
    26, 27, 28, 29, 30, 31, 32, 33,
    /* TSC 0~6*/
    34, 35, 36, 37, 38, 39, 40, 41,
    42, 43, 44, 45, 46, 47, 48, 49,
    50, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61,
    /* TSCF 0*/
    62, 63, 64, 65
};

static const int port_speed_max_op1_0[] = {
       0,  -1,
     /* TSC4L 0~5 */
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     /* TSC4Q 0~1 */
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
      10,  10,  10,  10,  10,  10,  10,  10,
     /* TSC 0~6*/
      10,  10,  10,  10, 100, 100, 100, 100,
     100, 100, 100, 100, 100, 100, 100, 100,
     100, 100, 100, 100, 100, 100, 100, 100,
     100, 100, 100, 100,
     /* TSCF 0*/
     100, 100, 100, 100
};

uint32 tdm_table_op1_0[234] = {
     80,  88,  42,  63,  82,  69,  61,  78,  75,  84,
     89,  64,  67,  72, 127,  71,  76,  81,  86,  62,
     33,  83,  68,  79,  74,  85,  57,  87,  65,  66,
     73,  70,  77,  37,  80,  88,  63,  82,  43,  69,
     78,  75,  84,  89,  64,  44,  67,  72,  71,  76,
     26,  81,  86,  62,  83,  68,  38,  79,  74,  85,
     87,  65,  50,  66,  73,  70,  77,  80,  88,  51,
     63,  82,  69,  78,  75,  84,  39,  89,  64,  67,
     72,  27,  71,  76,  81,  86,  62,  83,  45,  68,
     79,  74,  85,  87,  65,  58,  66,  73,  70,  77,
     60,  80,  88,  63,  82,  28,  69,  78,  75,  84,
     89,  64,  52,  67,  72,  71,  76,  81,  86,  40,
     62,  83,  68,  46,  79,  74,  85,  87,  65,  66,
     73,   0,  70,  77,  80,  88,  63,  29,  82,  69,
     78,  75,  84,  53,  89,  64,  67,  72,  71,  76,
     41,  81,  86,  62,  83,  47,  68,  79,  74,  85,
     87,  65,  59,  66,  73,  70,  77,  30,  80,  88,
     63,  82,  69,  34,  78,  75,  84,  89,  64,  48,
     67,  72,  71,  76,  81,  86,  54,  62,  83,  68,
     79,  74,  85,  35,  87,  65,  66,  73,  31,  70,
     77,  80,  88,  63,  82,  55,  69,  78,  75,  84,
     89,  64,  49,  67,  72,  71,  76,  56,  81,  86,
     62,  83,  32,  68,  79,  74,  85,  87,  65,  36,
     66,  73,  70,  77
};

/* System Clock Freq (MHz) */
#define _GH2_SYSTEM_FREQ_583          (583) /* 583.4 MHz */
#define _GH2_SYSTEM_FREQ_388          (388) /* 388.9 MHz */
#define _GH2_SYSTEM_FREQ_125          (125) /* 125 MHz */
#define _GH2_53570_SYSTEM_FREQ        _GH2_SYSTEM_FREQ_583

#define _GH2_MAX_TSC_COUNT              (8)
#define _GH2_MAX_QTC_COUNT              (2)

#define _GH2_QTC_SERDES_OVERRDE_NONE    (0)
#define _GH2_QTC_SERDES_OVERRDE_QSGMII  (1)
#define _GH2_QTC_SERDES_OVERRDE_SGMII   (2)

#define _GH2_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* 8 TSC (TSC0~6, TSCF0) */
static const int tsc_phy_port[] = {58, 62, 66, 70, 74, 78, 82, 86};
/* 2 QTC */
static const int qtc_phy_port[] = {26, 42};



static int matched_devid_idx = -1;
static int _gh2_port_config_id = -1;

typedef struct _gh2_tsc_info_s {
    int     port_count;
    uint8   valid;
    int     phy_port_base;
} _gh2_tsc_info_t;

typedef struct _gh2_sku_info_s {
    uint16      dev_id;
    int         config_op; /* sku option */
    int         freq;
    const int   *p2l_mapping;
    const int   *speed_max;
    uint32      *tdm_table;
    int         tdm_table_size;
    uint32      disabled_qtc_bmp; /* 2 bits: [qtc1~qtc0] */
    uint32      disabled_tsc_bmp; /* 8 bits: [tscf, tsce6~tsce0] */
    int         default_port_ratio[_GH2_MAX_TSC_COUNT];
    int         num_qsgmii_supported[_GH2_MAX_QTC_COUNT];
    int         num_sgmii_supported[_GH2_MAX_QTC_COUNT];
} _gh2_sku_info_t;

static _gh2_tsc_info_t _gh2_tsc[_GH2_MAX_TSC_COUNT];
static int _gh2_qtc_serdes_override[_GH2_MAX_QTC_COUNT];


STATIC _gh2_sku_info_t _gh2_sku_port_config[] = {
/* 56170 */
{BCM56170_DEVICE_ID,  1, _GH2_53570_SYSTEM_FREQ,
    p2l_mapping_op1_0, port_speed_max_op1_0,
    tdm_table_op1_0, _GH2_ARRAY_SIZE(tdm_table_op1_0),
    0x0, 0x0,
    {SOC_GH2_PORT_RATIO_QUAD, SOC_GH2_PORT_RATIO_QUAD,
     SOC_GH2_PORT_RATIO_QUAD, SOC_GH2_PORT_RATIO_QUAD,
     SOC_GH2_PORT_RATIO_QUAD, SOC_GH2_PORT_RATIO_QUAD,
     SOC_GH2_PORT_RATIO_QUAD, SOC_GH2_PORT_RATIO_QUAD},
    {4, 4}, {4, 4}},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, {-1, -1}, {0, 0}, {0, 0}},
};

/* MMU related */
#define GH2_MMU_CBP_FULL_SIZE 0x5fff
#define GH2_MMU_CBP_HALF_SIZE 0x47ff
#define GH2_MMU_CBP_QUARTER_SIZE 0x2fff

#define CMIC_PARITY_MMU_TO_CMIC_MEMFAIL_INTR_BIT    0x0001
#define CMIC_PARITY_EP_TO_CMIC_PERR_INTR_BIT        0x0002
#define CMIC_PARITY_IP0_TO_CMIC_PERR_INTR_BIT       0x0004
#define CMIC_PARITY_IP1_TO_CMIC_PERR_INTR_BIT       0x0008
#define CMIC_PARITY_IP2_TO_CMIC_PERR_INTR_BIT       0x0010

#define _SOC_GH2_MMU_IPMC_GROUP_MAX     65
static soc_mem_t ipmc_mems[_SOC_GH2_MMU_IPMC_GROUP_MAX] = {
        INVALIDm,  INVALIDm,
        MMU_IPMC_GROUP_TBL2m,  MMU_IPMC_GROUP_TBL3m,
        MMU_IPMC_GROUP_TBL4m,  MMU_IPMC_GROUP_TBL5m,
        MMU_IPMC_GROUP_TBL6m,  MMU_IPMC_GROUP_TBL7m,
        MMU_IPMC_GROUP_TBL8m,  MMU_IPMC_GROUP_TBL9m,
        MMU_IPMC_GROUP_TBL10m,  MMU_IPMC_GROUP_TBL11m,
        MMU_IPMC_GROUP_TBL12m,  MMU_IPMC_GROUP_TBL13m,
        MMU_IPMC_GROUP_TBL14m,  MMU_IPMC_GROUP_TBL15m,
        MMU_IPMC_GROUP_TBL16m,  MMU_IPMC_GROUP_TBL17m,
        MMU_IPMC_GROUP_TBL18m,  MMU_IPMC_GROUP_TBL19m,
        MMU_IPMC_GROUP_TBL20m,  MMU_IPMC_GROUP_TBL21m,
        MMU_IPMC_GROUP_TBL22m,  MMU_IPMC_GROUP_TBL23m,
        MMU_IPMC_GROUP_TBL24m,  MMU_IPMC_GROUP_TBL25m,
        MMU_IPMC_GROUP_TBL26m,  MMU_IPMC_GROUP_TBL27m,
        MMU_IPMC_GROUP_TBL28m,  MMU_IPMC_GROUP_TBL29m,
        MMU_IPMC_GROUP_TBL30m,  MMU_IPMC_GROUP_TBL31m,
        MMU_IPMC_GROUP_TBL32m,  MMU_IPMC_GROUP_TBL33m,
        MMU_IPMC_GROUP_TBL34m,  MMU_IPMC_GROUP_TBL35m,
        MMU_IPMC_GROUP_TBL36m,  MMU_IPMC_GROUP_TBL37m,
        MMU_IPMC_GROUP_TBL38m,  MMU_IPMC_GROUP_TBL39m,
        MMU_IPMC_GROUP_TBL40m,  MMU_IPMC_GROUP_TBL41m,
        MMU_IPMC_GROUP_TBL42m,  MMU_IPMC_GROUP_TBL43m,
        MMU_IPMC_GROUP_TBL44m,  MMU_IPMC_GROUP_TBL45m,
        MMU_IPMC_GROUP_TBL46m,  MMU_IPMC_GROUP_TBL47m,
        MMU_IPMC_GROUP_TBL48m,  MMU_IPMC_GROUP_TBL49m,
        MMU_IPMC_GROUP_TBL50m,  MMU_IPMC_GROUP_TBL51m,
        MMU_IPMC_GROUP_TBL52m,  MMU_IPMC_GROUP_TBL53m,
        MMU_IPMC_GROUP_TBL54m,  MMU_IPMC_GROUP_TBL55m,
        MMU_IPMC_GROUP_TBL56m,  MMU_IPMC_GROUP_TBL57m,
        MMU_IPMC_GROUP_TBL58m,  MMU_IPMC_GROUP_TBL59m,
        MMU_IPMC_GROUP_TBL60m,  MMU_IPMC_GROUP_TBL61m,
        MMU_IPMC_GROUP_TBL62m,  MMU_IPMC_GROUP_TBL63m,
        MMU_IPMC_GROUP_TBL64m
};

static int
soc_greyhound2_pipe_mem_clear(int unit)
{
    uint32              rval;
    int                 pipe_init_usec, index;
    soc_timeout_t       to;

    /*
     * Reset the IPIPE and EPIPE block
     */
    rval = 0;
    SOC_IF_ERROR_RETURN(WRITE_ING_HW_RESET_CONTROL_1r(unit, rval));
    soc_reg_field_set(unit, ING_HW_RESET_CONTROL_2r, &rval, RESET_ALLf, 1);
    soc_reg_field_set(unit, ING_HW_RESET_CONTROL_2r, &rval, VALIDf, 1);
    /* Set count to # entries in largest IPIPE table, L2_ENTRYm */
    soc_reg_field_set(unit, ING_HW_RESET_CONTROL_2r, &rval, COUNTf, 0x8000);
    SOC_IF_ERROR_RETURN(WRITE_ING_HW_RESET_CONTROL_2r(unit, rval));

    rval = 0;
    SOC_IF_ERROR_RETURN(WRITE_EGR_HW_RESET_CONTROL_0r(unit, rval));
    soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_1r, &rval, RESET_ALLf, 1);
    soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_1r, &rval, VALIDf, 1);
    /* Set count to # entries in largest EPIPE table, EGR_VLANm */
    soc_reg_field_set(unit, EGR_HW_RESET_CONTROL_1r, &rval, COUNTf, 0x1000);
    SOC_IF_ERROR_RETURN(WRITE_EGR_HW_RESET_CONTROL_1r(unit, rval));

    /* For simulation, set timeout to 10 sec.  Otherwise, timeout = 50 ms */
    if (SAL_BOOT_SIMULATION) {
        pipe_init_usec = 10000000;
    } else {
        pipe_init_usec = 50000;
    }
    soc_timeout_init(&to, pipe_init_usec, 0);

    /* Wait for IPIPE memory initialization done. */
    do {
        SOC_IF_ERROR_RETURN(READ_ING_HW_RESET_CONTROL_2r(unit, &rval));
        if (soc_reg_field_get(unit, ING_HW_RESET_CONTROL_2r, rval, DONEf)) {
            break;
        }
        if (soc_timeout_check(&to)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "unit %d : ING_HW_RESET timeout\n"), unit));
            break;
        }
    } while (TRUE);

    /* Wait for EPIPE memory initialization done. */
    do {
        SOC_IF_ERROR_RETURN(READ_EGR_HW_RESET_CONTROL_1r(unit, &rval));
        if (soc_reg_field_get(unit, EGR_HW_RESET_CONTROL_1r, rval, DONEf)) {
            break;
        }
        if (soc_timeout_check(&to)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit, "unit %d : EGR_HW_RESET timeout\n"), unit));
            break;
        }
    } while (TRUE);

    rval = 0;
    SOC_IF_ERROR_RETURN(WRITE_ING_HW_RESET_CONTROL_2r(unit, rval));
    SOC_IF_ERROR_RETURN(WRITE_EGR_HW_RESET_CONTROL_1r(unit, rval));

    /* Processing tables that are not handled by hardware reset */
    if (!SAL_BOOT_SIMULATION || SAL_BOOT_BCMSIM) {

        /* MMU_IPMC_VLAN_TBL */
        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, MMU_IPMC_VLAN_TBLm, COPYNO_ALL, TRUE));

        /* MMU_IPMC_GROUP_TBLn : n is 2 ~ 64 in GH2 */
        for (index = 0; index < _SOC_GH2_MMU_IPMC_GROUP_MAX; index++) {
            if (ipmc_mems[index] != INVALIDm) {
                SOC_IF_ERROR_RETURN(soc_mem_clear(unit,
                                    ipmc_mems[index], COPYNO_ALL, TRUE));
            }
        }

    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_greyhound2_gmac_init()
 * Purpose:
 *      To init GMAC (should be moved to mac)
 */
STATIC int
soc_greyhound2_gmac_init(int unit)
{
    uint32 rval;
    uint32 prev_reg_addr;
    int port;

    rval = 0;
    soc_reg_field_set(unit, GPORT_CONFIGr, &rval, CLR_CNTf, 1);
    soc_reg_field_set(unit, GPORT_CONFIGr, &rval, GPORT_ENf, 1);
    prev_reg_addr = 0xffffffff;
    PBMP_E_ITER(unit, port) {
        uint32  reg_addr;
        if (IS_XL_PORT(unit, port) || IS_CL_PORT(unit, port)) {
            continue;
        }
        reg_addr = soc_reg_addr(unit, GPORT_CONFIGr, port, 0);
        if (reg_addr != prev_reg_addr) {
            SOC_IF_ERROR_RETURN(WRITE_GPORT_CONFIGr(unit, port, rval));
            prev_reg_addr = reg_addr;
        }
    }
    prev_reg_addr = 0xffffffff;
    soc_reg_field_set(unit, GPORT_CONFIGr, &rval, CLR_CNTf, 0);
    PBMP_E_ITER(unit, port) {
        uint32  reg_addr;
        if (IS_XL_PORT(unit, port) || IS_CL_PORT(unit, port)) {
            continue;
        }
        reg_addr = soc_reg_addr(unit, GPORT_CONFIGr, port, 0);
        if (reg_addr != prev_reg_addr) {
            SOC_IF_ERROR_RETURN(WRITE_GPORT_CONFIGr(unit, port, rval));
            prev_reg_addr = reg_addr;
        }
    }

    return SOC_E_NONE;
}

/*
 * Func : soc_greyhound2_pgw_encap_field_modify
 *  - To write to PGW interface on GX/XL/CL port on indicated field
 */
int
soc_greyhound2_pgw_encap_field_modify(int unit,
                                soc_port_t lport,
                                soc_field_t field,
                                uint32 val)
{
    int     pport = SOC_INFO(unit).port_l2p_mapping[lport];

    if (SOC_PORT_BLOCK_TYPE(unit, pport) == SOC_BLK_XLPORT) {
        SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit,
                    PGW_XL_CONFIGr, lport, field, val));
    } else if (SOC_PORT_BLOCK_TYPE(unit, pport) == SOC_BLK_CLPORT) {
        SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit,
                    PGW_CL_CONFIGr, lport, field, val));
    } else {
        SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit,
                    PGW_GE_CONFIGr, lport, field, val));
    }

    return SOC_E_NONE;
}
/*
 * Function:
 *      soc_greyhound2_higig_mode_init()
 * Purpose:
 *      To init the default Higig mode for HG port
 */
STATIC int
soc_greyhound2_higig_mode_init(int unit)
{
    uint32 rval;
    int port;

    PBMP_PORT_ITER(unit, port) {
        if (!IS_HG_PORT(unit, port)) {
            continue;
        }
        if (!IS_XL_PORT(unit, port) && !IS_CL_PORT(unit, port)) {
            continue;
        }

        /* Section below is used to config XLPORT and PGW related HiGig encap
         * setting for normal init process.
         */
        if (IS_XL_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(READ_XLPORT_CONFIGr(unit, port, &rval));
            soc_reg_field_set(unit, XLPORT_CONFIGr, &rval, HIGIG_MODEf, 1);
            SOC_IF_ERROR_RETURN(WRITE_XLPORT_CONFIGr(unit, port, rval));
        }
        if (IS_CL_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(READ_CLPORT_CONFIGr(unit, port, &rval));
            soc_reg_field_set(unit, CLPORT_CONFIGr, &rval, HIGIG_MODEf, 1);
            SOC_IF_ERROR_RETURN(WRITE_CLPORT_CONFIGr(unit, port, rval));
        }

        /* only HG ports allows PGW encap setting below */
        SOC_IF_ERROR_RETURN(soc_greyhound2_pgw_encap_field_modify(unit,
                port, HIGIG_MODEf, 1));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_greyhound2_mib_reset()
 * Purpose:
 *      Reset MIB counter
 */
STATIC int
soc_greyhound2_mib_reset(int unit)
{
    uint32 rval, fval;
    int blk, bindex;
    soc_info_t *si = &SOC_INFO(unit);
    int blk_port, phy_port;

    /*
     * Reset XLPORT and CLPORT MIB counter
     * (registers implemented in memory).
     * The clear function is implemented with read-modify-write,
     * parity needs to be disabled
     */
    SOC_BLOCK_ITER(unit, blk, SOC_BLK_XLPORT) {
        blk_port = SOC_BLOCK_PORT(unit, blk);
        if (blk_port < 0) {
            continue;
        }
        phy_port = si->port_l2p_mapping[blk_port];
        fval = 0;
        for (bindex = 0; bindex < 4; bindex++) {
            if (si->port_p2l_mapping[phy_port + bindex] != -1) {
                fval |= 1 << bindex;
            }
        }
        rval = 0;
        soc_reg_field_set(unit, XLPORT_MIB_RESETr, &rval, CLR_CNTf, fval);
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_MIB_RESETr(unit, blk_port, rval));
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_MIB_RESETr(unit, blk_port, 0));
    }
    SOC_BLOCK_ITER(unit, blk, SOC_BLK_CLPORT) {
        blk_port = SOC_BLOCK_PORT(unit, blk);
        if (blk_port < 0) {
            continue;
        }
        phy_port = si->port_l2p_mapping[blk_port];
        fval = 0;
        for (bindex = 0; bindex < 4; bindex++) {
            if (si->port_p2l_mapping[phy_port + bindex] != -1) {
                fval |= 1 << bindex;
            }
        }
        rval = 0;
        soc_reg_field_set(unit, CLPORT_MIB_RESETr, &rval, CLR_CNTf, fval);
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_MIB_RESETr(unit, blk_port, rval));
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_MIB_RESETr(unit, blk_port, 0));
    }

    return SOC_E_NONE;
}

soc_field_t _soc_gh2_oam_interrupt_fields[] = {
    SOME_RMEP_CCM_DEFECT_INTRf,
    SOME_RDI_DEFECT_INTRf,
    ANY_RMEP_TLV_PORT_DOWN_INTRf,
    ANY_RMEP_TLV_PORT_UP_INTRf,
    ANY_RMEP_TLV_INTERFACE_DOWN_INTRf,
    ANY_RMEP_TLV_INTERFACE_UP_INTRf,
    XCON_CCM_DEFECT_INTRf,
    ERROR_CCM_DEFECT_INTRf,
    INVALIDf
};

STATIC soc_gh2_oam_ser_handler_t
gh2_oam_ser_handler[SOC_MAX_NUM_DEVICES] = {NULL};

STATIC soc_gh2_oam_handler_t
gh2_oam_handler[SOC_MAX_NUM_DEVICES] = {NULL};

void
soc_gh2_oam_ser_handler_register(int unit, soc_gh2_oam_ser_handler_t handler)
{
    gh2_oam_ser_handler[unit] = handler;
}

int
soc_gh2_oam_ser_process(int unit, soc_mem_t mem, int index)
{
    if (gh2_oam_ser_handler[unit]) {
        return gh2_oam_ser_handler[unit](unit, mem, index);
    } else {
        return SOC_E_UNAVAIL;
    }
}

void
soc_gh2_oam_handler_register(int unit, soc_gh2_oam_handler_t handler)
{
    uint64 rval;
    int rv, id = 0;
    gh2_oam_handler[unit] = handler;

    rv = READ_IP2_INTR_ENABLEr(unit, &rval);
    if (rv) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "unit %d: Error reading %s reg !!\n"),
                   unit, SOC_REG_NAME(unit, IP2_INTR_ENABLEr)));
    }
    while (_soc_gh2_oam_interrupt_fields[id] != INVALIDf) {
        soc_reg64_field32_set(unit, IP2_INTR_ENABLEr, &rval,
                              _soc_gh2_oam_interrupt_fields[id], 1);
        id++;
    }
    rv = WRITE_IP2_INTR_ENABLEr(unit, rval);
    if (rv) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "unit %d: Error writing %s reg !!\n"),
                   unit, SOC_REG_NAME(unit, IP1_INTR_ENABLEr)));
    }
}

int
_soc_gh2_process_oam_interrupt(int unit)
{
    uint64 rval;
    int found = 0, id = 0;
    soc_gh2_oam_handler_t oam_handler_snapshot = gh2_oam_handler[unit];


    SOC_IF_ERROR_RETURN(READ_IP2_INTR_STATUSr(unit, &rval));

    while (_soc_gh2_oam_interrupt_fields[id] != INVALIDf) {
        if (soc_reg64_field32_get(unit, IP2_INTR_STATUSr, rval,
                                  _soc_gh2_oam_interrupt_fields[id])) {
            if (oam_handler_snapshot != NULL) {
                (void)(oam_handler_snapshot(unit,
                                            _soc_gh2_oam_interrupt_fields[id]));
            }
            found++;
        }
        id++;
    }
    if (!found) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unexpected interrupt received for OAM !!\n")));
    }
    return SOC_E_NONE;
}

int
soc_greyhound2_init_port_mapping(int unit)
{
    soc_info_t *si;
    soc_mem_t mem;
    uint32 rval;
    ing_physical_to_logical_port_number_mapping_table_entry_t entry;
    int logical_port, phy_port, mmu_port;
    int num_port, num_phy_port;
    uint32 fval[3];
    egr_tdm_port_map_entry_t egr_tdm_port_map_entry;
    int idx;
    int shift;

    si = &SOC_INFO(unit);

    /* Ingress physical to logical port mapping */
    mem = ING_PHYSICAL_TO_LOGICAL_PORT_NUMBER_MAPPING_TABLEm;
    num_phy_port = soc_mem_index_count(unit, mem);
    sal_memset(&entry, 0, sizeof(entry));
    for (phy_port = 0; phy_port < num_phy_port; phy_port++) {
        logical_port = si->port_p2l_mapping[phy_port];
        soc_mem_field32_set(unit, mem, &entry, LOGICAL_PORT_NUMBERf,
                            logical_port == -1 ? 0x30 : logical_port);
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, mem, MEM_BLOCK_ALL, phy_port, &entry));
    }
    num_port = soc_mem_index_count(unit, PORT_TABm);

    /* Egress logical to physical port mapping */
    for (logical_port = 0; logical_port < num_port; logical_port++) {
        phy_port = si->port_l2p_mapping[logical_port];
        rval = 0;
        soc_reg_field_set(unit, EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPINGr,
                          &rval, PHYSICAL_PORT_NUMBERf,
                          phy_port == -1 ? 0x3f : phy_port);
        SOC_IF_ERROR_RETURN
            (WRITE_EGR_LOGICAL_TO_PHYSICAL_PORT_NUMBER_MAPPINGr(unit,
                                                                logical_port,
                                                                rval));
    }

    /* EGR_TDM_PORT_MAPm */
    sal_memset(&egr_tdm_port_map_entry, 0, sizeof(egr_tdm_port_map_entry));
    sal_memset(fval, 0, sizeof(fval));
    mem = EGR_TDM_PORT_MAPm;
    for (logical_port = 0; logical_port < num_port; logical_port++) {
        phy_port = si->port_l2p_mapping[logical_port];
        /*
            * Physical port of loopback port(1) is assigned -1.
            * The port should be programmed on physical port 1.
            */
        if (logical_port == 1) {
            phy_port = 1;
        }
        idx = phy_port / 32;
        shift = phy_port - idx * 32;
        fval[idx] |= (1 << shift);
    }
    soc_mem_field_set(unit, EGR_TDM_PORT_MAPm,
        (void *)&egr_tdm_port_map_entry, BITMAPf, fval);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, mem, MEM_BLOCK_ALL, 0, &egr_tdm_port_map_entry));

    /* MMU to physical port mapping and MMU to logical port mapping */
    for (mmu_port = 0; mmu_port < SOC_MAX_MMU_PORTS; mmu_port++) {
        /* MMU to Physical  port */
        phy_port = si->port_m2p_mapping[mmu_port];
        /* Physical to Logical port */
        logical_port = si->port_p2l_mapping[phy_port];

        if (phy_port == 1) {
            /* skip loopback port */
            continue;
        }
        if (phy_port == -1) {
            /* skip not mapped mmu port */
            continue;
        }
        rval = 0;
        soc_reg_field_set(unit, MMU_PORT_TO_PHY_PORT_MAPPINGr, &rval,
                          PHY_PORTf, phy_port);
        SOC_IF_ERROR_RETURN
            (WRITE_MMU_PORT_TO_PHY_PORT_MAPPINGr(unit, logical_port, rval));

        if (logical_port == -1) {
            logical_port = 1;
        }
        rval = 0;
        soc_reg_field_set(unit, MMU_PORT_TO_LOGIC_PORT_MAPPINGr, &rval,
                          LOGIC_PORTf, logical_port);
        SOC_IF_ERROR_RETURN
            (WRITE_MMU_PORT_TO_LOGIC_PORT_MAPPINGr(unit, logical_port, rval));
    }
    return SOC_E_NONE;
}

/*
 *  GH2 port mapping
 * cpu port number is fixed: physical 0, logical 0, mmu 0
 */
int
soc_greyhound2_port_config_init(int unit, uint16 dev_id)
{

    soc_info_t *si;
    int rv = SOC_E_NONE;
    int phy_port;
    const int *p2l_mapping = 0, *speed_max = 0;
    char *propval, *option;
    int match = -1, i, j, match_option;
    int p2l_mapping_override[SOC_MAX_PHY_PORTS];
    int port_speed_max_override[SOC_MAX_PHY_PORTS];
    _gh2_sku_info_t *matched_port_config;

    option = soc_property_get_str(unit, "init_port_config_option");
    if (option == NULL) {
        /* Backward compatible */
        option = soc_property_get_str(unit, "bcm53570_init_port_config");
    }


    if (option == NULL) {
        _gh2_port_config_id = 1;
    } else {
        _gh2_port_config_id = sal_ctoi(option, NULL);
    }
    match_option = -1;
    for (i = 0; _gh2_sku_port_config[i].dev_id; i++) {
        if (dev_id == _gh2_sku_port_config[i].dev_id) {
            match = i;
            if (_gh2_sku_port_config[i].config_op != -1 ) {
                match_option = 0;
                if (_gh2_sku_port_config[i].config_op == _gh2_port_config_id ) {
                    match = i;
                    match_option = _gh2_port_config_id;
                    break;
                }
            }
        }
    }
    for (i = 0; i < _GH2_MAX_TSC_COUNT; i++) {
        _gh2_tsc[i].port_count = 0;
        _gh2_tsc[i].valid = 0;
        _gh2_tsc[i].phy_port_base = tsc_phy_port[i];
        if (match == -1) {
            _gh2_tsc[i].port_count =
                _gh2_sku_port_config[0].default_port_ratio[i];
        } else {
            _gh2_tsc[i].port_count =
                _gh2_sku_port_config[match].default_port_ratio[i];
        }
    }

    /* Check QTC SGMII/QSGMII selection */
    for (i = 0; i < _GH2_MAX_QTC_COUNT; i++) {
        _gh2_qtc_serdes_override[i] = _GH2_QTC_SERDES_OVERRDE_NONE;
        propval = soc_property_suffix_num_str_get(
                      unit, i, "bcm53570_init_port_config", "qtc");
        if (propval) {
            if (sal_strcmp(propval, "SGMII") == 0) {
                _gh2_qtc_serdes_override[i] = _GH2_QTC_SERDES_OVERRDE_SGMII;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,"%s: QTC%1d is SGMII\n"),
                    FUNCTION_NAME(), i));
            } else if (sal_strcmp(propval, "QSGMII") == 0) {
                _gh2_qtc_serdes_override[i] = _GH2_QTC_SERDES_OVERRDE_QSGMII;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,"%s: QTC%1d is QSGMII\n"),
                    FUNCTION_NAME(), i));
            } else {
                _gh2_qtc_serdes_override[i] = _GH2_QTC_SERDES_OVERRDE_NONE;
            }
        }
    }

    for (phy_port = 0; phy_port < SOC_MAX_PHY_PORTS; phy_port++) {
        p2l_mapping_override[phy_port] = 0;
        port_speed_max_override[phy_port] = 0;
    }

    if (match == -1) {
        /* take 56170 option 1 as default config */
        p2l_mapping = p2l_mapping_op1_0;
        speed_max = port_speed_max_op1_0;
        LOG_WARN(BSL_LS_SOC_COMMON,
            (BSL_META_U(unit,"%s: no device_id matched\n"), FUNCTION_NAME()));
        LOG_WARN(BSL_LS_SOC_COMMON,
            (BSL_META_U(unit,"    take option 1 as default\n")));
    } else {
        matched_devid_idx = match;
        matched_port_config = &_gh2_sku_port_config[match];
        if (((option != NULL) && (matched_port_config->config_op == -1)) ||
            (match_option == 0)) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Warning: bcm53570_init_port_config=%d config "
                            "is not supported in %s ! \n"), _gh2_port_config_id,
                            soc_dev_name(unit)));
        }

        p2l_mapping = matched_port_config->p2l_mapping;
        speed_max = matched_port_config->speed_max;

        /* QSGMII or SGMII selection */
        for (i = 0; i < _GH2_MAX_QTC_COUNT; i++) {
            if (!((1 << i) & matched_port_config->disabled_qtc_bmp)) {
                /* Override to the SGMII port mapping and max speed */
                if (_gh2_qtc_serdes_override[i] ==
                    _GH2_QTC_SERDES_OVERRDE_SGMII) {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,"%s: QTC%1d SGMII overrided.\n"),
                        FUNCTION_NAME(), i));
                    if (matched_port_config->num_sgmii_supported[i] == 0) {
                        LOG_WARN(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                            "Warning: bcm53570_init_port_config_qtc%d config "
                            "is not supported in %s ! \n"), i,
                            soc_dev_name(unit)));
                    }
                    /* Disable original qsgmii ports */
                    for (j = 0;
                         j < matched_port_config->num_qsgmii_supported[i];
                         j++) {
                        p2l_mapping_override[qtc_phy_port[i] + j * 4] = -1;
                        p2l_mapping_override[qtc_phy_port[i] + j * 4 + 1] = -1;
                        p2l_mapping_override[qtc_phy_port[i] + j * 4 + 2] = -1;
                        p2l_mapping_override[qtc_phy_port[i] + j * 4 + 3] = -1;
                    }
                    for (j = 0;
                         j < matched_port_config->num_sgmii_supported[i];
                         j++) {
                        /* Override sgmii port to max speed  = 2.5 Gb */
                        p2l_mapping_override[qtc_phy_port[i] + j] = 1;
                        port_speed_max_override[qtc_phy_port[i] + j] = 25;
                    }
                } else if (_gh2_qtc_serdes_override[i] ==
                        _GH2_QTC_SERDES_OVERRDE_SGMII) {
                    if (matched_port_config->num_qsgmii_supported[i] == 0) {
                        LOG_WARN(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                            "Warning: bcm53570_init_port_config_qtc%d config "
                            "is not supported in %s ! \n"), i,
                            soc_dev_name(unit)));
                        /*
                         * Defaul configuration is QSGMII except QSGMII
                         * is not supported
                         */
                        /* No need to override the mapping and max port speed */
                    }
                }
            }
        }

    }

    si = &SOC_INFO(unit);

    for (phy_port = 0; phy_port < SOC_MAX_PHY_PORTS; phy_port++) {
        if (p2l_mapping_override[phy_port] == -1) {
            si->port_p2l_mapping[phy_port] = p2l_mapping_override[phy_port];
            /* Same as Egr P2L */
            si->port_p2m_mapping[phy_port] = p2l_mapping_override[phy_port];
            /* For COSQ programming */
            si->max_port_p2m_mapping[phy_port] = p2l_mapping_override[phy_port];
        } else {
            /* speed_max reflects the valid port */
            /*
             * For the same TDM, the phy_port should be existed in
             * p2l_mapping table
             */
            if ((speed_max[phy_port] == -1) && (phy_port != 0)) {
                si->port_p2l_mapping[phy_port] = -1;
                si->port_p2m_mapping[phy_port] = -1;
                si->max_port_p2m_mapping[phy_port] = -1;
                si->port_num_lanes[phy_port] = -1;
            } else {
                si->port_p2l_mapping[phy_port] = p2l_mapping[phy_port];
                si->port_p2m_mapping[phy_port] = p2l_mapping[phy_port];
                si->max_port_p2m_mapping[phy_port] = p2l_mapping[phy_port];
                si->port_num_lanes[phy_port] = 1;
            }
        }
        if (port_speed_max_override[phy_port] != 0) {
            if (port_speed_max_override[phy_port] != -1) {
                si->port_speed_max[si->port_p2l_mapping[phy_port]] =
                    port_speed_max_override[phy_port] * 100;
                if (port_speed_max_override[phy_port] == 25) { /* 2.5G */
                    si->port_num_lanes[phy_port] = 4;
                }
            }
        } else {
            if (speed_max[phy_port] != -1) {
                si->port_speed_max[si->port_p2l_mapping[phy_port]] =
                    speed_max[phy_port] * 100;
            }
        }
    }

    /* physical to mmu port mapping */
    {
        int phy_port_cpu;
        int phy_port_loopback;
        int mmu_port_cpu;
        int mmu_port_loopback;
        int num_mmu_port = SOC_MAX_MMU_PORTS;
        int mmu_port_max;
        int mmu_port;

        phy_port_cpu = 0;
        mmu_port_cpu = 0;
        phy_port_loopback = 1;
        mmu_port_loopback = 1;
        si->port_p2m_mapping[phy_port_cpu] = mmu_port_cpu;
        si->port_p2m_mapping[phy_port_loopback] = mmu_port_loopback;

        mmu_port_max = num_mmu_port - 1;
        for (phy_port = (SOC_MAX_PHY_PORTS - 1);
             phy_port > phy_port_loopback; phy_port--) {
            if (si->port_p2l_mapping[phy_port] != -1) {
                si->port_p2m_mapping[phy_port] = mmu_port_max;
                mmu_port_max--;
            }
        }

        /*
         * reset port_m2p_mapping, actual value is setup in
         * soc_info_config() later
         */
        for (mmu_port = 0; mmu_port < num_mmu_port; mmu_port++) {
            si->port_m2p_mapping[mmu_port] = -1;
        }
    }

    return rv;
}


int
_soc_greyhound2_tdm_init(int unit, uint16 dev_id)
{
    uint32              *arr = NULL;
    int                 tdm_size;
    uint32              rval;
    iarb_tdm_table_entry_t iarb_tdm;
    mmu_arb_tdm_table_entry_t mmu_arb_tdm;
    int i, port, phy_port;
    _gh2_sku_info_t *matched_sku_info;
    soc_info_t *si;

    si = &SOC_INFO(unit);



    SOC_IF_ERROR_RETURN(READ_IARB_TDM_CONTROLr(unit, &rval));
    soc_reg_field_set(unit, IARB_TDM_CONTROLr, &rval, DISABLEf, 1);
    soc_reg_field_set(unit, IARB_TDM_CONTROLr, &rval, TDM_WRAP_PTRf, 83);
    SOC_IF_ERROR_RETURN(WRITE_IARB_TDM_CONTROLr(unit, rval));

    if (matched_devid_idx == -1) {
        LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META_U(unit, "Warning: soc_greyhound2_port_config_init should "
                        "be invoked first! Choose bcm56170 port config.\n")));
        matched_devid_idx = 0;
    }
    matched_sku_info = &_gh2_sku_port_config[matched_devid_idx];

    arr = matched_sku_info->tdm_table;
    tdm_size = matched_sku_info->tdm_table_size;

    if (arr == NULL) {
        return SOC_E_CONFIG;
    }

    for (i = 0; i < tdm_size; i++) {
        phy_port = arr[i];
        port = (phy_port != 127) ? si->port_p2m_mapping[phy_port] : 127;
        sal_memset(&iarb_tdm, 0, sizeof(iarb_tdm_table_entry_t));
        sal_memset(&mmu_arb_tdm, 0, sizeof(mmu_arb_tdm_table_entry_t));

        soc_IARB_TDM_TABLEm_field32_set(unit, &iarb_tdm, PORT_NUMf,
                                        phy_port);
        soc_MMU_ARB_TDM_TABLEm_field32_set(unit, &mmu_arb_tdm, PORT_NUMf,
                                           port);

        if (i == tdm_size - 1) {
            soc_MMU_ARB_TDM_TABLEm_field32_set(unit, &mmu_arb_tdm, WRAP_ENf, 1);
        }
        SOC_IF_ERROR_RETURN(WRITE_IARB_TDM_TABLEm(unit, SOC_BLOCK_ALL, i,
                                                  &iarb_tdm));
        SOC_IF_ERROR_RETURN(WRITE_MMU_ARB_TDM_TABLEm(unit, SOC_BLOCK_ALL, i,
                                                     &mmu_arb_tdm));
    }
    rval = 0;
    soc_reg_field_set(unit, IARB_TDM_CONTROLr, &rval, DISABLEf, 0);
    soc_reg_field_set(unit, IARB_TDM_CONTROLr, &rval, TDM_WRAP_PTRf,
                      tdm_size -1);
    SOC_IF_ERROR_RETURN(WRITE_IARB_TDM_CONTROLr(unit, rval));

    /* system core clock */
    si->frequency = matched_sku_info->freq;
    return SOC_E_NONE;
}

STATIC int ceiling_func(uint32 numerators, uint32 denominator)
{
    uint32  result;
    if (denominator == 0) {
        return 0xFFFFFFFF;
    }
    result = numerators / denominator;
    if (numerators % denominator != 0) {
        result++;
    }
    return result;
}


STATIC int _soc_greyhound2_mmu_init_helper_lossy(int unit)
{
    uint32 rval;
    int port, phy_port;
    int index;
    soc_info_t *si = &SOC_INFO(unit);
    soc_pbmp_t pbmp_uplink;
    soc_pbmp_t pbmp_downlink_1g;
    soc_pbmp_t pbmp_downlink_2dot5g;

    int standard_jumbo_frame;
    int cell_size;
    int ethernet_mtu_cell;
    int standard_jumbo_frame_cell;
    int total_physical_memory;
    int total_cell_memory_for_admission;
    int number_of_used_memory_banks;
    int reserved_for_cfap;
    int skidmarker;
    int prefetch;
    int total_cell_memory;
    int cfapfullsetpoint;
    int total_advertised_cell_memory;
    int number_of_uplink_ports;
    int number_of_downlink_ports;
    int queue_port_limit_ratio;
    int egress_queue_min_reserve_uplink_ports_lossy;
    int egress_queue_min_reserve_downlink_ports_lossy;
    int egress_queue_min_reserve_uplink_ports_lossless;
    int egress_queue_min_reserve_downlink_ports_lossless;
    int egress_queue_min_reserve_cpu_ports;
    int egress_xq_min_reserve_lossy_ports;
    int egress_xq_min_reserve_lossless_uplink_ports;
    int egress_xq_min_reserve_lossless_downlink_ports;
    int num_active_pri_group_lossless;
    int num_lossy_queues;
    int mmu_xoff_pkt_threshold_uplink_ports;
    int mmu_xoff_pkt_threshold_downlink_ports;
    int mmu_xoff_cell_threshold_1g_port_downlink_ports;
    int mmu_xoff_cell_threshold_2dot5g_port_downlink_ports;
    int mmu_xoff_cell_threshold_all_uplink_ports;
    int num_cpu_queues;
    int num_cpu_ports;
    int numxqs_per_uplink_ports;
    int numxqs_per_downlink_ports_and_cpu_port;
    int headroom_for_1g_port;
    int headroom_for_2dot5g_port;
    int xoff_cell_thresholds_per_port_1g_port_downlink_ports;
    int xoff_cell_thresholds_per_port_2dot5g_downlink_ports;
    int xoff_cell_threshold_all_uplink_ports;
    int xoff_packet_thresholds_per_port_uplink_port;
    int xoff_packet_thresholds_per_port_downlink_port;
    int discard_limit_per_port_pg_uplink_1g_port;
    int discard_limit_per_port_pg_uplink_2dot5g_port;
    int discard_limit_per_port_pg_downlink_port;
    int total_reserved_cells_for_uplink_ports;
    int total_reserved_cells_for_downlink_ports;
    int total_reserved_cells_for_cpu_port;
    int total_reserved;
    int shared_space_cells;
    int reserved_xqs_per_uplink_port;
    int shared_xqs_per_uplink_port;
    int reserved_xqs_per_downlink_port;
    int shared_xqs_per_downlink_port;
    int cfapfullthreshold_cfapfullsetpoint_up;
    int gbllimitsetlimit_gblcellsetlimit_up;
    int totaldyncellsetlimit_totaldyncellsetlimit_up;
    int holcosminxqcnt_qlayer8_holcosminxqcnt_up;
    int holcospktsetlimit0_pktsetlimit_up;
    int holcospktsetlimit7_pktsetlimit_up;
    int holcospktsetlimit_qlayer0_pktsetlimit_up;
    int holcospktsetlimit_qlayer7_pktsetlimit_up;
    int holcospktsetlimit_qlayer8_pktsetlimit_up;
    int dynxqcntport_dynxqcntport_up;
    int lwmcoscellsetlimit0_cellsetlimit_up;
    int lwmcoscellsetlimit7_cellsetlimit_up;
    int lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    int lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    int lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    int holcoscellmaxlimit0_cellmaxlimit_up;
    int holcoscellmaxlimit7_cellmaxlimit_up;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_up;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_up;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_up;
    int dyncelllimit_dyncellsetlimit_up;
    int holcospktsetlimit_qgroup0_pktsetlimit_up;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_up;
    int holcosminxqcnt_qlayer8_holcosminxqcnt_down_1;
    int holcospktsetlimit0_pktsetlimit_down_1;
    int holcospktsetlimit7_pktsetlimit_down_1;
    int holcospktsetlimit_qlayer0_pktsetlimit_down_1;
    int holcospktsetlimit_qlayer7_pktsetlimit_down_1;
    int holcospktsetlimit_qlayer8_pktsetlimit_down_1;
    int dynxqcntport_dynxqcntport_down_1;
    int holcoscellmaxlimit0_cellmaxlimit_down_1;
    int holcoscellmaxlimit7_cellmaxlimit_down_1;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_down_1;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_down_1;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_down_1;
    int dyncelllimit_dyncellsetlimit_down_1;
    int holcospktsetlimit_qgroup0_pktsetlimit_down_1;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_down_1;
    int holcospktsetlimit0_pktsetlimit_down_2dot5;
    int holcospktsetlimit7_pktsetlimit_down_2dot5;
    int holcospktsetlimit_qlayer0_pktsetlimit_down_2dot5;
    int holcospktsetlimit_qlayer7_pktsetlimit_down_2dot5;
    int holcospktsetlimit_qlayer8_pktsetlimit_down_2dot5;
    int dynxqcntport_dynxqcntport_down_2dot5;
    int holcoscellmaxlimit0_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit7_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_down_2dot5;
    int dyncelllimit_dyncellsetlimit_down_2dot5;
    int holcospktsetlimit_qgroup0_pktsetlimit_down_2dot5;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_down_2dot5;

    /* setup port bitmap according the port max speed for lossy
     *   TSC/TSCF    : uplink port
     *   QGMII/SGMII : donnlink port
     */
    number_of_uplink_ports = 0;
    number_of_downlink_ports = 0;
    SOC_PBMP_CLEAR (pbmp_uplink);
    SOC_PBMP_CLEAR (pbmp_downlink_1g);
    SOC_PBMP_CLEAR (pbmp_downlink_2dot5g);
    for (phy_port = 0; phy_port < SOC_MAX_PHY_PORTS; phy_port++) {
        port = si->port_p2l_mapping[phy_port];
        if (IS_XL_PORT(unit, port) || IS_CL_PORT(unit, port)) {
            number_of_uplink_ports++;
            SOC_PBMP_PORT_ADD(pbmp_uplink, port);
        } else if (IS_GE_PORT(unit, port)) {
            number_of_downlink_ports++;
            if (SOC_INFO(unit).port_speed_max[port] > 1000) {
                SOC_PBMP_PORT_ADD(pbmp_downlink_2dot5g, port);
            } else {
                SOC_PBMP_PORT_ADD(pbmp_downlink_1g, port);
            }
        }
    }

    standard_jumbo_frame = 9216;
    cell_size = 144;
    ethernet_mtu_cell = ceiling_func(15 * 1024 / 10, cell_size);
    standard_jumbo_frame_cell = ceiling_func(standard_jumbo_frame, cell_size);
    total_physical_memory = 24 * 1024;
    total_cell_memory_for_admission = 225 * 1024 / 10;
    number_of_used_memory_banks = 8;
    reserved_for_cfap = (65) * 2 + number_of_used_memory_banks * 4;
    skidmarker = 3;
    prefetch = 64 + 4;
    total_cell_memory = total_cell_memory_for_admission;
    cfapfullsetpoint = total_physical_memory - reserved_for_cfap;
    total_advertised_cell_memory = total_cell_memory;
    queue_port_limit_ratio = 8;
    egress_queue_min_reserve_uplink_ports_lossy = ethernet_mtu_cell;
    egress_queue_min_reserve_downlink_ports_lossy = ethernet_mtu_cell;
    egress_queue_min_reserve_uplink_ports_lossless = 0;
    egress_queue_min_reserve_downlink_ports_lossless = 0;
    egress_queue_min_reserve_cpu_ports = ethernet_mtu_cell;
    egress_xq_min_reserve_lossy_ports
          = ethernet_mtu_cell;
    egress_xq_min_reserve_lossless_uplink_ports = 0;
    egress_xq_min_reserve_lossless_downlink_ports = 0;
    num_active_pri_group_lossless = 0;
    num_lossy_queues = 8;
    mmu_xoff_pkt_threshold_uplink_ports = total_advertised_cell_memory;
    mmu_xoff_pkt_threshold_downlink_ports = total_advertised_cell_memory;
    mmu_xoff_cell_threshold_1g_port_downlink_ports
          = total_advertised_cell_memory;
    mmu_xoff_cell_threshold_2dot5g_port_downlink_ports
          = total_advertised_cell_memory;
    mmu_xoff_cell_threshold_all_uplink_ports = total_advertised_cell_memory;
    num_cpu_queues = 8;
    num_cpu_ports = 1;
    numxqs_per_uplink_ports = 6 * 1024;
    numxqs_per_downlink_ports_and_cpu_port = 2 * 1024;
    headroom_for_1g_port = 0;
    headroom_for_2dot5g_port = 0;
    xoff_cell_thresholds_per_port_1g_port_downlink_ports
          = mmu_xoff_cell_threshold_1g_port_downlink_ports;
    xoff_cell_thresholds_per_port_2dot5g_downlink_ports
          = mmu_xoff_cell_threshold_2dot5g_port_downlink_ports;
    xoff_cell_threshold_all_uplink_ports
          = mmu_xoff_cell_threshold_all_uplink_ports;
    xoff_packet_thresholds_per_port_uplink_port
          = mmu_xoff_pkt_threshold_uplink_ports;
    xoff_packet_thresholds_per_port_downlink_port
          = mmu_xoff_pkt_threshold_downlink_ports;
    discard_limit_per_port_pg_uplink_1g_port
        = xoff_cell_thresholds_per_port_1g_port_downlink_ports
          + headroom_for_1g_port;
    discard_limit_per_port_pg_uplink_2dot5g_port
        = xoff_cell_thresholds_per_port_2dot5g_downlink_ports
          + headroom_for_2dot5g_port;
    discard_limit_per_port_pg_downlink_port = total_advertised_cell_memory;
    total_reserved_cells_for_uplink_ports
        = egress_queue_min_reserve_uplink_ports_lossy
          * number_of_uplink_ports * num_lossy_queues
          + number_of_uplink_ports
          * egress_queue_min_reserve_uplink_ports_lossless
          * num_active_pri_group_lossless;
    total_reserved_cells_for_downlink_ports
        = number_of_downlink_ports
          * egress_queue_min_reserve_downlink_ports_lossy
          * (num_lossy_queues) + number_of_downlink_ports
          * egress_queue_min_reserve_downlink_ports_lossless
          * num_active_pri_group_lossless;
    total_reserved_cells_for_cpu_port
        = num_cpu_ports * egress_queue_min_reserve_cpu_ports
          * num_cpu_queues;
    total_reserved
        = total_reserved_cells_for_uplink_ports
          + total_reserved_cells_for_downlink_ports
          + total_reserved_cells_for_cpu_port;
    shared_space_cells = total_advertised_cell_memory - total_reserved;
    reserved_xqs_per_uplink_port
        = egress_xq_min_reserve_lossy_ports
          * num_lossy_queues + egress_xq_min_reserve_lossless_uplink_ports
          * num_active_pri_group_lossless;
    shared_xqs_per_uplink_port
          = numxqs_per_uplink_ports - reserved_xqs_per_uplink_port;
    reserved_xqs_per_downlink_port
        = egress_xq_min_reserve_lossy_ports
          * num_lossy_queues + egress_xq_min_reserve_lossless_downlink_ports
          * num_active_pri_group_lossless;
    shared_xqs_per_downlink_port
        = numxqs_per_downlink_ports_and_cpu_port
          - reserved_xqs_per_downlink_port;
    cfapfullthreshold_cfapfullsetpoint_up = cfapfullsetpoint;
    gbllimitsetlimit_gblcellsetlimit_up = total_cell_memory_for_admission;
    totaldyncellsetlimit_totaldyncellsetlimit_up = shared_space_cells;
    holcosminxqcnt_qlayer8_holcosminxqcnt_up = 0;
    holcospktsetlimit0_pktsetlimit_up
        = shared_xqs_per_uplink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_up
        = shared_xqs_per_uplink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_up
        = shared_xqs_per_uplink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_up
        = shared_xqs_per_uplink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_up
        = shared_xqs_per_uplink_port
          + holcosminxqcnt_qlayer8_holcosminxqcnt_up;
    dynxqcntport_dynxqcntport_up
          = shared_xqs_per_uplink_port - skidmarker - prefetch;
    lwmcoscellsetlimit0_cellsetlimit_up
          = egress_queue_min_reserve_uplink_ports_lossy;
    lwmcoscellsetlimit7_cellsetlimit_up
          = egress_queue_min_reserve_uplink_ports_lossy;
    lwmcoscellsetlimit_qlayer0_cellsetlimit_up
          = egress_queue_min_reserve_uplink_ports_lossy;
    lwmcoscellsetlimit_qlayer7_cellsetlimit_up
          = egress_queue_min_reserve_uplink_ports_lossy;
    lwmcoscellsetlimit_qlayer8_cellsetlimit_up = 0;
    holcoscellmaxlimit0_cellmaxlimit_up
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_up
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_up
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_up
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_up
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_up = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_up = numxqs_per_uplink_ports - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_up
        = ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);
    holcosminxqcnt_qlayer8_holcosminxqcnt_down_1 = 0;
    holcospktsetlimit0_pktsetlimit_down_1
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_down_1
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_down_1
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_down_1
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_down_1
        = shared_xqs_per_downlink_port
          + holcosminxqcnt_qlayer8_holcosminxqcnt_down_1;
    dynxqcntport_dynxqcntport_down_1
          = shared_xqs_per_downlink_port - skidmarker - prefetch;
    holcoscellmaxlimit0_cellmaxlimit_down_1
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_down_1
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_down_1
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_down_1
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_down_1
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_down_1 = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_down_1
          = numxqs_per_downlink_ports_and_cpu_port - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_down_1
        = ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);
    holcospktsetlimit0_pktsetlimit_down_2dot5
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_down_2dot5
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_down_2dot5
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_down_2dot5
        = shared_xqs_per_downlink_port
          + egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_down_2dot5
        = shared_xqs_per_downlink_port
          + holcosminxqcnt_qlayer8_holcosminxqcnt_down_1;
    dynxqcntport_dynxqcntport_down_2dot5
          = shared_xqs_per_downlink_port - skidmarker - prefetch;
    holcoscellmaxlimit0_cellmaxlimit_down_2dot5
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_down_2dot5
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_down_2dot5
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_down_2dot5
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_down_2dot5
        = ceiling_func(shared_space_cells, queue_port_limit_ratio)
          + lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_down_2dot5 = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_down_2dot5
          = numxqs_per_downlink_ports_and_cpu_port - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_down_2dot5
        = ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);

    if ((shared_space_cells * cell_size)/1024 <= 800) {
        LOG_CLI((BSL_META_U(unit,
                 "Shared Pool Is Small,\
                 should be larger than 800 (value=%d)\n"),
                 (shared_space_cells * cell_size)/1024));
        return SOC_E_PARAM;
    }

    /* system-based */
    soc_reg_field32_modify(unit, CFAPFULLTHRESHOLDr,
                           REG_PORT_ANY,
                           CFAPFULLSETPOINTf,
                           cfapfullsetpoint);
    soc_reg_field32_modify(unit, CFAPFULLTHRESHOLDr,
                           REG_PORT_ANY,
                           CFAPFULLRESETPOINTf,
                           cfapfullthreshold_cfapfullsetpoint_up -
                           (standard_jumbo_frame_cell * 2));
    soc_reg_field32_modify(unit, GBLLIMITSETLIMITr,
                           REG_PORT_ANY,
                           GBLCELLSETLIMITf,
                           total_cell_memory_for_admission);
    soc_reg_field32_modify(unit, GBLLIMITRESETLIMITr,
                           REG_PORT_ANY,
                           GBLCELLRESETLIMITf,
                           gbllimitsetlimit_gblcellsetlimit_up);
    soc_reg_field32_modify(unit, TOTALDYNCELLSETLIMITr,
                           REG_PORT_ANY,
                           TOTALDYNCELLSETLIMITf,
                           shared_space_cells);
    soc_reg_field32_modify(unit, TOTALDYNCELLRESETLIMITr,
                           REG_PORT_ANY,
                           TOTALDYNCELLRESETLIMITf,
                           totaldyncellsetlimit_totaldyncellsetlimit_up -
                           (standard_jumbo_frame_cell * 2));
    soc_reg_field32_modify(unit, TWO_LAYER_SCH_MODEr,
                           REG_PORT_ANY,
                           SCH_MODEf,
                           0);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           MULTIPLE_ACCOUNTING_FIX_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           CNG_DROP_ENf,
                           0);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           DYN_XQ_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           HOL_CELL_SOP_DROP_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           DYNAMIC_MEMORY_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           SKIDMARKERf,
                           skidmarker);
    soc_reg_field32_modify(unit, MMUPORTTXENABLE_0r,
                           REG_PORT_ANY,
                           MMUPORTTXENABLEf,
                           0xFFFFFFFF);
    soc_reg_field32_modify(unit, MMUPORTTXENABLE_1r,
                           REG_PORT_ANY,
                           MMUPORTTXENABLEf,
                           0xFFFFFFFF);
    soc_reg_field32_modify(unit, MMUPORTTXENABLE_2r,
                           REG_PORT_ANY,
                           MMUPORTTXENABLEf,
                           3);

    /* port-based : uplink */
    SOC_PBMP_ITER(pbmp_uplink, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          1);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          2);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          3);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          4);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          5);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          6);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_uplink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNTr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_uplink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_uplink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_uplink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_up);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_up - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit_qlayer7_pktsetlimit_up - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_uplink_ports - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_uplink_ports - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_uplink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_up - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_up -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_up -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_up -
                          (2 * ethernet_mtu_cell));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);
    }

    /* port-based : downlink 1G */
    SOC_PBMP_ITER(pbmp_downlink_1g, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          1);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          2);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          3);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          4);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          5);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          6);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_downlink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLSETLIMITf,
                          xoff_cell_thresholds_per_port_1g_port_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLRESETLIMITf,
                          xoff_cell_thresholds_per_port_1g_port_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGDISCARDSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 7 */
        soc_reg32_get(unit, PGDISCARDSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                          DISCARDSETLIMITf,
                          discard_limit_per_port_pg_uplink_1g_port);
        soc_reg32_set(unit, PGDISCARDSETLIMITr,
                      port, 7, rval);

        /* HOLCOSMINXQCNTr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_down_1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_down_1 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit_qlayer7_pktsetlimit_down_1 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_downlink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_down_1 - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_down_1 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_down_1 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_down_1 -
                          (ethernet_mtu_cell * 2));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);
    }

    /* port-based : downlink 2.5G */
    SOC_PBMP_ITER(pbmp_downlink_2dot5g, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          1);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          2);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          3);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          4);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          5);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          6);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_downlink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLSETLIMITf,
                          xoff_cell_thresholds_per_port_2dot5g_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLRESETLIMITf,
                          xoff_cell_thresholds_per_port_2dot5g_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGDISCARDSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 7 */
        soc_reg32_get(unit, PGDISCARDSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                          DISCARDSETLIMITf,
                          discard_limit_per_port_pg_uplink_2dot5g_port);
        soc_reg32_set(unit, PGDISCARDSETLIMITr,
                      port, 7, rval);

        /* HOLCOSMINXQCNTr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_down_1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_down_2dot5 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_down_2dot5 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_down_2dot5
                              - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit_qlayer7_pktsetlimit_down_2dot5 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_down_2dot5
                              - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_downlink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_down_2dot5 - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_down_2dot5 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_down_2dot5 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_down_2dot5 -
                          (ethernet_mtu_cell * 2));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_down_2dot5
                              - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);
    }
    return SOC_E_NONE;
}

STATIC int _soc_greyhound2_mmu_init_helper_lossless(int unit)
{
    uint32 rval;
    int port, phy_port;
    int index;
    soc_info_t *si = &SOC_INFO(unit);
    soc_pbmp_t pbmp_uplink;
    soc_pbmp_t pbmp_downlink_1g;
    soc_pbmp_t pbmp_downlink_2dot5g;
    soc_pbmp_t pbmp_downlink_10g;

    int standard_jumbo_frame;
    int cell_size;
    int ethernet_mtu_cell;
    int standard_jumbo_frame_cell;
    int total_physical_memory;
    int total_cell_memory_for_admission;
    int number_of_used_memory_banks;
    int reserved_for_cfap;
    int skidmarker;
    int prefetch;
    int total_cell_memory;
    int cfapfullsetpoint;
    int total_advertised_cell_memory;
    int number_of_uplink_ports;
    int number_of_downlink_ports;
    int flow_control_type_pause_0_pfc_1;
    int queue_port_limit_ratio;
    int headroom_for_1g_port;
    int headroom_for_2dot5g_port;
    int headroom_for_10g_port;
    int headroom_for_20g_port;
    int headroom_for_25g_port;
    int headroom_for_40g_port;
    int num_1g_ports_downlink_ports;
    int num_2dot5g_ports_downlink_ports;
    int num_10g_ports_downlink_ports;
    int num_20g_ports_downlink_ports;
    int num_25g_ports_downlink_ports;
    int num_40g_ports_downlink_ports;
    int mmu_xoff_pkt_threshold_uplink_ports;
    int mmu_xoff_pkt_threshold_downlink_ports;
    int mmu_xoff_cell_threshold_1g_port_downlink_ports;
    int mmu_xoff_cell_threshold_2dot5g_port_downlink_ports;
    int mmu_xoff_cell_threshold_10g_port_downlink_ports;
    int mmu_xoff_cell_threshold_20g_port_downlink_ports;
    int mmu_xoff_cell_threshold_25g_port_downlink_ports;
    int mmu_xoff_cell_threshold_40g_port_downlink_ports;
    int mmu_xoff_cell_threshold_all_uplink_ports;
    int egress_xq_min_reserve_lossless_uplink_ports;
    int egress_xq_min_reserve_lossless_downlink_ports;
    int egress_queue_min_reserve_uplink_ports_lossy;
    int egress_queue_min_reserve_downlink_ports_lossy;
    int egress_queue_min_reserve_uplink_ports_lossless;
    int egress_queue_min_reserve_downlink_ports_lossless;
    int egress_queue_min_reserve_cpu_ports;
    int egress_xq_min_reserve_lossy_ports;
    int num_active_pri_group_lossless;
    int num_lossy_queues;
    int num_cpu_queues;
    int num_cpu_ports;
    int numxqs_per_uplink_ports;
    int numxqs_per_downlink_ports_and_cpu_port;
    int xoff_cell_thresholds_per_port_1g_port_downlink_ports;
    int xoff_cell_thresholds_per_port_2dot5g_downlink_ports;
    int xoff_cell_thresholds_per_port_10g_port_downlink_ports;
    int xoff_cell_threshold_all_uplink_ports;
    int xoff_packet_thresholds_per_port_uplink_port;
    int xoff_packet_thresholds_per_port_downlink_port;
    int discard_limit_per_port_pg_uplink_1g_port;
    int discard_limit_per_port_pg_uplink_2dot5g_port;
    int discard_limit_per_port_pg_uplink_10g_port;
    int discard_limit_per_port_pg_downlink_port;
    int total_reserved_cells_for_uplink_ports;
    int total_reserved_cells_for_downlink_ports;
    int total_reserved_cells_for_cpu_port;
    int total_reserved;
    int shared_space_cells;
    int reserved_xqs_per_uplink_port;
    int shared_xqs_per_uplink_port;
    int reserved_xqs_per_downlink_port;
    int shared_xqs_per_downlink_port;
    int cfapfullthreshold_cfapfullsetpoint_up;
    int gbllimitsetlimit_gblcellsetlimit_up;
    int totaldyncellsetlimit_totaldyncellsetlimit_up;
    int holcosminxqcnt_qlayer8_holcosminxqcnt_up;
    int holcospktsetlimit0_pktsetlimit_up;
    int holcospktsetlimit7_pktsetlimit_up;
    int holcospktsetlimit_qlayer0_pktsetlimit_up;
    int holcospktsetlimit_qlayer7_pktsetlimit_up;
    int holcospktsetlimit_qlayer8_pktsetlimit_up;
    int dynxqcntport_dynxqcntport_up;
    int lwmcoscellsetlimit0_cellsetlimit_up;
    int lwmcoscellsetlimit7_cellsetlimit_up;
    int lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    int lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    int lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    int holcoscellmaxlimit0_cellmaxlimit_up;
    int holcoscellmaxlimit7_cellmaxlimit_up;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_up;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_up;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_up;
    int dyncelllimit_dyncellsetlimit_up;
    int holcospktsetlimit_qgroup0_pktsetlimit_up;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_up;
    int holcosminxqcnt_qlayer8_holcosminxqcnt_down_1;
    int holcospktsetlimit0_pktsetlimit_down_1;
    int holcospktsetlimit7_pktsetlimit_down_1;
    int holcospktsetlimit_qlayer0_pktsetlimit_down_1;
    int holcospktsetlimit_qlayer7_pktsetlimit_down_1;
    int holcospktsetlimit_qlayer8_pktsetlimit_down_1;
    int dynxqcntport_dynxqcntport_down_1;
    int holcoscellmaxlimit0_cellmaxlimit_down_1;
    int holcoscellmaxlimit7_cellmaxlimit_down_1;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_down_1;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_down_1;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_down_1;
    int dyncelllimit_dyncellsetlimit_down_1;
    int holcospktsetlimit_qgroup0_pktsetlimit_down_1;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_down_1;
    int holcosminxqcnt_qlayer8_holcosminxqcnt_down_2dot5;
    int holcospktsetlimit0_pktsetlimit_down_2dot5;
    int holcospktsetlimit7_pktsetlimit_down_2dot5;
    int holcospktsetlimit_qlayer0_pktsetlimit_down_2dot5;
    int holcospktsetlimit_qlayer7_pktsetlimit_down_2dot5;
    int holcospktsetlimit_qlayer8_pktsetlimit_down_2dot5;
    int dynxqcntport_dynxqcntport_down_2dot5;
    int holcoscellmaxlimit0_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit7_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_down_2dot5;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_down_2dot5;
    int dyncelllimit_dyncellsetlimit_down_2dot5;
    int holcospktsetlimit_qgroup0_pktsetlimit_down_2dot5;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_down_2dot5;
    int holcosminxqcnt_qlayer8_holcosminxqcnt_down_10;
    int holcospktsetlimit0_pktsetlimit_down_10;
    int holcospktsetlimit7_pktsetlimit_down_10;
    int holcospktsetlimit_qlayer0_pktsetlimit_down_10;
    int holcospktsetlimit_qlayer7_pktsetlimit_down_10;
    int holcospktsetlimit_qlayer8_pktsetlimit_down_10;
    int dynxqcntport_dynxqcntport_down_10;
    int holcoscellmaxlimit0_cellmaxlimit_down_10;
    int holcoscellmaxlimit7_cellmaxlimit_down_10;
    int holcoscellmaxlimit_qlayer0_cellmaxlimit_down_10;
    int holcoscellmaxlimit_qlayer7_cellmaxlimit_down_10;
    int holcoscellmaxlimit_qlayer8_cellmaxlimit_down_10;
    int dyncelllimit_dyncellsetlimit_down_10;
    int holcospktsetlimit_qgroup0_pktsetlimit_down_10;
    int holcoscellmaxlimit_qgroup0_cellmaxlimit_down_10;
#if 0 /* FIXED compile error : set but not used */
    int holcospktsetlimit0_pktsetlimit_cpu;
    int holcospktsetlimit7_pktsetlimit_cpu;
    int dynxqcntport_dynxqcntport_cpu;
    int holcoscellmaxlimit0_cellmaxlimit_cpu;
    int holcoscellmaxlimit7_cellmaxlimit_cpu;
    int dyncelllimit_dyncellsetlimit_cpu;
#endif

    /* setup port bitmap according the port max speed for lossless(TBD)
     *   TSC4/TSC6/TSCF    : uplink port
     *   TSC0~3/TSC5/QGMII/SGMII : donnlink port
     */
    number_of_uplink_ports = 0;
    number_of_downlink_ports = 0;
    num_1g_ports_downlink_ports = 0;
    num_2dot5g_ports_downlink_ports = 0;
    num_10g_ports_downlink_ports = 0;
    num_20g_ports_downlink_ports = 0;
    num_25g_ports_downlink_ports = 0;
    num_40g_ports_downlink_ports = 0;
    SOC_PBMP_CLEAR (pbmp_uplink);
    SOC_PBMP_CLEAR (pbmp_downlink_1g);
    SOC_PBMP_CLEAR (pbmp_downlink_2dot5g);
    SOC_PBMP_CLEAR (pbmp_downlink_10g);
    for (phy_port = 0; phy_port < SOC_MAX_PHY_PORTS; phy_port++) {
        port = si->port_p2l_mapping[phy_port];
        if (IS_CL_PORT(unit, port) ||
            (74 <= phy_port && phy_port <= 77) ||
            (82 <= phy_port && phy_port <= 85)) {
            number_of_uplink_ports++;
            SOC_PBMP_PORT_ADD(pbmp_uplink, port);
        } else if (IS_GE_PORT(unit, port) ||
                   (58 <= phy_port && phy_port <= 73) ||
                   (78 <= phy_port && phy_port <= 81)) {
            number_of_downlink_ports++;
            if (SOC_INFO(unit).port_speed_max[port] > 2500) {
                num_10g_ports_downlink_ports++;
                SOC_PBMP_PORT_ADD(pbmp_downlink_10g, port);
            } else if (SOC_INFO(unit).port_speed_max[port] > 1000) {
                num_2dot5g_ports_downlink_ports++;
                SOC_PBMP_PORT_ADD(pbmp_downlink_2dot5g, port);
            } else {
                num_1g_ports_downlink_ports++;
                SOC_PBMP_PORT_ADD(pbmp_downlink_1g, port);
            }
        }
    }
    if (number_of_uplink_ports > 12) { /* TBD : excel says 4, but should be 12? */
        LOG_CLI((BSL_META_U(unit,
                 "Num of UplinkPorts Error,\
                 should be less than or equal to 4 (value=%d)\n"),
                 number_of_uplink_ports));
        return SOC_E_PARAM;
    }

    standard_jumbo_frame = 9216;
    cell_size = 144;
    ethernet_mtu_cell = ceiling_func(15 * 1024 / 10, cell_size);
    standard_jumbo_frame_cell =
          ceiling_func(standard_jumbo_frame, cell_size);
    total_physical_memory = 24 * 1024;
    total_cell_memory_for_admission = 225 * 1024 / 10;;
    number_of_used_memory_banks = 8;
    reserved_for_cfap = 65 * 2 + number_of_used_memory_banks * 4;
    skidmarker = 3;
    prefetch = 64 + 4;
    total_cell_memory = total_cell_memory_for_admission;
    cfapfullsetpoint = total_physical_memory - reserved_for_cfap;
    total_advertised_cell_memory = total_cell_memory;
    flow_control_type_pause_0_pfc_1 = 1;
    queue_port_limit_ratio = 8;
    headroom_for_1g_port = 30;
    headroom_for_2dot5g_port = 34;
    headroom_for_10g_port = 74;
    headroom_for_20g_port = 92;
    headroom_for_25g_port = 100;
    headroom_for_40g_port = 186;
    mmu_xoff_pkt_threshold_uplink_ports = total_advertised_cell_memory;
    mmu_xoff_pkt_threshold_downlink_ports = total_advertised_cell_memory;
    mmu_xoff_cell_threshold_1g_port_downlink_ports = headroom_for_1g_port;
    mmu_xoff_cell_threshold_2dot5g_port_downlink_ports =
          headroom_for_2dot5g_port;
    mmu_xoff_cell_threshold_10g_port_downlink_ports = headroom_for_10g_port;
    mmu_xoff_cell_threshold_20g_port_downlink_ports = headroom_for_20g_port;
    mmu_xoff_cell_threshold_25g_port_downlink_ports = headroom_for_25g_port;
    mmu_xoff_cell_threshold_40g_port_downlink_ports = headroom_for_40g_port;
    mmu_xoff_cell_threshold_all_uplink_ports = total_advertised_cell_memory;
    egress_xq_min_reserve_lossless_uplink_ports =
          (headroom_for_1g_port +
          mmu_xoff_cell_threshold_1g_port_downlink_ports) *
          num_1g_ports_downlink_ports + (headroom_for_2dot5g_port +
          mmu_xoff_cell_threshold_2dot5g_port_downlink_ports) *
          num_2dot5g_ports_downlink_ports + (headroom_for_10g_port +
          mmu_xoff_cell_threshold_10g_port_downlink_ports) *
          num_10g_ports_downlink_ports + (headroom_for_20g_port +
          mmu_xoff_cell_threshold_20g_port_downlink_ports) *
          num_20g_ports_downlink_ports + (headroom_for_25g_port +
          mmu_xoff_cell_threshold_25g_port_downlink_ports) *
          num_25g_ports_downlink_ports + (headroom_for_40g_port +
          mmu_xoff_cell_threshold_40g_port_downlink_ports) *
          num_40g_ports_downlink_ports;
    egress_xq_min_reserve_lossless_downlink_ports = 0;
    egress_queue_min_reserve_uplink_ports_lossy = ethernet_mtu_cell;
    egress_queue_min_reserve_downlink_ports_lossy = ethernet_mtu_cell;
    egress_queue_min_reserve_uplink_ports_lossless =
          egress_xq_min_reserve_lossless_uplink_ports;
    egress_queue_min_reserve_downlink_ports_lossless = 0;
    egress_queue_min_reserve_cpu_ports = ethernet_mtu_cell;
    egress_xq_min_reserve_lossy_ports =
          ethernet_mtu_cell;
    num_active_pri_group_lossless = 1;
    num_lossy_queues = 7;
    num_cpu_queues = 8;
    num_cpu_ports = 1;
    numxqs_per_uplink_ports = 6 * 1024;
    numxqs_per_downlink_ports_and_cpu_port = 2 * 1024;
    xoff_cell_thresholds_per_port_1g_port_downlink_ports =
          mmu_xoff_cell_threshold_1g_port_downlink_ports;
    xoff_cell_thresholds_per_port_2dot5g_downlink_ports =
          mmu_xoff_cell_threshold_2dot5g_port_downlink_ports;
    xoff_cell_thresholds_per_port_10g_port_downlink_ports =
          mmu_xoff_cell_threshold_10g_port_downlink_ports;
    xoff_cell_threshold_all_uplink_ports =
          mmu_xoff_cell_threshold_all_uplink_ports;
    xoff_packet_thresholds_per_port_uplink_port =
          mmu_xoff_pkt_threshold_uplink_ports;
    xoff_packet_thresholds_per_port_downlink_port =
          mmu_xoff_pkt_threshold_downlink_ports;
    discard_limit_per_port_pg_uplink_1g_port =
          xoff_cell_thresholds_per_port_1g_port_downlink_ports +
          headroom_for_1g_port;
    discard_limit_per_port_pg_uplink_2dot5g_port =
          xoff_cell_thresholds_per_port_2dot5g_downlink_ports +
          headroom_for_2dot5g_port;
    discard_limit_per_port_pg_uplink_10g_port =
          xoff_cell_thresholds_per_port_10g_port_downlink_ports +
          headroom_for_10g_port;
    discard_limit_per_port_pg_downlink_port = total_advertised_cell_memory;
    total_reserved_cells_for_uplink_ports =
          egress_queue_min_reserve_uplink_ports_lossy *
          number_of_uplink_ports * num_lossy_queues + number_of_uplink_ports *
          egress_queue_min_reserve_uplink_ports_lossless *
          num_active_pri_group_lossless;
    total_reserved_cells_for_downlink_ports =
          number_of_downlink_ports *
          egress_queue_min_reserve_downlink_ports_lossy * (num_lossy_queues) +
          number_of_downlink_ports *
          egress_queue_min_reserve_downlink_ports_lossless;
    total_reserved_cells_for_cpu_port =
          num_cpu_ports * egress_queue_min_reserve_cpu_ports * num_cpu_queues;
    total_reserved =
          total_reserved_cells_for_uplink_ports +
          total_reserved_cells_for_downlink_ports +
          total_reserved_cells_for_cpu_port;
    shared_space_cells = total_advertised_cell_memory - total_reserved;
    reserved_xqs_per_uplink_port =
          egress_xq_min_reserve_lossy_ports *
          num_lossy_queues + egress_xq_min_reserve_lossless_uplink_ports;
    shared_xqs_per_uplink_port =
          numxqs_per_uplink_ports - reserved_xqs_per_uplink_port;
    reserved_xqs_per_downlink_port =
          egress_xq_min_reserve_lossy_ports *
          num_lossy_queues + egress_xq_min_reserve_lossless_downlink_ports;
    shared_xqs_per_downlink_port =
          numxqs_per_downlink_ports_and_cpu_port -
          reserved_xqs_per_downlink_port;
    cfapfullthreshold_cfapfullsetpoint_up = cfapfullsetpoint;
    gbllimitsetlimit_gblcellsetlimit_up = total_cell_memory_for_admission;
    totaldyncellsetlimit_totaldyncellsetlimit_up = shared_space_cells;
    holcosminxqcnt_qlayer8_holcosminxqcnt_up = 0;
    holcospktsetlimit0_pktsetlimit_up =
          shared_xqs_per_uplink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_up =
          shared_xqs_per_uplink_port +
          egress_xq_min_reserve_lossless_uplink_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_up =
          shared_xqs_per_uplink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_up =
          shared_xqs_per_uplink_port +
          egress_xq_min_reserve_lossless_uplink_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_up =
          shared_xqs_per_uplink_port +
          holcosminxqcnt_qlayer8_holcosminxqcnt_up;
    dynxqcntport_dynxqcntport_up =
          shared_xqs_per_uplink_port - skidmarker - prefetch;
    lwmcoscellsetlimit0_cellsetlimit_up =
          egress_queue_min_reserve_uplink_ports_lossy;
    lwmcoscellsetlimit7_cellsetlimit_up =
          egress_queue_min_reserve_uplink_ports_lossless;
    lwmcoscellsetlimit_qlayer0_cellsetlimit_up =
          egress_queue_min_reserve_uplink_ports_lossy;
    lwmcoscellsetlimit_qlayer7_cellsetlimit_up =
          egress_queue_min_reserve_uplink_ports_lossless;
    lwmcoscellsetlimit_qlayer8_cellsetlimit_up = 0;
    holcoscellmaxlimit0_cellmaxlimit_up =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_up =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_up =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_up =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_up =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_up = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_up = numxqs_per_uplink_ports - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_up =
          ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);
    holcosminxqcnt_qlayer8_holcosminxqcnt_down_1 = 0;
    holcospktsetlimit0_pktsetlimit_down_1 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_down_1 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_down_1 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_down_1 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_down_1 =
          shared_xqs_per_downlink_port +
          holcosminxqcnt_qlayer8_holcosminxqcnt_down_1;
    dynxqcntport_dynxqcntport_down_1 =
          shared_xqs_per_downlink_port - skidmarker - prefetch;
    holcoscellmaxlimit0_cellmaxlimit_down_1 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_down_1 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_down_1 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_down_1 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_down_1 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_down_1 = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_down_1 =
          numxqs_per_downlink_ports_and_cpu_port - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_down_1 =
          ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);
    holcosminxqcnt_qlayer8_holcosminxqcnt_down_2dot5 = 0;
    holcospktsetlimit0_pktsetlimit_down_2dot5 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_down_2dot5 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_down_2dot5 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_down_2dot5 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_down_2dot5 =
          shared_xqs_per_downlink_port +
          holcosminxqcnt_qlayer8_holcosminxqcnt_down_2dot5;
    dynxqcntport_dynxqcntport_down_2dot5 =
          shared_xqs_per_downlink_port - skidmarker - prefetch;
    holcoscellmaxlimit0_cellmaxlimit_down_2dot5 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_down_2dot5 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_down_2dot5 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_down_2dot5 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_down_2dot5 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_down_2dot5 = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_down_2dot5 =
          numxqs_per_downlink_ports_and_cpu_port - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_down_2dot5 =
          ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);
    holcosminxqcnt_qlayer8_holcosminxqcnt_down_10 = 0;
    holcospktsetlimit0_pktsetlimit_down_10 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_down_10 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    holcospktsetlimit_qlayer0_pktsetlimit_down_10 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit_qlayer7_pktsetlimit_down_10 =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    holcospktsetlimit_qlayer8_pktsetlimit_down_10 =
          shared_xqs_per_downlink_port +
          holcosminxqcnt_qlayer8_holcosminxqcnt_down_10;
    dynxqcntport_dynxqcntport_down_10 =
          shared_xqs_per_downlink_port - skidmarker - prefetch;
    holcoscellmaxlimit0_cellmaxlimit_down_10 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_down_10 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer0_cellmaxlimit_down_10 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer0_cellsetlimit_up;
    holcoscellmaxlimit_qlayer7_cellmaxlimit_down_10 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer7_cellsetlimit_up;
    holcoscellmaxlimit_qlayer8_cellmaxlimit_down_10 =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit_qlayer8_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_down_10 = shared_space_cells;
    holcospktsetlimit_qgroup0_pktsetlimit_down_10 =
          numxqs_per_downlink_ports_and_cpu_port - 1;
    holcoscellmaxlimit_qgroup0_cellmaxlimit_down_10 =
          ceiling_func(total_advertised_cell_memory, queue_port_limit_ratio);
#if 0 /* FIXED compile error : set but not used */
    holcospktsetlimit0_pktsetlimit_cpu =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossy_ports;
    holcospktsetlimit7_pktsetlimit_cpu =
          shared_xqs_per_downlink_port +
          egress_xq_min_reserve_lossless_downlink_ports;
    dynxqcntport_dynxqcntport_cpu =
          shared_xqs_per_downlink_port - skidmarker - prefetch;
    holcoscellmaxlimit0_cellmaxlimit_cpu =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit0_cellsetlimit_up;
    holcoscellmaxlimit7_cellmaxlimit_cpu =
          ceiling_func(shared_space_cells, queue_port_limit_ratio) +
          lwmcoscellsetlimit7_cellsetlimit_up;
    dyncelllimit_dyncellsetlimit_cpu = shared_space_cells;
#endif

    if ((shared_space_cells * cell_size)/1024 <= 800) {
        LOG_CLI((BSL_META_U(unit,
                 "Shared Pool Is Small,\
                 should be larger than 800 (value=%d)\n"),
                 (shared_space_cells * cell_size)/1024));
        return SOC_E_PARAM;
    }

    /* system-based */
    soc_reg_field32_modify(unit, CFAPFULLTHRESHOLDr,
                           REG_PORT_ANY,
                           CFAPFULLSETPOINTf,
                           cfapfullsetpoint);
    soc_reg_field32_modify(unit, CFAPFULLTHRESHOLDr,
                           REG_PORT_ANY,
                           CFAPFULLRESETPOINTf,
                           cfapfullthreshold_cfapfullsetpoint_up -
                           (standard_jumbo_frame_cell * 2));
    soc_reg_field32_modify(unit, GBLLIMITSETLIMITr,
                           REG_PORT_ANY,
                           GBLCELLSETLIMITf,
                           total_cell_memory_for_admission);
    soc_reg_field32_modify(unit, GBLLIMITRESETLIMITr,
                           REG_PORT_ANY,
                           GBLCELLRESETLIMITf,
                           gbllimitsetlimit_gblcellsetlimit_up);
    soc_reg_field32_modify(unit, TOTALDYNCELLSETLIMITr,
                           REG_PORT_ANY,
                           TOTALDYNCELLSETLIMITf,
                           shared_space_cells);
    soc_reg_field32_modify(unit, TOTALDYNCELLRESETLIMITr,
                           REG_PORT_ANY,
                           TOTALDYNCELLRESETLIMITf,
                           totaldyncellsetlimit_totaldyncellsetlimit_up -
                           (standard_jumbo_frame_cell * 2));
    soc_reg_field32_modify(unit, TWO_LAYER_SCH_MODEr,
                           REG_PORT_ANY,
                           SCH_MODEf,
                           0);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           MULTIPLE_ACCOUNTING_FIX_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           CNG_DROP_ENf,
                           0);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           DYN_XQ_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           HOL_CELL_SOP_DROP_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           DYNAMIC_MEMORY_ENf,
                           1);
    soc_reg_field32_modify(unit, MISCCONFIGr,
                           REG_PORT_ANY,
                           SKIDMARKERf,
                           skidmarker);
    soc_reg_field32_modify(unit, MMUPORTTXENABLE_0r,
                           REG_PORT_ANY,
                           MMUPORTTXENABLEf,
                           0xFFFFFFFF);
    soc_reg_field32_modify(unit, MMUPORTTXENABLE_1r,
                           REG_PORT_ANY,
                           MMUPORTTXENABLEf,
                           0xFFFFFFFF);
    soc_reg_field32_modify(unit, MMUPORTTXENABLE_2r,
                           REG_PORT_ANY,
                           MMUPORTTXENABLEf,
                           3);

    /* port-based : uplink */
    SOC_PBMP_ITER(pbmp_uplink, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          0);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?0:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?1:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?2:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?3:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?4:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?5:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?6:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_uplink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          0);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNTr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNTr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNTr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_uplink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNTr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_uplink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_uplink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_uplink_port +
                          egress_xq_min_reserve_lossless_uplink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_uplink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_uplink_port +
                          egress_xq_min_reserve_lossless_uplink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_uplink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_up);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_up - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer7_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_uplink_ports - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_uplink_ports - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_uplink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_up - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_uplink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_uplink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_uplink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_uplink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_uplink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_up -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_up -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_up -
                          (2 * ethernet_mtu_cell));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_up - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_uplink_ports - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_up -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          127);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);
    }

    /* port-based : downlink 1G */
    SOC_PBMP_ITER(pbmp_downlink_1g, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          128);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?0:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?1:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?2:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?3:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?4:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?5:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?6:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* PG2TCr, index 7 */
        soc_reg32_get(unit, PG2TCr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PG2TCr, &rval,
                          PG_BMPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?128:255);
        soc_reg32_set(unit, PG2TCr,
                      port, 7, rval);

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_downlink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          128);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          128);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLSETLIMITf,
                          xoff_cell_thresholds_per_port_1g_port_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLRESETLIMITf,
                          xoff_cell_thresholds_per_port_1g_port_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGDISCARDSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 7 */
        soc_reg32_get(unit, PGDISCARDSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                          DISCARDSETLIMITf,
                          discard_limit_per_port_pg_uplink_1g_port);
        soc_reg32_set(unit, PGDISCARDSETLIMITr,
                      port, 7, rval);

        /* HOLCOSMINXQCNTr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNTr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNTr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNTr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_downlink_port +
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_downlink_port +
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_down_1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_down_1 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer7_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_downlink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_down_1 - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_down_1 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_down_1 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_down_1 -
                          (ethernet_mtu_cell * 2));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_down_1 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_down_1 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          127);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);
    }

    /* port-based : downlink 2.5G */
    SOC_PBMP_ITER(pbmp_downlink_2dot5g, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          128);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?0:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?1:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?2:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?3:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?4:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?5:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?6:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* PG2TCr, index 7 */
        soc_reg32_get(unit, PG2TCr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PG2TCr, &rval,
                          PG_BMPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?128:255);
        soc_reg32_set(unit, PG2TCr,
                      port, 7, rval);

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_downlink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          128);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          128);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLSETLIMITf,
                          xoff_cell_thresholds_per_port_2dot5g_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLRESETLIMITf,
                          xoff_cell_thresholds_per_port_2dot5g_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGDISCARDSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 7 */
        soc_reg32_get(unit, PGDISCARDSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                          DISCARDSETLIMITf,
                          discard_limit_per_port_pg_uplink_2dot5g_port);
        soc_reg32_set(unit, PGDISCARDSETLIMITr,
                      port, 7, rval);

        /* HOLCOSMINXQCNTr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNTr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNTr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNTr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_downlink_port +
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_downlink_port +
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_down_2dot5);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_down_2dot5 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_down_2dot5 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_down_2dot5 -
                              1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer7_pktsetlimit_down_2dot5 -
                              1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_down_2dot5 -
                              1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_downlink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_down_2dot5 - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_down_2dot5 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_down_2dot5 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_down_2dot5 -
                          (ethernet_mtu_cell * 2));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_down_2dot5 -
                              1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_down_2dot5 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          127);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);

    }

    /* port-based : downlink 10G */
    SOC_PBMP_ITER(pbmp_downlink_10g, port) {
        /* PG_CTRL0r, index 0 */
        soc_reg32_get(unit, PG_CTRL0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PPFC_PG_ENf,
                          128);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI0_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?0:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI1_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?1:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI2_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?2:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI3_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?3:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI4_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?4:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI5_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?5:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI6_GRPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?6:7);
        soc_reg_field_set(unit, PG_CTRL0r, &rval,
                          PRI7_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL0r,
                      port, 0, rval);

        /* PG_CTRL1r, index 0 */
        soc_reg32_get(unit, PG_CTRL1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI8_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI9_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI10_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI11_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI12_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI13_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI14_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg_field_set(unit, PG_CTRL1r, &rval,
                          PRI15_GRPf,
                          /* coverity[identical_branches : FALSE] */
                          flow_control_type_pause_0_pfc_1?7:7);
        soc_reg32_set(unit, PG_CTRL1r,
                      port, 0, rval);

        /* PG2TCr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PG2TCr,
                          port, index, &rval);
            soc_reg_field_set(unit, PG2TCr, &rval,
                              PG_BMPf,
                              0);
            soc_reg32_set(unit, PG2TCr,
                          port, index, rval);
        }

        /* PG2TCr, index 7 */
        soc_reg32_get(unit, PG2TCr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PG2TCr, &rval,
                          PG_BMPf,
                          /* coverity[dead_error_line : FALSE] */
                          flow_control_type_pause_0_pfc_1?128:255);
        soc_reg32_set(unit, PG2TCr,
                      port, 7, rval);

        /* IBPPKTSETLIMITr, index 0 */
        soc_reg32_get(unit, IBPPKTSETLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          xoff_packet_thresholds_per_port_downlink_port);
        soc_reg_field_set(unit, IBPPKTSETLIMITr, &rval,
                          RESETLIMITSELf,
                          3);
        soc_reg32_set(unit, IBPPKTSETLIMITr,
                      port, 0, rval);

        /* MMU_FC_RX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_RX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_RX_ENr, &rval,
                          MMU_FC_RX_ENABLEf,
                          128);
        soc_reg32_set(unit, MMU_FC_RX_ENr,
                      port, 0, rval);

        /* MMU_FC_TX_ENr, index 0 */
        soc_reg32_get(unit, MMU_FC_TX_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, MMU_FC_TX_ENr, &rval,
                          MMU_FC_TX_ENABLEf,
                          128);
        soc_reg32_set(unit, MMU_FC_TX_ENr,
                      port, 0, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLSETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLSETLIMITf,
                          xoff_cell_thresholds_per_port_10g_port_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGCELLLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGCELLLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                              CELLRESETLIMITf,
                              xoff_cell_threshold_all_uplink_ports);
            soc_reg32_set(unit, PGCELLLIMITr,
                          port, index, rval);
        }

        /* PGCELLLIMITr, index 7 */
        soc_reg32_get(unit, PGCELLLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGCELLLIMITr, &rval,
                          CELLRESETLIMITf,
                          xoff_cell_thresholds_per_port_10g_port_downlink_ports);
        soc_reg32_set(unit, PGCELLLIMITr,
                      port, 7, rval);

        /* PGDISCARDSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, PGDISCARDSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                              DISCARDSETLIMITf,
                              discard_limit_per_port_pg_downlink_port);
            soc_reg32_set(unit, PGDISCARDSETLIMITr,
                          port, index, rval);
        }

        /* PGDISCARDSETLIMITr, index 7 */
        soc_reg32_get(unit, PGDISCARDSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, PGDISCARDSETLIMITr, &rval,
                          DISCARDSETLIMITf,
                          discard_limit_per_port_pg_uplink_10g_port);
        soc_reg32_set(unit, PGDISCARDSETLIMITr,
                      port, 7, rval);

        /* HOLCOSMINXQCNTr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNTr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNTr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNTr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNTr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNTr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNTr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSMINXQCNT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                          HOLCOSMINXQCNTf,
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSMINXQCNT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSMINXQCNT_QLAYERr, &rval,
                              HOLCOSMINXQCNTf,
                              0);
            soc_reg32_set(unit, HOLCOSMINXQCNT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_downlink_port +
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              egress_xq_min_reserve_lossy_ports);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                          PKTSETLIMITf,
                          shared_xqs_per_downlink_port +
                          egress_xq_min_reserve_lossless_downlink_ports);
        soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSPKTSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QLAYERr, &rval,
                              PKTSETLIMITf,
                              shared_xqs_per_downlink_port +
                              holcosminxqcnt_qlayer8_holcosminxqcnt_down_10);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit0_pktsetlimit_down_10 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSPKTRESETLIMITr, &rval,
                          PKTRESETLIMITf,
                          holcospktsetlimit7_pktsetlimit_down_10 - 1);
        soc_reg32_set(unit, HOLCOSPKTRESETLIMITr,
                      port, 7, rval);

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer0_pktsetlimit_down_10 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer7_pktsetlimit_down_10 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qlayer8_pktsetlimit_down_10 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0r, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1r, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1r,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1r, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1r,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QLAYERr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QLAYERr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QLAYERr, index 0 ~ 63 */
        for (index = 0; index <= 63; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QLAYERr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QLAYERr,
                          port, index, rval);
        }

        /* CNGPORTPKTLIMIT0r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT0r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT0r, &rval,
                          CNGPORTPKTLIMIT0f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT0r,
                      port, 0, rval);

        /* CNGPORTPKTLIMIT1r, index 0 */
        soc_reg32_get(unit, CNGPORTPKTLIMIT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, CNGPORTPKTLIMIT1r, &rval,
                          CNGPORTPKTLIMIT1f,
                          numxqs_per_downlink_ports_and_cpu_port - 1);
        soc_reg32_set(unit, CNGPORTPKTLIMIT1r,
                      port, 0, rval);

        /* DYNXQCNTPORTr, index 0 */
        soc_reg32_get(unit, DYNXQCNTPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNXQCNTPORTr, &rval,
                          DYNXQCNTPORTf,
                          shared_xqs_per_downlink_port - skidmarker - prefetch);
        soc_reg32_set(unit, DYNXQCNTPORTr,
                      port, 0, rval);

        /* DYNRESETLIMPORTr, index 0 */
        soc_reg32_get(unit, DYNRESETLIMPORTr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNRESETLIMPORTr, &rval,
                          DYNRESETLIMPORTf,
                          dynxqcntport_dynxqcntport_down_10 - 2);
        soc_reg32_set(unit, DYNRESETLIMPORTr,
                      port, 0, rval);

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLSETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLSETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMITr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMITr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              egress_queue_min_reserve_downlink_ports_lossy);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                          CELLRESETLIMITf,
                          egress_queue_min_reserve_downlink_ports_lossless);
        soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                      port, 7, rval);

        /* LWMCOSCELLSETLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, LWMCOSCELLSETLIMIT_QLAYERr, &rval,
                              CELLRESETLIMITf,
                              0);
            soc_reg32_set(unit, LWMCOSCELLSETLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                              queue_port_limit_ratio) +
                              lwmcoscellsetlimit0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                          queue_port_limit_ratio) +
                          lwmcoscellsetlimit7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer0_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXLIMITf,
                          ceiling_func(shared_space_cells,
                                       queue_port_limit_ratio) +
                          lwmcoscellsetlimit_qlayer7_cellsetlimit_up);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(shared_space_cells,
                                           queue_port_limit_ratio) +
                              lwmcoscellsetlimit_qlayer8_cellsetlimit_up);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit0_cellmaxlimit_down_10 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMITr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMITr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit7_cellmaxlimit_down_10 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMITr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 0 ~ 6 */
        for (index = 0; index <= 6; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer0_cellmaxlimit_down_10 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 7 */
        soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, &rval);
        soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                          CELLMAXRESUMELIMITf,
                          holcoscellmaxlimit_qlayer7_cellmaxlimit_down_10 -
                          ethernet_mtu_cell);
        soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                      port, 7, rval);

        /* HOLCOSCELLMAXLIMIT_QLAYERr, index 8 ~ 63 */
        for (index = 8; index <= 63; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qlayer8_cellmaxlimit_down_10 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QLAYERr,
                          port, index, rval);
        }

        /* DYNCELLLIMITr, index 0 */
        soc_reg32_get(unit, DYNCELLLIMITr,
                      port, 0, &rval);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLSETLIMITf,
                          shared_space_cells);
        soc_reg_field_set(unit, DYNCELLLIMITr, &rval,
                          DYNCELLRESETLIMITf,
                          dyncelllimit_dyncellsetlimit_down_10 -
                          (ethernet_mtu_cell * 2));
        soc_reg32_set(unit, DYNCELLLIMITr,
                      port, 0, rval);

        /* COLOR_DROP_ENr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_ENr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_ENr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_ENr,
                      port, 0, rval);

        /* COLOR_DROP_EN_QLAYERr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QLAYERr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QLAYERr,
                      port, 0, rval);

        /* HOLCOSPKTSETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTSETLIMIT_QGROUPr, &rval,
                              PKTSETLIMITf,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, HOLCOSPKTSETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSPKTRESETLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr, &rval,
                              PKTRESETLIMITf,
                              holcospktsetlimit_qgroup0_pktsetlimit_down_10 - 1);
            soc_reg32_set(unit, HOLCOSPKTRESETLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT0_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT0_QGROUPr, &rval,
                              CNGPKTSETLIMIT0f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT0_QGROUPr,
                          port, index, rval);
        }

        /* CNGCOSPKTLIMIT1_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, CNGCOSPKTLIMIT1_QGROUPr, &rval,
                              CNGPKTSETLIMIT1f,
                              numxqs_per_downlink_ports_and_cpu_port - 1);
            soc_reg32_set(unit, CNGCOSPKTLIMIT1_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXLIMITf,
                              ceiling_func(total_advertised_cell_memory,
                                           queue_port_limit_ratio));
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* HOLCOSCELLMAXLIMIT_QGROUPr, index 0 ~ 7 */
        for (index = 0; index <= 7; index++) {
            soc_reg32_get(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, &rval);
            soc_reg_field_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr, &rval,
                              CELLMAXRESUMELIMITf,
                              holcoscellmaxlimit_qgroup0_cellmaxlimit_down_10 -
                              ethernet_mtu_cell);
            soc_reg32_set(unit, HOLCOSCELLMAXLIMIT_QGROUPr,
                          port, index, rval);
        }

        /* COLOR_DROP_EN_QGROUPr, index 0 */
        soc_reg32_get(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, &rval);
        soc_reg_field_set(unit, COLOR_DROP_EN_QGROUPr, &rval,
                          COLOR_DROP_ENf,
                          0);
        soc_reg32_set(unit, COLOR_DROP_EN_QGROUPr,
                      port, 0, rval);

        /* SHARED_POOL_CTRLr, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRLr,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          255);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_DISCARD_ENf,
                          127);
        soc_reg_field_set(unit, SHARED_POOL_CTRLr, &rval,
                          SHARED_POOL_XOFF_ENf,
                          0);
        soc_reg32_set(unit, SHARED_POOL_CTRLr,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT1r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT1r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT1r,
                      port, 0, rval);

        /* SHARED_POOL_CTRL_EXT2r, index 0 */
        soc_reg32_get(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, &rval);
        soc_reg_field_set(unit, SHARED_POOL_CTRL_EXT2r, &rval,
                          DYNAMIC_COS_DROP_ENf,
                          0xFFFFFFFF);
        soc_reg32_set(unit, SHARED_POOL_CTRL_EXT2r,
                      port, 0, rval);
    }

    return SOC_E_NONE;
}

#define  GH2_NUM_COS  8

STATIC int
_soc_greyhound2_mmu_init(int unit)
{
    int         cos;
    soc_port_t  port;
    uint32      val, oval, cfap_max_idx;
    int         age[GH2_NUM_COS], max_age, min_age;
    int         age_enable, disabled_age;
    uint16      dev_id;
    uint8       rev_id;
    soc_info_t *si = &SOC_INFO(unit);
    pbmp_t mmu_pbmp;
    int phy_port;
    int mmu_port;
    int qgroup;

    soc_cm_get_id(unit, &dev_id, &rev_id);
    if ((dev_id & 0xF000) == 0x8000) {
        dev_id &= 0xFF0F;
    }
    SOC_IF_ERROR_RETURN(_soc_greyhound2_tdm_init(unit, dev_id));

    cfap_max_idx = GH2_MMU_CBP_FULL_SIZE;
    SOC_IF_ERROR_RETURN(READ_CFAPCONFIGr(unit, &val));
    oval = val;
    soc_reg_field_set(unit, CFAPCONFIGr, &val, CFAPPOOLSIZEf, cfap_max_idx);
    if (oval != val) {
        SOC_IF_ERROR_RETURN(WRITE_CFAPCONFIGr(unit, val));
    }

    /* Enable IP to CMICM credit transfer */
    val = 0;
    soc_reg_field_set(unit, IP_TO_CMICM_CREDIT_TRANSFERr,
                      &val, TRANSFER_ENABLEf, 1);
    soc_reg_field_set(unit, IP_TO_CMICM_CREDIT_TRANSFERr,
                      &val, NUM_OF_CREDITSf, 32);
    SOC_IF_ERROR_RETURN(WRITE_IP_TO_CMICM_CREDIT_TRANSFERr(unit, val));

    if (soc_property_get(unit, spn_MMU_LOSSLESS, 0)) {
        SOC_IF_ERROR_RETURN(_soc_greyhound2_mmu_init_helper_lossless(unit));
    } else {
        SOC_IF_ERROR_RETURN(_soc_greyhound2_mmu_init_helper_lossy(unit));
    }

    /*
     * Configure per-XQ packet aging for the various COSQs.
     *
     * The shortest age allowed by H/W is 250 microseconds.
     * The longest age allowed is 7.162 seconds (7162 msec).
     * The maximum ratio between the longest age and the shortest
     * (nonzero) age is 7:2.
     */
    age_enable = disabled_age = max_age = 0;
    min_age = 7162;
    for (cos = 0; cos < NUM_COS(unit); cos++) {
        if ((age[cos] =
             soc_property_suffix_num_get(unit, cos, spn_MMU_XQ_AGING,
                                         "cos",  0)) > 0) {
            age_enable = 1;
            if (age[cos] > 7162) {
                age[cos] = 7162;
            }
            if (age[cos] < min_age) {
                min_age = age[cos];
            }
        } else {
            disabled_age = 1;
            age[cos] = 0;
        }
        if (age[cos] > max_age) {
            max_age = age[cos];
        }
    }
    if (!age_enable) {
        /* Disable packet aging on all COSQs */
        SOC_IF_ERROR_RETURN(WRITE_PKTAGINGTIMERr(unit, 0));
        for (qgroup = 0; qgroup < 8; qgroup++) {
            SOC_IF_ERROR_RETURN(WRITE_PKTAGINGLIMIT_r(unit, qgroup, 0));
        }
    } else {
        uint32 regval = 0;
        uint32 timerval;

        /* Enforce the 7:2 ratio between min and max values */
        if ((((max_age * 2) + 6) / 7) > min_age) {
            /* Keep requested max age; make min_age comply */
            min_age = ((max_age * 2) + 6) / 7;
        }

        /*
         * Give up granularity for range, if we need to
         * "disable" (max out) aging for any COSQ(s).
         */
        if (disabled_age) {
            /* Max range */
            max_age = min_age * 7 / 2;
        }

        /*
         * Compute shortest duration of one PKTAGINGTIMERr cycle.
         * This duration is 1/7th of the longest packet age.
         * This duration is in units of 125 usec (msec * 8).
         */
        timerval = ((8 * max_age) + 6) / 7;
        SOC_IF_ERROR_RETURN(WRITE_PKTAGINGTIMERr(unit, timerval));

        for (cos = 0; cos < NUM_COS(unit); cos++) {
            if (!age[cos]) {
                /*
                 * Requested to be disabled, but cannot disable individual
                 * COSQs once packet aging is enabled. Therefore, mark
                 * this COSQ's aging duration as maxed out.
                 */
                age[cos] = -1;
            } else if (age[cos] < min_age) {
                age[cos] = min_age;
            }

            /* Normalize each "age" into # of PKTAGINGTIMERr cycles. */
        if (age[cos] > 0) {
            /* coverity[divide_by_zero : FALSE] */
            age[cos] = ((8 * age[cos]) + timerval - 1) / timerval;
        }
        else {
            age[cos] = 7;
        }
            /* Format each "age" for its appropriate field */
            regval |= ((7 - age[cos]) << (cos * 3));
        }
        for (qgroup = 0; qgroup < 8; qgroup++) {
            SOC_IF_ERROR_RETURN(WRITE_PKTAGINGLIMIT_r(unit, qgroup, regval));
        }
    }

    /*
     * MMU Port enable
     */
    SOC_PBMP_CLEAR(mmu_pbmp);
    PBMP_ALL_ITER(unit, port) {
        phy_port = si->port_l2p_mapping[port];
        mmu_port = si->port_p2m_mapping[phy_port];
        SOC_PBMP_PORT_ADD(mmu_pbmp, mmu_port);
    }
    val = 0;
    soc_reg_field_set(unit, MMUPORTENABLE_0r, &val, MMUPORTENABLEf,
                        SOC_PBMP_WORD_GET(mmu_pbmp, 0));
    SOC_IF_ERROR_RETURN(WRITE_MMUPORTENABLE_0r(unit, val));
    val = 0;
    soc_reg_field_set(unit, MMUPORTENABLE_1r, &val, MMUPORTENABLEf,
                        SOC_PBMP_WORD_GET(mmu_pbmp, 1));
    SOC_IF_ERROR_RETURN(WRITE_MMUPORTENABLE_1r(unit, val));
    val = 0;
    soc_reg_field_set(unit, MMUPORTENABLE_2r, &val, MMUPORTENABLEf,
                        SOC_PBMP_WORD_GET(mmu_pbmp, 2));
    SOC_IF_ERROR_RETURN(WRITE_MMUPORTENABLE_2r(unit, val));

    return SOC_E_NONE;
}

/* soc_greyhound2_tsc_reset()
 *  - to reset TSCE, TSCF and QTC on Greyhound2
 */
int
soc_greyhound2_tsc_reset(int unit)
{
    int blk, port;
    uint32 rval;

    /* Reset QTC */
    SOC_BLOCK_ITER(unit, blk, SOC_BLK_PMQ) {
        port = blk | SOC_REG_ADDR_BLOCK_ID_MASK;
        SOC_IF_ERROR_RETURN(soc_tsc_xgxs_reset(unit, port, 0));
    }

    /* Reset TSCE */
    SOC_BLOCK_ITER(unit, blk, SOC_BLK_XLPORT) {
        port = SOC_BLOCK_PORT(unit, blk);
        SOC_IF_ERROR_RETURN(soc_tsc_xgxs_reset(unit, port, 0));
    }

    /* Reset TSCF */
    SOC_BLOCK_ITER(unit, blk, SOC_BLK_CLPORT) {
        port = SOC_BLOCK_PORT(unit, blk);
        SOC_IF_ERROR_RETURN(soc_tsc_xgxs_reset(unit, port, 0));
    }

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_XLPORT) {
        port = SOC_BLOCK_PORT(unit, blk);
        if (port == -1 || port == REG_PORT_ANY) {
            continue;
        }
        SOC_IF_ERROR_RETURN(READ_XLPORT_MAC_CONTROLr(unit, port, &rval));
        soc_reg_field_set(unit, XLPORT_MAC_CONTROLr, &rval, XMAC0_RESETf, 1);
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_MAC_CONTROLr(unit, port, rval));
        sal_udelay(10);
        soc_reg_field_set(unit, XLPORT_MAC_CONTROLr, &rval, XMAC0_RESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_MAC_CONTROLr(unit, port, rval));
    }

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_CLPORT) {
        port = SOC_BLOCK_PORT(unit, blk);
        if (port == -1 || port == REG_PORT_ANY) {
            continue;
        }
        SOC_IF_ERROR_RETURN(READ_CLPORT_MAC_CONTROLr(unit, port, &rval));
        soc_reg_field_set(unit, CLPORT_MAC_CONTROLr, &rval, XMAC0_RESETf, 1);
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_MAC_CONTROLr(unit, port, rval));
        sal_udelay(10);
        soc_reg_field_set(unit, CLPORT_MAC_CONTROLr, &rval, XMAC0_RESETf, 0);
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_MAC_CONTROLr(unit, port, rval));
    }

    return SOC_E_NONE;
}

extern int
(*_phy_tsce_firmware_set_helper[SOC_MAX_NUM_DEVICES])(int, int, uint8 *, int);
extern int
(*_phy_tscf_firmware_set_helper[SOC_MAX_NUM_DEVICES])(int, int, uint8 *, int);
extern int
(*_phy_qtce_firmware_set_helper[SOC_MAX_NUM_DEVICES])(int, int, uint8 *, int);


STATIC int
_soc_greyhound2_tscx_firmware_set(int unit, int port, uint8 *array, int datalen)
{
    soc_mem_t mem = IS_CL_PORT(unit, port) ? CLPORT_WC_UCMEM_DATAm :
                    (IS_XL_PORT(unit, port) ? XLPORT_WC_UCMEM_DATAm :
                                              GPORT_WC_UCMEM_DATAm);
    soc_reg_t reg = IS_CL_PORT(unit, port) ? CLPORT_WC_UCMEM_CTRLr :
                    (IS_XL_PORT(unit, port) ? XLPORT_WC_UCMEM_CTRLr :
                                              GPORT_WC_UCMEM_CTRLr);

    return soc_warpcore_firmware_set(unit, port, array, datalen, 0, mem, reg);
}

#define INVALID_PPORT_ID    -1
/*
 * Func : _soc_greyhound2_mdio_addr_to_port
 *  - To report pport_id from given phy_addr(for INT-PHY)
 *
 * INFO :
 *  - This is HW dependent design in TOP Spec.
 *      1. Int-MDIO-Bus0 : SGMII4Px2_0 ~ SGMII4Px2_2 (each for 8 ports)
 *      2. Int-MDIO-Bus1 : TSCE0 ~ TSCE6 (each for 4 ports)
 *      3. Int-MDIO-Bus2 : QTC0 ~ QTC1 (each for 16 ports)
 *      4. Int-MDIO-Bus3 : TSCF (each for 4 ports)
 *  - Param of "phy_addr" here has been constructed by phymod driver.
 */
STATIC int
_soc_greyhound2_mdio_addr_to_port(uint32 phy_addr_int)
{
    int bus, offset = 0;
    int mdio_addr;

    /* Must be internal MDIO address */
    if ((phy_addr_int & 0x80) == 0) {
        return INVALID_PPORT_ID;
    }

    bus = PHY_ID_BUS_NUM(phy_addr_int);
    mdio_addr = phy_addr_int & 0x1f;

    if (bus == 0) {
        /* for SGMII4Px2 */
        if (mdio_addr <= 0x18) {
            offset = 1;
        } else {
            return INVALID_PPORT_ID;
        }
    } else if (bus == 1) {
        /* for TSCE */
        if (mdio_addr <= 0x1c) {
            offset = 57;
        } else {
            return INVALID_PPORT_ID;
        }
    } else if (bus == 2) {
        if (mdio_addr <= 4) {
            /* for QTC0 */
            offset = 25;
        } else if (mdio_addr <= 8) {
            /* for QTC1 */
            offset = 37;
        } else {
            return INVALID_PPORT_ID;
        }
    } else if (bus == 3) {
        /* for TSCF */
        if (mdio_addr <= 0x4) {
            offset = 85;
        } else {
            return INVALID_PPORT_ID;
        }
    } else {
        return INVALID_PPORT_ID;
    }

    return mdio_addr + offset;
}

/*
 * PHY_REG address decode for TSCx phy access :
 *  - TSC_ADDR[31:0]={DEVICE(ADVAD)[31:27],??[26:24],TSC_ID(PHYAD)[23:19],
 *      PORT_MODE(LANE)[18:16], IEEE_BIT[15], BLOCK[14:4], REG[3:0]}
 */
#define TSC_REG_ADDR_TSCID_SET(_phy_reg, _phyad)    \
                            ((_phy_reg) |= ((_phyad) & 0x1f) << 19)

/*
 *  GH2's PMQ block definition.
 *      1. PMQ0-PMQ2 : SGMII4Px2_0-SGMII4Px2_2 for pport2-pport25
 *          >> Expected no SBUS-MDIO for SGMII4Px2
 *      2. PMQ3-PMQ4 for QTC0-QCT1 for pport26-pport57
 */
#define GH2_PMQ3_MIN_PPORT  26
#define GH2_PMQ3_MAX_PPORT  41
#define GH2_PMQ4_MIN_PPORT  42
#define GH2_PMQ4_MAX_PPORT  57

/*
 * Func :  soc_greyhound2_sbus_tsc_block()
 *  - to report the valid blk of this port for wc_ucmem_data
 *      (i.e. GPORT/XLPORT/CLPORT_WC_UCMEM_DATAm)access process.
 *
 *  - Parameters :
 *      phy_port(IN) : physical port.
 *      blk(OUT) : the valid block
 *
 * INFO :
 *  - PMQ block in GH2 are
 *      1. PMQ0-PMQ2 : SGMII4Px2_0-SGMII4Px2_2 for pport2-pport25
 *          >> Expected no SBUS-MDIO for SGMII4Px2
 *      2. PMQ3-PMQ4 for QTC0-QCT1 for pport26-pport57
 */
int
soc_greyhound2_sbus_tsc_block(int unit, int phy_port, int *blk)
{
    soc_info_t *si = &SOC_INFO(unit);

    *blk = -1;
    if (SOC_PORT_BLOCK_TYPE(unit, phy_port) == SOC_BLK_GPORT) {
        if ((phy_port >= GH2_PMQ3_MIN_PPORT) &&
                (phy_port <= GH2_PMQ3_MAX_PPORT)) {
            *blk = si->pmq_block[3];
        } else if ((phy_port >= GH2_PMQ4_MIN_PPORT) &&
                (phy_port <= GH2_PMQ4_MAX_PPORT)) {
            *blk = si->pmq_block[4];
        } else {
            /* Unexpected GE port! Only QTC ports need SBus-MDIO */
            LOG_WARN(BSL_LS_SOC_MII, (BSL_META_U(unit,
                     "unit(%d),pport(%d) is unable for TSC SBus-MDIO!\n"),
                     unit, phy_port));
            return SOC_E_PORT;
        }
    } else {
        if ((SOC_PORT_BLOCK_TYPE(unit, phy_port) == SOC_BLK_XLPORT) ||
            (SOC_PORT_BLOCK_TYPE(unit, phy_port) == SOC_BLK_CLPORT)) {
            /* for CL/XL port block */
            *blk = SOC_PORT_BLOCK(unit, phy_port);
        } else {
            /* Unexpected port for SBus-MDIO */
            LOG_WARN(BSL_LS_SOC_MII, (BSL_META_U(unit,
                     "unit(%d),pport(%d) is unable for TSC SBus-MDIO!\n"),
                     unit, phy_port));
            return SOC_E_PORT;
        }
    }

    return SOC_E_NONE;
}

/*
 * Func : _soc_greyhound2_tscx_reg_read
 *  - GH2 function to serve SBus MDIO read on TSCx
 */
STATIC int
_soc_greyhound2_tscx_reg_read(int unit, uint32 phy_addr,
                            uint32 phy_reg, uint32 *phy_data)
{
    int rv, blk, port;
    int phy_port = _soc_greyhound2_mdio_addr_to_port(phy_addr);

    if (phy_port == INVALID_PPORT_ID) {
        LOG_WARN(BSL_LS_SOC_MII, (BSL_META_U(unit,
                "unit%d: phy_addr=%d is unable for Sbus-MDIO read!\n"),
                unit, phy_addr));
        return SOC_E_PARAM;
    }

    port = SOC_INFO(unit).port_p2l_mapping[phy_port];

    SOC_IF_ERROR_RETURN
        (soc_greyhound2_sbus_tsc_block(unit, phy_port, &blk));

    LOG_INFO(BSL_LS_SOC_MII, (BSL_META_U(unit,
             "_soc_greyhound2_tscx_reg_read[%d]: %d/%d/%d/%d\n"),
             unit, phy_addr, phy_port, port, blk));
    TSC_REG_ADDR_TSCID_SET(phy_reg, phy_addr);
    rv = soc_sbus_tsc_reg_read(unit, port, blk, phy_addr,
                               phy_reg, phy_data);

    return rv;
}

/*
 * Func : _soc_greyhound2_tscx_reg_read
 *  - GH2 function to serve SBus MDIO read on TSCx
 */
STATIC int
_soc_greyhound2_tscx_reg_write(int unit, uint32 phy_addr,
                             uint32 phy_reg, uint32 phy_data)
{
    int rv, blk, port;
    int phy_port = _soc_greyhound2_mdio_addr_to_port(phy_addr);

    if (phy_port == INVALID_PPORT_ID) {
        LOG_WARN(BSL_LS_SOC_MII, (BSL_META_U(unit,
                "unit%d: phy_addr=%d is unable for Sbus-MDIO write!\n"),
                unit, phy_addr));
        return SOC_E_PARAM;
    }

    port = SOC_INFO(unit).port_p2l_mapping[phy_port];

    SOC_IF_ERROR_RETURN
        (soc_greyhound2_sbus_tsc_block(unit, phy_port, &blk));

    LOG_INFO(BSL_LS_SOC_MII, (BSL_META_U(unit,
             "_soc_greyhound2_tscx_reg_write[%d]: %d/%d/%d/%d\n"),
             unit, phy_addr, phy_port, port, blk));
    TSC_REG_ADDR_TSCID_SET(phy_reg, phy_addr);
    rv = soc_sbus_tsc_reg_write(unit, port, blk, phy_addr,
                                phy_reg, phy_data);

    return rv;
}

/*
 * Func : soc_greyhound2_pgw_encap_field_get
 *  - To write to PGW interface on GX/XL/CL port on indicated field
 */
int
soc_greyhound2_pgw_encap_field_get(int unit,
                                soc_port_t lport,
                                soc_field_t field,
                                uint32 *val)
{
    uint32  reg_val = 0;
    int     pport = SOC_INFO(unit).port_l2p_mapping[lport];

    *val = 0;
    if (SOC_PORT_BLOCK_TYPE(unit, pport) == SOC_BLK_XLPORT) {
        SOC_IF_ERROR_RETURN(READ_PGW_XL_CONFIGr(unit, lport, &reg_val));
        *val = soc_reg_field_get(unit, PGW_XL_CONFIGr, reg_val, field);
    } else if (SOC_PORT_BLOCK_TYPE(unit, pport) == SOC_BLK_CLPORT) {
        SOC_IF_ERROR_RETURN(READ_PGW_CL_CONFIGr(unit, lport, &reg_val));
        *val = soc_reg_field_get(unit, PGW_CL_CONFIGr, reg_val, field);
    } else {
        SOC_IF_ERROR_RETURN(READ_PGW_GE_CONFIGr(unit, lport, &reg_val));
        *val = soc_reg_field_get(unit, PGW_GE_CONFIGr, reg_val, field);

    }

    return SOC_E_NONE;
}

#define _GH2_PORTS_PER_CLBLK             4

STATIC int
_soc_greyhound2_misc_init(int unit)
{
#define NUM_SUBPORT 4 /* number of subport of a XL or CL port */
    static const soc_field_t port_field[] = {
        PORT0f, PORT1f, PORT2f, PORT3f
    };
    uint32              rval, l2_ovf_enable, l2_tbl_size;
    uint64              reg64;
    int                 port;
    uint32 entry[SOC_MAX_MEM_WORDS];
    soc_field_t         fields[3];
    uint32              values[3];
    int blk, bindex, mode;
    int phy_port_base;
    soc_info_t *si = &SOC_INFO(unit);
    int delay;
    int freq, target_freq, divisor, dividend;
    int index, count, sub_sel, offset;
    static int rtag7_field_width[] = { 16, 16, 4, 16, 8, 8, 16, 16 };
    int port_count = 0;
    int i = 0;

    if (!SOC_IS_RELOADING(unit)) {
        /* Clear IPIPE/EIPIE Memories */
        SOC_IF_ERROR_RETURN(soc_greyhound2_pipe_mem_clear(unit));

        /* CHECKME : GH2 needs the same programming as HR3 */
        /* _soc_hurricane3_gport_tdm_mode_init(unit) */

        /* Clear MIB counter */
        SOC_IF_ERROR_RETURN(soc_greyhound2_mib_reset(unit));
    }

    SOC_IF_ERROR_RETURN(soc_greyhound2_init_port_mapping(unit));

    /* Enable L2 overflow bucket */
    l2_tbl_size = soc_property_get(unit, spn_L2_TABLE_SIZE, 0);
    if (l2_tbl_size == (soc_mem_index_count(unit, L2Xm) +
                        soc_mem_index_count(unit, L2_ENTRY_OVERFLOWm))) {
        /*
         * if l2 table size equals to l2 table size + l2 overflow table size,
         * it indicates the l2 overflow table is enabled.
         */
        l2_ovf_enable = 1;
    } else {
        l2_ovf_enable = 0;
    }
    SOC_IF_ERROR_RETURN(READ_L2_LEARN_CONTROLr(unit, &rval));
    soc_reg_field_set(unit, L2_LEARN_CONTROLr, &rval,
                      OVERFLOW_BUCKET_ENABLEf, l2_ovf_enable);
    SOC_IF_ERROR_RETURN(WRITE_L2_LEARN_CONTROLr(unit, rval));
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->l2_overflow_bucket_enable = l2_ovf_enable;
    SOC_CONTROL_UNLOCK(unit);

    /* Enable L2 overflow interrupt */
    if (soc_property_get(unit, spn_L2_OVERFLOW_EVENT, FALSE)) {

        SOC_CONTROL_LOCK(unit);
        SOC_CONTROL(unit)->l2_overflow_enable = TRUE;
        SOC_CONTROL_UNLOCK(unit);
        SOC_IF_ERROR_RETURN(soc_hr3_l2_overflow_start(unit));
    } else {
        SOC_CONTROL_LOCK(unit);
        SOC_CONTROL(unit)->l2_overflow_enable = FALSE;
        SOC_CONTROL_UNLOCK(unit);
        SOC_IF_ERROR_RETURN(soc_hr3_l2_overflow_stop(unit));
    }

    /* If parity is not enabled, enable the l2 overflow interrupt */
    if (!soc_property_get(unit, spn_PARITY_ENABLE, TRUE)) {
        /* Write CMIC enable register */
        (void)soc_cmicm_intr3_enable(unit,
                    CMIC_PARITY_IP1_TO_CMIC_PERR_INTR_BIT);
    }

    /* GMAC init  */
    SOC_IF_ERROR_RETURN(soc_greyhound2_gmac_init(unit));

    /* HG init */
    SOC_IF_ERROR_RETURN(soc_greyhound2_higig_mode_init(unit));

    /* XLPORT and CLPORT blocks init */
    SOC_BLOCK_ITER(unit, blk, SOC_BLK_XLPORT) {
        port = SOC_BLOCK_PORT(unit, blk);
        if (port == -1 || port == REG_PORT_ANY) {
            continue;
        }
        phy_port_base = si->port_l2p_mapping[port];

        /* Assert XLPORT soft reset */
        port_count = -1;
        rval = 0;
        for (bindex = 0; bindex < NUM_SUBPORT; bindex++) {
            if (si->port_p2l_mapping[phy_port_base + bindex] != -1) {
                soc_reg_field_set(unit, XLPORT_SOFT_RESETr, &rval,
                                  port_field[bindex], 1);
            }
        }
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_SOFT_RESETr(unit, port, rval));

        for (i = 0; i < _GH2_MAX_TSC_COUNT ; i++) {
            if (_gh2_tsc[i].phy_port_base == phy_port_base) {
                port_count = _gh2_tsc[i].port_count;
                break;
            }
        }
        if (port_count == SOC_GH2_PORT_RATIO_QUAD) {
            mode = SOC_GH2_PORT_MODE_QUAD;
            si->port_num_lanes[port] = 1;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 1;
            si->port_num_lanes[port + 3] = 1;
        } else if ((port_count == SOC_GH2_PORT_RATIO_SINGLE) ||
            (port_count == SOC_GH2_PORT_RATIO_SINGLE_XAUI)) {
            mode = SOC_GH2_PORT_MODE_SINGLE;
            si->port_num_lanes[port] = 4;
        } else if (port_count == SOC_GH2_PORT_RATIO_DUAL_2_2) {
            mode = SOC_GH2_PORT_MODE_DUAL;
            si->port_num_lanes[port] = 2;
            si->port_num_lanes[port + 1] = 2;
        } else if (port_count == SOC_GH2_PORT_RATIO_TRI_012) {
            mode = SOC_GH2_PORT_MODE_TRI_012;
            si->port_num_lanes[port] = 1;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 2;
        } else if (port_count == SOC_GH2_PORT_RATIO_TRI_023) {
            mode = SOC_GH2_PORT_MODE_TRI_023;
            si->port_num_lanes[port] = 2;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 1;
        } else {
            mode = SOC_GH2_PORT_MODE_QUAD;
            si->port_num_lanes[port] = 1;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 1;
            si->port_num_lanes[port + 3] = 1;
        }

        rval = 0;
        soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval,
                          XPORT0_CORE_PORT_MODEf, mode);
        soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval,
                          XPORT0_PHY_PORT_MODEf, mode);
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_MODE_REGr(unit, port, rval));

        /* De-assert XLPORT soft reset */
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_SOFT_RESETr(unit, port, 0));

        /* Enable XLPORT */
        rval = 0;
        for (bindex = 0; bindex < NUM_SUBPORT; bindex++) {
            if (si->port_p2l_mapping[phy_port_base + bindex] != -1) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval,
                                  port_field[bindex], 1);
            }
        }
        SOC_IF_ERROR_RETURN(WRITE_XLPORT_ENABLE_REGr(unit, port, rval));
    }

    SOC_BLOCK_ITER(unit, blk, SOC_BLK_CLPORT) {
        port = SOC_BLOCK_PORT(unit, blk);
        if (port == -1 || port == REG_PORT_ANY) {
            continue;
        }
        phy_port_base = si->port_l2p_mapping[port];

        /* Assert XLPORT soft reset */
        port_count = -1;
        rval = 0;
        for (bindex = 0; bindex < NUM_SUBPORT; bindex++) {
            if (si->port_p2l_mapping[phy_port_base + bindex] != -1) {
                soc_reg_field_set(unit, CLPORT_SOFT_RESETr, &rval,
                                  port_field[bindex], 1);
            }
        }
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_SOFT_RESETr(unit, port, rval));

        for (i = 0; i < _GH2_MAX_TSC_COUNT ; i++) {
            if (_gh2_tsc[i].phy_port_base == phy_port_base) {
                port_count = _gh2_tsc[i].port_count;
                break;
            }
        }
        if (port_count == SOC_GH2_PORT_RATIO_QUAD) {
            mode = SOC_GH2_PORT_MODE_QUAD;
            si->port_num_lanes[port] = 1;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 1;
            si->port_num_lanes[port + 3] = 1;
        } else if ((port_count == SOC_GH2_PORT_RATIO_SINGLE) ||
            (port_count == SOC_GH2_PORT_RATIO_SINGLE_XAUI)) {
            mode = SOC_GH2_PORT_MODE_SINGLE;
            si->port_num_lanes[port] = 4;
        } else if (port_count == SOC_GH2_PORT_RATIO_DUAL_2_2) {
            mode = SOC_GH2_PORT_MODE_DUAL;
            si->port_num_lanes[port] = 2;
            si->port_num_lanes[port + 1] = 2;
        } else if (port_count == SOC_GH2_PORT_RATIO_TRI_012) {
            mode = SOC_GH2_PORT_MODE_TRI_012;
            si->port_num_lanes[port] = 1;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 2;
        } else if (port_count == SOC_GH2_PORT_RATIO_TRI_023) {
            mode = SOC_GH2_PORT_MODE_TRI_023;
            si->port_num_lanes[port] = 2;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 1;
        } else {
            mode = SOC_GH2_PORT_MODE_QUAD;
            si->port_num_lanes[port] = 1;
            si->port_num_lanes[port + 1] = 1;
            si->port_num_lanes[port + 2] = 1;
            si->port_num_lanes[port + 3] = 1;
        }

        rval = 0;
        soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval,
                          XPORT0_CORE_PORT_MODEf, mode);
        soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval,
                          XPORT0_PHY_PORT_MODEf, mode);
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_MODE_REGr(unit, port, rval));

        /* De-assert XLPORT soft reset */
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_SOFT_RESETr(unit, port, 0));

        /* Enable XLPORT */
        rval = 0;
        for (bindex = 0; bindex < NUM_SUBPORT; bindex++) {
            if (si->port_p2l_mapping[phy_port_base + bindex] != -1) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval,
                                  port_field[bindex], 1);
            }
        }
        SOC_IF_ERROR_RETURN(WRITE_CLPORT_ENABLE_REGr(unit, port, rval));
    }

    SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &rval));
    soc_reg_field_set(unit, MISCCONFIGr, &rval, METERING_CLK_ENf, 1);
    SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, rval));

    /* Enable dual hash on L2 and L3 tables */
    fields[0] = ENABLEf;
    values[0] = 1;
    fields[1] = HASH_SELECTf;
    values[1] = FB_HASH_CRC32_LOWER;
    fields[2] = INSERT_LEAST_FULL_HALFf;
    values[2] = 1;
    SOC_IF_ERROR_RETURN
        (soc_reg_fields32_modify(unit, L2_AUX_HASH_CONTROLr, REG_PORT_ANY, 3,
                                 fields, values));
    SOC_IF_ERROR_RETURN
        (soc_reg_fields32_modify(unit, L3_AUX_HASH_CONTROLr, REG_PORT_ANY, 3,
                                 fields, values));

    /*
     * Egress Enable
     */
    sal_memset(entry, 0, sizeof(egr_enable_entry_t));
    soc_mem_field32_set(unit, EGR_ENABLEm, entry, PRT_ENABLEf, 1);
    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN
            (WRITE_EGR_ENABLEm(unit, MEM_BLOCK_ALL,
                               SOC_INFO(unit).port_l2p_mapping[port], entry));
    }

    COMPILER_64_ZERO(reg64);
    soc_reg64_field32_set(unit, EPC_LINK_BMAP_LO_64r, &reg64, PORT_BITMAP_LOf,
                         SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0));
    SOC_IF_ERROR_RETURN(WRITE_EPC_LINK_BMAP_LO_64r(unit, reg64));
    SOC_IF_ERROR_RETURN(READ_ING_CONFIG_64r(unit, &reg64));
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          L3SRC_HIT_ENABLEf, 1);
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          L2DST_HIT_ENABLEf, 1);
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          APPLY_EGR_MASK_ON_L2f, 1);
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          APPLY_EGR_MASK_ON_L3f, 1);
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          ARP_RARP_TO_FPf, 0x3); /* enable both ARP & RARP */
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          ARP_VALIDATION_ENf, 1);
    soc_reg64_field32_set(unit, ING_CONFIG_64r, &reg64,
                          IGNORE_HG_HDR_LAG_FAILOVERf, 1);
    SOC_IF_ERROR_RETURN(WRITE_ING_CONFIG_64r(unit, reg64));

    SOC_IF_ERROR_RETURN(READ_EGR_CONFIG_1r(unit, &rval));
    soc_reg_field_set(unit, EGR_CONFIG_1r, &rval, RING_MODEf, 1);
    SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIG_1r(unit, rval));

    /* The HW defaults for EGR_VLAN_CONTROL_1.VT_MISS_UNTAG == 1, which
     * causes the outer tag to be removed from packets that don't have
     * a hit in the egress vlan tranlation table. Set to 0 to disable this.
     */
    rval = 0;
    soc_reg_field_set(unit, EGR_VLAN_CONTROL_1r, &rval, VT_MISS_UNTAGf, 0);

    /* Enable pri/cfi remarking on egress ports. */
    soc_reg_field_set(unit, EGR_VLAN_CONTROL_1r, &rval, REMARK_OUTER_DOT1Pf,
                      1);
    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_EGR_VLAN_CONTROL_1r(unit, port, rval));
    }

    /* Multicast range initialization */
    SOC_IF_ERROR_RETURN
        (soc_hbx_higig2_mcast_sizes_set(unit,
             soc_property_get(unit, spn_HIGIG2_MULTICAST_VLAN_RANGE,
                              SOC_HBX_MULTICAST_RANGE_DEFAULT),
             soc_property_get(unit, spn_HIGIG2_MULTICAST_L2_RANGE,
                              SOC_HBX_MULTICAST_RANGE_DEFAULT),
             soc_property_get(unit, spn_HIGIG2_MULTICAST_L3_RANGE,
                              SOC_HBX_MULTICAST_RANGE_DEFAULT)));

    /* Enable vrf based l3 lookup by default. */
    SOC_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, VRF_MASKr, REG_PORT_ANY, MASKf, 0));

    /* Setup SW2_FP_DST_ACTION_CONTROL */
    fields[0] = HGTRUNK_RES_ENf;
    fields[1] = LAG_RES_ENf;
    values[0] = values[1] = 1;
    SOC_IF_ERROR_RETURN
        (soc_reg_fields32_modify(unit, SW2_FP_DST_ACTION_CONTROLr,
                                 REG_PORT_ANY, 2, fields, values));

    /* Populate and enable RTAG7 Macro flow offset table */
    if (soc_mem_is_valid(unit, RTAG7_FLOW_BASED_HASHm)) {
        count = soc_mem_index_max(unit, RTAG7_FLOW_BASED_HASHm);
        sal_memset(entry, 0, sizeof(rtag7_flow_based_hash_entry_t));
        for (index = 0; index < count; ) {
            for (sub_sel = 0; sub_sel < 8 && index < count; sub_sel++) {
                for (offset = 0;
                     offset < rtag7_field_width[sub_sel] && index < count;
                     offset++) {
                    soc_mem_field32_set(unit, RTAG7_FLOW_BASED_HASHm, &entry,
                                        SUB_SEL_ECMPf, sub_sel);
                    soc_mem_field32_set(unit, RTAG7_FLOW_BASED_HASHm, &entry,
                                        OFFSET_ECMPf, offset);
                    SOC_IF_ERROR_RETURN
                        (soc_mem_write(unit, RTAG7_FLOW_BASED_HASHm,
                                       MEM_BLOCK_ANY, index, &entry));
                    index++;
                }
            }
        }
        rval = 0;
        soc_reg_field_set(unit, RTAG7_HASH_SELr, &rval, USE_FLOW_SEL_ECMPf, 1);
        SOC_IF_ERROR_RETURN(WRITE_RTAG7_HASH_SELr(unit, rval));
    }

    freq = si->frequency;

    /*
     * Set external MDIO freq to around 10MHz
     * target_freq = core_clock_freq * DIVIDEND / DIVISOR / 2
     */
    target_freq = 10;
    divisor = (freq + (target_freq * 2 - 1)) / (target_freq * 2);
    divisor = soc_property_get(unit, spn_RATE_EXT_MDIO_DIVISOR, divisor);
    dividend = soc_property_get(unit, spn_RATE_EXT_MDIO_DIVIDEND, 1);

    rval = 0;
    soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &rval, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUSTr, &rval, DIVIDENDf, dividend);
    SOC_IF_ERROR_RETURN(WRITE_CMIC_RATE_ADJUSTr(unit, rval));

    /*
     * Set internal MDIO freq to around 10MHz
     * Valid range is from 2.5MHz to 12.5MHz
     * target_freq = core_clock_freq * DIVIDEND / DIVISOR / 2
     * or
     * DIVISOR = core_clock_freq * DIVIDENT / (target_freq * 2)
     */
    target_freq = 10;
    divisor = (freq + (target_freq * 2 - 1)) / (target_freq * 2);
    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, divisor);
    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, 1);
    rval = 0;
    soc_reg_field_set (unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVISORf,
                       divisor);
    soc_reg_field_set (unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVIDENDf,
                       dividend);
    /* coverity[result_independent_of_operands] */
    SOC_IF_ERROR_RETURN(WRITE_CMIC_RATE_ADJUST_INT_MDIOr(unit, rval));

    delay = soc_property_get(unit, spn_MDIO_OUTPUT_DELAY, -1);
    if (delay >= 1  && delay <= 15) {
        SOC_IF_ERROR_RETURN(READ_CMIC_MIIM_CONFIGr(unit, &rval));
        soc_reg_field_set(unit, CMIC_MIIM_CONFIGr, &rval, MDIO_OUT_DELAYf,
                          delay);
        SOC_IF_ERROR_RETURN(WRITE_CMIC_MIIM_CONFIGr(unit, rval));
    }

    /* Directed Mirroring ON by default */
    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(READ_EGR_PORT_64r(unit, port, &reg64));
        soc_reg64_field32_set(unit, EGR_PORT_64r,
                              &reg64, EM_SRCMOD_CHANGEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_EGR_PORT_64r(unit, port, reg64));
        SOC_IF_ERROR_RETURN(READ_IEGR_PORT_64r(unit, port, &reg64));
        soc_reg64_field32_set(unit, IEGR_PORT_64r,
                              &reg64, EM_SRCMOD_CHANGEf, 1);
        SOC_IF_ERROR_RETURN(WRITE_IEGR_PORT_64r(unit, port, reg64));
    }

    /* Attach serdes firmware set function */
    _phy_tsce_firmware_set_helper[unit] = _soc_greyhound2_tscx_firmware_set;
    _phy_tscf_firmware_set_helper[unit] = _soc_greyhound2_tscx_firmware_set;
    _phy_qtce_firmware_set_helper[unit] = _soc_greyhound2_tscx_firmware_set;

#ifdef INCLUDE_AVS
    soc_hr3_avs_init(unit);
#endif /* INCLUDE_AVS  */

    /* Major for L3_IIFm.TRUST_DSCP_PTRf init */
    SOC_IF_ERROR_RETURN(_soc_hr3_l3iif_hw_mem_init(unit, TRUE));

    /* Remove SR/PRP loopback port from user view ports */
    SOC_PBMP_PORT_REMOVE(si->ge.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->xe.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->ether.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->hg.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->st.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->gx.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->xl.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->cl.bitmap, si->lb_port);
    SOC_PBMP_PORT_REMOVE(si->port.bitmap, si->lb_port);

    return SOC_E_NONE;
}



/* soc_greyhound2_mem_config:
 * Over-ride the default table sizes (from regsfile) for any SKUs here
 */
int
soc_greyhound2_mem_config(int unit, int dev_id)
{
    int rv = SOC_E_NONE;
    soc_persist_t *sop = SOC_PERSIST(unit);

    SOC_CONTROL(unit)->l3_defip_max_tcams = 2;
    SOC_CONTROL(unit)->l3_defip_tcam_size = 512; /* or 64 per OTP */

    if (SAL_BOOT_QUICKTURN) {
        /* QuickTurn with limited TCAM entries */
        /* To be defined */
        sop->memState[L2_USER_ENTRYm].index_max = 11;
        sop->memState[L2_USER_ENTRY_ONLYm].index_max = 11;
        sop->memState[MY_STATION_TCAMm].index_max = 63;
        sop->memState[VFP_TCAMm].index_max = 63;
        sop->memState[EFP_TCAMm].index_max = 63;
        sop->memState[FP_TCAMm].index_max = 63;
        sop->memState[VLAN_SUBNETm].index_max = 11;
        sop->memState[VLAN_SUBNET_ONLYm].index_max = 11;
        sop->memState[FP_GLOBAL_MASK_TCAMm].index_max = 63;
        sop->memState[CPU_COS_MAPm].index_max = 11;
        sop->memState[CPU_COS_MAP_ONLYm].index_max = 11;
        sop->memState[L3_DEFIPm].index_max = 63;
        sop->memState[L3_DEFIP_ONLYm].index_max = 63;
    }
    return rv;
}

/*
 * Function:
 *  _soc_greyhound2_gpio_set
 * Purpose:
 *  Write value to the specified GPIO pin
 */
STATIC int
_soc_greyhound2_gpio_set(int unit, int pin, int output, int val)
{
    uint32 rval = 0;
    uint32 fval = 0;
    uint8 mask = 0xFF;

    mask &= ~(1 << pin);

    /* coverity[result_independent_of_operands] */
    SOC_IF_ERROR_RETURN(READ_CMIC_GP_OUT_ENr(unit, &rval));
    fval = soc_reg_field_get(unit, CMIC_GP_OUT_ENr, rval, OUT_ENABLEf);
    if (output) {
        fval |= 1 << pin;
    } else {
        fval &= ~(1 << pin);
    }
    soc_reg_field_set(unit, CMIC_GP_OUT_ENr, &rval, OUT_ENABLEf, fval);
    /* coverity[result_independent_of_operands] */
    SOC_IF_ERROR_RETURN(WRITE_CMIC_GP_OUT_ENr(unit, rval));

    if (output) {
        /* coverity[result_independent_of_operands] */
        SOC_IF_ERROR_RETURN(READ_CMIC_GP_DATA_OUTr(unit, &rval));
        fval = soc_reg_field_get(unit, CMIC_GP_DATA_OUTr, rval, DATA_OUTf);
        if (val) {
            fval |= (val << pin);
        } else {
            fval &= ~(1 << pin);
        }

        soc_reg_field_set(unit, CMIC_GP_DATA_OUTr, &rval, DATA_OUTf, fval);
        /* coverity[result_independent_of_operands] */
        SOC_IF_ERROR_RETURN(WRITE_CMIC_GP_DATA_OUTr(unit, rval));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *  soc_greyhound2_serdes_disabled
 * Purpose:
 *  Get the disabled serdes core from strap status register
 */
STATIC int
soc_greyhound2_serdes_disabled(int unit,
                               uint32 *disabled_tsc,
                               uint32 *disabled_qtc)
{
    uint32 strap_sts_1 = 0;
    int power_off = 0;
    int i = 0;
    soc_field_t qtc_fields[] = {
        STRAP_TSCQ0_PWR_OFFf,
        STRAP_TSCQ1_PWR_OFFf,
        INVALIDf
    };
    soc_field_t tsc_fields[] = {
        STRAP_TSCE0_PWR_OFFf,
        STRAP_TSCE1_PWR_OFFf,
        STRAP_TSCE2_PWR_OFFf,
        STRAP_TSCE3_PWR_OFFf,
        STRAP_TSCE4_PWR_OFFf,
        STRAP_TSCE5_PWR_OFFf,
        STRAP_TSCE6_PWR_OFFf,
        STRAP_TSCF0_PWR_OFFf,
        INVALIDf
    };

    /* parameter validation */
    if ((disabled_tsc == NULL) || (disabled_qtc == NULL)) {
        return SOC_E_PARAM;
    }

    /*
     * For simulation, starp status register is empty.
     * Return all QTC and TSC not disabled.
     */
    if  (SAL_BOOT_SIMULATION) {
        *disabled_tsc = 0;
        *disabled_qtc = 0;
        return SOC_E_NONE;
    } else {
        /* read OTP information */
        /* qtc */
        for (i = 0; i < 2; i++) {
            if (SHR_BITGET(SOC_BOND_INFO(unit)->tsc_disabled, i)) {
                *disabled_qtc |= (1 << i);
            }
        }
        /* tsc */
        for (i = 0; i < 8; i++) {
            if (SHR_BITGET(SOC_BOND_INFO(unit)->tsc_disabled, 2 + i)) {
                *disabled_tsc |= (1 << i);
            }
        }
    }

    SOC_IF_ERROR_RETURN(READ_TOP_STRAP_STATUS_1r(unit, &strap_sts_1));

    /* Get the disabled QTC cores */
    i = 0;
    while (qtc_fields[i] != INVALIDf) {
        power_off = soc_reg_field_get(unit, TOP_STRAP_STATUS_1r,
                                      strap_sts_1, qtc_fields[i]);
        if (power_off) {
            *disabled_qtc |= (1 << i);
        }
        i++;
    };

    /* Get the disabled TSC cores */
    i = 0;
    while (tsc_fields[i] != INVALIDf) {
        power_off = soc_reg_field_get(unit, TOP_STRAP_STATUS_1r,
                                      strap_sts_1, tsc_fields[i]);
        if (power_off) {
            *disabled_tsc |= (1 << i);
        }
        i++;
    };

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"%s: disabled QTC(0x%x), "
                            "disabled TSC(0x%x)\n"),
                 FUNCTION_NAME(), *disabled_qtc, *disabled_tsc));

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_greyhound2_chip_reset
 * Purpose:
 *      Special reset sequencing for BCM53570
 */
int
soc_greyhound2_chip_reset(int unit)
{
    uint32 rval, to_usec;
    _gh2_sku_info_t *matched_sku_info;
    uint32 disabled_bmp;
    uint32 disabled_tsc = 0;
    uint32 disabled_qtc = 0;
    int i;

    to_usec = SAL_BOOT_QUICKTURN ? (250 * MILLISECOND_USEC) :
                                   (10 * MILLISECOND_USEC);

    WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x11110100); /* block 7  - 0 */
    WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x00430000); /* block 15 - 8 */
    WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x00005064); /* block 23 - 16 */
    WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x02220111); /* block 31 - 24 */
    WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x02220222); /* block 39 - 32 */
    WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x22222222); /* block 40 - 47 */
    WRITE_CMIC_SBUS_TIMEOUTr(unit, 0x7d0);

    sal_usleep(to_usec);



    /* 250Mhz TS PLL implies 4ns resolution */
    /* clock period in nanoseconds */
    SOC_TIMESYNC_PLL_CLOCK_NS(unit) = (1/250 * 1000);

    /*
     * Select LCPLL0 as external PHY reference clock and output 125MHz
     */
    /* Pull GPIO3 low to reset the ext. PHY */
    SOC_IF_ERROR_RETURN(_soc_greyhound2_gpio_set(unit, 3, TRUE, 0));
    SOC_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, TOP_XG_PLL0_CTRL_0r,
                                REG_PORT_ANY, CH5_MDIVf, 0x19));
    SOC_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, TOP_XG_PLL0_CTRL_6r,
                                REG_PORT_ANY, MSC_CTRLf, 0x71a2));
    /* Pull  GPIO high to leave the reset state */
    SOC_IF_ERROR_RETURN(_soc_greyhound2_gpio_set(unit, 3, TRUE, 1));

    /*
     * TSCx or QTCx to be disabled in some sku.
     */
    /* To be reviewed on chip */
    if (matched_devid_idx == -1) {
        LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META_U(unit, "Warning: soc_greyhound2_port_config_init should "
                        "be invoked first! Choose bcm56170 port config.\n")));
        matched_devid_idx = 0;
    }

    matched_sku_info = &_gh2_sku_port_config[matched_devid_idx];
    disabled_tsc = 0;
    disabled_qtc = 0;
    SOC_IF_ERROR_RETURN(
        soc_greyhound2_serdes_disabled(unit,
                                       &disabled_tsc, &disabled_qtc));
    disabled_bmp = matched_sku_info->disabled_tsc_bmp;
    for (i = 0; i < _GH2_MAX_TSC_COUNT; i++) {
        if ((1 << i) & disabled_bmp) {
            disabled_tsc |= (1 << i);
        }
    }

    disabled_bmp = matched_sku_info->disabled_qtc_bmp;
    for (i = 0; i < _GH2_MAX_QTC_COUNT; i++) {
        if ((1 << i) & disabled_bmp) {
            disabled_qtc |= (1 << i);
        }
    }

    SOC_IF_ERROR_RETURN(READ_PGW_CTRL_0r(unit, &rval));
    soc_reg_field_set(unit, PGW_CTRL_0r, &rval, SW_PM4X10_DISABLEf,
                      disabled_tsc & 0x7f);
    soc_reg_field_set(unit, PGW_CTRL_0r, &rval, SW_QTC_DISABLEf,
                      disabled_qtc);
    soc_reg_field_set(unit, PGW_CTRL_0r, &rval, SW_PM4X25_DISABLEf,
                      (disabled_tsc >> 7) & 0x1);
    SOC_IF_ERROR_RETURN(WRITE_PGW_CTRL_0r(unit, rval));
    sal_usleep(to_usec);

    /*
     * Bring port blocks out of reset
     */
    SOC_IF_ERROR_RETURN(READ_TOP_SOFT_RESET_REGr(unit, &rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_GE8P_RST_Lf, 0x7);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_GEP0_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_GEP1_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP0_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP1_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP2_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP3_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP4_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP5_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_XLP6_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_CLP0_RST_Lf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REGr(unit, rval));
    sal_usleep(to_usec);

    /* Bring network sync out of reset */
    SOC_IF_ERROR_RETURN(READ_TOP_SOFT_RESET_REGr(unit, &rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_TS_RST_Lf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REGr(unit, rval));
    sal_usleep(to_usec);

    SOC_IF_ERROR_RETURN(READ_TOP_SOFT_RESET_REG_2r(unit, &rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REG_2r, &rval, TOP_TS_PLL_RST_Lf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REG_2r(unit, rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REG_2r,
                      &rval, TOP_TS_PLL_POST_RST_Lf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REG_2r(unit, rval));

    /* Bring IP, EP, and MMU blocks out of reset */
    SOC_IF_ERROR_RETURN(READ_TOP_SOFT_RESET_REGr(unit, &rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_EP_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_IP_RST_Lf, 1);
    soc_reg_field_set(unit, TOP_SOFT_RESET_REGr, &rval, TOP_MMU_RST_Lf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REGr(unit, rval));
    sal_usleep(to_usec);

    /* speed limit settings for some SKUs option */

    return SOC_E_NONE;
}


STATIC int
_soc_greyhound2_age_timer_get(int unit, int *age_seconds, int *enabled)
{
    uint32 value;

    SOC_IF_ERROR_RETURN(READ_L2_AGE_TIMERr(unit, &value));
    *enabled = soc_reg_field_get(unit, L2_AGE_TIMERr, value, AGE_ENAf);
    *age_seconds = soc_reg_field_get(unit, L2_AGE_TIMERr, value, AGE_VALf);

    return SOC_E_NONE;
}

STATIC int
_soc_greyhound2_age_timer_max_get(int unit, int *max_seconds)
{
    *max_seconds =
        soc_reg_field_get(unit, L2_AGE_TIMERr, 0xffffffff, AGE_VALf);

    return SOC_E_NONE;
}

STATIC int
_soc_greyhound2_age_timer_set(int unit, int age_seconds, int enable)
{
    uint32 value;

    value = 0;
    soc_reg_field_set(unit, L2_AGE_TIMERr, &value, AGE_ENAf, enable);
    soc_reg_field_set(unit, L2_AGE_TIMERr, &value, AGE_VALf, age_seconds);
    SOC_IF_ERROR_RETURN(WRITE_L2_AGE_TIMERr(unit, value));

    return SOC_E_NONE;
}

static const soc_reg_t pvtmon_result_reg[] = {
    TOP_PVTMON_RESULT_0r
};

int
soc_gh2_temperature_monitor_get(int unit,
          int temperature_max,
          soc_switch_temperature_monitor_t *temperature_array,
          int *temperature_count)
{
    soc_reg_t reg;
    int index;
    uint32 rval;
    int fval, cur, peak;
    int num_entries_out;

    *temperature_count = 0;
    if (COUNTOF(pvtmon_result_reg) > temperature_max) {
        num_entries_out = temperature_max;
    } else {
        num_entries_out = COUNTOF(pvtmon_result_reg);
    }

    for (index = 0; index < num_entries_out; index++) {
        reg = pvtmon_result_reg[index];
        SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &rval));

        fval = soc_reg_field_get(unit, reg, rval, TEMP_DATAf);
        cur = (41004000 - (48705 * fval)) / 100000;
        fval = soc_reg_field_get(unit, reg, rval, PEAK_TEMP_DATAf);
        peak = (41004000 - (48705 * fval)) / 100000;
        (temperature_array + index)->curr = cur;
        (temperature_array + index)->peak = peak;
    }
    SOC_IF_ERROR_RETURN(READ_TOP_SOFT_RESET_REG_2r(unit, &rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REG_2r, &rval,
                      TOP_TEMP_MON_PEAK_RST_Lf, 0);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REG_2r(unit, rval));
    soc_reg_field_set(unit, TOP_SOFT_RESET_REG_2r, &rval,
                      TOP_TEMP_MON_PEAK_RST_Lf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REG_2r(unit, rval));

    *temperature_count = num_entries_out;
    return SOC_E_NONE;
}

typedef struct _gh2_pgw_reg_info_s {
    soc_reg_t   pgw_reg;
    uint32      valid_bm;
    uint32      invalid_bm;
    uint32      invalid_blk_num_bm;
}_gh2_pgw_reg_info_t;

STATIC _gh2_pgw_reg_info_t _gh2_pgw_reg_info[] = {
    {PGW_GX_CONFIGr, 0x3c, 0, 0x1f},
    {PGW_GX_TXFIFO_CTRLr, 0x3c, 0, 0x1f},
    {PGW_GX_SPARE0_REGr, 0x3c, 0, 0x1f},
    {PGW_GX_RXFIFO_SOFT_RESETr, 0, 0, 0x1f},
    {PGW_GX_ECC_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_RXFIFO0_ECC_SBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_RXFIFO0_ECC_DBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_RXFIFO1_ECC_SBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_RXFIFO1_ECC_DBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO0_ECC_SBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO0_ECC_DBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO1_ECC_SBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO1_ECC_DBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO2_ECC_SBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO2_ECC_DBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO3_ECC_SBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO3_ECC_DBE_STATUSr, 0, 0, 0x1f},
    {PGW_GX_TXFIFO_OVERFLOWr, 0, 0, 0x1f},
    {PGW_GX_RXFIFO0_OVERFLOW_ERRORr, 0, 0, 0x1f},
    {PGW_GX_RXFIFO1_OVERFLOW_ERRORr, 0, 0, 0x1f},
    {PGW_GX_TM0_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM1_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM2_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM3_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM4_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM5_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM6_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM7_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_TM8_CONTROLr, 0, 0, 0x1f},
    {PGW_GX_BYPASS_LED_CHAIN_CONFIGr, 0, 0, 0x1f},
    {PGW_GX_INTR_STATUSr, 0, 0, 0x1f},
    {PGW_GX_INTR_ENABLEr, 0, 0, 0x1f},
    {PGW_XL_CONFIGr, 0, 0x3c, 0x20},
    {PGW_XL_TXFIFO_CTRLr, 0, 0x3c, 0x20},
    {PGW_XL_SPARE0_REGr, 0, 0x3c, 0x20},
    {PGW_XL_RXFIFO_SOFT_RESETr, 0, 0, 0x20},
    {PGW_XL_ECC_CONTROLr, 0, 0, 0x20},
    {PGW_XL_RXFIFO_ECC_SBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_RXFIFO_ECC_DBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO0_ECC_SBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO0_ECC_DBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO1_ECC_SBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO1_ECC_DBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO2_ECC_SBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO2_ECC_DBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO3_ECC_SBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO3_ECC_DBE_STATUSr, 0, 0, 0x20},
    {PGW_XL_TXFIFO_OVERFLOWr, 0, 0, 0x20},
    {PGW_XL_RXFIFO_OVERFLOW_ERRORr, 0, 0, 0x20},
    {PGW_XL_TM0_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM1_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM2_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM3_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM4_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM5_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM6_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM7_CONTROLr, 0, 0, 0x20},
    {PGW_XL_TM8_CONTROLr, 0, 0, 0x20},
    {PGW_XL_BYPASS_LED_CHAIN_CONFIGr, 0, 0, 0x20},
    {PGW_XL_INTR_STATUSr, 0, 0, 0x20},
    {PGW_XL_INTR_ENABLEr, 0, 0, 0x20},
    {INVALIDr, 0, 0},
};

/*
 * Function:
 *  soc_greyhound2_pgw_reg_blk_index_get
 * Purpose:
 *  Deal with the blk, blknum for PGW_GX and PGW_XL registers
 * Parameters:
 *  reg (IN): register.
 *  port (IN): logical port if PGW_GX's port register.
 *  bm (IN/OUT): bm should be added for the reg.
 *  block (IN/OUT): blk used to access the reg.
 *  index (OUT): port index of the reg.
 *  invalid_blk_check (IN): perform the blk check.
 *                          skip the block if the return value = 1.
 * Return:
 *   SOC_E_NOT_FOUND : w/o bm,blk,invalid_blk_check case, register not matched.
 *   1 : register matched and the block/index has been override.
 *   1 : register matched and the valid bm has been override.
 *   1 : skip the block when the invalid_blk_check=1
 *   SOC_E_PARAM : invalid paramter
 */
int
soc_greyhound2_pgw_reg_blk_index_get(int unit,
    soc_reg_t reg, soc_port_t port,
    pbmp_t *bm, int *block, int *index, int invalid_blk_check) {

    int i, p, phy_port, blk_num, pgw_blk;
    soc_reg_info_t *reginfo;
    _gh2_pgw_reg_info_t *pgw_reg = NULL;
    pbmp_t pgw_pbm, pgw_invalid_pbm;

    pgw_blk = -1;
    SOC_PBMP_CLEAR(pgw_pbm);

    for (i = 0; _gh2_pgw_reg_info[i].pgw_reg != INVALIDr; i ++) {
        if (reg == _gh2_pgw_reg_info[i].pgw_reg) {
            pgw_reg = &_gh2_pgw_reg_info[i];
            break;
        }
    }

    if (!pgw_reg) {
        return SOC_E_NOT_FOUND;
    }

    reginfo = &SOC_REG_INFO(unit, reg);
    phy_port = 0;
    if (reginfo->regtype == soc_portreg) {
        SOC_PBMP_WORD_SET(pgw_pbm, 0, pgw_reg->valid_bm);
        SOC_PBMP_WORD_SET(pgw_invalid_pbm, 0, pgw_reg->invalid_bm);
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    }

    if (bm != NULL) {
        SOC_PBMP_ITER(pgw_pbm, p) {
            SOC_PBMP_PORT_ADD(*bm, p);
        }
        return 1;
    }
    if (block) {
        if (reginfo->regtype == soc_portreg) {
            if (SOC_PORT_BLOCK_TYPE(unit,phy_port) == SOC_BLK_GXPORT) {
                if (SOC_PBMP_MEMBER(pgw_pbm, port)) {
                    /* get the blk from phy_port=2 bindex=1 i.e. XLPORT5 */
                    pgw_blk = SOC_PORT_IDX_BLOCK(unit, 2, 1);
                    if (index) {
                        *index = phy_port - 2;
                    }
                    if (!invalid_blk_check) {
                        *block = pgw_blk;
                        return 1;
                    }
                }
                if (SOC_PBMP_MEMBER(pgw_invalid_pbm, port)) {
                    *block = 0;
                    return 1;
                }
            }
        }
    }
    if (invalid_blk_check) {
        if (!block) {
            return SOC_E_PARAM;
        }
        /* check if the *block is valid for the reg */

        if (pgw_blk == -1) {
            pgw_blk = *block;
        }
        blk_num = SOC_BLOCK_NUMBER(unit, pgw_blk) & 0xff;
        if (pgw_reg->invalid_blk_num_bm & (1 << blk_num)) {
            return 1;
        }
    }
    return SOC_E_NONE;
}

/*
* Function:
*      soc_gh2_chip_sku_get
* Purpose:
*      Get the chip sku
* Parameters :
*      sku - (OUT)Hurricane3 chip sku(SOC_GH2_SKU_XXX)
*/
STATIC int
soc_gh2_chip_sku_get(int unit, int *sku)
{
    uint16  dev_id;
    uint8    rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    if ((dev_id & 0xfff0) == 0xb170) {
        /* Hurricane3-MG (56170, 56172, 56174) */
        *sku = SOC_GH2_SKU_HURRICANE3MG;
    } else if ((dev_id & 0xfff0) == 0x8570) {
        /* Quartz (53570, 53575) */
        *sku = SOC_GH2_SKU_QUARTZ;
    } else if (dev_id == 0xb070) {
        /* simulation chip id (56070) */
        *sku = SOC_GH2_SKU_EMULATION;
    } else {
        *sku = -1;
        return SOC_E_BADID;
    }

    return SOC_E_NONE;
}

int
_soc_greyhound2_features(int unit, soc_feature_t feature)
{
    switch (feature) {
        case soc_feature_arl_hashed:
        case soc_feature_arl_insert_ovr:
        case soc_feature_cfap_pool:
        case soc_feature_cos_rx_dma:
        case soc_feature_dcb_type37:
        case soc_feature_ingress_metering:
        case soc_feature_egress_metering:
        case soc_feature_l3_lookup_cmd:
        case soc_feature_led_proc:
        case soc_feature_led_data_offset_a0:
        case soc_feature_schmsg_alias:
        case soc_feature_stack_my_modid:
        case soc_feature_stat_dma:
        case soc_feature_cpuport_stat_dma:
        case soc_feature_table_dma:
        case soc_feature_tslam_dma:
        case soc_feature_stg:
        case soc_feature_stg_xgs:
        case soc_feature_remap_ut_prio:
        case soc_feature_xgxs_v7:
        case soc_feature_phy_cl45:
        case soc_feature_aging_extended:
        case soc_feature_modmap:
        case soc_feature_l2_hashed:
        case soc_feature_l2_lookup_cmd:
        case soc_feature_l2_lookup_retry:
        case soc_feature_l2_user_table:
        case soc_feature_schan_hw_timeout:
        case soc_feature_mdio_enhanced:
        case soc_feature_dodeca_serdes:
        case soc_feature_rxdma_cleanup:
        case soc_feature_fe_maxframe:
        case soc_feature_l2x_parity:
        case soc_feature_l3x_parity:
        case soc_feature_l2_modfifo:
        case soc_feature_vlan_mc_flood_ctrl:
        case soc_feature_vlan_translation:
        case soc_feature_parity_err_tocpu:
        case soc_feature_nip_l3_err_tocpu:
        case soc_feature_l3mtu_fail_tocpu:
        case soc_feature_meter_adjust:
        case soc_feature_xgxs_power:
        case soc_feature_src_modid_blk:
        case soc_feature_src_modid_blk_ucast_override:
        case soc_feature_src_modid_blk_opcode_override:
        case soc_feature_egress_blk_ucast_override:
        case soc_feature_stat_jumbo_adj:
        case soc_feature_stat_xgs3:
        case soc_feature_port_flow_hash:
        case soc_feature_cpuport_switched:
        case soc_feature_cpuport_mirror:
        case soc_feature_higig2:
        case soc_feature_color:
        case soc_feature_color_inner_cfi:
        case soc_feature_color_prio_map:
        case soc_feature_untagged_vt_miss:
        case soc_feature_module_loopback:
        case soc_feature_dscp_map_per_port:
        case soc_feature_egr_dscp_map_per_port:
        case soc_feature_dscp_map_mode_all:
        case soc_feature_higig_lookup:
        case soc_feature_egr_mirror_path:
        case soc_feature_trunk_extended:
        case soc_feature_hg_trunking:
        case soc_feature_hg_trunk_override:
        case soc_feature_egr_vlan_check:
        case soc_feature_cpu_proto_prio:
        case soc_feature_hg_trunk_failover:
        case soc_feature_trunk_egress:
        case soc_feature_force_forward:
        case soc_feature_port_egr_block_ctl:
        case soc_feature_bucket_support:
        case soc_feature_remote_learn_trust:
        case soc_feature_src_mac_group:
        case soc_feature_storm_control:
        case soc_feature_hw_stats_calc:
        case soc_feature_mac_learn_limit:
        case soc_feature_linear_drr_weight:
        case soc_feature_igmp_mld_support:
        case soc_feature_basic_dos_ctrl:
        case soc_feature_enhanced_dos_ctrl:
        case soc_feature_proto_pkt_ctrl:
        case soc_feature_vlan_ctrl:
        case soc_feature_big_icmpv6_ping_check:
        case soc_feature_trunk_group_overlay:
        case soc_feature_xport_convertible:
        case soc_feature_dual_hash:
        case soc_feature_dscp:
        case soc_feature_rcpu_1:
        case soc_feature_unimac:
        case soc_feature_xlmac:
        case soc_feature_clmac:
        case soc_feature_generic_table_ops:
        case soc_feature_vlan_translation_range:
        case soc_feature_static_pfm:
        case soc_feature_sgmii_autoneg:
        case soc_feature_rcpu_priority:
        case soc_feature_rcpu_tc_mapping:
        case soc_feature_mem_push_pop:
        case soc_feature_dcb_reason_hi:
        case soc_feature_multi_sbus_cmds:
        case soc_feature_new_sbus_format:
        case soc_feature_new_sbus_old_resp:
        case soc_feature_sbus_format_v4:
        case soc_feature_fifo_dma:
        case soc_feature_fifo_dma_active:
        case soc_feature_l2_pending:
        case soc_feature_internal_loopback:
        case soc_feature_vlan_action:
        case soc_feature_mac_based_vlan:
        case soc_feature_ip_subnet_based_vlan:
        case soc_feature_packet_rate_limit:
        case soc_feature_system_mac_learn_limit:
        case soc_feature_field:
        case soc_feature_field_mirror_ovr:
        case soc_feature_field_udf_higig:
        case soc_feature_field_udf_ethertype:
        case soc_feature_field_udf_offset_hg_114B:
        case soc_feature_field_udf_offset_hg2_110B:
        case soc_feature_field_comb_read:
        case soc_feature_field_wide:
        case soc_feature_field_slice_enable:
        case soc_feature_field_cos:
        case soc_feature_field_color_indep:
        case soc_feature_field_qual_drop:
        case soc_feature_field_qual_IpType:
        case soc_feature_field_qual_Ip6High:
        case soc_feature_field_qual_vlanformat_reverse:
        case soc_feature_field_ingress_global_meter_pools:
        case soc_feature_field_ingress_ipbm:
        case soc_feature_field_egress_flexible_v6_key:
        case soc_feature_field_egress_global_counters:
        case soc_feature_field_ing_egr_separate_packet_byte_counters:
        case soc_feature_field_egress_metering:
        case soc_feature_field_intraslice_double_wide:
        case soc_feature_field_virtual_slice_group:
        case soc_feature_field_virtual_queue:
        case soc_feature_field_action_timestamp:
        case soc_feature_field_action_l2_change:
        case soc_feature_field_action_redirect_nexthop:
        case soc_feature_field_action_pfc_class:
        case soc_feature_field_action_redirect_ipmc:
        case soc_feature_field_slice_dest_entity_select:
        case soc_feature_field_packet_based_metering:
        case soc_feature_lport_tab_profile:
        case soc_feature_sample_thresh16:
        case soc_feature_field_oam_actions:
        case soc_feature_oam:
        case soc_feature_ignore_cmic_xgxs_pll_status:
        case soc_feature_use_double_freq_for_ddr_pll:
        case soc_feature_counter_parity:
        case soc_feature_extended_pci_error:
        case soc_feature_qos_profile:
        case soc_feature_rx_timestamp:
        case soc_feature_rx_timestamp_upper:
        case soc_feature_logical_port_num:
        case soc_feature_timestamp_counter:
        case soc_feature_modport_map_profile:
        case soc_feature_eee:
        case soc_feature_xy_tcam:
        case soc_feature_xy_tcam_direct:
        case soc_feature_xy_tcam_28nm:
        case soc_feature_vlan_egr_it_inner_replace:
        case soc_feature_cmicm:
        case soc_feature_iproc:
        case soc_feature_iproc_7:
        case soc_feature_unified_port:
        case soc_feature_sbusdma:
        case soc_feature_mirror_encap_profile:
        case soc_feature_higig_misc_speed_support:
        case soc_feature_vpd_profile:
        case soc_feature_color_prio_map_profile:
        case soc_feature_mem_parity_eccmask:
        case soc_feature_l2_no_vfi:
        case soc_feature_gmii_clkout:
        case soc_feature_fifo_dma_hu2:
        case soc_feature_proxy_port_property:
        case soc_feature_system_reserved_vlan:
        case soc_feature_ser_parity:
        case soc_feature_mem_cache:
        case soc_feature_ser_engine:
        case soc_feature_regs_as_mem:
        case soc_feature_cmicd_v2:
        case soc_feature_cmicm_extended_interrupts:
        case soc_feature_int_common_init:
        case soc_feature_inner_tpid_enable:
        case soc_feature_no_tunnel:
        case soc_feature_eee_bb_mode:
        case soc_feature_hg_no_speed_change:
        case soc_feature_src_modid_base_index:
        case soc_feature_avs:
        case soc_feature_l2_overflow:
        case soc_feature_l2_overflow_bucket:
        case soc_feature_miml:
        case soc_feature_custom_header:
        case soc_feature_ing_capwap_parser:
        case soc_feature_port_extension:
        case soc_feature_niv:
        case soc_feature_gh2_rtag7:
        case soc_feature_no_vlan_vrf:
            /* GH2 Service Metering Block is a simple version of Apache */
        case soc_feature_global_meter:
        case soc_feature_global_meter_v2:
        case soc_feature_separate_ing_lport_rtag7_profile:
        case soc_feature_rtag7_port_profile:
        case soc_feature_no_vrf_stats:
        case soc_feature_flowcnt:
        case soc_feature_high_portcount_register:
        case soc_feature_cosq_hol_drop_packet_count:
        case soc_feature_asf:
        case soc_feature_hg2_light_in_portmacro:
        case soc_feature_tsn:
        case soc_feature_tsn_sr:
            return TRUE;
        case soc_feature_time_v3_no_bs:
            return FALSE;
        case soc_feature_e2ecc:
        case soc_feature_priority_flow_control:
        case soc_feature_gh_style_pfc_config:
        case soc_feature_time_support:
        case soc_feature_time_v3:
        case soc_feature_timesync_support:
        case soc_feature_timesync_v3:
        case soc_feature_timesync_timestampingmode:
            return TRUE;
        case soc_feature_gh2_my_station:
            return TRUE;
        case soc_feature_preempt_mib_support:
        case soc_feature_clmib_support:
        case soc_feature_ecmp_hash_bit_count_select:
            return TRUE;
        case soc_feature_field_multi_stage:
            return TRUE;
        case soc_feature_field_slices12:
        case soc_feature_field_meter_pools12:
            
            return TRUE;
        case soc_feature_field_slices8:
        case soc_feature_field_meter_pools8:
            
            return FALSE;
        case soc_feature_field_slice_size128:
            
            return FALSE;
        case soc_feature_preemption:
        case soc_feature_preemption_cnt:
            return TRUE;
        case soc_feature_l3_no_ecmp:
            return FALSE;
        /* L3 related features */
        case soc_feature_l3:
        case soc_feature_l3_ip6:
        case soc_feature_l3_entry_key_type:
        case soc_feature_lpm_tcam:
        case soc_feature_ip_mcast:
        case soc_feature_ip_mcast_repl:
        case soc_feature_ipmc_unicast:
        case soc_feature_ipmc_use_configured_dest_mac:
        case soc_feature_urpf:
        case soc_feature_l3mc_use_egress_next_hop:
        case soc_feature_l3_sgv:
        case soc_feature_l3_dynamic_ecmp_group:
        case soc_feature_l3_ingress_interface:
        case soc_feature_l3_iif_zero_invalid:
        case soc_feature_l3_iif_under_4k:
        case soc_feature_discard_ability:
        case soc_feature_wred_drop_counter_per_port:
        case soc_feature_ecn_wred:
        case soc_feature_vxlan_lite:
        case soc_feature_rroce:
#if 1
            if (!SAL_BOOT_SIMULATION) {
                return TRUE;
            } else {
#endif
                int sku;

                SOC_IF_ERROR_RETURN(soc_gh2_chip_sku_get(unit, &sku));
                if (sku == SOC_GH2_SKU_QUARTZ) {
                    return FALSE;
                } else {
                    return TRUE;
                }
            }
            break;
        case soc_feature_untethered_otp:
#if 1
            return (!SAL_BOOT_SIMULATION);
#endif
        default:
            return FALSE;
    }
}

/*
 * Function:
 *      soc_gh2_tdm_size_get
 * Purpose:
 *      Get the TDM array size.
 * Parameters:
 *      unit - unit number.
 *      tdm_size (OUT) - TDM array size
 * Returns:
 *      SOC_E_XXX
 */
int soc_gh2_tdm_size_get(int unit, int *tdm_size)
{
    _gh2_sku_info_t *matched_sku_info;

    /* The function should be called only if the TDM is programmed */
    if (matched_devid_idx == -1) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit, "Warning: soc_greyhound2_port_config_init "
                             "should be invoked first! \n")));
        return SOC_E_INIT;
    }

    /* Return the TDM size */
    matched_sku_info = &_gh2_sku_port_config[matched_devid_idx];
    *tdm_size = matched_sku_info->tdm_table_size;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_gh2_64q_port_check
 * Purpose:
 *      Check if the port(logical) is capable of configured up to 64 COSQ.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - logical port.
 *      is_64q (OUT) - with 64 COSQ or not
 * Returns:
 *      BCM_E_XXX
 */
int soc_gh2_64q_port_check(int unit, soc_port_t port, int *is_64q_port)
{
    int phy_port;
    int mmu_port;

    *is_64q_port = 0;
    phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    mmu_port = SOC_INFO(unit).port_p2m_mapping[phy_port];
    if ((mmu_port >= SOC_GH2_64Q_MMU_PORT_IDX_MIN) &&
        (mmu_port <= SOC_GH2_64Q_MMU_PORT_IDX_MAX)) {
        *is_64q_port = 1;
    } else {
        *is_64q_port = 0;
    }
    return SOC_E_NONE;
}

/*
 * For calculate GH2 MMU_MAX/MIN_BUCKET_QLAYER table index.
 *    - mmu port 0~57 : 8 entries
 *    - mmu port 58~65 : 64 entries
 *  (IN) p : logical port
 *  (IN) q : cosq
 *  (OUT) idx : the entry index of MMU_MAX/MIN_BUCKET_QLAYER table
 */
int
soc_gh2_mmu_bucket_qlayer_index(int unit, int p, int q, int *idx)
{
    int is_64q;
    int phy_port;
    int mmu_port;

    is_64q = 0;
    phy_port = SOC_INFO(unit).port_l2p_mapping[p];
    mmu_port = SOC_INFO(unit).port_p2m_mapping[phy_port];

    SOC_IF_ERROR_RETURN(soc_gh2_64q_port_check(unit, p, &is_64q));


    if (!is_64q) {
        *idx = (mmu_port * SOC_GH2_LEGACY_QUEUE_NUM) + q;
    }  else {
        *idx = (SOC_GH2_64Q_MMU_PORT_IDX_MIN * SOC_GH2_LEGACY_QUEUE_NUM) + \
               ((mmu_port-SOC_GH2_64Q_MMU_PORT_IDX_MIN) * \
                 SOC_GH2_QLAYER_COSQ_PER_PORT_NUM) + \
               q;
    }
    return SOC_E_NONE;
}

/*
 * For calculate GH2 MMU_MAX/MIN_BUCKET_QGROUP table index.
 *    - mmu port 0~57 : not available
 *    - mmu port 58~65 : 8 entries
 *  (IN) p : logical port
 *  (IN) g : queue group id
 *  (OUT) idx : the entry index of MMU_MAX/MIN_BUCKET_QGROUP table
 */
int
soc_gh2_mmu_bucket_qgroup_index(int unit, int p, int g, int *idx)
{
    int is_64q = 0;
    int phy_port;
    int mmu_port;

    SOC_IF_ERROR_RETURN(soc_gh2_64q_port_check(unit, p, &is_64q));
    if (!is_64q) {
        /* MMU port 0~57 do not have QGROUP tables */
        return SOC_E_PARAM;
    }

    phy_port = SOC_INFO(unit).port_l2p_mapping[p];
    mmu_port = SOC_INFO(unit).port_p2m_mapping[phy_port];
    *idx = ((mmu_port - SOC_GH2_64Q_MMU_PORT_IDX_MIN) * \
             SOC_GH2_QGROUP_PER_PORT_NUM) + g;

    return SOC_E_NONE;
}

int
soc_gh2_cosq_event_handler_register(
    int unit, soc_gh2_cosq_event_handler_t handler)
{
    
    return SOC_E_NONE;
}


/*
 * Greyhound2 chip driver functions.
 */
soc_functions_t soc_greyhound2_drv_funs = {
    _soc_greyhound2_misc_init,
    _soc_greyhound2_mmu_init,
    _soc_greyhound2_age_timer_get,
    _soc_greyhound2_age_timer_max_get,
    _soc_greyhound2_age_timer_set,
    _soc_greyhound2_tscx_firmware_set,
    _soc_greyhound2_tscx_reg_read,
    _soc_greyhound2_tscx_reg_write,
    soc_greyhound2_bond_info_init
};


#define GH2_CLPORT_SPEED_READY 0
soc_error_t
soc_greyhound2_granular_speed_get(int unit, soc_port_t port, int *speed)
{

#if GH2_CLPORT_SPEED_READY
    /* TBD */
#else
    *speed = SOC_INFO(unit).port_speed_max[port];
#endif
    return SOC_E_NONE;
}

/*
 * soc_greyhound2_port_speed_update
 *  - major for CL port speed update ??
 *  - This process need confirm with chip team.
 */
int
soc_greyhound2_port_speed_update(int unit, soc_port_t port, int speed)
{
#if GH2_CLPORT_SPEED_READY
    /* TBD */
#else
    /* do nothing right now */
    if (speed > SOC_INFO(unit).port_speed_max[port]) {
        return SOC_E_PARAM;
    }
#endif

    return SOC_E_NONE;
}
#endif /* BCM_GREYHOUND2_SUPPORT */
