; $Id: sdk88030geup1.asm,v 1.1 Broadcom SDK $
; $Copyright: (c) 2016 Broadcom.
; Broadcom Proprietary and Confidential. All rights reserved.$
; 
;
;
; This is a dummy LED program for the BCM88030 48G reference system: LEDProc1
;
; In 48G+LEDProc1:
; There are 32 LEDs for 16 GE SFP ports, one for link and one for link&activity
;	(the schematics says "Link/Act" & "Error", but that is going to be boring ...)
; There are also LEDs for Host GE ports; GE0 and GE1.
;
; Each ports is represented by two bits
; Per port:	
;     Bit (0):  0=No link,  1=Link 
;     Bit (1):  Link + Activity 
;
; Total bits for ports = 2 bit for 16 GE ports + 2 bit for Host GE Ports = 36 bits.
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
;
;        
;-------------------------- end of program --------------------------
;

