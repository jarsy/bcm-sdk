/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfC2PmProfileSeqGenMemory.hx"
#ifndef SB_ZF_C2_PM_PROFSEQGENMEMORY_CONSOLE_H
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_CONSOLE_H



void
sbZfC2PmProfileSeqGenMemory_Print(sbZfC2PmProfileSeqGenMemory_t *pFromStruct);
char *
sbZfC2PmProfileSeqGenMemory_SPrint(sbZfC2PmProfileSeqGenMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfC2PmProfileSeqGenMemory_Validate(sbZfC2PmProfileSeqGenMemory_t *pZf);
int
sbZfC2PmProfileSeqGenMemory_SetField(sbZfC2PmProfileSeqGenMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2_PM_PROFSEQGENMEMORY_CONSOLE_H */
