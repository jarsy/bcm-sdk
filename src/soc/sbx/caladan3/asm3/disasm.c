/*
 * $Id: disasm.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose: mini disassembler for CLI interface of debugger
 *          Very crude, error prone, be extra careful if modifying!
 */

#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/cm.h>


#define DISASSEMBLER_VERBOSE      

#ifdef DISASSEMBLER_VERBOSE      
#define DISASSEMBLER_DEBUG(_a) LOG_VERBOSE(BSL_LS_SOC_COMMON, _a)
#else
#define DISASSEMBLER_DEBUG(_a)
#endif



#define PACK_INST(a,b,c,d) (((a) << 24)  | ((b) << 16) | ((c) << 8) | (d))

#define TASK(inst) (((inst) >> 29) & 0x7)
#define TYPE(inst) (((inst) >> 26) & 0x7)

int is_jmp_type(unsigned int inst) {
    return (TYPE(inst) == 0);
}
int is_reg_type(unsigned int inst) {
    return (TYPE(inst) == 1);
}
int is_flush_type(unsigned int inst) {
    return ((TYPE(inst) == 2) && (((inst >> 22) & 0x3)==3));
}
int is_store_type(unsigned int inst) {
    return ((TYPE(inst) == 2)  && (((inst >> 25) & 1)==1));
}
int is_load_type(unsigned int inst) {
    return ((TYPE(inst) == 2)  && (((inst >> 25) & 1)==0));
}
int is_header_type(unsigned int inst) {
    return (TYPE(inst) == 3);
}
int is_headerm_type(unsigned int inst) {
    return ((TYPE(inst) == 1) && (((inst >> 21) & 0x1f) ==0xb));
}
int is_dreq_type(unsigned int inst) {
    return ((TYPE(inst) == 1) && (((inst >> 23) & 0x7) == 0));
}
int is_key_type(unsigned int inst) {
    return ((TYPE(inst) == 1) && (((inst >> 18) & 0xff) == 0xff));
}
int is_switch_type(unsigned int inst) {
    return ((TYPE(inst) == 7));
}
int is_header_inst(char *base) {
    if ((strstr(base, "hbase") != NULL) ||
        (strstr(base, "hcount") != NULL) ||
        (strstr(base, "hfind") != NULL) ||
        (strstr(base, "hdepth") != NULL)) {
        return 1;
    } 
    return 0;
}

#define NOP          0x0
#define JLT          0x08
#define JLE          0x09
#define JGT          0x0a
#define JGE          0x0b
#define JAND         0x10
#define JOR          0x11
#define JXOR         0x12
#define JNZ          0x12
#define JNE          0x12
#define JANDNOT      0x13
#define JNAND        0x14
#define JMP          0x14
#define JNOR         0x15
#define JEQ          0x16
#define JZ           0x16
#define JNXOR        0x16
#define JORNOT       0x17
#define JLAND        0x18
#define JLOR         0x19
#define JLXOR        0x1a
#define JLANDNOT     0x1b
#define JLNAND       0x1c
#define JLNOR        0x1d
#define JLNXOR       0x1e
#define JLORNOT      0x1f

#define ADD              0x20
#define ADDC             0x21
#define ADDNC            0x22
#define AND              0x30
#define ANDNOT           0x33
#define CONCAT           0x39
#define FIRSTBITSET      0x47
#define HASH32           0x42
#define HCOUNT           0x50
#define HFIND            0x51
#define HDEPTH           0x52
#define HBASE            0x53
#define HINSERT          0x59
#define HINSERTQ         0x5A
#define HDELETE          0x5B
#define HDELETEQ         0x5C
#define HDELETEM         0x5D
#define LSL              0x3c
#define LSLC             0x3d
#define LSR              0x3e
#define LSRC             0x3f
#define MASK             0x44
#define MASK1            0x45
#define MAX              0x48
#define MEXT             0x46
#define MIN              0x49
#define MOV              0x31
#define MOVIFNZ          0x4e
#define MOVIFZ           0x4f
#define NAND             0x34
#define NOR              0x35
#define NOT              0x35
#define NXOR             0x36
#define OR               0x31
#define ORNOT            0x37
#define POPCNTAND        0x40
#define POPCNTSEL        0x41
#define REVERSE          0x3b
#define ROTL             0x38
#define ROTR             0x3a
#define SADD             0x28
#define SADDC            0x29
#define SADDNC           0x2a
#define SSUB             0x2c
#define SSUBB            0x2d
#define SSUBNB           0x2e
#define SUB              0x24
#define SUBB             0x25
#define SUBNB            0x26
#define XOR              0x32

