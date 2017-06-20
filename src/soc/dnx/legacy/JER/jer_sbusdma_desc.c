/*
 * $Id: jer2_jer_sbusdma_desc.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME
    #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_DMA
#include <soc/mem.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>

#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/sbusdma.h>

#include <soc/dnx/legacy/JER/jer_sbusdma_desc.h>
#include <sal/core/time.h>

#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/*#define JER2_JER_SBUSDMA_DESC_PRINTS_ENABLED*/

#define JER2_JER_KAPS_ARM_FIFO_SIZE_UINT32 (0x2000 + (SOC_REG_ABOVE_64_MAX_SIZE_U32 + 2))

/* } */

/*************
 *  MACROS   *
 *************/
/* { */


/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct { /* h/w desc structure */
    uint32 cntrl;    /* DMA control info */
    uint32 req;      /* DMA request info (refer h/w spec for details) */
    uint32 count;    /* DMA count */
    uint32 opcode;   /* Schan opcode (refer h/w spec for details) */
    uint32 addr;     /* Schan address */
    uint32 hostaddr; /* h/w mapped host address */
} soc_sbusdma_desc_t;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

static uint32 jer2_jer_sbusdma_desc_enabled[DNX_SAND_MAX_DEVICE];

/* Common memory double-buffer */
static uint32 *jer2_jer_sbusdma_desc_mem_buff_a[DNX_SAND_MAX_DEVICE];
static uint32 *jer2_jer_sbusdma_desc_mem_buff_b[DNX_SAND_MAX_DEVICE];
static uint32 *jer2_jer_sbusdma_desc_mem_buff_main[DNX_SAND_MAX_DEVICE];
static uint32 jer2_jer_sbusdma_desc_mem_buff_counter[DNX_SAND_MAX_DEVICE];

/* Common memory double-buffer for KAPS ARM FIFO */
/* Attempt to concatenate additional writes to the same descriptor DMA */
static uint32 *jer2_jer_sbusdma_desc_mem_fifo_buff_a[DNX_SAND_MAX_DEVICE];
static uint32 *jer2_jer_sbusdma_desc_mem_fifo_buff_b[DNX_SAND_MAX_DEVICE];
static uint32 *jer2_jer_sbusdma_desc_mem_fifo_buff_main[DNX_SAND_MAX_DEVICE];
static uint32 jer2_jer_sbusdma_desc_mem_fifo_buff_counter[DNX_SAND_MAX_DEVICE];
/* This points to the index of the KAPS ARM FIFO descriptor in jer2_jer_sbusdma_desc_cfg_array_main, -1 is invalid */
static uint32 jer2_jer_sbusdma_desc_mem_fifo_desc_index[DNX_SAND_MAX_DEVICE];

/* Host memory descriptors */
static soc_sbusdma_desc_cfg_t *jer2_jer_sbusdma_desc_cfg_array_a[DNX_SAND_MAX_DEVICE];
static soc_sbusdma_desc_cfg_t *jer2_jer_sbusdma_desc_cfg_array_b[DNX_SAND_MAX_DEVICE];
static soc_sbusdma_desc_cfg_t *jer2_jer_sbusdma_desc_cfg_array_main[DNX_SAND_MAX_DEVICE];
static uint32 jer2_jer_sbusdma_desc_counter[DNX_SAND_MAX_DEVICE];

/* Configuration of memory buffer size and max number of descriptors in chain */
static uint32 jer2_jer_sbusdma_desc_cfg_max[DNX_SAND_MAX_DEVICE];
static uint32 jer2_jer_sbusdma_desc_mem_max[DNX_SAND_MAX_DEVICE];
static VOLATILE uint32 jer2_jer_sbusdma_desc_time_out_max[DNX_SAND_MAX_DEVICE];

/* Holds the last descriptor handle, 0 if the last has ended */
static VOLATILE sbusdma_desc_handle_t desc_handle[DNX_SAND_MAX_DEVICE];

static VOLATILE sal_mutex_t jer2_jer_sbusdma_desc_mutex[DNX_SAND_MAX_DEVICE];

static VOLATILE sal_sem_t jer2_jer_sbusdma_desc_timeout_sem[DNX_SAND_MAX_DEVICE];
static VOLATILE sal_mutex_t jer2_jer_sbusdma_desc_timeout_mutex[DNX_SAND_MAX_DEVICE];
static VOLATILE sal_usecs_t jer2_jer_sbusdma_desc_timeout_add_time[DNX_SAND_MAX_DEVICE]; /* Holds the descriptor chain create time */
static VOLATILE sal_thread_t jer2_jer_sbusdma_desc_timeout_tid[DNX_SAND_MAX_DEVICE];
static VOLATILE uint8 jer2_jer_sbusdma_desc_timeout_terminate[DNX_SAND_MAX_DEVICE];

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

