/*
 * $Id: sbZfSbQe2000ElibPriTableConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfSbQe2000ElibPriTable.hx"
#ifndef SB_ZF_SB_QE2000_ELIB_PRI_TABLE_CONSOLE_H
#define SB_ZF_SB_QE2000_ELIB_PRI_TABLE_CONSOLE_H



void
sbZfSbQe2000ElibPriTable_Print(sbZfSbQe2000ElibPriTable_t *pFromStruct);
char *
sbZfSbQe2000ElibPriTable_SPrint(sbZfSbQe2000ElibPriTable_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfSbQe2000ElibPriTable_Validate(sbZfSbQe2000ElibPriTable_t *pZf);
int
sbZfSbQe2000ElibPriTable_SetField(sbZfSbQe2000ElibPriTable_t *s, char* name, int value);


#endif /* ifndef SB_ZF_SB_QE2000_ELIB_PRI_TABLE_CONSOLE_H */