#define TLT         0x08
#define TLE         0x09
#define TGT         0x0a
#define TGE         0x0b
#define TAND        0x10
#define TOR         0x11
#define TXOR        0x12
#define TANDNOT     0x13
#define TNAND       0x14
#define TNOR        0x15
#define TNXOR       0x16
#define TORNOT      0x17
#define TLAND       0x18
#define TLOR        0x19
#define TLXOR       0x1a
#define TLANDNOT    0x1b
#define TLNAND      0x1c
#define TLNOR       0x1d
#define TLNXOR      0x1e
#define TLORNOT     0x1f

void str_insert(char *s1, char *s2, int pos)
{
    char b1[256] = {0};
    char c;
    int i, j, l, m;
    if (!s1 || !s2) {
        return; 
    }
    l = strlen(s2); 
    m = strlen(s1);
    if (pos > m) { pos = m; }
    for (j=0, i=0; i < sizeof(b1)-1; i++) {
        c = ' ';
        if ((i < m) && (i < pos)) {
            c = s1[i]; j++;
        } else {
           if (i < pos+l) {
                c = s2[i-pos];
           } else if ((i-pos-l) < m) {
                c = s1[j]; j++;
           }
        }
        if (c != '\0') {
            b1[i] = c;
        }
    }
    b1[i] = '\0';
        
    DISASSEMBLER_DEBUG(("Inserting #%s# into #%s# at pos %d\n",
                       s2, s1, pos));
    sal_snprintf(s1, 80, "%s", b1);
    DISASSEMBLER_DEBUG(("New String #%s# \n", s1));
}

int get_insert_pos(char *p)
{
    /* Maintain order here */
    char *base = p;
    DISASSEMBLER_DEBUG(("\nSearching for insert pos in ##%s## ->", base));
    while (*p != 0) {
        if ((*p == 'h') && (*(p+1) == ':')) {
            DISASSEMBLER_DEBUG(("Found h:\n"));
            break;
        }
        if ((*p == 'h') && (*(p+1) == '-')) {
            DISASSEMBLER_DEBUG(("Found h:\n"));
            break;
        }
        if ((*p == 'h') && (*(p+1) == '+')) {
            DISASSEMBLER_DEBUG(("Found h:\n"));
            break;
        }
        if ((*p == '+') && (*(p+1) == ':')) {
            DISASSEMBLER_DEBUG(("Found +:\n"));
            break;
        }
        if ((*p == '-') && (*(p+1) == ':')) {
            DISASSEMBLER_DEBUG(("Found -:\n"));
            break;
        }
        if ((*p == ',') && (*(p+1) == ',')) {
            DISASSEMBLER_DEBUG(("Found ,,\n"));
            break;
        }
        if ((*p == 'x') && (*(p+1) == ',')) {
            DISASSEMBLER_DEBUG(("Found x,\n"));
            break;
        }
        if ((*p == '(') && (*(p+1) == ')')) {
            DISASSEMBLER_DEBUG(("Found ()\n"));
            break;
        }
        if ((*p == '(') && (*(p+1) == '+')) {
            DISASSEMBLER_DEBUG(("Found (+\n"));
            break;
        }
        if ((*p == '(') && (*(p+1) == '-')) {
            DISASSEMBLER_DEBUG(("Found (-\n"));
            break;
        }
        if ((*p == '+') && (*(p+1) == ')')) {
            DISASSEMBLER_DEBUG(("Found +)\n"));
            break;
        }
        if ((*p == '-') && (*(p+1) == ')')) {
            DISASSEMBLER_DEBUG(("Found -)\n"));
            break;
        }
        if ((*p == 'y') && (*(p+1) == ',') && (*(p+2) == ',')) {
            DISASSEMBLER_DEBUG(("Found y,,\n"));
            p++; 
            break;
        }
        if ((*p == 'y') && (*(p+1) == ',')) {
            DISASSEMBLER_DEBUG(("Found y,\n"));
            break;
        }
        p++;
    }
    if (*p==0) {
       DISASSEMBLER_DEBUG(("Inserting at EOS \n"));
    }
    return ((p-base) + 1);
}


