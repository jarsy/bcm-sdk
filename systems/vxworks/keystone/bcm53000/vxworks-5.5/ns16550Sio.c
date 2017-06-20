/* ns16550Sio.c - NS 16550 UART tty driver */

/* Copyright 1984-1997 Wind River Systems, Inc. */

#include "copyright_wrs.h"

/* $Id: ns16550Sio.c,v 1.3 2011/07/21 16:14:25 yshtil Exp $
modification history
--------------------
01j,14mar01,rcs  corrected baud rate divisor calculation formula. (SPR# 63899)
01i,17sep97,dat  fixed merge problems that caused baud rate setting to fail.
01h,06mar97,dat  SPR 7899, max baud rate set to 115200.
01g,08may97,db   added hardware options and modem control(SPRs #7570, #7082).
01f,18dec95,myz  added case IIR_TIMEOUT in ns16550Int routine.
01e,28nov95,myz  fixed bugs to work at 19200 baud or above with heavy traffic.
01d,09nov95,jdi  doc: style cleanup.
01c,02nov95,myz  undo 01b fix
01b,02nov95,p_m  added test for 960CA and 960JX around access to lcr field
		 in order to compile on all architectures.
01a,24oct95,myz  written from ns16550Serial.c.
*/

/*
DESCRIPTION
This is the driver for the NS16552 DUART. This device includes two universal
asynchronous receiver/transmitters, a baud rate generator, and a complete 
modem control capability. 

A NS16550_CHAN structure is used to describe the serial channel. This data 
structure is defined in ns16550Sio.h.

Only asynchronous serial operation is supported by this driver.
The default serial settings are 8 data bits, 1 stop bit, no parity, 9600
baud, and software flow control.  

USAGE
The BSP's sysHwInit() routine typically calls sysSerialHwInit(),
which creates the NS16550_CHAN structure and initializes all the values in the 
structure (except the SIO_DRV_FUNCS) before calling ns16550DevInit().
The BSP's sysHwInit2() routine typically calls sysSerialHwInit2(), which
connects the chips interrupts via intConnect() (either the single
interrupt `ns16550Int' or the three interrupts `ns16550IntWr', `ns16550IntRd',
and `ns16550IntEx').

This driver handles setting of hardware options such as parity(odd, even) and
number of data bits(5, 6, 7, 8). Hardware flow control is provided with the
handshakes RTS/CTS. The function HUPCL(hang up on last close) is available.
When hardware flow control is enabled, the signals RTS and DTR are set TRUE 
and remain set until a HUPCL is performed. 

INCLUDE FILES: drv/sio/ns16552Sio.h

*/

/* includes */

#include "vxWorks.h"
#include "intLib.h"
#include "errnoLib.h"
#include "errno.h"
#include "sioLib.h"
#include "drv/sio/ns16552Sio.h"

/* local defines       */

#ifndef SIO_HUP
#   define SIO_OPEN	0x100A
#   define SIO_HUP	0x100B
#endif

/* min/max baud rate */
 
#define NS16550_MIN_RATE 50
#define NS16550_MAX_RATE 115200

#define REG(reg, pchan) \
 (*(volatile UINT8 *)((UINT32)pchan->regs + (reg * pchan->regDelta)))
#define REGPTR(reg, pchan) \
 ((volatile UINT8 *)((UINT32)pchan->regs + (reg * pchan->regDelta)))

IMPORT  void     sysWbFlush ();

/* static forward declarations */

LOCAL 	int 	ns16550CallbackInstall (SIO_CHAN *, int, STATUS (*)(), void *);
LOCAL 	STATUS 	ns16550DummyCallback ();
LOCAL 	void 	ns16550InitChannel (NS16550_CHAN *);
LOCAL   STATUS  ns16550BaudSet (NS16550_CHAN *, UINT);
LOCAL 	STATUS  ns16550ModeSet (NS16550_CHAN *, UINT);
LOCAL 	STATUS 	ns16550Ioctl (NS16550_CHAN *, int, int);
LOCAL 	void 	ns16550TxStartup (NS16550_CHAN *);
LOCAL 	int 	ns16550PollOutput (NS16550_CHAN *, char);
LOCAL 	int 	ns16550PollInput (NS16550_CHAN *, char *);
LOCAL 	STATUS 	ns16550OptsSet (NS16550_CHAN *, UINT);
LOCAL 	STATUS 	ns16550Open (NS16550_CHAN * pChan );
LOCAL 	STATUS 	ns16550Hup (NS16550_CHAN * pChan );
LOCAL   void    ns16550RegWr(volatile UINT8 *reg, UINT8 data);
LOCAL   UINT8   ns16550RegRd(volatile UINT8 *reg);

