/*
 * $Id: sbZfSbQe2000ElibCRTConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibCRT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibCRT_Print(sbZfSbQe2000ElibCRT_t *pFromStruct);
char *
sbZfSbQe2000ElibCRT_SPrint(sbZfSbQe2000ElibCRT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibCRT_Validate(sbZfSbQe2000ElibCRT_t *pZf);
int
sbZfSbQe2000ElibCRT_SetField(sbZfSbQe2000ElibCRT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_CONSOLE_H */
