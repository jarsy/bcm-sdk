/*
 * $Id: sbZfKaEpIpPrependConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpPrepend.hx"
#ifndef SB_ZF_ZFKAEPIPPREPEND_CONSOLE_H
#define SB_ZF_ZFKAEPIPPREPEND_CONSOLE_H



void
sbZfKaEpIpPrepend_Print(sbZfKaEpIpPrepend_t *pFromStruct);
char *
sbZfKaEpIpPrepend_SPrint(sbZfKaEpIpPrepend_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpPrepend_Validate(sbZfKaEpIpPrepend_t *pZf);
int
sbZfKaEpIpPrepend_SetField(sbZfKaEpIpPrepend_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPPREPEND_CONSOLE_H */
