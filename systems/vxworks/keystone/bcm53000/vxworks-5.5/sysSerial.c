/* Copyright 2000-2001 Wind River Systems, Inc. */

#include "copyright_wrs.h"

/* $Id: sysSerial.c,v 1.3 2011/07/21 16:14:25 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

#include "vxWorks.h"
#include "intLib.h"
#include "config.h"
#include "sysLib.h"
#include "drv/sio/ns16552Sio.h"

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <vxbsp.h>
#include <siutils.h>
#include <hndmips.h>
#include <sbchipc.h>
#include <hndchipc.h>

#define MAX_UARTS	2

typedef struct {
    void	*regs;		/* Register base address */
    uint	gap;		/* Address Interval */
    uint	baud_base;	/* Interrupt level */
} uart_params_t;

typedef struct {
	uint8			num_tty;
	uart_params_t	uart[MAX_UARTS];
} uarts_t;

static uarts_t uarts;
static NS16550_CHAN  ns16550Chan[MAX_UARTS];

static void register_uart(void *regs, uint irq, uint baud_base, uint reg_shift);
int num_TTY(void);

/******************************************************************************
*
* register_uart:
*
*		si_serial_init callback function. This populates the uarts struct
*		and maintains a count of how many uarts are supported
*/
static void 
register_uart(void *regs, uint irq, uint baud_base, uint reg_shift)
{

	if (uarts.num_tty >= MAX_UARTS)
		return;

	uarts.uart[uarts.num_tty].regs = regs;
	uarts.uart[uarts.num_tty].gap = (1 << reg_shift);
	uarts.uart[uarts.num_tty].baud_base = baud_base;

	uarts.num_tty++;
}

/******************************************************************************
*
* num_TTY:
*
* 		Vx uses #define to identify how many uarts are supported, we point the
*		define at this routine to dynamically determine how many uarts there are.
*		This routine is also responsible for initialising the uarts.
*/
int
num_TTY(void) {
	si_t *sih;
if (!uarts.num_tty) {
	sih = si_kattach(SI_OSH);
	ASSERT(sih);

	bzero((char*)&uarts, sizeof(uarts_t));

	si_serial_init(sih, register_uart, 0);

	si_detach(sih);

}
	return(uarts.num_tty);
}

/******************************************************************************
*
* sysSerialHwInit - initialize the BSP serial devices to a quiesent state
*
* This routine initializes the BSP serial device descriptors and puts the
* devices in a quiesent state.  It is called from sysHwInit() with
* interrupts locked.
*
* RETURNS: N/A
*/

void sysSerialHwInit (void){
	if (!uarts.num_tty) {
		num_TTY();
	}
	ASSERT(uarts.num_tty);

    ns16550Chan[0].regs		= (UINT8 *) 0xb8000300;
    ns16550Chan[0].regDelta	= 1;

    ns16550Chan[0].xtal		= 25000000;
    ns16550Chan[0].baudRate	= CONSOLE_BAUD_RATE;

    ns16550Chan[1].regs		= (UINT8 *) 0xb8000400;
    ns16550Chan[1].regDelta	= 1;

    ns16550Chan[1].xtal		= 25000000;
    ns16550Chan[1].baudRate	= CONSOLE_BAUD_RATE;
     
    /* reset the chips */
    
    ns16550DevInit (&ns16550Chan[0]);  /* uart int. disabled */
    ns16550DevInit (&ns16550Chan[1]);  /* uart int. disabled */

}


/******************************************************************************
*
* sysSerialInt - BSP specific serial device ISR
*
*/
void sysSerialInt(int unit){

    ns16550Int (&ns16550Chan[unit]);    
}

/******************************************************************************
*
* sysSerialHwInit2 - connect BSP serial device interrupts
*
* This routine connects the BSP serial device interrupts.  It is called from
* sysHwInit2().  Serial device interrupts could not be connected in
* sysSerialHwInit() because the kernel memory allocator was not initialized
* at that point, and intConnect() calls malloc().
*
* RETURNS: N/A
*/

void sysSerialHwInit2 (void)
{
    int i;

    for (i = 0; i < uarts.num_tty; i++)
       {
         if(i==0) 
            intConnect (INUM_TO_IVEC (IV_EXT_ALT1_VEC), sysSerialInt, 0);      
         if(i==1)
            intConnect (INUM_TO_IVEC (IV_EXT_ALT2_VEC), sysSerialInt, 1);
       }      
}