void 
SPRINTF_REG(char *t, int q, int reg) {
    if ((reg) < 64) {
            sal_sprintf((t), "%s%d", ((q) > 0) ? "rr" : "r", 
                                     ((q) > 0) ? (reg)/2 : (reg));
    } else if (reg < 128) {
            sal_sprintf((t), "%s%d", ((q) > 0) ? "tt" : "t", 
                                     ((q) > 0) ? (reg-64)/2 : (reg-64));
    } else if (reg < 160) {
            sal_sprintf((t), "%s%d", ((q) > 0) ? "dd" : "d", 
                                     ((q) > 0) ? (reg-128)/2 : (reg-128));
    } else if ((reg > 223) && (reg < 256)) {
            sal_sprintf((t), "%s%d", ((q) > 0) ? "cc" : "c", 
                                     ((q) > 0) ? (reg-224)/2 : (reg-224));
    } else {
         switch (reg) {
         case 160:
             sal_sprintf(t, "%s", "pir");
             break;
         case 161:
             sal_sprintf(t, "%s", "psr");
             break;
         case 162:
             sal_sprintf(t, "%s", "tsr");
             break;
         case 163:
             sal_sprintf(t, "%s", "btr0");
             break;
         case 164:
             sal_sprintf(t, "%s", "btr1");
             break;
         case 165:
             sal_sprintf(t, "%s", "btr2");
             break;
         case 166:
             sal_sprintf(t, "%s", "ptpr");
             break;
         case 167:
             sal_sprintf(t, "%s", "ptpsr");
             break;
         case 168:
             sal_sprintf(t, "%s", "ntpr");
             break;
         case 169:
             sal_sprintf(t, "%s", "ntpsr");
             break;
         case 170:
             sal_sprintf(t, "%s", "rnr");
             break;
         case 171:
             sal_sprintf(t, "%s", "tmr");
             break;
         case 172:
             sal_sprintf(t, "%s", "gvr");
             break;
         case 174:
             sal_sprintf(t, "%s", "tvr");
             break;
         case 176:
             sal_sprintf(t, "%s", "bpr");
             break;
         case 178:
             sal_sprintf(t, "%s", "ibpr");
             break;
         case 180:
             sal_sprintf(t, "%s", "dttr");
             break;
         case 182:
             sal_sprintf(t, "%s", "dtsr");
             break;
         case 184:
             sal_sprintf(t, "%s", "rz");
             break;
         }
    }
}

int
SPRINTF_OP(char *t, int op) {
    return sal_sprintf(t, "(%s)", (op == 1) ? "-" : "+");
}

void
SPRINTF_SRC(char *t, char *srca) {
    char buf[64] = {0};
    int pos;
    pos =  get_insert_pos(t);
    sal_sprintf(buf, "%s", srca);
    str_insert(t, buf, pos);
}

void
SPRINTF_PORT(char *m, int port) {
    if ((port) < 10) {
            sal_sprintf((m), "p%d", (port));
    } else {
            sal_sprintf((m), "%s", ((port) == 10) ? "cop" : "cmu");
    }
}

void
decode_map(char *buf, int map) {
    int i, l=0, l1=0;
    l = sal_sprintf(buf, "%s", " [");
    for (i=0; i< 6; i++) {
        if (map & (1 << i)) {
            if (l1) {
                l1 += sal_sprintf(buf+l+l1, "%s", ",");
                l+=l1;
            }
            l1 = sal_sprintf(buf+l, "%d", i+1);
        }
    }
    sal_sprintf(buf+l+l1, "%s", "] ");
}

int
jmpinst(unsigned int inst, char *p) 
{
    unsigned int jop;
    unsigned int map, str, addr;
    int l=0;
    char *base = p;
    char buf[256]={0};
    if (is_jmp_type(inst)) {
        jop = ((inst >> 21) & 0x1f);
        map = ((inst >> 15) & 0x3F);
        str = ((inst >> 11) & 0xF);
        addr = (inst & 0x3ff);
        DISASSEMBLER_DEBUG(("%d %d %d %d\n", jop, map, str, addr));
        switch (jop) {
        case JAND:
            l=sal_sprintf(p, "%s", "jand");
            break;
        case JANDNOT:
            l=sal_sprintf(p, "%s", "jandnot");
            break;
        case JGE:
            l=sal_sprintf(p, "%s", "jge");
            break;
        case JGT:
            l=sal_sprintf(p, "%s", "jgt");
            break;
        case JLAND:
            l=sal_sprintf(p, "%s", "jland");
            break;
        case JLANDNOT:
            l=sal_sprintf(p, "%s", "jlandnot");
            break;
        case JLE:
            l=sal_sprintf(p, "%s", "jle");
            break;
        case JLNAND:
            l=sal_sprintf(p, "%s", "jlnand");
            break;
        case JLNOR:
            l=sal_sprintf(p, "%s", "jlnor");
            break;
        case JLNXOR:
            l=sal_sprintf(p, "%s", "jlnxor");
            break;
        case JLOR:
            l=sal_sprintf(p, "%s", "jlor");
            break;
        case JLORNOT:
            l=sal_sprintf(p, "%s", "jlornot");
            break;
        case JLT:
            l=sal_sprintf(p, "%s", "jlt");
            break;
        case JLXOR:
            l=sal_sprintf(p, "%s", "jlxor");
            break;
        case JNAND:
            l=sal_sprintf(p, "%s", "jnand");
            break;
        case JNOR:
            l=sal_sprintf(p, "%s", "jnor");
            break;
        case JNXOR:
            l=sal_sprintf(p, "%s", "jnxor");
            break;
        case JOR:
            l=sal_sprintf(p, "%s", "jor");
            break;
        case JORNOT:
            l=sal_sprintf(p, "%s", "jornot");
            break;
        case JXOR:
            l=sal_sprintf(p, "%s", "jxor");
            break;
        case NOP:
            l=sal_sprintf(p, "%s", "nop");
            p+=l;
            return p-base;
        }
        decode_map(buf, map);
        l += sal_sprintf(p+l, "%s", buf);
        l += sal_sprintf(p+l, " s(%d).i(%d) ", str, addr);
    } else {
        return 0;
    }
    p+=l;
    return p-base;
}

