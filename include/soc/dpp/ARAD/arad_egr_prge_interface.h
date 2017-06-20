/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __ARAD_EGR_PRGE_INTERFACE_INCLUDED__
/* { */
#define __ARAD_EGR_PRGE_INTERFACE_INCLUDED__

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

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*
   CE Defines
   ==========
*/

/* Data source */
#define ARAD_EGR_PROG_EDITOR_DATA_SRC_PKT           (0x0)
#define ARAD_EGR_PROG_EDITOR_DATA_SRC_GEN           (0x1)
#define ARAD_EGR_PROG_EDITOR_DATA_SRC_RES           (0x2)

/* Type of offset (register or constant) */
#define ARAD_EGR_PROG_EDITOR_OFFSET_CODE_REG        (0x0)
#define ARAD_EGR_PROG_EDITOR_OFFSET_CODE_CONST      (0x1)

/* Data offset */
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_0       (0x0)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_1       (0x1)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_2       (0x2)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_3       (0x3)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_SZ      (0x4)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_CONST_0     (0x5)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_CONST_128   (0x6)
#define ARAD_EGR_PROG_EDITOR_OFFSET_SRC_CONST_192   (0x7)

/* Type of size (register or constant */
#define ARAD_EGR_PROG_EDITOR_SIZE_CODE_REG          (0x0)
#define ARAD_EGR_PROG_EDITOR_SIZE_CODE_CONST        (0x1)

/* Size to copy */
#define ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_0         (0x0)
#define ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_1         (0x1)
#define ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_2         (0x2)
#define ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_3         (0x3)
#define ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_SZ        (0x4)
#define ARAD_EGR_PROG_EDITOR_SIZE_SRC_CONST_0       (0x5)


/*
   ALU Defines
   ===========
*/
#define ARAD_EGR_PROG_EDITOR_OP_TYPE_REG            (0x0)
#define ARAD_EGR_PROG_EDITOR_OP_TYPE_CONST          (0x1)
#define ARAD_EGR_PROG_EDITOR_OP_TYPE_FEM            (0x2)
#define ARAD_EGR_PROG_EDITOR_OP_TYPE_DATA_ADDR      (0x3)
#define ARAD_EGR_PROG_EDITOR_OP_TYPE_ALU            (0x4)

/* Op codes */
#define ARAD_PP_PRGE_ENG_OP_REG_R0       (0)
#define ARAD_PP_PRGE_ENG_OP_REG_R1       (1)
#define ARAD_PP_PRGE_ENG_OP_REG_R2       (2)
#define ARAD_PP_PRGE_ENG_OP_REG_R3       (3)
#define ARAD_PP_PRGE_ENG_OP_REG_RSZ      (4)
#define ARAD_PP_PRGE_ENG_OP_SIZE_ADDR    (5)
#define ARAD_PP_PRGE_ENG_OP_FEM          (6)
#define ARAD_PP_PRGE_ENG_OP_VALUE        (7)
#define ARAD_PP_PRGE_ENG_OP_ALU          (8)

/* CMP action */
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_NONE           (0x0)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_LT             (0x1)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_EQ             (0x2)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_GT             (0x4)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_GE             (ARAD_EGR_PROG_EDITOR_ALU_CMP_GT|ARAD_EGR_PROG_EDITOR_ALU_CMP_EQ)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_LE             (ARAD_EGR_PROG_EDITOR_ALU_CMP_LT|ARAD_EGR_PROG_EDITOR_ALU_CMP_EQ)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_NEQ            (ARAD_EGR_PROG_EDITOR_ALU_CMP_GT|ARAD_EGR_PROG_EDITOR_ALU_CMP_LT)
#define ARAD_EGR_PROG_EDITOR_ALU_CMP_ALL            (ARAD_EGR_PROG_EDITOR_ALU_CMP_GT|ARAD_EGR_PROG_EDITOR_ALU_CMP_EQ|ARAD_EGR_PROG_EDITOR_ALU_CMP_LT)

