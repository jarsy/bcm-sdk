/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmOamTimerRawFormat.hx"
#ifndef SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_CONSOLE_H
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_CONSOLE_H



void
sbZfFe2000PmOamTimerRawFormat_Print(sbZfFe2000PmOamTimerRawFormat_t *pFromStruct);
char *
sbZfFe2000PmOamTimerRawFormat_SPrint(sbZfFe2000PmOamTimerRawFormat_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmOamTimerRawFormat_Validate(sbZfFe2000PmOamTimerRawFormat_t *pZf);
int
sbZfFe2000PmOamTimerRawFormat_SetField(sbZfFe2000PmOamTimerRawFormat_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_CONSOLE_H */
