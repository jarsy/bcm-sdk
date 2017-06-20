/******************************************************************************
 * $Id: Bsc.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 ******************************************************************************/

/*********************************************************************
 Beg of File: Bsc.h
*********************************************************************/

#ifndef Bsc__h_defined
#define Bsc__h_defined

#include <soc/sbx/fe2k_common/defs_vn.h>

/* data types and functions for the emulation of the bit stream */

/* enumerated type for the control of the bit position move */
typedef enum {eBscBeg,eBscCur,eBscEnd} BscRel; 

/* bit stream structure / pseudo-class */
typedef struct Bsc__ 
{
   unsigned char 
       *mp_pduO;      /*pointer to the beginning of the PDU*/
   unsigned int
       m_lsbf     :1 /*less significant bit first flag*/
      ,m_lsof     :1 /*less significant octet first flag*/
      ,m_noc      :1 /*no buffer use, only length moving (for put only)*/
      ,m_err      :1 /*fatal error*/
      ,m_pend     :1 /*premature end of packet if 1*/
      ;
   unsigned int 
       m_shiftPduB :3 /*shift of the first bit of first octet of the pdu (bits)*/
      ,m_shiftCurB:3 /*shift of the dcurrent bit in the current octet in the current PDU*/
      ,m_lenPduB  :32/*length of the PDU (bits)*/
      ,m_offPduO  :26/*offset of the first octet of the current PDU relating to the beginning of the protocol of topmost level*/
      ,m_indCurO  :27/*index (offset) of the current octet in the pdu*/
      ;/* size = 32+5+3+3+32+26+27 = 128 bits = 16 octets   */
} Bsc;

   /*constructors, destructors and initializers*/
void          Bsc__init           (Bsc *pd);
void          Bsc__initFromBuf    (Bsc *pd, unsigned char *pp, unsigned int lb); /*initialize bit stream and attach it to the data buffer pp with length l*/ 
void          Bsc__initFromSuper  (Bsc *pd, Bsc *ps);        /*initialize destination bit stream from source so beginning of the new stream is at the current position of the source stream*/
void          Bsc__initFromSuperLen(Bsc *pd, Bsc *ps, unsigned int lb);/*initialize destination bit stream from source so beginning of the new stream is at the current position of the source stream and lengtrh is l*/
void          Bsc__initCopy       (Bsc *pd, Bsc *ps); /*initialize copy of the bit stream (buffers are not initialized and copied,just pointers, indexes and flags initialized)*/

   /*functions for the access to the actual and virtual parameters of the bit stream*/
BOOL_VN        Bsc__getErr     (Bsc *pd);   /*return value of the error flag*/
BOOL_VN        Bsc__setErr     (Bsc *pd, BOOL_VN  v); /*set error flag*/
unsigned char *Bsc__ptrPduO   (Bsc *pd);   /*return pointer to the first octet of the packet*/
unsigned char *Bsc__ptrCurO   (Bsc *pd);   /*return pointer to the current octet of the packet*/
unsigned int  Bsc__indCurO    (Bsc *pd);   /*return index of the current octet of the packet*/
unsigned int  Bsc__indCurB    (Bsc *pd);   /*return index of the current bit of the packet (number of the bits processed befor current)*/
unsigned int  Bsc__numProO    (Bsc *pd);   /*number of fully and partially processed octets*/
unsigned int  Bsc__numProB    (Bsc *pd);   /*number of processed bits*/
unsigned int  Bsc__lenCurB    (Bsc *pd);   /*return number of bits left unprocessed until the end of packet*/
void          Bsc__setCurLenB (Bsc *pd, unsigned int  l); /*set length of the packet left till the end of the packet in bits*/
unsigned int  Bsc__lenPduB    (Bsc *pd);   /*return number of bits in the current packet*/
unsigned int  Bsc__lenPduO    (Bsc *pd);   /*return number of the octets in the packet*/
unsigned int  Bsc__lenCurO    (Bsc *pd);   /*return number of octet left unprocessed until the end of the packet*/
void          Bsc__setCurLenO (Bsc *pd, unsigned int  l); /*set length of the packet left till the end of the packet in octets*/
unsigned int  Bsc__offCurO    (Bsc *pd);   /*return offset of the octet with the current bit from the beginning of the protocol*/
unsigned int  Bsc__offPduO    (Bsc *pd);   /*return offset of the octet packet beginning from the beginning of the protocol*/
unsigned int  Bsc__shiftPduB  (Bsc *pd);   /*return shift of the first bit of the packet in first octet of the packet*/
unsigned int  Bsc__shiftCurB  (Bsc *pd);   /*return shift of the current bit in the current octet of the packet*/
BOOL_VN        Bsc__lsbf       (Bsc *pd);   /*return value of the lsbf flag*/
BOOL_VN        Bsc__setLsbf    (Bsc *pd, BOOL_VN  v);/*set lsbf flag*/
BOOL_VN        Bsc__lsof       (Bsc *pd);   /*return value of the lsof flag*/
BOOL_VN        Bsc__setLsof    (Bsc *pd, BOOL_VN  v);/*set lsof flag*/

   /*bit stream navigation functions*/
void          Bsc__clear          (Bsc *pd);   /*initialize bit stream to the beginning*/ 
int           Bsc__moveB          (Bsc *pd, int v, BscRel r);/*moves internal indices to the number of bits v relative to the beginning, end or current position according to the value of the r*/
unsigned int  Bsc__aligningB      (Bsc *pd, unsigned int v);         /*return number of bits left to the boundary of the alligning on the value v*/
   
   /*bit stream manipulation functions*/
void          Bsc__getUnsignedB   (Bsc *pd, unsigned int *pv, unsigned int l); /*decode unsigned bit field length l to the *pv from the bit stream*/
void          Bsc__putUnsignedB   (Bsc *pd, unsigned int *pv, unsigned int l); /*encode value *pv to the unsigned bit field length l with the to the bit stream*/ 
unsigned int  Bsc__getUnsigned    (Bsc *pd, int o, unsigned int l);   /*decode value of the unsigned bit field length l at the offset from the current position o according to the bit flow flags, return field value*/
unsigned int  Bsc__putUnsigned    (Bsc *pd, unsigned int v,unsigned int l);    /*encode value v   to the unsigned bit field length l without moving of pointer*/
unsigned int  Bsc__getOctetStr    (Bsc *pd, unsigned char *pv, unsigned int lo); /*decode length terminated octet string from the bit stream*/
unsigned int  Bsc__putOctetStr    (Bsc *pd, unsigned char *pv, unsigned int lo); /*encode length terminated octet string to the bit stream*/
unsigned int  Bsc__getTermOctetStr(Bsc *pd, unsigned char *pv, unsigned int lo, int t);/*decode terminated octet string length l with terminator t from the bit stream to the buffer pv*/
unsigned int  Bsc__putTermOctetStr(Bsc *pd, unsigned char *pv, unsigned int lo, int t);/*encode terminated octet string with terminator t and length l from the buffer pv*/

#endif /*Bsc__h_defined*/

/*********************************************************************
 End of File: Bsc.h
*********************************************************************/