/* driver functions */

static SIO_DRV_FUNCS ns16550SioDrvFuncs =
    {
    (int (*)())ns16550Ioctl,
    (int (*)())ns16550TxStartup,
    (int (*)())ns16550CallbackInstall,
    (int (*)())ns16550PollInput,
    (int (*)(SIO_CHAN *,char))ns16550PollOutput
    };

LOCAL   void    ns16550RegWr(volatile UINT8 *reg, UINT8 data)
{
    volatile int x;
    int ikey;

    ikey = intLock();
    *reg = data;
    x = (*(volatile unsigned char*)0xb8000000);
    intUnlock(ikey);
}

LOCAL   UINT8   ns16550RegRd(volatile UINT8 *reg)
{
    volatile int x;
    int ikey;
    volatile UINT8 rval;

    ikey = intLock();
    rval = *reg;
    x = (*(volatile unsigned char*)0xb8000000);
    intUnlock(ikey);
    return(rval);
}

/******************************************************************************
*
* ns16550DummyCallback - dummy callback routine.
*/

LOCAL STATUS ns16550DummyCallback (void)
    {
    return (ERROR);
    }

/******************************************************************************
*
* ns16550DevInit - intialize an NS16550 channel
*
* This routine initializes some SIO_CHAN function pointers and then resets
* the chip in a quiescent state.  Before this routine is called, the BSP
* must already have initialized all the device addresses, etc. in the
* NS16550_CHAN structure.
*
* RETURNS: N/A
*/

void ns16550DevInit
    (
    NS16550_CHAN * pChan	/* pointer to channel */
    )
    {
    int oldlevel = intLock ();

    /* initialize the driver function pointers in the SIO_CHAN's */

    pChan->pDrvFuncs    = &ns16550SioDrvFuncs;

    /* set the non BSP-specific constants */

    pChan->getTxChar    = ns16550DummyCallback;
    pChan->putRcvChar   = ns16550DummyCallback;
    pChan->channelMode  = 0;    /* undefined */
    pChan->options      = (CLOCAL | CREAD | CS8);
    pChan->mcr		= MCR_OUT2;

    /* reset the chip */

    ns16550InitChannel (pChan);

    intUnlock (oldlevel);
    }

/*******************************************************************************
*
* ns16550InitChannel - initialize UART
*
* Initialize the number of data bits, parity and set the selected
* baud rate.
* Set the modem control signals if the option is selected.
*
* RETURNS: N/A
*/

LOCAL void ns16550InitChannel
    (
    NS16550_CHAN * pChan	/* pointer to channel */	
    )
    {

    /* set the requested baud rate */

    ns16550BaudSet(pChan, pChan->baudRate);

    /* set the options */

    ns16550OptsSet(pChan, pChan->options);
    }

/*******************************************************************************
*
* ns16550OptsSet - set the serial options
*
* Set the channel operating mode to that specified.  All sioLib options
* are supported: CLOCAL, HUPCL, CREAD, CSIZE, PARENB, and PARODD.
* When the HUPCL option is enabled, a connection is closed on the last
* close() call and opened on each open() call.
*
* Note, this routine disables the transmitter.  The calling routine
* may have to re-enable it.
*
* RETURNS:
* Returns OK to indicate success, otherwise ERROR is returned
*/

