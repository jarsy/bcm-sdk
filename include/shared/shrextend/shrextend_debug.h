/** \file shrextend_debug.h
 *
 * Generic macros for tracing function call trees.
 *
 * The main principle is to establish a single point of exit for each
 * function, and then combine this with a standard method of logging
 * error conditions.
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef SHREXTEND_DEBUG_H_INCLUDED
/** { */
#define SHREXTEND_DEBUG_H_INCLUDED

#include <shared/bsl.h>
#include <shared/bsltypes.h>
#include <shared/shrextend/shrextend_error.h>
#include <sal/core/alloc.h>
/**
 * \brief This macro is the 'unit' value to use when, actually,
 * there is no relevant unit. This may, for example, be the case
 * fo general purpose utilities, like bitmap handling.
 * See \ref SHR_FUNC_INIT_VARS
 * We need to use a value which is recognized by BSL.
 * See bslsink_out()
 */
#define NO_UNIT              BSL_UNIT_UNKNOWN
/**
 * \brief This macro is an empty string which is used to fill in
 * the required four parameters for the various macros below.
 */
#define EMPTY                ""
/*
 * DNX layer/source log macros.
 * Apart from other potential operations, these macros convert from
 * DNX (JR2) system to classic JR1 system
 * {
 */
/**
 * \brief If NO_LOG_INFO_AND_DOWN is set to a non zero value
 * then the following macros are empty and do not print (or
 * even calculate their variables). This may be useful when
 * the calculation of the variables, for the formatting string,
 * are time consuming.
 */
#define NO_LOG_INFO_AND_DOWN 0
/**
 * \brief For each of the following LOG_*_EX macro's, there are 6 input
 * parameters. See details below.
 *
 * \par  DIRECT INPUT
 *   \param [in] _log_module      -
 *     A combination of 'layer' and 'source' to indicate the identifying
 *     parameters of this invocation. See, for example, BSL_LS_BCMDNX_INITSEQDNX
 *     in bslenum.h
 *   \param [in] _format_string   -
 *     Formatting string to use for diaplay of the following 4 paramaters.
 *   \param [in] _p1
 *   \param [in] _p2
 *   \param [in] _p3
 *   \param [in] _p4
 *     Four parameters to use withing '_format_string'
 * \par  INDIRECT INPUT
 *   BSL system
 * \par  INDIRECT OUTPUT
 *   Formatted string which may be printed to a file or to the screen, depending    \n
 *   on setup (e.g., severity)
 */
#define LOG_FATAL_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)    \
  LOG_FATAL(_log_module,                                             \
    (BSL_META_U(_func_unit,_format_string),_p1,_p2,_p3,_p4)  )
#define LOG_ERROR_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)    \
  LOG_ERROR(_log_module,                                             \
    (BSL_META_U(_func_unit,_format_string),_p1,_p2,_p3,_p4)  )
#define LOG_WARN_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)     \
  LOG_WARN(_log_module,                                              \
    (BSL_META_U(_func_unit,_format_string),_p1,_p2,_p3,_p4)  )

#if !NO_LOG_INFO_AND_DOWN
/** { */
#define LOG_INFO_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)     \
  LOG_INFO(_log_module,                                              \
    (BSL_META_U(_func_unit,_format_string),_p1,_p2,_p3,_p4)  )
#define LOG_VERBOSE_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)  \
  LOG_VERBOSE(_log_module,                                           \
    (BSL_META_U(_func_unit,_format_string),_p1,_p2,_p3,_p4)  )
/**
 * \brief
 * This macro prints input message (stuff_) plus 'prefix' of
 * meta data such as 'procedure name' or 'line no.' provided the
 * severity level for the specified 'layer'/'source' between
 * 'bslSeverityDebug' and 'bslSeverityFatal'.
 * \par DIRECT INPUT:
 *   \param [in] _format_string
 *     String to display if print is not filtered. MUST
 *     be encapsulated in double quotes and have FOUR entries.
 *   \param [in] _p1
 *   \param [in] _p2
 *   \param [in] _p3
 *   \param [in] _p4
 *     Four parameters to display via the formatting string
 *   \param [in] _log_module
 *     Module (layer/source) to report on.
 * \par INDIRECT INPUT:
 *   Callback procedures retrieved using                            \n
 *     get_proc_get_prefix_range_min_max, get_proc_set_prefix_range_min_max \n
 * \remark
 * Printing of 'prefix' depends on the application that sets up BSL.
 * In our case, this is 'diag'. The application is assumed to print
 * print the 'prefix' provided the BSL 'severity' is within some
 * range. This range can be read (and set) via callback procedures
 * which the application is assumed to have invoked before this
 * macro is invoked. The application, then, calls, at
 * initialization, procedures set_proc_get_prefix_range_min_max()
 * and set_proc_set_prefix_range_min_max(). See bsldnx_mgmt_init().
 */
