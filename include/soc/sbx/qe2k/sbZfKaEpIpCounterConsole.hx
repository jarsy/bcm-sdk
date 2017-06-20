/*
 * $Id: sbZfKaEpIpCounterConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpCounter.hx"
#ifndef SB_ZF_ZFKAEPIPCOUNTER_CONSOLE_H
#define SB_ZF_ZFKAEPIPCOUNTER_CONSOLE_H



void
sbZfKaEpIpCounter_Print(sbZfKaEpIpCounter_t *pFromStruct);
char *
sbZfKaEpIpCounter_SPrint(sbZfKaEpIpCounter_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpCounter_Validate(sbZfKaEpIpCounter_t *pZf);
int
sbZfKaEpIpCounter_SetField(sbZfKaEpIpCounter_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPCOUNTER_CONSOLE_H */