int
reginst(unsigned int inst, char *p) 
{
    unsigned int regop;
    unsigned int rofs, rlen, reg;
    int l=0;
    char *base = p;
    if (is_reg_type(inst)) {
        regop = ((inst >> 18) & 0xFF);
        rofs = ((inst >> 13) & 0x1F);
        rlen = ((inst >> 8) & 0x1F);
        reg = (inst & 0xFF);
        switch(regop) {
        case ADD:
            l=sal_sprintf(p, "%s ", "add");
            break;
 /*
        case  ADD1:
            l=sal_sprintf(p, "%s ", "add1");
            break;
 */
        case ADDC:
            l=sal_sprintf(p, "%s ", "addc");
            break;
        case ADDNC:
            l=sal_sprintf(p, "%s ", "addnc");
            break;
        case AND:
            l=sal_sprintf(p, "%s ", "and");
            break;
        case ANDNOT:
            l=sal_sprintf(p, "%s ", "andnot");
            break;
/*
        case CLR:
            l=sal_sprintf(p, "%s ", "clr");
            break;
*/
        case CONCAT:
            l=sal_sprintf(p, "%s ", "concat");
            break;
/*
        case END:
            l=sal_sprintf(p, "%s ", "end");
            break;
        case ENDIFNZ:
            l=sal_sprintf(p, "%s ", "endifnz");
            break;
        case ENDIFZ:
            l=sal_sprintf(p, "%s ", "endifz");
            break;
*/
        case FIRSTBITSET:
            l=sal_sprintf(p, "%s ", "firstbitset");
            break;
        case HASH32:
            l=sal_sprintf(p, "%s ", "hash32");
            break;
/*
        case HASH32XOR:
            l=sal_sprintf(p, "%s ", "hash32xor");
            break;
*/
        case HBASE:
            l=sal_sprintf(p, "%s ", "hbase");
            break;
        case HCOUNT:
            l=sal_sprintf(p, "%s ", "hcount");
            break;
        case HDEPTH:
            l=sal_sprintf(p, "%s ", "hdepth");
            break;
        case HFIND:
            l=sal_sprintf(p, "%s ", "hfind");
            break;
        case LSL:
            l=sal_sprintf(p, "%s ", "lsl");
            break;
        case LSR:
            l=sal_sprintf(p, "%s ", "lsr");
            break;
        case MASK:
            l=sal_sprintf(p, "%s ", "mask");
            break;
        case MASK1:
            l=sal_sprintf(p, "%s ", "mask1");
            break;
        case MAX:
            l=sal_sprintf(p, "%s ", "max");
            break;
        case MEXT:
            l=sal_sprintf(p, "%s ", "mext");
            break;
        case MIN:
            l=sal_sprintf(p, "%s ", "min");
            break;
/*
        case MOV:
            l=sal_sprintf(p, "%s ", "mov");
            break;
*/
        case MOVIFNZ:
            l=sal_sprintf(p, "%s ", "movifnz");
            break;
        case MOVIFZ:
            l=sal_sprintf(p, "%s ", "movifz");
            break;
        case NAND:
            l=sal_sprintf(p, "%s ", "nand");
            break;
        case NOR:
            l=sal_sprintf(p, "%s ", "nor");
            break;
/*
        case NOT:
            l=sal_sprintf(p, "%s ", "not");
            break;
*/
        case NXOR:
            l=sal_sprintf(p, "%s ", "nxor");
            break;
        case OR:
            l=sal_sprintf(p, "%s ", "or");
            break;
        case ORNOT:
            l=sal_sprintf(p, "%s ", "ornot");
            break;
        case POPCNTAND:
            l=sal_sprintf(p, "%s ", "popcntand");
            break;
        case POPCNTSEL:
            l=sal_sprintf(p, "%s ", "popcntsel");
            break;
        case REVERSE:
            l=sal_sprintf(p, "%s ", "reverse");
            break;
        case ROTL:
            l=sal_sprintf(p, "%s ", "rotl");
            break;
        case ROTR:
            l=sal_sprintf(p, "%s ", "rotr");
            break;
        case SADD:
            l=sal_sprintf(p, "%s ", "sadd");
            break;
        case SADDC:
            l=sal_sprintf(p, "%s ", "saddc");
            break;
        case SADDNC:
            l=sal_sprintf(p, "%s ", "saddnc");
            break;
/*
        case SET:
            l=sal_sprintf(p, "%s ", "set");
            break;
*/
        case SSUB:
            l=sal_sprintf(p, "%s ", "ssub");
            break;
        case SSUBB:
            l=sal_sprintf(p, "%s ", "ssubb");
            break;
        case SSUBNB:
            l=sal_sprintf(p, "%s ", "ssubnb");
            break;
        case SUB:
            l=sal_sprintf(p, "%s ", "sub");
            break;
/*
        case SUB1:
            l=sal_sprintf(p, "%s ", "sub1");
            break;
*/
        case SUBB:
            l=sal_sprintf(p, "%s ", "subb");
            break;
        case SUBNB:
            l=sal_sprintf(p, "%s ", "subnb");
            break;
        case TAND:
            l=sal_sprintf(p, "%s ", "tand");
            break;
        case TANDNOT:
            l=sal_sprintf(p, "%s ", "tandnot");
            break;
/*
        case TEQ:
            l=sal_sprintf(p, "%s ", "teq");
            break;
*/
        case TGE:
            l=sal_sprintf(p, "%s ", "tge");
            break;
        case TGT:
            l=sal_sprintf(p, "%s ", "tgt");
            break;
        case TLAND:
            l=sal_sprintf(p, "%s ", "tland");
            break;
        case TLANDNOT:
            l=sal_sprintf(p, "%s ", "tlandnot");
            break;
        case TLE:
            l=sal_sprintf(p, "%s ", "tle");
            break;
        case TLNAND:
            l=sal_sprintf(p, "%s ", "tlnand");
            break;
        case TLNOR:
            l=sal_sprintf(p, "%s ", "tlnor");
            break;
        case TLNXOR:
            l=sal_sprintf(p, "%s ", "tlnxor");
            break;
        case TLOR:
            l=sal_sprintf(p, "%s ", "tlor");
            break;
        case TLORNOT:
            l=sal_sprintf(p, "%s ", "tlornot");
            break;
        case TLT:
            l=sal_sprintf(p, "%s ", "tlt");
            break;
        case TLXOR:
            l=sal_sprintf(p, "%s ", "tlxor");
            break;
        case TNAND:
            l=sal_sprintf(p, "%s ", "tnand");
            break;
/*
        case TNE:
            l=sal_sprintf(p, "%s ", "tne");
            break;
*/
        case TNOR:
            l=sal_sprintf(p, "%s ", "tnor");
            break;
        case TNXOR:
            l=sal_sprintf(p, "%s ", "tnxor");
            break;
/*
        case TNZ:
            l=sal_sprintf(p, "%s ", "tnz");
            break;
*/
        case TOR:
            l=sal_sprintf(p, "%s ", "tor");
            break;
        case TORNOT:
            l=sal_sprintf(p, "%s ", "tornot");
            break;
        case TXOR:
            l=sal_sprintf(p, "%s ", "txor");
            break;
/*
        case TZ:
            l=sal_sprintf(p, "%s ", "tz");
            break;
*/
        case XOR:
            l=sal_sprintf(p, "%s ", "xor");
            break;
        }
        SPRINTF_REG(p+l, 0, reg);
        l = strlen(base);
        sal_sprintf(base+l, "[%d:%d]", (rlen+rofs), rofs);
    } else {
       return 0;
    }
    l = strlen(base);
    sal_sprintf(base+l, "%s", ", ");
    return l+1;
}