#define LOG_DEBUG_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)  \
{                                                                  \
  bsl_severity_t sev_min, sev_max ;                                \
  BSLDNX_GET_PREFIX_RANGE_MIN_MAX get_proc ;                       \
  BSLDNX_SET_PREFIX_RANGE_MIN_MAX set_proc ;                       \
  get_proc_get_prefix_range_min_max(&get_proc) ;                   \
  get_proc_set_prefix_range_min_max(&set_proc) ;                   \
  if (get_proc && set_proc)                                        \
  {                                                                \
    get_proc(&sev_min,&sev_max) ;                                  \
    set_proc(bslSeverityOff + 1,bslSeverityDebug) ;                \
  }                                                                \
  LOG_DEBUG(_log_module,                                           \
    (BSL_META_U(_func_unit,_format_string),_p1,_p2,_p3,_p4)  ) ;   \
  if (get_proc && set_proc)                                        \
  {                                                                \
    set_proc(sev_min,sev_max) ;                                    \
  }                                                                \
}
/** } */
#else
/** { */
#define LOG_INFO_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)
#define LOG_VERBOSE_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)
#define LOG_DEBUG_EX(_log_module,_format_string, _p1,_p2,_p3,_p4)
/** } */
#endif
/*
 * }
 */
/**
 * \brief Set severity level for the current layer/source combination.
 *
 * This macro is used to manipulate severity level at run-time for
 * debug purposes.
 *
 * Example:
 *
 * \code{.c}
 * int my_func(int unit, int index, int value)
 * {
 *   int local_idx ;
 *   bsl_severity_t bsl_severity ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   SHR_GET_SEVERITY_FOR_MODULE(bsl_severity) ;
 *   SHR_SET_SEVERITY_FOR_MODULE(bslSeverityDebug) ;
 *   ...
 *
 * exit:
 *   SHR_SET_SEVERITY_FOR_MODULE(bsl_severity) ;
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _bsl_severity
 *     One of the bsl_severity_t levels:
 *     bslSeverityFatal,bslSeverityError,etc.
 * \par INDIRECT INPUT:
 *   \b BSL_LOG_MODULE               - \n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   \b BSL_LAYER_GET,BSL_SOURCE_GET - \n
 *     Macros to extract layer/source from a BSL module. \n
 *   \b Procedure bslenable_set()    - \n
 *     Procedure to place 'severity' in a 'BSL module' variable.
 * \par DIRECT OUTPUT:
 *   * None
 * \par INDIRECT OUTPUT:
 *   New severity level for BSL_LOG_MODULE
 * \remark
 *   If severity access procedure ahs not yet been initialized then
 *   no action is taken
 *
 */
#define SHR_SET_SEVERITY_FOR_MODULE(_bsl_severity) \
{                                                \
  int layer, source ;                            \
  BSLENABLE_SET set_proc ;                       \
  get_proc_bslenable_set(&set_proc) ;            \
  layer = BSL_LAYER_GET(BSL_LOG_MODULE) ;        \
  source = BSL_SOURCE_GET(BSL_LOG_MODULE) ;      \
  if (set_proc)                                  \
  {                                              \
    set_proc((bsl_layer_t)layer, (bsl_source_t)source, (bsl_severity_t)_bsl_severity) ; \
  }                                              \
}
/**
 * \brief Get severity level of the current layer/source combination.
 *
 * This macro is used to get base severity level at run-time so it can
 * be returned to its original value after debug session.
 *
 * Example:
 *
 * \code{.c}
 * int my_func(int unit, int index, int value)
 * {
 *   int local_idx ;
 *   bsl_severity_t bsl_severity ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   SHR_GET_SEVERITY_FOR_MODULE(bsl_severity) ;
 *   SHR_SET_SEVERITY_FOR_MODULE(bslSeverityDebug) ;
 *   ...
 *
 * exit:
 *   SHR_SET_SEVERITY_FOR_MODULE(bsl_severity) ;
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _bsl_severity
 *     Variable to be loaded by One of the bsl_severity_t levels:\n
 *     bslSeverityFatal,bslSeverityError,etc.
 * \par INDIRECT INPUT:
 *   Procedure bslenable_get()    - \n
 *     Procedure to extract 'severity' using 'BSL module'.
 *   BSL_LOG_MODULE               - \n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   BSL_LAYER_GET,BSL_SOURCE_GET - \n
 *     Macros to extract layer/source from a BSL module.
 * \par INDIRECT OUTPUT:
 *   Loaded severity level assigned to BSL_LOG_MODULE (of type bsl_severity_t) \n
 *   into _bsl_severity
 * \remark
 *   If severity access procedure variable has not yet been loaded,
 *   then 'bslSeverityOff' is returned.
 */
