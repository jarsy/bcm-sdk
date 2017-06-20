/*
 * $Id: Bsc.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/*********************************************************************
 * Bsc.c                                                                     
 ********************************************************************/

#include <soc/sbx/caladan3/Bsc.h>

/* macro versions of the functions 
   #define Bsc__initCopy(pd,ps)	   (*(pd) = *(ps))
   #define Bsc__getErr(pd)		   ((pd)->m_err)
   #define Bsc__ptrPduO(pd)	   ((pd)->mp_pduO)
   #define Bsc__ptrCurO(pd)	   ((pd)->mp_pduO+(pd)->m_indCurO)
   #define Bsc__indCurO(pd)	   ((pd)->m_indCurO)
   #define Bsc__indCurB(pd)           (((pd)->m_indCurO<<3)+(pd)->m_shiftCurB-(pd)->m_shiftPduB)
   #define Bsc__numProO(pd)           (((pd)->m_indCurO + ((pd)->m_shiftCurB?1:0)))
   #define Bsc__numProB(pd)           (Bsc__indCurB(pd))
   #define Bsc__lenCurB(pd)	   ((pd)->m_lenPduB-Bsc__IndCurB(pd))
   #define Bsc__lenPduB(pd)	   ((pd)->m_lenPduB)
   #define Bsc__lenPduO(pd)           (((pd)->m_lenPduB+(pd)->m_shiftPduB+7)>>3)
   #define Bsc__lenCurO(pd)           (Bsc__lenPduO(pd)-(pd)->m_indCurO)
   #define Bsc__offCurO(pd)	   ((pd)->m_offPduO+(pd)->m_indCurO)
   #define Bsc__offPduO(pd)	   ((pd)->m_offPduO)
   #define Bsc__clear(pd)             (Bsc__moveB(pd, 0, eBscBeg), (pd)->m_err = NTFALSE)
   #define Bsc__getPend(pd)	   ((pd)->m_pend)
   #define Bsc__setPend(pd,v)	   ((pd)->m_pend=(v))
   #define Bsc__setCurLenB(pd,l)	   ((pd)->m_lenPduB=Bsc__indCurB(pd)+MIN((l),Bsc__lenCurB(pd)))
   #define Bsc__setCurLenO(pd,l)	   (Bsc__setCurLenB(pd, (l)<<3))
   #define Bsc__setErr(pd,v)	   ((pd)->m_err=(v))
   #define Bsc__getSrc(pd)		   ((pd)->m_src)
   #define Bsc__setSrc(pd,v)	   ((pd)->m_src = v)
   #define Bsc__getDst(pd)		   ((pd)->m_dst)
   #define Bsc__setDst(pd,v)	   ((pd)->m_dst = v)
   #define Bsc__lenReB(pd,e)	   ((Bsc__lenReO(pd, e)<<3))         
   #define Bsc__lenTeB(pd,e)	   ((Bsc__lenTeO(pd, e)<<3)) 
   #define Bsc__lenReO(pd,e)	   (LenRe((w1*)Bsc__ptrCurO(pd), e))
   #define Bsc__getOctetStr(pd,v,l)   (Bsc__getTermOctetStr(pd, v,l,-1))
   #define Bsc__putOctetStr(pd,v,l)   (Bsc__putTermOctetStr(pd, v,l,-1))
   #define Bsc__shiftPduB(pd)         ((pd)->m_shiftPduB)
   #define Bsc__shiftCurB(pd)         ((pd)->m_shiftCurB)
   #define Bsc__getUnsignedB(pd,pv,l) (*(pv) = Bsc__getUnsigned((pd), 0, (l)), Bsc__moveB((pd), (l), eBscCur))
   #define Bsc__putUnsignedB(pd,pv,l) (Bsc__putUnsigned((pd), *(pv), (l)), Bsc__moveB((pd), (l), eBscCur))
   #define Bsc__setLsof(pd,v)	   ((pd)->m_lsof=(v))
   #define Bsc__lsof(pd)		   ((pd)->m_lsof)
   #define Bsc__setLsbf(pd,v)	   ((pd)->m_lsbf=(v))
   #define Bsc__lsbf(pd)		   ((pd)->m_lsbf)
*/

/********************************************************************
 * constructors, destructors and initializers
 ********************************************************************/