LOCAL STATUS ns16550OptsSet
    (
    NS16550_CHAN * pChan,	/* pointer to channel */
    UINT options		/* new hardware options */
    )
    {
    FAST int     oldlevel;		/* current interrupt level mask */

    pChan->lcr = 0; 
    pChan->mcr &= (~(MCR_RTS | MCR_DTR)); /* clear RTS and DTR bits */
    
    if (pChan == NULL || options & 0xffffff00)
	return ERROR;

    switch (options & CSIZE)
	{
	case CS5:
	    pChan->lcr = CHAR_LEN_5; break;
	case CS6:
	    pChan->lcr = CHAR_LEN_6; break;
	case CS7:
	    pChan->lcr = CHAR_LEN_7; break;
	default:
	case CS8:
	    pChan->lcr = CHAR_LEN_8; break;
	}

    if (options & STOPB)
	pChan->lcr |= LCR_STB;
    else
	pChan->lcr |= ONE_STOP;
    
    switch (options & (PARENB | PARODD))
	{
	case PARENB|PARODD:
	    pChan->lcr |= LCR_PEN; break;
	case PARENB:
	    pChan->lcr |= (LCR_PEN | LCR_EPS); break;
	default:
	case 0:
	    pChan->lcr |= PARITY_NONE; break;
	}

    ns16550RegWr(REGPTR(IER, pChan), 0);

    if (!(options & CLOCAL))
	{
	/* !clocal enables hardware flow control(DTR/DSR) */

	pChan->mcr |= (MCR_DTR | MCR_RTS);
    	pChan->ier &= (~TxFIFO_BIT); 
	pChan->ier |= IER_EMSI;    /* enable modem status interrupt */
	}
    else
        pChan->ier &= ~IER_EMSI; /* disable modem status interrupt */ 

    oldlevel = intLock ();

    ns16550RegWr(REGPTR(LCR, pChan), pChan->lcr);
    ns16550RegWr(REGPTR(MCR, pChan), pChan->mcr);

    /* now reset the channel mode registers */

    ns16550RegWr(REGPTR(FCR, pChan) , (RxCLEAR | TxCLEAR | FIFO_ENABLE));


    if (options & CREAD)  
	pChan->ier |= RxFIFO_BIT;
    if (pChan->channelMode == SIO_MODE_INT)
	{
        ns16550RegWr(REGPTR(IER, pChan) , pChan->ier);
        }
    intUnlock (oldlevel);

    pChan->options = options;

    return OK;
    }

/*******************************************************************************
*
* ns16550Hup - hang up the modem control lines 
*
* Resets the RTS and DTR signals and clears both the receiver and
* transmitter sections.
*
* RETURNS: OK
*/

LOCAL STATUS ns16550Hup
    (
    NS16550_CHAN * pChan 	/* pointer to channel */
    )
    {
    FAST int     oldlevel;	/* current interrupt level mask */

    oldlevel = intLock ();

    pChan->mcr &= (~(MCR_RTS | MCR_DTR));
    ns16550RegWr(REGPTR(MCR, pChan) , pChan->mcr);
    ns16550RegWr(REGPTR(FCR, pChan) , (RxCLEAR | TxCLEAR)); 

    intUnlock (oldlevel);

    return (OK);

    }    

/*******************************************************************************
*
* ns16550Open - Set the modem control lines 
*
* Set the modem control lines(RTS, DTR) TRUE if not already set.  
* It also clears the receiver, transmitter and enables the fifo. 
*
* RETURNS: OK
*/

LOCAL STATUS ns16550Open
    (
    NS16550_CHAN * pChan 	/* pointer to channel */
    )
    {
    FAST int     oldlevel;	/* current interrupt level mask */
    char mask;

    mask = ns16550RegRd(REGPTR(MCR, pChan)) & (MCR_RTS | MCR_DTR);

    if (mask != (MCR_RTS | MCR_DTR)) 
    	{
    	/* RTS and DTR not set yet */

    	oldlevel = intLock ();

	/* set RTS and DTR TRUE */

    	pChan->mcr |= (MCR_DTR | MCR_RTS); 
        ns16550RegWr(REGPTR(MCR, pChan) , pChan->mcr); 

    	/* clear Tx and receive and enable FIFO */

        ns16550RegWr(REGPTR(FCR, pChan) , (RxCLEAR | TxCLEAR | FIFO_ENABLE));

    	intUnlock (oldlevel);
	}

    return (OK);
    }

/******************************************************************************
*
* ns16550BaudSet - change baud rate for channel
*
* This routine sets the baud rate for the UART. The interrupts are disabled
* during chip access.
*
* RETURNS: OK
*/

LOCAL STATUS  ns16550BaudSet
    (
    NS16550_CHAN * pChan,	/* pointer to channel */
    UINT	   baud		/* requested baud rate */
    )
    {
    int   oldlevel;
    int   divisor = (pChan->xtal / 16);
    divisor /= baud;

    /* disable interrupts during chip access */

    oldlevel = intLock ();

    /* Enable access to the divisor latches by setting DLAB in LCR. */

    ns16550RegWr(REGPTR(LCR, pChan) , LCR_DLAB | pChan->lcr);

    /* Set divisor latches. */
    ns16550RegWr(REGPTR(DLL,pChan) , divisor);
    ns16550RegWr(REGPTR(DLM,pChan) , (divisor >> 8));

    /* Restore line control register */

    ns16550RegWr(REGPTR(LCR, pChan) , pChan->lcr);

    pChan->baudRate = baud;
 
    intUnlock (oldlevel);

    return (OK);
    }