#define SHR_GET_SEVERITY_FOR_MODULE(_bsl_severity)       \
{                                                        \
  BSLENABLE_GET local_get_proc ;                         \
  get_proc_bslenable_get(&local_get_proc) ;              \
  if (local_get_proc)                                    \
  {                                                      \
    _bsl_severity =                                      \
      local_get_proc((bsl_layer_t)BSL_LAYER_GET(BSL_LOG_MODULE), \
        (bsl_source_t)BSL_SOURCE_GET(BSL_LOG_MODULE)) ;  \
  }                                                      \
  else                                                   \
  {                                                      \
    _bsl_severity = bslSeverityOff ;                     \
  }                                                      \
}
/**
 * \brief Set severity level for the specified layer/source combination.
 *
 * This macro is used to manipulate severity level at run-time for
 * debug purposes.
 *
 * Example:
 *
 * \code{.c}
 * int my_func(int unit, int index, int value)
 * {
 *   int local_idx ;
 *   bsl_severity_t bsl_severity ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_BCMDNX_INITSEQDNX,bsl_severity) ;
 *   SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_BCMDNX_INITSEQDNX,bslSeverityDebug) ;
 *   ...
 *
 * exit:
 *   SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_BCMDNX_INITSEQDNX,bsl_severity) ;
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _module \n
 *     Layer/source combination: \n
 *     For example: BSL_L_BCMDNX,BSL_S_INITSEQDNX
 *   \param [in] _bsl_severity \n
 *     One of the bsl_severity_t levels: \n
 *     bslSeverityFatal,bslSeverityError,etc.
 * \par INDIRECT INPUT:
 *   BSL_LAYER_GET,BSL_SOURCE_GET - \n
 *     Macros to extract layer/source from a BSL module. \n
 *   Procedure bslenable_set()    - \n
 *     Procedure to place 'severity' in a 'BSL module' variable.
 * \par INDIRECT INPUT:
 *   New severity level for BSL_LOG_MODULE
 * \remark
 *   If severity access procedure ahs not yet been initialized then
 *   no action is taken
 *
 */
#define SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(_module,_bsl_severity) \
{                                                \
  int layer, source ;                            \
  BSLENABLE_SET set_proc ;                       \
  get_proc_bslenable_set(&set_proc) ;            \
  layer = BSL_LAYER_GET(_module) ;        \
  source = BSL_SOURCE_GET(_module) ;      \
  if (set_proc)                                  \
  {                                              \
    set_proc((bsl_layer_t)layer, (bsl_source_t)source, (bsl_severity_t)_bsl_severity) ; \
  }                                              \
}
/**
 * \brief Get severity level of the specified layer/source combination.
 *
 * This macro is used to get base severity level at run-time so it can
 * be returned to its original value after debug session.
 *
 * Example:
 *
 * \code{.c}
 * int my_func(int unit, int index, int value)
 * {
 *   int local_idx ;
 *   bsl_severity_t bsl_severity ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_BCMDNX_INITSEQDNX,bsl_severity) ;
 *   SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_BCMDNX_INITSEQDNX,bslSeverityDebug) ;
 *   ...
 *
 * exit:
 *   SHR_SET_SEVERITY_FOR_MODULE_SPECIFIC(BSL_LS_BCMDNX_INITSEQDNX,bsl_severity) ;
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _module
 *     Layer/source combination:
 *     For example: BSL_L_BCMDNX,BSL_S_INITSEQDNX
 *   \param [in] _bsl_severity
 *     Variable to be loaded by One of the bsl_severity_t levels:
 *     bslSeverityFatal,bslSeverityError,etc.
 * \par INDIRECT INPUT:
 *   Procedure bslenable_get()    -\n
 *     Procedure to extract 'severity' using 'BSL module'. \n
 *   BSL_LOG_MODULE               -\n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   BSL_LAYER_GET,BSL_SOURCE_GET -\n
 *     Macros to extract layer/source from a BSL module.
 * \par INDIRECT OUTPUT:
 *   Loaded severity level assigned to BSL_LOG_MODULE (of type bsl_severity_t)\n
 *   into _bsl_severity
 * \remark
 *   If severity access procedure variable has not yet been loaded,
 *   then 'bslSeverityOff' is returned.
 */
#define SHR_GET_SEVERITY_FOR_MODULE_SPECIFIC(_module,_bsl_severity)       \
{                                                        \
  BSLENABLE_GET local_get_proc ;                         \
  get_proc_bslenable_get(&local_get_proc) ;              \
  if (local_get_proc)                                    \
  {                                                      \
    _bsl_severity =                                      \
      local_get_proc((bsl_layer_t)BSL_LAYER_GET(_module), \
        (bsl_source_t)BSL_SOURCE_GET(_module)) ;  \
  }                                                      \
  else                                                   \
  {                                                      \
    _bsl_severity = bslSeverityOff ;                     \
  }                                                      \
}
/**
 * \brief Function entry declarations.
 *
 * This macro must appear in each function right after the local
 * variable declarations and right before the first coding instruction.
 * On 'debug' severity, it prints the details of the location in code
 * (file, line, function, ...) and the text "Enter"
 *
 * Example:
 *
 * \code{.c}
 * int my_func(int unit, int index, int value)
 * {
 *   int local_idx;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _unit -
 *     Value of 'unit' input parameter at entry. If there is no
 *     relevant unit to report, use NO_UNIT.
 * \par INDIRECT INPUT:
 *   Standard error values (Specifically _SHR_E_NONE)               \n
 *   BSL_LOG_MODULE       -                                        \n
 *     Module (layer/source) to report on. Defined at head of file.\n
 *   LOG_DEBUG            -                                        \n
 *     Standard BSL (JR1) LOG macro
 * \par INDIRECT OUTPUT:
 *   int _func_unit -                                              \n
 *     Variable carrying unit value at entry                       \n
 *   int _func_rv   -                                              \n
 *     Variable carrying direct output value throughout procedure
 */
#define SHR_FUNC_INIT_VARS(_unit)    \
  int _func_unit = _unit ;           \
  int _func_rv = _SHR_E_NONE ;       \
  LOG_DEBUG_EX(BSL_LOG_MODULE,       \
    " Enter. %s%s%s%s\r\n" ,EMPTY,EMPTY,EMPTY,EMPTY)

