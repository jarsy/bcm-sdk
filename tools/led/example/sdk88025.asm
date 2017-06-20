; $Id: sdk88025.asm,v 1.6 Broadcom SDK $
;
;
; $Id: sdk88025.asm,v 1.6 Broadcom SDK $
; $Copyright: (c) 2016 Broadcom.
; Broadcom Proprietary and Confidential. All rights reserved.$
; 
;
;
; This is a sample LED program for the BCM88020 reference system
;
; There are LEDs for 12 GE SFP ports, one for link and one for link&activity
;	(the schematics says "Link/Act" & "Error", but that is going to be boring ...)
; There is also LEDs for the 10G ports, I have disabled those for now
;
; Each ports is represented by two bits
; Per port:	
;	Bit (0):  0=No link,  1=Link 
;       Bit (1):  Link + Activity 
;
; Total for 12 ports = 24 bit + 4bits for 10G ports = 8 bits.
;
; The port ordering is a bit funky, so we deal with ports 23, 21, 19, 17, 15 & 13 first
; then we deal with                                 port  22, 20, 18, 16, 14 & 12
	;
; Link up/down info cannot be derived from LINKEN or LINKUP, as the LED
; processor does not always have access to link status.  This program
; assumes link status is kept current in bit 0 of RAM byte (0x80 + portnum).
; Generally, a program running on the main CPU must update these
; locations on link change; see linkscan callback in
; $SDK/src/appl/diag/sbx/ledproc.c.
;
;
;

MIN_GE_PORT_1     EQU     13
MAX_GE_PORT_1     EQU     23

MIN_GE_PORT_2     EQU     12
MAX_GE_PORT_2     EQU     22


MIN_XE_PORT       EQU     24
MAX_XE_PORT       EQU     25
;
; Main Update Routine
;
;  This routine is called once per tick.
;

update:
	; ----------------------------------------
	; Deal with 10G ports
	; ----------------------------------------
 	ld      a, MAX_XE_PORT

upxe1:
        port    a
        ld      (PORT_NUM),a

upxe2:			
  	call    activity        ; Activity LED for this port
	call    link_status     ; Link status LED for this port

        
        ld      a,(PORT_NUM)
        sub     a,1
        ld      (PORT_NUM),a

        cmp     a,MIN_XE_PORT
        jnc     upxe1

	; ----------------------------------------	
	; Deal with odd port numbers
	; ----------------------------------------
 	ld      a, MAX_GE_PORT_1
up1:
        port    a
        ld      (PORT_NUM),a

up2:			
  	call    link_status     ; Link status LED for this port
	call    activity        ; Activity LED for this port

        
        ld      a,(PORT_NUM)
        sub     a,2
        ld      (PORT_NUM),a

        cmp     a,MIN_GE_PORT_1
        jnc     up1

	; ----------------------------------------
	; Now deal with even port numbers
	; ----------------------------------------
	ld      a, MAX_GE_PORT_2
up3:
        port    a
        ld      (PORT_NUM),a

up4:
  	call    activity        ; Activity LED for this port
  	call    link_status     ; Link status LED for this port
        
        ld      a,(PORT_NUM)
        sub     a,2
        ld      (PORT_NUM),a

        cmp     a,MIN_GE_PORT_2
        jnc     up3

up5:
	; Update various timers
        inc     (TXRX_ALT_COUNT)	
        send    28



; link_status
;
;  This routine calculates the link status LED for the current port.
;
;  Inputs: (PORT_NUM)
;  Outputs: One bit sent to LED stream
;  Destroys: a, b
;

link_status:

ls1:
        ld      a,(PORT_NUM)    ; Check for link down
        call    get_link
        jnc     led_off         ; no link

	jmp	led_on	

;
; activity
;
;  This routine calculates the activity LED for the current port.
;
;  Inputs: Port number
;  Outputs: One bit sent to LED stream
;

activity:
	ld      a, (PORT_NUM)
	call	get_link
	jnc	led_off

	pushst  RX
	pushst  TX
	tor
        pop
        jnc     led_on                ; Always on if Link up, no Activity

        ld      b, (TXRX_ALT_COUNT)
        and     b, 8
        jnz     led_on
        jmp     led_off


;
; get_link
;
;  This routine finds the link status LED for a port.
;  Link info is in bit 0 of the byte read from PORTDATA[port]
;
;  Inputs: Port number in a
;  Outputs: Carry flag set if link is up, clear if link is down.
;  Destroys: a, b
;

get_link:
        ld      b,PORTDATA
        add     b,a
        ld      b,(b)
        tst     b,0
        ret
	

;
; led routines
;
;  Inputs: None
;  Outputs: One bit to the LED stream
;  Destroys: None
;

led_on:
	pushst  ZERO
	pack
	ret

led_off:
	pushst  ONE
	pack
	ret
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

PORTDATA        equ     0x80    ; Size 24 + 4? bytes

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