int
meminst(unsigned int inst, int *op, char *p) 
{
    char *base = p;
    int q=0, l=0 , o=0 ;
    int port = 0;
    int reg=0, seg=0;
    char t[16] = {0};
    char m[16] = {0};

    port = ((inst >> 18) & 0xF);
    reg = inst & 0xFF;
    seg = (inst >> 10) & 0xFF;
    q = (inst >> 24) & 1;
    o = (inst >> 22) & 1;
    SPRINTF_PORT(m, port);
    SPRINTF_REG(t, q, reg);
    if (is_load_type(inst)) {
        if (reg == 0xb8) {
            l=sal_sprintf(p, "dreq %s.%d", m, seg-128);
        } else {
            l=sal_sprintf(p, "load %s, %s.%d", t, m, seg);
        }
        SPRINTF_OP(p+l, o);
    } else if (is_flush_type(inst)) {
        l=sal_sprintf(p, "flush %s", m);
    } else if (is_store_type(inst)) {
        if ((inst >> 23) & 1) {
            l=sal_sprintf(p, "store %s.%d",m, seg);
            SPRINTF_OP(p+l, o); 
            l=strlen(p);
            sal_sprintf(p+l, ", 0x%x", (inst & 0x3FF));
        } else {
            l=sal_sprintf(p, "store %s.%d", m, seg); /* Handle bofs */
            SPRINTF_OP(p+l, o); 
            l=strlen(p);
            sal_sprintf(p+l, ", %s", t); /* Handle bofs */
        }
    } else {
       return 0;
    }
    *op = -1;
    return p-base;
}

