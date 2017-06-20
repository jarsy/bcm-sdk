/*
 * $Id: sbZfKaQmFbLineConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmFbLine.hx"
#ifndef SB_ZF_ZFKAQMFBLINE_CONSOLE_H
#define SB_ZF_ZFKAQMFBLINE_CONSOLE_H



void
sbZfKaQmFbLine_Print(sbZfKaQmFbLine_t *pFromStruct);
char *
sbZfKaQmFbLine_SPrint(sbZfKaQmFbLine_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmFbLine_Validate(sbZfKaQmFbLine_t *pZf);
int
sbZfKaQmFbLine_SetField(sbZfKaQmFbLine_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMFBLINE_CONSOLE_H */
