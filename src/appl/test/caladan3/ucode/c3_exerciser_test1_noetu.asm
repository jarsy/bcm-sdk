;
; $Copyright: (c) 2016 Broadcom.
; Broadcom Proprietary and Confidential. All rights reserved.$
; $Id: idCheck.py,v 1.12 Broadcom SDK $

        #define dDM0_BIT_RANGE                11:0 
        #define dDM1_BIT_RANGE                23:12
        #define dDM2_BIT_RANGE                31:20
        #define dDM3_BIT_RANGE                11:0
        #define dEMC_SK0_BIT_RANGE            15:0 
        #define dEMC_SK1_BIT_RANGE            31:16 
        #define dEML_SK0_BIT_RANGE            15:0 
        #define dEML_SK1_BIT_RANGE            31:16 
        #define dETU_BIT_RANGE                31:16 
        #define dLRP_PORT_ACCESS_BIT_RANGE    11:0 
        #define dIPV4_SK0_UPPER_BIT_RANGE     18:8    ; 15 moved to 18 for TAPs TCAM population
        #define dIPV4_SK1_UPPER_BIT_RANGE     26:16   ; 23 moved to 26 for TAPs TCAM population
        #define dIPV6_SK0_UPPER_BIT_RANGE     16:8    ; 15 moved to 16 for TAPs TCAM population
        #define dIPV6_SK1_UPPER_BIT_RANGE     24:16   ; 23 moved to 24 for TAPs TCAM population
        #define dDREQ_PROBABILITY              1:0 

        const TSR_ERROR_FLAG       = 0x80000000 
        const RCE_MISCOMPARE_FLAG  = 0x40000000 
        const EML_MISCOMPARE_FLAG  = 0x20000000 
        const EMC_MISCOMPARE_FLAG  = 0x10000000 
        const DM_MISCOMPARE_FLAG   = 0x08000000 
        const PORT_MISCOMPARE_FLAG = 0x04000000 
        const ETU_MISCOMPARE_FLAG  = 0x02000000
        const IPV4_MISCOMPARE_FLAG  = 0x01000000
        const IPV6_MISCOMPARE_FLAG  = 0x00800000
        const COHTAB_WRITE = 0x1000 
        const COHTAB_READ  = 0x0000
        const COHTAB_COUNT = 0x1800
        const EPOCH_COUNTER_OFFSET = 0
        const WDT_RESTART_CONTROL = 0xa0000000
        const WDT_LIST_BASE = 0x00000000
        const LEARNING_LIST_BASE = 0x00007fff
        const WDT_PORT_LRP_LOAD_ACCESS_BASE = 0x0000c000
        const WDT_PORT_LRP_STORE_ACCESS_BASE = 0x0000d000
        const MAX_KEYS = 0x00010000
        const PACKET_SIZE = 64
        const DEQUEUE = 1
        const DESC__SQUEUE_OFFSET = 35
        const DESC__FRAME_LEN_OFFSET = 36
        const DESC__BUFFERandSRCQ_OFFSET = 32
        const DESC__SRC_TYPE_OFFSET = 40
        const SQUEUE_NUM = 128
        const INGRESS_SQUEUE_NUM = 64
        const DESC__WORD12_OFFSET = 48
        const DESC__WORD13_OFFSET = 52
        const DESC__WORD14_OFFSET = 56
        const DESC__PKT_WORD0_OFFSET = 64
        const DESC__PKT_WORD1_OFFSET = 68
        const DESC__PKT_WORD2_OFFSET = 72
        const DESC__PKT_WORD3_OFFSET = 76
        const DESC__PKT_WORD4_OFFSET = 80
        const DESC__PKT_WORD5_OFFSET = 84
        const DESC__PKT_WORD6_OFFSET = 88
        const DESC__PKT_WORD7_OFFSET = 92
        const DESC__PKT_WORD8_OFFSET = 96
        const DESC__PKT_WORD9_OFFSET = 100
        const DESC__PKT_WORD10_OFFSET = 104
        const DESC__PKT_WORD11_OFFSET = 108

        reg   dcr = tsr[27:24]
        reg   tsr_SrcType = tsr[22:21]
        reg   tsr_Drop = tsr[19]
        reg   tsr_Unload = tsr[17]
        reg   tsr_ErrorFlag = tsr[10]
        reg   tsr_F0Flag = tsr[0]
        reg   tsr_F1Flag = tsr[1]
        reg   tsr_F2Flag = tsr[2]
        reg   tsr_F3Flag = tsr[3]
        reg   tsr_F4Flag = tsr[4]
        reg   tsr_F5Flag = tsr[5]
        reg   tsr_F6Flag = tsr[6]
        reg   tsr_F7Flag = tsr[7]
        reg   tsr_F8Flag = tsr[8]
        reg   tsr_F9Flag = tsr[9]
        reg   DontCare = tt0
        reg   DescByte = t1[7:0]
        reg   SrcType = t1[1:0]
        reg   FlowID = pir[9:0]
        reg   PEID_W_BUDDY = pir[6:0]
        reg   DM0_EntryIndex = r4 
        reg   DM1_EntryIndex = r5 
        reg   DM2_EntryIndex = r6 
        reg   DM3_EntryIndex = r7 
        reg   CountValue = tt29
        reg   CountValue_lo = tt29[31:0]
        reg   EpochCounterOffset = t60
        reg   SrcQField = t60[7:0]
        reg   SrcQ = t60[6:0]
        reg   PacketLengthCounterOffset = t60
        reg   FrameLengthField = tt29[15:0]
        reg   PacketLength = tt29[15:0]
        reg   WDT_Value = tt29
        reg   WDT_Value_lo = tt29[31:0]
        reg   WDT_Value_hi = tt29[63:32]
        reg   WDTlist_RingPointer = r62 
        reg   LearningList_RingPointer = r62
        reg   LearningList_Offset = r62
        reg   TMU_InsertCredit = r0
        reg   BitBucket0 = tt31 
        reg   MisCompare_Flag = t61[0]
        reg   SK0_TotalCounterID = r58;
        reg   SK1_TotalCounterID = r59;
        reg   SK0_MissCounterID = r60;
        reg   SK1_MissCounterID = r61;
        reg   SK0_MatchCounterID = r58;
        reg   SK1_MatchCounterID = r59;
        reg   ErrorMessage = r21
        reg   ErrorMessage_SK1ErrCode = r21[25:24]
        reg   ErrorMessage_SK0ErrCode = r21[21:20]
        reg   SK1ErrCode = c19[24:23]
        reg   SK0ErrCode = c23[24:23]
        reg   DescData = rr28
        reg   DescData_1 = rr28[63:32]
        reg   DescData_0 = rr28[31:0]
        reg   ttDescData0 = tt28
        reg   ttDescData0_1 = tt28[63:32]
        reg   ttDescData0_0 = tt28[31:0]
        reg   ttDescData1 = tt27
        reg   ttDescData1_1 = tt27[63:32]
        reg   ttDescData1_0 = tt27[31:0]
        reg   ttDescData2 = tt20
        reg   ttDescData2_1 = tt20[63:32]
        reg   ttDescData2_0 = tt20[31:0]
        reg   P2_EntryIndex = r12 
        reg   P1_EntryIndex = r13 
        reg   P0_EntryIndex = r14 
        reg   P0_EntryIndex_11_0 = r14[11:0] 
        reg   P3_EntryIndex = r15 
        reg   P4_EntryIndex = r16 
        reg   P5_EntryIndex = r17 
        reg   P6_EntryIndex = r18 
        reg   P7_EntryIndex = r19 
        reg   P8_EntryIndex = r20 
        reg   P2_ActualResults = rr11
        reg   P2_ActualResults_0 = r22
        reg   P2_ActualResults_1 = r23
        reg   P1_ActualResults = rr12
        reg   P1_ActualResults_0 = r24
        reg   P1_ActualResults_1 = r25
        reg   P0_ActualResults = rr13
        reg   P0_ActualResults_0 = r26
        reg   P0_ActualResults_1 = r27
        reg   P0_WatchDogTimerControlWord = rr13
        reg   P0_WatchDogTimerControlWord_lo = r26
        reg   P0_WatchDogTimerControlWord_hi = r27
        reg   P0_WDTctrlword_Valid = rr13[63]
        reg   P0_WDTctrlword_CopPort = rr13[62]
        reg   P0_WDTctrlword_Offset = r26
        reg   COP_LearningListEntry = rr0
        reg   COP_LearningListEntry_Key = rr0[15:0]
        reg   P3_ActualResults = rr14
        reg   P3_ActualResults_0 = r28
        reg   P3_ActualResults_1 = r29
        reg   P4_ActualResults = rr15
        reg   P4_ActualResults_0 = r30
        reg   P4_ActualResults_1 = r31
        reg   P5_ActualResults = rr16
        reg   P5_ActualResults_0 = r32
        reg   P5_ActualResults_1 = r33
        reg   P6_ActualResults = rr17
        reg   P6_ActualResults_0 = r34
        reg   P6_ActualResults_1 = r35
        reg   P7_ActualResults_0 = r36
        reg   P7_ActualResults = rr18
        reg   P7_ActualResults_1 = r37
        reg   P8_ActualResults = rr19
        reg   P8_ActualResults_0 = r38
        reg   P8_ActualResults_1 = r39
        reg   P2_ExpectResults = tt11
        reg   P2_ExpectResults_0 = t22
        reg   P2_ExpectResults_1 = t23
        reg   P1_ExpectResults = tt12
        reg   P1_ExpectResults_0 = t24
        reg   P1_ExpectResults_1 = t25
        reg   P0_ExpectResults = tt13
        reg   P0_ExpectResults_0 = t26
        reg   P0_ExpectResults_1 = t27
        reg   P3_ExpectResults = tt14
        reg   P3_ExpectResults_0 = t28
        reg   P3_ExpectResults_1 = t29
        reg   P4_ExpectResults = tt15
        reg   P4_ExpectResults_0 = t30
        reg   P4_ExpectResults_1 = t31
        reg   P5_ExpectResults = tt16
        reg   P5_ExpectResults_0 = t32
        reg   P5_ExpectResults_1 = t33
        reg   P6_ExpectResults = tt17
        reg   P6_ExpectResults_0 = t34
        reg   P6_ExpectResults_1 = t35
        reg   P7_ExpectResults_0 = t36
        reg   P7_ExpectResults = tt18
        reg   P7_ExpectResults_1 = t37
        reg   P8_ExpectResults = tt19
        reg   P8_ExpectResults_0 = t38
        reg   P8_ExpectResults_1 = t39

        reg   DM_ActualResults_0 = c12
        reg   DM_ActualResults_1 = c13
        reg   DM_ActualResults_2 = c14
        reg   DM_ActualResults_3 = c15
        reg   DM_ActualResults_4 = c8
        reg   DM_ActualResults_5 = c9
        reg   DM_ActualResults_6 = c10
        reg   DM_ActualResults_7 = c11
        reg   DM_ActualResults_8 = c4
        reg   DM_ActualResults_9 = c5
        reg   DM_ActualResults_10 = c6
        reg   DM_ActualResults_11 = c7
        reg   DM_ActualResults_12 = c0
        reg   DM_ActualResults_13 = c1
        reg   DM_ActualResults_14 = c2
        reg   DM_ActualResults_15 = c3
        reg   DM_ExpectResults_0 = t12
        reg   DM_ExpectResults_1 = t13
        reg   DM_ExpectResults_2 = t14
        reg   DM_ExpectResults_3 = t15
        reg   DM_ExpectResults_4 = t8
        reg   DM_ExpectResults_5 = t9
        reg   DM_ExpectResults_6 = t10
        reg   DM_ExpectResults_7 = t11
        reg   DM_ExpectResults_8 = t4
        reg   DM_ExpectResults_9 = t5
        reg   DM_ExpectResults_10 = t6
        reg   DM_ExpectResults_11 = t7
        reg   DM_ExpectResults_12 = t0
        reg   DM_ExpectResults_13 = t1
        reg   DM_ExpectResults_14 = t2
        reg   DM_ExpectResults_15 = t3

        reg   EML_SK0_ExpectResults_0 = t20
        reg   EML_SK0_ExpectResults_1 = t21
        reg   EML_SK0_ExpectResults_2 = t22
        reg   EML_SK0_ExpectResults_3 = t23
        reg   EML_SK0_ActualResults_0 = c20
        reg   EML_SK0_ActualResults_1 = c21
        reg   EML_SK0_ActualResults_2 = c22
        reg   EML_SK0_ActualResults_3 = c23
        reg   EML_SK1_ExpectResults_0 = t16
        reg   EML_SK1_ExpectResults_1 = t17
        reg   EML_SK1_ExpectResults_2 = t18
        reg   EML_SK1_ExpectResults_3 = t19
        reg   EML_SK1_ActualResults_0 = c16
        reg   EML_SK1_ActualResults_1 = c17
        reg   EML_SK1_ActualResults_2 = c18
        reg   EML_SK1_ActualResults_3 = c19

        reg   IPV4_SK0_ExpectResults_0 = t20
        reg   IPV4_SK0_ExpectResults_1 = t21
        reg   IPV4_SK0_ExpectResults_2 = t22
        reg   IPV4_SK0_ExpectResults_3 = t23
        reg   IPV4_SK0_ActualResults_0 = c20
        reg   IPV4_SK0_ActualResults_1 = c21
        reg   IPV4_SK0_ActualResults_2 = c22
        reg   IPV4_SK0_ActualResults_3 = c23
        reg   IPV4_SK1_ExpectResults_0 = t16
        reg   IPV4_SK1_ExpectResults_1 = t17
        reg   IPV4_SK1_ExpectResults_2 = t18
        reg   IPV4_SK1_ExpectResults_3 = t19
        reg   IPV4_SK1_ActualResults_0 = c16
        reg   IPV4_SK1_ActualResults_1 = c17
        reg   IPV4_SK1_ActualResults_2 = c18
        reg   IPV4_SK1_ActualResults_3 = c19

        reg   IPV6_SK0_ExpectResults_0 = t20
        reg   IPV6_SK0_ExpectResults_1 = t21
        reg   IPV6_SK0_ExpectResults_2 = t22
        reg   IPV6_SK0_ExpectResults_3 = t23
        reg   IPV6_SK0_ActualResults_0 = c20
        reg   IPV6_SK0_ActualResults_1 = c21
        reg   IPV6_SK0_ActualResults_2 = c22
        reg   IPV6_SK0_ActualResults_3 = c23
        reg   IPV6_SK1_ExpectResults_0 = t16
        reg   IPV6_SK1_ExpectResults_1 = t17
        reg   IPV6_SK1_ExpectResults_2 = t18
        reg   IPV6_SK1_ExpectResults_3 = t19
        reg   IPV6_SK1_ActualResults_0 = c16
        reg   IPV6_SK1_ActualResults_1 = c17
        reg   IPV6_SK1_ActualResults_2 = c18
        reg   IPV6_SK1_ActualResults_3 = c19

        reg   Key_63_0    = rr4
        reg   RCE_Key15   = rr4[14:0] 
        reg   RCE_Key14   = rr4[13:0] 
        reg   ETU_Key     = rr4[15:0]
        reg   EML_SubKey0 = rr4[15:0]
        reg   ETU_SubKey0 = rr4[15:0]
        reg   IPV4_SubKey0_31to24 = rr4[31:24]
        reg   IPV4_SubKey0_43to32 = rr4[43:32]
        reg   IPV6_SubKey0_127to120 = rr27[15:8]
        reg   IPV6_SubKey0_139to128 = rr27[27:16]
        reg   Key_127_64  = rr26
        reg   Key_191_128 = rr27
        reg   Key_255_192 = tt7
        reg   Key_319_256 = rr20
        reg   EMC_SubKey0 = rr20[15:0]
        reg   Key_383_320 = tt9
        reg   Key_447_384 = rr23
        reg   Key_511_448 = rr5
        reg   EML_SubKey1 = rr5[63:48]
        reg   EMC_SubKey1 = rr5[63:48]
        reg   IPV4_SubKey1_31to24 = rr5[31:24]
        reg   IPV4_SubKey1_43to32 = rr5[43:32]
        reg   IPV4_SubKey1_47to40 = rr5[47:40]
        reg   IPV6_SubKey1_127to120 = rr23[63:56]
        reg   IPV6_SubKey1_139to128 = rr5[11:0]
        reg   Key_575_512 = tt11
        reg   Key_639_576 = rr1

        reg   EMC_SK0_ExpectResults_0 = t20
        reg   EMC_SK0_ExpectResults_1 = t21
        reg   EMC_SK0_ExpectResults_2 = t22
        reg   EMC_SK0_ExpectResults_3 = t23
        reg   EMC_SK0_ActualResults_0 = c20
        reg   EMC_SK0_ActualResults_1 = c21
        reg   EMC_SK0_ActualResults_2 = c22
        reg   EMC_SK0_ActualResults_3 = c23
        reg   EMC_SK1_ExpectResults_0 = t16
        reg   EMC_SK1_ExpectResults_1 = t17
        reg   EMC_SK1_ExpectResults_2 = t18
        reg   EMC_SK1_ExpectResults_3 = t19
        reg   EMC_SK1_ActualResults_0 = c16
        reg   EMC_SK1_ActualResults_1 = c17
        reg   EMC_SK1_ActualResults_2 = c18
        reg   EMC_SK1_ActualResults_3 = c19

        reg   ETU_ActualResults_0 = c27
        reg   ETU_ActualResults_1 = c26
        reg   ETU_ActualResults_2 = c25
        reg   ETU_ActualResults_3 = c24
        reg   ETU_ExpectResults_0 = r45
        reg   ETU_ExpectResults_1 = r44
        reg   ETU_ExpectResults_2 = r43
        reg   ETU_ExpectResults_3 = r42

        reg   RCE_ExpectedResults_1_0 = rr24
        reg   RCE_ExpectedResults_0   = rr24[19:0]
        reg   RCE_ExpectedResults_1   = rr24[51:32]
        reg   RCE_ExpectedResults_3_2 = rr25
        reg   RCE_ExpectedResults_2   = rr25[19:0]
        reg   RCE_ExpectedResults_3   = rr25[51:32]
        reg   RCE_ActualResults_0     = c28[19:0]
        reg   RCE_ActualResults_1     = c29[19:0]
        reg   RCE_ActualResults_2     = c30[19:0]
        reg   RCE_ActualResults_3     = c31[19:0]