uint32 jer2_jer_sbusdma_desc_is_enabled(int unit) {
    return jer2_jer_sbusdma_desc_enabled[unit];
}

uint32 jer2_jer_sbusdma_desc_status(int unit, uint32 *desc_num_max, uint32 *mem_buff_size, uint32 *timeout_usec) {
    DNXC_INIT_FUNC_DEFS;

    *desc_num_max = jer2_jer_sbusdma_desc_cfg_max[unit];
    *mem_buff_size = jer2_jer_sbusdma_desc_mem_max[unit];
    *timeout_usec = jer2_jer_sbusdma_desc_time_out_max[unit];

    DNXC_FUNC_RETURN;
}


static uint32 jer2_jer_sbusdma_desc_wait_previous_done(int unit)
{
    soc_timeout_t to;

    DNXC_INIT_FUNC_DEFS;

    if (jer2_jer_sbusdma_desc_enabled[unit]) {
        soc_timeout_init(&to, SOC_SBUSDMA_DM_TO(unit) * 2, 0);
        /* Wait for the previous request to finish */
        sal_mutex_take(jer2_jer_sbusdma_desc_mutex[unit], sal_mutex_FOREVER);
        while (desc_handle[unit] != 0) {
            sal_mutex_give(jer2_jer_sbusdma_desc_mutex[unit]);
            if (soc_timeout_check(&to)) {
                DNXC_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_DNXC_MSG("Timeout waiting for descriptor DMA to finish.")));
            }
            sal_mutex_take(jer2_jer_sbusdma_desc_mutex[unit], sal_mutex_FOREVER);
        }
        sal_mutex_give(jer2_jer_sbusdma_desc_mutex[unit]);
    }

exit:
    DNXC_FUNC_RETURN;
}

void static
jer2_jer_sbusdma_desc_cb(int unit, int status, sbusdma_desc_handle_t handle,
                    void *data)
{
#ifdef JER2_JER_SBUSDMA_DESC_PRINTS_ENABLED
    if (status == SOC_E_NONE) {
        int rv = SOC_E_NONE;
        int i=0;
        soc_sbusdma_desc_ctrl_t ctrl = {0};
        soc_sbusdma_desc_cfg_t *cfg = NULL;

        LOG_CLI((BSL_META_U(0,"Successfully done DESC DMA, handle: %d\n"), handle));

        cfg = sal_alloc(sizeof(soc_sbusdma_desc_cfg_t) * jer2_jer_sbusdma_desc_cfg_max[unit], "soc_sbusdma_desc_cfg_t");
        if (cfg == NULL) {
            LOG_ERROR(BSL_LS_SOC_DMA,
                      (BSL_META_U(unit,
                                  "Error: Fail to allocate memory for SBUSDMA desc_cfg for failure log print.\n")));

        } else {
            rv = soc_sbusdma_desc_get(unit, handle, &ctrl, cfg);
            if (rv != SOC_E_NONE) {
                LOG_CLI((BSL_META_U(0,"%s(), soc_sbusdma_desc_get failed.\n"),FUNCTION_NAME()));
            }

            LOG_ERROR(BSL_LS_SOC_DMA,
                      (BSL_META_U(unit,
                                  "The following memory writes have succeeded (a total of %d):\n"), ctrl.cfg_count));
            for (i=0; i < ctrl.cfg_count; i++) {
                LOG_ERROR(BSL_LS_SOC_DMA,
                          (BSL_META_U(unit,
                                      "blk: %d, addr: %d, width: %d, count: %d, addr_shift: %d\n"),
                                      cfg[i].blk, cfg[i].addr, cfg[i].width, cfg[i].count, cfg[i].addr_shift
                           ));
            }

            sal_free(cfg);
        }
    }
#endif /*JER2_JER_SBUSDMA_DESC_PRINTS_ENABLED*/

    if (status != SOC_E_NONE) {
        int rv = SOC_E_NONE;
        int i=0;
        soc_sbusdma_desc_ctrl_t ctrl = {0};
        soc_sbusdma_desc_cfg_t *cfg = NULL;

        LOG_ERROR(BSL_LS_SOC_DMA,
                  (BSL_META_U(unit,
                              "Desc SBUSDMA failed, handle: %d\n"), handle
                   ));

        cfg = sal_alloc(sizeof(soc_sbusdma_desc_cfg_t) * jer2_jer_sbusdma_desc_cfg_max[unit], "soc_sbusdma_desc_cfg_t");
        if (cfg == NULL) {
            LOG_ERROR(BSL_LS_SOC_DMA,
                      (BSL_META_U(unit,
                                  "Error: Fail to allocate memory for SBUSDMA desc_cfg for failure log print.\n")));

        } else {
            rv = soc_sbusdma_desc_get(unit, handle, &ctrl, cfg);
            if (rv != SOC_E_NONE) {
                LOG_CLI((BSL_META_U(0,"%s(), soc_sbusdma_desc_get failed.\n"),FUNCTION_NAME()));
            }

            LOG_ERROR(BSL_LS_SOC_DMA,
                      (BSL_META_U(unit,
                                  "The following memory writes have failed:\n")));
            for (i=0; i < ctrl.cfg_count; i++) {
                LOG_ERROR(BSL_LS_SOC_DMA,
                          (BSL_META_U(unit,
                                      "blk: %d, addr: %d, width: %d, count: %d, addr_shift: %d\n"),
                                      cfg[i].blk, cfg[i].addr, cfg[i].width, cfg[i].count, cfg[i].addr_shift
                           ));
            }

            sal_free(cfg);
        }
    }

    sal_mutex_take(jer2_jer_sbusdma_desc_mutex[unit], sal_mutex_FOREVER);
    desc_handle[unit] = 0;
    sal_mutex_give(jer2_jer_sbusdma_desc_mutex[unit]);

    (void)soc_sbusdma_desc_delete(unit, handle);
}