/*initialize empty*/
void Bsc__init(Bsc* pd)
{
  pd->mp_pduO = 0;
  pd->m_lsbf  = TRUE_VN;
  pd->m_lsof  = TRUE_VN;
  pd->m_noc   = FALSE_VN;
  pd->m_err   = FALSE_VN;
  pd->m_pend  = FALSE_VN;

  pd->m_shiftPduB = 0;
  pd->m_shiftCurB = 0;
  pd->m_lenPduB   = 0;
  pd->m_offPduO   = 0;
  pd->m_indCurO   = 0;
}

/*initialize bit stream and attach it to the data buffer pp with length l*/ 
void Bsc__initFromBuf(Bsc* pd, unsigned char  *pp, unsigned int lb) 
{
  Bsc__init(pd);
  pd->mp_pduO     = pp;
  pd->m_lenPduB   = lb;

  if (pp==0) pd->m_noc=TRUE_VN;
}

/*initialize destination bit stream from source so beginning of the new stream is at the current position of the source stream*/
void Bsc__initFromSuper(Bsc* pd, Bsc *ps) 
{
  Bsc__initCopy(pd, ps);
  pd->mp_pduO     = Bsc__ptrCurO(ps);
  pd->m_shiftPduB = Bsc__shiftCurB(ps);
  pd->m_lenPduB   = Bsc__lenCurB(ps);
  pd->m_offPduO   = Bsc__offCurO(ps);
  pd->m_indCurO   = 0;
}

/*initialize destination bit stream from source so beginning of the new stream is at the current position of the source stream and lengtrh is l*/
void Bsc__initFromSuperLen(Bsc* pd, Bsc *ps, unsigned int lb) 
{
  Bsc__initFromSuper(pd, ps);
  pd->m_lenPduB = MIN_VN(lb, Bsc__lenCurB(ps));
}



/*initialize copy of the bit stream (buffers are not initialized and copied,just pointers, indexes and flags initialized)*/
void Bsc__initCopy       (Bsc *pd, Bsc *ps) 
{
  *(pd) = *(ps);
}

/********************************************************************
 * functions for the access to the actual and virtual parameters of the bit stream
 ********************************************************************/

/*return value of the error flag*/
BOOL_VN Bsc__getErr         (Bsc *pd)
{  
  return pd->m_err;
}

/*set error flag*/
BOOL_VN Bsc__setErr         (Bsc *pd, BOOL_VN  v)
{ 
  pd->m_err=v;
  return v;
}

/*return pointer to the first octet of the packet*/
unsigned char *Bsc__ptrPduO       (Bsc *pd)
{   
  return pd->mp_pduO;
}

/*return pointer to the current octet of the packet*/
unsigned char *Bsc__ptrCurO       (Bsc *pd)
{   
  return pd->mp_pduO+pd->m_indCurO;
}

/*return index of the current octet of the packet*/
unsigned int Bsc__indCurO        (Bsc *pd)
{   
  return pd->m_indCurO;
}

/*return index of the current bit of the packet (number of the bits processed befor current)*/
unsigned int Bsc__indCurB        (Bsc *pd)
{   
  return (((pd)->m_indCurO<<3)+(pd)->m_shiftCurB-(pd)->m_shiftPduB);
}

/*number of fully and partially processed octets*/
unsigned int Bsc__numProO        (Bsc *pd)
{   
  return (((pd)->m_indCurO + ((pd)->m_shiftCurB?1:0)));
}

/*number of processed bits*/
unsigned int Bsc__numProB        (Bsc *pd)
{   
  return (Bsc__indCurB(pd));
}

/*return number of bits left unprocessed until the end of packet*/
unsigned int Bsc__lenCurB        (Bsc *pd)
{   
  return ((pd)->m_lenPduB-Bsc__indCurB(pd));
}

/*set length of the packet left till the end of the packet in bits*/
void Bsc__setCurLenB     (Bsc *pd, unsigned int  l)
{ 
  ((pd)->m_lenPduB=Bsc__indCurB(pd)+MIN_VN((l),Bsc__lenCurB(pd)));
}

/*return number of bits in the current packet*/
unsigned int Bsc__lenPduB        (Bsc *pd)
{   
  return ((pd)->m_lenPduB);
}