/*******************************************************************************
*
* ns16550ModeSet - change channel mode setting
*
* This driver supports both polled and interrupt modes and is capable of
* switching between modes dynamically. 
*
* If interrupt mode is desired this routine enables the channels receiver and 
* transmitter interrupts. If the modem control option is TRUE, the Tx interrupt
* is disabled if the CTS signal is FALSE. It is enabled otherwise. 
*
* If polled mode is desired the device interrupts are disabled. 
*
* RETURNS:
* Returns a status of OK if the mode was set else ERROR.
*/

LOCAL STATUS ns16550ModeSet
    (
    NS16550_CHAN * pChan,	/* pointer to channel */
    UINT	newMode		/* mode requested */
    )
    {
    FAST int     oldlevel;	/* current interrupt level mask */
    char mask;

    if ((newMode != SIO_MODE_POLL) && (newMode != SIO_MODE_INT))
	return (ERROR);
           
    oldlevel = intLock ();

    if (newMode == SIO_MODE_INT)
	{
        /* Enable appropriate interrupts */
		
	if (pChan->options & CLOCAL) {
        ns16550RegWr(REGPTR(IER, pChan) , pChan->ier | RxFIFO_BIT | TxFIFO_BIT);
	}
	else  
		{
		mask = ns16550RegRd(REGPTR(MSR, pChan)) & MSR_CTS;

   		/* if the CTS is asserted enable Tx interrupt */

   		if (mask & MSR_CTS)
			pChan->ier |= TxFIFO_BIT;    /* enable Tx interrupt */
		else
           		pChan->ier &= (~TxFIFO_BIT); /* disable Tx interrupt */

        ns16550RegWr(REGPTR(IER, pChan) , pChan->ier); 
		}	
	}
    else
        {
        /* disable all ns16550 interrupts */ 

        ns16550RegWr(REGPTR(IER, pChan) , 0);   
	}

    pChan->channelMode = newMode;

    intUnlock (oldlevel);

    return (OK);
   }

/*******************************************************************************
*
* ns16550Ioctl - special device control
*
* Includes commands to get/set baud rate, mode(INT,POLL), hardware options(
* parity, number of data bits), and modem control(RTS/CTS and DTR/DSR).
* The ioctl command SIO_HUP is sent by ttyDrv when the last close() function 
* call is made. Likewise SIO_OPEN is sent when the first open() function call
* is made.
*
* RETURNS: OK on success, EIO on device error, ENOSYS on unsupported
*          request.
*/

LOCAL STATUS ns16550Ioctl
    (
    NS16550_CHAN * 	pChan,		/* pointer to channel */
    int			request,	/* request code */
    int        		arg		/* some argument */
    )
    {
    FAST STATUS  status;

    status = OK;

    switch (request)
	{
	case SIO_BAUD_SET:
	    if (arg < NS16550_MIN_RATE || arg > NS16550_MAX_RATE)
		status = EIO;		/* baud rate out of range */
	    else
	        status = ns16550BaudSet (pChan, arg);
	    break;

        case SIO_BAUD_GET:
            *(int *)arg = pChan->baudRate;
            break; 

        case SIO_MODE_SET:
	    status = (ns16550ModeSet (pChan, arg) == OK) ? OK : EIO;
            break;          

        case SIO_MODE_GET:
            *(int *)arg = pChan->channelMode;
            break;

        case SIO_AVAIL_MODES_GET:
            *(int *)arg = SIO_MODE_INT | SIO_MODE_POLL;
            break;

        case SIO_HW_OPTS_SET:
    	    status = (ns16550OptsSet (pChan, arg) == OK) ? OK : EIO;
    	    break;

        case SIO_HW_OPTS_GET:
            *(int *)arg = pChan->options;
            break;

        case SIO_HUP:
            /* check if hupcl option is enabled */

    	    if (pChan->options & HUPCL) 
	    	status = ns16550Hup (pChan);
            break;
	
	case SIO_OPEN:
            /* check if hupcl option is enabled */

    	    if (pChan->options & HUPCL) 
	    	status = ns16550Open (pChan);
	    break;

        default:
            status = ENOSYS;
	}
    return (status);
    }

