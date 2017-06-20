#include "vxWorks.h"
#include "intLib.h"
#include "excLib.h"
#include "epic.h"
#include "mpc107.h"
#include "stdio.h"
#include "string.h"

extern ULONG 	sysEUMBBARRead(ULONG regNum);
extern void 	sysEUMBBARWrite(ULONG regNum, ULONG regVal);

IMPORT STATUS	(*_func_intConnectRtn) (VOIDFUNCPTR *, VOIDFUNCPTR, int);
IMPORT int	(*_func_intEnableRtn) (int);
IMPORT int	(*_func_intDisableRtn)  (int);

/* $Id: epic.c,v 1.2 2011/07/21 16:14:08 yshtil Exp $
 * Typedef: epic_table_t
 * Purpose: Defines an entry in the EPIC table indexed by vector.
 *	    Each entry represents a vector that is permanently
 *	    assigned to a specific function as follows (see definitions
 *	    for EPIC_VECTOR_xxx):
 *
 *	Vector(s)	Assignment
 *	---------	-------------------
 *	0-15		External (and unused serial interrupts)
 *	16-19		Global timers
 *	20		I2C
 *	21-22		DMA
 *	23		I2O message
 *	24		UART1
 *	25		UART2
 *
 * NOTE: Higher priority numbers represent higher priority.
 */

typedef struct epic_table_s {
    int		et_prio;	/* Interrupt priority, fixed */
    VOIDFUNCPTR	et_func;	/* Function to call, set when connected */
    int		et_par;		/* Param to func, set when connected */
    UINT32	et_vecadr;	/* Address of EPIC vector register */
    UINT32	et_desadr;	/* Address of EPIC destination register */
} epic_table_t;

static epic_table_t epic_table[EPIC_VECTOR_USED] = {
  /* 0  */  {  5, 0, 0, EPIC_EX_INTx_VEC_REG(0),  EPIC_EX_INTx_DES_REG(0)  },
  /* 1  */  {  5, 0, 0, EPIC_EX_INTx_VEC_REG(1),  EPIC_EX_INTx_DES_REG(1)  },
  /* 2  */  {  5, 0, 0, EPIC_EX_INTx_VEC_REG(2),  EPIC_EX_INTx_DES_REG(2)  },
  /* 3  */  {  5, 0, 0, EPIC_EX_INTx_VEC_REG(3),  EPIC_EX_INTx_DES_REG(3)  },
  /* 4  */  { 10, 0, 0, EPIC_EX_INTx_VEC_REG(4),  EPIC_EX_INTx_DES_REG(4)  },
  /* 5  */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(5),  EPIC_SR_INTx_DES_REG(5)  },
  /* 6  */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(6),  EPIC_SR_INTx_DES_REG(6)  },
  /* 7  */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(7),  EPIC_SR_INTx_DES_REG(7)  },
  /* 8  */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(8),  EPIC_SR_INTx_DES_REG(8)  },
  /* 9  */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(9),  EPIC_SR_INTx_DES_REG(9)  },
  /* 10 */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(10), EPIC_SR_INTx_DES_REG(10) },
  /* 11 */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(11), EPIC_SR_INTx_DES_REG(11) },
  /* 12 */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(12), EPIC_SR_INTx_DES_REG(12) },
  /* 13 */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(13), EPIC_SR_INTx_DES_REG(13) },
  /* 14 */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(14), EPIC_SR_INTx_DES_REG(14) },
  /* 15 */  {  5, 0, 0, EPIC_SR_INTx_VEC_REG(15), EPIC_SR_INTx_DES_REG(15) },
  /* 16 */  {  8, 0, 0, EPIC_TMx_VEC_REG(0),      EPIC_TMx_DES_REG(0)      },
  /* 17 */  {  5, 0, 0, EPIC_TMx_VEC_REG(1),      EPIC_TMx_DES_REG(1)      },
  /* 18 */  {  5, 0, 0, EPIC_TMx_VEC_REG(2),      EPIC_TMx_DES_REG(2)      },
  /* 19 */  {  5, 0, 0, EPIC_TMx_VEC_REG(3),      EPIC_TMx_DES_REG(3)      },
  /* 20 */  {  5, 0, 0, EPIC_I2C_INT_VEC_REG,     EPIC_I2C_INT_DES_REG     },
  /* 21 */  {  5, 0, 0, EPIC_DMA0_INT_VEC_REG,    EPIC_DMA0_INT_DES_REG    },
  /* 22 */  {  5, 0, 0, EPIC_DMA1_INT_VEC_REG,    EPIC_DMA1_INT_DES_REG    },
  /* 23 */  {  5, 0, 0, EPIC_MSG_INT_VEC_REG,     EPIC_MSG_INT_DES_REG     },
  /* 24 */  {  4, 0, 0, EPIC_UART1_INT_VEC_REG,   EPIC_UART1_INT_DES_REG   },
  /* 25 */  {  4, 0, 0, EPIC_UART2_INT_VEC_REG,   EPIC_UART2_INT_DES_REG   },
};

