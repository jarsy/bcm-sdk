/*
 * $Id: debug.c,v 1.19.54.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: debug.c
 * Purpose: Caladan3 ucode debug manager
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/lrp.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/caladan3/ucodemgr.h>
#include <soc/sbx/caladan3/asm3/asm3_pkg_intf.h>
#include <soc/sbx/caladan3/asm3/debug.h>

#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <soc/util.h>
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/feature.h>
#include <soc/property.h>
#include <soc/sbx/sbDq.h>
#include <soc/debug.h>

enum {
    SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_DUMP_CONTEXT = 0x0,
    SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_SET_REGISTER = 0x1,
    SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_CONTINUE     = 0x2,
    SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_END          = 0x3,
    SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_SET_HEADER   = 0x4,
  SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_MAX          
};

#define SOC_SBX_CALADAN3_UCODE_DEBUG_OWNER_LRP           0xC3C3
#define SOC_SBX_CALADAN3_UCODE_DEBUG_OWNER_HOST          0xAABB

#define SOC_SBX_CALADAN3_UCODE_DEBUG_RSP_ERROR           0xE22
#define SOC_SBX_CALADAN3_UCODE_DEBUG_RSP_OK              0x01C

/* 
 * Break point:
 *    This driver will directly support a simple break point operation
 *    It will support conditional break points when interfaced to the MDE
 *    wherein MDE will provide the encoded instruction
 */
/*
 * uint8 break_inst[] = { 0xe4, 0xc5, 0x60, 0xa2, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
 */

uint8 break_inst[] = { 0xe4, 0xc4, 0x1f, 0xa2, 0xff, 0xf8, 0x1f, 0xa2, 0x00, 0x01, 0x08, 0x00 };
uint8 nop_inst[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


debugmgr_t _dbgmgr[SOC_MAX_NUM_DEVICES];

/*
 * Macros for manipulating messaging 
 */
#define UCODE_SESSION_ACTIVE   (1 << 10)
#define UCODE_RESPONSE(cmd) ((cmd) & 0x3FF)
#define UCODE_SESSIONID(peinfo) ((peinfo) & 0x3FF)

#define UCODE_SET_COMMAND(cmd) ((cmd) << 12)
#define UCODE_GET_COMMAND(cmd) (((cmd) >> 12) & 0xFF)

#define UCODE_SET_REGISTER(regid) ((regid) << 24) 
#define UCODE_GET_REGISTER(regid) (((regid) >> 24) & 0xFF) 


#define UCODE_OWNER(owner)     (SOC_SBX_CALADAN3_UCODE_DEBUG_OWNER_ ## owner)
#define UCODE_SET_OWNER(owner)    ((UCODE_OWNER(owner)) << 16)
#define UCODE_GET_OWNER(peinfo)   (((peinfo) >> 16) & 0xFFFF)
#define UCODE_SET_SESSION(peinfo) (UCODE_SET_OWNER(LRP) | UCODE_SESSIONID(peinfo) |   \
                                      UCODE_SESSION_ACTIVE)

/*
 * Taskmap handling
 */
#define IS_EGRESS(tmap, inst)  (((tmap).map[((inst) >> 5)] & (1 << (inst % 32))) > 0) 
#define IS_INGRESS(tmap, inst)  !(IS_EGRESS(tmap, inst))

int
find_next_ingress_inst(debugmgr_t *mgr, int inst) 
{
    int _inst = 0;
    for (_inst = inst+1; ; _inst++)
    {
        if (_inst == inst) return -1;
        if (_inst >= 1535) _inst = 0; /* wrapping?? */
        if (IS_INGRESS(mgr->taskmap, _inst)) break;
    }
    return _inst;
}

int
find_next_egress_inst(debugmgr_t *mgr, int inst) 
{
    int _inst = 0;
    for (_inst = inst+1; ; _inst++)
    {
        if (_inst == inst) return -1;
        if (_inst >= 1535) _inst = 0; /* wrapping?? */
        if (IS_EGRESS(mgr->taskmap, _inst)) break;
    }
    return _inst;
}

int
soc_sbx_caladan3_ucode_debug_taskmap_get(int unit) 
{
    debugmgr_t *mgr;
    int i, rv = SOC_E_NONE;

    mgr = &_dbgmgr[unit];
    if (mgr && mgr->init_done && (mgr->taskmap.map[0] == 0)) {
        for (i = soc_mem_index_min(unit, LRA_INST_TASK_MAPm);
             i < soc_mem_index_max(unit, LRA_INST_TASK_MAPm);
             i++) {
            rv = READ_LRA_INST_TASK_MAPm(unit, MEM_BLOCK_ANY, i, &(mgr->taskmap.map[i]));
            if (SOC_FAILURE(rv)) 
                LOG_CLI((BSL_META_U(unit,
                                    "WARNING:Failed reading TASKMAP info \n")));
        }
    }
    return rv;
}

                
/*
 * Context handling
 */
#define get_register_value(mgr, rnum) \
    *(((uint32*)&((mgr)->curr_context->registers))+(rnum));  

#define set_register_value(mgr, rnum, val) \
    do {    \
    LOG_CLI((BSL_META("Setting register (%d) to (%08x)\n"), (rnum), (val))); \
    *(((uint32*)&((mgr)->curr_context->registers))+(rnum)) = (val);   \
    } while (0);


#define UCODE_DEBUG_LOCK(unit)     \
            sal_mutex_take(_dbgmgr[(unit)].lock, sal_mutex_FOREVER)
#define UCODE_DEBUG_UNLOCK(unit)    \
            sal_mutex_give(_dbgmgr[(unit)].lock)

#define UCODE_DEBUG_CHECK_SWITCH(inst)   \
     (((inst)[0] == 0xff) && (((inst)[1] == 0xc0) || ((inst)[1] == 0x80)))


#define UCODE_DEBUG_CHECK_BREAK(inst)   \
      (((inst[0] & 0x1f) == (break_inst[0] & 0x1f)) && \
       ((inst[1] & 0xff) == (break_inst[1] & 0xff)) && \
       ((inst[2] & 0xff) == (break_inst[2] & 0xff)) && \
       ((inst[3] & 0xff) == (break_inst[3] & 0xff)) && \
       ((inst[4] & 0x00) == (break_inst[4] & 0x00)) && \
       ((inst[5] & 0x1f) == (break_inst[5] & 0x1f)) && \
        (sal_memcmp((&inst[6]), &break_inst[6], 6) == 0))

#define UCODE_DEBUG_CHECK_NOPREDICATION(inst)    \
      ((inst[4] == 0xff) && ((inst[5] & 0xe0) == 0xe0))

#define UCODE_DEBUG_CHECK_BRANCH(inst)   ((((inst)[0] >> 2) & 0x7) == 0)
#define UCODE_DEBUG_CHECK_NOP(inst)   (sal_memcmp(inst, nop_inst, 12) == 0)
#define UCODE_DEBUG_GET_BRANCH_STREAM(inst)   (((inst)[2] >> 3) & 0xF)



int 
soc_sbx_caladan3_ucode_debug_bp_update(int unit, int snum, int inum);
int 
soc_sbx_caladan3_ucode_debug_continue_on(int unit, debugmgr_t *mgr); 
int 
soc_sbx_caladan3_ucode_debug_step(int unit, debugmgr_t *mgr); 


bp_t *
bp_alloc(int unit) 
{
    bp_t *bp = NULL;
    bp = sal_alloc(sizeof(bp_t), "c3 breakpoint");
    if (!bp) {
        LOG_CLI((BSL_META_U(unit,
                            "\nERROR: No memory to allocate Breakpoint ")));
        return NULL;
    }
    sal_memset(bp, 0, sizeof(bp_t));
    bp->mgr = &_dbgmgr[unit];
    bp->bpnum = bp->mgr->curr_bp;
    bp->mgr->num_bp++;
    bp->mgr->curr_bp++;
    sal_memcpy(bp->break_inst, break_inst, 12);
    DQ_INSERT_HEAD(&bp->mgr->bplist, &bp->chain);
    return bp;
}

void 
bp_free(bp_t* bp) {
    bp->mgr->num_bp--;
    DQ_REMOVE(bp);
    sal_free(bp);
}

bp_t *
bp_findn(int unit, int bpnum) 
{
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;

    mgr = &_dbgmgr[unit];
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            if (bp->bpnum == bpnum) {
                return bp;
            }
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);
    return NULL;
}

bp_t *
bp_find(int unit, int snum, int inum) 
{
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;

    mgr = &_dbgmgr[unit];
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            if ((bp->inum == inum) && (bp->snum == snum)) {
                return bp;
            }
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);
    return NULL;
}

void
bp_dump(bp_t *bp) 
{
    char buf[256] = {0};
    if (bp) {
        LOG_CLI((BSL_META(" >> Bp num = %d Total hits: %d\n"), bp->bpnum, bp->hits)); 
        soc_sbx_caladan3_ucode_debug_decode_inst(buf, bp->orig_inst);
        LOG_CLI((BSL_META(" >> Str = %d Inst = %d -> %s\n"), bp->snum, bp->inum, buf));
        LOG_CLI((BSL_META(" >> Raw Instruction   = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
                 bp->orig_inst[0], bp->orig_inst[1], bp->orig_inst[2],
                 bp->orig_inst[3], bp->orig_inst[4], bp->orig_inst[5],
                 bp->orig_inst[6], bp->orig_inst[7], bp->orig_inst[8],
                 bp->orig_inst[9],bp->orig_inst[10],bp->orig_inst[11]));
    }
}

int
bp_set(int unit, int snum, int inum) 
{
    void *handle = INT_TO_PTR(unit);
    debugmgr_t *mgr;
    bp_t *bp = NULL;
    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "bp_set: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if ((bp = bp_find(unit, snum, inum)) == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "bp_set: breakpoint does not exists\n")));
        return SOC_E_NOT_FOUND;
    }
    mgr->ucodemgr->lr_iwrite(handle, snum, inum, bp->break_inst);
    soc_sbx_caladan3_lr_isr_enable(unit);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_ucode_debug_bp_replay(int unit)
{
    void *handle = INT_TO_PTR(unit);
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        /* Nothing to do */
        return SOC_E_NONE;
    }
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            if (!bp->active) {
                continue;
            }
            mgr->ucodemgr->lr_iread(handle, bp->snum, bp->inum, bp->orig_inst);
            if (UCODE_DEBUG_CHECK_SWITCH(bp->orig_inst)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "debug_bp_replay: cant break on switch, ignoring bp S(%d)I(%d)\n"),
                           bp->snum, bp->inum));
                bp_free(bp);
                continue;
            }
            mgr->ucodemgr->lr_iwrite(handle, bp->snum, bp->inum, bp->break_inst);
            soc_sbx_caladan3_lr_isr_enable(unit);
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_ucode_debug_bp_list(int unit) 
{
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;
    char buf[256] = {0};

    mgr = &_dbgmgr[unit];
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            soc_sbx_caladan3_ucode_debug_decode_inst(buf, bp->orig_inst);
            LOG_CLI((BSL_META_U(unit,
                                "\nBP:%d Str:%d Inst:%d %s"),
                     bp->bpnum, bp->snum, bp->inum, buf));
            LOG_CLI((BSL_META_U(unit,
                                "\nInst:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x"), 
                     bp->orig_inst[0], bp->orig_inst[1], bp->orig_inst[2],
                     bp->orig_inst[3], bp->orig_inst[4], bp->orig_inst[5],
                     bp->orig_inst[6], bp->orig_inst[7], bp->orig_inst[8],
                     bp->orig_inst[9],bp->orig_inst[10],bp->orig_inst[11]));
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    return SOC_E_NONE;
}

