/*
 * $Id: sbZfSbQe2000ElibVITConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibVIT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibVIT_Print(sbZfSbQe2000ElibVIT_t *pFromStruct);
char *
sbZfSbQe2000ElibVIT_SPrint(sbZfSbQe2000ElibVIT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibVIT_Validate(sbZfSbQe2000ElibVIT_t *pZf);
int
sbZfSbQe2000ElibVIT_SetField(sbZfSbQe2000ElibVIT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_VIT_ENTRY_CONSOLE_H */
