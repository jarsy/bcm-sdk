/*
 * $Id: ocm.c,v 1.47.10.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:        ocm.c
 * Purpose: Caladan3 on chip memory drivers
 * Requires:
 * Notes:
 */



#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/caladan3.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <soc/sbx/caladan3/soc_sw_db.h>
#endif
#include <soc/sbx/sbDq.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/error.h>

/*#define OCM_DEBUG 1*/

#ifdef OCM_DEBUG
int sbx_caladan3_ocm_dump_allocator(int unit, int port);
#endif

static char *ocm_port_name[SOC_SBX_CALADAN3_MAX_OCM_PORT] = {
    "SOC_SBX_CALADAN3_OCM_LRP0_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP1_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP2_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP3_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP4_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP5_PORT",
    "SOC_SBX_CALADAN3_OCM_CMU0_PORT",
    "SOC_SBX_CALADAN3_OCM_COP0_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP6_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP7_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP8_PORT",
    "SOC_SBX_CALADAN3_OCM_LRP9_PORT",
    "SOC_SBX_CALADAN3_OCM_COP1_PORT",
    "SOC_SBX_CALADAN3_OCM_CMU1_PORT"
};

const char *soc_sbx_ocm_port_num_to_name(sbx_caladan3_ocm_port_e_t port_num)
{
    if (port_num >= SOC_SBX_CALADAN3_MAX_OCM_PORT) {
            return NULL;
    } else {
        return ocm_port_name[port_num];
    }
    return NULL;
}

static int ocm_debug = 0;

/* OCM port attributes */

#define SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK (128)

#define SOC_SBX_CALADAN3_OCM_INVALID (-1)
#define SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY (64)


#define SOC_SBX_OCM_MAX_ENTRY_NUM_WORDS (BITS2WORDS(72))


#define SOC_SBX_CALADAN3_MAX_LRP_128_PORT_SEGMENT    (128)
#define SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT     (64)
#define SOC_SBX_CALADAN3_MAX_LRP_BUBBLE_PORT_SEGMENT (1) /* segment 0 is only valid segement */
#define SOC_SBX_CALADAN3_MAX_CMU_COP_PORT_SEGMENT    (-1)

typedef struct sbx_caladan3_ocm_phy_blk_info_s {
    sbx_caladan3_ocm_port_e_t port_num; /* <0 invalid or free phy blk */
    int      log_blk_num; /* logical block using this physical block */
} sbx_caladan3_ocm_phy_blk_info_t;

typedef struct sbx_caladan3_ocm_log_blk_info_s {
    int  phy_blk_num;
    /* max port supported is 96, so use array of 2 bit map for now */
    pbmp_t   seg_bitmap[SOC_SBX_CALADAN3_OCM_NUM_MEM];
} sbx_caladan3_ocm_log_blk_info_t;



typedef struct  sbx_caladan3_ocm_segment_info_s {
    dq_t     listnode;     /* list node on log_addrmap_sorted_list         */
    int      valid;
    int      segment;      /* segment id if applicable */
    uint32   log_addr_base;/* logical base address       */
    uint32   size;         /* total number of entries    */  
    uint8    datum_size;   /* entry size in this segment */
    int      base_log_blk_num; /* base logical block number  */
    int      num_log_blk;  /* number of logical blocks   */

    /* Associated Data from creator of segment. Called "table" and
     * "bank" but can be used by caller as they see fit.
     * Default is it used as a heirarchical structure with a 
     * "bank" existing within a table.
     *
     * Table Id of ZERO is reserved. */
    pbmp_t   table_id;

    pbmp_t   bank_id[_SOC_SBX_OCM_MAX_TABLE_ID]; /* Each table has its own banks */
} sbx_caladan3_ocm_segment_info_t;

#define _SOC_SBX_OCM_GET_LOG_SEG_FROM_LIST(e, var) \
            (var) = DQ_ELEMENT(sbx_caladan3_ocm_segment_info_t*,\
                               (e), (var), listnode)

#define SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK (16*1024)
#define SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK (SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK * SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY)

/* logical address layout 
 *.........................................................
 *+ 27/26.....20 + 19.......................6 + 5 .......0 +
 *.........................................................
 *+ log blk num  +   quad word index          + entry idx  + 
 *                                            + based on   +
 *                                            + datum-size +
 */ 
#define SOC_SBX_CALADAN3_OCM_LOG_BLK_SHIFT (20)
#define SOC_SBX_CALADAN3_OCM_ENTRY_SHIFT   (6)

typedef struct sbx_caladan3_ocm_port_info_s {
    sbx_caladan3_ocm_port_e_t port_num;
    char    *name;
    int     addr_width;
    int     min_blk;
    int     max_blk;
    int     num_segment;  /* <0 - not capable */
    int     num_segment_bits;  /* <0 - not capable */
    /* array of segments & their attribute */
    sbx_caladan3_ocm_segment_info_t *seg_attr;
    /* array of logical block & their attribute */
    sbx_caladan3_ocm_log_blk_info_t *log_blk_attr;
    /* logical address allocation map, sorted in ascending order */
    dq_t    log_addrmap_sorted_list;
    uint32 size;         /* total number of entries, used for ports without segment  */  
    uint32 datum_size;   /* size of entries, used for ports without segment */
    /* segment allocator bit map */
    pbmp_t   seg_alloc_bitmap[SOC_SBX_CALADAN3_OCM_NUM_MEM];
} sbx_caladan3_ocm_port_info_t;

typedef struct sbx_caladan3_ocm_control_info_s {
    int driver_init; /* >0 if driver is initialized */
    sbx_caladan3_ocm_port_info_t     portcb[SOC_SBX_CALADAN3_MAX_OCM_PORT];
    sbx_caladan3_ocm_phy_blk_info_t  phyBlkInfo[SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK]; 
    uint32 dma_threshold; /* dma threshold */
} sbx_caladan3_ocm_control_info_t;

static sbx_caladan3_ocm_control_info_t ocmState[SOC_MAX_NUM_DEVICES] = {{0}};

#define OCM_PORTCB(unit,index)  (ocmState[unit].portcb[index])
#define OCM_PHY_BLK(unit,index) (ocmState[unit].phyBlkInfo[index])

#define SOC_SBX_CALADAN3_OCM_PORT_NUM_BLK(unit,index) \
(OCM_PORTCB(unit,index).max_blk - OCM_PORTCB(unit,index).min_blk)

#define SOC_SBX_CALADAN3_OCM_ENA_ALL_PHY_BLK (0xFFFFFFFF)
#define SOC_SBX_CALADAN3_OCM_DMA_THRESHOLD   (2)

soc_mem_t oc_port_segment_mem[] = {
    OC_LRP_PORT0_SEGMENTm, OC_LRP_PORT1_SEGMENTm, OC_LRP_PORT2_SEGMENTm,
    OC_LRP_PORT3_SEGMENTm, OC_LRP_PORT4_SEGMENTm, OC_LRP_PORT5_SEGMENTm,
    SOC_SBX_CALADAN3_OCM_INVALID, SOC_SBX_CALADAN3_OCM_INVALID, OC_LRP_BUBBLE_PORT_SEGMENTm, 
    OC_LRP_PORT6_SEGMENTm, OC_LRP_PORT7_SEGMENTm, OC_LRP_PORT8_SEGMENTm,
    OC_LRP_PORT9_SEGMENTm, SOC_SBX_CALADAN3_OCM_INVALID, SOC_SBX_CALADAN3_OCM_INVALID
};

soc_mem_t oc_port_block_mem[] = {
    OC_LRP_PORT0_BLOCKm, OC_LRP_PORT1_BLOCKm, OC_LRP_PORT2_BLOCKm,
    OC_LRP_PORT3_BLOCKm, OC_LRP_PORT4_BLOCKm, OC_LRP_PORT5_BLOCKm,
    OC_CMU_PORT0_BLOCKm, OC_COP0_PORT_BLOCKm, OC_LRP_BUBBLE_PORT_BLOCKm,
    OC_LRP_PORT6_BLOCKm, OC_LRP_PORT7_BLOCKm, OC_LRP_PORT8_BLOCKm,
    OC_LRP_PORT9_BLOCKm, OC_CMU_PORT1_BLOCKm, OC_COP1_PORT_BLOCKm
};

soc_reg_t oc_port_mapping_error_mask_reg[] = {
    OC_LRP_PORT0_MAPPING_ERROR_MASKr, OC_LRP_PORT1_MAPPING_ERROR_MASKr,OC_LRP_PORT2_MAPPING_ERROR_MASKr,
    OC_LRP_PORT3_MAPPING_ERROR_MASKr, OC_LRP_PORT4_MAPPING_ERROR_MASKr,OC_LRP_PORT5_MAPPING_ERROR_MASKr,
    OC_CMU_PORT0_MAPPING_ERROR_MASKr, OC_COP0_PORT_MAPPING_ERROR_MASKr, OC_LRP_BUBBLE_PORT_MAPPING_ERROR_MASKr,
    OC_LRP_PORT6_MAPPING_ERROR_MASKr, OC_LRP_PORT7_MAPPING_ERROR_MASKr,OC_LRP_PORT8_MAPPING_ERROR_MASKr,
    OC_LRP_PORT9_MAPPING_ERROR_MASKr, OC_CMU_PORT1_MAPPING_ERROR_MASKr, OC_COP1_PORT_MAPPING_ERROR_MASKr
};

typedef struct ocm_intr_info_s {
    int reg;
    int mask;
    uint32 enable;
    int status_reg0;
    int status_reg1;
} ocm_intr_info_t; 

/* interrupts handled */
static ocm_intr_info_t ocm_interrupt[] = {
    {OC_LRP_PORT0_MAPPING_ERRORr, OC_LRP_PORT0_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT0_MAPPING_ERROR_STATUS0r, OC_LRP_PORT0_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT1_MAPPING_ERRORr, OC_LRP_PORT1_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT1_MAPPING_ERROR_STATUS0r, OC_LRP_PORT1_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT2_MAPPING_ERRORr, OC_LRP_PORT2_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT2_MAPPING_ERROR_STATUS0r, OC_LRP_PORT2_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT3_MAPPING_ERRORr, OC_LRP_PORT3_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT3_MAPPING_ERROR_STATUS0r, OC_LRP_PORT3_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT4_MAPPING_ERRORr, OC_LRP_PORT4_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT4_MAPPING_ERROR_STATUS0r, OC_LRP_PORT4_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT5_MAPPING_ERRORr, OC_LRP_PORT5_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT5_MAPPING_ERROR_STATUS0r, OC_LRP_PORT5_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT6_MAPPING_ERRORr, OC_LRP_PORT6_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT6_MAPPING_ERROR_STATUS0r, OC_LRP_PORT6_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT7_MAPPING_ERRORr, OC_LRP_PORT7_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT7_MAPPING_ERROR_STATUS0r, OC_LRP_PORT7_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT8_MAPPING_ERRORr, OC_LRP_PORT8_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT8_MAPPING_ERROR_STATUS0r, OC_LRP_PORT8_MAPPING_ERROR_STATUS1r},
    {OC_LRP_PORT9_MAPPING_ERRORr, OC_LRP_PORT9_MAPPING_ERROR_MASKr, 0x0, OC_LRP_PORT9_MAPPING_ERROR_STATUS0r, OC_LRP_PORT9_MAPPING_ERROR_STATUS1r},
    {OC_CMU_PORT0_MAPPING_ERRORr, OC_CMU_PORT0_MAPPING_ERROR_MASKr, 0x0, OC_CMU_PORT0_MAPPING_ERROR_STATUSr, INVALIDr},
    {OC_CMU_PORT1_MAPPING_ERRORr, OC_CMU_PORT1_MAPPING_ERROR_MASKr, 0x0, OC_CMU_PORT1_MAPPING_ERROR_STATUSr, INVALIDr},
    {OC_COP0_PORT_MAPPING_ERRORr, OC_COP0_PORT_MAPPING_ERROR_MASKr, 0x0, OC_COP0_PORT_MAPPING_ERROR_STATUSr, INVALIDr},
    {OC_COP1_PORT_MAPPING_ERRORr, OC_COP1_PORT_MAPPING_ERROR_MASKr, 0x0, OC_COP1_PORT_MAPPING_ERROR_STATUSr, INVALIDr},
    {OC_LRP_BUBBLE_PORT_MAPPING_ERRORr, OC_LRP_BUBBLE_PORT_MAPPING_ERROR_MASKr, 0x0, OC_LRP_BUBBLE_PORT_MAPPING_ERROR_STATUS0r, OC_LRP_BUBBLE_PORT_MAPPING_ERROR_STATUS1r},
    {OC_SEGMENT_TABLE_ECC_ERRORr, OC_SEGMENT_TABLE_ECC_ERROR_MASKr, 0x0, INVALIDr, INVALIDr},
    {OC_BLOCK_TABLE_PARITY_ERRORr, OC_BLOCK_TABLE_PARITY_ERROR_MASKr, 0x0, INVALIDr, INVALIDr},
    {OC_MISC_ERRORr, OC_MISC_ERROR_MASKr, 0x0, INVALIDr, INVALIDr},
    {OC_MEMORY_ERRORr, OC_MEMORY_ERROR_MASKr, 0x0, INVALIDr, INVALIDr}
};