int 
splinst(unsigned int inst, int *op, char *p) 
{
    char *base = p;
    int l = 0;
    char buf[64] = {0};
    int reg = 0;

    reg = inst & 0xff;
    if (is_switch_type(inst)) {
        l=sal_sprintf(p, "%s", "switch");   
    } else if (is_key_type(inst)) {
        *op = 6;
        if ( reg != 0xb8) {
            SPRINTF_REG(buf, 1, reg);
            l=sal_sprintf(p, "key,, %s", buf);   
        } else {
            l=sal_sprintf(p, "%s", "key");   
        }
    } else {
        return 0;
    }
    p+=l;
    return p-base;
}
    

int 
hdrinst(unsigned int inst, int *op, char *p) {
    int hopcode = 0;
    int bofs, blen, reg, hlen;
    int l = 0;
    char r[16] = {0};
    char *base = p;
    if (is_header_type(inst)) {
        hopcode = (inst >> 19) & 0x7F;
        bofs = (inst >> 8) & 0x3;
        blen = (inst >> 10) & 0x7;
        reg = (inst & 0xff);
        hlen = (inst >> 16) & 0x7;
        *op = (inst >> 20) & 0x3;
        if (((((blen+1) * 8) == 64) && (bofs==0))) {
            SPRINTF_REG(r, 1, reg);
            l = strlen(r);
        } else if (((((blen+1) * 8) == 32) && (bofs==0))) {
            SPRINTF_REG(r, 0, reg);
            l = strlen(r);
        } else {
            sal_sprintf(r+l, "[%d:%d]", (blen+1+bofs)*8, bofs*8);
        }
        switch (hopcode) {
            case 0x0:
                l=sal_sprintf(p, "hread %s, h:%d", r, hlen+1);
                *op = 5;
                break;
            case 0x2:
            case 0x4:
                l=sal_sprintf(p, "hload %s, h:%d", r, hlen+1);
                break;
            case 0x8:
                l=sal_sprintf(p, "%s", "hrestore");
                break;
            case 0x10:
            case 0x11:
                l=sal_sprintf(p, "%s", "hwrite");
                break;
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
                if ((inst >> 19) & 1) {
                    l=sal_sprintf(p, "hstore h:%d, 0x%x", hlen+1, (inst & 0xFFFF));
                } else {
                    l=sal_sprintf(p, "hstore h:%d, %s", hlen+1, r);
                }
                break;
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
                if ((inst >> 19) & 1) {
                    l=sal_sprintf(p, "hstorep h:%d, 0x%x", hlen+1, (inst & 0xFFFF));
                } else {
                    l=sal_sprintf(p, "hstorep h:%d, %s", hlen+1, r);
                }
                break;
        }
    } else if (is_headerm_type(inst)) {
        hopcode = (inst >> 18) & 0x7;
        switch (hopcode) {
            case 0x1:
                l=sal_sprintf(p, "%s, 0x%x ", "hinsert x", (inst & 0xff));
                *op=4;
                break;
            case 0x3:
                l=sal_sprintf(p, "%s", "hdelete");
                break;
            case 0x4:
                l=sal_sprintf(p, "%s", "hdeleteq");
                break;
            case 0x5:
                l=sal_sprintf(p, "%s", "hdeletem");
                break;
        }
    } else {
        return 0;
    }
    p+=l;
    return p-base;
}

int decode_inst_w0(unsigned int w0, int *op, char*p)
{
   int l = 0;
   char *base = p;
   if (w0 == 0) {
       /* optimized for nop */
       sal_sprintf(p, "0. nop");
       return strlen(p);
   } 
   p += sal_sprintf(p, " %d. ", TASK(w0));
   *op = 3;
   l = hdrinst(w0, op, p);
   if (l==0)
       l = splinst(w0, op, p);
   if (l==0)
       l = meminst(w0, op, p);
   if (l==0)
       l = reginst(w0, p);
   if (l==0)
       l = jmpinst(w0, p);
   p+=l;
   DISASSEMBLER_DEBUG(("Check w0: #%s# op= %d\n", base, *op));
   return p-base;
}

