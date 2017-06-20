/*
 * $Id: sbZfSbQe2000ElibVRTConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibVRT.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_CONSOLE_H



void
sbZfSbQe2000ElibVRT_Print(sbZfSbQe2000ElibVRT_t *pFromStruct);
char *
sbZfSbQe2000ElibVRT_SPrint(sbZfSbQe2000ElibVRT_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibVRT_Validate(sbZfSbQe2000ElibVRT_t *pZf);
int
sbZfSbQe2000ElibVRT_SetField(sbZfSbQe2000ElibVRT_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_VRT_ENTRY_CONSOLE_H */