#define OCM_INTR_REG(index) ocm_interrupt[index].reg
#define OCM_INTR_MASK(index) ocm_interrupt[index].mask
#define OCM_INTR_ENABLE_VAL(index) ocm_interrupt[index].enable
#define OCM_INTR_STATUS_REG0(index) ocm_interrupt[index].status_reg0
#define OCM_INTR_STATUS_REG1(index) ocm_interrupt[index].status_reg1




static char *soc_sbx_caladan3_ocm_util_port_num_enum_str(sbx_caladan3_ocm_port_e_t port)
{
    static char ocm_port_e_str[128];
    sal_memset(ocm_port_e_str, 0, sizeof(ocm_port_e_str));

    switch (port) {
    case SOC_SBX_CALADAN3_OCM_PORT_INVALID:
        sal_strncpy(ocm_port_e_str, "OCM_PORT_INVALID", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP0_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP0_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP1_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP1_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP2_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP2_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP3_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP3_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP4_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP4_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP5_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP5_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_CMU0_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_CMU0_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_COP0_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_COP0_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP_BUBBLE_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP6_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP6_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP7_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP7_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP8_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP8_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_LRP9_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_LRP9_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_CMU1_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_CMU1_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_OCM_COP1_PORT:
        sal_strncpy(ocm_port_e_str, "OCM_COP1_PORT", sizeof(ocm_port_e_str));
        break;

    case SOC_SBX_CALADAN3_MAX_OCM_PORT:
        sal_strncpy(ocm_port_e_str, "MAX_OCM_PORT", sizeof(ocm_port_e_str));
        break;

    default:
        sal_strncpy(ocm_port_e_str, "OCM_PORT_UNKNOWN", sizeof(ocm_port_e_str));
        break;

    }

    return ocm_port_e_str;
}



STATIC 
int _soc_sbx_caladan3_ocm_port_map_phy_blk(int unit, 
                                           sbx_caladan3_ocm_port_alloc_t *desc,
                                           uint8 map /*1-map, 0-unmap*/)
{
    int index, log_blk_num=0;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_log_blk_info_t *log_blk_info;
    oc_lrp_port0_block_entry_t entry;
    uint32 field_buf;

    /* assumes inputs are validated by caller */
    port_info = &OCM_PORTCB(unit,desc->port);

    log_blk_num = desc->size / SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK;
    if ((desc->size % SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK) > 0) {
        log_blk_num ++;
    }
    
    for (index=0; index < log_blk_num; index++) {

        log_blk_info = &port_info->log_blk_attr[index];

        SOC_IF_ERROR_RETURN(soc_mem_read(unit,oc_port_block_mem[desc->port],
                                         MEM_BLOCK_ANY,index,&entry));

        if (map > 0) {
            field_buf = log_blk_info->phy_blk_num - port_info->min_blk;
        } else {
            field_buf = 0;
        }
        soc_mem_field_set(unit, oc_port_block_mem[desc->port], 
                          &entry.entry_data[0], PHYSICAL_BLOCKf, &field_buf);
        
        field_buf = (map > 0)?1:0;
        soc_mem_field_set(unit, oc_port_block_mem[desc->port], 
                          &entry.entry_data[0], VALIDf, &field_buf);
        
        SOC_IF_ERROR_RETURN(soc_mem_write(unit,oc_port_block_mem[desc->port],
                                         MEM_BLOCK_ANY,index,&entry));
    }
    return SOC_E_NONE;
}


STATIC 
int _soc_sbx_caladan3_ocm_port_map_segment_phy_blk(int unit, 
                                                   sbx_caladan3_ocm_port_alloc_t *desc,
                                                   uint8 map /*1-map, 0-unmap*/)
{
    int index;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;
    sbx_caladan3_ocm_log_blk_info_t *log_blk_info;
    oc_lrp_port0_block_entry_t entry;
    uint32 field_buf;

    /* assumes inputs are validated by caller */
    port_info = &OCM_PORTCB(unit,desc->port);
    seg_info = &port_info->seg_attr[desc->segment];

    if (SOC_SBX_CALADAN3_OCM_INVALID == seg_info->valid) {
        return SOC_E_DISABLED;
    } else {

	if (ocm_debug) {
	    LOG_CLI((BSL_META_U(unit,
                                "\n%s\n"), FUNCTION_NAME()));
	}

        for (index=seg_info->base_log_blk_num; 
             index < (seg_info->base_log_blk_num + seg_info->num_log_blk); index++) {

            log_blk_info = &port_info->log_blk_attr[index];

            SOC_IF_ERROR_RETURN(soc_mem_read(unit,oc_port_block_mem[desc->port],
                                             MEM_BLOCK_ANY,index,&entry));

            if (map > 0) {
                /* Enforce HW constraint of programming PHY BLK # relative to MIN BLK number
                 * for Port; the MIN BLK # is either 0 or 64 (LRP 6-9 COP1 & CMU1)
                 */
                field_buf = log_blk_info->phy_blk_num - port_info->min_blk;
            } else {
                field_buf = 0;
            }

            soc_mem_field_set(unit, oc_port_block_mem[desc->port], 
                              &entry.entry_data[0], PHYSICAL_BLOCKf, &field_buf);
            
            field_buf = (map > 0)?1:0;
            soc_mem_field_set(unit, oc_port_block_mem[desc->port], 
                              &entry.entry_data[0], VALIDf, &field_buf);


            if (ocm_debug) {
                LOG_CLI((BSL_META_U(unit,
                                    "PROGRAMMING PORT_BLOCK Table-> SEG(%d) Port(%s) Logical Block(%d)-->Physical Block(SW:%d/HW:%d) V(%d)\n"),
                         seg_info->segment, soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port),
                         index, log_blk_info->phy_blk_num,
                         (log_blk_info->phy_blk_num - port_info->min_blk), field_buf));
            }
            
            SOC_IF_ERROR_RETURN(soc_mem_write(unit,oc_port_block_mem[desc->port],
                                             MEM_BLOCK_ANY,index,&entry));
        }
    }


    return SOC_E_NONE;
}


STATIC 
int _soc_sbx_caladan3_ocm_port_program_segment(int unit, 
                                               int port,
                                               int segment, 
                                               uint8 enable /*1-enable, 0-disable*/)
{
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;
    oc_lrp_bubble_port_segment_entry_t entry;
    uint32 field_buf;

#ifdef BCM_WARM_BOOT_SUPPORT
    /*
     * For warm boot, DMA segment is 'programmed' to allow
     * a DMA read, so allow that special case.
     */
    if(SOC_WARM_BOOT(unit) && (segment != _SOC_SBX_CALADAN3_DMA_SEGMENT))
    {
        /*
	 * For warm boot, none of the hardware reads below are
	 * necessary - and *may* be disruptive to re-program these 
	 * segments (even to the same values).
	 * 
	 * Rev A0 doesn't allow reads from bubble
	 * memory, so the code below fails in that instance.
	 */
        return SOC_E_NONE;
    }
    else if(SOC_WARM_BOOT(unit) && (port == SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT) )
    {
        return SOC_E_NONE;
    }
#endif 


    /* assumes inputs are validated by caller */
    port_info = &OCM_PORTCB(unit,port);
    seg_info = &port_info->seg_attr[segment];

    assert(seg_info->valid != SOC_SBX_CALADAN3_OCM_INVALID);

    SOC_IF_ERROR_RETURN(soc_mem_read(unit,oc_port_segment_mem[port],
                                     MEM_BLOCK_ANY,segment,&entry));

    if (ocm_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "\n%s\n"), FUNCTION_NAME()));
    }

    if (!enable) {
        field_buf = 7; /*disable*/
        soc_mem_field_set(unit, oc_port_segment_mem[port], 
                          &entry.entry_data[0], SEGMENT_SIZEf, &field_buf); 

        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "PROGRAMMING (D) SEG TABLE  -> Segment(%d) Port(%s) DISABLED\n"),
                     segment, soc_sbx_caladan3_ocm_util_port_num_enum_str(port)));
        }
        
    } else {
        /* assert if base address is not 64 bit alighned */
        assert(!(seg_info->log_addr_base % SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY));

        /* set to 64bit quad offset address logblk + quad offset (no entry offset notion) */
        field_buf = seg_info->log_addr_base >> SOC_SBX_CALADAN3_OCM_ENTRY_SHIFT;
        
        soc_mem_field_set(unit, oc_port_segment_mem[port], 
                          &entry.entry_data[0], SEGMENT_BASEf, &field_buf);    


        /* 27 bit max address limit */
        if (seg_info->size > 0) {
            field_buf = seg_info->size - 1;
        }
        else {
            field_buf = seg_info->size;
        }

        soc_mem_field_set(unit, oc_port_segment_mem[port], 
                          &entry.entry_data[0], SEGMENT_LIMITf, &field_buf); 

        /* dirty way for sqrt due to lack of portability & fp */
        switch(seg_info->datum_size) {
        case SOC_SBX_CALADAN3_DATUM_SIZE_BIT:
            field_buf = 0;
            break;
        case SOC_SBX_CALADAN3_DATUM_SIZE_DBIT:
            field_buf = 1;
            break;
        case SOC_SBX_CALADAN3_DATUM_SIZE_NIBBLE:
            field_buf = 2;
            break;
        case SOC_SBX_CALADAN3_DATUM_SIZE_BYTE:
            field_buf = 3;
            break;
        case SOC_SBX_CALADAN3_DATUM_SIZE_WORD:
            field_buf = 4;
            break;
        case SOC_SBX_CALADAN3_DATUM_SIZE_LONGWORD:
            field_buf = 5;
            break;
        case SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD:
            field_buf = 6;
            break;
        default:
            field_buf = 7; /*disable*/
            break;
        }

        soc_mem_field_set(unit, oc_port_segment_mem[port], 
                          &entry.entry_data[0], SEGMENT_SIZEf, &field_buf); 

        field_buf = 0;
        soc_mem_field_set(unit, oc_port_segment_mem[port], 
                          &entry.entry_data[0],
                          SEGMENT_NULL_OFFSETf,
                          &field_buf);     

        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "PROGRAMMING (A) SEG TABLE  -> Segment(%d) Port(%s) SZ:(%d)  LOG Base Addr(0x%x)  Limit(0x%x)\n\n"),
                     segment, soc_sbx_caladan3_ocm_util_port_num_enum_str(port),
                     seg_info->datum_size,
                     seg_info->log_addr_base >> SOC_SBX_CALADAN3_OCM_ENTRY_SHIFT,
                     seg_info->size ? (seg_info->size - 1): seg_info->size));
        }
        
    }

    SOC_IF_ERROR_RETURN(soc_mem_write(unit,oc_port_segment_mem[port],
                                      MEM_BLOCK_ANY,segment,&entry));
    return SOC_E_NONE;
}


STATIC 
int sbx_caladan3_ocm_port_update_segment_log_bitmap(int unit, 
                                                    int segment,
                                                    int port,
                                                    uint8 set /*0-clear, 1-set*/)
{
    int status = SOC_E_NONE, index=0;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;

    /* assumes inputs are validated by caller */
    port_info = &OCM_PORTCB(unit,port);
    seg_info = &port_info->seg_attr[segment];

    if (SOC_SBX_CALADAN3_OCM_INVALID == seg_info->valid) {
        status = SOC_E_PARAM;
    } else {
        sbx_caladan3_ocm_log_blk_info_t *log_blk_info;

        for(index=seg_info->base_log_blk_num; 
            index < seg_info->base_log_blk_num + seg_info->num_log_blk;
            index++) {
            log_blk_info = &port_info->log_blk_attr[index];

            if (set) {
                SOC_PBMP_PORT_ADD(log_blk_info->seg_bitmap[segment/
                                                           SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT],
                                  segment);
            } else {
                SOC_PBMP_PORT_REMOVE(log_blk_info->seg_bitmap[segment/
                                                              SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT],
                                     segment);
            }
        }
    }
    return status;
}

