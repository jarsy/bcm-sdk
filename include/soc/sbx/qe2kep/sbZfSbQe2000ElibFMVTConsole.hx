/*
 * $Id: sbZfSbQe2000ElibFMVTConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibFMVT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibFMVT_Print(sbZfSbQe2000ElibFMVT_t *pFromStruct);
char *
sbZfSbQe2000ElibFMVT_SPrint(sbZfSbQe2000ElibFMVT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibFMVT_Validate(sbZfSbQe2000ElibFMVT_t *pZf);
int
sbZfSbQe2000ElibFMVT_SetField(sbZfSbQe2000ElibFMVT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_FMVT_ENTRY_CONSOLE_H */
