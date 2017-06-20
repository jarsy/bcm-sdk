/*
 * $Id: sbZfSbQe2000ElibPCTSingleConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibPCTSingle.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibPCTSingle_Print(sbZfSbQe2000ElibPCTSingle_t *pFromStruct);
char *
sbZfSbQe2000ElibPCTSingle_SPrint(sbZfSbQe2000ElibPCTSingle_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibPCTSingle_Validate(sbZfSbQe2000ElibPCTSingle_t *pZf);
int
sbZfSbQe2000ElibPCTSingle_SetField(sbZfSbQe2000ElibPCTSingle_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_CONSOLE_H */
