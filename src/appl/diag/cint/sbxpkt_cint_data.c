/*
 * $Id: sbxpkt_cint_data.c,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * sbxpkt_cint_data.c
 *
 * Hand-coded support for sbx_pkt routines. 
 *
 */
int sbxpkt_cint_data_not_empty;
#include <sdk_config.h>
#if (defined(INCLUDE_LIB_CINT) && defined(INCLUDE_TEST))
#ifdef BCM_SBX_SUPPORT

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <sal/core/libc.h>

#include <appl/test/sbx_pkt.h>
#include <appl/test/sbx_rx.h>


CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_cmic_port,
                         int, int, cmic_port,0,0);
CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0,
                         sbxpkt_create, 
                         sbxpkt_t*,sbxpkt_t,packet,1,0,
                         char*,char,data,1,0);
CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0,
                         sbxpkt_prepend,
                         sbxpkt_t*,sbxpkt_t,packet,1,0,
                         char*,char,data,1,0);
CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0,
                         sbxpkt_append, 
                         sbxpkt_t*,sbxpkt_t,packet,1,0,
                         char*,char,data,1,0);
CINT_FWRAPPER_CREATE_RP0(sbxpkt_t*, sbxpkt_t, 1, 0,
                         sbxpkt_alloc);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_free,
                         sbxpkt_t*,sbxpkt_t,packet,1,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_print,
                         sbxpkt_t*,sbxpkt_t,packet,1,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_get_type,
                         sbxpkt_t*,sbxpkt_t,packet,1,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_check_ipv4,
                         sbxpkt_t*,sbxpkt_t,packet,1,0);
CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0,
                         sbxpkt_compare,
                         sbxpkt_t*,sbxpkt_t,tx_pkt,1,0,
                         sbxpkt_t*,sbxpkt_t,rx_pkt,1,0);
CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0,
                         sbxpkt_compare_ext,
                         sbxpkt_t*,sbxpkt_t,tx_pkt,1,0,
                         sbxpkt_t*,sbxpkt_t,rx_pkt,1,0,
                         int,int,length,0,0);
CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0,
                         sbxpkt_to_byte,
                         sbxpkt_t*,sbxpkt_t,packet,1,0,
                         unsigned char*,unsigned char,data,1,0);
CINT_FWRAPPER_CREATE_RP4(int, int, 0, 0,
                         sbxpkt_from_byte,
                         int,int,start_type,0,0,
                         uint8*,uint8,data,1,0,
                         int,int,length,0,0,
                         sbxpkt_t*,sbxpkt_t,packet,1,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_rx_sync_start,
                         int,int,unit,0,0);
CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0,
                         sbxpkt_rx_sync,
                         int,int,unit,0,0,
                         bcm_pkt_t*,bcm_pkt_t,packet,1,0,
                         int,int,timeout,0,0);
CINT_FWRAPPER_CREATE_RP4(int, int, 0, 0,
                         sbxpkt_rxs_sync,
                         int,int,unit,0,0,
                         bcm_pkt_t**,bcm_pkt_t,packet,2,0,
                         int,int,timeout,0,0,
			 int*,int,num_rx,1,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_rx_sync_stop,
                         int,int,unit,0,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_rx_sync_set_priority,
                         int,int,pri,0,0);
CINT_FWRAPPER_CREATE_RP1(int, int, 0, 0,
                         sbxpkt_rx_sync_queue_size,
                         int,int,size,1,0);
CINT_FWRAPPER_CREATE_RP4(int, int, 0, 0,
                         sbxpkt_data_memget,
                         bcm_pkt_t*,bcm_pkt_t,packet,1,0,
                         int,int,idx,0,0,
                         uint8*,uint8,dest,1,0,
                         int,int,len,0,0);
CINT_FWRAPPER_CREATE_VP1(
                         sbxpkt_data_clear,
                         bcm_pkt_t*,bcm_pkt_t,packet,1,0);
CINT_FWRAPPER_CREATE_VP3(
                         sbxpkt_one_buf_setup,
                         bcm_pkt_t*,bcm_pkt_t,packet,1,0,
                         unsigned char*,unsigned char,buf,1,0,
                         int,int,len,0,0);