STATUS
epic_int_configure(int vec, int prio, VOIDFUNCPTR func, int par)
/*
 * Function: 	epic_int_configure
 * Purpose:	Configure an EPIC interrupt with a given vector and prio.
 * Parameters:	vec - interrupt vector number/level
 *		prio - PPC interrupt priority
 *		func - function to call
 *		par - parameter passed to func on interrupt.
 * Returns:	OK/ERROR
 */
{
    epic_table_t *et = &epic_table[vec];

    if (NULL != et->et_func || vec < 0 || vec >= EPIC_VECTOR_USED) {
	return(ERROR);
    }
    et->et_func	 = func;
    et->et_par	 = par;
    et->et_prio	 = prio;
    return(OK);
}

static STATUS
epic_int_connect(VOIDFUNCPTR *v_ptr, VOIDFUNCPTR rtn, int par)
/*
 * Function: 	epic_int_connect
 * Purpose:	Connect an epic interrupt.
 * Parameters:	v_ptr - vector #.
 *		rtn - function to call.
 *		par - parameter passed to function.
 * Returns:	OK/ERROR
 * Notes:	This routine uses the same priority as found in the table.
 */
{
    int		vec	= (int) v_ptr;

    if (vec < 0 || vec >= EPIC_VECTOR_USED)
	return(ERROR);

    return epic_int_configure(vec, epic_table[vec].et_prio, rtn, par);
}

static	void
epic_int_handler(void)
/*
 * Function: 	epic_int_handler
 * Purpose:	Handle an interrupt from the epic.
 * Parameters:	None.
 * Returns:	Nothing
 */
{
    int	vec;
    ULONG	o_pri;			/* Old priority */
    epic_table_t *et;

    vec = epic_read(EPIC_PROC_INT_ACK_REG) ;
    et  = epic_table + vec;
    if ((vec >= 0) && (vec < EPIC_VECTOR_USED) && (NULL != et->et_func)) {
	o_pri = epic_read(EPIC_PROC_CTASK_PRI_REG); 	/* Save prev level */
	epic_write(EPIC_PROC_CTASK_PRI_REG, et->et_prio);/* Set task level */ 
	intUnlock (_PPC_MSR_EE);			/* Allow nesting */
	et->et_func(et->et_par);			/* Call handler */
	intLock();
	epic_write(EPIC_PROC_CTASK_PRI_REG, o_pri);	/* Restore  level */
    }
    epic_write(EPIC_PROC_EOI_REG, 0x0);			/* Signal EOI */
}

STATUS
epic_int_enable(int vec)
/*
 * Function: 	epic_int_enable
 * Purpose:	Enable interrupts for the specified vector.
 * Parameters:	vec - vector # to enable interrupts for.
 * Returns:	OK/ERROR
 */
{
    epic_table_t	*et = epic_table + vec;
    UINT32		vecreg;

    if (NULL == et->et_func) {
	return(ERROR);
    }

#if 0
    printf("EPIC: interrupt enable --- <vec=0x%x level=0x%x prio=0x%x>\n",
	   vec, et->et_level, et->et_prio);
#endif

    /*
     * Look up the interrupt level for the vector and enable that level
     * setting the correct vector #, priority, 
     */

    vecreg = (et->et_prio << EPIC_VECREG_PRIO_SHFT |
	      vec << EPIC_VECREG_VEC_SHFT);

    if (vec >= EPIC_VECTOR_EXT0 && vec <= EPIC_VECTOR_EXT4)
	vecreg |= EPIC_VECREG_S;	/* Set sense bit */

    epic_write(et->et_vecadr, vecreg);

    /* Direct the interrupt to this processor */
    epic_write(et->et_desadr, 1);

    return(OK);
}

STATUS
epic_int_disable(int vec)
/*
 * Function: 	epic_int_disable
 * Purpose:	Disable interrupts for the specified vector.
 * Parameters:	vec - vector number to disable.
 * Returns:	OK/ERROR
 */
{
    epic_table_t	*et = epic_table + vec;
    UINT32		vecreg;

    if (NULL == et->et_func) {
	return(ERROR);
    }

    /* Set mask bit */

    vecreg = epic_read(et->et_vecadr);
    vecreg |= EPIC_VECREG_M;
    epic_write(et->et_vecadr, vecreg);

    return(OK);
}

void
epic_init(void)
/*
 * Function: 	epic_init
 * Purpose:	Initialize the EPIC for interrupts.
 * Parameters:	None.
 * Returns:	Nothing.
 */
{
    ULONG	tmp;

    /* Set reset bit and wait for reset to complete */

    epic_write(EPIC_GLOBAL_REG, EPIC_GCR_RESET);
    while (epic_read(EPIC_GLOBAL_REG) & EPIC_GCR_RESET)
	;

    /* Discrete ints please */

    epic_write(EPIC_GLOBAL_REG, EPIC_GCR_M);

    /* Set direct interrupts (disable serial interrupts) */

    tmp = epic_read(EPIC_INT_CONF_REG);
    epic_write(EPIC_INT_CONF_REG, tmp & ~EPIC_ICR_SIE);

    /* Clear all pending interrupt */

    while (epic_read(EPIC_PROC_INT_ACK_REG) != 0xff);

    /* Broadcom uses EPIC only, no winpic stuff. */

    _func_intEnableRtn	= epic_int_enable;
    _func_intDisableRtn = epic_int_disable;
    _func_intConnectRtn = epic_int_connect;

    /* Connect the real PPC vector to our handler. */

    excIntConnect ((VOIDFUNCPTR *) _EXC_OFF_INTR, epic_int_handler);

    /* Set current task priority to 0 */

    epic_write(EPIC_PROC_CTASK_PRI_REG, 0);
}