STATIC 
int _soc_sbx_caladan3_ocm_port_segment_log_map(int unit, 
                                               sbx_caladan3_ocm_port_alloc_t *desc,
                                               uint8 alloc)
{
    int status = SOC_E_NONE;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;
    uint32 free_size, alloc_size;


    /* assumes inputs are validated by caller */
    port_info = &OCM_PORTCB(unit,desc->port);
    seg_info = &port_info->seg_attr[desc->segment];

    if (ocm_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "%s\nMAP  SEG(%d)  Port(%s)\n"), FUNCTION_NAME(), desc->segment, 
                 soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port)));
    }
  
    if (alloc) {
        if (DQ_EMPTY(&port_info->log_addrmap_sorted_list)) {
            /* allocate the size right away */
            seg_info->log_addr_base = 0;
            DQ_INSERT_HEAD(&port_info->log_addrmap_sorted_list, &seg_info->listnode);
        } else {
            sbx_caladan3_ocm_segment_info_t *logseginfo=NULL, *prevlogseginfo=NULL;
            dq_p_t listelem;            

            /* logical addresses are based on bits, adjust the size required to bits */
            alloc_size = desc->size * desc->datum_size;

            /* find out a logical space for this segment */
            DQ_TRAVERSE(&port_info->log_addrmap_sorted_list, listelem) {

                _SOC_SBX_OCM_GET_LOG_SEG_FROM_LIST(listelem, logseginfo);

                if(DQ_TAIL(&port_info->log_addrmap_sorted_list, dq_p_t) == listelem) {

                    /* only one element on this list */
                    /* This is really the "currently used space", the free space is calculated below */
                    free_size = logseginfo->log_addr_base +
                                logseginfo->size * logseginfo->datum_size;

                    /* This calculation can be misleading since Ports have access
                     * to more than one Physical Blocks, BUT some of those blocks can be in use by other
                     * Ports. This calculation does not account for what blocks are already in use.
                     */
                    free_size = ((port_info->max_blk - port_info->min_blk + 1) * \
                                   SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK) - free_size;

                    if (ocm_debug) {
                        LOG_CLI((BSL_META_U(unit,
                                            "At TAIL:  Map for SEG(%d) Total Free Size For Port(%d)  Port(%s)\n"),
                                 desc->segment, free_size, soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port)));
                    }
                    
                    /* verify if sufficient logical address space is available */
                    if (alloc_size > free_size) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "sbx_caladan3_ocm_port_segment_log_map: unit %d"
                                              " no space available for this port %d \n"),
                                   unit, desc->port));
                        return SOC_E_PARAM;
                    }
                    /* sorted insert */
                    DQ_INSERT_TAIL(&port_info->log_addrmap_sorted_list, &seg_info->listnode);
                    seg_info->log_addr_base = logseginfo->log_addr_base +
                                              logseginfo->size * logseginfo->datum_size;
                    if (ocm_debug) {
                        LOG_CLI((BSL_META_U(unit,
                                            "Insrt new TAIL. Map for SEG(%d) descSz(%d) datumSz(%d) Port(%s) LogBaseAddr(0x%x)\n"),
                                 desc->segment, desc->size, desc->datum_size,
                                 soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port),
                                 seg_info->log_addr_base));
                    }
                } else {
                    if (prevlogseginfo && prevlogseginfo != logseginfo) {

                        /* see if this hole can fit the new segment */
                        free_size =  logseginfo->log_addr_base - 
                                     (prevlogseginfo->log_addr_base +
                                      prevlogseginfo->size * prevlogseginfo->datum_size);

                        if (alloc_size <= free_size) {
                            /* create & insert the segment before this logseginfo */
                            DQ_INSERT_PREV(&logseginfo->listnode, &seg_info->listnode);

                            seg_info->log_addr_base = prevlogseginfo->log_addr_base +
                                prevlogseginfo->size * prevlogseginfo->datum_size;

                            if (ocm_debug) {
                                LOG_CLI((BSL_META_U(unit,
                                                    "Found Space after PREV Map for SEG(%d) Free Size(%d) descSz(%d) datumSz(%d) Port(%s) LogBaseAddr(0x%x)\n"),
                                         desc->segment, free_size, desc->size, desc->datum_size,
                                         soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port),
                                         seg_info->log_addr_base));
                            }
                            
                            break;
                        }
                        
                        prevlogseginfo = logseginfo;
                    } else {
                        prevlogseginfo = logseginfo;
                    }
                }
            } DQ_TRAVERSE_END(&port_info->log_addrmap_sorted_list, listelem);  
        }

        seg_info->valid = 1;

        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "\nTable Id[%d]  Bank Id[%d] Request for ALLOC\n"), desc->table_id - 1, 
                     desc->bank_id - 1));
        }

        if (desc->table_id && (desc->table_id < _SOC_SBX_OCM_MAX_TABLE_ID)) {

            SOC_PBMP_PORT_ADD(seg_info->table_id, desc->table_id);

            if (desc->bank_id && (desc->bank_id < _SOC_SBX_OCM_MAX_BANK_ID)) {

                SOC_PBMP_PORT_ADD(seg_info->bank_id[desc->table_id], desc->bank_id);
            }
        }

        seg_info->base_log_blk_num = seg_info->log_addr_base >> SOC_SBX_CALADAN3_OCM_LOG_BLK_SHIFT;
        seg_info->base_log_blk_num &= ((1 << port_info->num_segment_bits) -1);
        seg_info->datum_size = desc->datum_size;
        seg_info->size = desc->size;  
        seg_info->num_log_blk = 0;      


        if (seg_info->size > 0) {
            alloc_size = seg_info->datum_size * seg_info->size;

            /* 
             * If logical base is not on 16Kx64 boundary, the segment will share physical block of another 
             * segment. 
             *
             * NOTE: This can ONLY be true if the Logical Block Number is the SAME as that
             * OTHER Segment, since Logical Blocks can NOT share Physical Blocks!!
             * This constraint needs to be qualified by PORT_n however. Believe for the same 
             * PORT_x Logical Blocks CAN share Physical Blocks if one desires different
             * transfer width access to same Physical Block.
             *
             * So account that delta on number of logical block
             *
             */
            if (seg_info->log_addr_base % SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK) {

                free_size = SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK - 
                    (seg_info->log_addr_base % SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK);

                if (ocm_debug) {
                    LOG_CLI((BSL_META_U(unit,
                                        "\nBase Addr(%d) NOT aligned to SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK\n"),
                             seg_info->log_addr_base));
                    LOG_CLI((BSL_META_U(unit,
                                        "\nAlign SEG(%d) Free Size(%d bits) left in Log Blk.  AllocSz(%d) descSz(%d) datumSz(%d)\n"),
                             desc->segment, free_size, alloc_size, desc->size, desc->datum_size ));
                }

                if (alloc_size >= free_size) {
                    alloc_size -= free_size;
                } else {
                    alloc_size = 0;
                }

                seg_info->num_log_blk++;

                if (ocm_debug) {
                    LOG_CLI((BSL_META_U(unit,
                                        "Base Address not aligned SEG(%d) Port(%s), Free Size(%d)  New AllocSz?(%d)  descSz(%d) datumSz(%d) Blks(%d)\n"),
                             desc->segment, soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port),
                             free_size, alloc_size, desc->size, desc->datum_size,
                             seg_info->num_log_blk ));
                }
            }

            seg_info->num_log_blk += (alloc_size/SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK);

            if (ocm_debug) {
                LOG_CLI((BSL_META_U(unit,
                                    "SEG(%d) Port(%s) AllocSz(%d bits) descSz(%d) datumSz(%d) BaseLogAddr(0x%x) BaseBlk(%d) BlksRequired(%d)\n"),
                         desc->segment, soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port),
                         alloc_size,
                         desc->size, desc->datum_size,
                         seg_info->log_addr_base,
                         seg_info->base_log_blk_num,
                         seg_info->num_log_blk));
            }

            if (alloc_size % SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK) {
                seg_info->num_log_blk++;

                if (ocm_debug) {
                    LOG_CLI((BSL_META_U(unit,
                                        "Bumping # LOG BLKs required due to alignment SEG(%d) BlksReq(%d)\n"),
                             desc->segment, seg_info->num_log_blk )); 
                }
            }
            
        }

        /* update segment use map on logical blk info */
        status = sbx_caladan3_ocm_port_update_segment_log_bitmap(unit, desc->segment,
                                                                 desc->port, TRUE);
        if (SOC_FAILURE(status)) {
            seg_info->valid = SOC_SBX_CALADAN3_OCM_INVALID;
        }
    } else {
        if (SOC_SBX_CALADAN3_OCM_INVALID == seg_info->valid) {
            status = SOC_E_DISABLED;
        } else {

            if (desc->table_id && (desc->table_id < _SOC_SBX_OCM_MAX_TABLE_ID)) {                
                int existing_bank_count = 0;
                /* See how many banks are left in the table. */
                SOC_PBMP_COUNT(seg_info->bank_id[desc->table_id], 
                               existing_bank_count);

                if (desc->bank_id && 
                    (desc->bank_id < _SOC_SBX_OCM_MAX_BANK_ID) &&
                    existing_bank_count && 
                    SOC_PBMP_MEMBER(seg_info->bank_id[desc->table_id], desc->bank_id)) {
                    
                    SOC_PBMP_PORT_REMOVE(seg_info->bank_id[desc->table_id], 
                                         desc->bank_id);

                    existing_bank_count--;
                }

                /* Only remove table once last bank is removed from it.*/
                if (!existing_bank_count) {
                    SOC_PBMP_PORT_REMOVE(seg_info->table_id,
                                         desc->table_id);
                }

            }

            DQ_REMOVE(&seg_info->listnode);

            seg_info->log_addr_base = 0; /*clear*/
            status = sbx_caladan3_ocm_port_update_segment_log_bitmap(unit, desc->segment,
                                                                 desc->port, FALSE);
            seg_info->num_log_blk   = 0;
            seg_info->base_log_blk_num = 0;
            seg_info->valid = SOC_SBX_CALADAN3_OCM_INVALID;
        }
    }

    return status;
}



STATIC 
int _soc_sbx_caladan3_ocm_port_segment_phy_blk_alloc_free(int unit, 
                                                          sbx_caladan3_ocm_port_alloc_t *desc,
                                                          uint8 alloc)
{
    int status = SOC_E_NONE, index;
    int num_log_blk = 0;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;
    sbx_caladan3_ocm_log_blk_info_t *log_blk_info;
    sbx_caladan3_ocm_phy_blk_info_t *phy_blk_info=NULL;

    /* assumes inputs are validated by caller */
    port_info = &OCM_PORTCB(unit,desc->port);
    seg_info = &port_info->seg_attr[desc->segment];


    if (alloc) {
        dq_p_t listelem;
        sbx_caladan3_ocm_segment_info_t *logseginfo=NULL;
        int log_blk_num=0, free_phy_blk=0, reuse_log_blk_id=SOC_SBX_CALADAN3_OCM_INVALID;

        /* verify if there are enough physical blk to accomodate this segment */
        /* walk through current logical block number & verify if they are valid.
         * for 1 logical block physical memory might be shared, account it as free memory size to
         * be allocated. It is sufficient to consider the first & last logical block numbers */
        DQ_TRAVERSE(&port_info->log_addrmap_sorted_list, listelem) {

            _SOC_SBX_OCM_GET_LOG_SEG_FROM_LIST(listelem, logseginfo);

            if(SOC_SBX_CALADAN3_OCM_INVALID != logseginfo->valid) {

                log_blk_num = logseginfo->log_addr_base >> SOC_SBX_CALADAN3_OCM_LOG_BLK_SHIFT;
                log_blk_num &=  ((1 << port_info->num_segment_bits) -1);

                if (ocm_debug) {
                    LOG_CLI((BSL_META_U(unit,
                                        "LogSEG(%d) DescPort(%s) LogSegBaseLogBlkNum(%d) log_addr_base(%d)  log_blk_num(%d)\n"),
                             logseginfo->segment, soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port),
                             logseginfo->base_log_blk_num, logseginfo->log_addr_base, log_blk_num));
                }

                /* first block */
                if (log_blk_num == seg_info->base_log_blk_num) {
                    log_blk_info = &port_info->log_blk_attr[log_blk_num];

                    if (ocm_debug) {
                        LOG_CLI((BSL_META_U(unit,
                                            "First Block   log_blk_info->phy_blk_num(%d)\n"),
                                 log_blk_info->phy_blk_num));
                    }

                    if (SOC_SBX_CALADAN3_OCM_PORT_INVALID != log_blk_info->phy_blk_num) {
                        reuse_log_blk_id = seg_info->base_log_blk_num;
                        free_phy_blk++;

                        if (ocm_debug) {
                            LOG_CLI((BSL_META_U(unit,
                                                "   reuse_log_blk_id(%d)  free_phy_blk(%d)\n"),
                                     reuse_log_blk_id, free_phy_blk));
                        }

                        break;
                    }
                }
                /* look for last block */
                else if (log_blk_num + logseginfo->num_log_blk - 1 == seg_info->base_log_blk_num) {
                    log_blk_info = &port_info->log_blk_attr[seg_info->base_log_blk_num];

                    if (ocm_debug) {
                        LOG_CLI((BSL_META_U(unit,
                                            "Last Block   log_blk_info->phy_blk_num(%d)\n"),
                                 log_blk_info->phy_blk_num));
                    }

                    if (SOC_SBX_CALADAN3_OCM_PORT_INVALID != log_blk_info->phy_blk_num) {
                        reuse_log_blk_id = seg_info->base_log_blk_num;
                        free_phy_blk++;

                        if (ocm_debug) {
                            LOG_CLI((BSL_META_U(unit,
                                                "   reuse_log_blk_id(%d)  free_phy_blk(%d)\n"),
                                     reuse_log_blk_id, free_phy_blk));
                        }
                        break;
                    }
                }
            }
        } DQ_TRAVERSE_END(&port_info->log_addrmap_sorted_list, listelem);  

        for(index = port_info->min_blk; index <= port_info->max_blk; index++) {

            if (SOC_SBX_CALADAN3_OCM_PORT_INVALID == OCM_PHY_BLK(unit,index).port_num) {
                free_phy_blk++;
            }
            if (free_phy_blk >= seg_info->num_log_blk) 
                break;
        }

        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "Found at least free_phy_blks(%d)\n"),
                     free_phy_blk));
        }

        if (free_phy_blk < seg_info->num_log_blk) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: sbx_caladan3_ocm_port_segment_alloc: unit %d" 
                                  " OCM MEMORY Exhausted, no memory to support the segment %d \n"),
                       unit, desc->segment));

            /* invalidate the segment */
            /* unmap log-phy mapping if segment valid */
            _soc_sbx_caladan3_ocm_port_segment_log_map(unit, desc, FALSE);

