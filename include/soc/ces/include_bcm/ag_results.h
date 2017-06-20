
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name: 
    general include files    
File Name: 
    ag_results.h

File Description: 
    contains system wide return codes of AG functions.

$Revision: 1.1.2.1 $  - Visual SourceSafe automatic revision number 

History:
    Date            Name            Comment
    ----            ----            -------
    3/7/00          Gadi Admoni     initial version
    22/1/00         Gadi Admoni     Added AG_ to all.
    13/08/02        Raanan Refua    Addition of Capitator Driver base code,
                                    Header Insertion Program base code
    17/9/02         Raanan Refua    Added AG_E_BAD_ALIGNMENT to error codes
  28/7/2004  Edi Shmukler   Added AG_RESULT()
*******************************************************************/
#ifndef AG_RESULTS_H 
#define AG_RESULTS_H 

#ifdef AG_MNT
	/* disable annoying NT compilation warning */
	#pragma warning(disable:4786)
#endif

#include "ag_basic_types.h"

/*------------------------------**
** Base Error and success codes **
**------------------------------*/

/* general success */
#define AG_S_OK     ((AgResult)0x00000000)

/* general error */
#define AG_E_FAIL   ((AgResult)0x80000000)

/*-------------------------------------------------**
** General Macros to test for fail/success result. **
**-------------------------------------------------*/
#define AG_FAILED(x)        ((x) & AG_E_FAIL)
#define AG_SUCCEEDED(x)     (!((x) & AG_E_FAIL))

# define AG_RESULT(r) ((AG_FALSE == (r))? AG_E_FAIL: AG_S_OK)

/*------------------------------------------------------------------------------------**
** Module Base Codes                                                                  **
** -----------------                                                                  **
** A function can return a general result code (from the list of general result codes **
** below) or a module specific code.                                                  **
**                                                                                    **
** Example:                                                                           **
** to create a classification specific code use the following syntax:                 **
**                                                                                    **
** #define AG_E_CLS_BAD_TAG     (AG_E_FAIL | (AG_RES_CLS_BASE + 1))                      **
**------------------------------------------------------------------------------------*/

#define AG_RES_CLS_BASE   0x100 /* classification base code */
#define AG_RES_QOS_BASE   0x200 /* QOS base code */
#define AG_RES_BMD_BASE   0x300 /* buffer manager driver base code */
#define AG_RES_SCH_BASE   0x400 /* scheduler base code */
#define AG_RES_QMD_BASE   0x500 /* queue manager base code */
#define AG_RES_ITH_BASE   0x600 /* interrupt handler base code */
#define AG_RES_AGOS_BASE  0x700 /* agos (OS abstraction layer) base codes */
#define AG_RES_FLW_BASE   0x800 /* Flow handling base codes */
#define AG_RES_MMI_BASE   0x900 /* MMI base codes */
#define AG_RES_SIM_BASE   0x1000 /* simulation base code */
#define AG_RES_INT_BASE   0x1100 /* intc (interrupt controler) base code */
#define AG_RES_TIM_BASE   0x1200 /* Timers base code */
#define AG_RES_RULE_BASE  0x1300 /* Rule base code */
#define AG_RES_MNG_BASE   0x1400 /* Management Base - MMI is apart!!!*/
#define AG_RES_FLASH_BASE 0x1500 /* flash driver base code */
#define AG_RES_DBC_BASE   0x1600 /* DBConfig base codes: */
#define AG_RES_DBC_BASE_2 0x1700 /* DBConfig takes sequential error codes*/
#define AG_RES_DBC_BASE_3 0x1800 /* for the core objects in it */
#define AG_RES_DBC_BASE_4 0x1900 /* and future Shrinkwrap's objects  */
#define AG_RES_DBC_BASE_5 0x2000 /* yet to come */
#define AG_RES_DBC_BASE_6 0x2100
/*Raanan Refua 13/08/2002*/
#define AG_RES_CAP_BASE   0x2200 /* Capitator Driver base code */
#define AG_RES_HIP_BASE   0x2300 /* Header Insertion Program base code*/
#define AG_RES_SHWRAP_BASE_0 0x2400 /* ShrinkWrap applications base-0 */
#define AG_RES_SHWRAP_BASE_1 0x2500 /* ShrinkWrap applications base-1 */
#define AG_RES_SHWRAP_BASE_2 0x2600 /* ShrinkWrap applications base-2 */
#define AG_RES_UDL_BASE   0x2700 /* UpDownload base code */
#define AG_RES_RCP_BASE   0x2800
#define AG_RES_SHWRAP_CLI_BASE_0   0x2900 /* for CLI return code */
#define AG_RES_AGGR_BASE_0   0x3000 /* for CLI return code */
#define AG_RES_ND_BASE    0x3100 /* Nemo/Neptune driver return codes base */
#define AG_RES_FILE_DOWNLOAD_BASE  0x3200
#define AG_RES_DS0_BUNDLE_BASE     0x3300
#define AG_RES_DS0_BUNDLE_CONT_BASE 0x3400
#define AG_RES_CLOCK_BASE           0x3500      /*clock app */
#define AG_RES_SW_LOGGER_BASE 0x3600
#define AG_RES_NEMO_BASE 0x3700         /* Base for all nemo related modules */
#define AG_RES_MGMT_BASE 0x3800         /* Base for general management */
#define AG_RES_ARP_BASE  0x3900         /*Base for all ARP related modules */
#define AG_RES_ETH_APP_BASE  0x4000         /*Base for all ETH application related modules */
#define AG_RES_TDM_APP_BASE  0x4100			/*Base for all TDM application related modules */
#define AG_RES_APS_APP_BASE  0x4200			/*Base for all APS application related modules */
#define AG_RES_PTP_BASE 	 0x4300
                                            