/*return number of the octets in the packet*/
unsigned int Bsc__lenPduO        (Bsc *pd)
{   
  return (((pd)->m_lenPduB+(pd)->m_shiftPduB+7)>>3);
}

/*return number of octet left unprocessed until the end of the packet*/
unsigned int Bsc__lenCurO        (Bsc *pd) 
{ 
  return (Bsc__lenPduO(pd)-(pd)->m_indCurO); 
}

/*set length of the packet left till the end of the packet in octets*/
void Bsc__setCurLenO     (Bsc *pd, unsigned int  l) 
{
  Bsc__setCurLenB(pd, (l)<<3);
}

/*return offset of the octet with the current bit from the beginning of the protocol*/
unsigned int Bsc__offCurO        (Bsc *pd)
{   
  return ((pd)->m_offPduO+(pd)->m_indCurO);
}

/*return offset of the octet packet beginning from the beginning of the protocol*/
unsigned int Bsc__offPduO        (Bsc *pd)
{   
  return ((pd)->m_offPduO);
}

/*return shift of the first bit of the packet in first octet of the packet*/
unsigned int Bsc__shiftPduB      (Bsc *pd)
{   
  return ((pd)->m_shiftPduB);
}

/*return shift of the current bit in the current octet of the packet*/
unsigned int Bsc__shiftCurB      (Bsc *pd)
{  
  return ((pd)->m_shiftCurB);
}

/*return value of the lsbf flag*/
BOOL_VN Bsc__lsbf           (Bsc *pd)
{   
  return ((pd)->m_lsbf);
}

/*set lsbf flag*/
BOOL_VN Bsc__setLsbf        (Bsc *pd, BOOL_VN  v)
{
  pd->m_lsbf=v;
  return v;
}

/*return value of the lsof flag*/
BOOL_VN Bsc__lsof           (Bsc *pd)
{   
  return ((pd)->m_lsof);
}

/*set lsof flag*/
BOOL_VN Bsc__setLsof        (Bsc *pd, BOOL_VN  v)
{
  pd->m_lsof=v;
  return v;
}

/********************************************************************
 * functions for the bit stream navigation
 ********************************************************************/

/*initialize bit stream to the beginning*/
void Bsc__clear          (Bsc *pd)
{    
  Bsc__moveB(pd, 0, eBscBeg);
  pd->m_err = FALSE_VN;
}

/*moves internal indices to the number of bits v relative to the beginning, end or current position according to the value of the r*/
int Bsc__moveB(Bsc* pd, int v, BscRel r)
{
  int apozB = 0, /* relative move adjusted to fit PDU 					*/
    bpozB = 0, /* new absolute position relatively to the beginning	*/
    abp   = 0;

  if(r==eBscCur && v==0) return 0; /* to make function faster			*/

  if (v>0) switch(r) {
  case eBscBeg: bpozB = apozB = MIN_VN(v, (int)Bsc__lenPduB(pd)); break;    
  case eBscCur: bpozB = (int)Bsc__indCurB(pd) + (apozB = MIN_VN(v, (int)Bsc__lenCurB(pd))); break;   
  case eBscEnd: bpozB = apozB = 0;
  } else switch(r) {
  case eBscBeg: bpozB = apozB = 0; break;    
  case eBscCur: bpozB = (int)Bsc__indCurB(pd) + (apozB = -MIN_VN(-v, (int)Bsc__indCurB(pd))); break;   
  case eBscEnd: bpozB = (int)Bsc__lenPduB(pd) + (apozB = -MIN_VN(-v, (int)Bsc__lenCurB(pd))); break;
  }

  abp = bpozB + (int)Bsc__shiftPduB(pd);
  pd->m_indCurO   = abp>>3; 
  pd->m_shiftCurB = abp&7; 
   
  return apozB;   
}

/*return number of bits left to the boundary of the alligning on the value v*/
unsigned int Bsc__aligningB      (Bsc *pd, unsigned int v) 
{
  return (v - Bsc__numProB(pd) % v);
}
   
/********************************************************************
 * functions for the bit stream manipulation
 ********************************************************************/
   