#ifdef OCM_DEBUG
            sbx_caladan3_ocm_dump_allocator(unit, -1);
#endif
            return SOC_E_PARAM;

        } else {
            /* allocate */
            num_log_blk = seg_info->num_log_blk;
            log_blk_num = seg_info->base_log_blk_num;

            if (ocm_debug) {
                LOG_CLI((BSL_META_U(unit,
                                    "num_log_blk required(%d)  base log_blk_num(%d)\n"),
                         num_log_blk, log_blk_num));
            }

            if (SOC_SBX_CALADAN3_OCM_INVALID != reuse_log_blk_id) {
                num_log_blk--;
            }

            for(index = port_info->min_blk; index <= port_info->max_blk && num_log_blk > 0; index++) {

                if (SOC_SBX_CALADAN3_OCM_PORT_INVALID == OCM_PHY_BLK(unit,index).port_num) {
                    num_log_blk--;

                    if ((SOC_SBX_CALADAN3_OCM_INVALID != reuse_log_blk_id) &&
                        (log_blk_num == reuse_log_blk_id)) {
                        log_blk_num++;
                    } 

                    phy_blk_info = &OCM_PHY_BLK(unit,index);
                    phy_blk_info->log_blk_num = log_blk_num;
                    phy_blk_info->port_num = desc->port;

                    if (ocm_debug) {
                        LOG_CLI((BSL_META_U(unit,
                                            "***Phy BLK#(%d) phy_blk_info->log_blk_num(%d)  phy_blk_info->port_num(%d:%s)\n"),
                                 index, phy_blk_info->log_blk_num, phy_blk_info->port_num,
                                 soc_sbx_caladan3_ocm_util_port_num_enum_str(phy_blk_info->port_num)));
                    }

                    /* set association on logical blk if applicable */
                    log_blk_info = &port_info->log_blk_attr[log_blk_num++];

                    /* assert if the segment is not present on the log blk bitmap */
                    assert(SOC_PBMP_MEMBER(log_blk_info->seg_bitmap[desc->segment/
                                                                    SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT],
                                           desc->segment));

                    /* associate log blk to phy blk */
                    log_blk_info->phy_blk_num = index;

                    if (ocm_debug) {
                        LOG_CLI((BSL_META_U(unit,
                                            "Associate LOG BLK(%d) --> PHY BLK(%d)\n"),
                                 phy_blk_info->log_blk_num, log_blk_info->phy_blk_num));
                    }
                }
            }
            if (num_log_blk > 0) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: sbx_caladan3_ocm_port_segment_alloc: unit %d" 
                                      "OCM MEMORY Exhausted, no memory to support the segment %d \n"),
                           unit, desc->segment));
                
#ifdef OCM_DEBUG
                sbx_caladan3_ocm_dump_allocator(unit, -1);
#endif
                return SOC_E_PARAM;
            }

        }
    } else {
        if (SOC_SBX_CALADAN3_OCM_INVALID == seg_info->valid) {
            status = SOC_E_DISABLED;
        } else {
            /* walk through logical blocks & free association */
            int segidx, segcount, count;
            num_log_blk = seg_info->num_log_blk;

            for(index = seg_info->base_log_blk_num; 
                index < seg_info->base_log_blk_num + seg_info->num_log_blk;
                index++) {

                log_blk_info = &port_info->log_blk_attr[index];  

                /* assert if the segment is not present on the log blk bitmap */
                assert(SOC_PBMP_MEMBER(
                           log_blk_info->seg_bitmap[desc->segment/SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT],
                           desc->segment));     

                assert(log_blk_info->phy_blk_num >= 0); 
                /* coverity[negative_returns: FALSE] */
                phy_blk_info = &OCM_PHY_BLK(unit,log_blk_info->phy_blk_num);  
                assert(phy_blk_info->port_num == desc->port);
                assert(phy_blk_info->log_blk_num == index);

                for (segidx=0,segcount=0, count=0; segidx < SOC_SBX_CALADAN3_OCM_NUM_MEM; segidx++) {
                    SOC_PBMP_COUNT(log_blk_info->seg_bitmap[segidx], count);
                    segcount += count;
                }

                if (segcount <= 1) {
                    phy_blk_info->port_num = SOC_SBX_CALADAN3_OCM_PORT_INVALID;    
                    phy_blk_info->log_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
                    /* unlink logical blk to physical relation if this is last segment */
                    log_blk_info->phy_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
                }
            }
        }
    }

    return status;
}

STATIC 
int _soc_sbx_caladan3_ocm_common_validate_segment_desc(int unit,
                                                       sbx_caladan3_ocm_port_alloc_t *desc,
                                                       int alloc /*1-alloc, 0-free*/)
{
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;
    int align;

    if (desc->port<0 || desc->port >= SOC_SBX_CALADAN3_MAX_OCM_PORT) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_caladan3_ocm_port_segment_alloc: unit %d bad ocm port %d \n"),
                   unit, desc->port));
        return SOC_E_PARAM;
    }

    /* Segment 0 is used for DMA (except on BUBBLE_PORT) 
     * DMA will overwrite reserved segment configurationi
     */
    if ((desc->segment == _SOC_SBX_CALADAN3_DMA_SEGMENT) &&
        (desc->port != SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_caladan3_ocm_port_segment_alloc: unit %d segment 0 reserved for dma \n"),
                   unit));
        return SOC_E_PARAM;
    }

    port_info = &OCM_PORTCB(unit,desc->port);

    if (port_info->num_segment < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "_soc_sbx_caladan3_ocm_common_validate_segment_desc: unit %d"
                              " no segmentation available on this port %d \n"),
                   unit, desc->port));
        return SOC_E_PARAM;
    }

    if (desc->segment < 0 || desc->segment > port_info->num_segment-1) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "_soc_sbx_caladan3_ocm_common_validate_segment_desc: unit %d invalid segment %d \n"),
                   unit, desc->segment));
        return SOC_E_PARAM;
    }

    seg_info = &port_info->seg_attr[desc->segment];

    if((alloc > 0) && (seg_info->valid != SOC_SBX_CALADAN3_OCM_INVALID)) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "_soc_sbx_caladan3_ocm_common_validate_segment_desc: unit %d segment %d in use \n"),
                  unit, desc->segment));
        return SOC_E_PARAM;
    }

    if((alloc == 0) && (seg_info->valid == SOC_SBX_CALADAN3_OCM_INVALID)) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "_soc_sbx_caladan3_ocm_common_validate_segment_desc: "
                             "unit %d segment %d to be freed is invalid \n"),
                  unit, desc->segment));
        return SOC_E_PARAM;
    }

    /* If table size does not alighn to 64bit boundary, alighn it */
    align = desc->size * desc->datum_size;
    if (align % 64 > 0) {

        while (align % 64) {
            align += desc->datum_size;
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s: unit %d Align table to 64bit boundary Requested Size: %d Aligned Size: %d\n"),
                     FUNCTION_NAME(), unit, desc->size, align/desc->datum_size));
        desc->size = align / desc->datum_size;
    }

    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_ocm_port_segment_init
 *   Purpose
 *      Drivier initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_port_segment_alloc(int unit, 
                                            sbx_caladan3_ocm_port_alloc_t *desc)
{
    int status = SOC_E_NONE;
    sbx_caladan3_ocm_port_info_t *port_info;

    SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_ocm_common_validate_segment_desc(unit, desc, TRUE));

    if (desc->datum_size < SOC_SBX_CALADAN3_DATUM_SIZE_BIT ||
        desc->datum_size >= SOC_SBX_CALADAN3_DATUM_SIZE_MAX) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "sbx_caladan3_ocm_port_segment_alloc: unit %d dataun size unsupported %d \n"),
                  unit, desc->datum_size));
        return SOC_E_PARAM;
    }

    port_info = &OCM_PORTCB(unit,desc->port);

    /* verify the size to logical space availability */
    if (desc->size * desc->datum_size > (SOC_SBX_CALADAN3_OCM_PORT_NUM_BLK(unit, desc->port) * 
                                         SOC_SBX_CALADAN3_OCM_ADDR_PER_BLK)){
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\n%s: UNIT:%d No Space for Segment Allocation SzReq:%d DatumSz:%d LRP_Port:%d MxBlk:%d, MnBlk:%d\n"),
                 FUNCTION_NAME(), unit, desc->size, desc->datum_size, desc->port, 
                 port_info->max_blk, port_info->min_blk));

        return SOC_E_PARAM;
    }


    status = _soc_sbx_caladan3_ocm_port_segment_log_map(unit,desc, TRUE); 

    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_caladan3_ocm_port_segment_alloc: unit %d no" 
                              " Failed to allocate logical address space for segment %d \n"),
                   unit, desc->segment));
        return SOC_E_PARAM;
    } else {
        /* Allocate physical blocks for the logical port */
        status = _soc_sbx_caladan3_ocm_port_segment_phy_blk_alloc_free(unit, desc, TRUE);

        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: sbx_caladan3_ocm_port_segment_alloc: unit %d OCM MEMORY Exhausted" 
                                  " Failed to allocate physical block for segment %d \n"),
                       unit, desc->segment));
        } else {

            status = _soc_sbx_caladan3_ocm_port_map_segment_phy_blk(unit, desc, TRUE);

            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "sbx_caladan3_ocm_port_segment_alloc: unit %d no" 
                                      " Failed to Map physical block for segment %d \n"),
                           unit, desc->segment));
            } else {
                status = _soc_sbx_caladan3_ocm_port_program_segment(unit, 
                                                                    desc->port,
                                                                    desc->segment, 
                                                                    TRUE);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "sbx_caladan3_ocm_port_segment_alloc: unit %d no" 
                                          " Failed to program segment table %d \n"),
                               unit, desc->segment));
                }
            }
        }
    }

    if (SOC_FAILURE(status)) {
        /* disable this segment anyways */
        _soc_sbx_caladan3_ocm_port_program_segment(unit,  
                                                   desc->port,
                                                   desc->segment, 
                                                   FALSE);
        /* unmap log-phy mapping if segment valid */
        _soc_sbx_caladan3_ocm_port_map_segment_phy_blk(unit, desc, FALSE);
        /* undo physical block allocation if segment valid */
        _soc_sbx_caladan3_ocm_port_segment_phy_blk_alloc_free(unit, desc, FALSE);
        /* undo logical block allocation if segment valid */
        _soc_sbx_caladan3_ocm_port_segment_log_map(unit,desc, FALSE); 

    }

    return status;
}

/*
 *   Function
 *     sbx_caladan3_ocm_port_segment_free
 *   Purpose
 *      Free pre-allocated segments
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_port_segment_free(int unit, 
                                           sbx_caladan3_ocm_port_alloc_t *desc)
{
    int status = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_ocm_common_validate_segment_desc(unit, desc, FALSE));

    /* unmap log-phy mapping if segment valid */
    status = _soc_sbx_caladan3_ocm_port_map_segment_phy_blk(unit, desc, FALSE);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ocm_port_segment_free: unit %d no" 
                              " Failed to program phy block table segment %d table \n"),
                   unit, desc->segment));
    }

    /* undo physical block allocation if segment valid */
    status = _soc_sbx_caladan3_ocm_port_segment_phy_blk_alloc_free(unit, desc, FALSE);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ocm_port_segment_free: unit %d no" 
                              " Failed to free back physical blocks for segment %d \n"),
                   unit, desc->segment));
    }

    /* disable this segment anyways */
    status = _soc_sbx_caladan3_ocm_port_program_segment(unit, 
                                                        desc->port,
                                                        desc->segment, 
                                                        FALSE);
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ocm_port_segment_free: unit %d no" 
                              " Failed to program segment %d table \n"),
                   unit, desc->segment));
    }

    /* undo logical block allocation if segment valid */
    status = _soc_sbx_caladan3_ocm_port_segment_log_map(unit,desc, FALSE); 
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ocm_port_segment_free: unit %d no" 
                              " Failed to free logical blocks for segment %d \n"),
                   unit, desc->segment));
    }
 
    return status;
}