stream 0:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00000000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE] 
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 0.   nop
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 0.   nop
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load WDTlist_RingPointer, p0.204( DEQUEUE )
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 0.3, Key_639_576
         > 1.   key 0.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 0.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 0.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.128(DM0_EntryIndex)
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   mov P7_ActualResults_0, 0
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 7.   switch 2
   > (!E)? 1.   j STREAM0__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM0__TSR_ERROR_s0
STREAM0__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11100000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22200000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33300000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x44400000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x55500000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x66600000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x77700000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00700000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x99900000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0xaaa00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0xbbb00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0xccc00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0xddd00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0xeee00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0xfff00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00700000, DM0_EntryIndex 
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 0.   nop
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM0__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   jne STREAM0__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM0__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM0__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM0__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM0__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM0__DO_SK1_RESULT_CHECK_s0
STREAM0__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM0__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM0__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM0__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM0__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM0__DO_SK1_RESULT_CHECK_s0
STREAM0__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM0__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM0__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM0__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM0__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM0__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM0__EML_SUMMARIZE_RESULTS_s0
STREAM0__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM0__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM0__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM0__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM0__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM0__EML_SUMMARIZE_RESULTS_s0
STREAM0__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM0__EML_SUMMARIZE_RESULTS_s0:
         > 1.   jz STREAM0__DO_ETU_COMPARES_s0, MisCompare_Flag
         > 1.   j STREAM0__EML_MISCOMPARE_s0
         > 0.   nop
STREAM0__DO_ETU_COMPARES_s0:
         > 1.   jeq STREAM0__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM0__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM0__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM0__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
         > 1.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         > 0.   nop
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   hload DescData, h(DESC__WORD12_OFFSET):8
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   teq tsr_F8Flag, P2_ExpectResults_1, DescData_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   j STREAM0__s1

STREAM0__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM0__SEND_ERROR_MESSAGE_s0
STREAM0__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM0__SEND_ERROR_MESSAGE_s0
STREAM0__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM0__SEND_ERROR_MESSAGE_s0
STREAM0__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM0__SEND_ERROR_MESSAGE_s0
STREAM0__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM0__SEND_ERROR_MESSAGE_s0
STREAM0__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM0__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM0__THE_END

STREAM0__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00001000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F6Flag, 1, WDTlist_RingPointer
   > (F6)? 1.   or  P0_EntryIndex, WDT_LIST_BASE, WDTlist_RingPointer
         < 2.   mov  EMC_SubKey0, rnr[dEMC_SK0_BIT_RANGE]
         < 2.   mov  EMC_SubKey1, rnr[dEMC_SK1_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   mov ttDescData0_1, 0xaaaaaaaa
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov ttDescData0_0, 0x55555555
         < 2.   hstore h(DESC__WORD12_OFFSET):8, ttDescData0
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, DescData_1
         < 0.   nop
         < 0.   nop
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
   > (F6)? 1.   load P0_WatchDogTimerControlWord, p0.6( P0_EntryIndex )
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 3.3, Key_639_576
         < 2.   key 3.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 3.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 3.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM0__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM0__TSR_ERROR_s1
STREAM0__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  EMC_SK0_ExpectResults_0, 0x888c0000, EMC_SubKey0
         < 2.   or  EMC_SK0_ExpectResults_1, 0x007c0000, EMC_SubKey0

         < 2.   mov EMC_SK0_ExpectResults_2, 0x00000000
         < 2.   mov EMC_SK0_ExpectResults_3, 0x00000000
         < 2.   or  EMC_SK1_ExpectResults_0, 0x888c0000, EMC_SubKey1
         < 2.   or  EMC_SK1_ExpectResults_1, 0x007c0000, EMC_SubKey1
         < 2.   mov EMC_SK1_ExpectResults_2, 0x00000000
         < 2.   mov EMC_SK1_ExpectResults_3, 0x00000000
   > (F6)? 1.   mov tsr_F7Flag, P0_WDTctrlword_CopPort
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM0__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM0__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_0, EMC_SK0_ActualResults_0
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_1, EMC_SK0_ActualResults_1
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_2, EMC_SK0_ActualResults_2
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_3, EMC_SK0_ActualResults_3
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_0, EMC_SK1_ActualResults_0
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_1, EMC_SK1_ActualResults_1

         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_2, EMC_SK1_ActualResults_2
         < 2.   jne STREAM0__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_3, EMC_SK1_ActualResults_3
         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM0__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM0__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM0__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM0__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   teq tsr_F8Flag, 0x55555555, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   xor DescData_0, 0xffffffff, DescData_0
         < 2.   xor DescData_1, 0xffffffff, DescData_1
         < 2.   hstore h(DESC__WORD12_OFFSET):8, DescData
         < 2.   hstore h(DESC__WORD14_OFFSET):4, DescData_1

         < 2.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   xor DescData_0, 0xffffffff, ttDescData1_0
         < 2.   xor DescData_1, 0xffffffff, ttDescData1_1
         < 2.   xor ttDescData0_0, 0xffffffff, ttDescData0_1
         < 2.   hstore h(DESC__WORD12_OFFSET):8, DescData
         < 2.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0
         < 2.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   xor DescData_0, 0xffffffff, ttDescData1_0
         < 2.   xor DescData_1, 0xffffffff, ttDescData1_1
         < 2.   xor ttDescData0_0, 0xffffffff, ttDescData0_1
         < 2.   hstore h(DESC__WORD12_OFFSET):8, DescData
         < 2.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
   > (F6 && !F7)? 1. load BitBucket0, cop.33( P0_WDTctrlword_Offset + WDT_Value )
   > (F6 && F7)?  1. load BitBucket0, cop.65( P0_WDTctrlword_Offset + WDT_Value )
         < 2.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData0_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM0__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM0__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM0__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM0__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData1_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM0__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM0__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 0.   nop
         < 2.   j STREAM0__s2

STREAM0__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM0__SEND_ERROR_MESSAGE_s1
STREAM0__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM0__SEND_ERROR_MESSAGE_s1
STREAM0__EMC_MISCOMPARE_s1:
         < 2.   or ErrorMessage, EMC_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM0__SEND_ERROR_MESSAGE_s1
STREAM0__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM0__SEND_ERROR_MESSAGE_s1
STREAM0__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM0__SEND_ERROR_MESSAGE_s1
STREAM0__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM0__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM0__THE_END

STREAM0__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM0__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM0__THE_END
STREAM0__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1


STREAM0__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 1:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00010000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         > 1.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         > 1.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV4_SubKey0_31to24, rnr[7:0]
         > 1.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   mov  RCE_Key15, rnr[30:16]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV4_SubKey1_31to24, rnr[23:16]
         > 1.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         > 1.   mov ttDescData0_1, 0xaaaaaaaa
         > 1.   mov ttDescData0_0, 0x55555555
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   hstore h(DESC__WORD12_OFFSET):8, ttDescData0
         > 0.   nop
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 1.3, Key_639_576
         > 1.   key 1.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 1.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 1.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.129(DM0_EntryIndex)
  > (F9) ? 1.   dreq p5.130(DM1_EntryIndex)
  > (F9) ? 1.   dreq p6.131(DM2_EntryIndex)
         > 1.   hload DescData, h(DESC__WORD12_OFFSET):8
  > (F9) ? 1.   dreq p7.132(DM3_EntryIndex)
         > 7.   switch 2
   > (!E)? 1.   j STREAM1__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM1__TSR_ERROR_s0
STREAM1__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         > 0.   nop
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, DescData_1

         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   jeq STREAM1__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM1__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM1__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM1__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   movifz dcr, 1, tsr_F8Flag
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM1__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   jne STREAM1__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   teq tsr_F8Flag, 0x55555555, DescData_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   xor DescData_0, 0xffffffff, DescData_0
         > 1.   xor DescData_1, 0xffffffff, DescData_1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hstore h(DESC__WORD14_OFFSET):4, DescData_1

         > 1.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         > 1.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24
         > 1.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         > 1.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         > 1.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         > 1.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         > 1.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         > 1.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
         > 1.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         > 1.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         > 1.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         > 1.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         > 1.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         > 1.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         > 1.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         > 1.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32

         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         > 1.   jne STREAM1__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         > 1.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         > 1.   xor DescData_0, 0xffffffff, ttDescData1_0
         > 1.   xor DescData_1, 0xffffffff, ttDescData1_1
         > 1.   xor ttDescData0_0, 0xffffffff, ttDescData0_1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0 
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         > 1.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         > 1.   xor DescData_0, 0xffffffff, ttDescData1_0
         > 1.   xor DescData_1, 0xffffffff, ttDescData1_1
         > 1.   xor ttDescData0_0, 0xffffffff, ttDescData0_1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         > 1.   hload ttDescData0_0, h(DESC__WORD14_OFFSET):4
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData0_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData1_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   j STREAM1__s1

STREAM1__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM1__SEND_ERROR_MESSAGE_s0
STREAM1__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM1__SEND_ERROR_MESSAGE_s0
STREAM1__IPV4_MISCOMPARE_s0:
         > 1.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM1__SEND_ERROR_MESSAGE_s0
STREAM1__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM1__SEND_ERROR_MESSAGE_s0
STREAM1__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM1__SEND_ERROR_MESSAGE_s0
STREAM1__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM1__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM1__THE_END

STREAM1__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00011000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE] 
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   or  P0_EntryIndex, P0_EntryIndex, WDT_PORT_LRP_STORE_ACCESS_BASE
         < 2.   mov IPV4_SubKey0_31to24, rnr[7:0]
         < 2.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   mov DescData_1, 0xaaaaaaaa
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV4_SubKey1_31to24, rnr[23:16]
         < 2.   mov IPV4_SubKey1_47to40, 0
         < 2.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   store p0.6( P0_EntryIndex ), P0_ActualResults
         < 2.   mov DescData_0, 0x55555555
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 4.3, Key_639_576
         < 2.   key 4.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 4.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 4.0, Key_191_128, Key_127_64,  Key_63_0
         < 2.   dreq p4.128(DM0_EntryIndex)
         < 2.   load P5_ActualResults, p5.6( P5_EntryIndex )
         < 2.   load P6_ActualResults, p6.6( P6_EntryIndex )
         < 2.   mov P7_ActualResults_0, 0
         < 2.   load P7_ActualResults, p7.6( P7_EntryIndex )
         < 7.   switch 2
   < (!E)? 2.   j STREAM1__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM1__TSR_ERROR_s1
