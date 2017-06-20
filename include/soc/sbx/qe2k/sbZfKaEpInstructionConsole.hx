/*
 * $Id: sbZfKaEpInstructionConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpInstruction.hx"
#ifndef SB_ZF_ZFKAEPINSTRUCTION_CONSOLE_H
#define SB_ZF_ZFKAEPINSTRUCTION_CONSOLE_H



void
sbZfKaEpInstruction_Print(sbZfKaEpInstruction_t *pFromStruct);
char *
sbZfKaEpInstruction_SPrint(sbZfKaEpInstruction_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpInstruction_Validate(sbZfKaEpInstruction_t *pZf);
int
sbZfKaEpInstruction_SetField(sbZfKaEpInstruction_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPINSTRUCTION_CONSOLE_H */
