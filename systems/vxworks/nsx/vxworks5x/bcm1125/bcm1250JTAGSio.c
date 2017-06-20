/* bcm1250JTAGSio.c - BCM1250 JTAG communications driver */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: bcm1250JTAGSio.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01b,07dec01,agf  apply coding standard fix-ups
01a,15nov01,agf  written. Based on m68681Sio.c driver.
*/

/*
DESCRIPTION
This is the driver for the BCM1250 JTAG communications.
*/

#include "vxWorks.h"
#include "sioLib.h"
#include "intLib.h"
#include "taskLib.h"
#include "errno.h"
#include "bcm1250JTAGSio.h"
#include "config.h"

LOCAL void   bcm1250JTAGgetdword (BCM1250_JTAG_CHAN *);
LOCAL STATUS bcm1250JTAGtimer (BCM1250_JTAG_CHAN *);

LOCAL int    bcm1250JTAGIoctl (BCM1250_JTAG_CHAN *, int, void *);
LOCAL int    bcm1250JTAGTxStartup (BCM1250_JTAG_CHAN *);
LOCAL int    bcm1250JTAGCallbackInstall (BCM1250_JTAG_CHAN *, int, STATUS (*)(), void *);
LOCAL int    bcm1250JTAGPollInput (BCM1250_JTAG_CHAN *, char *);
LOCAL int    bcm1250JTAGPollOutput (BCM1250_JTAG_CHAN*, char);

/* driver functions */

LOCAL SIO_DRV_FUNCS bcm1250JTAGSioDrvFuncs =
    {
    (int (*)(SIO_CHAN *, int, void *))bcm1250JTAGIoctl,
    (int (*)(SIO_CHAN *))bcm1250JTAGTxStartup,
    (int (*)())bcm1250JTAGCallbackInstall,
    (int (*)(SIO_CHAN *, char*))bcm1250JTAGPollInput,
    (int (*)(SIO_CHAN *, char))bcm1250JTAGPollOutput
    };


#define JTAG_READ(x, result) \
    ((result) = MIPS3_LD(pChan->base_reg + x))
#define JTAG_WRITE(x,y) \
    MIPS3_SD( (pChan->base_reg + x), y) 

#define JTAG_CONS_INPUT_BYTE7_SHIFT      56
#define JTAG_CONS_INPUT_NEXT_CHAR_SHIFT  8
#define JTAG_CONS_OUTPUT_SIG_SHIFT       56
#define JTAG_CONS_OUTPUT_CHAR_SHIFT      48

LOCAL STATUS bcm1250JTAGDummyCallback (void)
{
    return (ERROR);
}

/******************************************************************************
*
* bcm1250JTAGDevInit - intialize a BCM1250_JTAGSio channel
*
* Initialize driver functions and dummy callback functions
* Set to poll mode
*
* RETURNS: N/A
*/                                    
                                         
void bcm1250JTAGDevInit 
    (
    BCM1250_JTAG_CHAN * pChan
    )
    {
    if((pChan->channel != JTAG_CHANNEL_A ))
        return;
    
    /* init callbacks and set default options */
    pChan->sio.pDrvFuncs= &bcm1250JTAGSioDrvFuncs;
    
    pChan->getTxChar	= bcm1250JTAGDummyCallback;
    pChan->putRcvChar	= bcm1250JTAGDummyCallback;
    pChan->mode         = SIO_MODE_POLL;
    }

/******************************************************************************
*
* bcm1250JTAGDevInit2 - second stage BCM1250_JTAGSio channel initialization 
*
* Wait until magic number show up and then spawn JTAG timer task
*
* RETURNS: N/A
*/

