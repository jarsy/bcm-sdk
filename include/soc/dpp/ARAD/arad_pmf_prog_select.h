/* $Id: arad_pmf_prog_select.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __ARAD_PMF_PROG_SELECT_INCLUDED__
/* { */
#define __ARAD_PMF_PROG_SELECT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>

#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_pmf_low_level.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PMF_NOF_LINES                      ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_program_selection_lines, \
                                                                 nof_egress_pmf_program_selection_lines, nof_flp_program_selection_lines, nof_slb_program_selection_lines, \
                                                                 nof_vt_program_selection_lines, nof_tt_program_selection_lines)
#define ARAD_PMF_NOF_LINES_LEN                  ARAD_BIT_TO_U32(ARAD_PMF_NOF_LINES)
#define ARAD_PMF_NOF_LINES_MAX_ALL_STAGES_LEN   ARAD_BIT_TO_U32(ARAD_PMF_NOF_LINES_MAX_ALL_STAGES)
#define ARAD_PMF_NOF_LEVELS                     (ARAD_PMF_NOF_LINES)
#define ARAD_PMF_NOF_LINES_IN_LEVEL             (ARAD_PMF_NOF_LINES)



#define ARAD_PMF_SEL_HW_DATA_NOF_BITS    (SOC_IS_ARAD_B1_AND_BELOW(unit) && (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF)? 84 /* Special case in Arad Ingress PMF different from A+ */: \
                                          SOC_IS_ARAD_B1_AND_BELOW(unit) && (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)? 56 /* Special case in Arad Ingress FLP different from A+ */: \
                                              ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(ihb_pmf_program_selection_cam_mask_nof_bits,                                                     \
                                                                 egq_pmf_program_selection_cam_mask_nof_bits, ihb_flp_program_selection_cam_mask_nof_bits,                                 \
                                                                                          ihp_consistent_hashing_program_sel_tcam_mask_nof_bits,                                           \
                                                                                          ihp_vtt_1st_program_selection_tcam_mask_nof_bits,                                           \
                                                                                          ihp_vtt_2nd_program_selection_tcam_mask_nof_bits))

/* if present don't mark resource as allocate, just check if possible */
#define ARAD_PMF_SEL_ALLOC_CHECK_ONLY     0x1
/* line should be deleted */
#define ARAD_PMF_SEL_ALLOC_FOR_DELETE     0x2
/* only in special case where no lines in previous level - only remove at current level */
#define ARAD_PMF_SEL_ALLOC_DO_NOT_ADD     0x4
/* for TM application */
#define ARAD_PMF_SEL_ALLOC_TM             0x8
/* for program selection management advanced mode */
#define ARAD_PMF_SEL_ALLOC_ADVANCED_MODE  0x10
/* for program selection second pass */
#define ARAD_PMF_SEL_ALLOC_SECOND_PASS    0x20

/* TCAM DB profile nof bits */
#define ARAD_PMF_KEY_TCAM_DB_PROFILE_NOF_BITS                  (6)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PMF_PROGRAM_STATIC_INDEX_GET(nof_reserved_lines, pmf_pgm_default, pmf_pgm_type_ndx) \
    (pmf_pgm_type_ndx + nof_reserved_lines + pmf_pgm_default)


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* autocoder workaround declare empty struct inside a macro */
#define PARSER_WORKAROUND_PMF_PROG_SELECT \
struct ARAD_PMF_PSL_LINE_INFO_s;\
struct ARAD_PMF_PSL_LEVEL_INFO_s;
PARSER_WORKAROUND_PMF_PROG_SELECT
#undef PARSER_WORKAROUND_PMF_PROG_SELECT

typedef union
{
  /* Ingress PMF */
    ARAD_PP_IHB_PMF_PROGRAM_SELECTION_CAM_TBL_DATA ingress_pmf;

  /*2nd-Pass Ingress PMF */
    ARAD_PP_IHB_PMF_2ND_PASS_PROGRAM_SELECTION_CAM_TBL_DATA second_pass_ingress_pmf;

  /* Egress PMF */
    ARAD_PP_EGQ_PMF_PROGRAM_SELECTION_CAM_TBL_DATA egress_pmf;
    
    /* Ingress SLB */
    ARAD_PP_IHB_SLB_PROGRAM_SELECTION_CAM_TBL_DATA ingress_slb;
    
} ARAD_PMF_PSL;

