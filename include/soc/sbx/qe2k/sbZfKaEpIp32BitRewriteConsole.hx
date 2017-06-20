/*
 * $Id: sbZfKaEpIp32BitRewriteConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIp32BitRewrite.hx"
#ifndef SB_ZF_ZFKAEPIP32BITREWRITE_CONSOLE_H
#define SB_ZF_ZFKAEPIP32BITREWRITE_CONSOLE_H



void
sbZfKaEpIp32BitRewrite_Print(sbZfKaEpIp32BitRewrite_t *pFromStruct);
char *
sbZfKaEpIp32BitRewrite_SPrint(sbZfKaEpIp32BitRewrite_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIp32BitRewrite_Validate(sbZfKaEpIp32BitRewrite_t *pZf);
int
sbZfKaEpIp32BitRewrite_SetField(sbZfKaEpIp32BitRewrite_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIP32BITREWRITE_CONSOLE_H */
