/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmGroupConfig.hx"
#ifndef SB_ZF_FE2000PMGROUPCONFIG_CONSOLE_H
#define SB_ZF_FE2000PMGROUPCONFIG_CONSOLE_H



void
sbZfFe2000PmGroupConfig_Print(sbZfFe2000PmGroupConfig_t *pFromStruct);
char *
sbZfFe2000PmGroupConfig_SPrint(sbZfFe2000PmGroupConfig_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmGroupConfig_Validate(sbZfFe2000PmGroupConfig_t *pZf);
int
sbZfFe2000PmGroupConfig_SetField(sbZfFe2000PmGroupConfig_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE2000PMGROUPCONFIG_CONSOLE_H */
