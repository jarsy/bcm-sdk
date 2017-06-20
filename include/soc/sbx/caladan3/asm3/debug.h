/*
 * $Id: debug.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: debug.h
 * Purpose: Caladan3 ucode debug manager
 */

#ifdef BCM_CALADAN3_SUPPORT
#ifndef _BCM_CALADAN3_DEBUG
#define _BCM_CALADAN3_DEBUG

#include <soc/sbx/caladan3/ocm.h>
#include <soc/types.h>
#include <soc/sbx/sbDq.h>

#define MAX_POLL_COUNT 100

typedef struct {
    uint32 pe_info;
    uint32 cmd_info;
} session_data_t;

/* Register map */
typedef struct {
    /* GPR */
    uint32 r0; uint32 r1; 
    uint32 r2; uint32 r3; 
    uint32 r4; uint32 r5; 
    uint32 r6; uint32 r7; 
    uint32 r8; uint32 r9; 
    uint32 r10; uint32 r11; 
    uint32 r12; uint32 r13; 
    uint32 r14; uint32 r15; 
    uint32 r16; uint32 r17; 
    uint32 r18; uint32 r19; 
    uint32 r20; uint32 r21; 
    uint32 r22; uint32 r23; 
    uint32 r24; uint32 r25; 
    uint32 r26; uint32 r27; 
    uint32 r28; uint32 r29; 
    uint32 r30; uint32 r31; 
    uint32 r32; uint32 r33; 
    uint32 r34; uint32 r35; 
    uint32 r36; uint32 r37; 
    uint32 r38; uint32 r39; 
    uint32 r40; uint32 r41; 
    uint32 r42; uint32 r43; 
    uint32 r44; uint32 r45; 
    uint32 r46; uint32 r47; 
    uint32 r48; uint32 r49; 
    uint32 r50; uint32 r51; 
    uint32 r52; uint32 r53; 
    uint32 r54; uint32 r55; 
    uint32 r56; uint32 r57; 
    uint32 r58; uint32 r59; 
    uint32 r60; uint32 r61; 
    uint32 r62; uint32 r63; 
    /* TPR */
    uint32 t0; uint32 t1; 
    uint32 t2; uint32 t3; 
    uint32 t4; uint32 t5; 
    uint32 t6; uint32 t7; 
    uint32 t8; uint32 t9; 
    uint32 t10; uint32 t11; 
    uint32 t12; uint32 t13; 
    uint32 t14; uint32 t15; 
    uint32 t16; uint32 t17; 
    uint32 t18; uint32 t19; 
    uint32 t20; uint32 t21; 
    uint32 t22; uint32 t23; 
    uint32 t24; uint32 t25; 
    uint32 t26; uint32 t27; 
    uint32 t28; uint32 t29; 
    uint32 t30; uint32 t31; 
    uint32 t32; uint32 t33; 
    uint32 t34; uint32 t35; 
    uint32 t36; uint32 t37; 
    uint32 t38; uint32 t39; 
    uint32 t40; uint32 t41; 
    uint32 t42; uint32 t43; 
    uint32 t44; uint32 t45; 
    uint32 t46; uint32 t47; 
    uint32 t48; uint32 t49; 
    uint32 t50; uint32 t51; 
    uint32 t52; uint32 t53; 
    uint32 t54; uint32 t55; 
    uint32 t56; uint32 t57; 
    uint32 t58; uint32 t59; 
    uint32 t60; uint32 t61; 
    uint32 t62; uint32 t63; 
    /* SPR */ 
    uint32 pir; 
    uint32 psr; 
    uint32 tsr;
    uint32 btr0; 
    uint32 btr2; 
    uint32 btr1;
    uint32 ptpsr; uint32 ptpnr; 
    uint32 ntpsr; uint32 ntpfr; 
    uint32 rnr;
    uint32 tmr; 
    uint32 gpv;
    uint32 tpv; 
    uint32 bpr;
    uint32 ibpr; 
    uint32 dttr;
    uint32 dtsr; 
    /* CRR */
    uint32 c0; 
    uint32 c1;
    uint32 c2; 
    uint32 c3;
    uint32 c4; 
    uint32 c5;
    uint32 c6; 
    uint32 c7;
    uint32 c8; 
    uint32 c9;
    uint32 c10; 
    uint32 c11;
    uint32 c12; 
    uint32 c13;
    uint32 c14; 
    uint32 c15;
    uint32 c16; 
    uint32 c17;
    uint32 c18; 
    uint32 c19;
    uint32 c20; 
    uint32 c21;
    uint32 c22; 
    uint32 c23;
    uint32 c24; 
    uint32 c25;
    uint32 c26; 
    uint32 c27;
    uint32 c28; 
    uint32 c29;
    uint32 c30; 
    uint32 c31;
} c3regs_t;


