/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmPolicerRawFormat.hx"
#ifndef SB_ZF_FE_2000_PM_POLICERRAWFORMAT_CONSOLE_H
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_CONSOLE_H



void
sbZfFe2000PmPolicerRawFormat_Print(sbZfFe2000PmPolicerRawFormat_t *pFromStruct);
char *
sbZfFe2000PmPolicerRawFormat_SPrint(sbZfFe2000PmPolicerRawFormat_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmPolicerRawFormat_Validate(sbZfFe2000PmPolicerRawFormat_t *pZf);
int
sbZfFe2000PmPolicerRawFormat_SetField(sbZfFe2000PmPolicerRawFormat_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_POLICERRAWFORMAT_CONSOLE_H */