void bcm1250JTAGDevInit2 
    (
    BCM1250_JTAG_CHAN * pChan
    )
    {
    unsigned long long magic = 0;

    do 
        {
        JTAG_READ(JTAG_CONS_CONTROL, magic);
        } while (magic != JTAG_CONS_MAGICNUM);
    
    taskSpawn("jtagConsPoll", 1, VX_SUPERVISOR_MODE, 4000,
              (FUNCPTR)bcm1250JTAGtimer, (int)pChan, 
              0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

/******************************************************************************
*
* bcm1250JTAGgetdword - Get the next dword (if any) from the JTAG client 
*
* RETURNS: N/A
*/

LOCAL void bcm1250JTAGgetdword 
    (
    BCM1250_JTAG_CHAN * pChan
    )
    {
    unsigned long long input;

    if (pChan->waiting_input)
        return;

    JTAG_READ(JTAG_CONS_INPUT, input);
    pChan->waiting_input = (input >> JTAG_CONS_INPUT_BYTE7_SHIFT) & 0xff;
    pChan->input_buf = input << JTAG_CONS_INPUT_NEXT_CHAR_SHIFT;
    }

/******************************************************************************
*
* bcm1250JTAGTxStartup - start the transmitter.
*
* RETURNS:
* Returns OK on success, or EIO on hardware error.
*/                                                 
 
LOCAL int bcm1250JTAGTxStartup 
    (
    BCM1250_JTAG_CHAN * pChan
    )
    {
    char   outChar;
    int    lvl;
    
    while ((*pChan->getTxChar) (pChan->getTxArg, &outChar) != ERROR)
        {
        lvl = intLock();
        JTAG_WRITE (JTAG_CONS_OUTPUT, 
                    (1LL << JTAG_CONS_OUTPUT_SIG_SHIFT) | 
                    (((unsigned long long)outChar) << JTAG_CONS_OUTPUT_CHAR_SHIFT));
        intUnlock (lvl);
        }

    return (OK);
    }


/******************************************************************************
*
* bcm1250JTAGCallbackInstall - install callbacks to get/put chars.
*
* RETURNS:
* Returns OK on success, or ENOSYS for an unsupported callback type.
*/                                                                                                                      
LOCAL int bcm1250JTAGCallbackInstall 
    (
    BCM1250_JTAG_CHAN *	pChan,
    int		callbackType,
    STATUS	(*callback)(),
    void *      callbackArg
    )
    {
    switch (callbackType)
        {
        case SIO_CALLBACK_GET_TX_CHAR:
            pChan->getTxChar	= callback;
            pChan->getTxArg	= callbackArg;
            return (OK);

        case SIO_CALLBACK_PUT_RCV_CHAR:
            pChan->putRcvChar	= callback;
            pChan->putRcvArg	= callbackArg;
            return (OK);

        default:
            return (ENOSYS);
        }
    }

/******************************************************************************
*
* bcm1250JTAGPollOutput - output a character.
*
* RETURNS:
* Returns OK if a character sent. 
*/                                                                                                                      
LOCAL int bcm1250JTAGPollOutput
    (
    BCM1250_JTAG_CHAN *	pChan,
    char	outChar
    )
    {
    int lvl;
    
    lvl = intLock();

    /* write out the character */
    JTAG_WRITE (JTAG_CONS_OUTPUT, 
                (1LL << JTAG_CONS_OUTPUT_SIG_SHIFT) | 
                ((unsigned long long)outChar << JTAG_CONS_OUTPUT_CHAR_SHIFT));

    intUnlock (lvl);
    return (OK);
    }


/******************************************************************************
*
* bcm1250JTAGPollInput - poll the device for input.
*
* RETURNS:
* Returns OK if a character arrived, EIO on device error, EAGAIN
* if the input buffer if empty.
*/                                                                   

LOCAL int bcm1250JTAGPollInput 
    (
    BCM1250_JTAG_CHAN *	pChan,
    char *	thisChar
    )
    {
    bcm1250JTAGgetdword(pChan);

    if (!pChan->waiting_input)
        return (EAGAIN);
    
    *thisChar = (pChan->input_buf >> JTAG_CONS_INPUT_BYTE7_SHIFT) & 0xff;
    pChan->input_buf <<= JTAG_CONS_INPUT_NEXT_CHAR_SHIFT;
    pChan->waiting_input--;
    
    return (OK);
    }

/*******************************************************************************
*
* bcm1250JTAGIoctl - special device control
*
* RETURNS:
* Returns OK on success, EIO on device error, ENOSYS on unsupported
* request.
*/                                                                                                                      
LOCAL int bcm1250JTAGIoctl 
    (
    BCM1250_JTAG_CHAN *	pChan,		/* device to control */
    int		request,		/* request code */
    void *	someArg			/* some argument */
    )
    {
    switch (request)
        {
        case SIO_BAUD_SET:
        case SIO_HW_OPTS_SET:
            return (OK);
        
        case SIO_MODE_SET:
            pChan->mode = (UINT)someArg;
            if (pChan->mode == SIO_MODE_INT) 
                 {
                 }
            else 
                 {
                 }
            return (OK);

        case SIO_BAUD_GET:
            *(int *)someArg = 9600;
            return (OK);
        
        case SIO_MODE_GET:
            *(int *)someArg = pChan->mode;
            return (OK);
        
        case SIO_AVAIL_MODES_GET:
            *(int *)someArg = SIO_MODE_POLL | SIO_MODE_INT;
            return (OK);
        
        case SIO_HW_OPTS_GET:
            *(int *)someArg = CS8|CLOCAL|CREAD;
            return (OK);
        
        default:
            return (ENOSYS);
    }
}

/*******************************************************************************
*
* bcm1250JTAGtimer - JTAG services task 
*
* It will extract all available input chars to input_buf and back to sleep for
* one system clock tick
*
* RETURNS: N/A
*/                                                  
 
LOCAL STATUS bcm1250JTAGtimer 
    (
    BCM1250_JTAG_CHAN *pChan
    )
    {
    unsigned long long inchar;

    while (1) 
        {
        bcm1250JTAGgetdword(pChan);
        if (pChan->mode == SIO_MODE_INT) 
            {
            while (pChan->waiting_input)
                {
                inchar = (pChan->input_buf >> JTAG_CONS_INPUT_BYTE7_SHIFT) & 0xff;

                taskLock();
                (*pChan->putRcvChar) (pChan->putRcvArg, (char)inchar);
                taskUnlock();

                pChan->input_buf <<= JTAG_CONS_INPUT_NEXT_CHAR_SHIFT;
                pChan->waiting_input--;
                bcm1250JTAGgetdword(pChan);
                }

            taskDelay(1);
            }
        }

    return (OK);
    }