uint32
soc_sbx_caladan3_ucode_debug_bp_count(int unit)
{
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;
    uint32 count = 0;

    mgr = &_dbgmgr[unit];
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            count++;
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);

    return count;
}

int
soc_sbx_caladan3_ucode_debug_bp_list_get(int unit, uint32 count, uint32 *bpnum, uint32 *snum, uint32 *inum) 
{
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;

    mgr = &_dbgmgr[unit];
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            *bpnum = bp->bpnum;
            *snum = bp->snum;
            *inum = bp->inum;
            bpnum++;
            snum++;
            inum++;
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);

    return SOC_E_NONE;
}




int 
soc_sbx_caladan3_ucode_debug_init(int unit, soc_sbx_caladan3_ucodemgr_t *ucodemgr) 
{
     debugmgr_t *mgr;
     int rv = SOC_E_NONE;
     sbx_caladan3_ocm_port_alloc_t segment = {0};
     mgr = &_dbgmgr[unit];    
     if (!mgr->init_done) {
         mgr->curr_context = soc_cm_salloc(unit, sizeof(context_t), "c3 debug context");

         if (!mgr->curr_context) {
             LOG_ERROR(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "Failed to alloc memory for context\n")));
             return SOC_E_MEMORY;
         }
         mgr->session_info.pe_info = 0;
         mgr->session_info.cmd_info = 0;
         sal_memset(mgr->curr_context, 0, sizeof(context_t));
         sal_memset(&mgr->taskmap, 0, sizeof(taskmap_t));
         mgr->num_bp = 0;
         mgr->curr_bp = 0;
         DQ_INIT(&_dbgmgr[unit].bplist);
         mgr->lock =  sal_mutex_create("debugger lock");
         if (!mgr->lock) {
             LOG_ERROR(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "\nFailed to create debugger lock ")));
             return SOC_E_MEMORY;
         }
         mgr->cready =  sal_sem_create("debugger context ready", sal_sem_BINARY, 0);
         if (!mgr->cready) {
             LOG_ERROR(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "Failed to create debugger context ready sem\n")));
             return SOC_E_MEMORY;
         }
         mgr->tready =  sal_sem_create("debugger ready", sal_sem_BINARY, 0);
         if (!mgr->tready) {
             LOG_ERROR(BSL_LS_SOC_COMMON,
                       (BSL_META_U(unit,
                                   "Failed to create debugger ready sem\n")));
             return SOC_E_MEMORY;
         }
         mgr->ucodemgr = ucodemgr;
         /*
          * soc_sbx_caladan3_ucodemgr_sym_get(unit, ucodemgr->ucode, UCODE_DEBUG_CMD_SEGMENT, &mgr->cmd_segment);
          * soc_sbx_caladan3_ucodemgr_sym_get(unit, ucodemgr->ucode, UCODE_DEBUG_RSP_SEGMENT, &mgr->rsp_segment);
          */
         mgr->cmd_segment = 127;
         mgr->cmd_port = SOC_SBX_CALADAN3_OCM_LRP0_PORT;
         segment.port =   mgr->cmd_port;
         segment.segment = mgr->cmd_segment;
         segment.size =        256;
         segment.datum_size =  SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD;
         soc_sbx_caladan3_ocm_port_segment_alloc(unit, &segment);

         mgr->rsp_port = SOC_SBX_CALADAN3_OCM_LRP0_PORT;
         mgr->rsp_segment = 126;
         segment.size =     1024;
         segment.segment = mgr->rsp_segment;
         segment.datum_size =  SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD;
         segment.port = mgr->rsp_port;
         soc_sbx_caladan3_ocm_port_segment_alloc(unit, &segment);

         rv = soc_sbx_caladan3_lr_isr_enable(unit);
         if (SOC_FAILURE(rv)) {
             LOG_CLI((BSL_META_U(unit,
                                 "Failed enabling the LRA Interrupt")));
         }

         mgr->thread_priority = 101;
         mgr->init_done = 1;
     }
    return SOC_E_NONE;
}

int
soc_sbx_caladan3_ucode_debug_pull_session(int unit) 
{
    int rv = SOC_E_NONE;
    debugmgr_t *mgr;
    session_data_t session;
    mgr = &_dbgmgr[unit];    
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_pull_session: debug not initialized\n")));
        return SOC_E_INIT;
    }
    UCODE_DEBUG_LOCK(unit);
    rv = soc_sbx_caladan3_ocm_port_mem_read(unit, mgr->cmd_port, mgr->cmd_segment, 0, 0, (uint32*)&session);
    if (SOC_SUCCESS(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "soc_sbx_caladan3_ucode_debug_session:  0x%x 0x%x\n"),  
                     session.pe_info, session.cmd_info));
        if (UCODE_GET_OWNER(session.pe_info) == SOC_SBX_CALADAN3_UCODE_DEBUG_OWNER_HOST) {
            if ((mgr->session_active) && (UCODE_GET_OWNER(session.pe_info) != UCODE_GET_OWNER(mgr->session_info.pe_info))) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "soc_sbx_caladan3_ucode_debug_poll: Session mismatch expected(0x%x) got (0x%x)\n"), 
                           UCODE_GET_OWNER(session.pe_info), UCODE_GET_OWNER(mgr->session_info.pe_info)));
                 UCODE_DEBUG_UNLOCK(unit);
                 return SOC_E_PARAM;
               
            } else {
                if ((session.cmd_info & 0xfff) == SOC_SBX_CALADAN3_UCODE_DEBUG_RSP_ERROR) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_caladan3_ucode_debug_pull_session: Debug Failed: 0x%x 0x%x\n"),  session.pe_info, session.cmd_info));
                    UCODE_DEBUG_UNLOCK(unit);
                    return SOC_E_FAIL;
                } else {
                    mgr->session_info.pe_info = session.pe_info;
                    mgr->session_info.cmd_info = session.cmd_info;
                }
            }
        } else {
            soc_sbx_caladan3_lr_isr_enable(unit);
            UCODE_DEBUG_UNLOCK(unit);
            return SOC_E_INTERNAL;
        }
    }        
    UCODE_DEBUG_UNLOCK(unit);

  /*  LOG_CLI((BSL_META_U(unit,
                          "%s: Active:%d [LRP pe_info:0x%08x  cmd_info:0x%08x] [SDK pe_info:0x%08x  cmd_info:0x%08x]\n"),
               FUNCTION_NAME(),
               mgr->session_active,
               session.pe_info,
               session.cmd_info,
               mgr->session_info.pe_info,
               mgr->session_info.cmd_info));
    */
    return rv;
}

int
soc_sbx_caladan3_ucode_debug_poll(int unit) 
{
    int i, rv = SOC_E_NONE;
    debugmgr_t *mgr;
    session_data_t session;
    mgr = &_dbgmgr[unit];    
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_poll: debug not initialized\n")));
        return SOC_E_INIT;
    }
    for (i=0; i < MAX_POLL_COUNT; i++) {
        UCODE_DEBUG_LOCK(unit);
        rv = soc_sbx_caladan3_ocm_port_mem_read(unit, mgr->cmd_port, mgr->cmd_segment, 0, 0, (uint32*)&session);
        if (SOC_SUCCESS(rv)) {
                LOG_CLI((BSL_META_U(unit,
                                    "Session 0x%08x 0x%08x\n"), session.pe_info , session.cmd_info));
            if (UCODE_GET_OWNER(session.pe_info) == SOC_SBX_CALADAN3_UCODE_DEBUG_OWNER_HOST) {
                if ((mgr->session_active) && (UCODE_GET_OWNER(session.pe_info) != UCODE_GET_OWNER(mgr->session_info.pe_info))) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_sbx_caladan3_ucode_debug_poll: Session mismatch expected(0x%x) got (0x%x)\n"), 
                               UCODE_GET_OWNER(session.pe_info), UCODE_GET_OWNER(mgr->session_info.pe_info)));
                     UCODE_DEBUG_UNLOCK(unit);
                     return SOC_E_PARAM;
                   
                } else {
                    mgr->session_active = 1;
                    mgr->session_info.pe_info = session.pe_info;
                    mgr->session_info.cmd_info = session.cmd_info;
                }
                UCODE_DEBUG_UNLOCK(unit);
                return rv;
            }
        }
        UCODE_DEBUG_UNLOCK(unit);
        sal_usleep(100);
    }
    if (i >= MAX_POLL_COUNT) {
        rv = SOC_E_TIMEOUT;
    }


    LOG_CLI((BSL_META_U(unit,
                        "%s: pe_info:0x%08x  cmd_info:0x%08x\n"),
             FUNCTION_NAME(),
             session.pe_info,
             session.cmd_info));
    return rv;
}

