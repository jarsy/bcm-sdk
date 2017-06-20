/* $Id: arad_egr_prge_mgmt.c,v 1.142.2.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/


#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_EGRESS


/*
 * This file contain the functionality of the management system for the egress editor programs.
 * The management system is in charge of manipulating programs to allow all the necessary programs
 * to load, by helping them share resources (at this time, the resources are LFEMs).
 *
 * To manage a program, it needs to be loaded using the management interface (arad_egr_prge_interface.h).
 * Adding a program is done in a script like process where a "state machine" in the management is moved
 * from it's initial state to a "Program start" state using arad_egr_prge_mgmt_build_program_start.
 * Then, the instructions of the program are added 1 after the other using the specified interface.
 * Finally, the "State machine" is returned to it's initial state using arad_egr_prge_mgmt_build_program_end.
 *
 * After all the un-managed programs finish loading, arad_egr_prge_mgmt_program_management_main is called
 * to run a back-tracking algorithm on different manipulations on the managed programs in order to load them
 * as well. 
 */

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#include <soc/dcmn/error.h>
#include <soc/dpp/drv.h>

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_stack.h>

#include <soc/dpp/ARAD/arad_egr_prge_mgmt.h>
#include <soc/dpp/ARAD/arad_egr_prog_editor.h>
#include <soc/dpp/ARAD/arad_egr_prge_interface.h>

#include <soc/dpp/ARAD/arad_sw_db.h>




/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_EGR_PRGE_MGMT_MAX_NOF_LFEMS_PER_PROGRAM 4

/* ARAD_EGR_PRGE_MAX_NOF_LFEM_TABLES^MAX_NOF_LFEMS_PER_PROGRAM * duplicated entries */

/* In Jericho we have more permutations than this define,
   but in practice we assume we will have enough permutations to succeed allocating all the programs.
   In case of failure this can be improved by choosing better the permutations to test
   (loop inside permutation_struct_build function should be smarter */
#define ARAD_EGR_PRGE_MGMT_MAX_NOF_PERMUTATIONS_PER_PROGRAM 16*4 


/*
 * Invalid values for marking exceptional values
 */

/*
 * Dependency offsets
 */
#define _ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R0           0
#define _ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R1           1
#define _ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R2           2
#define _ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R3           3
#define _ARAD_EGR_PRGE_MGMT_DEP_OFFSET_RSZ          4
#define _ARAD_EGR_PRGE_MGMT_DEP_OFFSET_CAT_SIZE     5

/*
 * Dependency flags
 */
#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_R0             _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R0)
#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_R1             _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R1)
#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_R2             _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R2)
#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_R3             _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_ARAD_EGR_PRGE_MGMT_DEP_OFFSET_R3)
#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_RSZ            _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_ARAD_EGR_PRGE_MGMT_DEP_OFFSET_RSZ)
#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_CAT_SIZE       _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_ARAD_EGR_PRGE_MGMT_DEP_OFFSET_CAT_SIZE)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#ifndef MIN
#define MIN(a, b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a)>(b)) ? (a) : (b))
#endif

#define _ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(_unit, _program) \
            (mgmt_programs[(_unit)][(_program)].management_type == ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_MANAGED)

#define _ARAD_EGR_PRGE_MGMT_IS_PROGRAM_LOADED_ONLY(_unit, _program) \
            (mgmt_programs[(_unit)][(_program)].management_type == ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_LOAD_ONLY)

#define _ARAD_EGR_PRGE_MGMT_IS_PROGRAM_OLD_INTERFACE(_unit, _program) \
            (mgmt_programs[(_unit)][(_program)].management_type == ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED)

#define _ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTR_IS_NOP(p_ce_interface_instr) (((p_ce_interface_instr)->src_select) == _ARAD_EGR_PRGE_MGMT_CE_DATA_SRC_NOP)
#define _ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTR_IS_NOP(p_alu_interface_instr) (((p_alu_interface_instr)->cmp_action) == ARAD_EGR_PROG_EDITOR_ALU_CMP_NONE)

#define _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET(_offset) (1<<(_offset))

#define _ARAD_EGR_PRGE_MGMT_MEM_LSB_FROM_ENTRY_GET(_unit, _entry) \
            (((_entry)>>1) & (SOC_IS_JERICHO(_unit) ? 0x3 : 0x1 ))

#define _ARAD_EGR_PRGE_MGMT_IS_INSTR_MEM_LSB_IN_BMP(_unit, _program, _instr) \
            (mgmt_programs[(_unit)][(_program)].graph[(_instr)].ce.anchor_ce_instr_mem_bmp & \
                (1 << _ARAD_EGR_PRGE_MGMT_MEM_LSB_FROM_ENTRY_GET((_unit), mgmt_programs[(_unit)][(_program)].graph[(_instr)].selected_entry)))

#define _ARAD_EGR_PRGE_MGMT_INSTR_USE_LFEM(_unit, _program, _instr) \
            (!(mgmt_programs[(_unit)][(_program)].graph[(_instr)].ce.is_nop)) \
             && (mgmt_programs[(_unit)][(_program)].graph[(_instr)].ce.ce_ifc.lfem < ARAD_EGR_PROG_EDITOR_LFEM_NULL)


#define _ARAD_EGR_PRGE_MGMT_CE_NOP_INSTR_LOAD(_unit, _instr_tbl)    \
do {                                                                \
    (_instr_tbl).src_select  = 0;                                   \
    (_instr_tbl).offset_base = 0;                                   \
    (_instr_tbl).offset_src  = 0;                                   \
    (_instr_tbl).size_src  = ARAD_EGR_PROG_EDITOR_SIZE_SRC_CONST_0; \
    (_instr_tbl).size_base   = 0;                                   \
    (_instr_tbl).fem_select  = ARAD_PP_EG_PROG_NOF_FEM(_unit);      \
} while (0)

#define _ARAD_EGR_PRGE_INTERFACE_ALU_NOP_INSTR_LOAD(_unit, _instr_tbl)    \
do {                                                                \
    (_instr_tbl).op_1_field_select  = 0;                            \
    (_instr_tbl).op_value           = 0;                            \
    (_instr_tbl).op_2_field_select  = 0;                            \
    (_instr_tbl).alu_action         = ARAD_PP_EG_PROG_ALU_SUB;      \
    (_instr_tbl).op_3_field_select  = 0;                            \
    (_instr_tbl).cmp_action         = 0;                            \
    (_instr_tbl).dst_select         = 0;                            \
} while (0)


#define _ARAD_EGR_PRGE_MGMT_LOG(_unit, _text) \
                LOG_VERBOSE(BSL_LS_SOC_EGRESS, (BSL_META_U((_unit),"        ++ %d: PRGE MGMT - " _text),(_unit)))

#define _ARAD_EGR_PRGE_MGMT_LOG_PARAM(_unit, _text, _param) \
                LOG_VERBOSE(BSL_LS_SOC_EGRESS, (BSL_META_U((_unit),"        ++ %d: PRGE MGMT - " _text),(_unit),(_param)))

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct {
    uint32 prog_index;
    lfem_maps_shadow_t  *program_permutations; /* pointer to array of size ARAD_EGR_PRGE_MGMT_MAX_NOF_PERMUTATIONS_PER_PROGRAM*/
    uint8 valid_permutation_number; /* should be init to 0 */
} single_program_permutations_t;


typedef struct {

    ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION ce_ifc;

    uint8 is_nop;

    uint8 size_change;

    /* Dependencies */
    uint8  reg_dep_instr[8]; /* This instr. must come 2+ entries after these instructions */
    int    last_reg_dep;
    int    reg_immediate_dep; /* This instr. must come at most 1 entries after this instruction */
    int    size_dep;

    /* Anchors */
    int anchor_ce_instr_mem_bmp; /* For LFEM buffer management */
    int anchor_instr_entry_range_min;
    int anchor_instr_entry_range_max;

} _ARAD_EGR_PRGE_MGMT_CE_INSTRUCTION;


typedef struct {

    ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION alu_ifc;

    uint8  is_nop;

    uint32 dep_flags;
    uint32 cond_dep_flags;

    /* Dependencies */
    uint8  reg_dep_instr[8]; /* This instr. must come 2+ entries after these instructions */
    int    last_reg_dep;
    int    reg_immediate_dep; /* This instr. must come at most 1 entries after this instruction */
    int    lfem_dep;

    /* Anchors */
    int anchor_instr_entry_range_min;
    int anchor_instr_entry_range_max;

} _ARAD_EGR_PRGE_MGMT_ALU_INSTRUCTION;

/* Instruction - for the program graph array.*/
typedef struct {

    int selected_entry;
    char *doc;

    _ARAD_EGR_PRGE_MGMT_CE_INSTRUCTION ce;
    _ARAD_EGR_PRGE_MGMT_ALU_INSTRUCTION alu;

} _ARAD_EGR_PRGE_MGMT_INSTRUCTION;


/* Management types */
typedef enum {

    ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED = 0,  /* Program is not in the management system */
    ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_LOAD_ONLY,  /* Strict or not for manipulation program - load only */
    ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_MANAGED,  /* Program should be managed and loaded */

    /* Number of management types  - maust be last */
    ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOF
} ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE;

/* Complete program */
typedef struct {

    /* Is the program managed */
    ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE management_type;

    /* Next instruction to insert / #instructions in the program */
    uint32 instructions_nof;

    /* Points in the program where jumps are called */
    ARAD_EGR_PROG_EDITOR_JUMP_POINT jump_points[ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF];
    uint8 nof_jump_points;

    /* dependency graph of instructions */
    _ARAD_EGR_PRGE_MGMT_INSTRUCTION graph[ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS];

} _ARAD_EGR_PRGE_MGMT_PROGRAM;


/* Branch */
typedef struct {

    /* Is the program managed */
    ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE management_type;

    /* Next instruction to insert / #instructions in the branch */
    uint32 instructions_nof;

    /* This branch ends in a jump to another branch/back to the program */
    uint8 jump_out;

    /* which jump-point IDs this branch considers for instruction placement */
    uint8 branch_uses[ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF];

    /* Points in the branch where jumps are called */
    ARAD_EGR_PROG_EDITOR_JUMP_POINT jump_points[ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF];
    uint8 nof_jump_points;

    /* dependency graph of instructions */
    _ARAD_EGR_PRGE_MGMT_INSTRUCTION graph[ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS];

} _ARAD_EGR_PRGE_MGMT_BRANCH;


/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* Program in loading */
STATIC ARAD_EGR_PROG_EDITOR_PROGRAMS current_program[SOC_MAX_NUM_DEVICES];

/* All programs data*/
STATIC _ARAD_EGR_PRGE_MGMT_PROGRAM mgmt_programs[SOC_MAX_NUM_DEVICES][ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS];

/*
 * Branches data
 */

/* Branch in loading */
STATIC ARAD_EGR_PROG_EDITOR_BRANCH current_branch[SOC_MAX_NUM_DEVICES];

/* Branch usage - Every program that uses a branch, declares it. When loading a branch the start instruction is determined
   according to the programs that use it */
STATIC int mgmt_branch_usage[SOC_MAX_NUM_DEVICES][ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF][2];

/* All programs data*/
STATIC _ARAD_EGR_PRGE_MGMT_BRANCH mgmt_branches[SOC_MAX_NUM_DEVICES][ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF];


