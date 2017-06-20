/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmProfileSeqGenMemory.hx"
#ifndef SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_CONSOLE_H
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_CONSOLE_H



void
sbZfFe2000PmProfileSeqGenMemory_Print(sbZfFe2000PmProfileSeqGenMemory_t *pFromStruct);
char *
sbZfFe2000PmProfileSeqGenMemory_SPrint(sbZfFe2000PmProfileSeqGenMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmProfileSeqGenMemory_Validate(sbZfFe2000PmProfileSeqGenMemory_t *pZf);
int
sbZfFe2000PmProfileSeqGenMemory_SetField(sbZfFe2000PmProfileSeqGenMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_CONSOLE_H */