/*decode unsigned bit field length l to the *pv from the bit stream*/
void Bsc__getUnsignedB   (Bsc *pd, unsigned int *pv, unsigned int l)
{
  if (!pd->m_noc) *(pv) = Bsc__getUnsigned((pd), 0, (l));
  Bsc__moveB(pd, l, eBscCur);
}

/*encode unsigned bit field length l with the value *pv to the bit stream*/
void Bsc__putUnsignedB   (Bsc *pd, unsigned int *pv, unsigned int l)
{  
  if (!pd->m_noc) Bsc__putUnsigned((pd), *(pv), (l));
  Bsc__moveB((pd), (l), eBscCur);
}


/*decode value of the unsigned bit field length l at the offset from the current position o according to the bit flow flags, return field value*/
unsigned int Bsc__getUnsigned (Bsc* pd, int o, unsigned int l) /*does not move pointer in the Bsc*/
{
  unsigned char *fptr; 
  static const unsigned int fmas[9]  =  { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00};
  static const unsigned int nfmas[9]  = {(unsigned int)~0xff,(unsigned int)~0x7f,(unsigned int)~0x3f,(unsigned int)~0x1f,
					 (unsigned int)~0x0f,(unsigned int)~0x07,(unsigned int)~0x03,(unsigned int)~0x01,(unsigned int)~0x00};
  unsigned int len     = l
    ,val     = 0 
    ,curval  = 0
    ,shift
    ,lbitlen    /* length in bits from the beginning of the current octet to the end of field	*/
    ,bitpad     /* length in bits of the rest of last octet after field end                  	*/
    ,fo_num     /* full number of octets occupied by current field                           	*/
    ,ls      = 0/* left shift of value for the current octet in the field  	*/
    ,rs      = 0/* right shift of value for the current octet in the field 	*/
    ,i       = 0/* number of current octet in the field                    	*/
    ;
   
  if (pd->m_noc) { return 0; } /*  If we are just calculating the length ...	*/
  if (Bsc__lenCurB(pd) < l+o) { Bsc__setErr(pd, TRUE_VN); return 0; }
   
  Bsc__moveB(pd, o, eBscCur);

  fptr    = Bsc__ptrCurO(pd); 
  shift   = Bsc__shiftCurB(pd);
  lbitlen = shift+len;          /* length in bits from the beginning of the current octet to the end of field	*/
  bitpad  = (8-(lbitlen&7))&7;  /* length in bits of the rest of last octet after field end                  	*/
  fo_num  = (lbitlen+7)>>3;     /* full number of octets occupied by current field                           	*/

   
  /* |8|7|6|5|4|3|2|1| <- (traffic goes in left direction) 	*/
  if(Bsc__lsbf(pd)) { 
    /* for the short fields witch fit to the one current octet (for better timing)	*/
    if(lbitlen <= 8)  val =  (((unsigned int)*fptr) >> shift) & fmas[8 - len];
    /* for the fields witch do not fit to the one current octet	*/
    else if(Bsc__lsof(pd)) { /* --------------- lsbf, lsof	*/
      ls = 0;
      rs = shift;
      for(i = 0; i < fo_num; i++) {
	/* take curval	*/
	curval = (unsigned int)(fptr[i]);
            
	/* clean unused bits with mask if necessary	*/
	if(i == 0 && shift != 0) curval &= nfmas[8-shift]; /*first octet*/
	if(i == (fo_num - 1) && bitpad != 0) curval &= fmas[bitpad]; /*last octet*/
            
	/* move curval to the appropriate position	*/
	if(ls > rs) curval <<= (ls - rs); else curval >>= (rs - ls);
            
	/* update value	*/
	val |= curval;
            
	/* update ls and rs as appropriate	*/
	ls += 8;
      }
    }
    else {/* ------------------------------------------- lsbf,lsol	*/
      ls = (fo_num-1)<<3;
      rs = shift + bitpad;
      for(i = 0; i < fo_num; i++) {
	/* take curval	*/
	curval = (unsigned int)(fptr[i]);
            
	/* clean unused bits with mask if necessary	*/
	if(i == 0 && shift != 0) curval &= nfmas[8-shift]; /* first octet*/
	if(i == (fo_num - 1) && bitpad != 0) curval &= fmas[bitpad]; /* last octet*/
            
	/* correction of the rs for the last octet	*/
	if(i == fo_num - 1) rs = 0;
            
	/* move curval to the appropriate position	*/
	if(ls > rs) curval <<= (ls - rs); else curval >>= (rs - ls);
            
	/* update value	*/
	val |= curval;
            
	/* update ls and rs as appropriate	*/
	ls -= 8;
	rs = bitpad;
      }
    }
  }
  /* (traffic goes in right direction) -> |8|7|6|5|4|3|2|1| 	*/
  else {
    /* for the short fields that fit in the one current octet (for improving timing)	*/
    if(lbitlen < 8) val =  ((((unsigned int)*fptr) >> (8 - lbitlen)) & fmas[8 - len]);
    /* for the fields witch do not fit to the one current octet	*/
    else if(Bsc__lsof(pd)) {/* ---------------- lsbl, lsof	*/
      ls = 0;
      rs = 0;
      for(i = 0; i < fo_num; i++) {
	/* take curval	*/
	curval = (unsigned int)(fptr[i]);
            
	/* clean unused bits with mask if necessary	*/
	if(i == 0 && shift != 0) curval &= fmas[shift]; /* first octet	*/
	if(i == (fo_num - 1) && bitpad != 0) curval &= nfmas[8 - bitpad]; /* last octet	*/
            
	/* correction of the rs for the last octet	*/
	if(i == fo_num - 1) rs = shift + bitpad;
            
	/* move curval to the appropriate position	*/
	if(ls > rs) curval <<= (ls - rs); else curval >>= (rs - ls);
            
	/* update value	*/
	val |= curval;
            
	/* update ls and rs as appropriate	*/
	ls += 8;
	rs = shift;
      }
    }
    else {/* ------------------------------------------- lsbl, lsol	*/
      ls = (fo_num - 1) << 3;
      rs = bitpad;
      for(i = 0; i < fo_num; i++) {
	/* take curval	*/
	curval = (unsigned int)(fptr[i]);
            
	/* clean unused bits with mask if necessary	*/
	if(i == 0 && shift != 0) curval &= fmas[shift]; /* first octet	*/
	if(i == (fo_num - 1) && bitpad != 0) curval &= nfmas[8 - bitpad]; /* last octet	*/
            
	/* move curval to the appropriate position	*/
	if(ls > rs) curval <<= (ls - rs); else curval >>= (rs - ls);
            
	/* update value	*/
	val |= curval;
            
	/* update ls and rs as appropriate	*/
	ls -= 8;
      }
    }
  }

  Bsc__moveB(pd, -o, eBscCur);

  return val;
}

