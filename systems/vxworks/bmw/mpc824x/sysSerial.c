/* sysSerial.c -  Serial device initialization */

/* $Id: sysSerial.c,v 1.2 2011/07/21 16:14:08 yshtil Exp $
modification history
--------------------
01c,01may02,jmb  support non-hardwired serial clock
01b,30apr02,jmb  cast to get rid of compiler warning
01a,05mar02,jmb  Adapt NS16550 to PPC 8245 DUART
*/

/*
DESCRIPTION
This file contains the board-specific routines for serial channel
initialization.  The DUART in the PPC8245 has two more registers
than the NS16550:

  UDSR:  0x10  -- the DMA status register

         This register is read-only and is unsupported

  DCR:   0x11  -- the DUART configuration register

         This register controls whether the device will function
         as a single UART with modem control or as 2 UARTs without
         modem control.  The default behavior is a single UART in
         modem operating mode.  Since we're probably just using the
         UART as a slow console, we'll put the device in DUART mode.

         DCR also determines whether the interrupt lines
         from the UART(s) will be routed to the EPIC or to the PCI interrupt.
         We'll send interrupt(s) to the EPIC.  This is the default behavior.
*/

#include "vxWorks.h"
#include "config.h"
#include "intLib.h"
#include "iv.h"
#include "sysLib.h"
#include "config.h"

#include "sio/ns16550Sio.c"

#define UDSR 0x10
#define DCR  0x11

#define UDSR_RXRDY 0x02
#define UDSR_TXRDY 0x01

#define DCR_SDM   0x01
#define DCR_IRQS1 0x08
#define DCR_IRQS2 0x04

#define UART_REG(reg,chan)              (SERIAL_BASE(chan) + reg)

extern sysRamClkGet();

int sysSerialClkGet();

/* static variables */

LOCAL NS16550_CHAN ns16550Chan[2];

/*
 * Array of pointers to all serial channels configured in system.
 * See sioChanGet(). It is this array that maps channel pointers
 * to standard device names.  The first entry will become "/tyCo/0",
 * the second "/tyCo/1", and so forth.
 *
 */

SIO_CHAN * sysSioChans [2] =
    {
    (SIO_CHAN *) &ns16550Chan[0].pDrvFuncs,	           /* /tyCo/0 */
    (SIO_CHAN *) &ns16550Chan[1].pDrvFuncs,	           /* /tyCo/0 */
    };

/* definitions */


/******************************************************************************
 *
 * Utility routines
 */

UCHAR sysSerialInByte
    (
    int addr
    )
    {
    return *(volatile unsigned char *) addr;
    }

void sysSerialOutByte
    (
    int addr, 
    UCHAR c
    )
    {
    *(volatile unsigned char *) addr = c;
    }

/******************************************************************************
 *
 * Serial debugging print routines
 *
 * The following routines are for debugging, especially useful in early
 * boot ROM and ISR code.
 *
 * sysSerialPutc
 * sysSerialGetc
 * sysSerialPrintString
 * sysSerialPrintHex
 *
 * The sysSerialPrintString and sysSerialPrintHex routines should
 * basically ALWAYS work.  They re-initialize the UART and turn off
 * interrupts every time.
 */

/* Polled debug print goes to UART1 */

#define SS_CHAN		0

void sysSerialDelay (void)
    {
    volatile int i;
    for (i = 0; i < 0x10000; i++)
	;
    }

void sysSerialPutc 
    (
    int c
    )
    {
    int i = 10000;

    /* Spin until the transmit holding register is empty or 10000 times */

    while (!(sysSerialInByte (UART_REG (LSR, SS_CHAN)) & LSR_THRE) && i--)
        ;

    sysSerialOutByte (UART_REG (THR, SS_CHAN), c);
    }

int sysSerialGetc (void)
    {

    /* Spin until line status register indicates data available */

    while (!(sysSerialInByte (UART_REG (LSR, SS_CHAN)) & LSR_DR))
        ;

    return sysSerialInByte (UART_REG (RBR, SS_CHAN));
    }

void sysSerialUARTPollInit 
   (
   int channel
   )
   {
   sysSerialDelay ();	/* Allow last char to flush */
   sysSerialOutByte (UART_REG (LCR, channel), LCR_DLAB);
   sysSerialOutByte (UART_REG (DLL, channel), BAUD_LO (CONSOLE_BAUD_RATE));
   sysSerialOutByte (UART_REG (DLM, channel), BAUD_HI (CONSOLE_BAUD_RATE));
   sysSerialOutByte (UART_REG (LCR, channel), CHAR_LEN_8 | ONE_STOP |
       PARITY_NONE);
   /* Make sure the DUART is in 2-pin mode (no modem control) */
   sysSerialOutByte (UART_REG (DCR, 0), DCR_SDM);
   }

