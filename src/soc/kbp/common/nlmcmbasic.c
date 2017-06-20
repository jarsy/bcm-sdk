/*
 * $Id: nlmcmbasic.c,v 1.2.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include <nlmcmbasic.h>
#include <nlmcmstring.h>
#include <nlmcmportable.h>
#include <nlmcmdebug.h>
#include <nlmcmcast.h>

void NlmCmBasic__WriteStdErr(const char* msg)
{
    NlmCm__printf(msg) ;    
}


void NlmCmBasic__Panic(const char* msg)
{
    NlmCmBasic__WriteStdErr("Error: Panic: ") ;
    NlmCmBasic__WriteStdErr(msg) ;
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmDebug__Break() ;
    NlmCm__pvt_abort() ;
}


extern int
NlmCmBasic__DemandFileLine(
    NlmBool cond,
    const char* assertion,
    const char* file,
    int line)
{
    /*
     * FYI: emit an error indication before the assert fires
     *      to force the error to show up in the VC++ transcript
     *      so that the VC++ generated error/warning summary is correct.
     */
    if (!cond) {
    NlmCmBasic__FailedFileLine(assertion, file, line) ;
    NlmCmBasic__Panic("NlmCmBasic__Demand") ;
    }
    return cond;
}


NlmBool NlmCmBasic__Identity(const NlmBool b)
{
    return b ;
}


char* NlmCmCharStar__Identity(char* c)
{
    return c ;
}


const char* NlmCmConstCharStar__Identity(const char* c)
{
    return c ;
}


int 
NlmCmBasic__Note(const char* msg)
{
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmBasic__WriteStdErr("=---=---=---=---=---=---=---=---=---=---=---=---=---=---=---=\n");
    NlmCmBasic__WriteStdErr("=---= NOTE: ") ; NlmCmBasic__WriteStdErr(msg) ; NlmCmBasic__WriteStdErr(" =---=\n") ;
    NlmCmBasic__WriteStdErr("=---=---=---=---=---=---=---=---=---=---=---=---=---=---=---=\n");
    return 0 ;
}


int
NlmCmBasic__FailedFileLine(const char* msg, const char* file, int line)
{
    char    linenum[10] ;
    NlmBool banner = 1 ;

    if (*msg == '@') {
    msg++ ;
    banner = 0 ;
    }

    /* FYI: using Emacs compatible error message format. NB: quite delicate. */
    NlmCm__snprintf(linenum, sizeof linenum, "%04d", line) ;

    NlmCmBasic__WriteStdErr("\n") ;
    if (banner) {
    NlmCmBasic__WriteStdErr("===============================================\n");
    NlmCmBasic__WriteStdErr("===============================================\n");
    NlmCmBasic__WriteStdErr("==  ####### ######  ######  ####### ######   ==\n");
    NlmCmBasic__WriteStdErr("==  #       #     # #     # #     # #     #  ==\n");
    NlmCmBasic__WriteStdErr("==  #       #     # #     # #     # #     #  ==\n");
    NlmCmBasic__WriteStdErr("==  #####   ######  ######  #     # ######   ==\n");
    NlmCmBasic__WriteStdErr("==  #       #   #   #   #   #     # #   #    ==\n");
    NlmCmBasic__WriteStdErr("==  #       #    #  #    #  #     # #    #   ==\n");
    NlmCmBasic__WriteStdErr("==  ####### #     # #     # ####### #     #  ==\n");
    }
    NlmCmBasic__WriteStdErr("===============================================\n");
    NlmCmBasic__WriteStdErr("===============================================\n");
    NlmCmBasic__WriteStdErr("ERROR in line ") ;
    NlmCmBasic__WriteStdErr(linenum) ;
    NlmCmBasic__WriteStdErr(" of file ") ;
    NlmCmBasic__WriteStdErr(file ? file : "<unknown>") ;
    NlmCmBasic__WriteStdErr(" ...\nFAILED: ") ;
    NlmCmBasic__WriteStdErr(msg) ;
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmBasic__WriteStdErr("===============================================\n");
    NlmCmBasic__WriteStdErr("===============================================\n");

    return 0;
}