/* Arithmetic actions */
#define ARAD_PP_EG_PROG_ALU_SUB  (0x0)
#define ARAD_PP_EG_PROG_ALU_ADD  (0x1)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/*
 *  PRGE Instruction
 *  ================
 *  An instruction structure is:
 *  INSTR( <Concatenation Engine Instruction> , <ALU Instruction>, "DOCUMENTATION of the instruction" );
 *  The instructions are documented below.
 */
#define INSTR(_interface_ce_instr, _interface_alu_instr, _doc_str) \
do { \
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION instr; \
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION_clear(&instr); \
    SOC_SAND_CHECK_FUNC_RESULT((_interface_ce_instr), 370, exit); \
    SOC_SAND_CHECK_FUNC_RESULT((_interface_alu_instr), 370, exit); \
    instr.doc_str = _doc_str; \
    SOC_SAND_CHECK_FUNC_RESULT(arad_egr_prge_mgmt_instr_add(unit, &instr), 370, exit); \
} while(0)




/*
 * CE instruction macros
 * =====================
 * CE instructions are used to copy bytes to the header or load data to the LFEM buffer.
 */

/*
 * CE arguments macros
 */
/* Offset is based on a register and an offset (_base) */
#define R_OFST(_reg_num, _base)         ARAD_EGR_PROG_EDITOR_OFFSET_CODE_REG, (ARAD_EGR_PROG_EDITOR_OFFSET_SRC_REG_##_reg_num), (_base)
/* Offset is constant */
#define C_OFST(_offset)                 ARAD_EGR_PROG_EDITOR_OFFSET_CODE_CONST, 0, (_offset)
/* Size is based on a register and an offset (_base) */
#define R_SIZE(_reg_num, _base)         ARAD_EGR_PROG_EDITOR_SIZE_CODE_REG, (ARAD_EGR_PROG_EDITOR_SIZE_SRC_REG_##_reg_num), (_base)
/* Size is constant */
#define C_SIZE(_size)                   ARAD_EGR_PROG_EDITOR_SIZE_CODE_CONST, 0, (_size)



/*
 * xREAD instructions
 * These instructions are loading data to the LFEM buffer without copying bytes to the header
 */