CINT_FWRAPPER_CREATE_VP1(
                         sbxpkt_one_buf_free,
                         bcm_pkt_t*,bcm_pkt_t,packet,1,0);

static cint_function_t __cint_sbxpkt_functions[] = 
    {
        CINT_FWRAPPER_ENTRY(sbxpkt_cmic_port),
        CINT_FWRAPPER_ENTRY(sbxpkt_create),
        CINT_FWRAPPER_ENTRY(sbxpkt_prepend),
        CINT_FWRAPPER_ENTRY(sbxpkt_append),
        CINT_FWRAPPER_ENTRY(sbxpkt_alloc),
        CINT_FWRAPPER_ENTRY(sbxpkt_free),
        CINT_FWRAPPER_ENTRY(sbxpkt_print),
        CINT_FWRAPPER_ENTRY(sbxpkt_get_type),
        CINT_FWRAPPER_ENTRY(sbxpkt_check_ipv4),
        CINT_FWRAPPER_ENTRY(sbxpkt_compare),
        CINT_FWRAPPER_ENTRY(sbxpkt_compare_ext),
        CINT_FWRAPPER_ENTRY(sbxpkt_to_byte),
        CINT_FWRAPPER_ENTRY(sbxpkt_from_byte),
        CINT_FWRAPPER_ENTRY(sbxpkt_rx_sync_start),
        CINT_FWRAPPER_ENTRY(sbxpkt_rx_sync),
        CINT_FWRAPPER_ENTRY(sbxpkt_rxs_sync),
        CINT_FWRAPPER_ENTRY(sbxpkt_rx_sync_stop),
        CINT_FWRAPPER_ENTRY(sbxpkt_rx_sync_set_priority),
        CINT_FWRAPPER_ENTRY(sbxpkt_rx_sync_queue_size),
        CINT_FWRAPPER_ENTRY(sbxpkt_data_memget),
        CINT_FWRAPPER_ENTRY(sbxpkt_data_clear),
        CINT_FWRAPPER_ENTRY(sbxpkt_one_buf_setup),
        CINT_FWRAPPER_ENTRY(sbxpkt_one_buf_free),

        CINT_ENTRY_LAST
    }; 


static void*
__cint_maddr__entry_desc_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    entry_desc_t* s = (entry_desc_t*) p;

    switch(mnum)
    {
        case 0: rv = &(s->type); break;
        case 1: rv = &(s->next); break;
        case 2: rv = &(s->length); break;
        default: rv = NULL; break;
    }

    return rv;
}

static void*
__cint_maddr__sbxpkt_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    sbxpkt_t* s = (sbxpkt_t*) p;

    switch(mnum)
    {
        case 0: rv = &(s->entry); break;
        case 1: rv = &(s->normalize); break;
        default: rv = NULL; break;
    }

    return rv;
}

static cint_parameter_desc_t __cint_struct_members__entry_desc_t[] =
{
    {
        "int",
        "type",
        0,
        0
    },
    {
        "void*",
        "next",
        0,
        0
    },
    {
        "uint32",
        "length",
        0,
        0
    },
    { NULL }
};

static cint_parameter_desc_t __cint_struct_members__sbxpkt_t[] =
{
    {
        "entry_desc_t",
        "entry",
        0,
        0
    },
    {
        "uint32",
        "normalize",
        0,
        0
    },
    { NULL }
};


static cint_struct_type_t __cint_sbxpkt_structures[] =
{   
    {
    "entry_desc_t",
    sizeof(entry_desc_t),
    __cint_struct_members__entry_desc_t,
    __cint_maddr__entry_desc_t
    },
    {
    "sbxpkt_t",
    sizeof(sbxpkt_t),
    __cint_struct_members__sbxpkt_t,
    __cint_maddr__sbxpkt_t
    },
    { NULL }
};



cint_data_t sbxpkt_cint_data = 
    {
        NULL,
        __cint_sbxpkt_functions,
        __cint_sbxpkt_structures,
        NULL, 
        NULL, 
        NULL, 
        NULL
    }; 

#endif /* BCM_FE2000_SUPPORT */
#endif /* INCLUDE_LIB_CINT && INCLUDE_TEST */

