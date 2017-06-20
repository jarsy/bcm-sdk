/*
 * $Id: sbZfKaEpIpTtlRangeConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpTtlRange.hx"
#ifndef SB_ZF_ZFKAEPIPTTLRANGE_CONSOLE_H
#define SB_ZF_ZFKAEPIPTTLRANGE_CONSOLE_H



void
sbZfKaEpIpTtlRange_Print(sbZfKaEpIpTtlRange_t *pFromStruct);
char *
sbZfKaEpIpTtlRange_SPrint(sbZfKaEpIpTtlRange_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpTtlRange_Validate(sbZfKaEpIpTtlRange_t *pZf);
int
sbZfKaEpIpTtlRange_SetField(sbZfKaEpIpTtlRange_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPTTLRANGE_CONSOLE_H */