int
soc_sbx_caladan3_ucode_debug_request_context(int unit) 
{
    int rv = SOC_E_NONE;
    debugmgr_t *mgr;
    session_data_t  cmd_msg;

    LOG_CLI((BSL_META_U(unit,
                        "%s:\n"), FUNCTION_NAME()));


    mgr = &_dbgmgr[unit];    
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_req_cxt: debug not initialized\n")));
        return SOC_E_INIT;
    }
    UCODE_DEBUG_LOCK(unit);
    cmd_msg.cmd_info = UCODE_SET_COMMAND(SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_DUMP_CONTEXT);
    cmd_msg.pe_info = UCODE_SET_SESSION(mgr->session_info.pe_info);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\nWriting command port %d segment %d 0x%x 0x%x\n"), mgr->cmd_port, mgr->cmd_segment,
                 cmd_msg.pe_info, cmd_msg.cmd_info));
    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, mgr->cmd_segment, 0, 0, (uint32*)&cmd_msg);
    if (SOC_SUCCESS(rv)) {
        mgr->context_valid = 0;
        soc_sbx_caladan3_lr_isr_enable(unit);
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_request_context: cant write data to mem\n")));
    }
    UCODE_DEBUG_UNLOCK(unit);
    return rv;
}

int
soc_sbx_caladan3_ucode_debug_get_current_frame(int unit, int timeout, int *str, int *inum, uint8 *inst)
{
    debugmgr_t *mgr;
    int rv = 0;
    mgr = &_dbgmgr[unit];  
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_pull_cxt: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (timeout <= 0) {
        rv = sal_sem_take(mgr->tready, sal_sem_FOREVER);
    } else {
        rv = sal_sem_take(mgr->tready, timeout);
    }
    if (SOC_FAILURE(rv)) {
        return rv;
    }
    UCODE_DEBUG_LOCK(unit);
    if (!mgr->context_valid) {
        UCODE_DEBUG_UNLOCK(unit);
        return SOC_E_NOT_FOUND;
    }
    if (str) *str = mgr->curr_frame.snum;
    if (inum) *inum = mgr->curr_frame.inum;
    if (inst) sal_memcpy(inst, mgr->curr_frame.orig_inst, 12);
    UCODE_DEBUG_UNLOCK(unit);

    return SOC_E_NONE;
} 


int
soc_sbx_caladan3_ucode_debug_pull_context(int unit) 
{
    int rv = 0, inum = 0, snum = 0;
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];    
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_pull_cxt: debug not initialized\n")));
        return SOC_E_INIT;
    }
    UCODE_DEBUG_LOCK(unit);
    rv = soc_sbx_caladan3_ocm_port_mem_read(unit, mgr->rsp_port, mgr->rsp_segment, 
                                            1, sizeof(context_t)/8,
                                            (uint32 *)mgr->curr_context);
    if (SOC_SUCCESS(rv)) {
        /* Update context */
        inum = ((mgr->curr_context->registers.bpr >> 16) & 0x7FF);
        snum = ((mgr->curr_context->registers.bpr >> 27) & 0xF);
        if (!mgr->session_active) {
            rv = soc_sbx_caladan3_ucode_debug_bp_hit(unit, snum, inum);
            /*LOG_CLI((BSL_META_U(unit,
                                  "\nBreak point hit inst %d on str %d (%x)"), 
                       inum, snum, 
                       mgr->curr_context->registers.bpr));
             */
            if (rv == SOC_E_NONE) {
                mgr->session_active = 1;
                mgr->context_valid = 1;
            }
        } else {
            if (mgr->curr_frame.bp) {
                soc_sbx_caladan3_ucode_debug_bp_update(unit, mgr->curr_frame.snum, mgr->curr_frame.inum);
                mgr->curr_frame.bp = 0;
            }
            mgr->context_valid = 1;
            mgr->curr_frame.inum = inum;
            mgr->curr_frame.snum = snum;
            if (mgr->curr_frame.cont) {
                soc_sbx_caladan3_ucode_debug_continue_on(unit, mgr);
            }
            /*LOG_CLI((BSL_META_U(unit,
                                  " > Str = %d Inst num = %d\n"), snum, inum));
             LOG_CLI((BSL_META_U(unit,
                                 " > Instruction   = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"), 
                      mgr->curr_frame.orig_inst[0], mgr->curr_frame.orig_inst[1], mgr->curr_frame.orig_inst[2],
                      mgr->curr_frame.orig_inst[3], mgr->curr_frame.orig_inst[4], mgr->curr_frame.orig_inst[5],
                      mgr->curr_frame.orig_inst[6], mgr->curr_frame.orig_inst[7], mgr->curr_frame.orig_inst[8],
                      mgr->curr_frame.orig_inst[9],mgr->curr_frame.orig_inst[10],mgr->curr_frame.orig_inst[11]));
             */
        }
   
    }
    UCODE_DEBUG_UNLOCK(unit);
    return rv;
}

void
soc_sbx_caladan3_ucode_debug_isr(int unit)
{
    debugmgr_t *mgr;
    mgr = &_dbgmgr[unit];

    sal_sem_give(mgr->cready);
    mgr->ready_int++;
}

int 
soc_sbx_caladan3_ucode_debug_server_init(int unit) 
{
    debugmgr_t *mgr;
    mgr = &_dbgmgr[unit];    
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_server_init: debug not initialized\n")));
    }
    /* spawn off a thread to handle the debugging session */
    return SOC_E_NONE;
}


int 
soc_sbx_caladan3_ucode_debug_bp_set(int unit, int snum, int inum, int *bpnum) 
{
    void *handle = INT_TO_PTR(unit);
    debugmgr_t *mgr;
    bp_t *bp = NULL;
    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_bp_set: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (bp_find(unit, snum, inum) != NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_bp_set: breakpoint already exists\n")));
        return SOC_E_EXISTS;
    }
    bp = bp_alloc(unit);
    if (!bp) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_bp_set: memory alloc failed\n")));
        return SOC_E_MEMORY;
    }
    mgr->ucodemgr->lr_iread(handle, snum, inum, bp->orig_inst);
    if (UCODE_DEBUG_CHECK_SWITCH(bp->orig_inst)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_bp_set: cant break on switch\n")));
        bp_free(bp);
        return SOC_E_PARAM;
    }
    bp->active = 1;
    bp->snum = snum;
    bp->inum = inum;
    *bpnum = bp->bpnum;

    /* use orig_inst task in breakpoint instruction */
    if (!(UCODE_DEBUG_CHECK_NOP(bp->orig_inst))) {
        bp->break_inst[0] = (bp->break_inst[0] & 0x1f) | (bp->orig_inst[0] & 0xe0);
        if (!(UCODE_DEBUG_CHECK_NOPREDICATION(bp->orig_inst))) {
            LOG_CLI((BSL_META_U(unit,
                                "WARNING: Predication at breakpoint detected\n")));
            bp->break_inst[4] = (bp->break_inst[4] & 0x00) | (bp->orig_inst[4] & 0xff);
            bp->break_inst[5] = (bp->break_inst[5] & 0x1f) | (bp->orig_inst[5] & 0xe0);
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "Set breakpoint %d\n"), bp->bpnum));
    LOG_CLI((BSL_META_U(unit,
                        " >> Raw Instruction   = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
             bp->orig_inst[0], bp->orig_inst[1], bp->orig_inst[2],
             bp->orig_inst[3], bp->orig_inst[4], bp->orig_inst[5],
             bp->orig_inst[6], bp->orig_inst[7], bp->orig_inst[8],
             bp->orig_inst[9], bp->orig_inst[10],bp->orig_inst[11]));
    LOG_CLI((BSL_META_U(unit,
                        " >> Break Instruction = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n"),
             bp->break_inst[0], bp->break_inst[1], bp->break_inst[2],
             bp->break_inst[3], bp->break_inst[4], bp->break_inst[5],
             bp->break_inst[6], bp->break_inst[7], bp->break_inst[8],
             bp->break_inst[9], bp->break_inst[10],bp->break_inst[11]));

    mgr->ucodemgr->lr_iwrite(handle, snum, inum, bp->break_inst);
    if (!mgr->session_active)
        soc_sbx_caladan3_lr_isr_enable(unit);
    LOG_CLI((BSL_META_U(unit,
                        "Created breakpoint number: %d\n"), bp->bpnum));
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_bp_clear(int unit, int bpnum) 
{
     void *handle = INT_TO_PTR(unit);
     uint8 temp_inst[12] = {0};
     debugmgr_t *mgr;
     bp_t *bp = NULL;
     mgr = &_dbgmgr[unit];
     if (!mgr->init_done) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_clear: debug not initialized\n")));
         return SOC_E_INIT;
     }
     bp = bp_findn(unit, bpnum);
     if (!bp) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_clear: breakpoint clear failed\n")));
         return SOC_E_NOT_FOUND;
     }
     
     mgr->ucodemgr->lr_iread(handle, bp->snum, bp->inum, temp_inst);
     if (UCODE_DEBUG_CHECK_BREAK(temp_inst)) {
         bp->active = 0;
         mgr->ucodemgr->lr_iwrite(handle, bp->snum, bp->inum, bp->orig_inst);
         bp_free(bp);
     } else {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_clear: bp ucode mismatch\n")));
         /* This should probably abort */
         return SOC_E_INTERNAL;
     }
     return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_bp_cleari(int unit, int snum, int inum) 
{
     void *handle = INT_TO_PTR(unit);
     uint8 temp_inst[12] = {0};
     debugmgr_t *mgr;
     bp_t *bp = NULL;
     mgr = &_dbgmgr[unit];
     if (!mgr->init_done) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_cleari: debug not initialized\n")));
         return SOC_E_INIT;
     }
     bp = bp_find(unit, snum, inum);
     if (!bp) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_cleari: breakpoint clear failed\n")));
         return SOC_E_NOT_FOUND;
     }
     
     mgr->ucodemgr->lr_iread(handle, snum, inum, temp_inst);
     if (UCODE_DEBUG_CHECK_BREAK(temp_inst)) {
         bp->active = 0;
         mgr->ucodemgr->lr_iwrite(handle, snum, inum, bp->orig_inst);
         bp_free(bp);
     } else {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_cleari: bp ucode mismatch\n")));
         /* This should probably abort */
         return SOC_E_INTERNAL;
     }
     return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_bp_update(int unit, int snum, int inum)
{
     void *handle = INT_TO_PTR(unit);
     debugmgr_t *mgr;
     bp_t *bp = NULL;
     mgr = &_dbgmgr[unit];
     if (!mgr->init_done) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_update: debug not initialized\n")));
         return SOC_E_INIT;
     }
     bp = bp_find(unit, snum, inum);
     if (!bp) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_update: breakpoint not found \n")));
         return SOC_E_NOT_FOUND;
     }
     if (!bp->active) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_update: not a active break pointt\n")));
         return SOC_E_INTERNAL;
     }
     mgr->ucodemgr->lr_iwrite(handle, snum, inum, bp->break_inst);
     return SOC_E_NONE;
}


