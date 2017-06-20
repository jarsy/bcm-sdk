/*
 * $Id: sbZfKaPmLastLineConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaPmLastLine.hx"
#ifndef SB_ZF_ZFKAPMLASTLINE_CONSOLE_H
#define SB_ZF_ZFKAPMLASTLINE_CONSOLE_H



void
sbZfKaPmLastLine_Print(sbZfKaPmLastLine_t *pFromStruct);
char *
sbZfKaPmLastLine_SPrint(sbZfKaPmLastLine_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaPmLastLine_Validate(sbZfKaPmLastLine_t *pZf);
int
sbZfKaPmLastLine_SetField(sbZfKaPmLastLine_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAPMLASTLINE_CONSOLE_H */