/*******************************************************************************
*
* ns16550IntWr - handle a transmitter interrupt 
*
* This routine handles write interrupts from the UART. It reads a character
* and puts it in the transmit holding register of the device for transfer.
*
* If there are no more characters to transmit, transmission is disabled by 
* clearing the transmit interrupt enable bit in the IER(int enable register).
*
* RETURNS: N/A
*
*/

void ns16550IntWr 
    (
    NS16550_CHAN * pChan		/* pointer to channel */	
    )
    {
    char           outChar;

    if ((*pChan->getTxChar) (pChan->getTxArg, &outChar) != ERROR)
        ns16550RegWr(REGPTR(THR,pChan) , outChar);	/* write char to Transmit Holding Reg */
    else
        {
        pChan->ier &= (~TxFIFO_BIT);	/* indicates to disable Tx Int */
        ns16550RegWr(REGPTR(IER, pChan) , pChan->ier);
        }
    }

/*******************************************************************************
*
* ns16550IntRd - handle a receiver interrupt 
*
* This routine handles read interrupts from the UART.
*
* RETURNS: N/A
*
*/

void ns16550IntRd 
    (
    NS16550_CHAN * pChan	/* pointer to channel */
    )
    {
    char   inchar;

    /* read character from Receive Holding Reg. */

    inchar = ns16550RegRd(REGPTR(RBR, pChan));

    (*pChan->putRcvChar) (pChan->putRcvArg, inchar);
    }

/*******************************************************************************
*
* ns16550IntEx - miscellaneous interrupt processing
*
* This routine handles miscellaneous interrupts on the UART.
* Not implemented yet.
*
* RETURNS: N/A
*
*/

void ns16550IntEx 
    (
    NS16550_CHAN *pChan		/* pointer to channel */
    )
    {

    /* Nothing for now... */
    }

/********************************************************************************
*
* ns16550Int - interrupt level processing
*
* This routine handles four sources of interrupts from the UART. They are
* prioritized in the following order by the Interrupt Identification Register:
* Receiver Line Status, Received Data Ready, Transmit Holding Register Empty
* and Modem Status.
*
* When a modem status interrupt occurs, the transmit interrupt is enabled if
* the CTS signal is TRUE.
*
* RETURNS: N/A
*
*/

void ns16550Int 
    (
    NS16550_CHAN * pChan	/* pointer to channel */
    )
    {
    FAST volatile char        intStatus;
    /* read the Interrrupt Status Register (Int. Ident.) */

    intStatus = ns16550RegRd((REGPTR(IIR, pChan))) & 0x0f;

    /*
     * This UART chip always produces level active interrupts, and the IIR 
     * only indicates the highest priority interrupt.  
     * In the case that receive and transmit interrupts happened at
     * the same time, we must clear both interrupt pending to prevent
     * edge-triggered interrupt(output from interrupt controller) from locking
     * up. One way doing it is to disable all the interrupts at the beginning
     * of the ISR and enable at the end.
     */

    ns16550RegWr(REGPTR(IER,pChan) , 0);    /* disable interrupt */

    switch (intStatus)
	{
	case IIR_RLS:
            /* overrun,parity error and break interrupt */

            intStatus = ns16550RegRd(REGPTR(LSR, pChan)); /* read LSR to reset interrupt */
	    break;

        case IIR_RDA:     		/* received data available */
	case IIR_TIMEOUT: 
	   /*
	    * receiver FIFO interrupt. In some case, IIR_RDA will
            * not be indicated in IIR register when there is more
	    * than one character in FIFO.
	    */

            ns16550IntRd (pChan);  	/* RxChar Avail */
            break;

       	case IIR_THRE:  /* transmitter holding register ready */
	    {
            char outChar;

            if ((*pChan->getTxChar) (pChan->getTxArg, &outChar) != ERROR)
                ns16550RegWr(REGPTR(THR, pChan) , outChar);   /* char to Transmit Holding Reg */
            else
                pChan->ier &= (~TxFIFO_BIT); /* indicates to disable Tx Int */

            }
            break;

	case IIR_MSTAT: /* modem status register */
	   {
	   char	msr;

	   msr = ns16550RegRd(REGPTR(MSR, pChan));

	   /* if CTS is asserted by modem, enable tx interrupt */

	   if ((msr & MSR_CTS) && (msr & MSR_DCTS)) 
	   	pChan->ier |= TxFIFO_BIT;
           else
           	pChan->ier &= (~TxFIFO_BIT); 
	   }
	   break;

        default:
	    break;
        }

    ns16550RegWr(REGPTR(IER, pChan) , pChan->ier); /* enable interrupts accordingly */

    sysWbFlush();
    }