int
NlmCmBasic__WarningFileLine(const char* msg, const char* file, int line)
{
    char    linenum[10] ;
    NlmBool banner = 1 ;
    const char* warning_in_line = "WARNING in line ";

    if (*msg == '@') {
    msg++ ;
    banner = 0 ;
    }

    if (*msg == '-') {
    msg++ ;
    warning_in_line = "ignored_WARNING in line ";
    }

    /*
      handle both @- and -@
    */
    if (*msg == '@') {
    msg++ ;
    banner = 0 ;
    }

    if (banner) {
    /* FYI: using Emacs compatible error message format. NB: quite delicate. */
    NlmCm__snprintf(linenum, sizeof linenum, "%04d", line) ;
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmBasic__WriteStdErr("==============================================================\n");
    NlmCmBasic__WriteStdErr("==============================================================\n");
    NlmCmBasic__WriteStdErr("==  #    #    ##    #####   #    #     #    #    #   ####   ==\n");
    NlmCmBasic__WriteStdErr("==  #    #   #  #   #    #  ##   #     #    ##   #  #    #  ==\n");
    NlmCmBasic__WriteStdErr("==  #    #  #    #  #    #  # #  #     #    # #  #  #       ==\n");
    NlmCmBasic__WriteStdErr("==  # ## #  ######  #####   #  # #     #    #  # #  #  ###  ==\n");
    NlmCmBasic__WriteStdErr("==  ##  ##  #    #  #   #   #   ##     #    #   ##  #    #  ==\n");
    NlmCmBasic__WriteStdErr("==  #    #  #    #  #    #  #    #     #    #    #   ####   ==\n");
    NlmCmBasic__WriteStdErr("==============================================================\n");
    NlmCmBasic__WriteStdErr("==============================================================\n");
    NlmCmBasic__WriteStdErr(warning_in_line) ;
    NlmCmBasic__WriteStdErr(linenum) ;
    NlmCmBasic__WriteStdErr(" of file ") ;
    NlmCmBasic__WriteStdErr(file ? file : "<unknown>") ;
    NlmCmBasic__WriteStdErr(" ...\n       ... ") ;
    NlmCmBasic__WriteStdErr(msg) ;
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmBasic__WriteStdErr("==============================================================\n");
    NlmCmBasic__WriteStdErr("==============================================================\n");
    }
    else {
    /* FYI: using Emacs compatible error message format. NB: quite delicate. */
    NlmCm__snprintf(linenum, sizeof linenum, "%04d", line) ;
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmBasic__WriteStdErr("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    NlmCmBasic__WriteStdErr(warning_in_line) ;
    NlmCmBasic__WriteStdErr(linenum) ;
    NlmCmBasic__WriteStdErr(" of file ") ;
    NlmCmBasic__WriteStdErr(file ? file : "<unknown>") ;
    NlmCmBasic__WriteStdErr(" :: ") ;
    NlmCmBasic__WriteStdErr(msg) ;
    NlmCmBasic__WriteStdErr("\n") ;
    NlmCmBasic__WriteStdErr("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    }

    return 0;
}


int
NlmCmBasic__DieFileLine(const char* msg, const char* file, int line)
{
    NlmCmBasic__FailedFileLine(msg, file, line) ;
    NlmCmBasic__Panic("NlmCmBasic__DieFileLine") ;
    return 0 ;
}


void
NlmCmBasic__CopyZeroFill(
    nlm_u8*         dst,
    int             dstSize,
    const nlm_u8*   src,
    int             srcSize)
{
    assert(0 <= srcSize && srcSize <= dstSize) ;
    assert(dst) ;
    assert(src || srcSize == 0) ;
    NlmCm__memcpy(dst, src, srcSize) ;
    NlmCm__memset(dst + srcSize, 0, dstSize-srcSize) ;
}

#include <nlmcmexterncstart.h>
void
NlmCmBasic__Assert(NlmBool b)
{
    NlmCmBasic__Require(b) ;
}
#include <nlmcmexterncend.h>

extern uintvs_t
NlmCmUintVS__Identity(uintvs_t value)
{
    return value ;
}


extern nlm_8
NlmCmInt8__Identity(nlm_8 value)
{
    return value ;
}


extern nlm_16
NlmCmInt16__Identity(nlm_16 value)
{
    return value ;
}


extern nlm_32
NlmCmInt32__Identity(nlm_32 value)
{
    return value ;
}


extern int
NlmCmInt__Identity(int value) 
{
    return value ;
}


extern nlm_u8
NlmCmUint8__Identity(nlm_u8 value)
{
    return value ;
}


extern nlm_u16
NlmCmUint16__Identity(nlm_u16 value) 
{
    return value ;
}


extern nlm_u32
NlmCmUint32__Identity(nlm_u32 value)
{
    return value ;
}


extern unsigned int
NlmCmUint__Identity(unsigned int value)
{
    return value ;
}


extern void*
NlmCmVoidStar__Identity(void *value)
{
    return value ;
}


extern const nlm_u8*
NlmCmUint8Star__Identity(const nlm_u8 *value)
{
    return value ;
}


extern nlm_u8*
NlmCmBasic__Zero(
    void *data_area, 
    size_t data_len)
{
    NlmCm__memset(data_area, 0, data_len) ;
    return (nlm_u8*) data_area ;
}


extern NlmBool
NlmCmBasic__IsZero(
    const void *data_area, 
    size_t data_len)
{
    const nlm_u8* cp = (const nlm_u8*)(data_area);

    while( data_len--) {
    if (*cp++) return 0;
    }
    return 1;
}


NlmBool
NlmCmBasic__IsStrEq(const char* s, const char* t)
{
    return 0 == NlmCm__strcmp(s, t) ;
}






/*[]*/