/*
 *   Function
 *     sbx_caladan3_ocm_port_mem_alloc
 *   Purpose
 *      Allocates memory for ports which does not support segmentation
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_port_mem_alloc(int unit, 
                                        sbx_caladan3_ocm_port_alloc_t *desc)
{
    int status = SOC_E_NONE;
    sbx_caladan3_ocm_port_info_t *port_info;
    int log_blk_num=0, free_phy_blk=0, index=0;
    sbx_caladan3_ocm_phy_blk_info_t *phy_blk_info=NULL;

    if (!desc) {
        return SOC_E_PARAM; 
    }

    port_info = &OCM_PORTCB(unit,desc->port);
    if (port_info->num_segment >= 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_caladan3_ocm_port_mem_alloc: unit %d"
                              " ERROR: segmentation on this port %d, not supported \n"),
                   unit, desc->port));
        return SOC_E_PARAM; 
    }

    if (port_info->size > 0) {
        /* cannot have more than one logical address block without segmentation */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_caladan3_ocm_port_log_blk_alloc: unit %d"
                              " ERROR: Can not have more than one logical address block without segmentation %d \n"),
                   unit, desc->port));
        return SOC_E_PARAM;
    }

    log_blk_num = desc->size / SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK;

    if ((desc->size % SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK) > 0) {
        log_blk_num++;
    }

    for(index = port_info->min_blk; 
        index <= port_info->max_blk && log_blk_num > 0 && free_phy_blk < log_blk_num;
        index++) {

        if (SOC_SBX_CALADAN3_OCM_PORT_INVALID == OCM_PHY_BLK(unit,index).port_num) {
            free_phy_blk++;
        }
    }

    if (free_phy_blk >= log_blk_num) {
        sbx_caladan3_ocm_log_blk_info_t *log_blk_info;

        for(index = port_info->min_blk, log_blk_num=0;
            index <= port_info->max_blk && free_phy_blk > 0; index++) {

            if (SOC_SBX_CALADAN3_OCM_PORT_INVALID == OCM_PHY_BLK(unit,index).port_num) {

                   phy_blk_info = &OCM_PHY_BLK(unit,index);
                   phy_blk_info->log_blk_num = log_blk_num;
                   phy_blk_info->port_num = desc->port;

                   if (ocm_debug) {
                       LOG_CLI((BSL_META_U(unit,
                                           "\n\n==========Phy BLK#(%d)  phy_blk_info->log_blk_num(%d)   phy_blk_info->port_num(%d:%s)\n"),
                                index, phy_blk_info->log_blk_num, phy_blk_info->port_num, 
                                soc_sbx_caladan3_ocm_util_port_num_enum_str(desc->port)));
                   }
                   
                   free_phy_blk--; /*using this as allocated count */
                   
                   /* associate log blk to phy blk */
                   log_blk_info = &port_info->log_blk_attr[log_blk_num++];
                   log_blk_info->phy_blk_num = index;
            }
        }
        if (index > port_info->max_blk) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: idx Exceeded. sbx_caladan3_ocm_port_mem_alloc: unit %d"
                                  " OCM MEMORY Exhausted NO free physical blocks for memory allocation"
                                  " requested [%d] available[%d] for port %d \n"),
                       unit, log_blk_num, free_phy_blk, desc->port));

#ifdef OCM_DEBUG
            sbx_caladan3_ocm_dump_allocator(unit, -1);
#endif

            return SOC_E_MEMORY;
        }

        status = _soc_sbx_caladan3_ocm_port_map_phy_blk(unit, desc, TRUE);

    } else {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "ERROR: sbx_caladan3_ocm_port_mem_alloc: unit %d"
                             " OCM MEMORY Exhausted NO free physical blocks for memory allocation"
                             " requested [%d] available[%d] for port %d \n"),
                  unit, log_blk_num, free_phy_blk, desc->port));
#ifdef OCM_DEBUG
            sbx_caladan3_ocm_dump_allocator(unit, -1);
#endif

       status = SOC_E_MEMORY;        
    }

    if (SOC_SUCCESS(status)) {
        port_info->size = desc->size;
        port_info->datum_size = desc->datum_size;
    }

    return status;
}

/*
 *   Function
 *     sbx_caladan3_ocm_port_mem_free
 *   Purpose
 *      free memory for ports which does not support segmentation
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_port_mem_free(int unit, 
                                       sbx_caladan3_ocm_port_alloc_t *desc)
{
    int status = SOC_E_NONE;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_log_blk_info_t *log_blk_info;
    sbx_caladan3_ocm_phy_blk_info_t *phy_blk_info=NULL;
    int log_blk_num=0, index=0;

    if (!desc) {
        return SOC_E_PARAM; 
    }

    port_info = &OCM_PORTCB(unit,desc->port);
    if (port_info->num_segment >= 0) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "sbx_caladan3_ocm_port_mem_alloc: unit %d"
                             " segmentation available on this port %d, not supported \n"),
                  unit, desc->port));
    }

    if (port_info->size <= 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "sbx_caladan3_ocm_port_log_blk_alloc: unit %d"
                              " no memory allocated to free for port %d \n"),
                   unit, desc->port));
        return SOC_E_PARAM;
    } 

    log_blk_num = port_info->size / SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK;

    if (port_info->size % SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK > 0) {
        log_blk_num ++;
    }

    status = _soc_sbx_caladan3_ocm_port_map_phy_blk(unit, desc, FALSE);

    for(index=0; index < log_blk_num && SOC_SUCCESS(status); index++) {
        log_blk_info = &port_info->log_blk_attr[index];

        assert(log_blk_info->phy_blk_num >= 0);
        /* coverity[negative_returns: FALSE] */
        phy_blk_info = &OCM_PHY_BLK(unit,log_blk_info->phy_blk_num);

        phy_blk_info->log_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
        phy_blk_info->port_num = SOC_SBX_CALADAN3_OCM_PORT_INVALID;
        log_blk_info->phy_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
    }

    port_info->size = 0 ;

    return status;
}

/* Read write interface to OCM */

STATIC 
int _soc_sbx_caladan3_ocm_common_validate_mem_access(int unit,
                                                     sbx_caladan3_ocm_port_e_t port, int segment,
                                                     int index_min, int index_max)

{
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;
    int range = index_max - index_min;

    if (port<0 || port >= SOC_SBX_CALADAN3_MAX_OCM_PORT) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "_soc_sbx_caladan3_ocm_common_validate_mem_access: unit %d bad ocm port %d \n"),
                   unit, port));
        return SOC_E_PARAM;
    }

    port_info = &OCM_PORTCB(unit,port);

    /* segmentation supported on this port */
    if (port_info->num_segment < 0) {
        if (segment >= 0) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "_soc_sbx_caladan3_ocm_common_validate_mem_access: unit %d"
                                 " no segmentation available on this port %d \n"),
                      unit, (int)port));
            /* ignore segment */
        }
        if (range < 0 || range >= port_info->size || 
            index_min < 0 || index_min >= port_info->size || 
            index_max < 0 || index_max >= port_info->size) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ocm_common_validate_mem_access: unit %d"
                                  " Max index supported is 0x%x \n"),
                       unit,port_info->size));
            return SOC_E_PARAM;
        }
    } else {
        if (segment < 0 || segment > port_info->num_segment-1) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ocm_common_validate_mem_access: unit %d invalid segment %d \n"),
                       unit, segment));
            return SOC_E_PARAM;
        }

        seg_info = &port_info->seg_attr[segment];
        
        if(seg_info->valid == SOC_SBX_CALADAN3_OCM_INVALID) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ocm_common_validate_mem_access: unit %d invalid segment %d \n"),
                       unit, segment));
            return SOC_E_PARAM;
        }

        /* index are table entry index, agnostic of size */
        if (range < 0 || range >= seg_info->size ||
            index_min < 0 || index_min >= seg_info->size || 
            index_max < 0 || index_max >= seg_info->size) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_soc_sbx_caladan3_ocm_common_validate_mem_access: unit %d " 
                                  "segment %d invalid index-min %d index-max %d range 0 - %d " 
                                  "\n log address base %u max logical address space %d\n"),
                       unit, segment, index_min, index_max,
                       seg_info->size, seg_info->log_addr_base,
                       seg_info->log_addr_base + seg_info->size -1));
            return SOC_E_PARAM;
        }
    }

    return SOC_E_NONE;
}




#define OCM_MAX_INDEX_VAL (num_quad_words * SOC_SBX_OCM_MAX_ENTRY_NUM_WORDS)

/*
 * OCM DMA
 * Note: OCM memory is BIG-Endian and this function reads it as such.
 * If the "Host" application is using Little-Endian processor swapping will 
 * be required.
 */
STATIC 
int _soc_sbx_caladan3_ocm_port_mem_dma_read_write(int unit, 
                                                  sbx_caladan3_ocm_port_e_t port,
                                                  int segment,
                                                  int index_min,
                                                  int index_max,
                                                  uint32 *entry_data,
                                                  uint8 write /*0-read, 1-write*/)
{
    uint32 regval=0;
    int quad_index_min = index_min, quad_index_max = index_max;
    int entry_count, index, num_quad_words=0;
    int index_min_off = 0;
    int index_max_off = 0;
    int entry_size=0, buf_idx=0, rv=SOC_E_NONE;
    uint32 *dmabuf;
    sbx_caladan3_ocm_port_info_t *port_info=NULL;
    sbx_caladan3_ocm_segment_info_t *seg_info=NULL, *dma_seg_info=NULL;
    int seg_is_64b = 1;

    /*This debug info will be used in next version*/
    /*uint32 tempdmabuf[20] = {0};*/ /* XXXDDD */

    /*int dbg_idx;
      int orig_entry_cnt;*/

    entry_count = (index_max > index_min) ? 
        (index_max - index_min + 1) : 
        (index_min - index_max + 1);

    /*This debug info will be used in next version*/
    /*orig_entry_cnt = entry_count;*/ /* XXXDDD*/

    port_info = &OCM_PORTCB(unit,port);

    if (port_info->num_segment < 0) {
        entry_size = SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY;
    } else {
        seg_info = &port_info->seg_attr[segment];
        entry_size = seg_info->datum_size;
        seg_is_64b = (seg_info->datum_size == SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD);
    }

    /* number of 64 bit entries required */
    num_quad_words = (entry_count * entry_size)/SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY;
    if ((entry_count * entry_size) % SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY) {
        num_quad_words++;
    }
    if (entry_size != SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY && (index_min % 2)) {
        num_quad_words++;
    }


    if (ocm_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "\n%s  num_Qwords:%d entry_count:%d entry_size:%d  min:%d  max%d\n"), 
                 FUNCTION_NAME(), num_quad_words, entry_count, entry_size,
                 index_min, index_max));
    }

    /* for now allocate DMA buffer every time. 
     * This could be optimized to preallocate once & reused
     * Each OCM location, accessed from host, via DMA, is configured as 64b.
     * But we also need to account for extra 8b of ECC even if no error 
     * protection is configured.
     */
    dmabuf = soc_cm_salloc(unit, (OCM_MAX_INDEX_VAL) * sizeof(uint32),
                           "ocm dma buffer");

    if (!dmabuf) {
        return SOC_E_MEMORY;
    }
    sal_memset(dmabuf, 0, (OCM_MAX_INDEX_VAL) * sizeof(uint32));

    soc_reg_field_set(unit, OC_CONFIGr, &regval, SOFT_RESET_Nf, 1); 


    if (segment >= 0) {

        uint32 QuadDatumDivisor = 1;

        /* set up dma segment to the table info */
       
        seg_info = &port_info->seg_attr[segment];
        dma_seg_info = &port_info->seg_attr[_SOC_SBX_CALADAN3_DMA_SEGMENT];

        sal_memcpy(dma_seg_info, seg_info, sizeof(sbx_caladan3_ocm_segment_info_t));

        /*
         * We need to override the segment size and datum size since we are accessing
         * this existing segment via the DMA segment.  The DMA segment is dynamic
         * and overlays an existing segment.
         */

        QuadDatumDivisor = SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD / seg_info->datum_size;

        dma_seg_info->size = seg_info->size / QuadDatumDivisor;
        dma_seg_info->datum_size = SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD;

        index_min_off = index_min % QuadDatumDivisor;
        index_max_off = index_max % QuadDatumDivisor;

        quad_index_min = index_min / QuadDatumDivisor;
        quad_index_max = index_max / QuadDatumDivisor;
        
        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "\n quad_index_min:%d   quad_index_max:%d\n"), 
                     quad_index_min, quad_index_max));
        }
        
#ifdef BCM_WARM_BOOT_SUPPORT    
	if(SOC_WARM_BOOT(unit)) {
            SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(
               _soc_sbx_caladan3_ocm_port_program_segment(unit, port, 
							  _SOC_SBX_CALADAN3_DMA_SEGMENT,
							  TRUE),rv);
	    SOC_IF_ERROR_RETURN(rv);        
	}
	else {
            SOC_IF_ERROR_RETURN(
               _soc_sbx_caladan3_ocm_port_program_segment(unit, port, 
							  _SOC_SBX_CALADAN3_DMA_SEGMENT,
							  TRUE));	  
	}
#else
        SOC_IF_ERROR_RETURN(
           _soc_sbx_caladan3_ocm_port_program_segment(unit, port, 
                                                      _SOC_SBX_CALADAN3_DMA_SEGMENT,
                                                      TRUE));
#endif /* BCM_WARM_BOOT_SUPPORT */
        soc_reg_field_set(unit, OC_CONFIGr, &regval, DMA_PORT_SEGMENTf, _SOC_SBX_CALADAN3_DMA_SEGMENT); 
    } 

    soc_reg_field_set(unit, OC_CONFIGr, &regval, DMA_PORT_IDf, port); 
#ifdef BCM_WARM_BOOT_SUPPORT       
    if(SOC_WARM_BOOT(unit)) {
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_OC_CONFIGr(unit, regval),rv);
    }
    else {
        rv = WRITE_OC_CONFIGr(unit, regval);
    }
#else
    rv = WRITE_OC_CONFIGr(unit, regval);
