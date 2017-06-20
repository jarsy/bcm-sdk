/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __ARAD_EGR_PRGE_MGMT_INCLUDED__
/* { */
#define __ARAD_EGR_PRGE_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/TMC/tmc_api_stack.h>

#include <soc/dpp/ARAD/arad_egr_prog_editor.h>

/*
 * Egress Programmable editor management
 * =====================================
 *
 * The Egress Editor management system is a module that automatically and dynamically modify
 * programs so they can share resources.
 * The system uses an interface documented in arad_egr_prge_interface.h
 * The general structure of a program using this interface is:
 *
 *     res = arad_egr_prge_mgmt_build_program_start(unit, <PROGRAM NAME>);
 *     SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
 *
 *     INSTR( CE_INSTR(...), ALU_INSTR(...) );
 *     INSTR( CE_INSTR(...), ALU_INSTR(...) );
 *     INSTR( CE_INSTR(...), ALU_INSTR(...) );
 *     ...
 *
 *     res = arad_egr_prge_mgmt_build_program_end(unit);
 *     SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
 *
 * The possibilities for CE_INSTR are
 * CE_COPY, CE_LCOPY, CE_FCOPY, CE_FLCOPY,
 * CE_READ, CE_LREAD, CE_FREAD, CE_FLREAD
 * or CE_NOP for no operation in the concat-engine for this instruction
 *
 * The possibilities for ALU_INSTR are
 * ALU_ADD, ALU_SUB, ALU_SET, ALU_SET_IF, ALU_JMP, ALU_JMP_IF
 * or ALU_NOP for no operation in the ALU for this instruction
 *
 * Programs using ALU_JMP or ALU_JMP_IF are programs that uses branches and
 * should define jump-points when jumping to those branches (see documentation
 * of ALU_JMP and ALU_JUMP_IF). The branch's instruction are separately defined.
 * To define a branch instructions, use the following sequence:
 *    res = arad_egr_prge_mgmt_build_branch_start(unit, <BRANCH NAME>);
 *    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
 *
 *    / * State all the jump points that may lead to this branch * /
 *    res = arad_egr_prge_mgmt_branch_usage_add(unit, <JUMP_POINT1_NAME>);
 *    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
 *    res = arad_egr_prge_mgmt_branch_usage_add(unit, <JUMP_POINT2_NAME>);
 *    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
 *    ...
 *
 *    INSTR( CE_INSTR(...), ALU_INSTR(...) );
 *    INSTR( CE_INSTR(...), ALU_INSTR(...) );
 *    INSTR( CE_INSTR(...), ALU_INSTR(...) );
 *    ...
 *
 *    res = arad_egr_prge_mgmt_build_branch_end(unit);
 *    SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
 *
 * The instructions are documented in arad_egr_prge_interface.h
 */

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* To mark a CE instruction as NOP */
#define _ARAD_EGR_PRGE_MGMT_CE_DATA_SRC_NOP (255)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* CE instruction interface */
typedef struct {

   uint8 src_select;

   int   offset_base;
   uint8 offset_src;

   int   size_base;
   uint8 size_src;

   ARAD_EGR_PROG_EDITOR_LFEM lfem;

} ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION;

/* ALU instructio interface */
typedef struct {
/*  op                    alu                    cmp
    value,    op1       , action,      op2     , action  ,   op3  , dst_select */

    int op_value;

    uint8 op1;
    uint8 alu_action;
    uint8 op2;

    uint8 cmp_action;
    uint8 op3;

    uint8 dst_select;

} ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION;

/* Instruction interface - to be used when writing the program */
typedef struct {
    /* CE fields */
    ARAD_EGR_PRGE_MGMT_CE_INTERFACE_INSTRUCTION ce_interface_instruction;

    /* ALU fields */
    ARAD_EGR_PRGE_MGMT_ALU_INTERFACE_INSTRUCTION alu_interface_instruction;

    /* Instruction documentation */
    char *doc_str;

} ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION;


/* ****************
   Diag
   ****************/

typedef struct {
    char* graph;
    char* deploy;
} ARAD_PP_PRGE_MGMT_DIAG_PARAMS;

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

/* Initiate the management system data.*/
uint32
  arad_egr_prge_mgmt_init(
     int     unit
  );

/* Signals start of the instructions sequence of a program. */
uint32
  arad_egr_prge_mgmt_build_program_start(
     int     unit,
     ARAD_EGR_PROG_EDITOR_PROGRAMS   program
  );

/* Adds an instruction to this program */
uint32
  arad_egr_prge_mgmt_instr_add(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION    *ifc_instr
  );

/* Signals a program end in the insertion state machine */
uint32
  arad_egr_prge_mgmt_build_program_end(
     int     unit
  );

/* Signals that the current program branches (has jump instruction */
uint32
  arad_egr_prge_mgmt_program_jump_point_add(
     int     unit,
     ARAD_EGR_PROG_EDITOR_JUMP_POINT jump_point
  );

/* Sets the current program status to load-only */
uint32
  arad_egr_prge_mgmt_current_program_load_only_set(
     int     unit
  );

/* Signals the start of the branch instruction sequence */
uint32
  arad_egr_prge_mgmt_build_branch_start(
     int     unit,
     ARAD_EGR_PROG_EDITOR_BRANCH    branch
  );

/* Signals that the current branch should consider branch_usage when loaded */
uint32
  arad_egr_prge_mgmt_branch_usage_add(
     int     unit,
     ARAD_EGR_PROG_EDITOR_JUMP_POINT    jump_point
  );

/* Signals the end of the branch instruction sequence */
uint32
  arad_egr_prge_mgmt_build_branch_end(
     int     unit
  );

/* Loads all needed managed program if possible */
int
  arad_egr_prge_mgmt_program_management_main(
     int     unit
  );

/* Load all the needed branches to their entries */
uint32
  arad_egr_prge_mgmt_branches_load(
     int     unit
  );

/* ***********
   Diag
   ***********/

int
  arad_egr_prge_mgmt_diag_print(
     SOC_SAND_IN int unit,
     SOC_SAND_IN ARAD_PP_PRGE_MGMT_DIAG_PARAMS *key_params
   );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_EGR_PRGE_MGMT_INCLUDED__*/
#endif

