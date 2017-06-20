/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmOamTimerConfig.hx"
#ifndef SB_ZF_FE_2000_PM_OAMTIMERCFG_CONSOLE_H
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_CONSOLE_H



void
sbZfFe2000PmOamTimerConfig_Print(sbZfFe2000PmOamTimerConfig_t *pFromStruct);
char *
sbZfFe2000PmOamTimerConfig_SPrint(sbZfFe2000PmOamTimerConfig_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmOamTimerConfig_Validate(sbZfFe2000PmOamTimerConfig_t *pZf);
int
sbZfFe2000PmOamTimerConfig_SetField(sbZfFe2000PmOamTimerConfig_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_OAMTIMERCFG_CONSOLE_H */
