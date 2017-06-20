/*
 * $Id: sbZfSbQe2000ElibVRTConsole.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibVRTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibVRT_Print(sbZfSbQe2000ElibVRT_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: vid0=0x%03x"), (unsigned int)  pFromStruct->m_nVid0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" vid1=0x%03x"), (unsigned int)  pFromStruct->m_nVid1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" off_set=0x%02x"), (unsigned int)  pFromStruct->m_nOffset));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_0=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_1=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_2=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_3=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_3));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_4=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_4));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_5=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_5));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_6=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_6));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_7=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_7));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_8=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_8));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_9=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_9));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_10=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_10));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_11=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_11));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_12=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_12));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_13=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_13));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_14=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_14));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_15=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_15));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_16=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_16));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_17=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_17));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_18=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_18));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_19=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_19));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_20=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_20));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_21=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_21));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_22=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_22));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_23=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_23));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_24=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_24));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_25=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_25));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_26=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_26));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_27=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_27));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_28=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_28));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_29=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_29));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_30=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_30));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_31=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_31));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_32=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_32));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_33=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_33));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_34=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_34));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_35=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_35));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_36=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_36));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_37=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_37));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_38=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_38));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_39=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_39));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_40=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_40));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_41=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_41));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_42=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_42));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_43=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_43));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_44=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_44));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_45=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_45));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_46=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_46));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_47=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_47));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" port_state_48=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_48));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("SbQe2000ElibVRT:: port_state_49=0x%01x"), (unsigned int)  pFromStruct->m_nPortState_49));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibVRT_SPrint(sbZfSbQe2000ElibVRT_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: vid0=0x%03x", (unsigned int)  pFromStruct->m_nVid0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," vid1=0x%03x", (unsigned int)  pFromStruct->m_nVid1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," off_set=0x%02x", (unsigned int)  pFromStruct->m_nOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_0=0x%01x", (unsigned int)  pFromStruct->m_nPortState_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_1=0x%01x", (unsigned int)  pFromStruct->m_nPortState_1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_2=0x%01x", (unsigned int)  pFromStruct->m_nPortState_2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_3=0x%01x", (unsigned int)  pFromStruct->m_nPortState_3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_4=0x%01x", (unsigned int)  pFromStruct->m_nPortState_4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_5=0x%01x", (unsigned int)  pFromStruct->m_nPortState_5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_6=0x%01x", (unsigned int)  pFromStruct->m_nPortState_6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_7=0x%01x", (unsigned int)  pFromStruct->m_nPortState_7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_8=0x%01x", (unsigned int)  pFromStruct->m_nPortState_8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_9=0x%01x", (unsigned int)  pFromStruct->m_nPortState_9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_10=0x%01x", (unsigned int)  pFromStruct->m_nPortState_10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_11=0x%01x", (unsigned int)  pFromStruct->m_nPortState_11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_12=0x%01x", (unsigned int)  pFromStruct->m_nPortState_12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_13=0x%01x", (unsigned int)  pFromStruct->m_nPortState_13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_14=0x%01x", (unsigned int)  pFromStruct->m_nPortState_14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_15=0x%01x", (unsigned int)  pFromStruct->m_nPortState_15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_16=0x%01x", (unsigned int)  pFromStruct->m_nPortState_16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_17=0x%01x", (unsigned int)  pFromStruct->m_nPortState_17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_18=0x%01x", (unsigned int)  pFromStruct->m_nPortState_18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_19=0x%01x", (unsigned int)  pFromStruct->m_nPortState_19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_20=0x%01x", (unsigned int)  pFromStruct->m_nPortState_20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_21=0x%01x", (unsigned int)  pFromStruct->m_nPortState_21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_22=0x%01x", (unsigned int)  pFromStruct->m_nPortState_22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_23=0x%01x", (unsigned int)  pFromStruct->m_nPortState_23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_24=0x%01x", (unsigned int)  pFromStruct->m_nPortState_24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_25=0x%01x", (unsigned int)  pFromStruct->m_nPortState_25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_26=0x%01x", (unsigned int)  pFromStruct->m_nPortState_26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_27=0x%01x", (unsigned int)  pFromStruct->m_nPortState_27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_28=0x%01x", (unsigned int)  pFromStruct->m_nPortState_28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_29=0x%01x", (unsigned int)  pFromStruct->m_nPortState_29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_30=0x%01x", (unsigned int)  pFromStruct->m_nPortState_30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_31=0x%01x", (unsigned int)  pFromStruct->m_nPortState_31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_32=0x%01x", (unsigned int)  pFromStruct->m_nPortState_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_33=0x%01x", (unsigned int)  pFromStruct->m_nPortState_33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_34=0x%01x", (unsigned int)  pFromStruct->m_nPortState_34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_35=0x%01x", (unsigned int)  pFromStruct->m_nPortState_35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_36=0x%01x", (unsigned int)  pFromStruct->m_nPortState_36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_37=0x%01x", (unsigned int)  pFromStruct->m_nPortState_37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_38=0x%01x", (unsigned int)  pFromStruct->m_nPortState_38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_39=0x%01x", (unsigned int)  pFromStruct->m_nPortState_39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_40=0x%01x", (unsigned int)  pFromStruct->m_nPortState_40);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_41=0x%01x", (unsigned int)  pFromStruct->m_nPortState_41);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_42=0x%01x", (unsigned int)  pFromStruct->m_nPortState_42);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_43=0x%01x", (unsigned int)  pFromStruct->m_nPortState_43);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_44=0x%01x", (unsigned int)  pFromStruct->m_nPortState_44);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_45=0x%01x", (unsigned int)  pFromStruct->m_nPortState_45);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_46=0x%01x", (unsigned int)  pFromStruct->m_nPortState_46);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_47=0x%01x", (unsigned int)  pFromStruct->m_nPortState_47);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port_state_48=0x%01x", (unsigned int)  pFromStruct->m_nPortState_48);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibVRT:: port_state_49=0x%01x", (unsigned int)  pFromStruct->m_nPortState_49);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibVRT_Validate(sbZfSbQe2000ElibVRT_t *pZf) {

  if (pZf->m_nVid0 > 0xfff) return 0;
  if (pZf->m_nVid1 > 0xfff) return 0;
  if (pZf->m_nOffset > 0x1f) return 0;
  if (pZf->m_nPortState_0 > 0x3) return 0;
  if (pZf->m_nPortState_1 > 0x3) return 0;
  if (pZf->m_nPortState_2 > 0x3) return 0;
  if (pZf->m_nPortState_3 > 0x3) return 0;
  if (pZf->m_nPortState_4 > 0x3) return 0;
  if (pZf->m_nPortState_5 > 0x3) return 0;
  if (pZf->m_nPortState_6 > 0x3) return 0;
  if (pZf->m_nPortState_7 > 0x3) return 0;
  if (pZf->m_nPortState_8 > 0x3) return 0;
  if (pZf->m_nPortState_9 > 0x3) return 0;
  if (pZf->m_nPortState_10 > 0x3) return 0;
  if (pZf->m_nPortState_11 > 0x3) return 0;
  if (pZf->m_nPortState_12 > 0x3) return 0;
  if (pZf->m_nPortState_13 > 0x3) return 0;
  if (pZf->m_nPortState_14 > 0x3) return 0;
  if (pZf->m_nPortState_15 > 0x3) return 0;
  if (pZf->m_nPortState_16 > 0x3) return 0;
  if (pZf->m_nPortState_17 > 0x3) return 0;
  if (pZf->m_nPortState_18 > 0x3) return 0;
  if (pZf->m_nPortState_19 > 0x3) return 0;
  if (pZf->m_nPortState_20 > 0x3) return 0;
  if (pZf->m_nPortState_21 > 0x3) return 0;
  if (pZf->m_nPortState_22 > 0x3) return 0;
  if (pZf->m_nPortState_23 > 0x3) return 0;
  if (pZf->m_nPortState_24 > 0x3) return 0;
  if (pZf->m_nPortState_25 > 0x3) return 0;
  if (pZf->m_nPortState_26 > 0x3) return 0;
  if (pZf->m_nPortState_27 > 0x3) return 0;
  if (pZf->m_nPortState_28 > 0x3) return 0;
  if (pZf->m_nPortState_29 > 0x3) return 0;
  if (pZf->m_nPortState_30 > 0x3) return 0;
  if (pZf->m_nPortState_31 > 0x3) return 0;
  if (pZf->m_nPortState_32 > 0x3) return 0;
  if (pZf->m_nPortState_33 > 0x3) return 0;
  if (pZf->m_nPortState_34 > 0x3) return 0;
  if (pZf->m_nPortState_35 > 0x3) return 0;
  if (pZf->m_nPortState_36 > 0x3) return 0;
  if (pZf->m_nPortState_37 > 0x3) return 0;
  if (pZf->m_nPortState_38 > 0x3) return 0;
  if (pZf->m_nPortState_39 > 0x3) return 0;
  if (pZf->m_nPortState_40 > 0x3) return 0;
  if (pZf->m_nPortState_41 > 0x3) return 0;
  if (pZf->m_nPortState_42 > 0x3) return 0;
  if (pZf->m_nPortState_43 > 0x3) return 0;
  if (pZf->m_nPortState_44 > 0x3) return 0;
  if (pZf->m_nPortState_45 > 0x3) return 0;
  if (pZf->m_nPortState_46 > 0x3) return 0;
  if (pZf->m_nPortState_47 > 0x3) return 0;
  if (pZf->m_nPortState_48 > 0x3) return 0;
  if (pZf->m_nPortState_49 > 0x3) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibVRT_SetField(sbZfSbQe2000ElibVRT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nvid0") == 0) {
    s->m_nVid0 = value;
  } else if (SB_STRCMP(name, "m_nvid1") == 0) {
    s->m_nVid1 = value;
  } else if (SB_STRCMP(name, "m_noffset") == 0) {
    s->m_nOffset = value;
  } else if (SB_STRCMP(name, "m_nportstate_0") == 0) {
    s->m_nPortState_0 = value;
  } else if (SB_STRCMP(name, "m_nportstate_1") == 0) {
    s->m_nPortState_1 = value;
  } else if (SB_STRCMP(name, "m_nportstate_2") == 0) {
    s->m_nPortState_2 = value;
  } else if (SB_STRCMP(name, "m_nportstate_3") == 0) {
    s->m_nPortState_3 = value;
  } else if (SB_STRCMP(name, "m_nportstate_4") == 0) {
    s->m_nPortState_4 = value;
  } else if (SB_STRCMP(name, "m_nportstate_5") == 0) {
    s->m_nPortState_5 = value;
  } else if (SB_STRCMP(name, "m_nportstate_6") == 0) {
    s->m_nPortState_6 = value;
  } else if (SB_STRCMP(name, "m_nportstate_7") == 0) {
    s->m_nPortState_7 = value;
  } else if (SB_STRCMP(name, "m_nportstate_8") == 0) {
    s->m_nPortState_8 = value;
  } else if (SB_STRCMP(name, "m_nportstate_9") == 0) {
    s->m_nPortState_9 = value;
  } else if (SB_STRCMP(name, "m_nportstate_10") == 0) {
    s->m_nPortState_10 = value;
  } else if (SB_STRCMP(name, "m_nportstate_11") == 0) {
    s->m_nPortState_11 = value;
  } else if (SB_STRCMP(name, "m_nportstate_12") == 0) {
    s->m_nPortState_12 = value;
  } else if (SB_STRCMP(name, "m_nportstate_13") == 0) {
    s->m_nPortState_13 = value;
  } else if (SB_STRCMP(name, "m_nportstate_14") == 0) {
    s->m_nPortState_14 = value;
  } else if (SB_STRCMP(name, "m_nportstate_15") == 0) {
    s->m_nPortState_15 = value;
  } else if (SB_STRCMP(name, "m_nportstate_16") == 0) {
    s->m_nPortState_16 = value;
  } else if (SB_STRCMP(name, "m_nportstate_17") == 0) {
    s->m_nPortState_17 = value;
  } else if (SB_STRCMP(name, "m_nportstate_18") == 0) {
    s->m_nPortState_18 = value;
  } else if (SB_STRCMP(name, "m_nportstate_19") == 0) {
    s->m_nPortState_19 = value;
  } else if (SB_STRCMP(name, "m_nportstate_20") == 0) {
    s->m_nPortState_20 = value;
  } else if (SB_STRCMP(name, "m_nportstate_21") == 0) {
    s->m_nPortState_21 = value;
  } else if (SB_STRCMP(name, "m_nportstate_22") == 0) {
    s->m_nPortState_22 = value;
  } else if (SB_STRCMP(name, "m_nportstate_23") == 0) {
    s->m_nPortState_23 = value;
  } else if (SB_STRCMP(name, "m_nportstate_24") == 0) {
    s->m_nPortState_24 = value;
  } else if (SB_STRCMP(name, "m_nportstate_25") == 0) {
    s->m_nPortState_25 = value;
  } else if (SB_STRCMP(name, "m_nportstate_26") == 0) {
    s->m_nPortState_26 = value;
  } else if (SB_STRCMP(name, "m_nportstate_27") == 0) {
    s->m_nPortState_27 = value;
  } else if (SB_STRCMP(name, "m_nportstate_28") == 0) {
    s->m_nPortState_28 = value;
  } else if (SB_STRCMP(name, "m_nportstate_29") == 0) {
    s->m_nPortState_29 = value;
  } else if (SB_STRCMP(name, "m_nportstate_30") == 0) {
    s->m_nPortState_30 = value;
  } else if (SB_STRCMP(name, "m_nportstate_31") == 0) {
    s->m_nPortState_31 = value;
  } else if (SB_STRCMP(name, "m_nportstate_32") == 0) {
    s->m_nPortState_32 = value;
  } else if (SB_STRCMP(name, "m_nportstate_33") == 0) {
    s->m_nPortState_33 = value;
  } else if (SB_STRCMP(name, "m_nportstate_34") == 0) {
    s->m_nPortState_34 = value;
  } else if (SB_STRCMP(name, "m_nportstate_35") == 0) {
    s->m_nPortState_35 = value;
  } else if (SB_STRCMP(name, "m_nportstate_36") == 0) {
    s->m_nPortState_36 = value;
  } else if (SB_STRCMP(name, "m_nportstate_37") == 0) {
    s->m_nPortState_37 = value;
  } else if (SB_STRCMP(name, "m_nportstate_38") == 0) {
    s->m_nPortState_38 = value;
  } else if (SB_STRCMP(name, "m_nportstate_39") == 0) {
    s->m_nPortState_39 = value;
  } else if (SB_STRCMP(name, "m_nportstate_40") == 0) {
    s->m_nPortState_40 = value;
  } else if (SB_STRCMP(name, "m_nportstate_41") == 0) {
    s->m_nPortState_41 = value;
  } else if (SB_STRCMP(name, "m_nportstate_42") == 0) {
    s->m_nPortState_42 = value;
  } else if (SB_STRCMP(name, "m_nportstate_43") == 0) {
    s->m_nPortState_43 = value;
  } else if (SB_STRCMP(name, "m_nportstate_44") == 0) {
    s->m_nPortState_44 = value;
  } else if (SB_STRCMP(name, "m_nportstate_45") == 0) {
    s->m_nPortState_45 = value;
  } else if (SB_STRCMP(name, "m_nportstate_46") == 0) {
    s->m_nPortState_46 = value;
  } else if (SB_STRCMP(name, "m_nportstate_47") == 0) {
    s->m_nPortState_47 = value;
  } else if (SB_STRCMP(name, "m_nportstate_48") == 0) {
    s->m_nPortState_48 = value;
  } else if (SB_STRCMP(name, "m_nportstate_49") == 0) {
    s->m_nPortState_49 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
