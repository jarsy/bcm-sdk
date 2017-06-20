/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmOamSeqGenRawFormat.hx"
#ifndef SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_CONSOLE_H
#define SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_CONSOLE_H



void
sbZfFe2000PmOamSeqGenRawFormat_Print(sbZfFe2000PmOamSeqGenRawFormat_t *pFromStruct);
char *
sbZfFe2000PmOamSeqGenRawFormat_SPrint(sbZfFe2000PmOamSeqGenRawFormat_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmOamSeqGenRawFormat_Validate(sbZfFe2000PmOamSeqGenRawFormat_t *pZf);
int
sbZfFe2000PmOamSeqGenRawFormat_SetField(sbZfFe2000PmOamSeqGenRawFormat_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_OAMSEQGENRAWFORMAT_CONSOLE_H */