#define MAX_GPR 64

/*
 * LRP Ucode debug manager
 */
struct debugmgr_s;

typedef struct bp_s {
    dq_t chain;
    int snum;
    int inum;
    int bpnum;
    int mde_bpnum;
    int hits;
    int conditional;
    int active;
    uint8 orig_inst[12];
    uint8 break_inst[12];
    struct debugmgr_s *mgr;
} bp_t;

typedef struct frame_s {
    int snum;
    int inum;
    int cont;
    int bp;
    uint8 orig_inst[12];
} frame_t;

#define MAX_REGS  256
#define MAX_DESC  64
#define MAX_MAP   50

typedef struct context_s {
    c3regs_t registers;
    uint32  descriptor[MAX_DESC];
} context_t;

typedef struct taskmap_s {
    unsigned int map[MAX_MAP];
} taskmap_t;

typedef struct debugmgr_s {
     sal_sem_t cready;
    sal_sem_t tready;
    sal_mutex_t lock;
    uint32 direction;
    uint32 ready_int;
    uint32 init_done;
    uint32 num_bp;
    uint32 curr_bp;
    dq_t bplist;
    soc_sbx_caladan3_ucodemgr_t *ucodemgr;
    frame_t curr_frame;
    int context_valid;
    int session_active;
    session_data_t session_info;
    sbx_caladan3_ocm_port_e_t cmd_port;
    int cmd_segment;
    sbx_caladan3_ocm_port_e_t rsp_port;
    int rsp_segment;
    context_t *curr_context;
    taskmap_t taskmap;
    volatile sal_thread_t thread_id;
    volatile int thread_run;
    int thread_priority;
} debugmgr_t;


int soc_sbx_caladan3_ucode_debug_dump_all(int unit, int mode);
int soc_sbx_caladan3_ucode_debug_pull_context(int unit);
int soc_sbx_caladan3_ucode_debug_poll(int unit);
int soc_sbx_caladan3_ucode_debug_request_context(int unit);
int soc_sbx_caladan3_ucode_debug_enable(int unit);
int soc_sbx_caladan3_ucode_debug_bp_set(int unit, int snum, int inum, int *bpnum);
int soc_sbx_caladan3_ucode_debug_bp_clear(int unit, int bpnum);
int soc_sbx_caladan3_ucode_debug_bp_cleari(int unit, int snum, int inum);
int soc_sbx_caladan3_ucode_debug_bp_clear_all(int unit);
int soc_sbx_caladan3_ucode_debug_bp_list(int unit);
int soc_sbx_caladan3_ucode_debug_bp_list_get(int unit, uint32 count, uint32 *bpnum, uint32 *snum, uint32 *inum);
uint32 soc_sbx_caladan3_ucode_debug_bp_count(int unit);
int soc_sbx_caladan3_ucode_debug_bp_hit(int unit, int snum, int inum);
int soc_sbx_caladan3_ucode_debug_next(int unit);
void soc_sbx_caladan3_ucode_debug_isr(int unit);
int soc_sbx_caladan3_ucode_debug_continue(int unit);
int soc_sbx_caladan3_ucode_debug_get_reg(int unit, char* rname, uint32 *value);
int soc_sbx_caladan3_ucode_debug_set_reg(int unit, char *rname, uint32 value, int update_context);
int soc_sbx_caladan3_ucode_debug_init(int unit, 
                                      soc_sbx_caladan3_ucodemgr_t *ucodemgr);
int soc_sbx_caladan3_ucode_debug_get_header(int unit, uint8 **data);
int soc_sbx_caladan3_ucode_debug_set_header(int unit, uint32 *header);
int soc_sbx_caladan3_ucode_debug_get_symbol(int unit, char *name, uint32 *value) ;
int soc_sbx_caladan3_ucode_debug_set_symbol(int unit, char *name, uint32 value);
int mde_agent_start(int unit);
int mde_agent_stop(void);

int soc_sbx_caladan3_ucode_debug_end(int unit);
int soc_sbx_caladan3_ucode_debug_get_current_frame(int unit, int timeout, int*snum, int*inum, uint8 *inst);
int soc_sbx_caladan3_ucode_debug_thread_start(int unit);
void soc_sbx_caladan3_ucode_debug_thread_stop(int unit);
void soc_sbx_caladan3_ucode_debug_decode_inst(char *buf, uint8 *inst);
int soc_sbx_caladan3_ucode_debug_clr_session(int unit);
int soc_sbx_caladan3_ucode_debug_bp_replay(int unit);
#endif
#endif