int
decode_bitfield(unsigned int w, char *p)
{
     char *base = p;
     int reg, rlen, roff;
     if ((w >> 18) & 1)
         p += sal_sprintf(p, "%s", "+");
     reg = w & 0xff;
     rlen = (w >> 8) & 0x1f;
     roff = (w >> 13) & 0x1f;
     SPRINTF_REG(p, 0, reg);
     p = base + strlen(base);
     if (reg < 160) {
        p+=sal_sprintf(p, "[%d:%d]", roff+rlen, roff);
     }
     DISASSEMBLER_DEBUG(("Bitfield decode: %s\n", base));
     return base-p;
}

char*
decode_flag(unsigned char flag)
{
    if (flag == 0) {
        return("F0");
    }else if (flag == 1) {
        return("F1");
    }else if (flag == 2) {
        return("F2");
    }else if (flag == 3) {
        return("F3");
    }else if (flag == 4) {
        return("F4");
    }else if (flag == 5) {
        return("F5");
    }else if (flag == 6) {
        return("F6");
    }else if (flag == 7) {
        return("F7");
    }else if (flag == 8) {
        return("F8");
    }else if (flag == 9) {
        return("F9");
    } else if (flag == 10) {
        return("E");
    } else if (flag == 11) {
        return("B");
    } else if (flag == 12) {
        return("H");
    } else if (flag == 13) {
        return("V");
    } else if (flag == 14) {
        return("Z");
    } else {
        return("K");
    }
}

int 
decode_pred(unsigned int pred, char *q)
{
    char *base, *f1, *f2;
    char b2[256] = {0}; char b1[256] = {0};
    int l = 0, cond = 0;
    f1 = decode_flag(pred & 0xF);
    f2 = decode_flag((pred > 4) & 0xF);
    base = q;
    while ((*q != ' ') && (*q != 0)) q++;
    sal_sprintf(b2, "%s ", q);
    l=sal_snprintf(b1, q-base, "%s ", base);
    q=b1+l;

    cond =  ((pred >> 8) & 0x7);
    switch (cond) {
    case 0:
        if (f1 == f2) {
            l=sal_sprintf(q, "(!%s)", f1);
        } else {
            l=sal_sprintf(q, "!(%s || %s)", f1, f2);
        }
        break;
    case 1:
        l=sal_sprintf(q, "(%s && %s)", f1, f2);
        break;
    case 2:
        l=sal_sprintf(q, "(%s ^ %s)", f1, f2);
        break;
    case 3:
        l=sal_sprintf(q, "!(%s && %s)", f1, f2);
        break;
    case 4:
        if (f1 == f2) {
            l=sal_sprintf(q, "(%s)", f1);
        } else {
            l=sal_sprintf(q, "(%s || %s)", f1, f2);
        }
        break;
    case 6:
        l=sal_sprintf(q, "!(%s ^ %s)", f1, f2);
        break;
    case 7:
        break;
    }
    if (cond != 7 ) {
        sal_sprintf(base, "%s %s", b1, b2);
        return strlen(base);
    } else {
        return 0;
    }
}