int 
soc_sbx_caladan3_ucode_debug_bp_hit(int unit, int snum, int inum)
{
     debugmgr_t *mgr;
     bp_t *bp = NULL;
     mgr = &_dbgmgr[unit];
     if (!mgr->init_done) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_hit: debug not initialized\n")));
         return SOC_E_INIT;
     }
     bp = bp_find(unit, snum, inum);
     if (!bp) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_hit: breakpoint not found (s:%d i:%d \n"), snum, inum));
         return SOC_E_NOT_FOUND;
     }
     if (!bp->active) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_bit: not a active break pointt\n")));
         return SOC_E_INTERNAL;
     }
     LOG_CLI((BSL_META_U(unit,
                         "LRP Breakpoint hit: \n")));
     bp->hits++ ;
     sal_memcpy(mgr->curr_frame.orig_inst, bp->orig_inst, 12);
     mgr->curr_frame.bp = 1;
     mgr->curr_frame.inum = inum;
     mgr->curr_frame.snum = snum;
     soc_sbx_caladan3_ucode_debug_taskmap_get(unit);
     mgr->direction = IS_INGRESS(mgr->taskmap, inum);
     bp_dump(bp);
     if ((UCODE_DEBUG_CHECK_BRANCH(mgr->curr_frame.orig_inst)) && 
          !(UCODE_DEBUG_CHECK_NOP(mgr->curr_frame.orig_inst))) {
         if (mgr->curr_frame.snum != UCODE_DEBUG_GET_BRANCH_STREAM(mgr->curr_frame.orig_inst)) {
             LOG_CLI((BSL_META_U(unit,
                                 "********************************************************************************************\n")));
             LOG_CLI((BSL_META_U(unit,
                                 "WARNING: Potential stream slip detected\n")));
             LOG_CLI((BSL_META_U(unit,
                                 "WARNING: single stepping stream slip is not supported yet!!!\n")));
             LOG_CLI((BSL_META_U(unit,
                                 "********************************************************************************************\n")));
         }
     }
     return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_bp_clear_all(int unit) 
{
    debugmgr_t *mgr;
    dq_p_t elem;
    bp_t *bp = NULL;
    void *handle = INT_TO_PTR(unit);
    
    mgr = &_dbgmgr[unit];
    DQ_TRAVERSE(&mgr->bplist, elem) {
        bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
        if (bp) {
            bp->active = 0;
            mgr->ucodemgr->lr_iwrite(handle, bp->snum, bp->inum, bp->orig_inst);
            bp_free(bp);
        }
    } DQ_TRAVERSE_END(&mgr->bplist, elem);
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_step(int unit, debugmgr_t *mgr) 
{
    int rv = 0;
    int inum = 0;
    void *handle = INT_TO_PTR(unit);
    session_data_t cmd_msg;
    bp_t *bp = NULL;
    
    /* put back old, replace next one */
    rv = mgr->ucodemgr->lr_iwrite(handle, mgr->curr_frame.snum,
                         mgr->curr_frame.inum, mgr->curr_frame.orig_inst);
    if (mgr->direction) {
        inum = find_next_ingress_inst(mgr, mgr->curr_frame.inum);
    } else {
        inum = find_next_egress_inst(mgr, mgr->curr_frame.inum);
    }
    if (inum < 0) {
        LOG_CLI((BSL_META_U(unit,
                            "Error: Could not detect next instruction after inst=%d, duirection=%s\n"), mgr->curr_frame.inum, mgr->direction ? "ingress" : "egress"));
        return SOC_E_INTERNAL;
    }
    if (SOC_SUCCESS(rv)) {
        rv = mgr->ucodemgr->lr_iread(handle, mgr->curr_frame.snum,
                             inum, mgr->curr_frame.orig_inst);
        if (SOC_SUCCESS(rv)) {
            if (UCODE_DEBUG_CHECK_SWITCH(mgr->curr_frame.orig_inst)) {
                if (mgr->direction) {
                    inum = find_next_ingress_inst(mgr, inum);
                } else {
                    inum = find_next_egress_inst(mgr, inum);
                }
                if (inum < 0) {
                    LOG_CLI((BSL_META_U(unit,
                                        "Error: Could not detect next instruction after inst=%d, duirection=%s\n"), 
                             mgr->curr_frame.inum, 
                             mgr->direction ? "ingress" : "egress"));
                    return SOC_E_INTERNAL;
                }
                rv = mgr->ucodemgr->lr_iread(handle, mgr->curr_frame.snum,
                                        inum, mgr->curr_frame.orig_inst);
            }
            if (SOC_SUCCESS(rv)) {
                if (UCODE_DEBUG_CHECK_BREAK(mgr->curr_frame.orig_inst)) {
                    bp = bp_find(unit, mgr->curr_frame.snum, inum);
                    if (!bp) {
                        LOG_CLI((BSL_META_U(unit,
                                            "Failed locating bp corresponding to instruction at Str(%d) inst(%d)\n"), mgr->curr_frame.snum, inum));
                        return SOC_E_INTERNAL;
                    }
                    sal_memcpy(mgr->curr_frame.orig_inst, bp->orig_inst, 12);
                } else {
                    mgr->ucodemgr->lr_iwrite(handle, mgr->curr_frame.snum,
                                             inum, break_inst);
                }
            }
            if ((UCODE_DEBUG_CHECK_BRANCH(mgr->curr_frame.orig_inst)) && 
                    !(UCODE_DEBUG_CHECK_NOP(mgr->curr_frame.orig_inst))) {
                if (mgr->curr_frame.snum != UCODE_DEBUG_GET_BRANCH_STREAM(mgr->curr_frame.orig_inst)) {
                    LOG_CLI((BSL_META_U(unit,
                                        "********************************************************************************************\n")));
                    LOG_CLI((BSL_META_U(unit,
                                        "WARNING: Potential stream slip detected\n")));
                    LOG_CLI((BSL_META_U(unit,
                                        "WARNING: single stepping stream slip is not supported yet!!!\n")));
                    LOG_CLI((BSL_META_U(unit,
                                        "********************************************************************************************\n")));
                }
            }
        }
    }
    /* update context and let go */
    cmd_msg.cmd_info = UCODE_SET_COMMAND(SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_CONTINUE);
    cmd_msg.pe_info = UCODE_SET_SESSION(mgr->session_info.pe_info);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\nWriting command port %d segment %d 0x%x 0x%x\n"), mgr->cmd_port, mgr->cmd_segment,
                 cmd_msg.pe_info, cmd_msg.cmd_info));
    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, 
                                             mgr->cmd_segment, 0, 0, 
                                             (uint32*)&cmd_msg);
    mgr->context_valid = 0;
    soc_sbx_caladan3_lr_isr_enable(unit);
 
    return rv;
}
int 
soc_sbx_caladan3_ucode_debug_next(int unit) 
{
    int rv;
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_next: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (!mgr->context_valid) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_next: context is not available\n")));
        return SOC_E_INIT;
    }
    rv = soc_sbx_caladan3_ucode_debug_step(unit, mgr);
    return rv;
}

int 
soc_sbx_caladan3_ucode_debug_clr_session(int unit) 
{
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_cont: debug not initialized\n")));
        return SOC_E_INIT;
    }
    mgr->session_active = 0;
    mgr->session_info.pe_info = 0;
    mgr->session_info.cmd_info = 0;
    mgr->ready_int = 0;
    mgr->direction = 0;
    mgr->context_valid = 0;
    sal_memset(&mgr->curr_frame, 0, sizeof(mgr->curr_frame));
    sal_memset(mgr->curr_context, 0, sizeof(context_t));
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_end(int unit) 
{
    int rv = 0;
    debugmgr_t *mgr;
    session_data_t  cmd_msg;
    void *handle = INT_TO_PTR(unit);

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_cont: debug not initialized\n")));
        return SOC_E_INIT;
    }
    /* put back all inst */
    rv = mgr->ucodemgr->lr_iwrite(handle, mgr->curr_frame.snum,
                         mgr->curr_frame.inum, mgr->curr_frame.orig_inst);

    soc_sbx_caladan3_ucode_debug_bp_clear_all(unit);
    soc_sbx_caladan3_ucode_debug_thread_stop(unit);

    /* Clear all session data */
    soc_sbx_caladan3_ucode_debug_clr_session(unit);
    if (mgr->lock)
        sal_mutex_destroy(mgr->lock);
    if (mgr->cready) 
         sal_sem_destroy(mgr->cready);
    if (mgr->tready) 
         sal_sem_destroy(mgr->tready);

    /* update context and let go */
    cmd_msg.cmd_info = UCODE_SET_COMMAND(SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_END);
    cmd_msg.pe_info = UCODE_SET_SESSION(mgr->session_info.pe_info);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\nWriting command port %d segment %d 0x%x 0x%x\n"), mgr->cmd_port, mgr->cmd_segment,
                 cmd_msg.pe_info, cmd_msg.cmd_info));
    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, mgr->cmd_segment, 0, 0, (uint32*)&cmd_msg);

    return rv;
}

int 
soc_sbx_caladan3_ucode_debug_continue_on(int unit, debugmgr_t *mgr) 
{
    int rv = 0;
    session_data_t  cmd_msg;
    void *handle = INT_TO_PTR(unit);

    /* put back old */
    rv = mgr->ucodemgr->lr_iwrite(handle, mgr->curr_frame.snum,
                         mgr->curr_frame.inum, mgr->curr_frame.orig_inst);

    /* update context and let go */
    cmd_msg.cmd_info = UCODE_SET_COMMAND(SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_CONTINUE);
    cmd_msg.pe_info = UCODE_SET_SESSION(mgr->session_info.pe_info);
 

    mgr->curr_frame.inum = 0;
    mgr->curr_frame.snum = 0;
    sal_memset(mgr->curr_frame.orig_inst, 0, 12);


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\nWriting command port %d segment %d 0x%x 0x%x\n"), mgr->cmd_port, mgr->cmd_segment,
                 cmd_msg.pe_info, cmd_msg.cmd_info));
    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, mgr->cmd_segment, 0, 0, (uint32*)&cmd_msg);

    soc_sbx_caladan3_ucode_debug_clr_session(unit);
    soc_sbx_caladan3_lr_isr_enable(unit);

    return rv;
}