#endif /* BCM_WARM_BOOT_SUPPORT */
    buf_idx=0;

    if (SOC_SUCCESS(rv)) {
        LOG_INFO(BSL_LS_SOC_DMA,
                 (BSL_META_U(unit,
                             "%s: unit %d OCM %s DMA buffer dump: port:%d index[%d-%d] \n"),
                  FUNCTION_NAME(), unit, 
                  (write)?"write":"read",
                  port, 0, num_quad_words));
        
        if (write > 0) {
            /* DMA WRITE */
            for (index = 0, buf_idx = 0;
                 index < OCM_MAX_INDEX_VAL;  ) {
                /* 
                 * Must perform a RMW cycle for first and last entry, 
                 * if they are not on a 64b boundary. The caller does not 
                 * know we are accessing OCM memory as 64b accesses and does
                 * NOT account for it.  This is an implementation detail we must
                 * account for, IN the implementation.
                 */
                if (seg_is_64b) {
                    dmabuf[index]     = entry_data[buf_idx];
                    dmabuf[index + 1] = entry_data[buf_idx + 1];
                    dmabuf[index + 2] = 0; /* ECC */
                    
                    LOG_INFO(BSL_LS_SOC_DMA,
                             (BSL_META_U(unit,
                                         "0x%x 0x%x 0x%x "),
                              dmabuf[index], dmabuf[index+1], dmabuf[index+2]));
                    
                    buf_idx += 2;
                } else {

                    if (index == 0 
                        && index_min_off) {

                        /* Read FIRST Quadword (64b) */
                        rv = soc_mem_read_range(unit, OC_MEMORYm, MEM_BLOCK_ANY,
                                                quad_index_min, quad_index_min , &dmabuf[index]);

                        /* Since index == 0 for this case entry_count should be 
                         * non-zero but sanity check
                         */
                        if (entry_count) {
                            /* Modify only MSLW of 64b data */
                            dmabuf[index] = entry_data[buf_idx];
                        
                            buf_idx++;
                            entry_count--;
                        
                        } else {
                            break;
                        }
 
                    } else if (index == (OCM_MAX_INDEX_VAL - SOC_SBX_OCM_MAX_ENTRY_NUM_WORDS) 
                               && !(index_max_off) ) {

                        /* Read LAST Quadword (64b) */
                        rv = soc_mem_read_range(unit, OC_MEMORYm, MEM_BLOCK_ANY,
                                                quad_index_max, quad_index_max , &dmabuf[index]);

                        if (entry_count) {
                            /* Modify only LSLW of 64b data */
                            dmabuf[index + 1] = entry_data[buf_idx];
                        
                            buf_idx++;
                            entry_count--;
                        
                        } else {
                            break;
                        }

                    } else {

                        if (entry_count) {

                            dmabuf[index + 1] = entry_data[buf_idx];

                            buf_idx++;
                            entry_count--;
                    
                        } else {
                            break;
                        }

                        if (entry_count) {

                            dmabuf[index] = entry_data[buf_idx];
                    
                            buf_idx++;;
                            entry_count--;                    
                        }
                        else {
                            break;
                        }
                    }
                }

                index += SOC_SBX_OCM_MAX_ENTRY_NUM_WORDS;

                LOG_INFO(BSL_LS_SOC_DMA,
                         (BSL_META_U(unit,
                                     "0x%x 0x%x 0x%x "),
                          dmabuf[index], dmabuf[index+1], dmabuf[index+2]));

            }

            if (quad_index_max >= quad_index_min) {
                /*    coverity[negative_returns : FALSE]    */
                rv = soc_mem_write_range(unit, OC_MEMORYm, MEM_BLOCK_ANY,
                                         quad_index_min, quad_index_max , dmabuf);
            }

            /*This debug info will be used in next version*/

            /*for (dbg_idx = 0; dbg_idx < 20 && dbg_idx < orig_entry_cnt; dbg_idx++) {
                tempdmabuf[dbg_idx] = entry_data[dbg_idx];
                }*/ /*XXXDDD */
            
        } else {  /* DMA READ */
            rv = soc_mem_read_range(unit, OC_MEMORYm, MEM_BLOCK_ANY,
                                    quad_index_min, quad_index_max , dmabuf);
            if (SOC_SUCCESS(rv)) {

                for (index = 0, buf_idx = 0;
                     index < (OCM_MAX_INDEX_VAL) ;  ) {

                    /* Don't ever overrun user supplied buffer! This handles case of 32b access
                     * from user with odd number of 32b longwords being accessed.
                     * Of course, the following algorithms depend on the user-supplied
                     * buffer being a 32b quantity. So entry_data parameter should NOT be
                     * changed unless one is willing to rework this algorithm.
                     * 
                     * The below algorithm accounts for both EVEN numbered start indices
                     * (read sequence L,M,L,M...)
                     * AND ODD numbered start indices(read sequence M,L,M,L...). 
                     * BOTH must be considered!
                     */

                    /* From HW Spec:
                     * The OCM uses big-endian addressing; e.g. longword location
                     * 0 is in the most-significant portion of the quadword, 
                     * longword location 1 is in the least-significant portion.
                     */
                    if (seg_is_64b) {
                        entry_data[buf_idx] = dmabuf[index];
                        entry_data[buf_idx + 1] = dmabuf[index + 1];
                        buf_idx += 2;

                    } else {

                        if (entry_count) {

                            /* Are we starting with "odd" 32b index or "even" */
                            if (index_min_off) {
                                entry_data[buf_idx] = dmabuf[index];
                            } else {
                                entry_data[buf_idx] = dmabuf[index + 1];
                            }

                            buf_idx++;
                            entry_count--;

                        } else {
                            break;
                        }

                        if (entry_count) {

                            if (index_min_off) {
                                entry_data[buf_idx] = dmabuf[index + SOC_SBX_OCM_MAX_ENTRY_NUM_WORDS + 1];
                            } else {
                                entry_data[buf_idx] = dmabuf[index];
                            }
                            
                            buf_idx++;;
                            entry_count--;
                            
                        } else {
                            break;
                        }
                    }

                    index += SOC_SBX_OCM_MAX_ENTRY_NUM_WORDS;

                   LOG_INFO(BSL_LS_SOC_DMA,
                            (BSL_META_U(unit,
                                        "0x%x 0x%x 0x%x "),
                             dmabuf[index], dmabuf[index+1], dmabuf[index+2]));

                }
                /*This debug info will be used in next version*/
                /*for (dbg_idx = 0; dbg_idx < 20 && dbg_idx < orig_entry_cnt; dbg_idx++) {
                    tempdmabuf[dbg_idx] = entry_data[dbg_idx];
                    }*/ /*XXXDDD */
                
            }

        }
        LOG_INFO(BSL_LS_SOC_DMA,
                 (BSL_META_U(unit,
                             "\n")));
    }

    soc_cm_sfree(unit,dmabuf);
    if (segment >= 0) {
        _soc_sbx_caladan3_ocm_port_program_segment(unit, port, 
                                                   _SOC_SBX_CALADAN3_DMA_SEGMENT,
                                                   FALSE);
    }
    return rv;
}





STATIC 
int _soc_sbx_caladan3_ocm_port_mem_read_write(int unit, 
                                              sbx_caladan3_ocm_port_e_t port,
                                              int segment,
                                              int index_min,
                                              int index_max,
                                              uint32 *entry_data,
                                              uint8 write /*0-read, 1-write*/)
{
    uint32 regval=0, entry_size = 0;
    int count, index;
    sbx_caladan3_ocm_port_info_t *port_info = NULL;
    sbx_caladan3_ocm_segment_info_t *seg_info = NULL;
#ifdef BCM_WARM_BOOT_SUPPORT
    int rv = SOC_E_NONE;
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (entry_data == NULL) {
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(
         _soc_sbx_caladan3_ocm_common_validate_mem_access(unit, port, segment,
                                                          index_min, index_max));
    
    count = (index_max > index_min) ? 
            (index_max - index_min + 1) : 
            (index_min - index_max + 1);

    /* always keep soft reset 1 or the block will go for a reset */
    soc_reg_field_set(unit, OC_CONFIGr, &regval, SOFT_RESET_Nf, 1); 

    port_info = &OCM_PORTCB(unit,port);
    if (port_info->num_segment < 0) {
        entry_size = SOC_SBX_CALADAN3_OCM_BITS_PER_ENTRY;
    } else {
        if (segment < 0 || segment > port_info->num_segment-1) {
          LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s: invalid segment %d \n"), FUNCTION_NAME(), segment));
          return SOC_E_PARAM;
        }
        seg_info = &port_info->seg_attr[segment];
        assert(seg_info->valid != SOC_SBX_CALADAN3_OCM_INVALID);
        entry_size = seg_info->datum_size;
    }


    if (count >= ocmState[unit].dma_threshold) {
        return _soc_sbx_caladan3_ocm_port_mem_dma_read_write(unit, port, segment,
                                                             index_min, index_max,
                                                             entry_data, write);
    } else {

        /* Treat as logical access. For LRP Port access. */
        if (segment >= 0) {
            soc_reg_field_set(unit, OC_CONFIGr, &regval, PROC_PORT_SEGMENTf, segment); 
        }

        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "### %s Port(%d) Segment(%d) Index min:max(%d:%d) R/W:%s  \n"),
                     FUNCTION_NAME(), port, segment, index_min, index_max,
                     (write)?"write":"read"));
        }
	
        soc_reg_field_set(unit, OC_CONFIGr, &regval, PROC_PORT_IDf, port);        

#ifdef BCM_WARM_BOOT_SUPPORT
        if(SOC_WARM_BOOT(unit)) {
	    SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_OC_CONFIGr(unit, regval), rv);
            SOC_IF_ERROR_RETURN(rv);
	}
        else {
            SOC_IF_ERROR_RETURN(WRITE_OC_CONFIGr(unit, regval));
	}
#else
        SOC_IF_ERROR_RETURN(WRITE_OC_CONFIGr(unit, regval));
#endif /* BCM_WARM_BOOT_SUPPORT */


        for (index=index_min; index <= index_max; index++) {
            /* A proper fix needs to handle all the various entry sizes. However since
               the debugger is the only one requesting for multliple data entries, which is 64-bit
               per entry, we will assume it's 64-bit. */
            uint32 *p = (uint32 *)entry_data + (index - index_min) * 2;
            if (write > 0) {
                SOC_IF_ERROR_RETURN(WRITE_OC_MEMORYm(unit, MEM_BLOCK_ANY,index,p));
            } else {
                uint32 data[3];
                if (64 == entry_size) {

                    SOC_IF_ERROR_RETURN(READ_OC_MEMORYm(unit, MEM_BLOCK_ANY, index, data));
                    *p++ = data[0];
                    *p = data[1];
                } else {
                    SOC_IF_ERROR_RETURN(READ_OC_MEMORYm(unit, MEM_BLOCK_ANY, index, data));
                    *p = data[0];
                }
            }

            if (ocm_debug) {
                uint8 *chPtr = (uint8 *)entry_data;
                LOG_CLI((BSL_META_U(unit,
                                    "\nW/R:%s  entry_data:0x%08x  chPt[0]:0x%x chPt[1]:0x%x chPt[2]:0x%x chPt[3]:0x%x\n"), 
                         (write)?"write":"read", 
                         entry_data[index - index_min], chPtr[0], chPtr[1], chPtr[2], chPtr[3]));
            }


        }

        if (ocm_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
    return SOC_E_NONE;
}

/*
 *   Function
 *     sbx_caladan3_ocm_port_mem_read
 *   Purpose
 *      read memory table on a given port 
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_port_mem_read(int unit, sbx_caladan3_ocm_port_e_t port, int segment,
                                       int index_min, int index_max, uint32 *entry_data)
{
    return _soc_sbx_caladan3_ocm_port_mem_read_write(unit, port, segment,
                                                     index_min, index_max,
                                                     entry_data, 0);
}

/*
 *   Function
 *     sbx_caladan3_ocm_port_mem_read
 *   Purpose
 *      read memory table on a given port 
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_port_mem_write(int unit, sbx_caladan3_ocm_port_e_t port, int segment,
                                        int index_min, int index_max, uint32 *entry_data)
{
    return _soc_sbx_caladan3_ocm_port_mem_read_write(unit, port, segment,
                                                     index_min, index_max,
                                                     entry_data, 1);
}

/* Alloc, free Segment ID ?? */