/*encode value v of the unsigned bit field length l to the bit stream without moving of pointer*/
unsigned int Bsc__putUnsigned (Bsc* pd, unsigned int v, unsigned int l) /*does not move pointer in the Bsc*/
{
  static const unsigned char fmas[9]  = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00};
  static const unsigned char nfmas[9] = {(unsigned char)~0xff,(unsigned char)~0x7f,(unsigned char)~0x3f,(unsigned char)~0x1f,
					 (unsigned char)~0x0f,(unsigned char)~0x07,(unsigned char)~0x03,(unsigned char)~0x01,(unsigned char)~0x00};
  unsigned char *fptr
    ,mas     = 0
    ;
  unsigned int len      = l
    ,fs         /* first shift - shift of first bit of current field in the current octet			*/
    ,lblen      /* length in bits from the beginning of the current octet to the end of field	*/
    ,bitpad     /* length in bits of the rest of last octet after field end 	*/
    ,fo_num     /* full number of octets occupied by current field    	*/
    ,loind      /* index of last octet	*/
    ,val     = v/* value of the field to be set 	*/
    ,curval  = 0/* value to be set in the current octet	*/	
    ,ls      = 0/* left shift of value for the current octet in the field 	*/
    ,rs      = 0/* right shift of value for the current octet in the field	*/
    ,i       = 0/* number of current octet in the field 	*/
    ;
   
  if(pd->m_noc) return val; /*  for the length calculation we do not need to do anything	*/
   
  fptr    = Bsc__ptrCurO(pd);
  fs      = Bsc__shiftCurB(pd);     /* first shift - shift of first bit of current field in the current octet	*/
  lblen   = fs+len;          /* length in bits from the beginning of the current octet to the end of field	*/
  bitpad  = (8-(lblen&7))&7; /* length in bits of the rest of last octet after field end 	*/
  fo_num  = (lblen+7)>>3;    /* full number of octets occupied by current field    	*/
  loind   = fo_num-1;        /* index of last octet	*/
   
  if (Bsc__lenCurB(pd) < l) { Bsc__setErr(pd, TRUE_VN); return 0; }

  if(Bsc__lsbf(pd)) /* |8|7|6|5|4|3|2|1| <- (traffic goes in left direction) 	*/
    { 
      if(Bsc__lsof(pd)) { /* ----------- lsbf, lsof	*/
	rs = 0;
	ls = fs;
	for(i = 0; i < fo_num; i++) /* for every octet of the field 	*/
	  { 
            mas     = (unsigned char)((i==0     ? nfmas[8-fs]   :  0xff)  /* calc mas for the first octet - cut unused right part	*/
				      & (i==loind ?  fmas[bitpad] : 0xff)); /* calc mas for the last octet - cut unused left part	*/
            curval  = ls>rs ? val<<(ls-rs) : val>>(rs-ls);/* shift to the correct position for the current octet	*/
            fptr[i] = (unsigned char)((fptr[i] & ~mas) | ((unsigned char)curval & mas));  /* assign value to the current bits 	*/
            rs      = rs+8;                               /* refresh rs for the next octet	*/
	  }
      }
      else {/* ------------------------------------------- lsbf,lsol	*/

	rs = (loind)<<3;
	ls = fs + bitpad;
	for(i = 0; i < fo_num; i++) {
	  mas     = (unsigned char)((i==0     ? nfmas[8-fs]   : 0xff)   /* calc mas for the first octet - cut unused right part	*/
				    & (i==loind ?  fmas[bitpad] : 0xff)); /* calc mas for the last octet - cut unused left part	*/
	  rs      = i==loind ? 0 : rs;                  /* correction of the rs for the last octet	*/
	  curval  = ls>rs ? val<<(ls-rs) : val>>(rs-ls);/* shift to the correct position for the current octet	*/
	  curval  = ((fo_num==1) && !fs) ? val : curval;/* checking do we need shift, may be it's less 1 byte and already in the right place*/
	  fptr[i] = (unsigned char)((fptr[i] & ~mas) | ((unsigned char)curval & mas));  /* assign value to the current bits	*/
	  rs      = rs-8;                               /* refresh rs for the next octet	*/
	  ls      = bitpad;                             /* refresh ls for the next octet	*/
	}
      }
    }
  else /* (traffic goes in right direction) -> |8|7|6|5|4|3|2|1| 	*/
    {
      if(Bsc__lsof(pd)) {/* ------------ lsbl, lsof	*/
	rs = 0;
	ls = 0;
	for(i = 0; i < fo_num; i++) {
	  mas     = (unsigned char)((i==0     ? fmas[fs]        : 0xff)    /* calc mas for the first octet - cut unused right part	*/
				    & (i==loind ? nfmas[8-bitpad] : 0xff));  /* calc mas for the last octet - cut unused left part	*/
	  curval  = ls>rs ? val<<(ls-rs) : val>>(rs-ls);/* shift to the correct position for the current octet	*/
	  fptr[i] = (unsigned char)((fptr[i] & ~mas) | ((unsigned char)curval & mas));  /* assign value to the current bits	*/
	  rs      = rs+8;                               /* refresh rs for the next octet 	*/
	  ls      = fs;                                 /* refresh ls for the next octet	*/
	}
      }
      else {/* ------------------------------------------- lsbl, lsol	*/
	rs = (loind)<<3;
	ls = bitpad;
	for(i = 0; i < fo_num; i++) {
	  mas     = (unsigned char)((i==0     ? fmas[fs]        : 0xff)    /* calc mas for the first octet - cut unused right part	*/
				    & (i==loind ? nfmas[8-bitpad] : 0xff));  /* calc mas for the last octet - cut unused left part	*/
	  curval  = ls>rs ? val<<(ls-rs) : val>>(rs-ls);/* shift to the correct position for the current octet	*/
	  fptr[i] = (unsigned char)((fptr[i] & ~mas) | ((unsigned char)curval & mas));  /* assign value to the current bits	*/
	  rs      = rs-8;                               /* refresh rs for the next octet	*/
	}
      }
    }
   
  return val;   
}