/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* Init CE Instruction struct */
void _ARAD_EGR_PRGE_MGMT_CE_INSTRUCTION_init(_ARAD_EGR_PRGE_MGMT_CE_INSTRUCTION * p_ce_instr) {
    p_ce_instr->last_reg_dep                    = -1;
    p_ce_instr->reg_immediate_dep               = -1;
    p_ce_instr->size_dep                        = -1;
    p_ce_instr->anchor_instr_entry_range_min    = -1;
    p_ce_instr->anchor_instr_entry_range_max    = -1;
    p_ce_instr->anchor_ce_instr_mem_bmp         = 0;
}

/* Init ALU Instruction struct */
void _ARAD_EGR_PRGE_MGMT_ALU_INSTRUCTION_init(_ARAD_EGR_PRGE_MGMT_ALU_INSTRUCTION * p_alu_instr) {
    p_alu_instr->last_reg_dep                   = -1;
    p_alu_instr->reg_immediate_dep              = -1;
    p_alu_instr->lfem_dep                       = -1;
    p_alu_instr->anchor_instr_entry_range_min   = -1;
    p_alu_instr->anchor_instr_entry_range_max   = -1;
}

/* Init Instruction struct */
void _ARAD_EGR_PRGE_MGMT_INSTRUCTION_clear(_ARAD_EGR_PRGE_MGMT_INSTRUCTION* p_instr) {
    sal_memset(p_instr, 0, sizeof(_ARAD_EGR_PRGE_MGMT_INSTRUCTION));
    p_instr->selected_entry                 = -1;
    _ARAD_EGR_PRGE_MGMT_CE_INSTRUCTION_init(&(p_instr->ce));
    _ARAD_EGR_PRGE_MGMT_ALU_INSTRUCTION_init(&(p_instr->alu));
}

/* Init program struct */
void _ARAD_EGR_PRGE_MGMT_PROGRAM_init(_ARAD_EGR_PRGE_MGMT_PROGRAM *p_program) {
    uint32 j;

    p_program->management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED;

    p_program->instructions_nof = 0;
    p_program->nof_jump_points = 0;

    for (j = 0; j < ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS; j++) {
        _ARAD_EGR_PRGE_MGMT_INSTRUCTION_clear(&(p_program->graph[j]));
    }
}

/* Init branch structure */
void _ARAD_EGR_PRGE_MGMT_BRANCH_init(_ARAD_EGR_PRGE_MGMT_BRANCH *p_branch) {
    uint32 j;

    p_branch->management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED;

    p_branch->instructions_nof = 0;
    p_branch->jump_out = 0;
    p_branch->nof_jump_points = 0;

    for (j = 0; j < ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF; j++) {
        p_branch->branch_uses[j] = 0;
    }

    for (j = 0; j < ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS; j++) {
        _ARAD_EGR_PRGE_MGMT_INSTRUCTION_clear(&(p_branch->graph[j]));
    }
}


/* Initiate the management system data.*/
uint32
  arad_egr_prge_mgmt_init(
     int     unit
  ){

    int i;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    current_program[unit] = ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS;
    current_branch[unit] = ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF;

    for (i = 0; i < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; i++) {
        _ARAD_EGR_PRGE_MGMT_PROGRAM_init(&(mgmt_programs[unit][i]));
    }

    for (i = 0; i < ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF; i++) {
        _ARAD_EGR_PRGE_MGMT_BRANCH_init(&(mgmt_branches[unit][i]));
    }

    for (i = 0; i < ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF; i++) {
        /* Invalid values that will be ovrwritten the first time the jump-point is used */
        mgmt_branch_usage[unit][i][0] = ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS;
        mgmt_branch_usage[unit][i][1] = -1;
    }

    arad_egr_prge_interface_field_available_by_device_init(unit);

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_init()",0,0);
}

/* Moves the program adding "State machine" to instructions entering state.
 * This function should be called before starting to add a managed program's instructions.
 */
uint32
  arad_egr_prge_mgmt_build_program_start(
     int     unit,
     ARAD_EGR_PROG_EDITOR_PROGRAMS   program
  ){

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (current_program[unit] != ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) {
        /* Previous program didn't end */
        char *next_name, *cur_name;
        arad_egr_prog_editor_prog_name_get_by_id(current_program[unit], &cur_name);
        arad_egr_prog_editor_prog_name_get_by_id(program, &next_name);

        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - Can't start %s, Previus program(%s) hasn't ended yet"),
                                next_name, cur_name));
    }

    if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(unit, program)) {
        /* Program already loaded - Can't reload */
        char *next_name;
        arad_egr_prog_editor_prog_name_get_by_id(program, &next_name);

        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - Program %s already loaded"),
                                next_name));
    }

    /* Set the state of the program insertion state machine to inserting this program */
    current_program[unit] = program;
    mgmt_programs[unit][program].instructions_nof = 0;
    mgmt_programs[unit][program].management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_build_program_start()",0,0);
}

/* Returns a dependency bitmap for register dependency of a CE instruction */
uint32
  _arad_egr_prge_mgmt_ce_reg_dependency_get(
     int     unit,
     ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION    *ce_interface_instr,
     uint32  *dep_res
  ){
    uint32 result = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (ce_interface_instr->offset_src < ARAD_EGR_PROG_EDITOR_OFFSET_SRC_CONST_0) {
        /* offset src is from a reg */
        result |= _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((ce_interface_instr->offset_src - ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_0));
    }

    if (ce_interface_instr->size_src < ARAD_EGR_PROG_EDITOR_SIZE_SRC_CONST_0) {
        /* size src is a reg */
        result |= _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((ce_interface_instr->size_src - ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_0));
    }

    

    *dep_res = result;

    SOC_SAND_EXIT_NO_ERROR;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_ce_instr_add()",0,0);
}


/* Returns a dependency bitmap for register dependency of an ALU instruction */
uint32
  _arad_egr_prge_mgmt_alu_reg_dependency_get(
     int     unit,
     ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION    *alu_interface_instr,
     uint32  *dep_res
  ){
    uint32 result = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (alu_interface_instr->op1 <= ARAD_PP_PRGE_ENG_OP_REG_RSZ) {
        /* Reg src is from a reg */
        result |= _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((alu_interface_instr->op1 - ARAD_PP_PRGE_ENG_OP_REG_R0));
    }

    if (alu_interface_instr->op2 <= ARAD_PP_PRGE_ENG_OP_REG_RSZ) {
        /* Reg src is from a reg */
        result |= _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((alu_interface_instr->op2 - ARAD_PP_PRGE_ENG_OP_REG_R0));
    }

    if (alu_interface_instr->op3 <= ARAD_PP_PRGE_ENG_OP_REG_RSZ) {
        /* Reg src is from a reg */
        result |= _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((alu_interface_instr->op3 - ARAD_PP_PRGE_ENG_OP_REG_R0));
    }

    *dep_res = result;

    SOC_SAND_EXIT_NO_ERROR;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_ce_instr_add()",0,0);
}

/* Add a CE instruction to the current program */
uint32
  _arad_egr_prge_mgmt_prog_ce_instr_add(
     int     unit,
     ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION    *ce_interface_instr
  ){

    uint32 res;
    ARAD_EGR_PROG_EDITOR_PROGRAMS prog = current_program[unit];
    uint32 next_instr = mgmt_programs[unit][prog].instructions_nof;

    uint32 reg_dep_vector;

    int i;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (_ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTR_IS_NOP(ce_interface_instr)) {
        /* Add CE NOP instruction */
        mgmt_programs[unit][prog].graph[next_instr].ce.is_nop = 1;
        SOC_SAND_EXIT_NO_ERROR;
    }

    /* Copy the interface instruction as is */
    sal_memcpy(&(mgmt_programs[unit][prog].graph[next_instr].ce.ce_ifc), ce_interface_instr,
               sizeof(ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION));

    if (next_instr > 0) { /* previous instructions exist. find dependencies */
        if ((ce_interface_instr->size_src != ARAD_EGR_PROG_EDITOR_SIZE_SRC_CONST_0)
            || (ce_interface_instr->size_base)) {
            /* Search size dependency */
            for (i = next_instr - 1; i >= 0; i--) {
                if (mgmt_programs[unit][prog].graph[i].ce.size_change) {
                    mgmt_programs[unit][prog].graph[next_instr].ce.size_dep = i;
                    break;
                }
            }
        }

        res = _arad_egr_prge_mgmt_ce_reg_dependency_get(unit, ce_interface_instr, &reg_dep_vector);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

        if (reg_dep_vector & mgmt_programs[unit][prog].graph[next_instr - 1].alu.dep_flags) {
            /* Previous instr. write to a register which value is used in this instr. */
            mgmt_programs[unit][prog].graph[next_instr].ce.reg_immediate_dep = next_instr - 1;
        }

        /* Search reg dependency (if neccessary) */
        for (i = next_instr - 2; (i >= 0) && reg_dep_vector; i--) {
            if (reg_dep_vector & mgmt_programs[unit][prog].graph[i].alu.dep_flags) {
                /* Instr. i writes to a register this instr. uses unconditionally */
                mgmt_programs[unit][prog].graph[next_instr].ce
                    .reg_dep_instr[ ++mgmt_programs[unit][prog].graph[next_instr].ce.last_reg_dep ] = i;
                reg_dep_vector &= ~(mgmt_programs[unit][prog].graph[i].alu.dep_flags);
            }
            else if (reg_dep_vector & mgmt_programs[unit][prog].graph[i].alu.cond_dep_flags) {
                /* Instr. i writes to a register this instr. uses under a condition */
                mgmt_programs[unit][prog].graph[next_instr].ce
                    .reg_dep_instr[ ++mgmt_programs[unit][prog].graph[next_instr].ce.last_reg_dep ] = i;
            }
        }

        


    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_ce_instr_add()",0,0);
}

/* Add a ALU istruction to the current program */
uint32
  _arad_egr_prge_mgmt_prog_alu_instr_add(
     int     unit,
     ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION    *alu_interface_instr
  ){

    uint32 res;
    ARAD_EGR_PROG_EDITOR_PROGRAMS prog = current_program[unit];
    uint32 next_instr = mgmt_programs[unit][prog].instructions_nof;

    uint32 reg_dep_vector;

    int i;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (_ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTR_IS_NOP(alu_interface_instr)) {
        /* Add ALU NOP instruction */
        mgmt_programs[unit][prog].graph[next_instr].alu.is_nop = 1;
        SOC_SAND_EXIT_NO_ERROR;
    }

    /* Copy the interface instruction as is */
    sal_memcpy(&(mgmt_programs[unit][prog].graph[next_instr].alu.alu_ifc), alu_interface_instr,
               sizeof(ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION));

    /* LFEM dependency */
    if ((alu_interface_instr->op1 == ARAD_PP_PRGE_ENG_OP_FEM)
        || (alu_interface_instr->op2 == ARAD_PP_PRGE_ENG_OP_FEM)) {
        if (next_instr < 2) {
            char *prog_name;
            arad_egr_prog_editor_prog_name_get_by_id(prog, &prog_name);
            SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - LFEM as ALU input in instruction %d (Program: %s)"),
                                    next_instr , prog_name));
        }
        mgmt_programs[unit][prog].graph[next_instr].alu.lfem_dep = next_instr-2;
    }

    if (next_instr > 0) { /* previous instructions exist. find dependencies */

        res = _arad_egr_prge_mgmt_alu_reg_dependency_get(unit, alu_interface_instr, &reg_dep_vector);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

        if (reg_dep_vector & mgmt_programs[unit][prog].graph[next_instr - 1].alu.dep_flags) {
            /* Previous instr. write to a register which value is used in this instr. */
            mgmt_programs[unit][prog].graph[next_instr].alu.reg_immediate_dep = next_instr - 1;
        }

        /* Search reg dependency (if neccessary) */
        for (i = next_instr - 2; (i >= 0) && reg_dep_vector; i--) {
            if (reg_dep_vector & mgmt_programs[unit][prog].graph[i].alu.dep_flags) {
                /* Instr. i writes to a register this instr. uses unconditionally */
                mgmt_programs[unit][prog].graph[next_instr]
                    .alu.reg_dep_instr[ ++mgmt_programs[unit][prog].graph[next_instr].alu.last_reg_dep ] = i;
                reg_dep_vector &= ~(mgmt_programs[unit][prog].graph[i].alu.dep_flags);
            }
            else if (reg_dep_vector & mgmt_programs[unit][prog].graph[i].alu.cond_dep_flags) {
                /* Instr. i writes to a register this instr. uses under a condition */
                mgmt_programs[unit][prog].graph[next_instr]
                    .alu.reg_dep_instr[ ++mgmt_programs[unit][prog].graph[next_instr].alu.last_reg_dep ] = i;
            }
        }
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_ce_instr_add()",0,0);
}

