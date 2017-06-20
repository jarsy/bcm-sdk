/*
 * $Id: editline.h,v 1.4 2011/08/31 18:26:51 dkelley Exp $
 *
 * $Copyright: (c) 2011 Broadcom Corporation
 * All Rights Reserved.$
 */

#if !defined(_SAL_EDITLINE_H)
#define _SAL_EDITLINE_H

#ifdef INCLUDE_EDITLINE

#if	!defined(CONST)
#if	defined(__STDC__)
#define CONST	const
#else
#define CONST
#endif	/* defined(__STDC__) */
#endif	/* !defined(CONST) */

extern char	*readline(CONST char *prompt);
extern void	add_history(char *p);

/* editline asynchronous callback interface */

/** Called when an end of line is reached. */
typedef void (*rl_vcpfunc_t)(char *line, void *ctx);
/** Called when an end of file is reached. */
typedef void (*rf_vcpfunc_t)(void *ctx);
extern void rl_callback_read_char(CONST char *prompt);

/** Called when asynchronous edit line support is no longer needed. */
extern void rl_callback_handler_remove(void **eolCtx, void **eofCtx);
                                                                               
/** Must be called initially to enable asynchronous edit line support. */
extern void rl_callback_handler_install(CONST char *prompt, 
                                       rl_vcpfunc_t eol_handler, void *eolCtx,
                                       rf_vcpfunc_t eof_handler, void *eofCtx);


typedef struct rl_input_state_s {
    unsigned char *line;
    int point;
} rl_input_state_t;

extern void rl_input_state(rl_input_state_t *state);

extern char	*(*rl_complete)(char *, int *);
extern int	(*rl_list_possib)(char *, char ***);

#endif /* INCLUDE_EDITLINE */

#endif /* _SAL_EDITLINE_H */