/******************************************************************************
*
* sysSerialHwReset - reset the serial controllers
*
* Shutdown all controllers capable of generating interrupts, especially
* controllers using DMA to transfer channel command blocks.  Most Bug
* monitors presume that a hardware reset precedes entry to the monitor
* code.
*
* RETURNS: N/A.
*/

VOID sysSerialHwReset (void)
    {
    int i;

    for (i = 0; i < uarts.num_tty; i++)
        ns16550DevInit (&ns16550Chan[i]);
    }

/******************************************************************************
*
* sysSerialChanGet - get the SIO_CHAN device associated with a serial channel
*
* This routine gets the SIO_CHAN device associated with a specified serial
* channel.
*
* RETURNS: A pointer to the SIO_CHAN structure for the channel, or ERROR
* if the channel is invalid.
*/
SIO_CHAN * sysSerialChanGet
    (
    int		channel
    )
    {
    switch (channel)
        {
        case 0:
            return ((SIO_CHAN *)&ns16550Chan[0]);
        case 1:
            return ((SIO_CHAN *)&ns16550Chan[1]);
	default:
	    return NULL;
	}
    }

typedef struct
    {
    USHORT vector;                      /* Interrupt vector */
    ULONG  baseAdrs;                    /* Register base address */
    USHORT regSpace;                    /* Address Interval */
    USHORT intLevel;                    /* Interrupt level */
    } NS16550_CHAN_PARAS;

static NS16550_CHAN_PARAS devParas[] = 
   {
   {INT_VEC_IORQ0, 0xb8000300, 1, INT_VEC_IORQ0},
   {INT_VEC_IORQ0, 0xb8000400, 1, INT_VEC_IORQ0}
   };

static volatile int dummyx;
#define SS_CHAN         0

#define ADDR_XOR_3 0

#define UART_REG(reg,chan) \
                (devParas[chan].baseAdrs + reg * devParas[chan].regSpace)

UCHAR sysSerialInByte(int addr )
{
    dummyx = (*(volatile unsigned char*)0xb8000000);
    return *(volatile unsigned char *) (addr^ADDR_XOR_3);
}

void sysSerialOutByte (int addr,UCHAR c)
{
    unsigned char x;
    dummyx = (*(volatile unsigned char*)0xb8000000);
    *(volatile unsigned char *) (addr^ADDR_XOR_3) = c;

    /* PR13509 - CHIPC hang on UART register write:
     * The workaround is that MIPS must read any SB location that isn't
     * in the UART after any UART write. The location read doesn't even
     * have to be in chipcommon. It's believed that this bug does not
     * affect any other cores accessing UART.
     */

     x = (*(volatile unsigned char*)0xb8000000);
}

void sysSerialDelay(void)
{
    volatile int i;
    for (i = 0; i < 0x10000; i++)
        ;
}

void sysSerialPutc(int c)
{
    int i = 10000;
    while (!(sysSerialInByte(UART_REG(LSR, SS_CHAN)) & 0x40) && i--)
        ;
    sysSerialOutByte(UART_REG(THR, SS_CHAN), c);
}

int sysSerialGetc(void)
{
    while (!(sysSerialInByte(UART_REG(LSR, SS_CHAN)) & 0x01))
        ;
    return sysSerialInByte(UART_REG(RBR, SS_CHAN));
}

void sysSerialPrintString(char *s)
{
    int c, il;

    il = intLock();
    /* sysSerialUARTPollInit (SS_CHAN);*/

    while ((c = *s++) != 0) {
        if (c == '\n')
            sysSerialPutc('\r');
        sysSerialPutc(c);
    }
    sysSerialDelay();   /* Allow last char to flush */
    intUnlock(il);
}

void sysSerialPrintHex(UINT32 value, int cr)
{
    const char hex[] = "0123456789abcdef";
    int i, il;
    il = intLock();
    /* sysSerialUARTPollInit (SS_CHAN);*/
    for (i = 0; i < 8; i++)
        sysSerialPutc(hex[value >> (28 - i * 4) & 0xf]);
    if (cr) {
        sysSerialPutc('\r');
        sysSerialPutc('\n');
    }
    sysSerialDelay();   /* Allow last char to flush */
    intUnlock(il);
}


