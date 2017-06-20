/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_UCODE_H__
#define __NEMO_UCODE_H__

#ifdef __cplusplus
extern "C"
{
#endif


extern AG_U32 nNdUcodeMajorVersionNumber;
extern AG_U32 nNdUcodeMinorVersionNumber; 
extern AG_U32 nNdUcodeSize;
extern AG_U32 nNdUcodeDestMacInstr[];
extern AG_U32 nNdUcodeVlanInstr[];
extern AG_U32 nNdUcodeDestIpv4Instr[];
extern AG_U32 nNdUcodeDestIpv6Instr[];
extern AG_U32 nNdUcodeUdpCmdInstr[];
extern AG_U32 nNdUcodeMplsCmdInstr[];
extern AG_U32 nNdUcodeEcidCmdInstr[];
extern AG_U32 nNdUcodeSrcMacInstr[];
extern AG_U16 nNdUcodeInstructions[];
#ifdef CES16_BCM_VERSION
extern AG_U32 nNdUcodeL2TpCmdInstr[];
#endif


typedef enum
{
    AG_ND_UCODE_OXC_DONT_EXEC       = 0x0,
    AG_ND_UCODE_OXC_EXEC_IF_EQ      = 0x1,
    AG_ND_UCODE_OXC_EXEC_IF_NEQ     = 0x2,
    AG_ND_UCODE_OXC_EXEC            = 0x3,

} AgNdUcodeOxc;

/*ORI*/
/*add AG_ND_UCODE_OPCODE_LBL          = 0x7 to support l2tpv3 */
typedef enum
{
    AG_ND_UCODE_OPCODE_LBU          = 0x5,
    AG_ND_UCODE_OPCODE_LBE          = 0x6,
    AG_ND_UCODE_OPCODE_LBM          = 0x4,
    AG_ND_UCODE_OPCODE_UID          = 0x8,
    AG_ND_UCODE_OPCODE_CID          = 0x9,
    AG_ND_UCODE_OPCODE_LBL          = 0x7

} AgNdUcodeOpcode;



#ifdef __cplusplus
}
#endif

#endif /* __NEMO_UCODE_H__ */

