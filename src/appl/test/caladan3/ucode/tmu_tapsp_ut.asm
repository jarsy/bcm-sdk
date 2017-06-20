;;;
;;;   $Id: tmu_tapsp_ut.asm,v 1.3 Broadcom SDK $
;;;
;;; $Copyright: (c) 2016 Broadcom.
;;; Broadcom Proprietary and Confidential. All rights reserved.$   
;;;   Internal -- latency 19 cycles, shadow 48 cycles
;;;   External -- latency 26 cycles, shadow 48 cycles
;;;
;;; --------------------------------------------------------------------------

        const TSR_ERROR_FLAG    = 0x40000000 
        reg   dcr = tsr[27:24]
        reg   ValidContext = tsr[0]
        reg   tsr_ErrorFlag = tsr[10]
        reg   ErrorMessage = r24

        reg   IPV4_SK0_ActualResults_0 = c20
        reg   IPV4_SK0_ActualResults_1 = c21
        reg   IPV4_SK0_ActualResults_2 = c22
        reg   IPV4_SK0_ActualResults_3 = c23

        reg   IPV4_SK1_ActualResults_0 = c16
        reg   IPV4_SK1_ActualResults_1 = c17
        reg   IPV4_SK1_ActualResults_2 = c18
        reg   IPV4_SK1_ActualResults_3 = c19
       
        reg   IPV4_SubKey0_31to0 = r0
        reg   IPV4_SubKey0_63to32 = r1
        reg   IPV4_SubKey0_95to64 = r2
        reg   IPV4_SubKey0_127to96 = r3
        reg   IPV4_SubKey0_159to128 = r36
        reg   IPV4_SubKey0_191to160 = r37
        reg   IPV4_SubKey0_223to192 = r38
        reg   IPV4_SubKey0_255to224 = r39
        reg   IPV4_SubKey0_287to256 = r40
        reg   IPV4_SubKey0_319to288 = r41
        reg   IPV4_SubKey0_351to320 = r42
        reg   IPV4_SubKey0_383to352 = r43
        reg   IPV4_SubKey0_415to384 = r44
        reg   IPV4_SubKey0_447to416 = r45
        reg   IPV4_SubKey0_479to448 = r46
        reg   IPV4_SubKey0_511to480 = r47


        reg   Key_63_0    = rr0
        reg   Key_127_64  = rr1
        reg   Key_191_128 = rr18
        reg   Key_255_192 = rr19
        reg   Key_319_256 = rr20
        reg   Key_383_320 = rr21
        reg   Key_447_384 = rr22
        reg   Key_511_448 = rr23

        reg   PEID = pir[6:0]

        stream	0:
        ;; execute only for PE id 0, buddy 0
         > 1. jnz THE_END, PEID
         > 1.   mov ValidContext, 1

; Initialize the key registers
         > 1.   clr Key_63_0[0]
         > 1.   clr Key_63_0[1]
         > 1.   clr Key_63_0[1]
         > 1.   clr Key_127_64[0]
         > 1.   clr Key_127_64[1]
         > 1.   clr Key_191_128[0]
         > 1.   clr Key_191_128[1]
         > 1.   clr Key_255_192[0]
         > 1.   clr Key_255_192[1]
         > 1.   clr Key_319_256[0]
         > 1.   clr Key_319_256[1]
         > 1.   clr Key_383_320[0]
         > 1.   clr Key_383_320[1]
         > 1.   clr Key_447_384[0]
         > 1.   clr Key_447_384[1]
         > 1.   clr Key_511_448[0]
         > 1.   clr Key_511_448[1]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Load in the values from Host
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

         > 1.   load IPV4_SubKey0_31to0,  p0.129(0)
         > 1.   load IPV4_SubKey0_63to32, p1.129(0)
         > 1.   load IPV4_SubKey0_95to64, p2.129(0)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Wait out the shadow for the port load
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
 ;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
 ;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Load the key in prep for the lookup
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

         > 1.   key 4.2, Key_511_448, Key_447_384
         > 1.   key 4.1, Key_383_320, Key_319_256, Key_255_192
         > 1.   key 4.0, Key_191_128, Key_127_64,  Key_63_0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Do the switch. Results will be available after we return
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

         > 7.   switch
   > (!E)? 1.   j NO_TSR_ERROR_s1
   >  (E)? 1.   mov tsr_ErrorFlag, 0
         > 1.   or ErrorMessage, TSR_ERROR_FLAG, ErrorMessage
         > 0.   nop
         > 1.   j  TSR_ERROR_s1
NO_TSR_ERROR_s1:
         > 1.   jz  THE_END, ValidContext

; Write the results from Taps instance 0 back for the host
	     > 1.   load  IPV4_SK0_ActualResults_0, p0.130(IPV4_SK0_ActualResults_0)
         > 1.   load  IPV4_SK0_ActualResults_1, p1.130(IPV4_SK0_ActualResults_1)
         > 1.   load  IPV4_SK0_ActualResults_2, p2.130(IPV4_SK0_ActualResults_2)
         > 1.   load  IPV4_SK0_ActualResults_3, p3.130(IPV4_SK0_ActualResults_3)


; Shadow nops before writing results for TAPS Instance 1 back to the host
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
 ;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
 ;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop

; Write results for taps instance 1 back to the host
         > 1.   load  IPV4_SK1_ActualResults_0, p0.132(IPV4_SK1_ActualResults_0)
         > 1.   load  IPV4_SK1_ActualResults_1, p1.132(IPV4_SK1_ActualResults_1)
         > 1.   load  IPV4_SK1_ActualResults_2, p2.132(IPV4_SK1_ActualResults_2)
         > 1.   load  IPV4_SK1_ActualResults_3, p3.132(IPV4_SK1_ActualResults_3)

         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
 ;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
 ;
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop
         > 0.   nop

         > 1.   j THE_END
TSR_ERROR_s1:
         > 1.   mov dcr, 1
THE_END:
         > 0.   nop