static uint32 jer2_jer_sbusdma_desc_commit(int unit, uint8 safe)
{
    soc_sbusdma_desc_ctrl_t desc_ctrl = {0};
    soc_sbusdma_desc_cfg_t *desc_cfg_array;

    DNXC_INIT_FUNC_DEFS;

    if (!safe) {
        sal_mutex_take(jer2_jer_sbusdma_desc_timeout_mutex[unit], sal_mutex_FOREVER);
    }

    DNXC_IF_ERR_EXIT(jer2_jer_sbusdma_desc_wait_previous_done(unit));

    if (jer2_jer_sbusdma_desc_counter[unit] != 0) {
        desc_ctrl.flags = SOC_SBUSDMA_MEMORY_CMD_MSG | SOC_SBUSDMA_WRITE_CMD_MSG;
        sal_strncpy(desc_ctrl.name, "DESC DMA", sizeof(desc_ctrl.name)-1);
        desc_ctrl.cfg_count = jer2_jer_sbusdma_desc_counter[unit];
        desc_ctrl.hw_desc = NULL;
        desc_ctrl.buff = NULL;
        desc_ctrl.cb = jer2_jer_sbusdma_desc_cb;
        desc_ctrl.data = NULL;

        /* Switch to the other buffer */
        desc_cfg_array = jer2_jer_sbusdma_desc_cfg_array_main[unit];
        if (jer2_jer_sbusdma_desc_mem_buff_main[unit] == jer2_jer_sbusdma_desc_mem_buff_a[unit]) {
            jer2_jer_sbusdma_desc_cfg_array_main[unit] = jer2_jer_sbusdma_desc_cfg_array_b[unit];
            jer2_jer_sbusdma_desc_mem_buff_main[unit] = jer2_jer_sbusdma_desc_mem_buff_b[unit];
        } else {
            jer2_jer_sbusdma_desc_cfg_array_main[unit] = jer2_jer_sbusdma_desc_cfg_array_a[unit];
            jer2_jer_sbusdma_desc_mem_buff_main[unit] = jer2_jer_sbusdma_desc_mem_buff_a[unit];
        }
        jer2_jer_sbusdma_desc_mem_buff_counter[unit] = 0;
        jer2_jer_sbusdma_desc_counter[unit] = 0;

        if (jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit] != -1) {
            /* Switch to the other buffer for ARM FIFO */
            if (jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit] == jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit]) {
                jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit] = jer2_jer_sbusdma_desc_mem_fifo_buff_b[unit];
            } else {
                jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit] = jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit];
            }
            jer2_jer_sbusdma_desc_mem_fifo_buff_counter[unit] = 0;
            jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit] = -1;
        }

        if (!safe) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
        }

        sal_mutex_take(jer2_jer_sbusdma_desc_mutex[unit], sal_mutex_FOREVER);
        DNXC_IF_ERR_REPORT(soc_sbusdma_desc_create(unit, &desc_ctrl, desc_cfg_array, (sbusdma_desc_handle_t *)&desc_handle[unit]));
        DNXC_IF_ERR_REPORT(soc_sbusdma_desc_run(unit, (sbusdma_desc_handle_t)desc_handle[unit]));
        sal_mutex_give(jer2_jer_sbusdma_desc_mutex[unit]);
    } else {
        if (!safe) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_sbusdma_desc_wait_done(int unit) {
    DNXC_INIT_FUNC_DEFS;
    if (jer2_jer_sbusdma_desc_enabled[unit]) {
        DNXC_IF_ERR_REPORT(jer2_jer_sbusdma_desc_commit(unit, 0/*safe*/));
        DNXC_IF_ERR_EXIT(jer2_jer_sbusdma_desc_wait_previous_done(unit));
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_sbusdma_desc_add_fifo_dma(int unit, soc_mem_t mem, uint32 array_index, int blk, uint32 offset, void *entry_data, uint32 count, uint32 addr_shift, uint8 new_desc){
    uint32 cfg_addr;
    uint8  cfg_acc_type;
    int cfg_blk = blk;
    uint32 entry_size = SOC_MEM_WORDS(unit, mem);
    uint32 give_sem = 0;
    uint32 rv = 0;

    DNXC_INIT_FUNC_DEFS;

    if ((jer2_jer_sbusdma_desc_enabled[unit] != 1)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Desc DMA is not enabled.")));
    }

    sal_mutex_take(jer2_jer_sbusdma_desc_timeout_mutex[unit], sal_mutex_FOREVER);

    if (jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit] == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Desc DMA to FIFO is not enabled.")));
    }

    /* Commit the descriptors if we have reached the descriptor limit or we do not have enough memory in buffer */
    if ((jer2_jer_sbusdma_desc_counter[unit] == jer2_jer_sbusdma_desc_cfg_max[unit]) ||
        /* The extra buffer is necessary for the descriptor DMA to operate correctly */
        (jer2_jer_sbusdma_desc_mem_fifo_buff_counter[unit] + (entry_size * count) + (SOC_REG_ABOVE_64_MAX_SIZE_U32 + 2) >= JER2_JER_KAPS_ARM_FIFO_SIZE_UINT32)) {
        rv = jer2_jer_sbusdma_desc_commit(unit, 1/*safe*/);
        if (rv != SOC_E_NONE) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            DNXC_EXIT_WITH_ERR(rv, (_BSL_DNXC_MSG("jer2_jer_sbusdma_desc_commit_unsafe failed.")));
        }
    }

    if (jer2_jer_sbusdma_desc_counter[unit] == 0) {
        jer2_jer_sbusdma_desc_timeout_add_time[unit] = sal_time_usecs();
        give_sem++;
    }

    sal_memcpy(&jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit][jer2_jer_sbusdma_desc_mem_fifo_buff_counter[unit]], entry_data, sizeof(uint32) * entry_size * count);

    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        cfg_blk = SOC_BLOCK2SCH(unit, cfg_blk);
    }

    /* Attempt to concatenate this write to the previous descriptor */
    if ((new_desc == 0) && (jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit] != -1)) {
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit]].count += count;
    } else {
        cfg_addr = soc_mem_addr_get(unit, mem, array_index, cfg_blk, offset, &cfg_acc_type);
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].acc_type = cfg_acc_type;
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].addr = cfg_addr;
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].addr_shift = addr_shift;
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].blk = cfg_blk;
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].buff = &jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit][jer2_jer_sbusdma_desc_mem_fifo_buff_counter[unit]];
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].count = count;
        jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].width = entry_size;
        jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit] = jer2_jer_sbusdma_desc_counter[unit];
        jer2_jer_sbusdma_desc_counter[unit] += 1;
    }

    jer2_jer_sbusdma_desc_mem_fifo_buff_counter[unit] += entry_size * count;

    sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);

    if (give_sem) {
        sal_sem_give(jer2_jer_sbusdma_desc_timeout_sem[unit]);
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_sbusdma_desc_add(int unit, soc_mem_t mem, uint32 array_index, int blk, uint32 offset, void *entry_data){
    uint32 cfg_addr;
    uint8  cfg_acc_type;
    int cfg_blk = blk;
    uint32 entry_size = SOC_MEM_WORDS(unit, mem);
    uint32 give_sem = 0;
    uint32 rv = 0;

    DNXC_INIT_FUNC_DEFS;

    if (jer2_jer_sbusdma_desc_enabled[unit] != 1) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Desc DMA not enabled.")));
    }

    sal_mutex_take(jer2_jer_sbusdma_desc_timeout_mutex[unit], sal_mutex_FOREVER);
    /* Commit the descriptors if we have reached the descriptor limit or we do not have enough memory in buffer */
    if ((jer2_jer_sbusdma_desc_counter[unit] == jer2_jer_sbusdma_desc_cfg_max[unit]) ||
        /* The extra buffer is necessary for the descriptor DMA to operate correctly */
        (jer2_jer_sbusdma_desc_mem_buff_counter[unit] + entry_size + (SOC_REG_ABOVE_64_MAX_SIZE_U32 + 2) >= jer2_jer_sbusdma_desc_mem_max[unit])) {
        rv = jer2_jer_sbusdma_desc_commit(unit, 1/*safe*/);
        if (rv != SOC_E_NONE) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            DNXC_EXIT_WITH_ERR(rv, (_BSL_DNXC_MSG("jer2_jer_sbusdma_desc_commit_unsafe failed.")));
        }
    }

    if (jer2_jer_sbusdma_desc_counter[unit] == 0) {
        jer2_jer_sbusdma_desc_timeout_add_time[unit] = sal_time_usecs();
        give_sem++;
    }

    sal_memcpy(&jer2_jer_sbusdma_desc_mem_buff_main[unit][jer2_jer_sbusdma_desc_mem_buff_counter[unit]], entry_data, sizeof(uint32) * entry_size);

    if (soc_feature(unit, soc_feature_new_sbus_format)) {
        cfg_blk = SOC_BLOCK2SCH(unit, cfg_blk);
    }

    cfg_addr = soc_mem_addr_get(unit, mem, array_index, cfg_blk, offset, &cfg_acc_type);
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].acc_type = cfg_acc_type;
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].addr = cfg_addr;
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].addr_shift = 0;
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].blk = cfg_blk;
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].buff = &jer2_jer_sbusdma_desc_mem_buff_main[unit][jer2_jer_sbusdma_desc_mem_buff_counter[unit]];
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].count = 1;
    jer2_jer_sbusdma_desc_cfg_array_main[unit][jer2_jer_sbusdma_desc_counter[unit]].width = entry_size;
    jer2_jer_sbusdma_desc_counter[unit] += 1;

    jer2_jer_sbusdma_desc_mem_buff_counter[unit] += entry_size;

    sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);

    if (give_sem) {
        sal_sem_give(jer2_jer_sbusdma_desc_timeout_sem[unit]);
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_sbusdma_desc_cleanup(int unit) {
    DNXC_INIT_FUNC_DEFS;

    if (jer2_jer_sbusdma_desc_timeout_mutex[unit] != NULL) {
        sal_mutex_take(jer2_jer_sbusdma_desc_timeout_mutex[unit], sal_mutex_FOREVER);
    }

    if (jer2_jer_sbusdma_desc_mem_buff_a[unit] != NULL) {
        soc_cm_sfree(unit, jer2_jer_sbusdma_desc_mem_buff_a[unit]);
        jer2_jer_sbusdma_desc_mem_buff_a[unit] = NULL;
    }
    if (jer2_jer_sbusdma_desc_mem_buff_b[unit] != NULL) {
        soc_cm_sfree(unit, jer2_jer_sbusdma_desc_mem_buff_b[unit]);
        jer2_jer_sbusdma_desc_mem_buff_b[unit] = NULL;
    }
    jer2_jer_sbusdma_desc_mem_buff_main[unit] = NULL;

    if (jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit] != NULL) {
        soc_cm_sfree(unit, jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit]);
        jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit] = NULL;
    }
    if (jer2_jer_sbusdma_desc_mem_fifo_buff_b[unit] != NULL) {
        soc_cm_sfree(unit, jer2_jer_sbusdma_desc_mem_fifo_buff_b[unit]);
        jer2_jer_sbusdma_desc_mem_fifo_buff_b[unit] = NULL;
    }
    jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit] = NULL;
    jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit] = -1;

    if (jer2_jer_sbusdma_desc_cfg_array_a[unit] != NULL) {
        sal_free(jer2_jer_sbusdma_desc_cfg_array_a[unit]);
        jer2_jer_sbusdma_desc_cfg_array_a[unit] = NULL;
    }
    if (jer2_jer_sbusdma_desc_cfg_array_b[unit] != NULL) {
        sal_free(jer2_jer_sbusdma_desc_cfg_array_b[unit]);
        jer2_jer_sbusdma_desc_cfg_array_b[unit] = NULL;
    }
    jer2_jer_sbusdma_desc_cfg_array_main[unit] = NULL;

    if (jer2_jer_sbusdma_desc_mutex[unit]) {
        sal_mutex_destroy(jer2_jer_sbusdma_desc_mutex[unit]);
        jer2_jer_sbusdma_desc_mutex[unit] = NULL;
    }

    /* Signal the thread to destroy the mutex and sem if it is active, otherwise directly destroy them */
    if ((jer2_jer_sbusdma_desc_timeout_tid[unit] != NULL) && (jer2_jer_sbusdma_desc_timeout_tid[unit] != SAL_THREAD_ERROR)) {
        jer2_jer_sbusdma_desc_timeout_terminate[unit] = 1;
        sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
        sal_sem_give(jer2_jer_sbusdma_desc_timeout_sem[unit]);
    } else {
        if (jer2_jer_sbusdma_desc_timeout_mutex[unit]) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            sal_mutex_destroy(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            jer2_jer_sbusdma_desc_timeout_mutex[unit] = NULL;
        }

        if (jer2_jer_sbusdma_desc_timeout_sem[unit]) {
            sal_sem_destroy(jer2_jer_sbusdma_desc_timeout_sem[unit]);
            jer2_jer_sbusdma_desc_timeout_sem[unit] = NULL;
        }
    }

    jer2_jer_sbusdma_desc_enabled[unit] = 0;
    jer2_jer_sbusdma_desc_mem_max[unit] = 0;
    jer2_jer_sbusdma_desc_cfg_max[unit] = 0;

    DNXC_FUNC_RETURN;
}