/*
 *   Function
 *     sbx_caladan3_ocm_hw_init
 *   Purpose
 *      OCM hardware initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_hw_init(int unit) 
{
    int status = SOC_E_NONE, index;
    uint32 regval=0;

    /*READ_OC_CONFIGr(unit, &regval) ;*/
    soc_reg_field_set( unit, OC_CONFIGr, &regval, SOFT_RESET_Nf, 1 ); 
    SOC_IF_ERROR_RETURN(WRITE_OC_CONFIGr(unit, regval));

    
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_DATA0r(unit, 0));
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_DATA1r(unit, 0));
    /* only used when proc raw is enabled */
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_DATA2r(unit, 0));

    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_ENABLE0r(unit,SOC_SBX_CALADAN3_OCM_ENA_ALL_PHY_BLK));
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_ENABLE1r(unit,SOC_SBX_CALADAN3_OCM_ENA_ALL_PHY_BLK));
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_ENABLE2r(unit,SOC_SBX_CALADAN3_OCM_ENA_ALL_PHY_BLK));
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_ENABLE3r(unit,SOC_SBX_CALADAN3_OCM_ENA_ALL_PHY_BLK));

    /* no error protection for now */
    regval = 0;
    soc_reg_field_set(unit, OC_MEMORY_INIT_CONTROLr, &regval, INIT_ENABLEf, 1 ); 
    soc_reg_field_set(unit, OC_MEMORY_INIT_CONTROLr, &regval, INIT_GOf, 1 );
    SOC_IF_ERROR_RETURN(WRITE_OC_MEMORY_INIT_CONTROLr(unit, regval));

    regval = 0;
    soc_reg_field_set(unit, OC_SEGMENT_TABLE_INIT_CONTROLr, &regval, INIT_GOf, 1 );
    SOC_IF_ERROR_RETURN(WRITE_OC_SEGMENT_TABLE_INIT_CONTROLr(unit, regval));

    /* enable all interrupts */
    for (index=0; index < COUNTOF(ocm_interrupt) && SOC_SUCCESS(status); index++) {
        status = soc_sbx_caladan3_reg32_reset_val_get(unit, 
                                                      OCM_INTR_REG(index),
                                                      &regval, SOCF_W1TC);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s unit %d: failed(%d) to read default register value\n"),
                       FUNCTION_NAME(), unit, status));
        } else {
            status = soc_reg32_set(unit, OCM_INTR_REG(index),
                                   REG_PORT_ANY, 0, regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to clear interrupt !!!\n"),
                           FUNCTION_NAME(), unit, status));
            }
        }

        if (SOC_SUCCESS(status)) {        
            regval = OCM_INTR_ENABLE_VAL(index);
            status = soc_reg32_set(unit, OCM_INTR_MASK(index), REG_PORT_ANY, 0, regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to re-enable interrupt !!!\n"),
                           FUNCTION_NAME(), unit, status));
            }
        }
    }

    if (SOC_FAILURE(status)) {
        return status;
    }


    status = soc_cmicm_intr3_enable(unit, (1<<SOC_SBX_CALADAN3_OC_INTR_POS));
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to enable OCM block interrupt on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return status;
    }

    return status;
}

/*
 *   Function
 *     sbx_caladan3_ocm_driver_init
 *   Purpose
 *      Drivier initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_driver_init(int unit) 
{
    int status = SOC_E_NONE, index=0, segidx=0;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info;

    /* Clean up the driver before reinitializing */
    if (ocmState[unit].driver_init > 0)
        {
        status = soc_sbx_caladan3_ocm_driver_uninit(unit);
        if (status != SOC_E_NONE) {
            return status;
        }
    }

    for(index=0; index < SOC_SBX_CALADAN3_MAX_OCM_PORT; index++) {

        port_info = &OCM_PORTCB(unit,index);
        port_info->port_num = index;
        port_info->name     = ocm_port_name[index];
        DQ_INIT(&port_info->log_addrmap_sorted_list);
        port_info->num_segment_bits = 0;
        port_info->size = 0;

        switch(index) {
        case SOC_SBX_CALADAN3_OCM_LRP0_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP1_PORT:
            port_info->num_segment = SOC_SBX_CALADAN3_MAX_LRP_128_PORT_SEGMENT;
            port_info->num_segment_bits = 7;
            port_info->addr_width  = 27;
            port_info->min_blk     = 0;
            port_info->max_blk     = SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK-1;
            break;

        case SOC_SBX_CALADAN3_OCM_LRP2_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP3_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP4_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP5_PORT:
            port_info->num_segment = SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT;
            port_info->num_segment_bits = 6;
            port_info->addr_width  = 26;
            port_info->min_blk     = 0;
            port_info->max_blk     = SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM - 1;
            break;

        case SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT:
            port_info->num_segment = SOC_SBX_CALADAN3_MAX_LRP_BUBBLE_PORT_SEGMENT;
            port_info->addr_width = 20;
            port_info->min_blk     = 0;
            port_info->max_blk     = SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM - 1;
            break;

        case SOC_SBX_CALADAN3_OCM_CMU0_PORT:
        case SOC_SBX_CALADAN3_OCM_COP0_PORT:
            port_info->num_segment = -1;
            port_info->addr_width  = 20;
            port_info->min_blk     = 0;
            port_info->max_blk     = SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM - 1;
            break;

        case SOC_SBX_CALADAN3_OCM_LRP6_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP7_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP8_PORT:
        case SOC_SBX_CALADAN3_OCM_LRP9_PORT:
            port_info->num_segment = SOC_SBX_CALADAN3_MAX_LRP_64_PORT_SEGMENT;
            port_info->num_segment_bits = 6;
            port_info->addr_width  = 26;
            port_info->min_blk     = SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM;
            port_info->max_blk     = SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK-1;
            break;

        case SOC_SBX_CALADAN3_OCM_CMU1_PORT:
        case SOC_SBX_CALADAN3_OCM_COP1_PORT:
            port_info->num_segment = -1;
            port_info->addr_width  = 20;
            port_info->min_blk     = SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM;
            port_info->max_blk     = SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK-1;
            break;

	/* coverity[dead_error_begin] */
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "sbx_caladan3_ocm_driver_init: unit %d bad ocm port type\n"),
                       unit));
            return SOC_E_INTERNAL;
            break;
        }

        port_info->log_blk_attr = sal_alloc(sizeof(sbx_caladan3_ocm_log_blk_info_t) * 
                                                  (port_info->max_blk - port_info->min_blk + 1),
                                                 "ocm segment log to phy blk mapping");
        if (!port_info->log_blk_attr) {
            sal_free(port_info->seg_attr);
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "sbx_caladan3_ocm_driver_init: unit %d low memory type\n"),
                       unit));
            status = SOC_E_MEMORY;
            break;
        } else {
            sal_memset(port_info->log_blk_attr, 0, 
                       sizeof(sbx_caladan3_ocm_log_blk_info_t)  * 
                             (port_info->max_blk - port_info->min_blk + 1));            
            for (segidx=0; segidx < port_info->num_segment; segidx++) {
                port_info->log_blk_attr[segidx].phy_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
            }
        }

        if (port_info->num_segment > 0) {
            port_info->seg_attr = sal_alloc(sizeof(sbx_caladan3_ocm_segment_info_t) * \
                                            port_info->num_segment, "ocm seg attribute");
        
            if (!port_info->seg_attr) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "sbx_caladan3_ocm_driver_init: unit %d low memory type\n"),
                           unit));
                status = SOC_E_MEMORY;
                break;
            } 
            
            for (segidx=0; segidx < port_info->num_segment; segidx++) {
                seg_info = &port_info->seg_attr[segidx];
                sal_memset(seg_info, 0, sizeof(sbx_caladan3_ocm_segment_info_t));
                seg_info->segment = segidx;
                seg_info->valid   = SOC_SBX_CALADAN3_OCM_INVALID;
            }
        }
    }

    for(index=0; index < SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK; index++) {
        sbx_caladan3_ocm_phy_blk_info_t *phy_blk_info = &OCM_PHY_BLK(unit,index);
        phy_blk_info->port_num    = SOC_SBX_CALADAN3_OCM_PORT_INVALID;
        phy_blk_info->log_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
    }

    if (SOC_E_MEMORY == status) {
        /* free up dynamic memories */
    }

    if (SOC_SUCCESS(status)) {
        ocmState[unit].driver_init = 1;
        ocmState[unit].dma_threshold = SOC_SBX_CALADAN3_OCM_DMA_THRESHOLD;
    }
    return status;
}

/*
 *   Function
 *     sbx_caladan3_ocm_driver_uninit
 *   Purpose
 *      OCM driver un-initializer
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
int soc_sbx_caladan3_ocm_driver_uninit(int unit) 
{
    int index;
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_phy_blk_info_t *phy_blk_info;

    /* Set the state of the driver to invalid */
    ocmState[unit].driver_init = SOC_SBX_CALADAN3_OCM_INVALID;

    /* Walk the OCM ports and clean up anything associated */
    for(index=0; index < SOC_SBX_CALADAN3_MAX_OCM_PORT; index++)
    {
        port_info = &OCM_PORTCB(unit,index);
        port_info->port_num = index;
        port_info->name     = NULL;
        port_info->num_segment_bits = 0;
        port_info->num_segment = 0;
        port_info->addr_width  = 0;
        port_info->min_blk     = 0;
        port_info->max_blk     = 0;

        if(port_info->log_blk_attr != NULL)
            sal_free(port_info->log_blk_attr);

        if(port_info->seg_attr != NULL)
            sal_free(port_info->seg_attr);
      }

    for(index=0; index < SOC_SBX_CALADAN3_OCM_MAX_PHY_BLK; index++) {
        phy_blk_info = &OCM_PHY_BLK(unit,index);
        phy_blk_info->port_num    = SOC_SBX_CALADAN3_OCM_PORT_INVALID;
        phy_blk_info->log_blk_num = SOC_SBX_CALADAN3_OCM_INVALID;
     }

    return SOC_E_NONE;
}


uint32 ocm_total_alloc_size_in_bits[SOC_MAX_NUM_DEVICES] = {0};

/* There are SOC_SBX_CALADAN3_OCM_NUM_MEM (currently 2; Physical Blocks 0-63 & 64-127) 
 * of OCM Banks on the C3
 */
uint32 ocm_mem_bank_occupation[SOC_MAX_NUM_DEVICES][SOC_SBX_CALADAN3_OCM_NUM_MEM];
uint32 per_port_ocm_mem_bank_occupation[SOC_MAX_NUM_DEVICES][SOC_SBX_CALADAN3_MAX_OCM_PORT][SOC_SBX_CALADAN3_OCM_NUM_MEM];


/*
 *   Function
 *     sbx_caladan3_ocm_util_memmap_dump
 *   Purpose
 *      OCM memory allocator dump
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfully initialized
 *       BCM_E_* as appropriate otherwise
 */