int 
soc_sbx_caladan3_ucode_debug_continue(int unit) 
{
    int rv;
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_cont: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (!mgr->context_valid) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_cont: context is not available\n")));
        return SOC_E_INIT;
    }
    if (mgr->curr_frame.bp) {
        mgr->curr_frame.cont = 1;
        rv = soc_sbx_caladan3_ucode_debug_step(unit, mgr);
    } else {
        rv = soc_sbx_caladan3_ucode_debug_continue_on(unit, mgr);
    }
    return rv;
}

int 
soc_sbx_caladan3_ucode_debug_bp_set_condition(int unit, int snum, int inum, uint8 *inst) 
{
     void *handle = INT_TO_PTR(unit);
     debugmgr_t *mgr;
     bp_t *bp = NULL;
     mgr = &_dbgmgr[unit];
     if (!mgr->init_done) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_set: debug not initialized\n")));
         return SOC_E_INIT;
     }
     bp = bp_alloc(unit);
     if (!bp) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_set: memory alloc failed\n")));
         return SOC_E_MEMORY;
     }
     mgr->ucodemgr->lr_iread(handle, snum, inum, bp->orig_inst);
     if (UCODE_DEBUG_CHECK_SWITCH(bp->orig_inst)) {
         LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "soc_sbx_caladan3_ucode_debug_bp_set: cant break on switch\n")));
         bp_free(bp);
         return SOC_E_PARAM;
     }
     bp->active = 1;
     bp->conditional = 1;
     bp->snum = snum;
     bp->inum = inum;
     mgr->ucodemgr->lr_iwrite(handle, snum, inum, inst);
     soc_sbx_caladan3_lr_isr_enable(unit);
     return SOC_E_NONE;
}

int 
_map_rname_to_rnum(char *rname) 
{
    int rn = 0;
    int rbase = 2*MAX_GPR;
    if (sal_strcmp(rname, "pir")==0)
        rbase += 0;
    else if (sal_strcmp(rname, "psr")==0)
        rbase += 1 ;
    else if (sal_strcmp(rname, "tsr")==0)
        rbase += 2 ;
    else if (sal_strcmp(rname, "btr0")==0)
        rbase += 3 ;
    else if (sal_strcmp(rname, "btr1")==0)
        rbase += 4 ;
    else if (sal_strcmp(rname, "btr2")==0)
        rbase += 5 ;
    else if (sal_strcmp(rname, "ptpnr")==0)
        rbase += 6 ;
    else if (sal_strcmp(rname, "ptpsr")==0)
        rbase += 7 ;
    else if (sal_strcmp(rname, "ntpnr")==0)
        rbase += 8 ;
    else if (sal_strcmp(rname, "ntpsr")==0)
        rbase += 9 ;
    else if (sal_strcmp(rname, "rnr")==0)
        rbase += 10 ;
    else if (sal_strcmp(rname, "tnr")==0)
        rbase += 11 ;
    else if (sal_strcmp(rname, "gpv")==0)
        rbase += 12 ;
    else if (sal_strcmp(rname, "tpv")==0)
        rbase += 13 ;
    else if (sal_strcmp(rname, "bpr")==0)
        rbase += 14 ;
    else if (sal_strcmp(rname, "ibpr")==0)
        rbase += 15 ;
    else if (sal_strcmp(rname, "dttr")==0)
        rbase += 16 ;
    else if (sal_strcmp(rname, "dtsr")==0)
        rbase += 17 ;
    else if(rname[0] == 'c') {
        rbase += 18;
        rn += _shr_ctoi(&rname[1]);
        if ((rn < 0) || (rn > 31))
            rn = 0;
        rbase += rn;
    }
    else if(rname[0] == 'r') {
        rbase = 0;
        rn += _shr_ctoi(&rname[1]);
        if ((rn < 0) || (rn > 63))
            rn = 0;
        rbase += rn;
        /*rbase += (rn % 2) ? -1 : 1;*/
    } else if (rname[0] == 't') {
        rbase = MAX_GPR;
        rn += _shr_ctoi(&rname[1]);
        if ((rn < 0) || (rn > 63))
            rn = 0;
        rbase += rn;
        /*rbase += (rn % 2) ? -1 : 1;*/
    }
    return rbase;
}

int 
soc_sbx_caladan3_ucode_debug_get_reg(int unit, char* rname, uint32 *value) 
{
    int rnum = 0;
    debugmgr_t *mgr;
    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_reg: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (!mgr->context_valid) {
        *value = 0;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_reg: context is not available\n")));
        return SOC_E_INIT;
    }
    rnum = _map_rname_to_rnum(rname);
    *value = 0xdeadbeaf;
    if (rnum < (sizeof(c3regs_t)/sizeof(uint32))) {
        *value = get_register_value(mgr, rnum);
    }
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_set_reg(int unit, char *rname, uint32 value, int update_context) 
{
    int ra = 0, rb = 0;
    debugmgr_t *mgr;
    uint32 regval[2] = {0};
    session_data_t  cmd_msg;
    int rv;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_set_reg: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (!mgr->context_valid) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_set_reg: context is not available\n")));
        return SOC_E_INIT;
    }
    UCODE_DEBUG_LOCK(unit);    
    ra = _map_rname_to_rnum(rname);
    if (((ra >=0) && (ra < 128)) || (ra==172) || (ra==174)) {

        set_register_value(mgr, ra, value);

        if (ra & 1) {
            rb = ra - 1;
            regval[0] = get_register_value(mgr, rb);
            regval[1] = get_register_value(mgr, ra);
        } else {
            rb = ra + 1;
            regval[0] = get_register_value(mgr, ra);
            regval[1] = get_register_value(mgr, rb);
        }
        rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, 
                                             mgr->cmd_segment, 1, 1, 
                                             &regval[0]);
        if (SOC_SUCCESS(rv)) {
            cmd_msg.cmd_info = UCODE_SET_COMMAND(SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_SET_REGISTER);
            cmd_msg.pe_info = UCODE_SET_SESSION(mgr->session_info.pe_info);
            ra = (ra & 1) ? rb : ra;
#if 0
            LOG_CLI((BSL_META_U(unit,
                                "Setting register %s(%d) to %x:%x\n"), rname, ra, regval[0], regval[1]));
            LOG_CLI((BSL_META_U(unit,
                                "Setting cmd port %d seg %d to %x:%x\n"), mgr->cmd_port,
                     mgr->cmd_segment, regval[0], regval[1]));
#endif
            cmd_msg.cmd_info |= UCODE_SET_REGISTER(ra);
            rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, mgr->cmd_segment, 
                                                      0, 0, (uint32*)&cmd_msg);
#if 0
            LOG_CLI((BSL_META_U(unit,
                                "Setting cmd port %d seg %d to %x:%x\n"), mgr->cmd_port,
                     mgr->cmd_segment,cmd_msg.pe_info, cmd_msg.cmd_info));
#endif
            if (SOC_SUCCESS(rv)) {
                if (update_context) {
                    LOG_CLI((BSL_META_U(unit,
                                        "%s: Update context\n"), FUNCTION_NAME()));
                    mgr->context_valid = 0; 
                    soc_sbx_caladan3_lr_isr_enable(unit); 
                }
            }
        } else {
            mgr->context_valid = 0; 
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucode_debug_set_reg: cant write data to mem\n")));
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_set_reg: cant write to read-only/Special purpose register\n")));
    }
    UCODE_DEBUG_UNLOCK(unit);    
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ucode_debug_get_symbol(int unit, char *name, uint32 *value) 
{
    debugmgr_t *mgr;
    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_symbol: debug not initialized\n")));
        return SOC_E_INIT;
    }
    return (soc_sbx_caladan3_ucodemgr_sym_get(unit, mgr->ucodemgr->ucode, name, value));
}

int 
soc_sbx_caladan3_ucode_debug_set_symbol(int unit, char *name, uint32 value) 
{
    debugmgr_t *mgr;
    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_set_symbol: debug not initialized\n")));
        return SOC_E_INIT;
    }
    return (soc_sbx_caladan3_ucodemgr_sym_set(unit, mgr->ucodemgr->ucode, name, value));
}

int 
soc_sbx_caladan3_ucode_debug_get_header(int unit, uint8 **data) 
{
    debugmgr_t *mgr;
    int i;
    uint8 *buf;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_hdr: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (!mgr->context_valid) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_hdr: context is not available\n")));
        return SOC_E_INIT;
    }
    buf = sal_alloc(256, "c3 debug pkt hdr");
    if (!buf) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_hdr: no memory available\n")));
        return SOC_E_MEMORY;
    }
    for (i=0; i< 256; i += 8) {
        buf[i+4] = (mgr->curr_context->descriptor[i/4] >> 24) & 0xFF;
        buf[i+5] = (mgr->curr_context->descriptor[i/4] >> 16) & 0xFF;
        buf[i+6] = (mgr->curr_context->descriptor[i/4] >> 8) & 0xFF;
        buf[i+7] = mgr->curr_context->descriptor[i/4] & 0xFF;

        buf[i+0] = (mgr->curr_context->descriptor[(i + 4)/4] >> 24) & 0xFF;
        buf[i+1] = (mgr->curr_context->descriptor[(i + 4)/4] >> 16) & 0xFF;
        buf[i+2] = (mgr->curr_context->descriptor[(i + 4)/4] >> 8) & 0xFF;
        buf[i+3] = mgr->curr_context->descriptor[(i + 4)/4] & 0xFF;

    }
    *data = buf;
    return SOC_E_NONE;
}