/**
 * \brief Single point of exit code. NO formatted string.
 *
 * This macro can appear at the very bottom of each function, and it
 * must be preceded an 'exit' label and optionally some resource
 * clean-up code (if any).
 *
 * Example:
 *
 * \code{.c}
 * void my_func(int unit, int index, int value)
 * {
 *   int local_idx;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par INDIRECT INPUT:
 *   BSL_LOG_MODULE       - \n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   LOG_DEBUG            - \n
 *     Standard BSL (JR1) LOG macro \n
 *   _func_unit           - \n
 *     Value of 'unit' input parameter at entry
 * \par INDIRECT INPUT:
 *   Minimal formatted string which may also be logged or printed.
 */
#define SHR_FUNC_EXIT                                                 \
  LOG_DEBUG_EX(BSL_LOG_MODULE," Exit. %s%s%s%s\r\n" ,EMPTY,EMPTY,EMPTY,EMPTY ) ; \
  return _func_rv

/**
 * \brief Goto single point of exit.
 *
 * Go to label 'exit', which must be defined explicitly in each
 * function. This macro is normally not called directly, but it
 * is used as part of other error handling macros.
 */
#define SHR_EXIT() goto exit
/*
 * Group of macros which, all, correspond to exit with error
 * {
 */
/**
 * \brief This macro, _SHR_ERR_EXIT, is the actual implementation of \ref SHR_ERR_EXIT
 *   Its documentation applies to \ref SHR_ERR_EXIT below.
 *   This macro is NOT to be used except from within \ref SHR_ERR_EXIT
 */
/**
 * \brief Set exit error code and error-exit on expression error
 * with user log that has any number of parameters on formatting
 * string.
 *
 * The log message will use BSL_LOG_MODULE as the log layer/source.
 *
 * Example:
 *
 * \code{.c}
 * void my_func(int unit, int index, int value)
 * {
 *   int local_idx, var1, var2, result ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ... (do some calculation. if 'result' is zero, then this is a
 *   ...  _SHR_E_PARAM error)
 *   if (reault == 0)
 *   {
 *     SHR_ERR_EXIT(_SHR_E_PARAM," var1 %d var2\r\n", var1,var2 ) ;
 *   }
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr              -\n
 *     Expression representing error. One of shr_error_e (_shr_error_t).
 *     If it evaluates to a 'non error' code then no log is created.
 *   \param [in] _formatting_string -
 *     Formatting string (to display) such that it matches the list
 *     of parameters (_p1, _p2, ...)
 *   \param [in] ...
 *     Any number of parameters (_p1,_p2,_p3,...) to display via the
 *     formatting string.
 * \par INDIRECT INPUT:
 *    BSL_LOG_MODULE       -             \n
 *      Module (layer/source) to report on. Defined at head of file. \n
 *    LOG_ERROR            -             \n
 *      Standard BSL (JR1) LOG macro     \n
 *    _func_unit           -             \n
 *      Value of 'unit' input parameter at entry     \n
 *    BSL_META_U           -                         \n
 *      Standard BSL (JR1) meta data constructor
 * \par INDIRECT OUTPUT:
 *   * Newly set exit error value.
 *   * Full formatted string which may also be logged or printed.
 *   * Exit procedure.
 * \remark
 *   The number of parameters is fleaxible. This is forwarded
 *   to underlying macros using __VA_ARGS__
 *   See also: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 *   (including the explanation for ## before __VA_ARGS__)
 * \remark
 *   Note that with the introduction of the SHR_ERR_EXIT/_SHR_ERR_EXIT
 *   pair, the number of input parameters to _SHR_ERR_EXIT is at least
 *   one and it is not necessary to add the '##' before __VA_ARGS__.
 *   However, we leave it there as an example of usage.
 *   Also, the formatting string assumes an extra parameter of type
 *   'string"!
 */
