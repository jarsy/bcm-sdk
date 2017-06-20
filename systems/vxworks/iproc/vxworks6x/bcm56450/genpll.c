/*
 * Copyright (C) 2013, Broadcom Corporation. All Rights Reserved. 
 *  
 * Permission to use, copy, modify, and/or distribute this software for any 
 * purpose with or without fee is hereby granted, provided that the above 
 * copyright notice and this permission notice appear in all copies. 
 *  
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION 
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

#include <vxWorks.h> 

#define ChipcommonA_CoreCtrl	            		0x18000008
#define ChipcommonA_CoreCtrl__UARTClkOvr    		0
#define ChipcommonA_ClkDiv				0x180000a4
#define ChipcommonA_ClkDiv__UartClkDiv_R		0
#define ChipcommonA_ClkDiv__UartClkDiv_WIDTH		8

#define APBW_IDM_IDM_IO_CONTROL_DIRECT			0x18131408
#define APBW_IDM_IDM_IO_CONTROL_DIRECT__UARTClkSel	17

#define CONFIG_SYS_REF_CLK			(25000000) /*Reference clock = 25MHz */

static uint32_t reg32_read(volatile uint32_t *reg)
{
  return *reg;
}

uint32_t iproc_get_axi_clk(uint32_t refclk)
{
    return(495000000);
}

uint32_t iproc_get_uart_clk(uint32_t uart)
{
    uint32_t uartclk, uartclkovr, uartclksel; 

    uartclk = iproc_get_axi_clk(CONFIG_SYS_REF_CLK) / 4; /* APB clock */

    if (uart < 2) {
      /* CCA UART */
      uartclkovr = (reg32_read((volatile uint32_t *)ChipcommonA_CoreCtrl) 
		    >> ChipcommonA_CoreCtrl__UARTClkOvr) & 0x01;
      uartclksel = (reg32_read((volatile uint32_t *)APBW_IDM_IDM_IO_CONTROL_DIRECT) >> 
		    APBW_IDM_IDM_IO_CONTROL_DIRECT__UARTClkSel) & 0x01;

      if(!uartclkovr) {
	if(uartclksel) {
	  uartclk /= ((reg32_read((volatile uint32_t *)ChipcommonA_ClkDiv) >> 
		      ChipcommonA_ClkDiv__UartClkDiv_R) & 
		      ((1 << ChipcommonA_ClkDiv__UartClkDiv_WIDTH) - 1));
	}
	else{
	  uartclk = CONFIG_SYS_REF_CLK; /* Reference clock */
	}
      }
    }
    return(uartclk);
}

