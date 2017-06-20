/*
 * $Id: nlmcmlogger.h,v 1.2.8.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 


#ifndef INCLUDED_NLMCMLOGGER_H
#define INCLUDED_NLMCMLOGGER_H

#ifndef NLMPLATFORM_BCM 
#include <nlmcmportable.h>
#else
#include <soc/kbp/common/nlmcmportable.h>
#endif

/* Define "NLMCM_LOGGING_ENABLE" to enable the logging */
#if NLM_CMN_DC
#define NLMCM_LOGGING_ENABLE
#endif

/* All the modules of code*/
typedef enum
{
    NLMNS_MODULE_NONE = 0,
    NLMNS_MODULE_FTM,
    NLMNS_MODULE_9KFTM,
    NLMNS_MODULE_TRIE,
    NLMNS_MODULE_LSN,
    NLMNS_MODULE_MCM,
    NLMNS_MODULE_SES,
    NLMNS_MODULE_IXM,
    NLMNS_MODULE_ALL
}NlmNsModules;

/*Different logging level*/
typedef enum
{
    NLMCM_LOG_NONE = 0,
    NLMCM_LOG_WARNING,
    NLMCM_LOG_ERROR,
    NLMCM_LOG_INFO,
    NLMCM_LOG_DEBUG
}NlmCmLogLevel;



/*  This function initializes the gNlmCmLogger with the given value */
void NlmCmLogger__Create(NlmCmLogLevel logLevel, char *logFile);

void NlmCmLogger__Destroy(void);

/* Get the logging level */
int NlmCmLogger__GetLevel(void);

/* Set the logging level. It returns the older version, 
so you can store and reset it at the end of the block */
nlm_u32 NlmCmLogger__SetLevel(NlmCmLogLevel logLevel);

/* Set logging for given module */
void NlmCmLogger__SetModule(NlmNsModules logModule);

void NlmCmLogger__DisableModule(NlmNsModules module);


/* Log the data if logging is enabled for that module */
void NlmCmLogger__Log (NlmNsModules module,
                       NlmCmLogLevel logLevel, const char *format, ...);

/*Log the prefix if the logging for the module is enabled */
void NlmCmLogger__LogPrefix(
    NlmNsModules module,
    NlmCmLogLevel logLevel,
    nlm_u16 inuse,
    nlm_u8* prefix_p);


#ifdef NLMCM_LOGGING_ENABLE


#define NLMCM_LOGGER_CREATE(x1, x2)                 NlmCmLogger__Create(x1, x2) 
#define NLMCM_LOGGER_DESTROY()                      NlmCmLogger__Destroy()
#define NLMCM_LOGGER_GETLEVEL()                     NlmCmLogger__GetLevel()
#define NLMCM_LOGGER_SETLEVEL(x)                    NlmCmLogger__SetLevel(x)
#define NLMCM_LOGGER_SETMODULE(x)                   NlmCmLogger__SetModule(x)
#define NLMCM_LOGGER_DISABLE_MODULE(x)              NlmCmLogger__DisableModule(x)
#define NLMCM_LOGGER_LOG                            NlmCmLogger__Log
#define NLMCM_LOGGER_LOGPREFIX                      NlmCmLogger__LogPrefix

#else

#define NLMCM_LOGGER_CREATE(x1, x2)                 
#define NLMCM_LOGGER_DESTROY()              
#define NLMCM_LOGGER_GETLEVEL()             0
#define NLMCM_LOGGER_SETLEVEL(x)                0
#define NLMCM_LOGGER_SETMODULE(x)       
#define NLMCM_LOGGER_DISABLE_MODULE(x)  
#define NLMCM_LOGGER_LOGPREFIX(x1,x2,x3,x4)             


#ifdef NLMPLATFORM_WIN32
#define NLMCM_LOGGER_LOG 
#else
/* Dummy function call since varidiac function macros are not suppoted by ANSI,
#define NLMCM_LOGGER_LOG(...)   
*/
extern void NlmCmLogger__Dummy( NlmNsModules module, ...);


#define NLMCM_LOGGER_LOG        NlmCmLogger__Dummy

#endif /* NLMPLATFORM_WIN32 */

#endif /* NLMCM_LOGGING_ENABLE*/

#endif/*INCLUDED_NLMCMLOGGER_H*/

