/*
 * $Id: sbZfSbQe2000ElibPTConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibPT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_PT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_PT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibPT_Print(sbZfSbQe2000ElibPT_t *pFromStruct);
char *
sbZfSbQe2000ElibPT_SPrint(sbZfSbQe2000ElibPT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibPT_Validate(sbZfSbQe2000ElibPT_t *pZf);
int
sbZfSbQe2000ElibPT_SetField(sbZfSbQe2000ElibPT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_PT_ENTRY_CONSOLE_H */
