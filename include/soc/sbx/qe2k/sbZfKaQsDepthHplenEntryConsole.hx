/*
 * $Id: sbZfKaQsDepthHplenEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsDepthHplenEntry.hx"
#ifndef SB_ZF_ZFKAQSDEPTHHPLENENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_CONSOLE_H



void
sbZfKaQsDepthHplenEntry_Print(sbZfKaQsDepthHplenEntry_t *pFromStruct);
char *
sbZfKaQsDepthHplenEntry_SPrint(sbZfKaQsDepthHplenEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsDepthHplenEntry_Validate(sbZfKaQsDepthHplenEntry_t *pZf);
int
sbZfKaQsDepthHplenEntry_SetField(sbZfKaQsDepthHplenEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSDEPTHHPLENENTRY_CONSOLE_H */