/* Sets the bitmap of changes this instruction does that other instructions may depand on */
uint32
  _arad_egr_prge_mgmt_dep_flags_set(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION    *ifc_instr
  ){

    const ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION    *ce_interface_instr = &(ifc_instr->ce_interface_instruction);
    const ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION   *alu_interface_instr = &(ifc_instr->alu_interface_instruction);

    ARAD_EGR_PROG_EDITOR_PROGRAMS prog = current_program[unit];
    uint32 next_instr = mgmt_programs[unit][prog].instructions_nof;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if ((!_ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTR_IS_NOP(alu_interface_instr))
        && (alu_interface_instr->dst_select <= ARAD_PP_PRGE_ENG_OP_REG_RSZ)) {
        /* Dst is a reg */
        if (alu_interface_instr->cmp_action == ARAD_EGR_PROG_EDITOR_ALU_CMP_ALL) {
            mgmt_programs[unit][prog].graph[next_instr].alu.dep_flags |=
                _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((alu_interface_instr->dst_select - ARAD_PP_PRGE_ENG_OP_REG_R0));
        }
        else if (alu_interface_instr->cmp_action != ARAD_EGR_PROG_EDITOR_ALU_CMP_NONE) {
            mgmt_programs[unit][prog].graph[next_instr].alu.cond_dep_flags
                |= _ARAD_EGR_PRGE_MGMT_DEP_FLAG_FROM_OFFSET((alu_interface_instr->dst_select - ARAD_PP_PRGE_ENG_OP_REG_R0));
        }
    }

    if ((!_ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTR_IS_NOP(ce_interface_instr))
        && ((ce_interface_instr->size_src != ARAD_EGR_PROG_EDITOR_SIZE_SRC_CONST_0)
            || (ce_interface_instr->size_base != 0))){
        mgmt_programs[unit][prog].graph[next_instr].ce.size_change = 1;
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_dep_flags_set()",0,0);
}

/* Adds an instruction to the current program */
uint32
  _arad_egr_prge_mgmt_prog_instr_add(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION    *ifc_instr
  ){
    uint32 res;

    ARAD_EGR_PROG_EDITOR_PROGRAMS prog = current_program[unit];
    uint32 next_instr = mgmt_programs[unit][prog].instructions_nof;

    ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION    *ce_interface_instr = &(ifc_instr->ce_interface_instruction);
    ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION   *alu_interface_instr = &(ifc_instr->alu_interface_instruction);

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Validity checks */
    if (prog == ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - Can't add instruction - no program is started")));
    }
    if (mgmt_programs[unit][current_program[unit]].instructions_nof >= ARAD_PP_EG_PROG_NOF_INSTR(unit)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Can't add instruction - too many instructions in the program")));
    }

    /* Add the 2 parts of the instruction */
    res = _arad_egr_prge_mgmt_prog_ce_instr_add(unit, ce_interface_instr);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
    res = _arad_egr_prge_mgmt_prog_alu_instr_add(unit, alu_interface_instr);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

    /* Add the documentation of the instruction */
    mgmt_programs[unit][prog].graph[next_instr].doc = ifc_instr->doc_str;

    /* Set the dependency for the following instructions */
    res = _arad_egr_prge_mgmt_dep_flags_set(unit, ifc_instr);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

    /* Set initial entry selection */
    mgmt_programs[unit][prog].graph[next_instr].selected_entry = mgmt_programs[unit][current_program[unit]].instructions_nof++;

    /* If the program has ARAD_PP_EG_PROG_NOF_INSTR(unit) instructions, it cannot be manipulated (yet)
       So there's no use trying. Mark as load-only to save time */
    if (mgmt_programs[unit][current_program[unit]].instructions_nof == ARAD_PP_EG_PROG_NOF_INSTR(unit)) {
        mgmt_programs[unit][current_program[unit]].management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_LOAD_ONLY;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_prog_instr_add()",0,0);
}

/* Adds an instruction to the current branch */
uint32
  _arad_egr_prge_mgmt_branch_instr_add(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION    *ifc_instr
  ){

    ARAD_EGR_PROG_EDITOR_BRANCH branch = current_branch[unit];
    uint32 next_instr = mgmt_branches[unit][branch].instructions_nof;

    ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION    *ce_interface_instr = &(ifc_instr->ce_interface_instruction);
    ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION   *alu_interface_instr = &(ifc_instr->alu_interface_instruction);

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Validity checks */
    if (mgmt_branches[unit][branch].instructions_nof >= (ARAD_PP_EG_PROG_NOF_INSTR(unit)-4)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Can't add instruction - too many instructions in the branch")));
    }

    /* Add the 2 parts of the instruction */
    
    /* CE */
    if (_ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTR_IS_NOP(ce_interface_instr)) {
        /* Add CE NOP instruction */
        mgmt_branches[unit][branch].graph[next_instr].ce.is_nop = 1;
    }
    else {
        /* Copy the interface instruction as is */
        sal_memcpy(&(mgmt_branches[unit][branch].graph[next_instr].ce.ce_ifc), ce_interface_instr,
                   sizeof(ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION));
    }
    /* ALU */
    if (_ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTR_IS_NOP(alu_interface_instr)) {
        /* Add ALU NOP instruction */
        mgmt_branches[unit][branch].graph[next_instr].alu.is_nop = 1;
    }
    else {
        /* Copy the interface instruction as is */
        sal_memcpy(&(mgmt_branches[unit][branch].graph[next_instr].alu.alu_ifc), alu_interface_instr,
                   sizeof(ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION));
    }

    /* Add the documentation of the instruction */
    mgmt_branches[unit][branch].graph[next_instr].doc = ifc_instr->doc_str;


    /* Set initial entry selection */
    mgmt_branches[unit][branch].graph[next_instr].selected_entry = mgmt_branches[unit][branch].instructions_nof++;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_branch_instr_add()",0,0);
}

/* Adds an instruction to the current program or branch */
uint32
  arad_egr_prge_mgmt_instr_add(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION    *ifc_instr
  ){

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (current_program[unit] < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) {
        SOC_SAND_CHECK_FUNC_RESULT(_arad_egr_prge_mgmt_prog_instr_add(unit,ifc_instr), 370, exit);
    }
    else if (current_branch[unit] < ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF) {
        SOC_SAND_CHECK_FUNC_RESULT(_arad_egr_prge_mgmt_branch_instr_add(unit,ifc_instr), 370, exit);
    }
    else {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Not in program or branch. can't add instruction.")));
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_instr_add()",0,0);
}


/* 
 * Moves the program insertion "State machine" back to the initial state.
 * This function should be called after the insertion of the last meaningful (not NOP)
 * instruction of the current program.
 */
uint32
  arad_egr_prge_mgmt_build_program_end(
     int     unit
  ){

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Validity check */
    if (current_program[unit] == ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - Can't end a program - no program is started")));
    }

    /* Set state */
    if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_OLD_INTERFACE(unit, current_program[unit])) {
        /* If this program wasn't previously marked as load only or other type - set managed */
        mgmt_programs[unit][current_program[unit]].management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_MANAGED;
    }
    current_program[unit] = ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_build_program_end()",0,0);
}

/* Signals that the current program branches (has jump instruction */
uint32
  arad_egr_prge_mgmt_program_jump_point_add(
     int     unit,
     ARAD_EGR_PROG_EDITOR_JUMP_POINT jump_point
  ){
    int
        current_instr;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Validity check */
    if ((current_program[unit] == ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS)
        && (current_branch[unit] == ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - Not in a program - can't add jump point")));
    }
    if (jump_point >= ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internal error - Undefined jump point ID")));
    }

    /* Set state */
    if (current_branch[unit] == ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF) {
        /* Jump from a program */
        _ARAD_EGR_PRGE_MGMT_PROGRAM *program = &mgmt_programs[unit][current_program[unit]];

        current_instr = program->instructions_nof;

        if (program->nof_jump_points < ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF) {
            /* Add the jump point to the program */
            program->jump_points[program->nof_jump_points++] = jump_point;
        }
        else {
            SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Too many jumps from a single program")));
        }

        /* Mark current program to "Load-Only" */ 
        program->management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_LOAD_ONLY;
    }
    else {
        /* Jump from a branch */
        _ARAD_EGR_PRGE_MGMT_BRANCH *branch = &mgmt_branches[unit][current_branch[unit]];

        current_instr = branch->instructions_nof;

        if (branch->nof_jump_points < ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF) {
            /* Add the jump point to the program */
            branch->jump_points[branch->nof_jump_points] = jump_point;
        }
        else {
            SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Too many jumps from a single branch")));
        }
    }

    if (current_instr < mgmt_branch_usage[unit][jump_point][0]) {
        mgmt_branch_usage[unit][jump_point][0] = current_instr;
    }
    if (current_instr > mgmt_branch_usage[unit][jump_point][1]) {
        mgmt_branch_usage[unit][jump_point][1] = current_instr;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_program_branch_usage_add()",jump_point,0);
}

/* Sets the current program to a load-only state.
   This is automatically done for programs with jumps or data-memory usage.
   This can be added to other programs manually if necessary or for debugging */
uint32
  arad_egr_prge_mgmt_current_program_load_only_set(
     int     unit
  ){
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (current_program[unit] < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) {
        /* In a program */
        mgmt_programs[unit][current_program[unit]].management_type = ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_LOAD_ONLY;
    }
    else if (current_branch[unit] < ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF){
        /* In a branch */
        
        SOC_SAND_EXIT_NO_ERROR;
    }
    else {
        /* Neither in a program nor a branch */
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Error - not in a branch or a program")));
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_current_program_load_only_set()",0,0);
}

/* Moves the program adding "State machine" to instructions entering state - for a branch.
 * This function should be called before starting to add a branch's instructions.
 */
uint32
  arad_egr_prge_mgmt_build_branch_start(
     int     unit,
     ARAD_EGR_PROG_EDITOR_BRANCH    branch
  ){

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (current_branch[unit] != ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Branch build error - branch already started")));
    }
    if (mgmt_branches[unit][branch].management_type != ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Branch build error - branch already loaded")));
    }

    /* Set state */
    current_branch[unit] = branch;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_build_branch_start()",branch,0);
}

uint32
  arad_egr_prge_mgmt_branch_usage_add(
     int     unit,
     ARAD_EGR_PROG_EDITOR_JUMP_POINT    jump_point /* jump pts that may reach the current branch */
  ){
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (current_branch[unit] == ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Branch error - No branch started to add usage to")));
    }
    if (jump_point >= ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Branch error - Invalid usage code")));
    }

    /* Set state */
    mgmt_branches[unit][current_branch[unit]].branch_uses[jump_point] = 1;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_branch_usage_add()",jump_point,0);
}

/*
 * Moves the program insertion "State machine" back to the initial state.
 * This function should be called after the insertion of the last meaningful (not NOP)
 * instruction of the current branch.
 */
uint32
  arad_egr_prge_mgmt_build_branch_end(
     int     unit
  ){
    uint32 i;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (current_branch[unit] == ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Branch error - No branch started")));
    }

    for (i = 0; i < ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF; i++) {
        /* Make sure the branch has a jump point leading to it */
        if (mgmt_branches[unit][current_branch[unit]].branch_uses[i]) {
            /* Jump point found - Set state */
            mgmt_branches[unit][current_branch[unit]].management_type =  ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_LOAD_ONLY; 
            current_branch[unit] = ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF;
            SOC_SAND_EXIT_NO_ERROR;
        }
    }

    /* Should not reach here. Reaching here means that a call to arad_egr_prge_mgmt_branch_usage_add is missing */
    SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Branch error - No jump point defined for the branch. Branch cannot be loaded.")));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_build_branch_end()",0,0);
}

/* Init program data */
uint32
  _arad_egr_prge_mgmt_program_init(
     int     unit,
     ARAD_EGR_PROG_EDITOR_PROGRAMS   program
  ){
    int instr;
    uint32 instr_nof = mgmt_programs[unit][program].instructions_nof;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    for (instr = 0; instr < instr_nof; instr++) {
        /* Set no possible memory tables to put this instruction in */
        mgmt_programs[unit][program].graph[instr].ce.anchor_ce_instr_mem_bmp = 0;
        /* Set initial default entry selection */
        mgmt_programs[unit][program].graph[instr].selected_entry = instr;
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_program_init()",0,0);
}

/* Find the buffer(s) containing a specific LFEM in a shadow object */
uint32
  _arad_egr_prge_mgmt_lfem_buffer_find(
     int     unit,
     ARAD_EGR_PROG_EDITOR_LFEM   lfem,
     lfem_maps_shadow_t          lfem_shadow,
     uint32                     *buffers,
     uint32                     *num_res
  ) {
    uint32 i,j;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(lfem_shadow);
    SOC_SAND_CHECK_NULL_INPUT(buffers);
    SOC_SAND_CHECK_NULL_INPUT(num_res);

    if (lfem >= ARAD_EGR_PROG_EDITOR_LFEM_NOF_LFEMS) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("Internal error - Invalid LFEM")));
    }

    *num_res = 0;

    for (i = 0; i < ARAD_PP_EG_PROG_NOF_LFEM_TABLES(unit); i++) {
        for (j = 0; j < ARAD_PP_EG_PROG_NOF_FEM(unit); j++) {
            if (lfem == lfem_shadow[i][j]) {
                buffers[*num_res] = i;
                (*num_res)++;
                break;
            }
        }
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_lfem_buffer_find()",0,0);
}

/* Add possible mem table to put an instruction in */
uint32
  _arad_egr_prge_mgmt_lfem_option_add(
    int     unit,
    ARAD_EGR_PROG_EDITOR_PROGRAMS   program,
    uint32  instruction,
    uint32  mem_lsb
  ){

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    mgmt_programs[unit][program].graph[instruction].ce.anchor_ce_instr_mem_bmp |= (1<<mem_lsb);

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_prge_mgmt_lfem_option_add()",0,0);
}

/* Updates the memory tables available for an instruction according to the lfem shadow object */
uint32
  _arad_egr_prge_mgmt_program_anchors_update(
    int     unit,
    ARAD_EGR_PROG_EDITOR_PROGRAMS   program,
    lfem_maps_shadow_t          lfem_shadow
  ){
    uint32 res;
    int instr;

    uint32 buffers[ARAD_EGR_PRGE_MAX_NOF_LFEM_TABLES];
    uint32 num_res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    for (instr = 0; instr < mgmt_programs[unit][program].instructions_nof; instr++) {
        if (_ARAD_EGR_PRGE_MGMT_INSTR_USE_LFEM(unit, program, instr)) {
            /* Instruction uses an LFEM. Find to which buffers is it loaded */
            res = _arad_egr_prge_mgmt_lfem_buffer_find(unit, mgmt_programs[unit][program].graph[instr].ce.ce_ifc.lfem,
                                                       lfem_shadow, buffers, &num_res);
            SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

            if (num_res == 0) { /* LFEM wasn't found */
                SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Required LFEM wasn't loaded")));
            }

            while (num_res) {
                num_res--;
                res = _arad_egr_prge_mgmt_lfem_option_add(unit, program, instr, buffers[num_res]);
                SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
            }
        }
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_program_anchors_update()",0,0);
}

/* Spread the program so that instructions fall on memory tables from which they can access the LFEMs they need */
uint32
  _arad_egr_prge_mgmt_program_entries_by_lfems_adjust(
    int     unit,
    ARAD_EGR_PROG_EDITOR_PROGRAMS   program,
    uint8                           error_on_fail
  ){
    int instr;
    int i, highest_to_move;
    int instr_added=0;

    uint32 instr_nof = mgmt_programs[unit][program].instructions_nof;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Try to satisfy lfem anchors */
    for (instr = 0; instr < instr_nof; instr++) {
        if (_ARAD_EGR_PRGE_MGMT_INSTR_USE_LFEM(unit, program, instr)) {

            highest_to_move = instr;

            if (!_ARAD_EGR_PRGE_MGMT_IS_INSTR_MEM_LSB_IN_BMP(unit, program, instr)) {
                /* Find first instruction that needs moving */
                while ((mgmt_programs[unit][program].graph[highest_to_move].alu.lfem_dep != -1)
                       || (mgmt_programs[unit][program].graph[highest_to_move+1].alu.lfem_dep != -1)
                       || (mgmt_programs[unit][program].graph[highest_to_move].ce.reg_immediate_dep != -1)
                       || (mgmt_programs[unit][program].graph[highest_to_move].alu.reg_immediate_dep != -1)) {
                    highest_to_move--;
                }

                while (!_ARAD_EGR_PRGE_MGMT_IS_INSTR_MEM_LSB_IN_BMP(unit, program, instr)) {
                    /* While anchor isn't satisfied */

                    if (instr_nof + instr_added < ARAD_PP_EG_PROG_NOF_INSTR(unit)) {

                        /* Move all needed instructions, one entry down */
                        for (i = highest_to_move; i < instr_nof; i++) {
                            mgmt_programs[unit][program].graph[i].selected_entry++;
                        }
                        instr_added++;
                    } else {
                        if (error_on_fail) {
                            SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Can't deploy with current lfem arrangement")));
                        }
                        else {
                            return SOC_SAND_ERR;
                        }
                    }
                }
            }

            if (highest_to_move < instr) {
                /* If an instruction higher than 'instr' was moved, re-check anchors starting from it */
                instr = highest_to_move-1;
            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_program_entries_by_anchors_adjust()", 0, 0);
}

/* Loads to HW an instruction where both the ALU and the CE parts are NOPs */
uint32
  _arad_egr_pge_mgmt_null_instruction_load(
     int     unit,
     int     mem,
     int     entry
  ){
    uint32 res;
    ARAD_PP_EPNI_PRGE_INSTRUCTION_TBL_DATA
        prge_instruction_tbl_data;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Init NOP command */
    _ARAD_EGR_PRGE_MGMT_CE_NOP_INSTR_LOAD(unit, prge_instruction_tbl_data);
    _ARAD_EGR_PRGE_INTERFACE_ALU_NOP_INSTR_LOAD(unit, prge_instruction_tbl_data);
    res = arad_pp_epni_prge_instruction_tbl_set_unsafe(unit, mem, entry, &prge_instruction_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_pge_mgmt_null_instruction_load()",mem,entry);
}

/* Loads an instruction to HW */
uint32
  _arad_egr_prge_mgmt_instruction_load(
     int     unit,
     ARAD_EGR_PROG_EDITOR_PROGRAMS       *program_list,
     uint8                               num_programs,
     _ARAD_EGR_PRGE_MGMT_INSTRUCTION     *instruction,
     int     mem,
     int     entry
  ){
    uint32 res;
    uint8 i;

    ARAD_PP_EPNI_PRGE_INSTRUCTION_TBL_DATA
        prge_instruction_tbl_data;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (num_programs == 0) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE internale error - Loaded instruction for no program")));
    }

    /* CE fields */
    if (!(instruction->ce.is_nop)) {
        prge_instruction_tbl_data.src_select  = instruction->ce.ce_ifc.src_select;
        prge_instruction_tbl_data.offset_base = instruction->ce.ce_ifc.offset_base;
        prge_instruction_tbl_data.offset_src  = instruction->ce.ce_ifc.offset_src;
        prge_instruction_tbl_data.size_src    = instruction->ce.ce_ifc.size_src;
        prge_instruction_tbl_data.size_base   = instruction->ce.ce_ifc.size_base;
        for (i = 0; i < num_programs; i++) {
            res = arad_egr_prog_editor_supply_lfem_idx(unit, instruction->ce.ce_ifc.lfem, mem, program_list[i],
                                                       &(prge_instruction_tbl_data.fem_select));
            SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
        }
    }
    else {
        _ARAD_EGR_PRGE_MGMT_CE_NOP_INSTR_LOAD(unit, prge_instruction_tbl_data);
    }

    /* ALU fields */
    if (!(instruction->alu.is_nop)) {
        prge_instruction_tbl_data.op_1_field_select     = instruction->alu.alu_ifc.op1;
        prge_instruction_tbl_data.op_2_field_select     = instruction->alu.alu_ifc.op2;
        prge_instruction_tbl_data.op_3_field_select     = instruction->alu.alu_ifc.op3;
        prge_instruction_tbl_data.alu_action            = instruction->alu.alu_ifc.alu_action;
        prge_instruction_tbl_data.cmp_action            = instruction->alu.alu_ifc.cmp_action;
        prge_instruction_tbl_data.dst_select            = instruction->alu.alu_ifc.dst_select;
        /* Handle jumps */
        if (instruction->alu.alu_ifc.dst_select != ARAD_PP_PRGE_ENG_OP_SIZE_ADDR) {
            /* Not a jump instruction */
            prge_instruction_tbl_data.op_value              = instruction->alu.alu_ifc.op_value;
        }
        else {
            /* Jump instruction - handle case of jump to known branch (const) or to address in register */
            if (instruction->alu.alu_ifc.op3 == ARAD_PP_PRGE_ENG_OP_VALUE) {
                /* Jump to known address - op_value contains branch entry */
                prge_instruction_tbl_data.op_value = ARAD_EGR_PROG_EDITOR_PTR_TO_BRANCH_ADDR((instruction->alu.alu_ifc.op_value), (entry & 0x1));
            }
            else {
                /* Jump address already in a register, op_value should adjust the address
                   according to even or odd jump-from instructions */
                prge_instruction_tbl_data.op_value = (entry & 0x1) ? 1 : 0;
            }
        }
    }
    else {
        _ARAD_EGR_PRGE_INTERFACE_ALU_NOP_INSTR_LOAD(unit, prge_instruction_tbl_data);
    }

    /* Load instruction */
    res = arad_pp_epni_prge_instruction_tbl_set_unsafe(unit, mem, entry, &prge_instruction_tbl_data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in _arad_egr_pge_mgmt_null_instruction_load()",mem,entry);
}

/* Load a program (after deploy) to the HW */
uint32
  _arad_egr_prge_mgmt_program_load(
    int     unit,
    ARAD_EGR_PROG_EDITOR_PROGRAMS   program
  ){
    uint32 res;
    int mem;
    int entry, entry_even, entry_odd;
    int next_instr_entry = 0;
    uint32 program_pointer;

    int instr;
    uint32 instr_nof = mgmt_programs[unit][program].instructions_nof;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    mem = EPNI_PRGE_INSTRUCTION_0m - 1;
    res = sw_state_access[unit].dpp.soc.arad.tm.egr_prog_editor.programs.program_pointer.get(unit, program, &program_pointer);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    entry_odd = _ARAD_EGR_PROG_EDITOR_PTR_TO_BRANCH_ENTRY_ODD(program_pointer);
    entry_even = _ARAD_EGR_PROG_EDITOR_PTR_TO_BRANCH_ENTRY_EVEN(program_pointer);
    entry = entry_odd;

    for (instr = 0; instr < instr_nof; instr++ ) {
        if (mgmt_programs[unit][program].graph[instr].ce.is_nop
            && mgmt_programs[unit][program].graph[instr].alu.is_nop) {
            /* Instruction is null. continue because null padding will be done anyway */
            continue;
        }
        /* Padd with nulls until the next instruction. */
        while (next_instr_entry < mgmt_programs[unit][program].graph[instr].selected_entry) {
            mem   = (entry == entry_odd) ? (mem + 1) : mem;
            entry = (entry == entry_even) ? entry_odd : entry_even;
            res = _arad_egr_pge_mgmt_null_instruction_load(unit, mem, entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
            next_instr_entry++;
        }
        /* Load the next instruction */
        mem   = (entry == entry_odd) ? (mem + 1) : mem;
        entry = (entry == entry_even) ? entry_odd : entry_even;
        res = _arad_egr_prge_mgmt_instruction_load(unit, &program, 1 /*num_programs*/,
                                                   &mgmt_programs[unit][program].graph[instr], mem, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
        next_instr_entry++;
    }

    mem   = (entry == entry_odd) ? (mem + 1) : mem;
    entry = (entry == entry_even) ? entry_odd : entry_even;
    SOC_SAND_IF_ERR_EXIT( arad_egr_prog_editor_cat_nops(unit, mem, entry, entry_even, entry_odd) );

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_program_load()",0,0);
}

/* Perform deploy algorithm to the program according to the lfem shadow object */
uint32
  arad_egr_prge_mgmt_program_deploy(
    int     unit,
    ARAD_EGR_PROG_EDITOR_PROGRAMS   program,
    lfem_maps_shadow_t          lfem_shadow,
    uint8 do_load
  ){
    uint32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = _arad_egr_prge_mgmt_program_init(unit, program);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

    /* Update possible mem tables for each instruction */
    res = _arad_egr_prge_mgmt_program_anchors_update(unit, program, lfem_shadow);
    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

    /* Spread the program according to what's available */
    res = _arad_egr_prge_mgmt_program_entries_by_lfems_adjust(unit, program, do_load);
    if (do_load) {
        /* Actual HW writing. should not fail */
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

        res = _arad_egr_prge_mgmt_program_load(unit, program);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
    }
    else {
        /* Part of the back tracking process, might fail but shouldn't print error message */
        return res;
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_program_deploy()",0,0);
}

/* List the needed LFEMs in a program */
uint32
  arad_egr_prge_mgmt_pre_processing_program_scan(
    int     unit,
    ARAD_EGR_PROG_EDITOR_PROGRAMS   program,
    ARAD_EGR_PROG_EDITOR_LFEM       lfem_buffer[],
    uint8                           *num_lfems
  ){

    uint32 instr,i;
    ARAD_EGR_PROG_EDITOR_LFEM lfem;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(lfem_buffer);
    SOC_SAND_CHECK_NULL_INPUT(num_lfems);

    if (!_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(unit,program)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE management error - program isn't managed")));
    }

    *num_lfems = 0;

    for (instr = 0; instr < mgmt_programs[unit][program].instructions_nof; instr++ ) {
        if (_ARAD_EGR_PRGE_MGMT_INSTR_USE_LFEM(unit, program, instr)) {
            /* Instruction uses an LFEM. Check if it's its first occurance. */
            lfem = mgmt_programs[unit][program].graph[instr].ce.ce_ifc.lfem;
            for (i = 0; (i < *num_lfems) && (lfem_buffer[i] != lfem); i++)
                ;
            if (i == *num_lfems) { /* First occurance */
                if (*num_lfems >= ARAD_EGR_PRGE_MGMT_MAX_NOF_LFEMS_PER_PROGRAM) {
                    SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE management error - program has too many different lfems to manage")));
                }
                lfem_buffer[*num_lfems] = lfem;
                (*num_lfems)++;
            }
        }
    }

    SOC_SAND_EXIT_NO_ERROR;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_pre_processing_program_scan()",0,0);
}





/******************************************
 * Management
 ******************************************/


/* Sorting using Selection sort */
int
    arad_egr_prge_mgmt_sort_program_according_to_permutation_number(
       SOC_SAND_IN    int                             unit,
       single_program_permutations_t                    *all_program_permutations)
{
    uint8 program_ndx, program_ndx_2, program_ndx_min, permutation_num_min;
    uint8 prog_id;
    lfem_maps_shadow_t *permutations_temp;

    SOCDNX_INIT_FUNC_DEFS;

    for (program_ndx = 0; program_ndx < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; program_ndx++) {
        program_ndx_min = program_ndx;
        permutation_num_min = all_program_permutations[program_ndx].valid_permutation_number;

        /* Sort non-managed programs as well, they will beon the top */
        for (program_ndx_2 = program_ndx+1; program_ndx_2 < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; program_ndx_2++) {
            if (all_program_permutations[program_ndx_2].valid_permutation_number < permutation_num_min) {
                program_ndx_min = program_ndx_2;
                permutation_num_min = all_program_permutations[program_ndx_2].valid_permutation_number;
            }
        }

        if (program_ndx_min != program_ndx) {
            /* Replace current program with the minimal */
            permutations_temp = all_program_permutations[program_ndx_min].program_permutations;
            prog_id = all_program_permutations[program_ndx_min].prog_index;
            all_program_permutations[program_ndx_min].program_permutations = all_program_permutations[program_ndx].program_permutations;
            all_program_permutations[program_ndx_min].valid_permutation_number = all_program_permutations[program_ndx].valid_permutation_number;
            all_program_permutations[program_ndx_min].prog_index = all_program_permutations[program_ndx].prog_index;
            all_program_permutations[program_ndx].program_permutations = permutations_temp;
            all_program_permutations[program_ndx].valid_permutation_number = permutation_num_min;
            all_program_permutations[program_ndx].prog_index = prog_id; 
        }
    }

    SOC_EXIT;

exit:
    SOCDNX_FUNC_RETURN;
}

/* Init LFEM shadow structure. */
int
    arad_egr_prge_mgmt_lfem_shadow_init(
       SOC_SAND_IN    int                             unit,
       lfem_maps_shadow_t lfem_maps
    ) {

    uint8 lfem_ndx, lfem_table_ndx;

    SOCDNX_INIT_FUNC_DEFS;

    /* Using the macro SOC_DPP_DEFS_MAX may leads to such defects of same on both sides,
     * When there are 2 definitions of the same value
     */
    /* coverity[same_on_both_sides] */
    for (lfem_ndx = 0; lfem_ndx < ARAD_EGR_PRGE_MAX_NOF_LFEMS; lfem_ndx++) {
        for (lfem_table_ndx = 0; lfem_table_ndx < ARAD_EGR_PRGE_MAX_NOF_LFEM_TABLES; lfem_table_ndx++) {
            lfem_maps[lfem_table_ndx][lfem_ndx] = ARAD_EGR_PROG_EDITOR_LFEM_NOF_LFEMS;
        }
    }

    SOC_EXIT;

exit:
    SOCDNX_FUNC_RETURN;
}

/* Add all LFEMs from lfem_maps_src struct to lfem_maps_dst struct. */
int
    arad_egr_prge_mgmt_copy_shadow_internal(
       SOC_SAND_IN    int                             unit,
       lfem_maps_shadow_t lfem_maps_src,
       lfem_maps_shadow_t lfem_maps_dst,
       uint8                print_error)
{
    uint32 rv = SOC_SAND_OK;
    uint8 lfem_ndx_src, lfem_ndx_dst, lfem_table_ndx;

    SOCDNX_INIT_FUNC_DEFS;

    lfem_ndx_dst = 0;
    /* Using the macro SOC_DPP_DEFS_MAX may leads to such defects of same on both sides,
     * When there are 2 definitions of the same value
     */
    /* coverity[same_on_both_sides] */
    for (lfem_ndx_src = 0; lfem_ndx_src < ARAD_EGR_PRGE_MAX_NOF_LFEMS; lfem_ndx_src++) {
        for (lfem_table_ndx = 0; lfem_table_ndx < ARAD_EGR_PRGE_MAX_NOF_LFEM_TABLES; lfem_table_ndx++) {
            if (lfem_maps_src[lfem_table_ndx][lfem_ndx_src] != ARAD_EGR_PROG_EDITOR_LFEM_NOF_LFEMS) {
                /* LFEM lfem_maps_src[lfem_table_ndx][lfem_ndx_src] should be written to dst */
                /* check if it already exist there */
                lfem_ndx_dst = 0;
                while (lfem_ndx_dst < ARAD_PP_EG_PROG_NOF_FEM(unit)) {
                    if (lfem_maps_dst[lfem_table_ndx][lfem_ndx_dst] == ARAD_EGR_PROG_EDITOR_LFEM_NOF_LFEMS) {
                        /* end of allocated LFEMs in this array. Allocate one for this action */
                        lfem_maps_dst[lfem_table_ndx][lfem_ndx_dst] = lfem_maps_src[lfem_table_ndx][lfem_ndx_src];
                        break;
                    }
                    else if (lfem_maps_dst[lfem_table_ndx][lfem_ndx_dst] == lfem_maps_src[lfem_table_ndx][lfem_ndx_src]) {
                        /* lfem is already allocated. no need to allocate again. */
                        break;
                    }
                    lfem_ndx_dst++;
                }

                /* Not enough place in lfem_maps_dst for all the LFEMs */
                if (lfem_ndx_dst == ARAD_PP_EG_PROG_NOF_FEM(unit)) {
                    if (print_error) {
                        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Error - Not enough place in lfem_maps_dst for all the LFEMs")));
                    }
                    else {
                        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
                    }
                }
            }
        }
    }

    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

int
    arad_egr_prge_mgmt_copy_shadow(
       SOC_SAND_IN    int                             unit,
       lfem_maps_shadow_t lfem_maps_src,
       lfem_maps_shadow_t lfem_maps_dst)
{
    uint32 rv = SOC_SAND_OK;

    SOCDNX_INIT_FUNC_DEFS;

    rv = arad_egr_prge_mgmt_copy_shadow_internal(unit, lfem_maps_src, lfem_maps_dst, TRUE /* print_error */);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


int
    arad_egr_prge_mgmt_add_permutation_according_to_lfem_shadow(
       SOC_SAND_IN    int                             unit,
       lfem_maps_shadow_t lfem_maps_shadow,
       single_program_permutations_t *permutation_struct)
{
    uint32 rv = SOC_SAND_OK;

    SOCDNX_INIT_FUNC_DEFS;

    /* single permutation is a shadow */
    rv = arad_egr_prge_mgmt_copy_shadow(unit, lfem_maps_shadow, permutation_struct->program_permutations[permutation_struct->valid_permutation_number]);
    SOCDNX_IF_ERR_EXIT(rv);

    permutation_struct->valid_permutation_number++;

exit:
    SOCDNX_FUNC_RETURN;
}

int
    arad_egr_prge_mgmt_lfem_add_to_shadow(
       int                              unit,
       ARAD_EGR_PROG_EDITOR_LFEM        lfem,
       lfem_maps_shadow_t               lfem_shadow,
       uint8                            lfem_buffer_ndx
    ){

    uint32  i;

    SOCDNX_INIT_FUNC_DEFS;

    for (i = 0; i < ARAD_PP_EG_PROG_NOF_FEM(unit); i++) {
        if (lfem_shadow[lfem_buffer_ndx][i] == lfem) {
            return SOC_SAND_OK;
        }
        else if (lfem_shadow[lfem_buffer_ndx][i] == ARAD_EGR_PROG_EDITOR_LFEM_NOF_LFEMS) {
            lfem_shadow[lfem_buffer_ndx][i] = lfem;
            return SOC_SAND_OK;
        }
    }

    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Error - No room in LFEM buffer")));

    return SOC_SAND_ERR;

exit:
    SOCDNX_FUNC_RETURN;
}


int
    arad_egr_prge_mgmt_pre_processing_program_lfem_permutation_struct_build(int unit,
                                                                              int program_ndx,
                                                                              ARAD_EGR_PROG_EDITOR_LFEM lfems_usage_per_program[],
                                                                              uint8 num_lfems_usage_per_program,
                                                                              uint8 is_duplicate,
                                                                              single_program_permutations_t *permutation_struct)
{
    lfem_maps_shadow_t lfem_maps_shadow_dummy;
    uint8 one_permutation_succcess = 0;
    uint8 lfem_ndx, lfem_ndx_to_duplicate, lfem_table_ndx, lfem_id;
    uint16 permutation;
    uint8 nof_permutations;

    uint32 rv = SOC_SAND_OK;

    SOCDNX_INIT_FUNC_DEFS;

    nof_permutations = SOC_IS_JERICHO(unit) ? ((1 << num_lfems_usage_per_program*2) -1) /* 2 bit per LFEM - 0-3 table index */ :
        ((1 << num_lfems_usage_per_program) -1) /* each bit is 0/1 */;

    /* Loop on all possible LFEM permutations */
    
    for (permutation = 0; permutation <= nof_permutations; permutation++) {

        /* Init LFEM shadow structure */
        rv = arad_egr_prge_mgmt_lfem_shadow_init(unit, lfem_maps_shadow_dummy);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Loop on all possible LFEM permutations with one of the LFEMs duplicated in the 2 arrays */
        for (lfem_ndx_to_duplicate = 0; lfem_ndx_to_duplicate <= (is_duplicate?4:0)/*max nof LFEMs per program*/; lfem_ndx_to_duplicate++) {

            /* Set the current permutation in the structure as input to deploy function */
            for (lfem_ndx = 0; lfem_ndx < num_lfems_usage_per_program; lfem_ndx++)  {
                if (SOC_IS_JERICHO(unit)) {
                    lfem_table_ndx = (permutation >> (lfem_ndx*2)) & 3; /* take the relevant index from the permutation. value 0-3 is the index table */
                }
                else {
                    lfem_table_ndx = (permutation >> lfem_ndx) & 1; /* take the relevant index from the permutation. value 0 is odd, 1 is even */
                }
                lfem_id = lfems_usage_per_program[lfem_ndx];

                rv = arad_egr_prge_mgmt_lfem_add_to_shadow(unit, lfem_id, lfem_maps_shadow_dummy, lfem_table_ndx);
                SOCDNX_IF_ERR_EXIT(rv);
                if (is_duplicate && (lfem_ndx_to_duplicate==lfem_ndx)) {
                    if (SOC_IS_JERICHO(unit)) {
                        /*duplicate to the next LFEM array*/
                        lfem_table_ndx = (lfem_table_ndx==3) ? 0 : 1 + lfem_table_ndx;
                    }
                    else {
                        /*duplicate to the other LFEM array*/
                        lfem_table_ndx = 1 - lfem_table_ndx;

                    }
                    rv = arad_egr_prge_mgmt_lfem_add_to_shadow(unit, lfem_id, lfem_maps_shadow_dummy, lfem_table_ndx);
                    SOCDNX_IF_ERR_EXIT(rv); 
                }
            }

            /* Deploy test, result is success/failure */
            rv = arad_egr_prge_mgmt_program_deploy(unit, program_ndx, lfem_maps_shadow_dummy, FALSE/*do_load*/);

            if (rv == SOC_SAND_OK) {
                one_permutation_succcess = 1;
                /* mark permutation as valid in the permutations per program struct */
                rv = arad_egr_prge_mgmt_add_permutation_according_to_lfem_shadow(unit, lfem_maps_shadow_dummy, permutation_struct);
                SOCDNX_IF_ERR_EXIT(rv);

                if (permutation_struct->valid_permutation_number == ARAD_EGR_PRGE_MGMT_MAX_NOF_PERMUTATIONS_PER_PROGRAM) {
                    /* Here we assume that ARAD_EGR_PRGE_MGMT_MAX_NOF_PERMUTATIONS_PER_PROGRAM is enough permutations to enable mgmt */
                    return SOC_SAND_OK;
                }
            }
            /*else go to next permutation */
        }
    }

    if (one_permutation_succcess) {
        return SOC_SAND_OK;
    }
    else {
        return SOC_SAND_ERR;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
Stage 1: pre-processing
Get all possible LFEM allocations per program.
1.	Loop on all programs
    a.	Get list of all LFEMS used in current program
    b.	If program is marked for management (less than 24 instructions, no jumps etc.)
        i.	Loop on all possible LFEM permutations (this stage can be improved later by better pre-deploy algorithm)
            1.	Clear LFEM shadow structure and set the current permutation in the  structure as input to deploy function
            2.	Call deploy (with "test" flag to indicate no action in HW is required)
            3.	If deploy return success mark permutation as valid
        ii.	If all permutations fail (can happen only in case of one LFEM used more than once in the same program)
            1.	Loop on all possible LFEM permutations with one of the LFEMs duplicated in the 2 arrays
                a.	Clear LFEM shadow structure and set the current permutation in the  structure as input to deploy function
                b.	Call deploy (with "test" flag to indicate no action in HW is required)
                c.	If deploy return success mark permutation as valid
        iii.	If all permutations still fail
            1.	Return failure
    c.	Else
        i.	Mark existing permutation as valid
*/
int
  arad_egr_prge_mgmt_pre_processing(
     SOC_SAND_IN    int                             unit,
     single_program_permutations_t                    *all_program_permutations
  )
{
    /*single_program_permutations_t all_program_permutations[ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS];*/
    ARAD_EGR_PROG_EDITOR_LFEM lfems_usage_per_program[ARAD_EGR_PRGE_MGMT_MAX_NOF_LFEMS_PER_PROGRAM];
    uint8 program_ndx, num_lfems_usage_per_program;

    char* prog_name=NULL;

    uint32 rv = SOC_SAND_OK;

    SOCDNX_INIT_FUNC_DEFS;

    _ARAD_EGR_PRGE_MGMT_LOG(unit,"Pre processing - start\n");

    /* Keep shadow for next stage, use dummy shadow for further pre-processing */

    /* Loop on all programs */
    for (program_ndx = 0; program_ndx < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; program_ndx++) {
        /* If program is not marked for management (less than 24 instructions, no jumps etc.) */
        if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(unit, program_ndx)) { 
            _ARAD_EGR_PRGE_MGMT_LOG_PARAM(unit,"Pre processing - scanning program: %d\n", program_ndx);

            /* Get list of all LFEMS used in current program */
            rv = arad_egr_prge_mgmt_pre_processing_program_scan(unit, program_ndx, lfems_usage_per_program, &num_lfems_usage_per_program);
            SOCDNX_IF_ERR_EXIT(rv);

            /* Loop on all possible LFEM permutations and fill valid permutations struct */
            rv = arad_egr_prge_mgmt_pre_processing_program_lfem_permutation_struct_build(unit, program_ndx, lfems_usage_per_program,
                                                                                         num_lfems_usage_per_program, 0/*duplicate*/, &all_program_permutations[program_ndx]);

            /* If all permutations fail (can happen only in case of one LFEM used more than once in the same program) */
            if (rv != SOC_SAND_OK) {
                /* Loop on all possible LFEM permutations with duplications and fill valid permutations struct */
                rv = arad_egr_prge_mgmt_pre_processing_program_lfem_permutation_struct_build(unit, program_ndx, lfems_usage_per_program,
                                                                                             num_lfems_usage_per_program, 1/*duplicate*/, &all_program_permutations[program_ndx]);
                if (rv != SOC_SAND_OK) {
                    arad_egr_prog_editor_prog_name_get_by_id(program_ndx, &prog_name);
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Error - PRGE Pre-Processing failed to find LFEMs for program %s.\n\r"), prog_name));
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
Stage 2: Program allocation
1.	Sort the programs according to the total number of possible permutations
2.	Loop on all programs with 1 possible permutation
    a.	Set current permutation in the shadow struct and deploy
3.	Allocate all LFEMs using the following recursive algorithm (input: shadow struct and program list with valid permutations and indication for already allocated programs):
    a.	If all programs are allocated
        i.	Finish, return success
    b.	Choose first program in the list (lowest degree of freedom)
    c.	Loop on all valid permutations
        i.	Set current permutation in the shadow struct
        ii.	If not enough space in shadow struct
            1.	Remove current permutation from the shadow struct
            2.	Move to the next permutation
        iii.	Else
            1.	Mark program as allocated
            2.	Goto stage 3
            3.	If return value is failure
                a.	Remove current permutation from the shadow struct
                b.	Move to the next permutation
            4.	Else
                a.	Return success
    d.	If last deployment failed (no more permutations to try) return fail

*/
int
  arad_egr_prge_mgmt_programs_allocate_recursively(
     SOC_SAND_IN    int                             unit,
     lfem_maps_shadow_t                             lfem_maps_shadow,
     single_program_permutations_t                  *all_program_permutations,
     uint8                                          current_program
  )
{
    lfem_maps_shadow_t lfem_maps_shadow_dummy;
    uint8 permutation;
    char* prog_name;

    uint32 rv = SOC_SAND_OK;

    SOCDNX_INIT_FUNC_DEFS;

    /* If all programs are already allocated return success */
    if ((current_program >= ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) || (all_program_permutations[current_program].valid_permutation_number == 0)) {
        return SOC_SAND_OK;
    }

    arad_egr_prog_editor_prog_name_get_by_id(all_program_permutations[current_program].prog_index, &prog_name);
    _ARAD_EGR_PRGE_MGMT_LOG_PARAM(unit,"Allocating program: %s\n", prog_name);

    /* Create dummy shadow struct */
    rv = arad_egr_prge_mgmt_lfem_shadow_init(unit, lfem_maps_shadow_dummy);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = arad_egr_prge_mgmt_copy_shadow(unit, lfem_maps_shadow, lfem_maps_shadow_dummy);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Loop on all valid permutations */
    for (permutation = 0; permutation < all_program_permutations[current_program].valid_permutation_number; permutation++) {
        /* Set current permutation in the shadow struct */
        rv = arad_egr_prge_mgmt_copy_shadow_internal(unit, all_program_permutations[current_program].program_permutations[permutation], lfem_maps_shadow_dummy,
                                                     FALSE /* print_error */);

        /* If not enough space in shadow struct */
        if (rv != SOC_SAND_NO_ERR) {
            /* Remove current permutation from the shadow struct and move to the next permutation */
            rv = arad_egr_prge_mgmt_lfem_shadow_init(unit, lfem_maps_shadow_dummy);
            SOCDNX_IF_ERR_EXIT(rv);

            rv = arad_egr_prge_mgmt_copy_shadow(unit, lfem_maps_shadow, lfem_maps_shadow_dummy);
            SOCDNX_IF_ERR_EXIT(rv);
        }
        else {
            /* lfem_maps_shadow_dummy now contains the original shadow + new prmutation, try allocatng it */
            rv = arad_egr_prge_mgmt_programs_allocate_recursively(unit, lfem_maps_shadow_dummy, all_program_permutations, current_program+1);

            if (rv != SOC_SAND_NO_ERR) {
                /* Allocation failed - Remove current permutation from the shadow struct and move to the next permutation */
                rv = arad_egr_prge_mgmt_lfem_shadow_init(unit, lfem_maps_shadow_dummy);
                SOCDNX_IF_ERR_EXIT(rv);

                rv = arad_egr_prge_mgmt_copy_shadow(unit, lfem_maps_shadow, lfem_maps_shadow_dummy);
                SOCDNX_IF_ERR_EXIT(rv);
            }
            else {
                /* Allocation succeeded - current permutation is correct */

                /* Replace shadow with dummy shadow */
                rv = arad_egr_prge_mgmt_copy_shadow(unit, lfem_maps_shadow_dummy, lfem_maps_shadow);
                SOCDNX_IF_ERR_EXIT(rv);

                return SOC_SAND_NO_ERR;
            }
        }
    }

    /* If we got here it means that last permutation failed (no more permutations to try) return fail */
    return SOC_SAND_ERR;

exit:
    SOCDNX_FUNC_RETURN;
}

int
  arad_egr_prge_mgmt_program_allocation(
     SOC_SAND_IN    int                             unit,
     single_program_permutations_t                  *all_program_permutations,
     lfem_maps_shadow_t                             lfem_maps_shadow
  )
{
    uint8 current_program;

    uint32 rv = SOC_SAND_OK;

    SOCDNX_INIT_FUNC_DEFS;

    /* Sort the programs according to the total number of possible permutations */
    rv = arad_egr_prge_mgmt_sort_program_according_to_permutation_number(unit, all_program_permutations);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Loop on all programs with 1 possible permutation, find the first after all the managed */
    for (current_program = 0; (current_program < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) &&
                                (all_program_permutations[current_program].valid_permutation_number<1) ;
          current_program++ ) {
        /* Do nothing */
    }

   if (current_program < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS) {
        /* Allocate all LFEMs using recursive algorithm */
        rv = arad_egr_prge_mgmt_programs_allocate_recursively(unit, lfem_maps_shadow, all_program_permutations, current_program);
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* Finds the minimal and maximal instructions that jump to 'branch' */
uint32
  _arad_egr_prge_mgmt_branch_usage_min_max_instr_find(
     int        unit,
     ARAD_EGR_PROG_EDITOR_BRANCH branch,
     int        *min_jump_instr,
     int        *max_jump_instr
  ){

    ARAD_EGR_PROG_EDITOR_JUMP_POINT
        usage;

    SOCDNX_INIT_FUNC_DEFS;

    *min_jump_instr = ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS;
    *max_jump_instr = -1;

    for (usage = 0; usage < ARAD_EGR_PROG_EDITOR_JUMP_POINT_IDS_NOF; usage++ ) {
        if (mgmt_branches[unit][branch].branch_uses[usage]) {
            *min_jump_instr = MIN(*min_jump_instr, mgmt_branch_usage[unit][usage][0]);
            *max_jump_instr = MAX(*max_jump_instr, mgmt_branch_usage[unit][usage][1]);
        }
    }

    if ((*min_jump_instr == ARAD_EGR_PRGE_MAX_NOF_PROG_INSTRUCTIONS)
        || (*max_jump_instr == -1)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Error - No branch usage mentioned that define this branch's instructions")));
    }

    /* Set to actual jump-to instructions */
    *min_jump_instr += 4;
    *max_jump_instr += 4;

exit:
    SOCDNX_FUNC_RETURN;
}

/* Find all programs possibly jumping to a branch */
uint32
  _arad_egr_prge_mgmt_programs_jumping_to_branch_find(
     int unit, ARAD_EGR_PROG_EDITOR_BRANCH branch,
     ARAD_EGR_PROG_EDITOR_PROGRAMS jumped_from_programs[],
     uint8 *num_results) {

    ARAD_EGR_PROG_EDITOR_PROGRAMS
        program;

    uint32
        i;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(jumped_from_programs);
    SOCDNX_NULL_CHECK(num_results);

    *num_results = 0;

    for (program = 0; program < ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; program++) {
        if (!_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_OLD_INTERFACE(unit, program)) {
            /* program in management system */
            _ARAD_EGR_PRGE_MGMT_PROGRAM *p_program = &mgmt_programs[unit][program];
            for (i = 0; i < p_program->nof_jump_points; i++) {
                if (mgmt_branches[unit][branch].branch_uses[p_program->jump_points[i]]) {
                    /* program jumps to this branch (possibly) */
                    jumped_from_programs[(*num_results)++] = program;
                }
            }
        }
    }

    if (*num_results == 0) {
        
        SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("Error - No programs jump to the loaded branch")));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/* Load a specific branch */
uint32
  _arad_egr_prge_mgmt_branch_load(int unit, ARAD_EGR_PROG_EDITOR_BRANCH branch) {

    uint32 res;

    int
        min_jump_instr, max_jump_instr;
    int
        instr;
    int
        mem, entry_odd, entry_even, entry;

    /* Programs jumping to this branch */
    ARAD_EGR_PROG_EDITOR_PROGRAMS jumped_from_programs[ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS];
    uint8 nof_program_jumps; /* Number of Programs jumping to this branch */

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Find where to start the branch instructions.
       Note: currently only 1 program jumps to each branch and so min_jump_instr == max_jump_instr always. other cases were not tested */
    res = _arad_egr_prge_mgmt_branch_usage_min_max_instr_find(unit, branch, &min_jump_instr, &max_jump_instr);
    if (res != SOC_E_NONE) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Could not load branch.\n\r")));
    }
    /* Make sure there's enough room for the branch instructions */
    if (max_jump_instr + mgmt_branches[unit][branch].instructions_nof > ARAD_PP_EG_PROG_NOF_INSTR(unit)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - After jump, not enough instructions left for the branch.\n\r")));
    }
    res = _arad_egr_prge_mgmt_programs_jumping_to_branch_find(unit, branch, jumped_from_programs, &nof_program_jumps);
    if ((res != SOC_E_NONE) || (nof_program_jumps == 0)) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG("PRGE error - Loaded branch jump points aren't used by any program.\n\r")));
    }

    entry_odd = _ARAD_EGR_PROG_EDITOR_PTR_TO_BRANCH_ENTRY_ODD(branch_entries[unit][branch]);
    entry_even = _ARAD_EGR_PROG_EDITOR_PTR_TO_BRANCH_ENTRY_EVEN(branch_entries[unit][branch]);

    mem = EPNI_PRGE_INSTRUCTION_0m + (min_jump_instr >> 1) - ((min_jump_instr & 0x1) ? 0 : 1);
    entry = (min_jump_instr & 0x1) ? entry_even : entry_odd ;

    /* Fill NOPs from the smallest instruction possible for starting this branch to the largest */
    for ( ; min_jump_instr < max_jump_instr; min_jump_instr++ ) {
        mem   = (entry == entry_odd) ? (mem + 1) : mem;
        entry = (entry == entry_even) ? entry_odd : entry_even;
        res = _arad_egr_pge_mgmt_null_instruction_load(unit, mem, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
    }

    /* Load the actual branch instructions */
    for (instr = 0; instr < mgmt_branches[unit][branch].instructions_nof; instr++ ) {
        mem   = (entry == entry_odd) ? (mem + 1) : mem;
        entry = (entry == entry_even) ? entry_odd : entry_even;
        res = _arad_egr_prge_mgmt_instruction_load(unit, jumped_from_programs, nof_program_jumps,
                                                   &mgmt_branches[unit][branch].graph[instr], mem, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
    }

    if (!mgmt_branches[unit][branch].jump_out) {
        mem   = (entry == entry_odd) ? (mem + 1) : mem;
        entry = (entry == entry_even) ? entry_odd : entry_even;
        SOC_SAND_IF_ERR_EXIT( arad_egr_prog_editor_cat_nops(unit, mem, entry, entry_even, entry_odd) );
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_egr_prge_mgmt_program_deploy()",0,0);
}

/* Load all the needed branches to their entries */
uint32
  arad_egr_prge_mgmt_branches_load(
     int     unit
  ){
    uint32 rv = SOC_SAND_OK;
    ARAD_EGR_PROG_EDITOR_BRANCH branch;

    SOCDNX_INIT_FUNC_DEFS;

    for (branch = 0; branch < ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF; branch++) {
        if ((mgmt_branches[unit][branch].management_type > ARAD_EGR_PRGE_MGMT_MANAGEMENT_TYPE_NOT_MANAGED)
            && (branch_entries[unit][branch] > -1)) {
            /* branch is used, allocated and marked for loading by the management system */
            rv = _arad_egr_prge_mgmt_branch_load(unit, branch);
            SOCDNX_IF_ERR_EXIT(rv);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* Loads all needed managed program if possible */
int
  arad_egr_prge_mgmt_program_management_main(
     int    unit
  )
{
    single_program_permutations_t        all_program_permutations[ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS];

    uint8 prog_ndx, permutation;
    lfem_maps_shadow_t                             allocated_lfem_maps_shadow;
    uint32 rv = SOC_SAND_OK;
    char *prog_name;

    uint8 management_enabled;

    SOCDNX_INIT_FUNC_DEFS;

    _ARAD_EGR_PRGE_MGMT_LOG(unit,"Starting PRGE Management\n");

    management_enabled = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "prge_management_enable", 1);
    if (!management_enabled) {
        _ARAD_EGR_PRGE_MGMT_LOG(unit,"Programs manipulation is disabled!\n");
    }

    /* Allocate all_program_permutations array */
    for (prog_ndx = 0; prog_ndx<ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; prog_ndx++) {
        if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(unit, prog_ndx) && management_enabled) {
            all_program_permutations[prog_ndx].prog_index = prog_ndx;
            all_program_permutations[prog_ndx].program_permutations = NULL;
            SOCDNX_ALLOC(all_program_permutations[prog_ndx].program_permutations, lfem_maps_shadow_t, ARAD_EGR_PRGE_MGMT_MAX_NOF_PERMUTATIONS_PER_PROGRAM, "program_permutations");

            for (permutation=0; permutation < ARAD_EGR_PRGE_MGMT_MAX_NOF_PERMUTATIONS_PER_PROGRAM; permutation++) {
                rv = arad_egr_prge_mgmt_lfem_shadow_init(unit, all_program_permutations[prog_ndx].program_permutations[permutation]);
                SOCDNX_IF_ERR_EXIT(rv);
            }

            /* Clear permutation struct */
            all_program_permutations[prog_ndx].valid_permutation_number = 0;
            arad_egr_prog_editor_prog_name_get_by_id(prog_ndx, &prog_name);
            _ARAD_EGR_PRGE_MGMT_LOG_PARAM(unit,"Allocating (with management) program: %s\n", prog_name);

        }
        else {
            all_program_permutations[prog_ndx].prog_index = ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS;
            all_program_permutations[prog_ndx].valid_permutation_number = 0;
            all_program_permutations[prog_ndx].program_permutations = NULL;
            if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_LOADED_ONLY(unit, prog_ndx)
                || ((!management_enabled) && (!_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_OLD_INTERFACE(unit, prog_ndx)))) {
                /* The program needs loading as-is (either management disabled or program is strict */
                rv = _arad_egr_prge_mgmt_program_load(unit, prog_ndx);
                SOCDNX_IF_ERR_EXIT(rv);

                arad_egr_prog_editor_prog_name_get_by_id(prog_ndx, &prog_name);
                _ARAD_EGR_PRGE_MGMT_LOG_PARAM(unit,"Loading (without management) program: %s\n", prog_name);

            }
        }
    }

    _ARAD_EGR_PRGE_MGMT_LOG(unit,"Allocate buffer for program permutation success\n");

    /* Load branches */
    
    rv = arad_egr_prge_mgmt_branches_load(unit);
    SOCDNX_IF_ERR_EXIT(rv);

    _ARAD_EGR_PRGE_MGMT_LOG(unit,"Branches loading success\n");

    if (management_enabled) {
        /* Pre processing */
        rv = arad_egr_prge_mgmt_pre_processing(unit, all_program_permutations);
        SOCDNX_IF_ERR_EXIT(rv);

        _ARAD_EGR_PRGE_MGMT_LOG(unit,"Pre processing success\n");

        /* Program allocation */
        rv = arad_egr_prge_mgmt_lfem_shadow_init(unit, allocated_lfem_maps_shadow);
        SOCDNX_IF_ERR_EXIT(rv);

        rv = arad_egr_prge_mgmt_copy_shadow(unit, lfem_maps_shadow[unit], allocated_lfem_maps_shadow);
        SOCDNX_IF_ERR_EXIT(rv);

        rv = arad_egr_prge_mgmt_program_allocation(unit, all_program_permutations, allocated_lfem_maps_shadow);
        SOCDNX_IF_ERR_EXIT(rv);

        _ARAD_EGR_PRGE_MGMT_LOG(unit,"Program allocation success\n");

        rv = arad_egr_prge_mgmt_copy_shadow(unit, allocated_lfem_maps_shadow, lfem_maps_shadow[unit]);
        SOCDNX_IF_ERR_EXIT(rv);

        _ARAD_EGR_PRGE_MGMT_LOG(unit,"Copy to LFEMs shadow buffer success\n");

        for (prog_ndx = 0; prog_ndx<ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; prog_ndx++) {
            if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(unit, prog_ndx)) {
                rv = arad_egr_prge_mgmt_program_deploy(unit, prog_ndx, lfem_maps_shadow[unit], TRUE /*do_load*/);
                SOCDNX_IF_ERR_EXIT(rv);
            }
        }

        /* Free all_program_permutations array */
        for (prog_ndx = 0; prog_ndx<ARAD_EGR_PROG_EDITOR_PROG_NOF_PROGS; prog_ndx++) {
            if (all_program_permutations[prog_ndx].program_permutations) {
                SOCDNX_FREE(all_program_permutations[prog_ndx].program_permutations);
            }
        }
    }

    _ARAD_EGR_PRGE_MGMT_LOG(unit,"Managed programs loaded to HW successfully\n");

exit:
    SOCDNX_FUNC_RETURN;
}

/* ***********
   Diag
   ***********/
/* Print out an instruction for diagnostics */
void print_instr(_ARAD_EGR_PRGE_MGMT_INSTRUCTION* instr){
    int i;

    bsl_printf("\tselected_entry: %d\n",instr->selected_entry);

    if (!instr->ce.is_nop) {
        bsl_printf("\tCE fields\n");
        bsl_printf("\t\tRegister dependencies: "); for (i = 0; i <= instr->ce.last_reg_dep; i++) {bsl_printf(" %d",instr->ce.reg_dep_instr[i]);} bsl_printf("\n");
        bsl_printf("\t\tSize dependency: %d\n",instr->ce.size_dep);
        bsl_printf("\t\tImmediate register dependency: %d\n",instr->ce.reg_immediate_dep);
        bsl_printf("\t\tThis program copies byte to the header: %d\n",instr->ce.size_change);
    }
    else {
        bsl_printf("\t-CE NOP-\n");
    }

    if (!instr->alu.is_nop) {
        bsl_printf("\tALU fields\n");
        bsl_printf("\t\tRegister dependencies: "); for (i = 0; i <= instr->alu.last_reg_dep; i++) {bsl_printf(" %d",instr->alu.reg_dep_instr[i]);} bsl_printf("\n");
        bsl_printf("\t\tLFEM dependency: %d\n",instr->alu.lfem_dep);
        bsl_printf("\t\tImmediate Register dependency: %d\n",instr->alu.reg_immediate_dep);
        bsl_printf("\t\tRegister change bitmap: 0x%04x\n",instr->alu.dep_flags);
        bsl_printf("\t\tRegister conditional change bitmap: 0x%04x\n",instr->alu.cond_dep_flags);
    }
    else {
        bsl_printf("\t-ALU NOP-\n");
    }

}

/* Prints a branch for diagnostics */
void arad_egr_prge_mgmt_branch_print(int unit, uint32 branch){
    char *branch_name;
    int instr, instr_start, nop_start;
    uint32 res;

    arad_egr_prog_editor_branch_name_get_by_id(branch, &branch_name);

    res = _arad_egr_prge_mgmt_branch_usage_min_max_instr_find(unit, branch, &nop_start, &instr_start);
    if (res != SOC_E_NONE) {
        bsl_printf("PRGE error - Could find where branch %s is loaded.\n", branch_name);
        return;
    }

    bsl_printf("\nBranch %s:\n"
               "--------------------------------------------------\n", branch_name);

    for ( ; nop_start < instr_start; nop_start++) {
        bsl_printf("Entry %02d - NOP (Added dynamically by management)\n", nop_start);
    }

    for (instr = 0; instr < mgmt_branches[unit][branch].instructions_nof; instr++) {

        bsl_printf("Entry %02d - Instruction %02d: %s\n", instr + instr_start, instr,
                   mgmt_branches[unit][branch].graph[instr].doc);

    }

}

/* Prints a program out for diagnostics */
void arad_egr_prge_mgmt_program_print(int unit, uint32 program, char* prog_name, uint8 print_full_graph){
    int instr, entry, branch;
    int i;
    uint32 instr_nof = mgmt_programs[unit][program].instructions_nof;

    bsl_printf("\nProgram %s:\n"
             "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", prog_name);
    if (print_full_graph) {
        for (instr = 0; instr < instr_nof; instr++) {
            bsl_printf("Instruction %d\n", instr);
            print_instr(&mgmt_programs[unit][program].graph[instr]);
        }
    }
    else {
        for (entry = 0, instr = 0; instr < instr_nof; instr++, entry++) {
            for ( ; entry < mgmt_programs[unit][program].graph[instr].selected_entry; entry++) {
                bsl_printf("Entry %02d - NOP (Added dynamically by management)\n", entry);
            }
            bsl_printf("Entry %02d - Instruction %02d: %s\n", entry, instr,
                       mgmt_programs[unit][program].graph[instr].doc);
        }

        for (branch = 0; branch < ARAD_EGR_PROG_EDITOR_BRANCH_IDS_NOF; branch++ ) {
            for (i = 0; i < mgmt_programs[unit][program].nof_jump_points; i++) {
                if (mgmt_branches[unit][branch].branch_uses[ mgmt_programs[unit][program].jump_points[i] ]) {
                    /* Brunch 'branch' is jumped to from the printed program */
                    arad_egr_prge_mgmt_branch_print(unit, branch);
                    break; /* 'branch' printed for this program, stop checking the jump points of the program and continue to next branch */
                }
            }
        }
    }
}


int
  arad_egr_prge_mgmt_diag_print(
     SOC_SAND_IN int unit,
     SOC_SAND_IN ARAD_PP_PRGE_MGMT_DIAG_PARAMS *key_params
   ){

    uint32 res;

    char *prog_name = key_params->deploy?key_params->deploy:key_params->graph;
    ARAD_EGR_PROG_EDITOR_PROGRAMS prog;

    res = arad_egr_prog_editor_program_by_name_find(unit, prog_name, &prog);
    if (res != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_EGRESS,
            (BSL_META_U(unit,"Error - Program %s not found"), prog_name));
        return(-1);
    }

    if (key_params->deploy == NULL) {
        /* Print graph - chaeck that it exists */
        if (!_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_MANAGED(unit, prog)) {
            LOG_ERROR(BSL_LS_SOC_EGRESS,
                      (BSL_META_U(unit, "Error - No graph for program %s"), prog_name));
            return (-1);
        }
    }
    else if (_ARAD_EGR_PRGE_MGMT_IS_PROGRAM_OLD_INTERFACE(unit, prog)) {
        /* Print deploy (or load) - but the program uses the old interface :( */
        LOG_ERROR(BSL_LS_SOC_EGRESS,
                  (BSL_META_U(unit, "Error - program %s uses the old interface"), prog_name));
        return (-1);
    }


    arad_egr_prge_mgmt_program_print(unit, prog, prog_name, (key_params->deploy == NULL));

    return(0);

}

#if ARAD_DEBUG_IS_LVL1

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

/****************
 * UN-DEFINES   *
 ****************/
/* { */


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

#undef _ERR_MSG_MODULE_NAME