void soc_sbx_caladan3_ocm_util_allocator_dump(int unit, 
                                              sbx_caladan3_ocm_port_e_t port) 
{
    sbx_caladan3_ocm_port_info_t *port_info;
    sbx_caladan3_ocm_segment_info_t *seg_info = NULL;
    sbx_caladan3_ocm_log_blk_info_t *log_blk_info;
    dq_p_t listelem;

    int index, pindex, pbmpidx, segidx, num_log_blk, print_term;

    uint32 port_alloc_size_in_bits = 0;
    int ocm_port_idx, ocm_mem_idx;

    float percent_occupied = 0;

    if (port >= SOC_SBX_CALADAN3_MAX_OCM_PORT) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s unit %d: Invalid port value: %d\n"),
                   FUNCTION_NAME(), unit, port));
        return;
    }

    for (ocm_port_idx = 0; ocm_port_idx <  SOC_SBX_CALADAN3_MAX_OCM_PORT; ocm_port_idx++) {
        for (ocm_mem_idx = 0; ocm_mem_idx < SOC_SBX_CALADAN3_OCM_NUM_MEM; ocm_mem_idx++) {
            per_port_ocm_mem_bank_occupation[unit][ocm_port_idx][ocm_mem_idx] = 0;
        }
    }

    /* if invalid port is specified dump all */
    for (pindex=0; pindex < SOC_SBX_CALADAN3_MAX_OCM_PORT; pindex++) {

        if (port > SOC_SBX_CALADAN3_OCM_PORT_INVALID && 
            port < SOC_SBX_CALADAN3_MAX_OCM_PORT && 
            port != pindex) {
            continue;
        }

        port_info = &OCM_PORTCB(unit,port);
        port_alloc_size_in_bits = 0;
        
        LOG_CLI((BSL_META_U(unit,
                            "\n\n\n=====================================================\n")));
        LOG_CLI((BSL_META_U(unit,
                            "OCM Allocator Dump for Port [%s] \n"), 
                 soc_sbx_caladan3_ocm_util_port_num_enum_str(port)));
        LOG_CLI((BSL_META_U(unit,
                            "=====================================================\n")));

        if (port_info->num_segment > 0) {
            int table_count = 0;
            int table_entry = 0;
            int bank_count = 0;
            int bank_entry = 0;
            uint32 segment_alloc_size_in_bits = 0;


            if (DQ_EMPTY(&port_info->log_addrmap_sorted_list)) {
                LOG_CLI((BSL_META_U(unit,
                                    " EMPTY - no allocations... \n")));
            } else {
                LOG_CLI((BSL_META_U(unit,
                                    "\n=============================\n")));
                LOG_CLI((BSL_META_U(unit,
                                    "Segments For This Port ")));
                LOG_CLI((BSL_META_U(unit,
                                    "\n=============================\n")));

                DQ_TRAVERSE(&port_info->log_addrmap_sorted_list, listelem) {

                    _SOC_SBX_OCM_GET_LOG_SEG_FROM_LIST(listelem, seg_info);

                    assert(seg_info->valid != SOC_SBX_CALADAN3_OCM_INVALID);

                    LOG_CLI((BSL_META_U(unit,
                                        "Segment ID [%d]\n"), seg_info->segment));

                    /* ID of ZERO is reserved to indicate not used by client */
                    SOC_PBMP_COUNT(seg_info->table_id, table_count);
                    if (table_count) {
                        SOC_PBMP_ITER(seg_info->table_id, table_entry) {
                            if (table_entry < _SOC_SBX_OCM_MAX_TABLE_ID) {
                                LOG_CLI((BSL_META_U(unit,
                                                    "\n   Table Id [%d]"), table_entry - 1)); 

                                SOC_PBMP_COUNT(seg_info->bank_id[table_entry], bank_count);
                            
                                if (bank_count) {
                                    SOC_PBMP_ITER(seg_info->bank_id[table_entry], bank_entry) {
                                        LOG_CLI((BSL_META_U(unit,
                                                            "\n       Bank Id [%d]"), bank_entry - 1)); 
                                    }
                                    
                                }
                                LOG_CLI((BSL_META_U(unit,
                                                    "\n")));
                            }
                        }
                    }

                    LOG_CLI((BSL_META_U(unit,
                                        "\n")));

                    LOG_CLI((BSL_META_U(unit,
                                        "   Allocation Request: Size [%d]  of Datum Size [%d] \n"), 
                             seg_info->size, seg_info->datum_size));

                    LOG_CLI((BSL_META_U(unit,
                                        "   Logical Address Range(Quadword) = 0x%x - 0x%x \n"), 
                             seg_info->log_addr_base >> SOC_SBX_CALADAN3_OCM_ENTRY_SHIFT, 
                             ((seg_info->log_addr_base +
                             seg_info->size * seg_info->datum_size) >> 
                             SOC_SBX_CALADAN3_OCM_ENTRY_SHIFT) - 1));

                    LOG_CLI((BSL_META_U(unit,
                                        "   Logical Address Range(Bits) = 0x%x - 0x%x \n"), 
                             seg_info->log_addr_base, 
                             (seg_info->log_addr_base +
                             seg_info->size * seg_info->datum_size)  - 1));
                    
                    segment_alloc_size_in_bits = ((seg_info->log_addr_base +
                                                   seg_info->size * seg_info->datum_size) - 1) -
                                                   seg_info->log_addr_base;


                    LOG_CLI((BSL_META_U(unit,
                                        "   Address Limit [0x%x]  R/W Entry/Transfer Size [%dbit]\n"), 
                             seg_info->size - 1, seg_info->datum_size));

                    LOG_CLI((BSL_META_U(unit,
                                        "   Base Log Block [%d]   Num Log Blocks [%d]\n\n"), 
                             seg_info->base_log_blk_num, seg_info->num_log_blk));            

                    LOG_CLI((BSL_META_U(unit,
                                        "    Segment Consumes [%d kb]  [%d KB] of OCM Memory\n"),
                             (segment_alloc_size_in_bits + 1023)/1024,
                             (((segment_alloc_size_in_bits + 1023)/1024) + 7)/8));

                    port_alloc_size_in_bits += segment_alloc_size_in_bits;

                    LOG_CLI((BSL_META_U(unit,
                                        "\n")));

                } DQ_TRAVERSE_END(&port_info->log_addrmap_sorted_list, listelem);  

                LOG_CLI((BSL_META_U(unit,
                                    "\n  %s Consumes [%d kb]  [%d KB] of OCM Memory\n"),
                         soc_sbx_caladan3_ocm_util_port_num_enum_str(port),
                         (port_alloc_size_in_bits + 1023)/1024, 
                         (((port_alloc_size_in_bits + 1023)/1024) + 7)/8));

                ocm_total_alloc_size_in_bits[unit] += port_alloc_size_in_bits; 


                LOG_CLI((BSL_META_U(unit,
                                    "\n===============================\n")));
                LOG_CLI((BSL_META_U(unit,
                                    "Log Block TO Phy Block Mapping")));
                LOG_CLI((BSL_META_U(unit,
                                    "\n===============================\n")));

                for (index=0; index < port_info->max_blk - port_info->min_blk + 1; index++) {
                    log_blk_info = &port_info->log_blk_attr[index];

                    if (log_blk_info->phy_blk_num != SOC_SBX_CALADAN3_OCM_INVALID) {
                        LOG_CLI((BSL_META_U(unit,
                                            "\nLB [%d] --> PB [%d]"), 
                                 index, log_blk_info->phy_blk_num));

                        
                        if (log_blk_info->phy_blk_num < SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM) {

                            /* XXXTBC This conditional (not statement in conditional) is a 
                               Temp for issue in BUBBLE port
                               allocations/frees somewhere! */
                            if (port_info->port_num != SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT ||
                                log_blk_info->phy_blk_num != 0) {

                                ocm_mem_bank_occupation[unit][0]++;
                                per_port_ocm_mem_bank_occupation[unit][port_info->port_num][0]++;
                            }
                            
                        } else if (log_blk_info->phy_blk_num >= SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM &&
                                   log_blk_info->phy_blk_num < 
                                   SOC_SBX_CALADAN3_OCM_NUM_MEM*SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM) {
                            
                            ocm_mem_bank_occupation[unit][1]++;
                            per_port_ocm_mem_bank_occupation[unit][port_info->port_num][1]++;
                        }
                        
                    }

                    for (pbmpidx=0, print_term = 0; pbmpidx < SOC_SBX_CALADAN3_OCM_NUM_MEM; pbmpidx++) {
                        SOC_PBMP_COUNT(log_blk_info->seg_bitmap[pbmpidx], segidx);
                        if (segidx > 0) {
                            print_term = 1;
                            LOG_CLI((BSL_META_U(unit,
                                                "\n    Log Blk Segments: [")));
                        }
                        SOC_PBMP_ITER(log_blk_info->seg_bitmap[pbmpidx], segidx) {
                            LOG_CLI((BSL_META_U(unit,
                                                "%d "),segidx));
                        }
                        if (print_term) {
                            LOG_CLI((BSL_META_U(unit,
                                                "]")));
                            print_term = 0;
                        }
                    }

                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n\n")));
                for (ocm_mem_idx = 0; ocm_mem_idx < SOC_SBX_CALADAN3_OCM_NUM_MEM; ocm_mem_idx++) {
                    
                    percent_occupied = (float)per_port_ocm_mem_bank_occupation[unit][port_info->port_num][ocm_mem_idx]/
                        (float)SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM;
                    
                    percent_occupied *= 100;
                    
                    LOG_CLI((BSL_META_U(unit,
                                        "  %s Consumes [%d PBs] [%.1f%%] of %d Total Available PBs for OCM MEM[%d] .\n"),
                             soc_sbx_caladan3_ocm_util_port_num_enum_str(port),
                             per_port_ocm_mem_bank_occupation[unit][port_info->port_num][ocm_mem_idx],
                             percent_occupied,
                             SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM, 
                             ocm_mem_idx));
                    
                }
                
            }
        } else {
            if (port_info->size > 0) {
                LOG_CLI((BSL_META_U(unit,
                                    "Non-SEGMENTED Logical Address Range(Bits) = 0x%x - 0x%x \n"),
                         0, port_info->size));

                LOG_CLI((BSL_META_U(unit,
                                    "\n===============================\n")));
                LOG_CLI((BSL_META_U(unit,
                                    "Log Block TO Phy Block Mapping")));
                LOG_CLI((BSL_META_U(unit,
                                    "\n===============================\n")));

                /* logical block dump */
                num_log_blk = port_info->size/SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK;
                if (port_info->size % SOC_SBX_CALADAN3_OCM_QUADWORD_PER_BLK) {
                    num_log_blk++;
                }
                for (index=0; index < num_log_blk; index++) {
                    log_blk_info = &port_info->log_blk_attr[index];
                    
                    if (log_blk_info->phy_blk_num != SOC_SBX_CALADAN3_OCM_INVALID) {
                        LOG_CLI((BSL_META_U(unit,
                                            " LB [%d] --> PB [%d]\n"), 
                                 index, log_blk_info->phy_blk_num));

                        if (log_blk_info->phy_blk_num < SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM) {

                            per_port_ocm_mem_bank_occupation[unit][port_info->port_num][0]++;
                            ocm_mem_bank_occupation[unit][0]++;

                        } else if (log_blk_info->phy_blk_num >= SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM &&
                            log_blk_info->phy_blk_num < 
                                   SOC_SBX_CALADAN3_OCM_NUM_MEM*SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM) {

                            per_port_ocm_mem_bank_occupation[unit][port_info->port_num][1]++;
                            ocm_mem_bank_occupation[unit][1]++;
                        }

                    }
                }


                LOG_CLI((BSL_META_U(unit,
                                    "\n\n")));
                for (ocm_mem_idx = 0; ocm_mem_idx < SOC_SBX_CALADAN3_OCM_NUM_MEM; ocm_mem_idx++) {
                    
                    percent_occupied = (float)per_port_ocm_mem_bank_occupation[unit][port_info->port_num][ocm_mem_idx]/
                        (float)SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM;
                    
                    percent_occupied *= 100;
                    
                    LOG_CLI((BSL_META_U(unit,
                                        "  %s Consumes [%d PBs] [%.1f%%] of %d Total Available PBs for OCM MEM[%d] .\n"),
                             soc_sbx_caladan3_ocm_util_port_num_enum_str(port),
                             per_port_ocm_mem_bank_occupation[unit][port_info->port_num][ocm_mem_idx],
                             percent_occupied,
                             SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM, 
                             ocm_mem_idx));
                    
                }


                port_alloc_size_in_bits = port_info->size * port_info->datum_size;

                LOG_CLI((BSL_META_U(unit,
                                    "\n  %s Consumes [%d kb]  [%d KB] of OCM Memory\n"),
                         soc_sbx_caladan3_ocm_util_port_num_enum_str(port),
                         (port_alloc_size_in_bits + 1023)/1024, 
                         (((port_alloc_size_in_bits + 1023)/1024) + 7)/8));
                

                ocm_total_alloc_size_in_bits[unit] += port_alloc_size_in_bits;

            } else {
                LOG_CLI((BSL_META_U(unit,
                                    " EMPTY - no allocation... \n")));
            }
        }

    }

}

/*
 *   Function
 *     soc_sbx_caladan3_ocm_isr
 *   Purpose
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) d1-4   : not used
 *   Returns
 *      VOID
 */
void soc_sbx_caladan3_ocm_isr(void *unit_vp,
                              void *d1, void *d2,
                              void *d3, void *d4)
{
    int unit = PTR_TO_INT(unit_vp);
    uint32 regval=0;
    uint32 statval=0;
    int status = SOC_E_NONE, index;
/*
    int block = 0;
    uint8 acc_type;
*/

    for (index=0; index < COUNTOF(ocm_interrupt); index++) {

        status = soc_reg32_get(unit, OCM_INTR_REG(index), REG_PORT_ANY, 0, &regval);

        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s unit %d: failed(%d) to read interrupt register: 0x%x\n"),
                       FUNCTION_NAME(), unit, status, OCM_INTR_REG(index)));

        } else if (regval > 0) {

            soc_sbx_caladan3_reg32_dump(unit, OCM_INTR_REG(index), regval);

	    /*
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s unit %d: OCM Error Interrupt register: 0x%x\n"),
                       FUNCTION_NAME(), unit, regval));
	    */

            /* Read the STATUS Regs */
	    if (OCM_INTR_STATUS_REG0(index) != INVALIDr) {
		statval = 0;
		soc_reg32_get(unit, OCM_INTR_STATUS_REG0(index), REG_PORT_ANY, 0, &statval);
		
		soc_sbx_caladan3_reg32_dump(unit, OCM_INTR_STATUS_REG0(index), statval);
		
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: OCM STATUS_0 Error Register: 0x%x  \n"),
                           FUNCTION_NAME(), unit, statval));
	    }

	    if (OCM_INTR_STATUS_REG1(index) != INVALIDr) {
		statval = 0;
		soc_reg32_get(unit, OCM_INTR_STATUS_REG1(index), REG_PORT_ANY, 0, &statval);
		
		soc_sbx_caladan3_reg32_dump(unit, OCM_INTR_STATUS_REG1(index), statval);
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: OCM STATUS_1 Error Register: 0x%x\n"),
                           FUNCTION_NAME(), unit, statval));
	    }

            /* unmask the interrupt */
            status = soc_sbx_caladan3_reg32_reset_val_get(unit, 
                                                          OCM_INTR_MASK(index),
                                                          &regval, 0);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to get unmask interrupt value !!!\n"),
                           FUNCTION_NAME(), unit, status));
            }

            status = soc_reg32_set(unit, OCM_INTR_MASK(index),
                                   REG_PORT_ANY, 0, regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to unmask interrupt !!!\n"),
                           FUNCTION_NAME(), unit, status));
            }

            /* clear the status register */
            status = soc_sbx_caladan3_reg32_reset_val_get(unit, 
                                                          OCM_INTR_REG(index),
                                                          &regval, SOCF_W1TC);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to read default register value\n"),
                           FUNCTION_NAME(), unit, status));
            }

            status = soc_reg32_set(unit, OCM_INTR_REG(index),
                                   REG_PORT_ANY, 0, regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to clear interrupt !!!\n"),
                           FUNCTION_NAME(), unit, status));
            }

            /* re-enbale the interrupt */
            regval = OCM_INTR_ENABLE_VAL(index);
            status = soc_reg32_set(unit, OCM_INTR_MASK(index), REG_PORT_ANY, 0, regval);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s unit %d: failed(%d) to re-enable interrupt !!!\n"),
                           FUNCTION_NAME(), unit, status));
            }

            regval = 0;
        }
    }

    status = soc_cmicm_intr3_enable(unit, (1<<SOC_SBX_CALADAN3_OC_INTR_POS));
    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s Failed to enable OCM block interrupt on unit %d\n"),
                   FUNCTION_NAME(), unit));
    }
}

#endif


