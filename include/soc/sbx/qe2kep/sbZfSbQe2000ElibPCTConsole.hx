/*
 * $Id: sbZfSbQe2000ElibPCTConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibPCT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibPCT_Print(sbZfSbQe2000ElibPCT_t *pFromStruct);
char *
sbZfSbQe2000ElibPCT_SPrint(sbZfSbQe2000ElibPCT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibPCT_Validate(sbZfSbQe2000ElibPCT_t *pZf);
int
sbZfSbQe2000ElibPCT_SetField(sbZfSbQe2000ElibPCT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_PCT_ENTRY_CONSOLE_H */