/*******************************************************************************
*
* ns16550TxStartup - transmitter startup routine
*
* Call interrupt level character output routine and enable interrupt if it is
* in interrupt mode with no hardware flow control.
* If the option for hardware flow control is enabled and CTS is set TRUE,
* then the Tx interrupt is enabled.
* 
* RETURNS: N/A
*/

LOCAL void ns16550TxStartup
    (
    NS16550_CHAN * pChan 	/* pointer to channel */
    )
    {
    char mask;

    if (pChan->channelMode == SIO_MODE_INT)
	{
	if (pChan->options & CLOCAL)
		{
		/* No modem control */

		pChan->ier |= TxFIFO_BIT;
        ns16550RegWr(REGPTR(IER,pChan) , pChan->ier); 
		}
	else
		{
		mask = ns16550RegRd(REGPTR(MSR, pChan)) & MSR_CTS;

   		/* if the CTS is asserted enable Tx interrupt */

   		if (mask & MSR_CTS)
			pChan->ier |= TxFIFO_BIT;    /* enable Tx interrupt */
		else
           		pChan->ier &= (~TxFIFO_BIT); /* disable Tx interrupt */

        ns16550RegWr(REGPTR(IER, pChan) , pChan->ier); 
		}
	}
    sysWbFlush();
    }

/******************************************************************************
*
* ns16550PollOutput - output a character in polled mode.
*
* RETURNS: OK if a character arrived, EIO on device error, EAGAIN
* if the output buffer if full.
*/

LOCAL int ns16550PollOutput
    (
    NS16550_CHAN *  pChan,	/* pointer to channel */
    char            outChar	/* char to send */
    )
    {
    char pollStatus;
    char msr;

    pollStatus = ns16550RegRd(REGPTR(LSR, pChan));
    msr = ns16550RegRd(REGPTR(MSR, pChan));

    /* is the transmitter ready to accept a character? */

    if ((pollStatus & LSR_THRE) == 0x00)
        return (EAGAIN);

    if (!(pChan->options & CLOCAL))	 /* modem flow control ? */
    	{
    	if (msr & MSR_CTS)
            ns16550RegWr(REGPTR(THR, pChan) , outChar);
	else
		return (EAGAIN);
	}
    else
        ns16550RegWr(REGPTR(THR, pChan) , outChar);       /* transmit character */

    return (OK);
    }
/******************************************************************************
*
* ns16550PollInput - poll the device for input.
*
* RETURNS: OK if a character arrived, EIO on device error, EAGAIN
* if the input buffer if empty.
*/

LOCAL int ns16550PollInput
    (
    NS16550_CHAN *  pChan,	/* pointer to channel */
    char *          pChar 	/* pointer to char */
    )
    {

    char pollStatus;

    pollStatus = ns16550RegRd(REGPTR(LSR, pChan));

    if ((pollStatus & LSR_DR) == 0x00)
        return (EAGAIN);

    /* got a character */

    *pChar = ns16550RegRd(REGPTR(RBR, pChan));

    return (OK);
    }

/******************************************************************************
*
* ns16550CallbackInstall - install ISR callbacks to get/put chars.
*
* This routine installs the callback functions for the driver
*
* RETURNS: OK on success or ENOSYS on unsupported callback type.
*/

LOCAL int ns16550CallbackInstall
    (
    SIO_CHAN *  pSioChan,	/* pointer to device to control */
    int         callbackType,	/* callback type(tx or receive) */
    STATUS      (*callback)(),	/* pointer to callback function */
    void *      callbackArg	/* callback function argument */
    )
    {
    NS16550_CHAN * pChan = (NS16550_CHAN *)pSioChan;
    switch (callbackType)
        {
        case SIO_CALLBACK_GET_TX_CHAR:
            pChan->getTxChar    = callback;
            pChan->getTxArg     = callbackArg;
            return (OK);
        case SIO_CALLBACK_PUT_RCV_CHAR:
            pChan->putRcvChar   = callback;
            pChan->putRcvArg    = callbackArg;
            return (OK);
        default:
            return (ENOSYS);
        }

    }