void sysSerialPrintString
    (
    char *s
    )
    {
    int c, il;
    il = intLock ();		/* LOCK INTERRUPTS */
#if 0
    sysSerialUARTPollInit (SS_CHAN);
#endif

    while ((c = *s++) != 0) 
        {
        if (c == '\n')
            sysSerialPutc ('\r');
        sysSerialPutc (c);
        }

    sysSerialDelay ();	/* Allow last char to flush */
    intUnlock (il);		/* UNLOCK INTERRUPTS */
    }

void sysSerialPrintHex
    (
    UINT32 value, 
    int cr
    )
    {
    const char hex[] = "0123456789abcdef";
    int i, il;
    il = intLock ();		/* LOCK INTERRUPTS */
    sysSerialUARTPollInit (SS_CHAN);

    for (i = 0; i < 8; i++)
        sysSerialPutc (hex[value >> (28 - i * 4) & 0xf]);

    if (cr) 
        {
	sysSerialPutc ('\r');
	sysSerialPutc ('\n');
        }
    sysSerialDelay ();	/* Allow last char to flush */
    intUnlock (il);		/* UNLOCK INTERRUPTS */
    }

/******************************************************************************
*
* sysSerialHwInit - initialize the BSP serial devices to a quiescent state
*
* This routine initializes the BSP serial device descriptors and puts the
* devices in a quiescent state.  It is called from sysHwInit() with
* interrupts locked.   Polled mode serial operations are possible, but not
* interrupt mode operations which are enabled by sysSerialHwInit2().
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit()
*/ 

void sysSerialHwInit (void)
    {
    ns16550Chan[0].regDelta = 1;
    ns16550Chan[0].regs =     (UINT8 *) (EUMBBAR_VAL + 0x4500);
    ns16550Chan[0].baudRate = CONSOLE_BAUD_RATE;
    ns16550Chan[0].xtal =     sysSerialClkGet();

    /*
     * Initialize driver functions, getTxChar, putRcvChar and
     * channelMode and init UART.
     */

    ns16550DevInit (&ns16550Chan[0]);

#if (DUART_CONFIG == UART_SIMPLE)
   sysSerialOutByte (UART_REG (DCR, 0), DCR_SDM);
#endif

#if (N_SIO_CHANNELS == 2)
    ns16550Chan[1].regDelta = 1;
    ns16550Chan[1].regs =     (UINT8 *) (EUMBBAR_VAL + 0x4600);
    ns16550Chan[1].baudRate = CONSOLE_BAUD_RATE;
    ns16550Chan[1].xtal =     sysSerialClkGet();

    /*
     * Initialize driver functions, getTxChar, putRcvChar and
     * channelMode and init UART.
     */

    ns16550DevInit (&ns16550Chan[1]);
#endif
    }

/******************************************************************************
*
* sysSerialHwInit2 - connect BSP serial device interrupts
*
* This routine connects the BSP serial device interrupts.  It is called from
* sysHwInit2().  
*
* Serial device interrupts cannot be connected in sysSerialHwInit() because
* the  kernel memory allocator is not initialized at that point and
* intConnect() calls malloc().
*
* RETURNS: N/A
*
* SEE ALSO: sysHwInit2()
*/ 

void sysSerialHwInit2 (void)
    {

    /* connect and enable serial interrupt */

    (void) intConnect (INUM_TO_IVEC (EPIC_VECTOR_UART1), 
        ns16550Int, (unsigned int) &ns16550Chan[0]);
    intEnable (EPIC_VECTOR_UART1);

#if (N_SIO_CHANNELS == 2)
    (void) intConnect (INUM_TO_IVEC (EPIC_VECTOR_UART2), 
        ns16550Int, (unsigned int) &ns16550Chan[1]);
    intEnable (EPIC_VECTOR_UART2);
#endif

    }

/******************************************************************************
*
* sysSerialChanGet - get the SIO_CHAN device associated with a serial channel
*
* This routine returns a pointer to the SIO_CHAN device associated
* with a specified serial channel.  It is called by usrRoot() to obtain 
* pointers when creating the system serial devices, `/tyCo/x'.  It
* is also used by the WDB agent to locate its serial channel.
*
* RETURNS: A pointer to the SIO_CHAN structure for the channel, or ERROR
* if the channel is invalid.
*/

SIO_CHAN * sysSerialChanGet
    (
    int channel         /* serial channel */
    )
    {
    if (channel < 0
     || channel >= NELEMENTS(sysSioChans) )
	return (SIO_CHAN *) ERROR;

    return sysSioChans[channel];
    }

/******************************************************************************
*
* sysSerialClkGet - get the value of the serial clock
*
* Get the serial clock.  It may be a constant, as defined by XTAL in 
* a header file, or it may be determined at runtime by computing the
* value of the RAM clock.  If the RAM clock can't be determined, this routine
* doesn't return, it reboots the system.
*
* RETURNS: value in Hz of the serial clock
*
*/ 

int
sysSerialClkGet()
    {
    int clk = sysRamClkGet ();

    if (clk == -1)
        sysToMonitor (BOOT_NORMAL);  /* reboot */

    return (clk);
    }