#define _SHR_ERR_EXIT(_expr,_formatting_string,...) \
{ \
  int _rv = _expr;    \
  LOG_ERROR(BSL_LOG_MODULE,          \
    (BSL_META_U(_func_unit," Error '%s' indicated, " _formatting_string "%s"), shrextend_errmsg_get(_rv), ##__VA_ARGS__) ) ; \
  _func_rv = _rv ;    \
  SHR_EXIT() ;        \
}
/**
 * \brief
 *   Use this macro with the input as described on \ref _SHR_ERR_EXIT above!!!
 *
 *   This is only a 'prelude' for the main macro \ref _SHR_ERR_EXIT and
 *   is only intended TO ADD AN EMPTY PARAMETER.
 *   This is required because a variadic macro does not take invcation with no
 *   parameters when compiled under --pedantic-64
 * \sa https://notmuchmail.org/pipermail/notmuch/2012/007436.html
 */
#define SHR_ERR_EXIT(...) _SHR_ERR_EXIT(__VA_ARGS__, "")
/**
 * \brief If 'expression' is 'error code', error-exit on that error
 * with 'prefix' log info (no user parameters).
 *
 * The expression can be a function call or a fixed error code.
 *
 * The 'prefix includes procedure name, file name, line number, etc.
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr
 *     Expression to evaluate.
 * \par INDIRECT INPUT:
 *    BSL_LOG_MODULE       -             \n
 *      Module (layer/source) to report on. Defined at head of file. \n
 *    LOG_ERROR_EX -\n
 *      Used, with input, for implementation of log. \n
 *    _func_unit           -             \n
 *      Value of 'unit' input parameter at entry     \n
 *    BSL_META_U           -                         \n
 *      Standard BSL (JR1) meta data constructor
 * \par INDIRECT OUTPUT:
 *   * Newly set exit error value.
 *   * Full formatted string which may also be logged or printed.
 *   * Exit procedure.
 */
#define SHR_IF_ERR_EXIT(_expr)      \
do                      \
{                       \
  int _rv = _expr;      \
  if (SHR_FAILURE(_rv)) \
  {                     \
    LOG_ERROR_EX(BSL_LOG_MODULE,                       \
      " Error '%s' indicated ; %s%s%s\r\n" ,           \
      shrextend_errmsg_get(_rv) ,EMPTY,EMPTY,EMPTY) ;  \
    _func_rv = _rv ;  \
    SHR_EXIT() ;        \
  }                     \
} while (0)
/**
 * \brief Check 'expression'. If it is an error code, error-exit with it
 * and with user log that has the standard 4 parameters on formatting
 * string.
 *
 * The log message will use BSL_LOG_MODULE as the log layer/source.
 *
 * Example:
 *
 * \code{.c}
 * void my_func(int unit, int index, int value)
 * {
 *   int var1, var2 ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 *   SHR_IF_ERR_EXIT_WITH_LOG(<invoke some procedure>," var1 %d var2 %s\r\n", var1,var2,EMPTY ) ;
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr              -
 *     Expression representing error. One of shr_error_e (_shr_error_t).
 *     If it evaluates to a 'non error' code then no log is created.
 *   \param [in] _formatting_string
 *     String to display if print is not filtered. MUST be encapsulated in double quotes.
 *   \param [in] _param2
 *   \param [in] _param3
 *   \param [in] _param4
 *     Extra three parameters (_param2,_param3,_param4) to display via the \n
 *     formatting string. Param1 is _expr above.
 * \par INDIRECT INPUT:
 *    BSL_LOG_MODULE       -             \n
 *      Module (layer/source) to report on. Defined at head of file.  \n
 *    LOG_ERROR            -             \n
 *      Standard BSL (JR1) LOG macro     \n
 *    _func_unit           -             \n
 *      Value of 'unit' input parameter at entry     \n
 *    BSL_META_U           -                         \n
 *      Standard BSL (JR1) meta data constructor
 * \par INDIRECT OUTPUT:
 *   * Newly set exit error value.
 *   * Exit procedure.
 */
#define SHR_IF_ERR_EXIT_WITH_LOG(_expr,_formatting_string,_param2,_param3,_param4) \
do                      \
{                       \
  int _rv = _expr;      \
  if (SHR_FAILURE(_rv)) \
  {                     \
    LOG_ERROR_EX(BSL_LOG_MODULE,                   \
      " Error: %s ; " _formatting_string,          \
      shrextend_errmsg_get(_rv) ,_param2,_param3,_param4) ; \
    _func_rv = _rv ;    \
    SHR_EXIT() ;        \
  }                     \
  /* Coverity: the dead code is empty */ \
  /* coverity[dead_error_line:FALSE] */ \
} while (0)
/*
 * }
 */

/**
 * \brief Error-exit on verification procedure fail without user extended log.
 *
 * Evaluate an expression which is the result of invoking a verification
 * procedure. If it evaluates to a standard error code, then log a standard
 * (fixed formatting string) error message and go to the function's single
 * point of exit.
 *
 * The log message will use BSL_LOG_MODULE as the log layer/source.
 *
 * Example:
 *
 * \code{.c}
 * void my_func(int unit, int index, int value)
 * {
 *   int local_idx ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   SHR_INVOKE_VERIFY_DNX(verification_procedure(unit,index,value)) ;
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr
 *     Expression to evaluate. This is also _param1 of the formatting string
 * \par INDIRECT INPUT:
 *   BSL_LOG_MODULE       - \n
 *     Module (layer/source) to report on. Defined at head of file.m \n
 *   _func_unit           - \n
 *     Value of 'unit' input parameter at entry
 * \par INDIRECT INPUT:
 *   Invoked verify procedure and potential print of error message using BSL
 */
#define SHR_INVOKE_VERIFY_DNX(_expr) \
do \
{  \
  int _rv = _expr ;     \
  if (SHR_FAILURE(_rv)) \
  {                     \
    LOG_ERROR_EX(BSL_LOG_MODULE,                       \
      " Error indicated (%s) on VERIFY ; %s%s%s\r\n" , \
      shrextend_errmsg_get(_rv) ,EMPTY,EMPTY,EMPTY) ;  \
    _func_rv = _rv ;      \
    SHR_EXIT() ;          \
  }                     \
} while (0)
/**
 * \brief Check for null-pointer.
 *
 * Check if a pointer is NULL, and if so, log an error (with some fixed extended
 * info, provided specified return code is an error value) and exit. Do print
 * input name of tested pointer.
 *
 * The macro is intended for both input parameter checks and memory
 * allocation errors.
 *
 * \par DIRECT INPUT:
 *   \param [in] _ptr
 *     Pointer to check.
 *   \param [in] _rv
 *     Function return value to use if pointer is NULL.
 *   \param [in] _str
 *     Ascii. Name of tested pointer. MUST be encapsulated in double quotes.
 * \par INDIRECT INPUT:
 *    BSL_LOG_MODULE           - \n
 *      Module to report on. Defined at head of file. \n
 *    LOG_ERROR_EX - \n
 *      Used, with input, for implementation of log. \n
 *    _func_unit               - \n
 *      Value of 'unit' input parameter at entry
 */
#define SHR_NULL_CHECK(_ptr, _rv, _str) \
{ \
  if ((_ptr) == NULL) \
  {                   \
    if (SHR_FAILURE(_rv)) \
    {                     \
      LOG_ERROR_EX(BSL_LOG_MODULE,                       \
        " Error: %s, because %s is NULL %s%s\r\n",       \
        shrextend_errmsg_get(_rv), _str, EMPTY, EMPTY) ; \
    }                     \
    _func_rv = _rv ;      \
    SHR_EXIT() ;          \
  }                       \
  /* Coverity: the dead code is empty */ \
  /* coverity[dead_error_line:FALSE] */ \
}

/**
 * \brief Check for null-pointer.
 *
 * Check if a pointer is NULL, and if not so, fill pointed memory with
 * indicated value
 *
 * \par DIRECT INPUT:
 *   \param [in] _ptr
 *     Pointer to check. If not NULL, pointed memory is loaded by '_value'
 *   \param [in] _value
 *     See '_ptr'.
 * \par INDIRECT INPUT:
 *   None.
 * \par DIRECT OUTPUT:
 *   None.
 * \par INDIRECT OUTPUT:
 *   See '_ptr' in DIRECT INPUT above.
 */
#define SHR_IF_NOT_NULL_FILL(_ptr, _value) \
do                    \
{                     \
  if ((_ptr) != NULL) \
  {                   \
    *(_ptr) = (_value);    \
  }                        \
} while (0)

/**
 * \brief Check for function error state.
 *
 * This macro is a Boolean expression, which evaluates to TRUE, if the
 * macro-based function return value (declared by \ref SHR_FUNC_INIT_VARS)
 * is set to error.
 *
 * It can be used to clean up allocated resources in case of failure.
 *
 * for example:
 *
 * \code{.c}
 * void my_func(int unit,int index, int value)
 * {
 *   int local_idx ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 *
 * exit:
 *   if (SHR_FUNC_ERR())
 *   {
 *         ...
 *   }
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par INDIRECT INPUT:
 *   _func_rv           - \n
 *     Curent value of function's return value.
 * \see
 *   See shr_error_e
 */
#define SHR_FUNC_ERR()    SHR_FAILURE(_func_rv)

/**
 * \brief If 'expression' is 'error code', continue and return fail at the end of the function.
 *
 * The expression can be a function call or a fixed error code.
 *
 * The 'prefix includes procedure name, file name, line number, etc.
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr
 *     Expression to evaluate.
 * \par INDIRECT INPUT:
 *    BSL_LOG_MODULE       -             \n
 *      Module (layer/source) to report on. Defined at head of file. \n
 *    LOG_ERROR_EX -\n
 *      Used, with input, for implementation of log. \n
 *    _func_unit           -             \n
 *      Value of 'unit' input parameter at entry     \n
 *    BSL_META_U           -                         \n
 *      Standard BSL (JR1) meta data constructor
 * \par INDIRECT OUTPUT:
 *   * Newly set exit error value.
 *   * Full formatted string which may also be logged or printed.
 *   * Exit procedure.
 */
#define SHR_IF_ERR_CONT(_expr)      \
{                       \
  int _rv = _expr;      \
  if (SHR_FAILURE(_rv)) \
  {                     \
    LOG_ERROR_EX(BSL_LOG_MODULE,                       \
      " Error '%s' indicated ; %s%s%s\r\n" ,             \
      shrextend_errmsg_get(_rv) ,EMPTY,EMPTY,EMPTY) ;  \
    _func_rv = _rv ;  \
  }                     \
}

/**
 * \brief Warn on expression error.
 *
 * Evaluate an expression and log a warning message if the expression
 * evaluates to a standard error code. The result is assigned to the
 * macro-based function return value (declared by \ref SHR_FUNC_INIT_VARS),
 * but code execution continues immediately below the statement (i.e.,
 * no jump to the single point of exit).
 *
 * The log message will use BSL_LOG_MODULE as the log source, so this
 * name must be assigned to a relevant BSL log source in advance.
 *
 * Example:
 *
 * \code{.c}
 * void my_func(int unit,int index, int value)
 * {
 *   int local_idx ;
 *   int var1, var2 ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *
 *   SHR_IF_ERR_CONT_WITH_LOG(some_other_func(unit)," var1: %d, var2: %d %s",var1,var2,EMPTY) ;
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr -
 *     Expression to evaluate. This is also _param1 of the formatting string
 *   \param [in] _formatting_string -
 *     String to display if print is not filtered. MUST be encapsulated in double quotes.
 *   \param [in] _param2
 *   \param [in] _param3
 *   \param [in] _param4
 *     Extra three parameters (_param2,_param3,_param4) to display via \n
 *     the formatting string. Param1 is _expr above.
 * \par INDIRECT INPUT:
 *   BSL_LOG_MODULE       - \n
 *     Module (layer/source) to report on. Defined at head of file.
 *   LOG_WARN             - \n
 *     Standard BSL (JR1) LOG macro
 *   _func_unit           - \n
 *     Value of 'unit' input parameter at entry
 */
#define SHR_IF_ERR_CONT_WITH_LOG(_expr,_formatting_string,_param2,_param3,_param4) \
{                                                 \
  shr_error_e _rv = _expr ;                       \
  if (SHR_FAILURE(_rv))                           \
  {                                               \
    LOG_WARN_EX(BSL_LOG_MODULE,                   \
      " Error: %s ; " _formatting_string,shrextend_errmsg_get(_rv),_param2,_param3,_param4 ) ; \
      _func_rv = _rv ;                            \
  }                                               \
}

/**
 * \brief Error-exit on expression, BUT - with exception
 *
 * Evaluate an expression and if it evaluates to a standard error code
 * which differs from the exception value, then log a error message
 * and go to the function's single point of exit.
 *
 * The log message will use BSL_LOG_MODULE as the log source, so this
 * name must be assigned to a relevant BSL log source in advance.
 * 
 * Typical exception error codes are _SHR_E_NOT_FOUND and
 * _SHR_E_UNAVAIL, because these error codes may be a simple reflection
 * of the device state or capabilties, which are used to control the
 * logical flow of the function.
 *
 *
 * Example:
 *
 * \code{.c}
 * void my_func(int unit,int index, int value)
 * {
 *   int rv ;
 *   int var1, var2 ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *
 *   rv = some_other_func(unit) ;
 *   SHR_IF_ERR_EXIT_EXCEPT_IF(rv, _SHR_E_UNAVAIL," var1: %d, var2: %d %s",var1,var2,EMPTY) ;
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _expr
 *     Expression to evaluate. This is also _param1 of the formatting string
 *   \param [in] _rv_except
 *     Error code to ignore.
 *   \param [in] _formatting_string
 *     String to display if print is not filtered. MUST be encapsulated in double quotes.
 *   \param [in] _param2
 *   \param [in] _param3
 *   \param [in] _param4
 *     Extra three parameters (_param2,_param3,_param4) to display via the \n
 *     formatting string. Param1 is _expr above.
 * \par INDIRECT INPUT:
 *   BSL_LOG_MODULE       - \n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   LOG_ERROR            - \n
 *     Standard BSL (JR1) LOG macro \n
 *   _func_unit           - \n
 *     Value of 'unit' input parameter at entry
 */
#define SHR_IF_ERR_EXIT_EXCEPT_IF(_expr, _rv_except,_formatting_string,_param2,_param3,_param4)   \
{                                                  \
  int _rv = _expr ;                                \
  if (SHR_FAILURE(_rv) && _rv != _rv_except)       \
  {                                                \
    LOG_ERROR_EX(BSL_LOG_MODULE,                   \
      " Error: %s ; " _formatting_string,          \
      shrextend_errmsg_get(_rv) ,_param2,_param3,_param4) ; \
    _func_rv = _rv ;                            \
    SHR_EXIT() ;                                \
  }                                             \
}
/**
 * \brief Check for specific function error value.
 *
 * This macro is a Boolean expression, which evaluates to TRUE, if the
 * macro-based function return value (declared by \ref SHR_FUNC_INIT_VARS)
 * matches the specified value.
 *
 * It can be used to take special actions for specific error codes.
 *
 * \code{.c}
 * void my_func(int unit,int index, int value)
 * {
 *   int local_idx ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 *
 *   if (SHR_FUNC_VAL_IS(_SHR_E_UNAVAIL))
 *   {
 *     SHR_SET_CURRENT_ERR(_SHR_E_NONE) ;
 *     SHR_EXIT() ;
 *   }
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _rv
 *     Return value to check against.
 * \par INDIRECT INPUT:
 *   _func_rv           - \n
 *     Curent value of function's return value.
 * \par DIRECT OUTPUT:
 *   Integer (result of conditional comparison): If non-zero then \n
 *   current return value is equal to DIRECT INPUT.
 */
#define SHR_FUNC_VAL_IS(_rv)     _func_rv == (_rv)

/**
 * \brief Get current return value.
 *
 * This macro gets the current return value (declared by \ref SHR_FUNC_INIT_VARS).
 *
 * It can be used to take special actions for specific error codes.
 *
 * \code{.c}
 * void my_func(int unit,int index, int value)
 * {
 *   int local_idx, ret_val ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 *
 *   ret_val = SHR_GET_CURRENT_ERR() ;
 *   if (ret_val == _SHR_E_UNAVAIL)
 *   {
 *     SHR_SET_CURRENT_ERR(_SHR_E_NONE) ;
 *     SHR_EXIT() ;
 *   }
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par INDIRECT INPUT:
 *   _func_rv           - \n
 *     Curent value of function's return value.
 * \par DIRECT OUTPUT:
 *   Curent value of function's return value.
 */
#define SHR_GET_CURRENT_ERR()     _func_rv

/**
 * \brief Set current return value.
 *
 * This macro sets the current return value (declared by \ref SHR_FUNC_INIT_VARS).
 *
 * It can be used to take special actions for specific error codes.
 *
 * \code{.c}
 * void my_func(int unit,int index, int value)
 * {
 *   int local_idx, ret_val ;
 *
 *   SHR_FUNC_INIT_VARS(unit) ;
 *   ...
 *
 *   SHR_SET_CURRENT_ERR(_SHR_E_UNAVAIL) ;
 *   ...
 *   if (ret_val == _SHR_E_UNAVAIL)
 *   {
 *     SHR_SET_CURRENT_ERR(_SHR_E_NONE) ;
 *     SHR_EXIT() ;
 *   }
 *   ...
 *
 * exit:
 *   SHR_FUNC_EXIT ;
 * }
 * \endcode
 *
 * \par DIRECT INPUT:
 *   \param [in] _rv
 *     Error value to set for function's return value.
 * \par INDIRECT INPUT:
 *   _func_rv           - \n
 *     Curent value of function's return value.
 */
#define SHR_SET_CURRENT_ERR(_rv)     _func_rv = _rv

/**
 * \brief Memory allocation with leak check.
 *
 * This macro is intended to prevent memory leaks from premature
 * pointer recycling. This type of leak happens if a pointer to an
 * allocated block of memory is used for another memory allocation
 * before the first block is freed.
 *
 * If the above error is detected, the macro will log and error and
 * exit the current function, otherwise a normal memory allocation
 * will be done using sal_alloc.
 *
 * The macro should be used in conjunction with \ref SHR_FREE.
 *
 * \par DIRECT INPUT:
 *   \param [in] _ptr
 *     Pointer variable to use for memory allocation.
 *   \param [in] _sz
 *     Amount of memory to allocate (in bytes).
 *   \param [in] _str
 *     Debug string to be passed to sal_alloc.
 *   \param [in] _formatting_string
 *     to display if print is not filtered. MUST be encapsulated in double quotes.
 *   \param [in] _param2
 *   \param [in] _param3
 *   \param [in] _param4
 *     Extra three parameters (_param2,_param3,_param4) to display via the formatting string. Param1 is _str above.
 * \par INDIRECT INPUT:
 *   BSL_LOG_MODULE       -\n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   LOG_ERROR            - \n
 *     Standard BSL (JR1) LOG macro \n
 *   _func_unit           - \n
 *     Value of 'unit' input parameter at entry
 */
#define SHR_ALLOC(_ptr, _sz, _str, _formatting_string,_param2,_param3,_param4)  \
{                                                \
  if ((_ptr) == NULL)                            \
  {                                              \
    (_ptr) = sal_alloc(_sz, _str) ;              \
  }                                              \
  else                                           \
  {                                              \
    LOG_ERROR_EX(BSL_LOG_MODULE,                 \
      " Ptr is NOT NULL at alloc: mem id is '%s' ; " _formatting_string, \
      _str ,_param2,_param3,_param4) ;           \
    SHR_EXIT() ;                                 \
  }                                              \
}

/**
 * \brief Memory allocation with leak check and setting to zero
 *        the allocated mamory.
 * 
 * See detailed explanation of SHR_ALLOC.
 * In addition, use memset in order to set the allocated memoty
 * to zero.
 * The macro should be used in conjunction with \ref SHR_FREE.
 *
 * \par DIRECT INPUT:
 *   \param [in] _ptr
 *     Pointer variable to use for memory allocation.
 *   \param [in] _sz
 *     Amount of memory to allocate (in bytes).
 *   \param [in] _str
 *     Debug string to be passed to sal_alloc.
 *   \param [in] _formatting_string
 *     to display if print is not filtered. MUST be encapsulated in double quotes.
 *   \param [in] _param2
 *   \param [in] _param3
 *   \param [in] _param4
 *     Extra three parameters (_param2,_param3,_param4) to display via the formatting string. Param1 is _str above.
 * \par INDIRECT INPUT:
 *   BSL_LOG_MODULE       -\n
 *     Module (layer/source) to report on. Defined at head of file. \n
 *   LOG_ERROR            - \n
 *     Standard BSL (JR1) LOG macro \n
 *   _func_unit           - \n
 *     Value of 'unit' input parameter at entry
 */
#define SHR_ALLOC_SET_ZERO(_ptr, _sz, _str, _formatting_string,_param2,_param3,_param4) \
{                                                                           \
  if ((_ptr) == NULL)                                                       \
  {                                                                         \
    SHR_ALLOC(_ptr, _sz, _str, _formatting_string,_param2,_param3,_param4); \
    sal_memset(_ptr, 0, _sz);                                               \
  }                                                                         \
}

/**
 * \brief Free memory and reset pointer.
 *
 * This macro will free allocated memory and reset the associated
 * pointer to NULL. The memory will be freed using sal_free.
 *
 * The macro may be used in conjunction with \ref SHR_ALLOC.
 *
 * \par DIRECT INPUT:
 *   \param [in] _ptr -
 *     Pointer to memory to be freed.
 * \par INDIRECT INPUT:
 *    Memory released back to heap
 */
#define SHR_FREE(_ptr)          \
{                                 \
  if ((_ptr) != NULL)             \
  {                               \
    sal_free((void *)(_ptr)) ;    \
    (_ptr) = NULL ;               \
  }                               \
}

/** } */
#endif /* !SHREXTEND_DEBUG_H_INCLUDED */