int 
soc_sbx_caladan3_ucode_debug_set_header(int unit, uint32 *header) 
{
    debugmgr_t *mgr;
    session_data_t  cmd_msg;
    int rv;

    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_hdr: debug not initialized\n")));
        return SOC_E_INIT;
    }
    if (!mgr->context_valid) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_sbx_caladan3_ucode_debug_get_hdr: context is not available\n")));
        return SOC_E_INIT;
    }
    

    UCODE_DEBUG_LOCK(unit);    
    /*
     * Write data 
     */
    rv = soc_sbx_caladan3_ocm_port_mem_write(unit, 
                                             mgr->cmd_port, 
                                             mgr->cmd_segment, 
                                             1, 
                                             64/2, 
                                             header);

    /*
     * Write command
     */
    if (SOC_SUCCESS(rv)) {
        cmd_msg.cmd_info = UCODE_SET_COMMAND(SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_SET_HEADER);
        cmd_msg.pe_info = UCODE_SET_SESSION(mgr->session_info.pe_info);
#if 0
        LOG_CLI((BSL_META_U(unit,
                            "Setting register %s(%d) to %x:%x\n"), rname, ra, regval[0], regval[1]));
        LOG_CLI((BSL_META_U(unit,
                            "Setting cmd port %d seg %d to %x:%x\n"), mgr->cmd_port,
                 mgr->cmd_segment, regval[0], regval[1]));
#endif
        rv = soc_sbx_caladan3_ocm_port_mem_write(unit, mgr->cmd_port, mgr->cmd_segment, 
                                                      0, 0, (uint32*)&cmd_msg);
#if 0
        LOG_CLI((BSL_META_U(unit,
                            "Setting cmd port %d seg %d to %x:%x\n"), mgr->cmd_port,
                 mgr->cmd_segment,cmd_msg.pe_info, cmd_msg.cmd_info));
#endif
        if (SOC_SUCCESS(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: Update context\n"), FUNCTION_NAME()));
            mgr->context_valid = 0; 
            soc_sbx_caladan3_lr_isr_enable(unit); 
        } else {
            mgr->context_valid = 0; 
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucode_debug_set_reg: cant write data to mem\n")));
        }
    }

    UCODE_DEBUG_UNLOCK(unit);    
    return SOC_E_NONE;
}