typedef struct
{
  /*
   *  for each PFG what is the Db in use.
   */
  SHR_BITDCL pfgs_db_pmb[SOC_PPC_FP_NOF_PFGS_ARAD][ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS)];
  /*
   *  the Db always in use because they have no preselectors.
   */
  SHR_BITDCL default_db_pmb[ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS)];
  /* 
   * TM PFGs for the special usage of PMF by TM applications
   */
  uint32 pfgs_tm_bmp[ARAD_BIT_TO_U32(SOC_PPC_FP_NOF_PFGS_ARAD)];

  ARAD_PMF_PSL_LEVEL_INFO levels_info[2/*is-TM*/][ARAD_PMF_NOF_LEVELS_MAX_ALL_STAGES];

  ARAD_PMF_SEL_INIT_INFO init_info;

  ARAD_PMF_PSL second_pass_psl_info[SOC_PPC_FP_NOF_PFGS_ARAD];

} ARAD_PMF_PSL_INFO;

typedef struct
{
    /*
     * PMF-Program Copy Engine instructions 
     * for each program x cycle: 
     *   16 CE 
     *   - 8 * 16
     *   - 8 * 32
     *   where
     *   - 8 for LSB (72)
     *   - 8 for MSB (144)
     */

     
    uint32 ce_internal_table[SOC_PPC_NOF_FP_QUAL_TYPES+1][2];
 
    uint32 ce_signals_table[SOC_PPC_NOF_FP_QUAL_TYPES+1];

    ARAD_PMF_CE pgm_ce[ARAD_PMF_NOF_PROGS][ARAD_PMF_NOF_CYCLES][ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];

    ARAD_PMF_FES pgm_fes[ARAD_PMF_NOF_PROGS][ARAD_PMF_NOF_FES]; 

    SOC_PPC_FP_FEM_ENTRY fem_entry[ARAD_PMF_NOF_FEM_PGMS][ARAD_PMF_LOW_LEVEL_NOF_FEMS];

    ARAD_PMF_RESOURCE rsources;

    /* List of Databases per Program */
    SHR_BITDCL pgm_db_pmb[ARAD_PMF_NOF_PROGS][ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS)]; 

    ARAD_PMF_DB_INFO db_info[ARAD_PMF_NOF_DBS]; 

    SOC_SAND_OCC_BM_PTR  kaps_db_used;

    ARAD_PMF_PSL_INFO psl_info;

    /* FP info */
    ARAD_FP_INFO fp_info;

} ARAD_PMF;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

uint32
  arad_pmf_prog_select_init(
    SOC_SAND_IN  int                 unit
  );


uint32
  arad_pmf_prog_select_eth_init(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE     stage,
    SOC_SAND_IN  uint8                      is_tm_pmf_per_port_mode,
    SOC_SAND_INOUT  ARAD_PMF_SEL_INIT_INFO  *init_info
  );

uint32 
  arad_pmf_psl_map(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO *pfg_info,
      SOC_SAND_IN  uint8                    is_for_tm, 
      SOC_SAND_OUT ARAD_PMF_PSL         *psl
  );

uint32 
  arad_pmf_psl_add(
      SOC_SAND_IN  int             unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32               pfg_id,
      SOC_SAND_IN  uint32                       flags,
      SOC_SAND_IN  ARAD_PMF_PSL       *psl,
      SOC_SAND_OUT  uint8             *success
  );

uint32
  arad_pmf_sel_table_get(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint32  line_indx,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE  stage,
    SOC_SAND_OUT uint32  *valid,
    SOC_SAND_OUT uint32  *program,
    SOC_SAND_OUT ARAD_PMF_PSL *tbl_data
  );

uint32 
  arad_pmf_sel_line_valid_set(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 entry_offset,
      SOC_SAND_IN   uint8               is_valid,
      SOC_SAND_IN   uint32               valid_or_program
  );

uint32
  arad_pmf_prog_select_line_borders_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
    SOC_SAND_IN  uint8                    is_for_tm, 
    SOC_SAND_OUT  uint32  *line_ndx_min,
    SOC_SAND_OUT  uint32  *line_ndx_max
  );

uint32
  arad_pmf_prog_select_eth_section_nof_lines_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
    SOC_SAND_OUT uint32                   *nof_eth_lines
  );



uint32 
  arad_pmf_sel_line_hw_write(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 entry_offset,
      SOC_SAND_IN   struct ARAD_PMF_PSL_LINE_INFO_s      *line,
      SOC_SAND_IN   uint32                prog,
      SOC_SAND_IN   uint8               valid
  );


/*
 * commit last PSL changes
 */