static void
jer2_jer_sbusdma_desc_init_timeout_thread(void *cookie)
{
    int                         unit = PTR_TO_INT(cookie);
    int                         rv = SOC_E_NONE;
    char                        thread_name[SAL_THREAD_NAME_MAX_LEN];
    sal_thread_t	            thread;
    sal_usecs_t                 elapsed_time;
    sal_usecs_t                 add_time;
    sal_usecs_t                 timeout_max = jer2_jer_sbusdma_desc_time_out_max[unit];

    while (1) {
        sal_sem_take(jer2_jer_sbusdma_desc_timeout_sem[unit], sal_sem_FOREVER);
        sal_mutex_take(jer2_jer_sbusdma_desc_timeout_mutex[unit], sal_mutex_FOREVER);
        if (jer2_jer_sbusdma_desc_timeout_terminate[unit]) {
            /* Destroy the mutex and sem along with the thread */
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            if (jer2_jer_sbusdma_desc_timeout_mutex[unit]) {
                sal_mutex_destroy(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
                jer2_jer_sbusdma_desc_timeout_mutex[unit] = NULL;
            }

            if (jer2_jer_sbusdma_desc_timeout_sem[unit]) {
                sal_sem_destroy(jer2_jer_sbusdma_desc_timeout_sem[unit]);
                jer2_jer_sbusdma_desc_timeout_sem[unit] = NULL;
            }

            jer2_jer_sbusdma_desc_timeout_tid[unit] = NULL;

            sal_thread_exit(0);
        }

        /* In case the main thread has already committed this descriptor chain */
        if (jer2_jer_sbusdma_desc_counter[unit] == 0) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            continue;
        }
        add_time = jer2_jer_sbusdma_desc_timeout_add_time[unit];
        elapsed_time = sal_time_usecs() - add_time;
        while (elapsed_time < timeout_max) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            sal_usleep(timeout_max - elapsed_time);
            sal_mutex_take(jer2_jer_sbusdma_desc_timeout_mutex[unit], sal_mutex_FOREVER);
            elapsed_time = sal_time_usecs() - add_time;
        }

        /* Check if the main thread has already committed this descriptor chain and started a new one */
        if (add_time == jer2_jer_sbusdma_desc_timeout_add_time[unit]) {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
            rv = jer2_jer_sbusdma_desc_commit(unit,0/*safe*/);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_DMA,
                          (BSL_META_U(unit,
                                      "jer2_jer_sbusdma_desc_commit returned with error: %d, unit %d\n"), rv, unit));
            }
        } else {
            sal_mutex_give(jer2_jer_sbusdma_desc_timeout_mutex[unit]);
        }
    }

    /* Some error happened. */
    thread = sal_thread_self();
    thread_name[0] = 0;
    sal_thread_name(thread, thread_name, sizeof (thread_name));
    LOG_ERROR(BSL_LS_SOC_DMA,
              (BSL_META_U(unit,
                          "AbnormalThreadExit:%s, unit %d\n"), thread_name, unit));

    jer2_jer_sbusdma_desc_timeout_tid[unit] = SAL_THREAD_ERROR;

    sal_thread_exit(0);
}

