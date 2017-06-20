/*  $Revision: 1.4 $
**
**  VxWorks system-dependant routines for editline library.
**
**  This version of rl_ttyset does not switch in and out of
**  raw TTY mode because this flushes the VxWorks input buffer
**  and thus loses type-ahead, which is severely annoying.
**
**  Currently, this file does not contain any VxWorks specific
**  code and is used by default if SYS_UNIX is undefined.
*/

#if !defined(SYS_UNIX) && defined(INCLUDE_EDITLINE)

#include "editline.h"

void
rl_ttyset(Reset)
    int		Reset;
{
    rl_erase = 'H' & 037;
    rl_kill = 'U' & 037;
    rl_eof = 'D' & 037;
    rl_intr = 'C' & 037;
    rl_quit = 'Q' & 037;
#if	defined(DO_SIGTSTP)
    rl_susp = 'Z' & 037;
#endif	/* defined(DO_SIGTSTP) */
}

void
rl_add_slash(path, p)
    char	*path;
    char	*p;
{
    return;
}

#endif /* !defined(SYS_UNIX) && defined(INCLUDE_EDITLINE) */

int _bcm_diag_editline_sysvxworks_not_empty;
