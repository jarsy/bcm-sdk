; $Id: sdk88030geup0.asm,v 1.1 Broadcom SDK $
; $Copyright: (c) 2016 Broadcom.
; Broadcom Proprietary and Confidential. All rights reserved.$
; 
;
;
; This is a sample LED program for the BCM88030 48G reference system: LEDProc0
;
; In 48G+LEDProc0:
; There are 100 LEDs for 50 GE  ports, one for link and one for link&activity
;	(the schematics says "Link/Act" & "Error", but that is going to be boring ...)
;
; Each ports is represented by two bits
; Per port:	
;     Bit (0):  0=No link,  1=Link 
;     Bit (1):  Link + Activity 
;
; Total bits for ports = 2 bit for 50 ports  = 100 bits.
;
;
; Link up/down info cannot be derived from LINKEN or LINKUP, as the LED
; processor does not always have access to link status.  This program
; assumes link status is kept current in bit 0 of RAM byte (0x80 + portnum).
; Generally, a program running on the main CPU must update these
; locations on link change; see linkscan callback in
; $SDK/src/appl/diag/sbx/ledproc.c.
;
;
;-------------------------- start of program --------------------------
MIN_GE_PORT_0       EQU      1
MAX_GE_PORT_0       EQU     50

;
; Main Update Routine
;
;  This routine is called once per tick.
;

update:
; ----------------------------------------
; Deal with GE ports
; ----------------------------------------
 	ld      a, MAX_GE_PORT_0
        ld      (PORT_NUM),a

upxe1:
        ld      a,(PORT_NUM)
        sub     a,1
        ld      (PORT_NUM),a
        port    a

; link_status
;
;  This subroutine calculates the link status LED for the current port.

link_status:
        ld      a,(PORT_NUM)    ; Check for link down
        ld      b,PORTDATA
        add     b,a
        ld      b,(b)
        tst     b,0
        push    CY
	pack	

; activity
;
;  This subroutine calculates the activity LED for the current port.

activity:
        ld      a,(PORT_NUM)
        ld      b,PORTDATA
        add     b,a
        ld      b,(b)
        tst     b,0
        push    CY
	jnt     wrapup          ;if Link down

; if Link up
        PORT    a
	pushst  RX
	pushst  TX
	tor
        jnt     wrapup          ; Always on if Link up, no Activity               

; Led blinks
        ld      b, (TXRX_ALT_COUNT)
        tst     b,3
        push    CY

wrapup:
        pack


; Judge for next loop
nextloop:             
        ld      a,(PORT_NUM)
        cmp     a,MIN_GE_PORT_0
        jnc     upxe1
        
; Update various timers
up5:
        inc     (TXRX_ALT_COUNT)
        send    100              


;
; Variables (SDK software initializes LED memory from 0x80-0xff to 0)
;

TXRX_ALT_COUNT  equ     0xe0
PORT_NUM        equ     0xe3

;
; Port data, which must be updated continually by main CPU's
; linkscan task.  See $SDK/src/appl/diag/ledproc.c for examples.
; In this program, bit 0 is assumed to contain the link up/down status.
;

PORTDATA        equ     0xa0    

;
; Symbolic names for the bits of the port status fields
;

RX              equ     0x0     ; received packet
TX              equ     0x1     ; transmitted packet
COLL            equ     0x2     ; collision indicator
SPEED_C         equ     0x3     ; 100 Mbps
SPEED_M         equ     0x4     ; 1000 Mbps
DUPLEX          equ     0x5     ; half/full duplex
FLOW            equ     0x6     ; flow control capable
LINKUP          equ     0x7     ; link down/up status
LINKEN          equ     0x8     ; link disabled/enabled status
ZERO            equ     0xE     ; always 0
ONE             equ     0xF     ; always 1

;        
;-------------------------- end of program --------------------------
;