/*---------------------**
** general Success codes **
**---------------------*/

#define AG_S_PENDING              (AG_S_OK | 1) /* successful but uncompleted asynchronous call.*/
#define AG_S_EMPTY                (AG_S_OK | 2) 
#define AG_S_EXISTS               (AG_S_OK | 3) /* element was not inserted because it is already in container */
#define AG_S_NULL_PTR             (AG_S_OK | 4) /* valid null pointer */
#define AG_S_CHANGED              (AG_S_OK | 5) /*The function test for state change and it changes*/

/*---------------------**
** general Error codes **
**---------------------*/

#define AG_E_FATAL                  (AG_E_FAIL | 1)     /*0x1 */
#define AG_E_OUT_OF_MEM             (AG_E_FAIL | 2)     /*0x2 */
#define AG_E_BAD_PARAMS             (AG_E_FAIL | 3)     /*0x3 */
#define AG_E_NOT_FOUND              (AG_E_FAIL | 4)     /*0x4 */
#define AG_E_MAX_EXCEEDED           (AG_E_FAIL | 5)     /*0x5 */
#define AG_E_OUT_OF_RANGE           (AG_E_FAIL | 6)     /*0x6 */
#define AG_E_NOT_INIT               (AG_E_FAIL | 7)     /*0x7 */
#define AG_E_FULL                   (AG_E_FAIL | 8)     /*0x8 */
#define AG_E_EMPTY                  (AG_E_FAIL | 9)     /*0x9 */
#define AG_E_INVALID_SIZE           (AG_E_FAIL | 10)    /*0xA */
#define AG_E_INVALID_OPERATION      (AG_E_FAIL | 11)    /*0xB */
#define AG_E_TIMEOUT                (AG_E_FAIL | 12)    /*0xC */
#define AG_E_NULL_PTR               (AG_E_FAIL | 13)    /*0xD */
#define AG_E_BUSY                   (AG_E_FAIL | 14)    /*0xE */
#define AG_E_ALREADY_INIT           (AG_E_FAIL | 15)    /*0xF */
#define AG_E_BAD_ALIGNMENT          (AG_E_FAIL | 16)    /*0x10 */
#define AG_E_NOT_SUPPORTED_ON_BGA   (AG_E_FAIL | 17)    /*0x11 */
#define AG_E_SYSTEM_EXCEPTION       (AG_E_FAIL | 18)    /*0x12 */
#define AG_E_INVALID_RESPONSE       (AG_E_FAIL | 19)    /*0x13 */
#define AG_E_FILE_NOT_OPEN          (AG_E_FAIL | 20)    /*0x14 */
#define AG_E_OPERATION_NOT_SUPPORTED (AG_E_FAIL | 21)   /*0x15 */

#endif  /* AG_RESULTS_H */