STREAM1__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11100000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22200000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33300000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x44400000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x55500000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x66600000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x77700000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00700000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x99900000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0xaaa00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0xbbb00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0xccc00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0xddd00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0xeee00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0xfff00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00700000, DM0_EntryIndex 
         < 2.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24

         < 2.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         < 2.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
         > 0.   nop
         > 0.   nop
         > 0.   nop
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
         < 2.   jne STREAM1__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM1__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hstore h(DESC__WORD12_OFFSET):8, DescData
         < 2.   hstore h(DESC__WORD14_OFFSET):4, DescData_0
         < 2.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData1_0
         < 2.   movifz dcr, 1, tsr_F8Flag

         < 2.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         < 2.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         < 2.   jne STREAM1__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3

         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData0_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jeq STREAM1__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM1__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM1__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM1__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         < 2.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         < 2.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         < 2.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         < 2.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         < 2.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         < 2.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         < 0.   nop
         < 0.   nop
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P5_ExpectResults_0,  P5_ActualResults_0
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P5_ExpectResults_1,  P5_ActualResults_1
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P6_ExpectResults_0,  P6_ActualResults_0
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P6_ExpectResults_1,  P6_ActualResults_1
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P7_ExpectResults_0,  P7_ActualResults_0
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P7_ExpectResults_1,  P7_ActualResults_1
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM1__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 0.   nop
         < 2.   j STREAM1__s2

STREAM1__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM1__SEND_ERROR_MESSAGE_s1
STREAM1__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM1__SEND_ERROR_MESSAGE_s1
STREAM1__IPV4_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM1__SEND_ERROR_MESSAGE_s1
STREAM1__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM1__SEND_ERROR_MESSAGE_s1
STREAM1__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM1__SEND_ERROR_MESSAGE_s1
STREAM1__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM1__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM1__THE_END

STREAM1__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM1__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM1__THE_END
STREAM1__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1

STREAM1__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 2:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00020000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV6_SubKey0_127to120, rnr[7:0]
         > 1.   mov IPV6_SubKey0_139to128, rnr[dIPV6_SK0_UPPER_BIT_RANGE]
         > 1.   mov IPV6_SubKey1_127to120, rnr[23:16]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV6_SubKey1_139to128, rnr[dIPV6_SK1_UPPER_BIT_RANGE]
         > 1.   mov DescData_1, 0xaaaaaaaa
         > 1.   mov DescData_0, 0x55555555
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load WDTlist_RingPointer, p0.204( DEQUEUE )
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 13.3, Key_639_576
         > 1.   key 13.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 13.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 13.0, Key_191_128, Key_127_64,  Key_63_0
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
  > (F9) ? 1.   dreq p5.135(DM1_EntryIndex)
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   mov P7_ActualResults_0, 0
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 7.   switch 2
   > (!E)? 1.   j STREAM2__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM2__TSR_ERROR_s0
STREAM2__NO_TSR_ERROR_s0:
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hload ttDescData0, h(DESC__WORD12_OFFSET):8

         > 1.   or  DM_ExpectResults_4,  0x11170000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x22270000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x33370000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x44470000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x55570000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x66670000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x77770000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00770000, DM1_EntryIndex 
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM2__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11

         > 1.   jne STREAM2__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData0_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData0_0 
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   xor DescData_0, 0xffffffff, ttDescData0_0
         > 1.   xor DescData_1, 0xffffffff, ttDescData0_1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hstore h(DESC__WORD14_OFFSET):4, DescData_1
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         > 1.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         > 1.   xor DescData_0, 0xffffffff, ttDescData1_0
         > 1.   xor DescData_1, 0xffffffff, ttDescData1_1
         > 1.   xor ttDescData0_0, 0xffffffff, ttDescData0_1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0

         > 1.   or  IPV6_SK0_ExpectResults_0, 0x11000000, IPV6_SubKey0_127to120
         > 1.   or  IPV6_SK0_ExpectResults_1, 0x22000000, IPV6_SubKey0_127to120
         > 1.   or  IPV6_SK0_ExpectResults_2, 0x33000000, IPV6_SubKey0_127to120
         > 1.   or  IPV6_SK0_ExpectResults_3, 0x00400000, IPV6_SubKey0_127to120
         > 1.   mov IPV6_SK0_ExpectResults_0[19:8], IPV6_SubKey0_139to128
         > 1.   mov IPV6_SK0_ExpectResults_1[19:8], IPV6_SubKey0_139to128
         > 1.   mov IPV6_SK0_ExpectResults_2[19:8], IPV6_SubKey0_139to128
         > 1.   mov IPV6_SK0_ExpectResults_3[19:8], IPV6_SubKey0_139to128
         > 1.   or  IPV6_SK1_ExpectResults_0, 0x11100000, IPV6_SubKey1_127to120
         > 1.   or  IPV6_SK1_ExpectResults_1, 0x22100000, IPV6_SubKey1_127to120
         > 1.   or  IPV6_SK1_ExpectResults_2, 0x33100000, IPV6_SubKey1_127to120
         > 1.   or  IPV6_SK1_ExpectResults_3, 0x00400000, IPV6_SubKey1_127to120
         > 1.   mov IPV6_SK1_ExpectResults_0[19:8], IPV6_SubKey1_139to128
         > 1.   mov IPV6_SK1_ExpectResults_1[19:8], IPV6_SubKey1_139to128
         > 1.   mov IPV6_SK1_ExpectResults_2[19:8], IPV6_SubKey1_139to128
         > 1.   mov IPV6_SK1_ExpectResults_3[19:8], IPV6_SubKey1_139to128

         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_0, IPV6_SK0_ActualResults_0
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_1, IPV6_SK0_ActualResults_1
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_2, IPV6_SK0_ActualResults_2
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_3, IPV6_SK0_ActualResults_3
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_0, IPV6_SK1_ActualResults_0
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_1, IPV6_SK1_ActualResults_1
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_2, IPV6_SK1_ActualResults_2
         > 1.   jne STREAM2__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_3, IPV6_SK1_ActualResults_3
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8 
         > 1.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         > 1.   xor DescData_0, 0xffffffff, ttDescData1_0 
         > 1.   xor DescData_1, 0xffffffff, ttDescData1_1
         > 1.   xor ttDescData0_0, 0xffffffff, ttDescData0_1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8

         > 1.   hload ttDescData0_0, h(DESC__WORD14_OFFSET):4
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData0_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData1_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   jeq STREAM2__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM2__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM2__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM2__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1
 
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P4_ExpectResults_0,  P4_ActualResults_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   j STREAM2__s1

STREAM2__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM2__SEND_ERROR_MESSAGE_s0
STREAM2__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM2__SEND_ERROR_MESSAGE_s0
STREAM2__IPV6_MISCOMPARE_s0:
         > 1.   or ErrorMessage, IPV6_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM2__SEND_ERROR_MESSAGE_s0
STREAM2__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM2__SEND_ERROR_MESSAGE_s0
STREAM2__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM2__SEND_ERROR_MESSAGE_s0
STREAM2__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM2__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM2__THE_END

STREAM2__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00021000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F6Flag, 1, WDTlist_RingPointer
   > (F6)? 1.   or  P0_EntryIndex, WDT_LIST_BASE, WDTlist_RingPointer
         < 2.   mov IPV4_SubKey0_31to24, rnr[7:0]
         < 2.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   mov DescData_1, 0xaaaaaaaa
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV4_SubKey1_31to24, rnr[23:16]
         < 2.   mov IPV4_SubKey1_47to40, 0
         < 2.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         < 2.   mov DescData_0, 0x55555555
         < 2.   hstore h(DESC__WORD12_OFFSET):8, DescData
         < 2.   hstore h(DESC__WORD14_OFFSET):4, DescData_0
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
   > (F6)? 1.   load P0_WatchDogTimerControlWord, p0.6( P0_EntryIndex )
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 7.3, Key_639_576
         < 2.   key 7.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 7.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 7.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM2__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM2__TSR_ERROR_s1
STREAM2__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24

         < 2.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         < 2.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
   > (F6)? 1.   mov tsr_F7Flag, P0_WDTctrlword_CopPort
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM2__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM2__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData1_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData0_1
         < 2.   movifz dcr, 1, tsr_F8Flag

         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM2__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM2__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM2__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM2__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         < 2.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32

         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         < 2.   jne STREAM2__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
   > (F6 && !F7)? 1. load BitBucket0, pmgr.33( P0_WDTctrlword_Offset + WDT_Value )
   > (F6 && F7)?  1. load BitBucket0, pmgr.65( P0_WDTctrlword_Offset + WDT_Value )
         < 2.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, P2_ExpectResults_1, DescData_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   hstore h(DESC__WORD12_OFFSET):8, P8_ActualResults
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM2__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM2__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM2__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM2__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   teq tsr_F8Flag, P8_ExpectResults_0, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, P8_ExpectResults_1, DescData_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM2__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM2__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 0.   nop
         < 2.   j STREAM2__s2

STREAM2__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM2__SEND_ERROR_MESSAGE_s1
STREAM2__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM2__SEND_ERROR_MESSAGE_s1
STREAM2__IPV4_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM2__SEND_ERROR_MESSAGE_s1
STREAM2__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM2__SEND_ERROR_MESSAGE_s1
STREAM2__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM2__SEND_ERROR_MESSAGE_s1
STREAM2__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM2__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM2__THE_END

STREAM2__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM2__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM2__THE_END
STREAM2__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1


STREAM2__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 3:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00030000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM3_EntryIndex, rnr[11:0]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 0.   nop
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 2.3, Key_639_576
         > 1.   key 2.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 2.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 2.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.136(DM0_EntryIndex)
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 0.   nop
  > (F9) ? 1.   dreq p7.132(DM3_EntryIndex)
         > 7.   switch 2
   > (!E)? 1.   j STREAM3__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM3__TSR_ERROR_s0
STREAM3__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11180000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22280000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33380000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x44480000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x55580000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x66680000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x77780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x99980000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0xaaa80000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0xbbb80000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 0.   nop
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM3__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 0.   nop
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM3__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM3__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM3__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM3__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM3__DO_SK1_RESULT_CHECK_s0
STREAM3__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM3__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM3__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM3__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM3__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM3__DO_SK1_RESULT_CHECK_s0
STREAM3__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM3__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM3__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM3__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM3__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM3__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM3__EML_SUMMARIZE_RESULTS_s0
STREAM3__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM3__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM3__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM3__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM3__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM3__EML_SUMMARIZE_RESULTS_s0
STREAM3__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM3__EML_SUMMARIZE_RESULTS_s0:
         > 1.   jnz STREAM3__EML_MISCOMPARE_s0, MisCompare_Flag
         > 0.   nop
         > 1.   jne STREAM3__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   jeq STREAM3__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM3__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM3__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM3__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 1.   hstore h(DESC__WORD12_OFFSET):8, P8_ActualResults
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   hload DescData, h(DESC__WORD12_OFFSET):8
         > 0.   nop
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   teq tsr_F8Flag, P8_ExpectResults_0, DescData_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   teq tsr_F8Flag, P8_ExpectResults_1, DescData_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   jne STREAM3__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   j STREAM3__s1
         > 0.   nop

STREAM3__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM3__SEND_ERROR_MESSAGE_s0
STREAM3__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM3__SEND_ERROR_MESSAGE_s0
STREAM3__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM3__SEND_ERROR_MESSAGE_s0
STREAM3__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM3__SEND_ERROR_MESSAGE_s0
STREAM3__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM3__SEND_ERROR_MESSAGE_s0
STREAM3__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM3__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM3__THE_END