int decode_inst_w1(unsigned int w1, int *op, int *sctl, char*p)
{
    char *base = p;
    int ins = 0;
    char buf[256] = {0};
    char bitf[256] = {0};
    int l, pred, sc, srca;
    pred = (w1 >> 21) & 0x7ff;
    sc = (w1 >> 19) & 0x3;
    srca = (w1  & 0x7ffff);
    l = decode_pred(pred, p);
    p+=l;
    ins = get_insert_pos(base);
    if (*op == 5) {
        /* hread */
        return 0;
    } else if ( *op == 6) {
        /* key */
        srca = w1 & 0xff;
        if (srca != 0xb8) {
            SPRINTF_REG(bitf, 1, srca);
            sal_sprintf(buf, " %s", bitf);
            str_insert(base, buf, ins);
            return strlen(p);
        } else {
            return 0;
        }
    }
    switch (sc) {
    case 0:
        if (*op > 0) {
            if (*op == 1) {
                sal_sprintf(buf, "(0x%x +", srca);
            } else if (*op == 2) {
                sal_sprintf(buf, "(0x%x -", srca);
            } else if (*op == 4) {
                sal_sprintf(buf, ", 0x%x", srca);
                ins = strlen(base);
            } else {
                sal_sprintf(buf, "0x%x,", srca);
            }
        } else {
            sal_sprintf(buf, "0x%x ", srca);
        }
        str_insert(base, buf, ins);
        break;
    case 1:
    case 3:
        l  = decode_bitfield(srca, bitf);
        if (*op > 0 ) {
            if (*op == 1) {
                sal_sprintf(buf, "(%s +", bitf);
            } else if (*op == 2) {
                sal_sprintf(buf, "(%s -", bitf);
            } else if (*op == 4) {
                sal_sprintf(buf, ", %s", bitf);
                ins = strlen(base);
            } else {
               sal_sprintf(buf, "%s, ", bitf);
            }
        } else {
            sal_sprintf(buf, "%s ", bitf);
        }
        str_insert(base, buf, ins);
        break;
    case 2:
        decode_bitfield(srca, bitf);
        if (*op > 0) {
            if (*op == 1) {
                sal_sprintf(buf, "+ %s)", bitf);
            } else if (*op == 2) {
                sal_sprintf(buf, "- %s)", bitf);
            } else {
                ins += get_insert_pos(base+ins);
                sal_sprintf(buf, "x, %s", bitf);
                *op = 4;
            }
        } else {
            ins += get_insert_pos(base+ins);
            sal_sprintf(buf, "%s", bitf);
        }
        str_insert(base, buf, ins);
        break;
    }
    *sctl = sc;
   DISASSEMBLER_DEBUG(("Check w1: #%s#, op=%d, sctl=%d\n", base, *op, *sctl));
    return strlen(p);
}

int decode_inst_w2(unsigned int w2, int op, int *sctl, char*p)
{
    char *base = p;
    char buf[256] = {0};
    int srca, l=0, ins=0;
    int bitfield = (w2 >> 13);
    ins = get_insert_pos(base);
    COMPILER_REFERENCE(l);
    if ( op == 6) {
        /* key */
        srca = (w2 >> 13 )& 0xff;
        if (srca != 0xb8) {
            int dest = 0, part = 0;
            SPRINTF_REG(buf, 1, srca);
            str_insert(base, buf, ins);
            dest = w2 & 0x1f;
            part = (w2 >> 8 ) & 0x3;
            sal_sprintf(buf, " %d.%d ", dest, part);
            str_insert(base, buf, ins);
            return strlen(p);
        } else {
            return 0;
        }
    } else if (is_header_inst(base)) {
        w2 >>= 12;
    }
    switch (*sctl) {
    case 2:
        if ((op == 1) || (op == 2)) {
            l = sal_sprintf(buf, "(0x%x ", w2);
        } else if (op == 4) {
            l = sal_sprintf(buf, "0x%x", w2);
            base[ins-1] = ' ';
        } else if (op == 3) {
            l = sal_sprintf(buf, "0x%x ", w2);
        } else {
            l = sal_sprintf(buf, "0x%x", w2);
        }
        break;
    case 0:
        if (op == 5) {
            l = sal_sprintf(buf, "({%d,%d} + %x)",
                            ((w2 >> 12) & 0x3F),
                            ((w2 >>  8) & 0xF),
                            (w2 & 0xFF));
            break;
        }
    case 3:
        if ((op == 1) || (op == 2)) {
            l = sal_sprintf(buf, " 0x%x)", w2);
        } else if (op == 4) {
            l = sal_sprintf(buf, "0x%x", w2);
            base[ins-1] = ' ';
        } else if (op == 3) {
            l = sal_sprintf(buf, " 0x%x", w2);
        } else {
            l = sal_sprintf(buf, "0x%x", w2);
        }
        break;
    case 1:
        buf[0] = ' ';
        l  = decode_bitfield(bitfield, &buf[1]);
        break;
    }
    str_insert(base, buf, ins);
    return strlen(base);
}

void soc_sbx_caladan3_ucode_debug_decode_inst(char *buf, uint8 *p) 
{
    unsigned int w0=0, w1=0, w2=0;
    int len = 0, sctl = 0;
    char decoded[80] = {0};
    int op=0;

    w0 = PACK_INST(p[0], p[1], p[2], p[3]); p+=4;
    w1 = PACK_INST(p[0], p[1], p[2], p[3]); p+=4;
    w2 = PACK_INST(p[0], p[1], p[2], p[3]); 

    len += decode_inst_w0(w0, &op, decoded);
    if (((w1 != 0) && (w1 != 0xffe00000)) || (w2 != 0)) {
         /* decode only non-nop */
         /* coverity[overrun-buffer-val] */
         len += decode_inst_w1(w1, &op, &sctl, decoded);
         len += decode_inst_w2(w2, op, &sctl, decoded);
    }
    DISASSEMBLER_DEBUG((" Decoded inst: %s\n", decoded));
    sal_sprintf(buf, "%s", decoded);
}