/*decode length terminated octet string from the bit stream*/
unsigned int Bsc__getOctetStr    (Bsc *pd, unsigned char *pv, unsigned int lo)
{
  unsigned int lenO=0, i=0, tmp=0;
  if(!pv) { Bsc__setErr(pd, TRUE_VN); return 0; }
   
  /* Bsc__MoveB(pd, Bsc__AligningB(pd, 8), eBscCur);   align string to the octet boundary if necessary - to be safe	*/
  lenO = MIN_VN(Bsc__lenCurO(pd), lo);
  if (lenO < lo) Bsc__setErr(pd, TRUE_VN);

  for(i=0; i<lenO; i++) {
    Bsc__getUnsignedB(pd, &tmp, 8);
    if (!pd->m_noc) pv[i] = (unsigned char)tmp;
  }

  return i;
}

/*encode length terminated octet string to the bit stream*/
unsigned int Bsc__putOctetStr    (Bsc *pd, unsigned char *pv, unsigned int lo)
{ 
  unsigned int lenO=0, i=0, tmp=0;
  if(!pv) { Bsc__setErr(pd, TRUE_VN); return 0; }
   
  /* Bsc__MoveB(pd, Bsc__AligningB(pd, 8), eBscCur);   align string to the octet boundary if necessary - to be safe	*/
  lenO = MIN_VN(Bsc__lenCurO(pd), lo);
  if (lenO < lo) Bsc__setErr(pd, TRUE_VN);

  for(i=0; i<lenO; i++) {
    if (!pd->m_noc) tmp = (unsigned char)pv[i];
    Bsc__putUnsignedB(pd, &tmp, 8);
  }

  return i;
}