uint32 jer2_jer_sbusdma_desc_init(int unit, uint32 desc_num_max, uint32 mem_buff_size, uint32 timeout_usec)
{
    char buffer_name[50];
    DNXC_INIT_FUNC_DEFS;

    if ((jer2_jer_sbusdma_desc_mem_buff_a[unit] != NULL) || (jer2_jer_sbusdma_desc_mem_buff_b[unit] != NULL) ) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Descriptor DMA buffers already allocated.")));
    }

    jer2_jer_sbusdma_desc_mem_max[unit] = mem_buff_size;
    jer2_jer_sbusdma_desc_cfg_max[unit] = desc_num_max;
    jer2_jer_sbusdma_desc_time_out_max[unit] = timeout_usec;

    /* allocate memory for common memory double-buffer */
    sal_sprintf(buffer_name, "SBUSDMA desc buffer a, Unit %d", unit);
    if ((jer2_jer_sbusdma_desc_mem_buff_a[unit] = (uint32 *)soc_cm_salloc(unit, sizeof(uint32) * jer2_jer_sbusdma_desc_mem_max[unit], buffer_name)) == NULL) {
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Error: Fail to allocate memory for SBUSDMA desc buffer a!!!")));
    }
    sal_memset(jer2_jer_sbusdma_desc_mem_buff_a[unit], 0, sizeof(uint32) * jer2_jer_sbusdma_desc_mem_max[unit]);

    sal_sprintf(buffer_name, "SBUSDMA desc buffer b, Unit %d", unit);
    if ((jer2_jer_sbusdma_desc_mem_buff_b[unit] = (uint32 *)soc_cm_salloc(unit, sizeof(uint32) * jer2_jer_sbusdma_desc_mem_max[unit], buffer_name)) == NULL) {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Error: Fail to allocate memory for SBUSDMA desc buffer b!!!")));
    }
    sal_memset(jer2_jer_sbusdma_desc_mem_buff_b[unit], 0, sizeof(uint32) * jer2_jer_sbusdma_desc_mem_max[unit]);

    jer2_jer_sbusdma_desc_mem_buff_main[unit] = jer2_jer_sbusdma_desc_mem_buff_a[unit];
    jer2_jer_sbusdma_desc_mem_buff_counter[unit] = 0;

    /* allocate memory for common memory fifo double-buffer, only valid for Jericho KAPS ARM implementation */
    if (!SOC_IS_JERICHO_PLUS(unit) && (soc_property_suffix_num_get(unit, -1, spn_DMA_DESC_AGGREGATOR_ENABLE_SPECIFIC, "KAPS", 0))) {
        sal_sprintf(buffer_name, "SBUSDMA desc fifo buffer a, Unit %d", unit);
        if ((jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit] = (uint32 *)soc_cm_salloc(unit, sizeof(uint32) * JER2_JER_KAPS_ARM_FIFO_SIZE_UINT32, buffer_name)) == NULL) {
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Error: Fail to allocate memory for SBUSDMA desc fifo buffer a!!!")));
        }
        sal_memset(jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit], 0, sizeof(uint32) * JER2_JER_KAPS_ARM_FIFO_SIZE_UINT32);

        sal_sprintf(buffer_name, "SBUSDMA desc fifo buffer b, Unit %d", unit);
        if ((jer2_jer_sbusdma_desc_mem_fifo_buff_b[unit] = (uint32 *)soc_cm_salloc(unit, sizeof(uint32) * JER2_JER_KAPS_ARM_FIFO_SIZE_UINT32, buffer_name)) == NULL) {
            jer2_jer_sbusdma_desc_cleanup(unit);
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Error: Fail to allocate memory for SBUSDMA desc fifo buffer b!!!")));
        }
        sal_memset(jer2_jer_sbusdma_desc_mem_fifo_buff_b[unit], 0, sizeof(uint32) * JER2_JER_KAPS_ARM_FIFO_SIZE_UINT32);

        jer2_jer_sbusdma_desc_mem_fifo_buff_main[unit] = jer2_jer_sbusdma_desc_mem_fifo_buff_a[unit];
        jer2_jer_sbusdma_desc_mem_fifo_buff_counter[unit] = 0;
    }
    jer2_jer_sbusdma_desc_mem_fifo_desc_index[unit] = -1;

    /* allocate memory for host memory descriptor buffer */
    jer2_jer_sbusdma_desc_cfg_array_a[unit] = sal_alloc(sizeof(soc_sbusdma_desc_cfg_t) * jer2_jer_sbusdma_desc_cfg_max[unit], "soc_sbusdma_desc_cfg_t");
    if (jer2_jer_sbusdma_desc_cfg_array_a[unit] == NULL) {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Error: Fail to allocate memory for SBUSDMA desc_cfg a")));
    }

    jer2_jer_sbusdma_desc_cfg_array_b[unit] = sal_alloc(sizeof(soc_sbusdma_desc_cfg_t) * jer2_jer_sbusdma_desc_cfg_max[unit], "soc_sbusdma_desc_cfg_t");
    if (jer2_jer_sbusdma_desc_cfg_array_b[unit] == NULL) {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Error: Fail to allocate memory for SBUSDMA desc_cfg b")));
    }
    jer2_jer_sbusdma_desc_cfg_array_main[unit] = jer2_jer_sbusdma_desc_cfg_array_a[unit];
    jer2_jer_sbusdma_desc_counter[unit] = 0;

    jer2_jer_sbusdma_desc_mutex[unit] = sal_mutex_create("DESC DMA mutex");
    if(jer2_jer_sbusdma_desc_mutex[unit] == NULL) {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Mutex allocation failure.")));
    }

    jer2_jer_sbusdma_desc_timeout_mutex[unit] = sal_mutex_create("DESC DMA TO mutex");
    if(jer2_jer_sbusdma_desc_timeout_mutex[unit] == NULL) {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("TO Mutex allocation failure.")));
    }

    jer2_jer_sbusdma_desc_timeout_sem[unit] = sal_sem_create("DESC DMA TO sem", sal_sem_COUNTING, 0);
    if(jer2_jer_sbusdma_desc_timeout_sem[unit] == NULL) {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("TO Sempahore allocation failure.")));
    }

    jer2_jer_sbusdma_desc_timeout_add_time[unit] = sal_time_usecs();

    jer2_jer_sbusdma_desc_timeout_terminate[unit] = 0;

    if ((jer2_jer_sbusdma_desc_timeout_tid[unit] == SAL_THREAD_ERROR) || (jer2_jer_sbusdma_desc_timeout_tid[unit] == NULL)) {
        jer2_jer_sbusdma_desc_timeout_tid[unit] = sal_thread_create("Desc DMA Timeout",
                                          SAL_THREAD_STKSZ,
                                          50 /* priority */,
                                          jer2_jer_sbusdma_desc_init_timeout_thread,
                                          INT_TO_PTR(unit));
        if ((jer2_jer_sbusdma_desc_timeout_tid[unit] == NULL) || (jer2_jer_sbusdma_desc_timeout_tid[unit] == SAL_THREAD_ERROR)) {
            jer2_jer_sbusdma_desc_cleanup(unit);
            DNXC_EXIT_WITH_ERR(DNX_SAND_GEN_ERR, (_BSL_DNXC_MSG("DESC DMA TO thread create failed. \n")));
        }
    } else {
        jer2_jer_sbusdma_desc_cleanup(unit);
        DNXC_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_DNXC_MSG("TO thread already exists.")));
    }

    jer2_jer_sbusdma_desc_enabled[unit] = 1;

    desc_handle[unit] = 0;

exit:
    DNXC_FUNC_RETURN;
}

uint32 jer2_jer_sbusdma_desc_deinit(int unit)
{
    DNXC_INIT_FUNC_DEFS;

    if (jer2_jer_sbusdma_desc_enabled[unit]) {
        DNXC_IF_ERR_REPORT(jer2_jer_sbusdma_desc_wait_done(unit));
        jer2_jer_sbusdma_desc_cleanup(unit);

        jer2_jer_sbusdma_desc_enabled[unit] = 0;
    }

    DNXC_FUNC_RETURN;
}

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

