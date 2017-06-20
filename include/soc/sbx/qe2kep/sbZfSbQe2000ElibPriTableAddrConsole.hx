/*
 * $Id: sbZfSbQe2000ElibPriTableAddrConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibPriTableAddr.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_CONSOLE_H



void
sbZfSbQe2000ElibPriTableAddr_Print(sbZfSbQe2000ElibPriTableAddr_t *pFromStruct);
char *
sbZfSbQe2000ElibPriTableAddr_SPrint(sbZfSbQe2000ElibPriTableAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibPriTableAddr_Validate(sbZfSbQe2000ElibPriTableAddr_t *pZf);
int
sbZfSbQe2000ElibPriTableAddr_SetField(sbZfSbQe2000ElibPriTableAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_CONSOLE_H */
