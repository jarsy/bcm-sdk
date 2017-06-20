/*
 * $Id: sbZfKaEpIp16BitRewriteConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIp16BitRewrite.hx"
#ifndef SB_ZF_ZFKAEPIP16BITREWRITE_CONSOLE_H
#define SB_ZF_ZFKAEPIP16BITREWRITE_CONSOLE_H



void
sbZfKaEpIp16BitRewrite_Print(sbZfKaEpIp16BitRewrite_t *pFromStruct);
char *
sbZfKaEpIp16BitRewrite_SPrint(sbZfKaEpIp16BitRewrite_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIp16BitRewrite_Validate(sbZfKaEpIp16BitRewrite_t *pZf);
int
sbZfKaEpIp16BitRewrite_SetField(sbZfKaEpIp16BitRewrite_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIP16BITREWRITE_CONSOLE_H */