STREAM3__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00031000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV6_SubKey0_127to120, rnr[7:0]
         < 2.   mov IPV6_SubKey0_139to128, rnr[dIPV6_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   mov DescData_1, 0xaaaaaaaa
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV6_SubKey1_127to120, rnr[23:16]
         < 2.   mov IPV6_SubKey1_139to128, rnr[dIPV6_SK1_UPPER_BIT_RANGE]
         < 2.   mov Key_383_320[31:0], 0
         < 2.   mov Key_383_320[63:32], 0
         < 2.   mov Key_63_0[31:16], 0
         < 2.   mov Key_63_0[63:32], 0
         < 2.   mov DescData_0, 0x55555555
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         < 2.   hstore h(DESC__WORD12_OFFSET):8, DescData
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 8.3, Key_639_576
         < 2.   key 8.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 8.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 8.0, Key_191_128, Key_127_64,  Key_63_0
         < 2.   dreq p4.129(DM0_EntryIndex)
         < 2.   dreq p5.130(DM1_EntryIndex)
         < 2.   dreq p6.131(DM2_EntryIndex)
         < 2.   hstore h(DESC__WORD14_OFFSET):4, DescData_0
         < 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM3__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM3__TSR_ERROR_s1
STREAM3__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  IPV6_SK0_ExpectResults_0, 0x11000000, IPV6_SubKey0_127to120
         < 2.   or  IPV6_SK0_ExpectResults_1, 0x22000000, IPV6_SubKey0_127to120

         < 2.   or  IPV6_SK0_ExpectResults_2, 0x33000000, IPV6_SubKey0_127to120
         < 2.   or  IPV6_SK0_ExpectResults_3, 0x00400000, IPV6_SubKey0_127to120
         < 2.   mov IPV6_SK0_ExpectResults_0[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_1[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_2[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_3[19:8], IPV6_SubKey0_139to128
         > 0.   nop
         > 0.   nop
         > 0.   nop
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
         < 2.   jne STREAM3__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM3__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData1_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData0_1
         < 2.   movifz dcr, 1, tsr_F8Flag

         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM3__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM3__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM3__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM3__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  IPV6_SK1_ExpectResults_0, 0x11100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_1, 0x22100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_2, 0x33100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_3, 0x00400000, IPV6_SubKey1_127to120
         < 2.   mov IPV6_SK1_ExpectResults_0[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_1[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_2[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_3[19:8], IPV6_SubKey1_139to128

         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_0, IPV6_SK0_ActualResults_0
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_1, IPV6_SK0_ActualResults_1
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_2, IPV6_SK0_ActualResults_2
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_3, IPV6_SK0_ActualResults_3
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_0, IPV6_SK1_ActualResults_0
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_1, IPV6_SK1_ActualResults_1
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_2, IPV6_SK1_ActualResults_2
         < 2.   jne STREAM3__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_3, IPV6_SK1_ActualResults_3
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         < 2.   hstore h(DESC__WORD14_OFFSET):4, P8_ActualResults_1
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData1_1, h(DESC__WORD14_OFFSET):4
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         < 2.   teq tsr_F8Flag, P2_ExpectResults_1, DescData_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 0.   nop
         < 0.   nop
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM3__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM3__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM3__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM3__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         < 0.   nop
         < 2.   teq tsr_F8Flag, P8_ExpectResults_1, ttDescData1_1 
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   jne STREAM3__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM3__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   j STREAM3__s2

STREAM3__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM3__SEND_ERROR_MESSAGE_s1
STREAM3__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM3__SEND_ERROR_MESSAGE_s1
STREAM3__IPV6_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV6_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM3__SEND_ERROR_MESSAGE_s1
STREAM3__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM3__SEND_ERROR_MESSAGE_s1
STREAM3__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM3__SEND_ERROR_MESSAGE_s1
STREAM3__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM3__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM3__THE_END

STREAM3__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM3__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM3__THE_END
STREAM3__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1

STREAM3__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop




stream 4:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00040000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE] 
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov ttDescData0_1, 0xaaaaaaaa
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   mov ttDescData0_0, 0x55555555
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 1.   hstore h(DESC__WORD12_OFFSET):8, ttDescData0
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load WDTlist_RingPointer, p0.204( DEQUEUE )
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 0.3, Key_639_576
         > 1.   key 0.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 0.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 0.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.133(DM0_EntryIndex)
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
  > (F9) ? 1.   dreq p6.134(DM2_EntryIndex) 
         > 1.   mov P7_ActualResults_0, 0
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 7.   switch 2
   > (!E)? 1.   j STREAM4__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM4__TSR_ERROR_s0
STREAM4__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11150000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22250000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33350000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x44450000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x55550000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x66650000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x77750000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00750000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x11160000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x22260000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x33360000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x44460000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x55560000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x66660000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x77760000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00760000, DM2_EntryIndex 
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM4__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   jne STREAM4__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM4__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM4__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM4__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM4__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM4__DO_SK1_RESULT_CHECK_s0
STREAM4__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM4__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM4__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM4__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM4__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM4__DO_SK1_RESULT_CHECK_s0
STREAM4__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM4__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM4__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM4__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM4__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM4__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM4__EML_SUMMARIZE_RESULTS_s0
STREAM4__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM4__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM4__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM4__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM4__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM4__EML_SUMMARIZE_RESULTS_s0
STREAM4__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM4__EML_SUMMARIZE_RESULTS_s0:
         > 0.   nop
         > 1.   jz STREAM4__DO_ETU_COMPARES_s0, MisCompare_Flag
         > 1.   j STREAM4__EML_MISCOMPARE_s0
STREAM4__DO_ETU_COMPARES_s0:
         > 1.   jeq STREAM4__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM4__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM4__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM4__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData1_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 0.   nop
         > 0.   nop
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P5_ExpectResults_1,  P5_ActualResults_1
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   j STREAM4__s1
         > 0.   nop

STREAM4__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM4__SEND_ERROR_MESSAGE_s0
STREAM4__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM4__SEND_ERROR_MESSAGE_s0
STREAM4__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM4__SEND_ERROR_MESSAGE_s0
STREAM4__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM4__SEND_ERROR_MESSAGE_s0
STREAM4__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM4__SEND_ERROR_MESSAGE_s0
STREAM4__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM4__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM4__THE_END

STREAM4__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00041000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F6Flag, 1, WDTlist_RingPointer
   > (F6)? 1.   or  P0_EntryIndex, WDT_LIST_BASE, WDTlist_RingPointer
         < 2.   mov  EMC_SubKey0, rnr[dEMC_SK0_BIT_RANGE]
         < 2.   mov  EMC_SubKey1, rnr[dEMC_SK1_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   mov ttDescData1_1, 0xaaaaaaaa
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov ttDescData1_0, 0x55555555
         < 2.   hstore h(DESC__WORD12_OFFSET):8, ttDescData1
         < 2.   hstore h(DESC__WORD14_OFFSET):4, ttDescData1_1
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 0.   nop
         < 0.   nop
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
   > (F6)? 1.   load P0_WatchDogTimerControlWord, p0.6( P0_EntryIndex )
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 3.3, Key_639_576
         < 2.   key 3.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 3.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 3.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM4__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM4__TSR_ERROR_s1
STREAM4__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  EMC_SK0_ExpectResults_0, 0x888c0000, EMC_SubKey0
         < 2.   or  EMC_SK0_ExpectResults_1, 0x007c0000, EMC_SubKey0

         < 2.   mov EMC_SK0_ExpectResults_2, 0x00000000
         < 2.   mov EMC_SK0_ExpectResults_3, 0x00000000
         < 2.   or  EMC_SK1_ExpectResults_0, 0x888c0000, EMC_SubKey1
         < 2.   or  EMC_SK1_ExpectResults_1, 0x007c0000, EMC_SubKey1
         < 2.   mov EMC_SK1_ExpectResults_2, 0x00000000
         < 2.   mov EMC_SK1_ExpectResults_3, 0x00000000
   > (F6)? 1.   mov tsr_F7Flag, P0_WDTctrlword_CopPort
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM4__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM4__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_0, EMC_SK0_ActualResults_0
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_1, EMC_SK0_ActualResults_1
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_2, EMC_SK0_ActualResults_2
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_3, EMC_SK0_ActualResults_3
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_0, EMC_SK1_ActualResults_0
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_1, EMC_SK1_ActualResults_1

         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_2, EMC_SK1_ActualResults_2
         < 2.   jne STREAM4__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_3, EMC_SK1_ActualResults_3
         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM4__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM4__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM4__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM4__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   hload ttDescData1, h(DESC__WORD13_OFFSET):8
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, DescData_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x55555555, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x55555555, ttDescData1_1

         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   hstore h(48):2, 0x1122
         < 2.   hstore h(50):2, 0x3344
         < 2.   hstore h(52):2, 0x5566
         < 2.   hstore h(54):2, 0x7788
         < 2.   hstore h(56):2, 0x99aa
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
   > (F6 && !F7)? 1. load BitBucket0, pmgr.33( P0_WDTctrlword_Offset + WDT_Value )
   > (F6 && F7)?  1. load BitBucket0, pmgr.65( P0_WDTctrlword_Offset + WDT_Value )
         < 2.   hstore h(58):2, 0xbbcc 
         < 2.   hload ttDescData0_0, h(48):1  
         < 2.   hload ttDescData0_1, h(49):1  
         < 2.   teq tsr_F8Flag, 0x00000011, ttDescData0_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x00000022, ttDescData0_1
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM4__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM4__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM4__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM4__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         < 0.   movifz dcr, 1, tsr_F8Flag
         < 2.   hload ttDescData0, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData1_0, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, 0x11223344, ttDescData0_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x55667788, ttDescData0_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, 0x99aabbcc, ttDescData1_0
         < 2.   jne STREAM4__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM4__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   j STREAM4__s2

STREAM4__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM4__SEND_ERROR_MESSAGE_s1
STREAM4__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM4__SEND_ERROR_MESSAGE_s1
STREAM4__EMC_MISCOMPARE_s1:
         < 2.   or ErrorMessage, EMC_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM4__SEND_ERROR_MESSAGE_s1
STREAM4__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM4__SEND_ERROR_MESSAGE_s1
STREAM4__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM4__SEND_ERROR_MESSAGE_s1
STREAM4__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM4__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM4__THE_END

STREAM4__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM4__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM4__THE_END
STREAM4__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1


STREAM4__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 5:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00050000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         > 1.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         > 1.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov  EMC_SubKey0, rnr[dEMC_SK0_BIT_RANGE]
         > 1.   mov  EMC_SubKey1, rnr[dEMC_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   mov  RCE_Key15, rnr[30:16]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   hstore h(48):2, 0x1122
         > 1.   hstore h(50):2, 0x3344
         > 1.   hstore h(52):2, 0x5566
         > 1.   hstore h(54):2, 0x7788
         > 1.   hstore h(56):2, 0x99aa
         > 1.   hstore h(58):2, 0xbbcc
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   hload DescData_1, h(DESC__WORD12_OFFSET):4
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 5.3, Key_639_576
         > 1.   key 5.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 5.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 5.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.129(DM0_EntryIndex)
  > (F9) ? 1.   dreq p5.130(DM1_EntryIndex)
  > (F9) ? 1.   dreq p6.131(DM2_EntryIndex)
         > 1.   hload DescData_0, h(DESC__WORD13_OFFSET):4
  > (F9) ? 1.   dreq p7.132(DM3_EntryIndex)
         > 7.   switch 2
   > (!E)? 1.   j STREAM5__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM5__TSR_ERROR_s0
STREAM5__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         > 1.   or  EMC_SK0_ExpectResults_0, 0x888c0000, EMC_SubKey0
         > 1.   or  EMC_SK0_ExpectResults_1, 0x007c0000, EMC_SubKey0

         > 1.   mov EMC_SK0_ExpectResults_2, 0x00000000
         > 1.   mov EMC_SK0_ExpectResults_3, 0x00000000
         > 1.   or  EMC_SK1_ExpectResults_0, 0x888c0000, EMC_SubKey1
         > 1.   or  EMC_SK1_ExpectResults_1, 0x007c0000, EMC_SubKey1
         > 1.   mov EMC_SK1_ExpectResults_2, 0x00000000
         > 1.   mov EMC_SK1_ExpectResults_3, 0x00000000
         > 1.   teq tsr_F8Flag, 0x11223344, DescData_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55667788, DescData_0
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM5__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   jne STREAM5__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK0_ExpectResults_0, EMC_SK0_ActualResults_0
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK0_ExpectResults_1, EMC_SK0_ActualResults_1
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK0_ExpectResults_2, EMC_SK0_ActualResults_2
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK0_ExpectResults_3, EMC_SK0_ActualResults_3
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK1_ExpectResults_0, EMC_SK1_ActualResults_0
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK1_ExpectResults_1, EMC_SK1_ActualResults_1

         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK1_ExpectResults_2, EMC_SK1_ActualResults_2
         > 1.   jne STREAM5__EMC_MISCOMPARE_s0, EMC_SK1_ExpectResults_3, EMC_SK1_ActualResults_3
         > 1.   hload DescData_0, h(DESC__WORD14_OFFSET):4
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x99aabbcc, DescData_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   hload DescData_0, h(48):2
         > 1.   hload DescData_1, h(50):2
         > 1.   hload ttDescData0_0, h(52):2
         > 1.   hload ttDescData0_1, h(54):2
         > 1.   hload ttDescData1_0, h(56):2
         > 1.   hload ttDescData1_1, h(58):2
         > 1.   teq tsr_F8Flag, 0x00001122, DescData_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x00003344, DescData_1
         > 1.   movifz dcr, 1, tsr_F8Flag

         > 1.   teq tsr_F8Flag, 0x00005566, ttDescData0_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x00007788, ttDescData0_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x000099aa, ttDescData1_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x0000bbcc, ttDescData1_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         > 1.   hstore h(DESC__WORD14_OFFSET):4, P1_ActualResults_1
         > 1.   hload DescData, h(DESC__WORD12_OFFSET):8
         > 1.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   teq tsr_F8Flag, P2_ExpectResults_1, DescData_1
         > 1.   movifz dcr, 1, tsr_F8Flag

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, P1_ExpectResults_1, ttDescData0_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         > 1.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         > 1.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         > 1.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData0_0
         > 1.   movifnz dcr, 1, ttDescData0_1
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 0.   nop
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   j STREAM5__s1

STREAM5__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM5__SEND_ERROR_MESSAGE_s0
STREAM5__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM5__SEND_ERROR_MESSAGE_s0
STREAM5__EMC_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EMC_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM5__SEND_ERROR_MESSAGE_s0
STREAM5__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM5__SEND_ERROR_MESSAGE_s0
STREAM5__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM5__SEND_ERROR_MESSAGE_s0
STREAM5__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM5__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM5__THE_END

STREAM5__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00051000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE] 
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   or  P0_EntryIndex, P0_EntryIndex, WDT_PORT_LRP_STORE_ACCESS_BASE
         < 2.   mov IPV4_SubKey0_31to24, rnr[7:0]
         < 2.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV4_SubKey1_31to24, rnr[23:16]
         < 2.   mov IPV4_SubKey1_47to40, 0
         < 2.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   store p0.6( P0_EntryIndex ), P0_ActualResults
         < 2.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 4.3, Key_639_576
         < 2.   key 4.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 4.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 4.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.137(DM1_EntryIndex)
         < 2.   load P6_ActualResults, p6.6( P6_EntryIndex )
         < 2.   mov P7_ActualResults_0, 0
         < 2.   load P7_ActualResults, p7.6( P7_EntryIndex )
         < 7.   switch 2
   < (!E)? 2.   j STREAM5__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM5__TSR_ERROR_s1
STREAM5__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11190000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22290000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33390000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x44490000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x55590000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x66690000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x77790000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00790000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x99990000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0xaaa90000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0xbbb90000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00790000, DM1_EntryIndex 
         < 2.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24

         < 2.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         < 2.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
         > 0.   nop
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM5__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM5__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload ttDescData0_1, h(DESC__PKT_WORD6_OFFSET):4
         < 2.   hload ttDescData0_0, h(DESC__PKT_WORD7_OFFSET):4
         < 2.   hload ttDescData1_1, h(DESC__PKT_WORD8_OFFSET):4
         < 2.   hload ttDescData1_0, h(DESC__PKT_WORD9_OFFSET):4
         < 2.   hload ttDescData2_1, h(DESC__PKT_WORD10_OFFSET):4
         < 2.   hload ttDescData2_0, h(DESC__PKT_WORD11_OFFSET):4

         < 2.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         < 2.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         < 2.   jne STREAM5__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3

         < 2.   movifnz dcr, 1, DescData_0
         < 2.   movifnz dcr, 1, DescData_1
         < 2.   movifnz dcr, 1, ttDescData0_0
         < 2.   movifnz dcr, 1, ttDescData0_1
         < 2.   movifnz dcr, 1, ttDescData1_0
         < 2.   movifnz dcr, 1, ttDescData1_1
         < 2.   movifnz dcr, 1, ttDescData2_0
         < 2.   movifnz dcr, 1, ttDescData2_1
         < 0.   nop
         < 0.   nop
         < 2.   jeq STREAM5__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM5__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM5__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM5__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         < 2.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         < 2.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         < 2.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0 
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P6_ExpectResults_0,  P6_ActualResults_0
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P6_ExpectResults_1,  P6_ActualResults_1
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P7_ExpectResults_0,  P7_ActualResults_0
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P7_ExpectResults_1,  P7_ActualResults_1
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM5__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   j STREAM5__s2
         < 0.   nop

STREAM5__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM5__SEND_ERROR_MESSAGE_s1
STREAM5__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM5__SEND_ERROR_MESSAGE_s1
STREAM5__IPV4_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM5__SEND_ERROR_MESSAGE_s1
STREAM5__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM5__SEND_ERROR_MESSAGE_s1
STREAM5__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM5__SEND_ERROR_MESSAGE_s1
STREAM5__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM5__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM5__THE_END

STREAM5__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM5__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM5__THE_END
STREAM5__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1

STREAM5__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 6:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00060000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 0.   nop
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 0.   nop
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load WDTlist_RingPointer, p0.204( DEQUEUE )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 0.3, Key_639_576
         > 1.   key 0.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 0.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 0.0, Key_191_128, Key_127_64,  Key_63_0
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
  > (F9) ? 1.   dreq p5.135(DM1_EntryIndex)
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   mov P7_ActualResults_0, 0
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 7.   switch 2
   > (!E)? 1.   j STREAM6__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM6__TSR_ERROR_s0
STREAM6__NO_TSR_ERROR_s0:
         > 1.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         > 1.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8

         > 1.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         > 1.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         > 1.   or  DM_ExpectResults_4,  0x11170000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x22270000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x33370000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x44470000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x55570000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x66670000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x77770000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00770000, DM1_EntryIndex 
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData0_0
         > 1.   movifnz dcr, 1, ttDescData0_1
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 0.   nop
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM6__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM6__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM6__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM6__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM6__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM6__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM6__DO_SK1_RESULT_CHECK_s0
STREAM6__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM6__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM6__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM6__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM6__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM6__DO_SK1_RESULT_CHECK_s0
STREAM6__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM6__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM6__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM6__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM6__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM6__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM6__EML_SUMMARIZE_RESULTS_s0
STREAM6__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM6__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM6__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM6__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM6__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM6__EML_SUMMARIZE_RESULTS_s0
STREAM6__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM6__EML_SUMMARIZE_RESULTS_s0:
         > 1.   jz STREAM6__DO_ETU_COMPARES_s0, MisCompare_Flag
         > 1.   j STREAM6__EML_MISCOMPARE_s0
         > 0.   nop
STREAM6__DO_ETU_COMPARES_s0:
         > 1.   jeq STREAM6__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM6__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM6__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM6__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1
 
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P4_ExpectResults_0,  P4_ActualResults_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   j STREAM6__s1

STREAM6__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM6__SEND_ERROR_MESSAGE_s0
STREAM6__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM6__SEND_ERROR_MESSAGE_s0
STREAM6__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM6__SEND_ERROR_MESSAGE_s0
STREAM6__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM6__SEND_ERROR_MESSAGE_s0
STREAM6__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM6__SEND_ERROR_MESSAGE_s0
STREAM6__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM6__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM6__THE_END

STREAM6__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00061000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F6Flag, 1, WDTlist_RingPointer
   > (F6)? 1.   or  P0_EntryIndex, WDT_LIST_BASE, WDTlist_RingPointer
         < 2.   mov IPV4_SubKey0_31to24, rnr[7:0]
         < 2.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV4_SubKey1_31to24, rnr[23:16]
         < 2.   mov IPV4_SubKey1_47to40, 0
         < 2.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
   > (F6)? 1.   load P0_WatchDogTimerControlWord, p0.6( P0_EntryIndex )
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 9.3, Key_639_576
         < 2.   key 9.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 9.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 9.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM6__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM6__TSR_ERROR_s1
STREAM6__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24

         < 2.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         < 2.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
   > (F6)? 1.   mov tsr_F7Flag, P0_WDTctrlword_CopPort
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM6__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM6__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         < 2.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         < 2.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         < 2.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         < 2.   movifnz dcr, 1, DescData_0
         < 2.   movifnz dcr, 1, DescData_1

         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM6__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM6__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM6__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM6__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         < 2.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32

         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         < 2.   jne STREAM6__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3
         < 2.   movifnz dcr, 1, ttDescData0_0
         < 2.   movifnz dcr, 1, ttDescData0_1
         < 2.   movifnz dcr, 1, ttDescData1_0
         < 2.   movifnz dcr, 1, ttDescData1_1
         < 2.   movifnz dcr, 1, ttDescData2_0
         < 2.   movifnz dcr, 1, ttDescData2_1
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
   > (F6 && !F7)? 1. load BitBucket0, pmgr.33( P0_WDTctrlword_Offset + WDT_Value )
   > (F6 && F7)?  1. load BitBucket0, pmgr.65( P0_WDTctrlword_Offset + WDT_Value )
         < 2.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         < 2.   hstore h(DESC__WORD14_OFFSET):4, P8_ActualResults_1
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, P2_ExpectResults_1, DescData_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM6__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM6__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM6__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM6__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         < 2.   teq tsr_F8Flag, P8_ExpectResults_1, ttDescData0_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM6__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM6__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   j STREAM6__s2
         < 0.   nop

STREAM6__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM6__SEND_ERROR_MESSAGE_s1
STREAM6__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM6__SEND_ERROR_MESSAGE_s1
STREAM6__IPV4_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM6__SEND_ERROR_MESSAGE_s1
STREAM6__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM6__SEND_ERROR_MESSAGE_s1
STREAM6__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM6__SEND_ERROR_MESSAGE_s1
STREAM6__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM6__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM6__THE_END

STREAM6__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM6__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM6__THE_END
STREAM6__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1


STREAM6__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 7:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00070000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM3_EntryIndex, rnr[11:0]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 0.   nop
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 2.3, Key_639_576
         > 1.   key 2.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 2.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 2.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.136(DM0_EntryIndex)
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 0.   nop
  > (F9) ? 1.   dreq p7.132(DM3_EntryIndex)
         > 7.   switch 2
   > (!E)? 1.   j STREAM7__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM7__TSR_ERROR_s0
STREAM7__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11180000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22280000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33380000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x44480000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x55580000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x66680000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x77780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x99980000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0xaaa80000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0xbbb80000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 1.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM7__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM7__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM7__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM7__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM7__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM7__DO_SK1_RESULT_CHECK_s0
STREAM7__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM7__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM7__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM7__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM7__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM7__DO_SK1_RESULT_CHECK_s0
STREAM7__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM7__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM7__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM7__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM7__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM7__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM7__EML_SUMMARIZE_RESULTS_s0
STREAM7__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM7__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM7__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM7__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM7__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM7__EML_SUMMARIZE_RESULTS_s0
STREAM7__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM7__EML_SUMMARIZE_RESULTS_s0:
         > 1.   jnz STREAM7__EML_MISCOMPARE_s0, MisCompare_Flag
         > 1.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         > 1.   jne STREAM7__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   jeq STREAM7__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM7__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM7__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM7__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData0_0
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   movifnz dcr, 1, ttDescData0_1
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 1.   j STREAM7__s1

STREAM7__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM7__SEND_ERROR_MESSAGE_s0
STREAM7__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM7__SEND_ERROR_MESSAGE_s0
STREAM7__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM7__SEND_ERROR_MESSAGE_s0
STREAM7__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM7__SEND_ERROR_MESSAGE_s0
STREAM7__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM7__SEND_ERROR_MESSAGE_s0
STREAM7__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM7__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM7__THE_END

STREAM7__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00071000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV6_SubKey0_127to120, rnr[7:0]
         < 2.   mov IPV6_SubKey0_139to128, rnr[dIPV6_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV6_SubKey1_127to120, rnr[23:16]
         < 2.   mov IPV6_SubKey1_139to128, rnr[dIPV6_SK1_UPPER_BIT_RANGE]
         < 2.   mov Key_383_320[31:0], 0
         < 2.   mov Key_383_320[63:32], 0
         < 2.   mov Key_63_0[31:16], 0
         < 2.   mov Key_63_0[63:32], 0
         < 0.   nop
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 10.3, Key_639_576
         < 2.   key 10.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 10.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 10.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM7__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM7__TSR_ERROR_s1
STREAM7__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  IPV6_SK0_ExpectResults_0, 0x11000000, IPV6_SubKey0_127to120
         < 2.   or  IPV6_SK0_ExpectResults_1, 0x22000000, IPV6_SubKey0_127to120

         < 2.   or  IPV6_SK0_ExpectResults_2, 0x33000000, IPV6_SubKey0_127to120
         < 2.   or  IPV6_SK0_ExpectResults_3, 0x00400000, IPV6_SubKey0_127to120
         < 2.   mov IPV6_SK0_ExpectResults_0[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_1[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_2[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_3[19:8], IPV6_SubKey0_139to128
         > 0.   nop
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM7__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM7__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         < 2.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         < 2.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         < 2.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         < 2.   movifnz dcr, 1, DescData_0
         < 2.   movifnz dcr, 1, DescData_1

         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM7__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM7__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM7__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM7__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  IPV6_SK1_ExpectResults_0, 0x11100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_1, 0x22100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_2, 0x33100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_3, 0x00400000, IPV6_SubKey1_127to120
         < 2.   mov IPV6_SK1_ExpectResults_0[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_1[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_2[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_3[19:8], IPV6_SubKey1_139to128

         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_0, IPV6_SK0_ActualResults_0
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_1, IPV6_SK0_ActualResults_1
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_2, IPV6_SK0_ActualResults_2
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_3, IPV6_SK0_ActualResults_3
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_0, IPV6_SK1_ActualResults_0
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_1, IPV6_SK1_ActualResults_1
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_2, IPV6_SK1_ActualResults_2
         < 2.   jne STREAM7__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_3, IPV6_SK1_ActualResults_3
         < 2.   movifnz dcr, 1, ttDescData0_0
         < 2.   movifnz dcr, 1, ttDescData0_1
         < 2.   movifnz dcr, 1, ttDescData1_0
         < 2.   movifnz dcr, 1, ttDescData1_1
         < 2.   movifnz dcr, 1, ttDescData2_0
         < 2.   movifnz dcr, 1, ttDescData2_1
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         < 2.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         < 2.   hstore h(DESC__WORD14_OFFSET):4, P1_ActualResults_1
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM7__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM7__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM7__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM7__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         < 2.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM7__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM7__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 0.   nop
         < 2.   j STREAM7__s2

STREAM7__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM7__SEND_ERROR_MESSAGE_s1
STREAM7__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM7__SEND_ERROR_MESSAGE_s1
STREAM7__IPV6_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV6_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM7__SEND_ERROR_MESSAGE_s1
STREAM7__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM7__SEND_ERROR_MESSAGE_s1
STREAM7__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM7__SEND_ERROR_MESSAGE_s1
STREAM7__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM7__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM7__THE_END

STREAM7__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM7__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM7__THE_END
STREAM7__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1

STREAM7__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop




stream 8:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00080000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE] 
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 0.   nop
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load WDTlist_RingPointer, p0.204( DEQUEUE )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 0.3, Key_639_576
         > 1.   key 0.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 0.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 0.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.128(DM0_EntryIndex)
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   mov P7_ActualResults_0, 0
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 7.   switch 2
   > (!E)? 1.   j STREAM8__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM8__TSR_ERROR_s0
STREAM8__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11100000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22200000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33300000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x44400000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x55500000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x66600000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x77700000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00700000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x99900000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0xaaa00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0xbbb00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0xccc00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0xddd00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0xeee00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0xfff00000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00700000, DM0_EntryIndex 
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 1.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM8__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   jne STREAM8__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM8__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM8__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM8__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM8__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM8__DO_SK1_RESULT_CHECK_s0
STREAM8__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM8__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM8__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM8__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM8__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM8__DO_SK1_RESULT_CHECK_s0
STREAM8__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM8__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM8__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM8__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM8__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM8__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM8__EML_SUMMARIZE_RESULTS_s0
STREAM8__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM8__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM8__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM8__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM8__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM8__EML_SUMMARIZE_RESULTS_s0
STREAM8__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM8__EML_SUMMARIZE_RESULTS_s0:
         > 1.   jz STREAM8__DO_ETU_COMPARES_s0, MisCompare_Flag
         > 1.   j STREAM8__EML_MISCOMPARE_s0
STREAM8__DO_ETU_COMPARES_s0:
         > 1.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         > 1.   jeq STREAM8__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM8__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM8__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM8__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   j STREAM8__s1
         > 0.   nop

STREAM8__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM8__SEND_ERROR_MESSAGE_s0
STREAM8__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM8__SEND_ERROR_MESSAGE_s0
STREAM8__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM8__SEND_ERROR_MESSAGE_s0
STREAM8__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM8__SEND_ERROR_MESSAGE_s0
STREAM8__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM8__SEND_ERROR_MESSAGE_s0
STREAM8__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM8__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM8__THE_END

STREAM8__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00081000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F6Flag, 1, WDTlist_RingPointer
   > (F6)? 1.   or  P0_EntryIndex, WDT_LIST_BASE, WDTlist_RingPointer
         < 2.   mov  EMC_SubKey0, rnr[dEMC_SK0_BIT_RANGE]
         < 2.   mov  EMC_SubKey1, rnr[dEMC_SK1_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
   > (F6)? 1.   load P0_WatchDogTimerControlWord, p0.6( P0_EntryIndex )
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 3.3, Key_639_576
         < 2.   key 3.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 3.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 3.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM8__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM8__TSR_ERROR_s1
STREAM8__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  EMC_SK0_ExpectResults_0, 0x888c0000, EMC_SubKey0
         < 2.   or  EMC_SK0_ExpectResults_1, 0x007c0000, EMC_SubKey0

         < 2.   mov EMC_SK0_ExpectResults_2, 0x00000000
         < 2.   mov EMC_SK0_ExpectResults_3, 0x00000000
         < 2.   or  EMC_SK1_ExpectResults_0, 0x888c0000, EMC_SubKey1
         < 2.   or  EMC_SK1_ExpectResults_1, 0x007c0000, EMC_SubKey1
         < 2.   mov EMC_SK1_ExpectResults_2, 0x00000000
         < 2.   mov EMC_SK1_ExpectResults_3, 0x00000000
   > (F6)? 1.   mov tsr_F7Flag, P0_WDTctrlword_CopPort
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM8__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM8__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_0, EMC_SK0_ActualResults_0
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_1, EMC_SK0_ActualResults_1
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_2, EMC_SK0_ActualResults_2
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK0_ExpectResults_3, EMC_SK0_ActualResults_3
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_0, EMC_SK1_ActualResults_0
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_1, EMC_SK1_ActualResults_1

         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_2, EMC_SK1_ActualResults_2
         < 2.   jne STREAM8__EMC_MISCOMPARE_s1, EMC_SK1_ExpectResults_3, EMC_SK1_ActualResults_3
         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM8__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM8__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM8__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM8__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   hload DescData_1, h(DESC__PKT_WORD4_OFFSET):4
         < 2.   hload DescData_0, h(DESC__PKT_WORD5_OFFSET):4
         < 2.   hload ttDescData0_1, h(DESC__PKT_WORD6_OFFSET):4
         < 2.   hload ttDescData0_0, h(DESC__PKT_WORD7_OFFSET):4
         < 2.   hload ttDescData1_1, h(DESC__PKT_WORD8_OFFSET):4
         < 2.   hload ttDescData1_0, h(DESC__PKT_WORD9_OFFSET):4

         < 2.   hload ttDescData2_1, h(DESC__PKT_WORD10_OFFSET):4
         < 2.   hload ttDescData2_0, h(DESC__PKT_WORD11_OFFSET):4
         < 2.   movifnz dcr, 1, DescData_0
         < 2.   movifnz dcr, 1, DescData_1
         < 2.   movifnz dcr, 1, ttDescData0_0
         < 2.   movifnz dcr, 1, ttDescData0_1
         < 2.   movifnz dcr, 1, ttDescData1_0
         < 2.   movifnz dcr, 1, ttDescData1_1
         < 2.   movifnz dcr, 1, ttDescData2_0
         < 2.   movifnz dcr, 1, ttDescData2_1
         < 2.   hstore h(DESC__WORD12_OFFSET):8, P2_ActualResults
         < 2.   hstore h(DESC__WORD14_OFFSET):4, P8_ActualResults_1
         < 2.   hload DescData, h(DESC__WORD12_OFFSET):8
         < 2.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
   > (F6 && !F7)? 1. load BitBucket0, pmgr.33( P0_WDTctrlword_Offset + WDT_Value )
   > (F6 && F7)?  1. load BitBucket0, pmgr.65( P0_WDTctrlword_Offset + WDT_Value )
         < 2.   teq tsr_F8Flag, P2_ExpectResults_1, DescData_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   teq tsr_F8Flag, P2_ExpectResults_0, DescData_0
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 0.   nop
         < 0.   nop
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM8__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM8__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM8__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM8__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         < 2.   teq tsr_F8Flag, P8_ExpectResults_1, ttDescData0_1
         < 2.   movifz dcr, 1, tsr_F8Flag
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM8__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM8__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 0.   nop
         < 2.   j STREAM8__s2

STREAM8__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM8__SEND_ERROR_MESSAGE_s1
STREAM8__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM8__SEND_ERROR_MESSAGE_s1
STREAM8__EMC_MISCOMPARE_s1:
         < 2.   or ErrorMessage, EMC_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM8__SEND_ERROR_MESSAGE_s1
STREAM8__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM8__SEND_ERROR_MESSAGE_s1
STREAM8__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM8__SEND_ERROR_MESSAGE_s1
STREAM8__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM8__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM8__THE_END

STREAM8__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM8__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM8__THE_END
STREAM8__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1


STREAM8__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 9:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x00090000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         > 1.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         > 1.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV6_SubKey0_127to120, rnr[7:0]
         > 1.   mov IPV6_SubKey0_139to128, rnr[dIPV6_SK0_UPPER_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   mov  RCE_Key15, rnr[30:16]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV6_SubKey1_127to120, rnr[23:16]
         > 1.   mov IPV6_SubKey1_139to128, rnr[dIPV6_SK1_UPPER_BIT_RANGE]
         > 1.   mov ttDescData0_1, 0xaaaaaaaa
         > 1.   mov ttDescData0_0, 0x55555555
         > 1.   hstore h(DESC__WORD12_OFFSET):8, ttDescData0
         > 1.   hstore h(DESC__WORD14_OFFSET):4, ttDescData0_0
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   hload DescData, h(DESC__WORD12_OFFSET):8
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 6.3, Key_639_576
         > 1.   key 6.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 6.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 6.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.129(DM0_EntryIndex)
  > (F9) ? 1.   dreq p5.130(DM1_EntryIndex)
  > (F9) ? 1.   dreq p6.131(DM2_EntryIndex)
         > 0.   nop
  > (F9) ? 1.   dreq p7.132(DM3_EntryIndex)
         > 7.   switch 2
   > (!E)? 1.   j STREAM9__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM9__TSR_ERROR_s0
STREAM9__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, DescData_1
         > 1.   movifz dcr, 1, tsr_F8Flag

         > 1.   teq tsr_F8Flag, 0x55555555, DescData_0
         > 1.   hload ttDescData1_0, h(DESC__WORD14_OFFSET):4
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData1_0
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   xor DescData_0, 0xffffffff, DescData_0
         > 1.   xor DescData_1, 0xffffffff, DescData_1
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8

  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM9__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15
         > 1.   jne STREAM9__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   hstore h(DESC__WORD12_OFFSET):8, DescData
         > 1.   hstore h(DESC__WORD14_OFFSET):4, DescData_1
         > 1.   hload ttDescData1, h(DESC__WORD12_OFFSET):8
         > 1.   hload ttDescData0_1, h(DESC__WORD14_OFFSET):4
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData1_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 1.   teq tsr_F8Flag, 0xaaaaaaaa, ttDescData1_0
         > 1.   movifz dcr, 1, tsr_F8Flag

         > 1.   or  IPV6_SK0_ExpectResults_0, 0x11000000, IPV6_SubKey0_127to120
         > 1.   or  IPV6_SK0_ExpectResults_1, 0x22000000, IPV6_SubKey0_127to120
         > 1.   or  IPV6_SK0_ExpectResults_2, 0x33000000, IPV6_SubKey0_127to120
         > 1.   or  IPV6_SK0_ExpectResults_3, 0x00400000, IPV6_SubKey0_127to120
         > 1.   mov IPV6_SK0_ExpectResults_0[19:8], IPV6_SubKey0_139to128
         > 1.   mov IPV6_SK0_ExpectResults_1[19:8], IPV6_SubKey0_139to128
         > 1.   mov IPV6_SK0_ExpectResults_2[19:8], IPV6_SubKey0_139to128
         > 1.   mov IPV6_SK0_ExpectResults_3[19:8], IPV6_SubKey0_139to128
         > 1.   or  IPV6_SK1_ExpectResults_0, 0x11100000, IPV6_SubKey1_127to120
         > 1.   or  IPV6_SK1_ExpectResults_1, 0x22100000, IPV6_SubKey1_127to120
         > 1.   or  IPV6_SK1_ExpectResults_2, 0x33100000, IPV6_SubKey1_127to120
         > 1.   or  IPV6_SK1_ExpectResults_3, 0x00400000, IPV6_SubKey1_127to120
         > 1.   mov IPV6_SK1_ExpectResults_0[19:8], IPV6_SubKey1_139to128
         > 1.   mov IPV6_SK1_ExpectResults_1[19:8], IPV6_SubKey1_139to128
         > 1.   mov IPV6_SK1_ExpectResults_2[19:8], IPV6_SubKey1_139to128
         > 1.   mov IPV6_SK1_ExpectResults_3[19:8], IPV6_SubKey1_139to128

         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_0, IPV6_SK0_ActualResults_0
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_1, IPV6_SK0_ActualResults_1
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_2, IPV6_SK0_ActualResults_2
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK0_ExpectResults_3, IPV6_SK0_ActualResults_3
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_0, IPV6_SK1_ActualResults_0
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_1, IPV6_SK1_ActualResults_1
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_2, IPV6_SK1_ActualResults_2
         > 1.   jne STREAM9__IPV6_MISCOMPARE_s0, IPV6_SK1_ExpectResults_3, IPV6_SK1_ActualResults_3
         > 1.   teq tsr_F8Flag, 0x55555555, ttDescData0_1
         > 1.   movifz dcr, 1, tsr_F8Flag
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   hload DescData_1, h(DESC__PKT_WORD4_OFFSET):4
         > 1.   hload DescData_0, h(DESC__PKT_WORD5_OFFSET):4
         > 1.   hload ttDescData0_1, h(DESC__PKT_WORD6_OFFSET):4
         > 1.   hload ttDescData0_0, h(DESC__PKT_WORD7_OFFSET):4
         > 1.   hload ttDescData1_1, h(DESC__PKT_WORD8_OFFSET):4
         > 1.   hload ttDescData1_0, h(DESC__PKT_WORD9_OFFSET):4
         > 1.   hload ttDescData2_1, h(DESC__PKT_WORD10_OFFSET):4
         > 1.   hload ttDescData2_0, h(DESC__PKT_WORD11_OFFSET):4
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData0_0
         > 1.   movifnz dcr, 1, ttDescData0_1
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   j STREAM9__s1

STREAM9__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM9__SEND_ERROR_MESSAGE_s0
STREAM9__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM9__SEND_ERROR_MESSAGE_s0
STREAM9__IPV6_MISCOMPARE_s0:
         > 1.   or ErrorMessage, IPV6_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM9__SEND_ERROR_MESSAGE_s0
STREAM9__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM9__SEND_ERROR_MESSAGE_s0
STREAM9__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM9__SEND_ERROR_MESSAGE_s0
STREAM9__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM9__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM9__THE_END

STREAM9__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x00091000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE] 
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   or  P0_EntryIndex, P0_EntryIndex, WDT_PORT_LRP_STORE_ACCESS_BASE
         < 2.   mov IPV4_SubKey0_31to24, rnr[7:0]
         < 2.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV4_SubKey1_31to24, rnr[23:16]
         < 2.   mov IPV4_SubKey1_47to40, 0
         < 2.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   store p0.6( P0_EntryIndex ), P0_ActualResults
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 4.3, Key_639_576
         < 2.   key 4.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 4.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 4.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.128(DM0_EntryIndex)
         < 2.   load P5_ActualResults, p5.6( P5_EntryIndex )
         < 2.   load P6_ActualResults, p6.6( P6_EntryIndex )
         < 2.   mov P7_ActualResults_0, 0
         < 2.   load P7_ActualResults, p7.6( P7_EntryIndex )
         < 7.   switch 2
   < (!E)? 2.   j STREAM9__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM9__TSR_ERROR_s1
STREAM9__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11100000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22200000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33300000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x44400000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x55500000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x66600000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x77700000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00700000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x99900000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0xaaa00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0xbbb00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0xccc00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0xddd00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0xeee00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0xfff00000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00700000, DM0_EntryIndex 
         < 2.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24

         < 2.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         < 2.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
         > 0.   nop
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM9__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM9__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop

         < 2.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         < 2.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         < 2.   jne STREAM9__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3

         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jeq STREAM9__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM9__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM9__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM9__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         < 2.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         < 2.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         < 2.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         < 2.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         < 2.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         < 2.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         < 0.   nop
         < 0.   nop
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P5_ExpectResults_0,  P5_ActualResults_0
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P5_ExpectResults_1,  P5_ActualResults_1
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P6_ExpectResults_0,  P6_ActualResults_0
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P6_ExpectResults_1,  P6_ActualResults_1
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P7_ExpectResults_0,  P7_ActualResults_0
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P7_ExpectResults_1,  P7_ActualResults_1
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM9__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   j STREAM9__s2
         < 0.   nop

STREAM9__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM9__SEND_ERROR_MESSAGE_s1
STREAM9__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM9__SEND_ERROR_MESSAGE_s1
STREAM9__IPV4_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM9__SEND_ERROR_MESSAGE_s1
STREAM9__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM9__SEND_ERROR_MESSAGE_s1
STREAM9__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM9__SEND_ERROR_MESSAGE_s1
STREAM9__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM9__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM9__THE_END

STREAM9__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM9__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM9__THE_END
STREAM9__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1

STREAM9__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 10:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x000a0000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   mov  EML_SubKey0, rnr[dEML_SK0_BIT_RANGE]
         > 1.   mov  EML_SubKey1, rnr[dEML_SK1_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov SK0_TotalCounterID, EML_SubKey0
         > 1.   mov SK1_TotalCounterID, EML_SubKey1
         > 0.   nop
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load WDTlist_RingPointer, p0.204( DEQUEUE )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 0.3, Key_639_576
         > 1.   key 0.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 0.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 0.0, Key_191_128, Key_127_64,  Key_63_0
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
  > (F9) ? 1.   dreq p5.135(DM1_EntryIndex)
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   mov P7_ActualResults_0, 0
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 7.   switch 2
   > (!E)? 1.   j STREAM10__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM10__TSR_ERROR_s0
STREAM10__NO_TSR_ERROR_s0:
         > 0.   nop
         > 0.   nop

         > 0.   nop
         > 0.   nop
         > 1.   or  DM_ExpectResults_4,  0x11170000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x22270000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x33370000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x44470000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x55570000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0x66670000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0x77770000, DM1_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00770000, DM1_EntryIndex 
         > 1.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         > 1.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         > 1.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         > 1.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         > 1.   mov tsr_F0Flag, 1
         > 1.   mov tsr_F1Flag, 1

         > 1.   or  EML_SK0_ExpectResults_0, 0x11100000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_1, 0x22200000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_2, 0x33300000, EML_SubKey0
         > 1.   or  EML_SK0_ExpectResults_3, 0x00700000, EML_SubKey0
         > 1.   or  EML_SK1_ExpectResults_0, 0x11100000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_1, 0x22200000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_2, 0x33300000, EML_SubKey1
         > 1.   or  EML_SK1_ExpectResults_3, 0x00700000, EML_SubKey1
         > 0.   nop
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData0_0
         > 1.   movifnz dcr, 1, ttDescData0_1
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6

  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM10__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 1.   jne STREAM10__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 1.   mov MisCompare_Flag, 0
         > 1.   jne STREAM10__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_0, EML_SK0_ActualResults_0
         > 1.   jne STREAM10__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_1, EML_SK0_ActualResults_1
         > 1.   jne STREAM10__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_2, EML_SK0_ActualResults_2
         > 1.   jne STREAM10__EML_SK0_MISCOMPARE_s0, EML_SK0_ExpectResults_3, EML_SK0_ActualResults_3
         > 1.   mov tsr_F2Flag, 1

         > 1.   j STREAM10__DO_SK1_RESULT_CHECK_s0
STREAM10__EML_SK0_MISCOMPARE_s0:
         > 1.   jne STREAM10__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_0
         > 1.   jne STREAM10__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_1
         > 1.   jne STREAM10__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_2
         > 1.   jne STREAM10__EML_TRUE_SK0_MISCOMPARE_s0, 0, EML_SK0_ActualResults_3
         > 1.   or SK0_MissCounterID, MAX_KEYS, EML_SubKey0
         > 1.   mov tsr_F3Flag, 1
         > 1.   j STREAM10__DO_SK1_RESULT_CHECK_s0
STREAM10__EML_TRUE_SK0_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM10__DO_SK1_RESULT_CHECK_s0:
         > 1.   jne STREAM10__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_0, EML_SK1_ActualResults_0
         > 1.   jne STREAM10__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_1, EML_SK1_ActualResults_1
         > 1.   jne STREAM10__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_2, EML_SK1_ActualResults_2
         > 1.   jne STREAM10__EML_SK1_MISCOMPARE_s0, EML_SK1_ExpectResults_3, EML_SK1_ActualResults_3
         > 1.   mov tsr_F4Flag, 1
         > 1.   j STREAM10__EML_SUMMARIZE_RESULTS_s0
STREAM10__EML_SK1_MISCOMPARE_s0:
         > 1.   jne STREAM10__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_0

         > 1.   jne STREAM10__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_1
         > 1.   jne STREAM10__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_2
         > 1.   jne STREAM10__EML_TRUE_SK1_MISCOMPARE_s0, 0, EML_SK1_ActualResults_3
         > 1.   or SK1_MissCounterID, MAX_KEYS, EML_SubKey1
         > 1.   mov tsr_F5Flag, 1
         > 1.   j STREAM10__EML_SUMMARIZE_RESULTS_s0
STREAM10__EML_TRUE_SK1_MISCOMPARE_s0:
         > 1.   mov MisCompare_Flag, 1
STREAM10__EML_SUMMARIZE_RESULTS_s0:
         > 1.   jz STREAM10__DO_ETU_COMPARES_s0, MisCompare_Flag
         > 1.   j STREAM10__EML_MISCOMPARE_s0
         > 0.   nop
STREAM10__DO_ETU_COMPARES_s0:
         > 1.   jeq STREAM10__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM10__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM10__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM10__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1
 
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P4_ExpectResults_0,  P4_ActualResults_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   j STREAM10__s1
         > 0.   nop

STREAM10__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM10__SEND_ERROR_MESSAGE_s0
STREAM10__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM10__SEND_ERROR_MESSAGE_s0
STREAM10__EML_MISCOMPARE_s0:
         > 1.   or ErrorMessage, EML_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM10__SEND_ERROR_MESSAGE_s0
STREAM10__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM10__SEND_ERROR_MESSAGE_s0
STREAM10__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM10__SEND_ERROR_MESSAGE_s0
STREAM10__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM10__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM10__THE_END

STREAM10__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x000a1000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   movifnz tsr_F6Flag, 1, WDTlist_RingPointer
   > (F6)? 1.   or  P0_EntryIndex, WDT_LIST_BASE, WDTlist_RingPointer
         < 2.   mov IPV4_SubKey0_31to24, rnr[7:0]
         < 2.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 0.   nop
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov IPV4_SubKey1_31to24, rnr[23:16]
         < 2.   mov IPV4_SubKey1_47to40, 0
         < 2.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
   > (F6)? 1.   load P0_WatchDogTimerControlWord, p0.6( P0_EntryIndex )
         < 0.   nop
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 11.3, Key_639_576
         < 2.   key 11.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 11.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 11.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM10__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM10__TSR_ERROR_s1
STREAM10__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24

         < 2.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         < 2.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         < 2.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         < 2.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
   > (F6)? 1.   mov tsr_F7Flag, P0_WDTctrlword_CopPort
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM10__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM10__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         < 2.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         < 2.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         < 2.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         < 2.   movifnz dcr, 1, DescData_0
         < 2.   movifnz dcr, 1, DescData_1

         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM10__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM10__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM10__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM10__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         < 2.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         < 2.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         < 2.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32

         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         < 2.   jne STREAM10__IPV4_MISCOMPARE_s1, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3
         < 2.   movifnz dcr, 1, ttDescData0_0
         < 2.   movifnz dcr, 1, ttDescData0_1
         < 2.   movifnz dcr, 1, ttDescData1_0
         < 2.   movifnz dcr, 1, ttDescData1_1
         < 2.   movifnz dcr, 1, ttDescData2_0
         < 2.   movifnz dcr, 1, ttDescData2_1
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 0.   nop
         > 0.   nop
   > (F6 && !F7)? 1. load BitBucket0, pmgr.33( P0_WDTctrlword_Offset + WDT_Value )
   > (F6 && F7)?  1. load BitBucket0, pmgr.65( P0_WDTctrlword_Offset + WDT_Value )
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM10__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM10__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM10__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM10__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM10__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM10__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   j STREAM10__s2
         < 0.   nop

STREAM10__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM10__SEND_ERROR_MESSAGE_s1
STREAM10__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM10__SEND_ERROR_MESSAGE_s1
STREAM10__IPV4_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM10__SEND_ERROR_MESSAGE_s1
STREAM10__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM10__SEND_ERROR_MESSAGE_s1
STREAM10__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM10__SEND_ERROR_MESSAGE_s1
STREAM10__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM10__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM10__THE_END

STREAM10__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM10__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM10__THE_END
STREAM10__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1


STREAM10__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop







stream 11:

;        > 7.   switch 2
         > 1.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         > 1.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         > 1.   or ErrorMessage, 0x000b0000, FlowID
         > 1.   mov  PacketLengthCounterOffset, SrcQ
         > 1.   store cmgr.2(SrcQ), PacketLength
         > 1.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov DM3_EntryIndex, rnr[11:0]
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV4_SubKey0_31to24, rnr[7:0]
         > 1.   mov IPV4_SubKey0_43to32, rnr[dIPV4_SK0_UPPER_BIT_RANGE]
         > 1.   load BitBucket0, cop.32(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)

         > 1.   mov  RCE_Key15, rnr[30:16]
         > 1.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key15 )
         > 7.   hrestore
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov IPV4_SubKey1_31to24, rnr[23:16]
         > 1.   mov IPV4_SubKey1_43to32, rnr[dIPV4_SK1_UPPER_BIT_RANGE]
         > 1.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 0.   nop
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 1.   key 1.3, Key_639_576
         > 1.   key 1.2, Key_575_512, Key_511_448, Key_447_384
         > 1.   key 1.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 1.0, Key_191_128, Key_127_64,  Key_63_0
  > (F9) ? 1.   dreq p4.136(DM0_EntryIndex)
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 0.   nop
  > (F9) ? 1.   dreq p7.132(DM3_EntryIndex)
         > 7.   switch 2
   > (!E)? 1.   j STREAM11__NO_TSR_ERROR_s0
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 1.   j  STREAM11__TSR_ERROR_s0
STREAM11__NO_TSR_ERROR_s0:
         > 1.   or  DM_ExpectResults_0,  0x11180000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_1,  0x22280000, DM0_EntryIndex 

         > 1.   or  DM_ExpectResults_2,  0x33380000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_3,  0x44480000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_4,  0x55580000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_5,  0x66680000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_6,  0x77780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_7,  0x00780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_8,  0x99980000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_9,  0xaaa80000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_10, 0xbbb80000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_11, 0x00780000, DM0_EntryIndex 
         > 1.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         > 1.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         > 0.   nop
         > 0.   nop

  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_0,  DM_ActualResults_0
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_1,  DM_ActualResults_1
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_2,  DM_ActualResults_2
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_3,  DM_ActualResults_3
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_4,  DM_ActualResults_4
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_5,  DM_ActualResults_5
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_6,  DM_ActualResults_6
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_7,  DM_ActualResults_7
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_8,  DM_ActualResults_8
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_9,  DM_ActualResults_9
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_10, DM_ActualResults_10
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_11, DM_ActualResults_11
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_12, DM_ActualResults_12
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_13, DM_ActualResults_13
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_14, DM_ActualResults_14
  > (F9) ? 1.   jne STREAM11__DM_MISCOMPARE_s0, DM_ExpectResults_15, DM_ActualResults_15

         > 1.   or  IPV4_SK0_ExpectResults_0, 0x11000000, IPV4_SubKey0_31to24
         > 1.   or  IPV4_SK0_ExpectResults_1, 0x22000000, IPV4_SubKey0_31to24
         > 1.   or  IPV4_SK0_ExpectResults_2, 0x33000000, IPV4_SubKey0_31to24
         > 1.   or  IPV4_SK0_ExpectResults_3, 0x00400000, IPV4_SubKey0_31to24
         > 1.   mov IPV4_SK0_ExpectResults_0[19:8], IPV4_SubKey0_43to32
         > 1.   mov IPV4_SK0_ExpectResults_1[19:8], IPV4_SubKey0_43to32
         > 1.   mov IPV4_SK0_ExpectResults_2[19:8], IPV4_SubKey0_43to32
         > 1.   mov IPV4_SK0_ExpectResults_3[19:8], IPV4_SubKey0_43to32
         > 1.   or  IPV4_SK1_ExpectResults_0, 0x11100000, IPV4_SubKey1_31to24
         > 1.   or  IPV4_SK1_ExpectResults_1, 0x22100000, IPV4_SubKey1_31to24
         > 1.   or  IPV4_SK1_ExpectResults_2, 0x33100000, IPV4_SubKey1_31to24
         > 1.   or  IPV4_SK1_ExpectResults_3, 0x00400000, IPV4_SubKey1_31to24
         > 1.   mov IPV4_SK1_ExpectResults_0[19:8], IPV4_SubKey1_43to32
         > 1.   mov IPV4_SK1_ExpectResults_1[19:8], IPV4_SubKey1_43to32
         > 1.   mov IPV4_SK1_ExpectResults_2[19:8], IPV4_SubKey1_43to32
         > 1.   mov IPV4_SK1_ExpectResults_3[19:8], IPV4_SubKey1_43to32

         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_0, IPV4_SK0_ActualResults_0
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_1, IPV4_SK0_ActualResults_1
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_2, IPV4_SK0_ActualResults_2
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK0_ExpectResults_3, IPV4_SK0_ActualResults_3
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_0, IPV4_SK1_ActualResults_0
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_1, IPV4_SK1_ActualResults_1
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_2, IPV4_SK1_ActualResults_2
         > 1.   jne STREAM11__IPV4_MISCOMPARE_s0, IPV4_SK1_ExpectResults_3, IPV4_SK1_ActualResults_3
         > 1.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         > 1.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         > 1.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         > 1.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         > 1.   movifnz dcr, 1, DescData_0
         > 1.   movifnz dcr, 1, DescData_1
         > 1.   movifnz dcr, 1, ttDescData0_0
         > 1.   movifnz dcr, 1, ttDescData0_1

         > 1.   jne STREAM11__RCE_MISCOMPARE_s0, RCE_ExpectedResults_0, RCE_ActualResults_0
         > 0.   nop
         > 1.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         > 1.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         > 1.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         > 1.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         > 1.   jeq STREAM11__ETU_MISCOMPARE_s0, ETU_ExpectResults_0, ETU_ActualResults_0
         > 1.   jeq STREAM11__ETU_MISCOMPARE_s0, ETU_ExpectResults_1, ETU_ActualResults_1
         > 1.   jeq STREAM11__ETU_MISCOMPARE_s0, ETU_ExpectResults_2, ETU_ActualResults_2
         > 1.   jeq STREAM11__ETU_MISCOMPARE_s0, ETU_ExpectResults_3, ETU_ActualResults_3
         > 1.   movifnz dcr, 1, ttDescData1_0
         > 1.   movifnz dcr, 1, ttDescData1_1
         > 1.   movifnz dcr, 1, ttDescData2_0
         > 1.   movifnz dcr, 1, ttDescData2_1
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 0.   nop
         > 0.   nop
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P2_ExpectResults_1,  P2_ActualResults_1

         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P0_ExpectResults_1,  P0_ActualResults_1
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P8_ExpectResults_0,  P8_ActualResults_0
         > 0.   nop
         > 0.   nop
         > 1.   jne STREAM11__PORT_MISCOMPARE_s0, P8_ExpectResults_1,  P8_ActualResults_1
         > 1.   j STREAM11__s1
         > 0.   nop

STREAM11__DM_MISCOMPARE_s0:
         > 1.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM11__SEND_ERROR_MESSAGE_s0
STREAM11__IPV4_MISCOMPARE_s0:
         > 1.   or ErrorMessage, IPV4_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM11__SEND_ERROR_MESSAGE_s0
STREAM11__ETU_MISCOMPARE_s0:
         > 1.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM11__SEND_ERROR_MESSAGE_s0
STREAM11__PORT_MISCOMPARE_s0:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM11__SEND_ERROR_MESSAGE_s0
STREAM11__RCE_MISCOMPARE_s0:
         > 1.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         > 1.   j STREAM11__SEND_ERROR_MESSAGE_s0
STREAM11__TSR_ERROR_s0:
         > 1.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         > 1.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         > 0.   nop
STREAM11__SEND_ERROR_MESSAGE_s0:
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1
         > 1.   j STREAM11__THE_END

STREAM11__s1:
         < 2.   hload  SrcQField, h(DESC__SQUEUE_OFFSET):1
         < 2.   hload  FrameLengthField, h(DESC__FRAME_LEN_OFFSET):2
         < 2.   or ErrorMessage, 0x000b1000, FlowID
         < 2.   sub  PacketLengthCounterOffset, SrcQ, INGRESS_SQUEUE_NUM
         < 2.   load BitBucket0, cop.64(PacketLengthCounterOffset + PacketLength + COHTAB_COUNT)
         < 2.   store cmgr.3(SrcQ), PacketLength
         < 2.   mov DM0_EntryIndex, rnr[dDM0_BIT_RANGE]
         < 2.   mov DM1_EntryIndex, rnr[dDM1_BIT_RANGE]
         < 2.   mov DM2_EntryIndex, rnr[dDM2_BIT_RANGE]
         < 2.   mov DM3_EntryIndex, rnr[dDM3_BIT_RANGE]
         < 2.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 0.   nop
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov Key_63_0[31:0], 0
         < 2.   mov Key_63_0[63:32], 0 

         < 2.   mov  ETU_Key, rnr[dETU_BIT_RANGE]
         < 2.   load RCE_ExpectedResults_1_0, p9.6( RCE_Key14 )
         < 2.   mov Key_511_448[31:0], 0
         < 2.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         < 2.   mov Key_511_448[63:32], 0
         < 2.   mov Key_383_320[31:0], 0
         < 2.   mov Key_383_320[63:32], 0
         < 2.   mov IPV6_SubKey0_127to120, rnr[7:0]
         < 2.   mov IPV6_SubKey0_139to128, rnr[dIPV6_SK0_UPPER_BIT_RANGE]
         < 2.   mov IPV6_SubKey1_127to120, rnr[23:16]
         < 2.   mov IPV6_SubKey1_139to128, rnr[dIPV6_SK1_UPPER_BIT_RANGE]
         < 2.   load P2_ActualResults, p2.6( P2_EntryIndex )
         < 2.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         < 2.   movifnz tsr_F9Flag, 1, rnr[dDREQ_PROBABILITY]
         < 2.   load P8_ActualResults, p8.6( P8_EntryIndex )

         < 2.   key 12.3, Key_639_576
         < 2.   key 12.2, Key_575_512, Key_511_448, Key_447_384
         < 2.   key 12.1, Key_383_320, Key_319_256, Key_255_192
         < 2.   key 12.0, Key_191_128, Key_127_64,  Key_63_0
  < (F9) ? 2.   dreq p4.129(DM0_EntryIndex)
  < (F9) ? 2.   dreq p5.130(DM1_EntryIndex)
  < (F9) ? 2.   dreq p6.131(DM2_EntryIndex)
         < 0.   nop
  < (F9) ? 2.   dreq p7.132(DM3_EntryIndex)
         < 7.   switch 2
   < (!E)? 2.   j STREAM11__NO_TSR_ERROR_s1
   <  (E)? 2.   mov tsr_ErrorFlag, 0
         < 2.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         < 2.   j  STREAM11__TSR_ERROR_s1
STREAM11__NO_TSR_ERROR_s1:
         < 2.   or  DM_ExpectResults_0,  0x11110000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_1,  0x22210000, DM0_EntryIndex 

         < 2.   or  DM_ExpectResults_2,  0x33310000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_3,  0x00410000, DM0_EntryIndex 
         < 2.   or  DM_ExpectResults_4,  0x11120000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_5,  0x22220000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_6,  0x33320000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_7,  0x00420000, DM1_EntryIndex 
         < 2.   or  DM_ExpectResults_8,  0x11130000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_9,  0x22230000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_10, 0x33330000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_11, 0x00430000, DM2_EntryIndex 
         < 2.   or  DM_ExpectResults_12, 0x11140000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_13, 0x22240000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_14, 0x33340000, DM3_EntryIndex 
         < 2.   or  DM_ExpectResults_15, 0x00440000, DM3_EntryIndex 
         < 2.   or  IPV6_SK0_ExpectResults_0, 0x11000000, IPV6_SubKey0_127to120
         < 2.   or  IPV6_SK0_ExpectResults_1, 0x22000000, IPV6_SubKey0_127to120

         < 2.   or  IPV6_SK0_ExpectResults_2, 0x33000000, IPV6_SubKey0_127to120
         < 2.   or  IPV6_SK0_ExpectResults_3, 0x00400000, IPV6_SubKey0_127to120
         < 2.   mov IPV6_SK0_ExpectResults_0[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_1[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_2[19:8], IPV6_SubKey0_139to128
         < 2.   mov IPV6_SK0_ExpectResults_3[19:8], IPV6_SubKey0_139to128
         > 0.   nop
         > 0.   nop
         > 0.   nop
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_0,  DM_ActualResults_0
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_1,  DM_ActualResults_1
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_2,  DM_ActualResults_2
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_3,  DM_ActualResults_3
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_4,  DM_ActualResults_4
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_5,  DM_ActualResults_5
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_6,  DM_ActualResults_6

  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_7,  DM_ActualResults_7
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_8,  DM_ActualResults_8
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_9,  DM_ActualResults_9
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_10, DM_ActualResults_10
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_11, DM_ActualResults_11
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_12, DM_ActualResults_12
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_13, DM_ActualResults_13
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_14, DM_ActualResults_14
  < (F9) ? 2.   jne STREAM11__DM_MISCOMPARE_s1, DM_ExpectResults_15, DM_ActualResults_15
         < 2.   jne STREAM11__RCE_MISCOMPARE_s1, RCE_ExpectedResults_0, RCE_ActualResults_0
         < 2.   hload DescData, h(DESC__PKT_WORD4_OFFSET):8
         < 2.   hload ttDescData0, h(DESC__PKT_WORD6_OFFSET):8
         < 2.   hload ttDescData1, h(DESC__PKT_WORD8_OFFSET):8
         < 2.   hload ttDescData2, h(DESC__PKT_WORD10_OFFSET):8
         < 2.   movifnz dcr, 1, DescData_0
         < 2.   movifnz dcr, 1, DescData_1

         < 2.   or  ETU_ExpectResults_0, 0xc0000000, ETU_Key
         < 2.   or  ETU_ExpectResults_1, 0xc0020000, ETU_Key
         < 2.   or  ETU_ExpectResults_2, 0xc0040000, ETU_Key
         < 2.   or  ETU_ExpectResults_3, 0xc0060000, ETU_Key
         < 2.   jeq STREAM11__ETU_MISCOMPARE_s1, ETU_ExpectResults_0, ETU_ActualResults_0
         < 2.   jeq STREAM11__ETU_MISCOMPARE_s1, ETU_ExpectResults_1, ETU_ActualResults_1
         < 2.   jeq STREAM11__ETU_MISCOMPARE_s1, ETU_ExpectResults_2, ETU_ActualResults_2
         < 2.   jeq STREAM11__ETU_MISCOMPARE_s1, ETU_ExpectResults_3, ETU_ActualResults_3
         < 2.   or  IPV6_SK1_ExpectResults_0, 0x11100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_1, 0x22100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_2, 0x33100000, IPV6_SubKey1_127to120
         < 2.   or  IPV6_SK1_ExpectResults_3, 0x00400000, IPV6_SubKey1_127to120
         < 2.   mov IPV6_SK1_ExpectResults_0[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_1[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_2[19:8], IPV6_SubKey1_139to128
         < 2.   mov IPV6_SK1_ExpectResults_3[19:8], IPV6_SubKey1_139to128

         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_0, IPV6_SK0_ActualResults_0
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_1, IPV6_SK0_ActualResults_1
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_2, IPV6_SK0_ActualResults_2
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK0_ExpectResults_3, IPV6_SK0_ActualResults_3
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_0, IPV6_SK1_ActualResults_0
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_1, IPV6_SK1_ActualResults_1
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_2, IPV6_SK1_ActualResults_2
         < 2.   jne STREAM11__IPV6_MISCOMPARE_s1, IPV6_SK1_ExpectResults_3, IPV6_SK1_ActualResults_3
         < 2.   movifnz dcr, 1, ttDescData0_0
         < 2.   movifnz dcr, 1, ttDescData0_1
         < 2.   movifnz dcr, 1, ttDescData1_0
         < 2.   movifnz dcr, 1, ttDescData1_1
         < 2.   movifnz dcr, 1, ttDescData2_0
         < 2.   movifnz dcr, 1, ttDescData2_1
         < 2.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         < 2.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex

         < 2.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         < 2.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 0.   nop
         > 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 0.   nop
         < 2.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         < 2.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         < 2.   jne STREAM11__PORT_MISCOMPARE_s1, P2_ExpectResults_0,  P2_ActualResults_0
         < 2.   jne STREAM11__PORT_MISCOMPARE_s1, P2_ExpectResults_1,  P2_ActualResults_1

         < 2.   jne STREAM11__PORT_MISCOMPARE_s1, P1_ExpectResults_0,  P1_ActualResults_0
         < 2.   jne STREAM11__PORT_MISCOMPARE_s1, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         < 0.   nop
         < 0.   nop
         < 2.   mov ttDescData0_1, 0xffffffff
         < 2.   mov ttDescData0_0, 0xffffffff
         < 2.   hstore h(DESC__PKT_WORD4_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD6_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD8_OFFSET):8, ttDescData0
         < 2.   hstore h(DESC__PKT_WORD10_OFFSET):8, ttDescData0
         < 2.   jne STREAM11__PORT_MISCOMPARE_s1, P8_ExpectResults_0,  P8_ActualResults_0
         < 2.   jne STREAM11__PORT_MISCOMPARE_s1, P8_ExpectResults_1,  P8_ActualResults_1
         < 2.   j STREAM11__s2
         < 0.   nop

STREAM11__DM_MISCOMPARE_s1:
         < 2.   or ErrorMessage, DM_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM11__SEND_ERROR_MESSAGE_s1
STREAM11__RCE_MISCOMPARE_s1:
         < 2.   or ErrorMessage, RCE_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM11__SEND_ERROR_MESSAGE_s1
STREAM11__IPV6_MISCOMPARE_s1:
         < 2.   or ErrorMessage, IPV6_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM11__SEND_ERROR_MESSAGE_s1
STREAM11__ETU_MISCOMPARE_s1:
         < 2.   or ErrorMessage, ETU_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM11__SEND_ERROR_MESSAGE_s1
STREAM11__PORT_MISCOMPARE_s1:
         < 2.   or ErrorMessage, PORT_MISCOMPARE_FLAG, ErrorMessage
         < 2.   j STREAM11__SEND_ERROR_MESSAGE_s1
STREAM11__TSR_ERROR_s1:
         < 2.   mov ErrorMessage_SK0ErrCode, SK0ErrCode
         < 2.   mov ErrorMessage_SK1ErrCode, SK1ErrCode
         < 0.   nop
STREAM11__SEND_ERROR_MESSAGE_s1:
         < 2.   load BitBucket0, p3.128(ErrorMessage)
         < 2.   mov dcr, 1
         < 2.   j STREAM11__THE_END

STREAM11__s2:
         > 1.   mov P2_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P1_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   or  P0_EntryIndex, WDT_PORT_LRP_LOAD_ACCESS_BASE, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P4_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P5_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P6_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P7_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   mov P8_EntryIndex, rnr[dLRP_PORT_ACCESS_BIT_RANGE]
         > 1.   load P2_ActualResults, p2.6( P2_EntryIndex )
         > 1.   load P1_ActualResults, p1.6( P1_EntryIndex )
         > 1.   load P0_ActualResults, p0.6( P0_EntryIndex )
         > 1.   load P4_ActualResults, p4.6( P4_EntryIndex )
         > 1.   load P5_ActualResults, p5.6( P5_EntryIndex )
         > 1.   load P6_ActualResults, p6.6( P6_EntryIndex )
         > 1.   load P7_ActualResults, p7.6( P7_EntryIndex )
         > 1.   load P8_ActualResults, p8.6( P8_EntryIndex )

         > 0.   nop
         > 7.   switch
         > 1.   or  P2_ExpectResults_0,  0xee220000, P2_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P2_ExpectResults_0,  P2_ActualResults_0
         > 1.   or  P2_ExpectResults_1,  0xff220000, P2_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P2_ExpectResults_1,  P2_ActualResults_1
         > 1.   or  P1_ExpectResults_0,  0xee110000, P1_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P1_ExpectResults_0,  P1_ActualResults_0
         > 1.   or  P1_ExpectResults_1,  0xff110000, P1_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P1_ExpectResults_1,  P1_ActualResults_1
         > 1.   or  P0_ExpectResults_0,  0xee000000, P0_EntryIndex_11_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P0_ExpectResults_0,  P0_ActualResults_0
         > 1.   or  P0_ExpectResults_1,  0xff000000, P0_EntryIndex_11_0
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P0_ExpectResults_1,  P0_ActualResults_1
         > 1.   or  P4_ExpectResults_0,  0xee440000, P4_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P4_ExpectResults_0,  P4_ActualResults_0

         > 1.   or  P4_ExpectResults_1,  0xff440000, P4_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P4_ExpectResults_1,  P4_ActualResults_1
         > 1.   or  P5_ExpectResults_0,  0xee550000, P5_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P5_ExpectResults_0,  P5_ActualResults_0
         > 1.   or  P5_ExpectResults_1,  0xff550000, P5_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P5_ExpectResults_1,  P5_ActualResults_1
         > 1.   or  P6_ExpectResults_0,  0xee660000, P6_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P6_ExpectResults_0,  P6_ActualResults_0
         > 1.   or  P6_ExpectResults_1,  0xff660000, P6_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P6_ExpectResults_1,  P6_ActualResults_1
         > 1.   or  P7_ExpectResults_0,  0xee770000, P7_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P7_ExpectResults_0,  P7_ActualResults_0
         > 1.   or  P7_ExpectResults_1,  0xff770000, P7_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P7_ExpectResults_1,  P7_ActualResults_1
         > 1.   or  P8_ExpectResults_0,  0xee880000, P8_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P8_ExpectResults_0,  P8_ActualResults_0

         > 1.   or  P8_ExpectResults_1,  0xff880000, P8_EntryIndex
         > 1.   jne STREAM11__PORT_MISCOMPARE_s2, P8_ExpectResults_1,  P8_ActualResults_1
         > 0.   nop
         > 1.   mov ttDescData0_0, 0xffffffff
         > 1.   hstore h(DESC__PKT_WORD4_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD5_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD6_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD7_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD8_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD9_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD10_OFFSET):4, ttDescData0_0
         > 1.   hstore h(DESC__PKT_WORD11_OFFSET):4, ttDescData0_0
         > 1.   j STREAM11__THE_END
STREAM11__PORT_MISCOMPARE_s2:
         > 1.   or ErrorMessage, PORT_MISCOMPARE_FLAG, FlowID
         > 1.   load BitBucket0, p3.128(ErrorMessage)
         > 1.   mov dcr, 1

STREAM11__THE_END:
         > 0.   nop
   > (F0)? 1.   store cmgr.0(SK0_TotalCounterID), PACKET_SIZE
   > (F1)? 1.   store cmgr.0(SK1_TotalCounterID), PACKET_SIZE
   > (F2)? 1.   store cmgr.1(SK0_MatchCounterID), PACKET_SIZE
   > (F3)? 1.   store cmgr.1(SK0_MissCounterID), PACKET_SIZE
   > (F4)? 1.   store cmgr.1(SK1_MatchCounterID), PACKET_SIZE
   > (F5)? 1.   store cmgr.1(SK1_MissCounterID), PACKET_SIZE
         < 0.   nop


