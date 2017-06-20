/*
 * $Id: sbZfSbQe2000ElibMVTConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibMVT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibMVT_Print(sbZfSbQe2000ElibMVT_t *pFromStruct);
char *
sbZfSbQe2000ElibMVT_SPrint(sbZfSbQe2000ElibMVT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibMVT_Validate(sbZfSbQe2000ElibMVT_t *pZf);
int
sbZfSbQe2000ElibMVT_SetField(sbZfSbQe2000ElibMVT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_CONSOLE_H */
