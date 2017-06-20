/*
 * $Id: sbZfKaEpIpPriExpTosRewriteConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpPriExpTosRewrite.hx"
#ifndef SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_CONSOLE_H
#define SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_CONSOLE_H



void
sbZfKaEpIpPriExpTosRewrite_Print(sbZfKaEpIpPriExpTosRewrite_t *pFromStruct);
char *
sbZfKaEpIpPriExpTosRewrite_SPrint(sbZfKaEpIpPriExpTosRewrite_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpPriExpTosRewrite_Validate(sbZfKaEpIpPriExpTosRewrite_t *pZf);
int
sbZfKaEpIpPriExpTosRewrite_SetField(sbZfKaEpIpPriExpTosRewrite_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_CONSOLE_H */