/*decode terminated octet string length l with terminator t from the bit stream to the buffer pv*/
unsigned int Bsc__getTermOctetStr (Bsc* pd, unsigned char  *pv, unsigned int lo, int t) /*  does not allocate memory	*/
{
  unsigned int lenO=0, i=0, tmp=0;
  if(!pv || t<0 || t>255) { Bsc__setErr(pd, TRUE_VN); return 0; }
   
  /* Not good for Per */
  /* Bsc__MoveB(pd, Bsc__AligningB(pd, 8), eBscCur);   align string to the octet boundary if necessary - to be safe	*/
   
  lenO = MIN_VN(Bsc__lenCurO(pd), lo);
  if (lenO < lo) Bsc__setErr(pd, TRUE_VN);

  for(i=0; i<lenO; i++) {
    Bsc__getUnsignedB(pd, &tmp, 8);
    if (!pd->m_noc) pv[i] = (unsigned char)tmp;
    if(tmp==t) break;
  }

  /*if there is no terminator in the string, get it to the buffer and ser error state*/
  if(tmp != t)  {
    if (!pd->m_noc) pv[i] = (unsigned char)t;
    i++;
    Bsc__setErr(pd, TRUE_VN);
  }

  return i;
}

/*encode terminated octet string with terminator t and length l from the buffer pv, length l*/
unsigned int Bsc__putTermOctetStr (Bsc* pd, unsigned char  *pv, unsigned int lo, int t)
{
  unsigned int lenO=0, i=0, tmp=0;
  if(!pv || t<0 || t>255) { Bsc__setErr(pd, TRUE_VN); return 0; }
   
  /* Bsc__MoveB(pd, Bsc__AligningB(pd, 8), eBscCur);   align string to the octet boundary if necessary - to be safe	*/
  lenO = MIN_VN(Bsc__lenCurO(pd), lo);
  if (lenO < lo) Bsc__setErr(pd, TRUE_VN);

  for(i=0; i<lenO; i++) {
    if (!pd->m_noc) tmp = (unsigned char)pv[i];
    Bsc__putUnsignedB(pd, &tmp, 8);
    if(tmp==t) break;
  }

  /*if there is no terminator in the string, put it and ser error state*/
  if(tmp != t)  {
    Bsc__putUnsignedB(pd, (unsigned int*)&t, 8);
    i++;
    Bsc__setErr(pd, TRUE_VN);
  }

  return i;
}

/*********************************************************************
 End of File: Bsc.c
*********************************************************************/

