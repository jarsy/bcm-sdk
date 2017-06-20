/*
 * $Id: sbZfG2EplibMvtEntryConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfG2EplibMvtEntry.hx"
#ifndef SB_ZF_G2_EPLIB_MVTENTRY_CONSOLE_H
#define SB_ZF_G2_EPLIB_MVTENTRY_CONSOLE_H



void
sbZfG2EplibMvtEntry_Print(sbZfG2EplibMvtEntry_t *pFromStruct);
char *
sbZfG2EplibMvtEntry_SPrint(sbZfG2EplibMvtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfG2EplibMvtEntry_Validate(sbZfG2EplibMvtEntry_t *pZf);
int
sbZfG2EplibMvtEntry_SetField(sbZfG2EplibMvtEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_G2_EPLIB_MVTENTRY_CONSOLE_H */
