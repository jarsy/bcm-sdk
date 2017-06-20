/*
 * $Id: sbZfSbQe2000ElibCITConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibCIT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibCIT_Print(sbZfSbQe2000ElibCIT_t *pFromStruct);
char *
sbZfSbQe2000ElibCIT_SPrint(sbZfSbQe2000ElibCIT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibCIT_Validate(sbZfSbQe2000ElibCIT_t *pZf);
int
sbZfSbQe2000ElibCIT_SetField(sbZfSbQe2000ElibCIT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_CONSOLE_H */