int
soc_sbx_caladan3_ucode_debug_dump_all(int unit, int mode) 
{
    debugmgr_t *mgr;
    int i;
    dq_p_t elem;
    bp_t *bp = NULL;
    char buf[256]={0};

    mgr = &_dbgmgr[unit];

    if (mode==1) {
        LOG_CLI((BSL_META_U(unit,
                            "Debug context info:\n")));
        LOG_CLI((BSL_META_U(unit,
                            "Mgr Details:\n")));
        LOG_CLI((BSL_META_U(unit,
                            " Session Active = %d\n"), mgr->session_active));
        LOG_CLI((BSL_META_U(unit,
                            " PE info        = 0x%x\n"), (mgr->session_info.pe_info & 0xFFF)));
        LOG_CLI((BSL_META_U(unit,
                            " Ready int      = %d\n"), mgr->ready_int));
        if (mgr->session_active) {
            LOG_CLI((BSL_META_U(unit,
                                " Direction      = %s\n"), mgr->direction ? "ingress" : "egress"));
        } else {
            LOG_CLI((BSL_META_U(unit,
                                " Direction      = not-ready\n")));
        }
        LOG_CLI((BSL_META_U(unit,
                            " Init state     = %d\n"), mgr->init_done));
        LOG_CLI((BSL_META_U(unit,
                            " Num of bp      = %d\n"), mgr->num_bp));
        LOG_CLI((BSL_META_U(unit,
                            " Context Valid  = %d\n"), mgr->context_valid));
        LOG_CLI((BSL_META_U(unit,
                            " Cmd Port       = %d\n"), mgr->cmd_port));
        LOG_CLI((BSL_META_U(unit,
                            " Cmd segment    = %d\n"), mgr->cmd_segment));
        LOG_CLI((BSL_META_U(unit,
                            " Rsp port       = %d\n"), mgr->rsp_port));
        LOG_CLI((BSL_META_U(unit,
                            " Rsp segment    = %d\n"), mgr->rsp_segment));
        LOG_CLI((BSL_META_U(unit,
                            "Break point list:")));
        DQ_TRAVERSE(&mgr->bplist, elem) {
            bp = DQ_ELEMENT(bp_t*, elem, bp, chain);
            bp_dump(bp);
        } DQ_TRAVERSE_END(&mgr->bplist, elem);
    }

    LOG_CLI((BSL_META_U(unit,
                        "Current Frame:\n")));
    LOG_CLI((BSL_META_U(unit,
                        " Stream  = %d\n"), mgr->curr_frame.snum));
    LOG_CLI((BSL_META_U(unit,
                        " Inst    = %d\n"), mgr->curr_frame.inum));
    LOG_CLI((BSL_META_U(unit,
                        " BP      = %d\n"), mgr->curr_frame.bp));
    soc_sbx_caladan3_ucode_debug_decode_inst(buf, mgr->curr_frame.orig_inst);
    LOG_CLI((BSL_META_U(unit,
                        " Cont    = %d\n"), mgr->curr_frame.cont));
    LOG_CLI((BSL_META_U(unit,
                        " Inst    = %s\n"), buf));
    LOG_CLI((BSL_META_U(unit,
                        " RawInst = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n"), 
             mgr->curr_frame.orig_inst[0], mgr->curr_frame.orig_inst[1], mgr->curr_frame.orig_inst[2],
             mgr->curr_frame.orig_inst[3], mgr->curr_frame.orig_inst[4], mgr->curr_frame.orig_inst[5],
             mgr->curr_frame.orig_inst[6], mgr->curr_frame.orig_inst[7], mgr->curr_frame.orig_inst[8],
             mgr->curr_frame.orig_inst[9],mgr->curr_frame.orig_inst[10],mgr->curr_frame.orig_inst[11]));
    LOG_CLI((BSL_META_U(unit,
                        "Register Dump:\n")));
    LOG_CLI((BSL_META_U(unit,
                        " r0  = %08x"), mgr->curr_context->registers. r0));
    LOG_CLI((BSL_META_U(unit,
                        " r1  = %08x"), mgr->curr_context->registers. r1)); 
    LOG_CLI((BSL_META_U(unit,
                        " r2  = %08x"), mgr->curr_context->registers. r2));
    LOG_CLI((BSL_META_U(unit,
                        " r3  = %08x\n"), mgr->curr_context->registers. r3)); 
    LOG_CLI((BSL_META_U(unit,
                        " r4  = %08x"), mgr->curr_context->registers. r4));
    LOG_CLI((BSL_META_U(unit,
                        " r5  = %08x"), mgr->curr_context->registers. r5)); 
    LOG_CLI((BSL_META_U(unit,
                        " r6  = %08x"), mgr->curr_context->registers. r6));
    LOG_CLI((BSL_META_U(unit,
                        " r7  = %08x\n"), mgr->curr_context->registers. r7)); 
    LOG_CLI((BSL_META_U(unit,
                        " r8  = %08x"), mgr->curr_context->registers. r8));
    LOG_CLI((BSL_META_U(unit,
                        " r9  = %08x"), mgr->curr_context->registers. r9)); 
    LOG_CLI((BSL_META_U(unit,
                        " r10 = %08x"), mgr->curr_context->registers.r10));
    LOG_CLI((BSL_META_U(unit,
                        " r11 = %08x\n"), mgr->curr_context->registers.r11));  
    LOG_CLI((BSL_META_U(unit,
                        " r12 = %08x"), mgr->curr_context->registers.r12));
    LOG_CLI((BSL_META_U(unit,
                        " r13 = %08x"), mgr->curr_context->registers.r13));  
    LOG_CLI((BSL_META_U(unit,
                        " r14 = %08x"), mgr->curr_context->registers.r14));
    LOG_CLI((BSL_META_U(unit,
                        " r15 = %08x\n"), mgr->curr_context->registers.r15));  
    LOG_CLI((BSL_META_U(unit,
                        " r16 = %08x"), mgr->curr_context->registers.r16));
    LOG_CLI((BSL_META_U(unit,
                        " r17 = %08x"), mgr->curr_context->registers.r17));  
    LOG_CLI((BSL_META_U(unit,
                        " r18 = %08x"), mgr->curr_context->registers.r18));
    LOG_CLI((BSL_META_U(unit,
                        " r19 = %08x\n"), mgr->curr_context->registers.r19));  
    LOG_CLI((BSL_META_U(unit,
                        " r20 = %08x"), mgr->curr_context->registers.r20));
    LOG_CLI((BSL_META_U(unit,
                        " r21 = %08x"), mgr->curr_context->registers.r21));  
    LOG_CLI((BSL_META_U(unit,
                        " r22 = %08x"), mgr->curr_context->registers.r22));
    LOG_CLI((BSL_META_U(unit,
                        " r23 = %08x\n"), mgr->curr_context->registers.r23));  
    LOG_CLI((BSL_META_U(unit,
                        " r24 = %08x"), mgr->curr_context->registers.r24));
    LOG_CLI((BSL_META_U(unit,
                        " r25 = %08x"), mgr->curr_context->registers.r25));  
    LOG_CLI((BSL_META_U(unit,
                        " r26 = %08x"), mgr->curr_context->registers.r26));
    LOG_CLI((BSL_META_U(unit,
                        " r27 = %08x\n"), mgr->curr_context->registers.r27));  
    LOG_CLI((BSL_META_U(unit,
                        " r28 = %08x"), mgr->curr_context->registers.r28));
    LOG_CLI((BSL_META_U(unit,
                        " r29 = %08x"), mgr->curr_context->registers.r29));  
    LOG_CLI((BSL_META_U(unit,
                        " r30 = %08x"), mgr->curr_context->registers.r30));
    LOG_CLI((BSL_META_U(unit,
                        " r31 = %08x\n"), mgr->curr_context->registers.r31));  
    LOG_CLI((BSL_META_U(unit,
                        " r32 = %08x"), mgr->curr_context->registers.r32));  
    LOG_CLI((BSL_META_U(unit,
                        " r33 = %08x"), mgr->curr_context->registers.r33));  
    LOG_CLI((BSL_META_U(unit,
                        " r34 = %08x"), mgr->curr_context->registers.r34));  
    LOG_CLI((BSL_META_U(unit,
                        " r35 = %08x\n"), mgr->curr_context->registers.r35));  
    LOG_CLI((BSL_META_U(unit,
                        " r36 = %08x"), mgr->curr_context->registers.r36));  
    LOG_CLI((BSL_META_U(unit,
                        " r37 = %08x"), mgr->curr_context->registers.r37));  
    LOG_CLI((BSL_META_U(unit,
                        " r38 = %08x"), mgr->curr_context->registers.r38));  
    LOG_CLI((BSL_META_U(unit,
                        " r39 = %08x\n"), mgr->curr_context->registers.r39));  
    LOG_CLI((BSL_META_U(unit,
                        " r40 = %08x"), mgr->curr_context->registers.r40));  
    LOG_CLI((BSL_META_U(unit,
                        " r41 = %08x"), mgr->curr_context->registers.r41));  
    LOG_CLI((BSL_META_U(unit,
                        " r42 = %08x"), mgr->curr_context->registers.r42));  
    LOG_CLI((BSL_META_U(unit,
                        " r43 = %08x\n"), mgr->curr_context->registers.r43));  
    LOG_CLI((BSL_META_U(unit,
                        " r44 = %08x"), mgr->curr_context->registers.r44));  
    LOG_CLI((BSL_META_U(unit,
                        " r45 = %08x"), mgr->curr_context->registers.r45));  
    LOG_CLI((BSL_META_U(unit,
                        " r46 = %08x"), mgr->curr_context->registers.r46));  
    LOG_CLI((BSL_META_U(unit,
                        " r47 = %08x\n"), mgr->curr_context->registers.r47));  
    LOG_CLI((BSL_META_U(unit,
                        " r48 = %08x"), mgr->curr_context->registers.r48));  
    LOG_CLI((BSL_META_U(unit,
                        " r49 = %08x"), mgr->curr_context->registers.r49));  
    LOG_CLI((BSL_META_U(unit,
                        " r50 = %08x"), mgr->curr_context->registers.r50));  
    LOG_CLI((BSL_META_U(unit,
                        " r51 = %08x\n"), mgr->curr_context->registers.r51));  
    LOG_CLI((BSL_META_U(unit,
                        " r52 = %08x"), mgr->curr_context->registers.r52));  
    LOG_CLI((BSL_META_U(unit,
                        " r53 = %08x"), mgr->curr_context->registers.r53));  
    LOG_CLI((BSL_META_U(unit,
                        " r54 = %08x"), mgr->curr_context->registers.r54));  
    LOG_CLI((BSL_META_U(unit,
                        " r55 = %08x\n"), mgr->curr_context->registers.r55));  
    LOG_CLI((BSL_META_U(unit,
                        " r56 = %08x"), mgr->curr_context->registers.r56));  
    LOG_CLI((BSL_META_U(unit,
                        " r57 = %08x"), mgr->curr_context->registers.r57));  
    LOG_CLI((BSL_META_U(unit,
                        " r58 = %08x"), mgr->curr_context->registers.r58));  
    LOG_CLI((BSL_META_U(unit,
                        " r59 = %08x\n"), mgr->curr_context->registers.r59));  
    LOG_CLI((BSL_META_U(unit,
                        " r60 = %08x"), mgr->curr_context->registers.r60));  
    LOG_CLI((BSL_META_U(unit,
                        " r61 = %08x"), mgr->curr_context->registers.r61));  
    LOG_CLI((BSL_META_U(unit,
                        " r62 = %08x"), mgr->curr_context->registers.r62));  
    LOG_CLI((BSL_META_U(unit,
                        " r63 = %08x\n\n"), mgr->curr_context->registers.r63));  
    LOG_CLI((BSL_META_U(unit,
                        " t0  = %08x"), mgr->curr_context->registers. t0));
    LOG_CLI((BSL_META_U(unit,
                        " t1  = %08x"), mgr->curr_context->registers. t1)); 
    LOG_CLI((BSL_META_U(unit,
                        " t2  = %08x"), mgr->curr_context->registers. t2));
    LOG_CLI((BSL_META_U(unit,
                        " t3  = %08x\n"), mgr->curr_context->registers. t3)); 
    LOG_CLI((BSL_META_U(unit,
                        " t4  = %08x"), mgr->curr_context->registers. t4));
    LOG_CLI((BSL_META_U(unit,
                        " t5  = %08x"), mgr->curr_context->registers. t5)); 
    LOG_CLI((BSL_META_U(unit,
                        " t6  = %08x"), mgr->curr_context->registers. t6));
    LOG_CLI((BSL_META_U(unit,
                        " t7  = %08x\n"), mgr->curr_context->registers. t7)); 
    LOG_CLI((BSL_META_U(unit,
                        " t8  = %08x"), mgr->curr_context->registers. t8));
    LOG_CLI((BSL_META_U(unit,
                        " t9  = %08x"), mgr->curr_context->registers. t9)); 
    LOG_CLI((BSL_META_U(unit,
                        " t10 = %08x"), mgr->curr_context->registers.t10));
    LOG_CLI((BSL_META_U(unit,
                        " t11 = %08x\n"), mgr->curr_context->registers.t11));  
    LOG_CLI((BSL_META_U(unit,
                        " t12 = %08x"), mgr->curr_context->registers.t12));
    LOG_CLI((BSL_META_U(unit,
                        " t13 = %08x"), mgr->curr_context->registers.t13));  
    LOG_CLI((BSL_META_U(unit,
                        " t14 = %08x"), mgr->curr_context->registers.t14));
    LOG_CLI((BSL_META_U(unit,
                        " t15 = %08x\n"), mgr->curr_context->registers.t15));  
    LOG_CLI((BSL_META_U(unit,
                        " t16 = %08x"), mgr->curr_context->registers.t16));
    LOG_CLI((BSL_META_U(unit,
                        " t17 = %08x"), mgr->curr_context->registers.t17));  
    LOG_CLI((BSL_META_U(unit,
                        " t18 = %08x"), mgr->curr_context->registers.t18));
    LOG_CLI((BSL_META_U(unit,
                        " t19 = %08x\n"), mgr->curr_context->registers.t19));  
    LOG_CLI((BSL_META_U(unit,
                        " t20 = %08x"), mgr->curr_context->registers.t20));
    LOG_CLI((BSL_META_U(unit,
                        " t21 = %08x"), mgr->curr_context->registers.t21));  
    LOG_CLI((BSL_META_U(unit,
                        " t22 = %08x"), mgr->curr_context->registers.t22));
    LOG_CLI((BSL_META_U(unit,
                        " t23 = %08x\n"), mgr->curr_context->registers.t23));  
    LOG_CLI((BSL_META_U(unit,
                        " t24 = %08x"), mgr->curr_context->registers.t24));
    LOG_CLI((BSL_META_U(unit,
                        " t25 = %08x"), mgr->curr_context->registers.t25));  
    LOG_CLI((BSL_META_U(unit,
                        " t26 = %08x"), mgr->curr_context->registers.t26));
    LOG_CLI((BSL_META_U(unit,
                        " t27 = %08x\n"), mgr->curr_context->registers.t27));  
    LOG_CLI((BSL_META_U(unit,
                        " t28 = %08x"), mgr->curr_context->registers.t28));
    LOG_CLI((BSL_META_U(unit,
                        " t29 = %08x"), mgr->curr_context->registers.t29));  
    LOG_CLI((BSL_META_U(unit,
                        " t30 = %08x"), mgr->curr_context->registers.t30));
    LOG_CLI((BSL_META_U(unit,
                        " t31 = %08x\n"), mgr->curr_context->registers.t31));  
    LOG_CLI((BSL_META_U(unit,
                        " t32 = %08x"), mgr->curr_context->registers.t32));  
    LOG_CLI((BSL_META_U(unit,
                        " t33 = %08x"), mgr->curr_context->registers.t33));  
    LOG_CLI((BSL_META_U(unit,
                        " t34 = %08x"), mgr->curr_context->registers.t34));  
    LOG_CLI((BSL_META_U(unit,
                        " t35 = %08x\n"), mgr->curr_context->registers.t35));  
    LOG_CLI((BSL_META_U(unit,
                        " t36 = %08x"), mgr->curr_context->registers.t36));  
    LOG_CLI((BSL_META_U(unit,
                        " t37 = %08x"), mgr->curr_context->registers.t37));  
    LOG_CLI((BSL_META_U(unit,
                        " t38 = %08x"), mgr->curr_context->registers.t38));  
    LOG_CLI((BSL_META_U(unit,
                        " t39 = %08x\n"), mgr->curr_context->registers.t39));  
    LOG_CLI((BSL_META_U(unit,
                        " t40 = %08x"), mgr->curr_context->registers.t40));  
    LOG_CLI((BSL_META_U(unit,
                        " t41 = %08x"), mgr->curr_context->registers.t41));  
    LOG_CLI((BSL_META_U(unit,
                        " t42 = %08x"), mgr->curr_context->registers.t42));  
    LOG_CLI((BSL_META_U(unit,
                        " t43 = %08x\n"), mgr->curr_context->registers.t43));  
    LOG_CLI((BSL_META_U(unit,
                        " t44 = %08x"), mgr->curr_context->registers.t44));  
    LOG_CLI((BSL_META_U(unit,
                        " t45 = %08x"), mgr->curr_context->registers.t45));  
    LOG_CLI((BSL_META_U(unit,
                        " t46 = %08x"), mgr->curr_context->registers.t46));  
    LOG_CLI((BSL_META_U(unit,
                        " t47 = %08x\n"), mgr->curr_context->registers.t47));  
    LOG_CLI((BSL_META_U(unit,
                        " t48 = %08x"), mgr->curr_context->registers.t48));  
    LOG_CLI((BSL_META_U(unit,
                        " t49 = %08x"), mgr->curr_context->registers.t49));  
    LOG_CLI((BSL_META_U(unit,
                        " t50 = %08x"), mgr->curr_context->registers.t50));  
    LOG_CLI((BSL_META_U(unit,
                        " t51 = %08x\n"), mgr->curr_context->registers.t51));  
    LOG_CLI((BSL_META_U(unit,
                        " t52 = %08x"), mgr->curr_context->registers.t52));  
    LOG_CLI((BSL_META_U(unit,
                        " t53 = %08x"), mgr->curr_context->registers.t53));  
    LOG_CLI((BSL_META_U(unit,
                        " t54 = %08x"), mgr->curr_context->registers.t54));  
    LOG_CLI((BSL_META_U(unit,
                        " t55 = %08x\n"), mgr->curr_context->registers.t55));  
    LOG_CLI((BSL_META_U(unit,
                        " t56 = %08x"), mgr->curr_context->registers.t56));  
    LOG_CLI((BSL_META_U(unit,
                        " t57 = %08x"), mgr->curr_context->registers.t57));  
    LOG_CLI((BSL_META_U(unit,
                        " t58 = %08x"), mgr->curr_context->registers.t58));  
    LOG_CLI((BSL_META_U(unit,
                        " t59 = %08x\n"), mgr->curr_context->registers.t59));  
    LOG_CLI((BSL_META_U(unit,
                        " t60 = %08x"), mgr->curr_context->registers.t60));  
    LOG_CLI((BSL_META_U(unit,
                        " t61 = %08x"), mgr->curr_context->registers.t61));  
    LOG_CLI((BSL_META_U(unit,
                        " t62 = %08x"), mgr->curr_context->registers.t62));  
    LOG_CLI((BSL_META_U(unit,
                        " t63 = %08x\n\n"), mgr->curr_context->registers.t63));  
    LOG_CLI((BSL_META_U(unit,
                        " c0  = %08x"), mgr->curr_context->registers. c0));
    LOG_CLI((BSL_META_U(unit,
                        " c1  = %08x"), mgr->curr_context->registers. c1));
    LOG_CLI((BSL_META_U(unit,
                        " c2  = %08x"), mgr->curr_context->registers. c2));
    LOG_CLI((BSL_META_U(unit,
                        " c3  = %08x\n"), mgr->curr_context->registers. c3));
    LOG_CLI((BSL_META_U(unit,
                        " c4  = %08x"), mgr->curr_context->registers. c4));
    LOG_CLI((BSL_META_U(unit,
                        " c5  = %08x"), mgr->curr_context->registers. c5));
    LOG_CLI((BSL_META_U(unit,
                        " c6  = %08x"), mgr->curr_context->registers. c6));
    LOG_CLI((BSL_META_U(unit,
                        " c7  = %08x\n"), mgr->curr_context->registers. c7));
    LOG_CLI((BSL_META_U(unit,
                        " c8  = %08x"), mgr->curr_context->registers. c8));
    LOG_CLI((BSL_META_U(unit,
                        " c9  = %08x"), mgr->curr_context->registers. c9));
    LOG_CLI((BSL_META_U(unit,
                        " c10 = %08x"), mgr->curr_context->registers.c10));
    LOG_CLI((BSL_META_U(unit,
                        " c11 = %08x\n"), mgr->curr_context->registers.c11));
    LOG_CLI((BSL_META_U(unit,
                        " c12 = %08x"), mgr->curr_context->registers.c12));
    LOG_CLI((BSL_META_U(unit,
                        " c13 = %08x"), mgr->curr_context->registers.c13));
    LOG_CLI((BSL_META_U(unit,
                        " c14 = %08x"), mgr->curr_context->registers.c14));
    LOG_CLI((BSL_META_U(unit,
                        " c15 = %08x\n"), mgr->curr_context->registers.c15));
    LOG_CLI((BSL_META_U(unit,
                        " c16 = %08x"), mgr->curr_context->registers.c16));
    LOG_CLI((BSL_META_U(unit,
                        " c17 = %08x"), mgr->curr_context->registers.c17));
    LOG_CLI((BSL_META_U(unit,
                        " c18 = %08x"), mgr->curr_context->registers.c18));
    LOG_CLI((BSL_META_U(unit,
                        " c19 = %08x\n"), mgr->curr_context->registers.c19));
    LOG_CLI((BSL_META_U(unit,
                        " c20 = %08x"), mgr->curr_context->registers.c20));
    LOG_CLI((BSL_META_U(unit,
                        " c21 = %08x"), mgr->curr_context->registers.c21));
    LOG_CLI((BSL_META_U(unit,
                        " c22 = %08x"), mgr->curr_context->registers.c22));
    LOG_CLI((BSL_META_U(unit,
                        " c23 = %08x\n"), mgr->curr_context->registers.c23));
    LOG_CLI((BSL_META_U(unit,
                        " c24 = %08x"), mgr->curr_context->registers.c24));
    LOG_CLI((BSL_META_U(unit,
                        " c25 = %08x"), mgr->curr_context->registers.c25));
    LOG_CLI((BSL_META_U(unit,
                        " c26 = %08x"), mgr->curr_context->registers.c26));
    LOG_CLI((BSL_META_U(unit,
                        " c27 = %08x\n"), mgr->curr_context->registers.c27));
    LOG_CLI((BSL_META_U(unit,
                        " c28 = %08x"), mgr->curr_context->registers.c28));
    LOG_CLI((BSL_META_U(unit,
                        " c29 = %08x"), mgr->curr_context->registers.c29));
    LOG_CLI((BSL_META_U(unit,
                        " c30 = %08x"), mgr->curr_context->registers.c30));
    LOG_CLI((BSL_META_U(unit,
                        " c31 = %08x\n\n"), mgr->curr_context->registers.c31));
    LOG_CLI((BSL_META_U(unit,
                        " pir   = %08x"), mgr->curr_context->registers.pir));  
    LOG_CLI((BSL_META_U(unit,
                        " psr   = %08x"), mgr->curr_context->registers.psr));
    LOG_CLI((BSL_META_U(unit,
                        " tsr   = %08x\n"), mgr->curr_context->registers.tsr));
    LOG_CLI((BSL_META_U(unit,
                        " btr0  = %08x"), mgr->curr_context->registers.btr0 ));
    LOG_CLI((BSL_META_U(unit,
                        " btr1  = %08x"), mgr->curr_context->registers.btr1 ));
    LOG_CLI((BSL_META_U(unit,
                        " btr2  = %08x\n"), mgr->curr_context->registers.btr2 ));
    LOG_CLI((BSL_META_U(unit,
                        " ptpnr = %08x"), mgr->curr_context->registers.ptpnr));
    LOG_CLI((BSL_META_U(unit,
                        " ptpsr = %08x"), mgr->curr_context->registers.ptpsr)); 
    LOG_CLI((BSL_META_U(unit,
                        " rnr   = %08x\n"), mgr->curr_context->registers.rnr));
    LOG_CLI((BSL_META_U(unit,
                        " ntpfr = %08x"), mgr->curr_context->registers.ntpfr));
    LOG_CLI((BSL_META_U(unit,
                        " ntpsr = %08x"), mgr->curr_context->registers.ntpsr));
    LOG_CLI((BSL_META_U(unit,
                        " tmr   = %08x\n"), mgr->curr_context->registers.tmr));
    LOG_CLI((BSL_META_U(unit,
                        " gpv   = %08x"), mgr->curr_context->registers.gpv));
    LOG_CLI((BSL_META_U(unit,
                        " tpv   = %08x"), mgr->curr_context->registers.tpv));
    LOG_CLI((BSL_META_U(unit,
                        " dttr  = %08x\n"), mgr->curr_context->registers.dttr ));
    LOG_CLI((BSL_META_U(unit,
                        " bpr   = %08x"), mgr->curr_context->registers.bpr));
    LOG_CLI((BSL_META_U(unit,
                        " ibpr  = %08x"), mgr->curr_context->registers.ibpr ));
    LOG_CLI((BSL_META_U(unit,
                        " dtsr  = %08x\n"), mgr->curr_context->registers.dtsr ));
       
    LOG_CLI((BSL_META_U(unit,
                        "\nHeader Dump: ")));
    for (i=0; i<64; i+=2) {
        if (i%4==0) {
            LOG_CLI((BSL_META_U(unit,
                                "\n 0x%02x:"), i*4));
        }
        LOG_CLI((BSL_META_U(unit,
                            " %08x"), mgr->curr_context->descriptor[i+1]));
        LOG_CLI((BSL_META_U(unit,
                            " %08x"), mgr->curr_context->descriptor[i]));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    return SOC_E_NONE;
}

/*
 * Debug server thread
 */
void
soc_sbx_caladan3_ucode_debug_thread(void *handle)
{
    int rv = 0;
    debugmgr_t *mgr;
    soc_sbx_caladan3_ucodemgr_t *ucodemgr = NULL;
    int unit = PTR_TO_INT(handle);

    /* Init processing */
    mgr = &_dbgmgr[unit];
    if (!mgr->init_done) {
        ucodemgr = soc_sbx_caladan3_lr_ucodemgr_get(unit);
        if (!ucodemgr) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucode_debug_thread: Failed to initialize, no ucode manager\n")));
            return;
        }
        rv = soc_sbx_caladan3_ucode_debug_init(unit, ucodemgr);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_sbx_caladan3_ucode_debug_thread: Failed to initialize, debug init failed\n")));
            return;
        }
    }

    /* Thread processing */
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "C3Debug thread started\n")));
    while (mgr->thread_run) {

        sal_sem_take(mgr->cready, sal_sem_FOREVER);

        rv = soc_sbx_caladan3_ucode_debug_pull_session(unit);
        if (SOC_SUCCESS(rv)) {
            if (UCODE_GET_COMMAND(mgr->session_info.cmd_info) == SOC_SBX_CALADAN3_UCODE_DEBUG_CMD_DUMP_CONTEXT) {
                soc_sbx_caladan3_ucode_debug_pull_context(unit);
            } else {
                soc_sbx_caladan3_ucode_debug_request_context(unit);
            }
            sal_sem_give(mgr->tready);
        }

    }
    mgr->thread_id = 0;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "C3Debug thread exited\n")));
    sal_thread_exit(0);

    /* Thread Ended */
}