/* Simple read from packet, generated header or resolved data (_src = PKT, GEN or RES respectively) from the specified offset */
#define CE_READ(_src, _offset) \
                arad_egr_prge_interface_read_data_instr(unit, (ARAD_EGR_PROG_EDITOR_DATA_SRC_##_src), _offset, ARAD_EGR_PROG_EDITOR_LFEM_NULL, &instr)

/* Read from packet, generated header or resolved data (_src = PKT, GEN or RES respectively) from the specified offset using a special LFEM macro */
#define CE_LREAD(_src, _offset, _lfem) \
                arad_egr_prge_interface_read_data_instr(unit, (ARAD_EGR_PROG_EDITOR_DATA_SRC_##_src), _offset, (ARAD_EGR_PROG_EDITOR_##_lfem), &instr)

/* Read from a pre-defined field from the specified offset */
#define CE_FREAD(_field_enum, _offset) \
                arad_egr_prge_interface_read_field_instr(unit, (_field_enum), (_offset), ARAD_EGR_PROG_EDITOR_LFEM_NULL, &instr)

/* Read from a pre-defined field from the specified offset using a special LFEM macro */
#define CE_FLREAD(_field_enum, _offset, _lfem) \
                arad_egr_prge_interface_read_field_instr(unit, (_field_enum), (_offset), (ARAD_EGR_PROG_EDITOR_##_lfem), &instr)

/*
 * xCOPY instructions
 * These instructions are copying _size bytes to the header
 */

/* Simple copy from packet, generated header or resolved data (_src = PKT, GEN or RES respectively) from the specified offset */
#define CE_COPY(_src, _offset, _size) \
                arad_egr_prge_interface_copy_data_instr(unit, (ARAD_EGR_PROG_EDITOR_DATA_SRC_##_src), _offset, _size, ARAD_EGR_PROG_EDITOR_LFEM_NULL, &instr)

/* Copy from packet, generated header or resolved data (_src = PKT, GEN or RES respectively) from the specified offset using a special LFEM macro */
#define CE_LCOPY(_src, _offset, _size, _lfem) \
                arad_egr_prge_interface_copy_data_instr(unit, (ARAD_EGR_PROG_EDITOR_DATA_SRC_##_src), _offset, _size, (ARAD_EGR_PROG_EDITOR_##_lfem), &instr)

/* Copy from a pre-defined field from the specified offset */
#define CE_FCOPY(_field_enum, _offset, _size) \
                arad_egr_prge_interface_copy_field_instr(unit, (_field_enum), (_offset), _size, ARAD_EGR_PROG_EDITOR_LFEM_NULL, &instr)

/* Copy from a pre-defined field from the specified offset using a special LFEM macro */
#define CE_FLCOPY(_field_enum, _offset, _size, _lfem) \
                arad_egr_prge_interface_copy_field_instr(unit, (_field_enum), (_offset), _size, (ARAD_EGR_PROG_EDITOR_##_lfem), &instr)

/* No operation in the concat-engine for this instruction */
#define CE_NOP                      arad_egr_prge_interface_ce_nop(unit, &instr)



/*
 * ALU instruction macros
 * ======================
 * ALU instructions are used to perform aritmetic calculations, load values to registers, check conditions and/or perform jumps.
 */

/*
 * ALU arguments macros
 */
/* Operand is a register */
#define REG_OP(_reg_num)    ARAD_EGR_PROG_EDITOR_OP_TYPE_REG, (ARAD_PP_PRGE_ENG_OP_REG_R##_reg_num)
/* Operand is constant */
#define CNST(_val)          ARAD_EGR_PROG_EDITOR_OP_TYPE_CONST, (_val)
/* Operand is the FEM buffer (Cannot be the dest operand) */
#define FEM                 ARAD_EGR_PROG_EDITOR_OP_TYPE_FEM, 0
/* Operand is the Data-Memory access address (Can only be the dest operand) */
#define DATA_ADDR           ARAD_EGR_PROG_EDITOR_OP_TYPE_DATA_ADDR, (ARAD_PP_PRGE_ENG_DST_DATA_ADDR)
#define ALU                 ARAD_EGR_PROG_EDITOR_OP_TYPE_ALU, 0


/* _dest = _op1 + _op2 */
#define ALU_ADD(_op1, _op2, _dst) \
                arad_egr_prge_interface_add_instr(unit, _op1, _op2, _dst, &instr)

/* _dest = _op1 - _op2 */
#define ALU_SUB(_op1, _op2, _dst) \
                arad_egr_prge_interface_sub_instr(unit, _op1, _op2, _dst, &instr)

/* _dst = _val */
#define ALU_SET(_val, _dst) \
                arad_egr_prge_interface_set_instr(unit, (_val), _dst, &instr)

/* performs a compare (>, <, ==, !=, etc.) between _op1 and _op2 and if the result is TRUE, performs _dst = _res
   Compare options:
     LT  - Less Then
     EQ  - EQual
     GT  - Greater Then
     GE  - Greater or Equal
     LE  - Less then or Equal
     NEQ - Not EQual
*/
#define ALU_SET_IF(_op1, _cmp_type, _op2, _res, _dst) \
                arad_egr_prge_interface_set_if_instr(unit, _op1, (ARAD_EGR_PROG_EDITOR_ALU_CMP_##_cmp_type), _op2, _res, _dst, ARAD_PP_EG_PROG_ALU_SUB, &instr)

/* performs a compare (>, <, ==, !=, etc.) between (_op1 + _op2) and zero, if the result is TRUE, performs _dst = _res
   Compare options:
     LT  - Less Then
     EQ  - EQual
     GT  - Greater Then
     GE  - Greater or Equal
     LE  - Less then or Equal
     NEQ - Not EQual
*/
#define ALU_SET_COMP_SUM(_op1, _op2, _cmp_type, _res, _dst) \
                    arad_egr_prge_interface_set_if_instr(unit, _op1, (ARAD_EGR_PROG_EDITOR_ALU_CMP_##_cmp_type), _op2, _res, _dst, ARAD_PP_EG_PROG_ALU_ADD, &instr)

/* Jump to a specific branch if some condition applies.
   Conditions are written the same as in ALU_SET_IF

   _jump_point is used for multiple branches to be able to be jumped from the same instruction
   and know in which instruction to start
*/
#define ALU_JMP_IF(_op1, _cmp_type, _op2, _branch, _jump_point) \
                arad_egr_prge_interface_jump_if_instr(unit, _op1, (ARAD_EGR_PROG_EDITOR_ALU_CMP_##_cmp_type), _op2, \
                                                (ARAD_EGR_PROG_EDITOR_##_branch), (ARAD_EGR_PROG_EDITOR_##_jump_point), &instr)

/* Jump to a branch previously allocated and loaded to a register
   This is meant to be used by branches which addresses are written to, for example, the port-var

   _addr_op cannot be of type CNST or FEM!  only REG_OP is supported.

   _jump_point is used for multiple branches to be able to be jumped from the same instruction
   and know in which instruction to start.
*/
#define ALU_JMP(_addr_op, _jump_point) \
                arad_egr_prge_interface_jump_instr(unit, _addr_op, (ARAD_EGR_PROG_EDITOR_##_jump_point), &instr)

/* No operation in the ALU for this instruction */
#define ALU_NOP                      arad_egr_prge_interface_alu_nop(unit, &instr)



/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* Pre-defined data fields in the packet, generated headers or resolved data. */
typedef enum predefined_data_s {

    /* Packet common fields */
    PrgePktFieldFheiSize_UseR2TM,
    PrgePktFieldFwdHdrOffset_UseR2TM,

    /* Resolved Data fields */
    /* QAX */
    PrgeDataFieldOutLifProfile5,
    PrgeDataFieldOutLifProfile6,
    PrgeDataFieldOutLifProfile7,
    PrgeDataFieldNativeLLSize,
    PrgeDataFieldCud2,
    PrgeDataFieldLBKey,
    /* Jericho and above */
    PrgeDataFieldOutLifProfile1,
    PrgeDataFieldOutLifProfile2,
    PrgeDataFieldOutLifProfile3,
    PrgeDataFieldOutLifProfile4,
    PrgeDataFieldOutLifMsb,
    /* Jericho _ONLY_ */
    PrgeDataFieldEesData3,
    /* Arad */
    PrgeDataFieldR0,
    PrgeDataFieldR1,
    PrgeDataFieldR2,
    PrgeDataFieldR3,
    PrgeDataFieldSrcSysPort,
    PrgeDataFieldDstSysPort,
    PrgeDataFieldNwkHdrSize,
    PrgeDataFieldSysHdrSize,
    PrgeDataFieldEesData1,
    PrgeDataFieldEesData2,
    PrgeDataFieldProgVar,
    PrgeDataFieldOutTmPort,
    PrgeDataFieldOutPpPort,
    PrgeDataFieldOamCounterValue,
    PrgeDataFieldOamExtId,
    PrgeDataFieldOamExtSubType,
    PrgeDataFieldTod1588,
    PrgeDataFieldTodNtp,
    PrgeDataFieldVsiVar,
    PrgeDataFieldAceVar,
    PrgeDataFieldOutLif,
    PrgeDataFieldTmPortVar,
    PrgeDataFieldPpPortVar,
    PrgeDataFieldOamId,
    PrgeDataFieldOamSubType,
    PrgeDataFieldDataMemoryEntry,

    /* Number of pre-defined data fields */
    PRGE_PREDEFINED_FIELDS_NOF
} predefined_data_e;

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

/* Init interface instruction struct */
void
  ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION_clear(
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION * instr
  );

/* Init inreface pre-defined fields availability by device */
void
  arad_egr_prge_interface_field_available_by_device_init(
     int unit
  );

/* CE instruction to read data to the LFEM buffer without copying data to the packet */
uint32
  arad_egr_prge_interface_read_data_instr(
     int     unit,
     uint32  data_src,
     uint32  offset_code,
     uint32  offset_src,
     int     offset_base,
     ARAD_EGR_PROG_EDITOR_LFEM lfem,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* CE instruction to read data to the LFEM buffer without copying data to the packet.
   The data is a pre-defined field. */
uint32
  arad_egr_prge_interface_read_field_instr(
     int                                  unit,
     predefined_data_e                    field,
     int                                  offset,
     ARAD_EGR_PROG_EDITOR_LFEM            lfem,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* CE instruction to copy some bytes to the packet */
uint32
  arad_egr_prge_interface_copy_data_instr(
     int                                  unit,
     uint32                               data_src,
     uint32                               offset_code,
     uint32                               offset_src,
     int                                  offset_base,
     uint32                               size_code,
     uint32                               size_src,
     int                                  size_base,
     ARAD_EGR_PROG_EDITOR_LFEM            lfem,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* CE instruction to copy some bytes to the packet.
   The data is a pre-defined field. */
uint32
  arad_egr_prge_interface_copy_field_instr(
     int     unit,
     predefined_data_e                    field,
     int                                  offset,
     uint32                               size_code,
     uint32                               size_src,
     int                                  size_base,
     ARAD_EGR_PROG_EDITOR_LFEM            lfem,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* Fill instruction as a CE NOP */
uint32
  arad_egr_prge_interface_ce_nop(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );


/* ALU instruction to add 2 operands and write the result to a register */
uint32
  arad_egr_prge_interface_add_instr(
    int      unit,
    uint32   op1_type,
    uint32   op1_val,
    uint32   op2_type,
    uint32   op2_val,
    uint32   dst_type,
    uint32   dst_reg,
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* ALU instruction to subtract op2 from op1 and write the result to a register */
uint32
  arad_egr_prge_interface_sub_instr(
    int      unit,
    uint32   op1_type,
    uint32   op1_val,
    uint32   op2_type,
    uint32   op2_val,
    uint32   dst_type,
    uint32   dst_reg,
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* ALU instruction to set a constant value to a register */
uint32
  arad_egr_prge_interface_set_instr(
    int      unit,
    uint32   val,
    uint32   dst_type,
    uint32   dst_reg,
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* ALU instruction to set 'res' to a register if some condition applies between op1 and op2 */
uint32
  arad_egr_prge_interface_set_if_instr(
    int      unit,
    uint32   op1_type,
    uint32   op1_val,
    uint32   cmp_type,
    uint32   op2_type,
    uint32   op2_val,
    uint32   res_type,
    uint32   res_val,
    uint32   __ignored,
    uint32   dst_reg,
    uint32   alu_action,
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* ALU instruction to jump to a branch if some condition applies between op1 and op2. op1, op2 cannot be constants */
uint32
  arad_egr_prge_interface_jump_if_instr(
    int      unit,
    uint32   op1_type,
    uint32   op1_val,
    uint32   cmp_type,
    uint32   op2_type,
    uint32   op2_val,
    ARAD_EGR_PROG_EDITOR_BRANCH branch,
    ARAD_EGR_PROG_EDITOR_JUMP_POINT branch_usage,
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* ALU instruction to jump to a branch (unconditionally) that is loaded to a register */
uint32
  arad_egr_prge_interface_jump_instr(
    int      unit,
    uint32   addr_op_type,
    uint32   addr_op_val,
    ARAD_EGR_PROG_EDITOR_JUMP_POINT branch_usage,
    ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );

/* Fill instruction as a ALU NOP */
uint32
  arad_egr_prge_interface_alu_nop(
     int     unit,
     ARAD_EGR_PRGE_MGMT_INTERFACE_INSTRUCTION  *instr
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_EGR_PRGE_INTERFACE_INCLUDED__*/
#endif

