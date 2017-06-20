/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmPolicerConfig.hx"
#ifndef SB_ZF_FE_2000_PM_POLICERCFG_CONSOLE_H
#define SB_ZF_FE_2000_PM_POLICERCFG_CONSOLE_H



void
sbZfFe2000PmPolicerConfig_Print(sbZfFe2000PmPolicerConfig_t *pFromStruct);
char *
sbZfFe2000PmPolicerConfig_SPrint(sbZfFe2000PmPolicerConfig_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmPolicerConfig_Validate(sbZfFe2000PmPolicerConfig_t *pZf);
int
sbZfFe2000PmPolicerConfig_SetField(sbZfFe2000PmPolicerConfig_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_POLICERCFG_CONSOLE_H */