uint32 
  arad_pmf_psl_hw_commit(
      SOC_SAND_IN  int             unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                       flags
  );

uint32 
  arad_pmf_sel_prog_alloc(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint8                    is_for_tm, 
      SOC_SAND_OUT   uint32              *alloced_prog_id,
      SOC_SAND_OUT   uint8               *found
  );

uint32 
  arad_pmf_sel_prog_free(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                       is_for_tm,
      SOC_SAND_IN   uint32              prog_id,
      SOC_SAND_IN  uint32                   pmf_pgm_to
  );

uint32
  arad_pmf_psl_pmf_pgms_get(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_INOUT  SOC_PPC_FP_DATABASE_INFO *database_info,
      SOC_SAND_IN  uint8                    is_default_db, /* Relevant for all lines */
      SOC_SAND_IN  uint8                    is_for_tm, 
      SOC_SAND_OUT uint32                  *pgm_bmp_used,
      SOC_SAND_OUT uint32                  *pgm_bmp_remain, /* List of PMF-Programs not to suppress */
      SOC_SAND_OUT uint32                   presel_bmp_update[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES_LEN], /* Bitmap of the Presel lines to update */
      SOC_SAND_OUT uint8                   *success
  );


/* 
 * At Egress stage, the action resolution is done 
 * according to the Key index: Key-B is always superior 
 * to Key-A. 
 * See if switching the keys is necessary
 */
uint32
  arad_pmf_psl_swicth_key_egq_get(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN  uint32                   pmf_pgm_from,
      SOC_SAND_IN  uint32                   db_id_ndx,
      SOC_SAND_IN  uint32                   db_strength,
      SOC_SAND_OUT uint8                    *switch_key_egq
  );

/* Duplicate the SW DB and HW of PMF-Programs */
uint32
  arad_pmf_psl_pmf_pgm_duplicate(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN  uint32                   pmf_pgm_from,
      SOC_SAND_IN  uint32                   pmf_pgm_to,
      SOC_SAND_IN  uint8                    switch_key_egq
  );

uint32
  arad_pmf_prog_select_pmf_pgm_borders_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
    SOC_SAND_IN  uint8                    is_for_tm, 
    SOC_SAND_OUT  uint32  *pmf_pgm_ndx_min,
    SOC_SAND_OUT  uint32  *pmf_pgm_ndx_max
  );

/* Update the Preselector line's PMF program at SW DB and HW */
uint32
  arad_pmf_psl_pmf_pgm_update(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN  uint32                       is_for_tm,
      SOC_SAND_IN  uint32                   presel_bmp_update[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES_LEN], /* Bitmap of the Presel lines to update */
      SOC_SAND_IN  uint8                    is_for_all_lines, /* scan all lines*/
      SOC_SAND_IN  uint32                   pmf_pgm_from,
      SOC_SAND_IN  uint32                   pmf_pgm_to
  );

uint32
arad_pmf_psl_second_pass_update(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_INOUT  SOC_PPC_FP_DATABASE_STAGGERED_INFO       *db_staggered_info,
      SOC_SAND_IN  uint32                   pmf_pgm_from,
      SOC_SAND_IN  uint32                   pmf_pgm_to
      );

uint32
  arad_pmf_counter_set(
      SOC_SAND_IN  int                      unit,
      SOC_SAND_IN  int                      command_id,
      SOC_SAND_IN  SOC_TMC_CNT_MODE_EG_TYPE eg_type, 
      SOC_SAND_IN  int                      counter_base);

void
ARAD_PMF_PSL_clear(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
    SOC_SAND_OUT ARAD_PMF_PSL *select_data,
    uint8 ethernet_or_tm_only, /* 0 - FALSE, 1 - Ethernet, 2 - TM */
    uint8 mask_all
);

void
ARAD_PMF_SEL_INIT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_SEL_INIT_INFO *info);

#if ARAD_DEBUG_IS_LVL1

/* 
 * flavor 
 *  0: print all
 *  1: don't print empty level
 *  2: print only lines
 */
void
  arad_pmf_prog_select_print_all(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
    SOC_SAND_IN  uint32                  flavor
  );

void
  arad_pmf_prog_select_test1(
    SOC_SAND_IN  int                 unit
  );

void
  arad_pmf_prog_select_test2(
    SOC_SAND_IN  int                 unit
  );



#endif /* ARAD_DEBUG_IS_LVL1 */
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PMF_PROG_SELECT_INCLUDED__*/
#endif