void
soc_sbx_caladan3_ucode_debug_thread_stop(int unit) 
{
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];

    if (mgr->thread_id) {
        mgr->thread_run = 0;
    }
}

int 
soc_sbx_caladan3_ucode_debug_thread_start(int unit)
{
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];

    if (mgr->thread_id) {
        soc_sbx_caladan3_ucode_debug_thread_stop(unit);
    }

    mgr->thread_run = 1;
    mgr->thread_id = sal_thread_create("bcmC3Dbg",
                                  SAL_THREAD_STKSZ,
                                  mgr->thread_priority,
                                  soc_sbx_caladan3_ucode_debug_thread, INT_TO_PTR(unit));
    if (mgr->thread_id == SAL_THREAD_ERROR){
        LOG_CLI((BSL_META_U(unit,
                            "Could not start thread\n")));
        mgr->thread_id = 0;
        return SOC_E_MEMORY;
    }
    return 0;
}

void
soc_sbx_caladan3_ucode_debug_thread_priority_set(int unit, int priority)
{
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];
    mgr->thread_priority = priority;
}

int
soc_sbx_caladan3_ucode_debug_thread_priority_get(int unit)
{
    debugmgr_t *mgr;

    mgr = &_dbgmgr[unit];
    return (mgr->thread_priority);
}




#endif
