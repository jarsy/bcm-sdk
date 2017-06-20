/* $Id: ui_nvram_hrp.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#if (defined(LINUX) || defined(UNIX)) || defined(__DUNE_GTO_BCM_CPU__)
/* { */

int
  app_source_checker(
    void           *current_line_ptr,
    unsigned long  source_id,
    char           *err_msg,
    unsigned long  partial
    )
{
  return 0;
}

int
  get_telnet_activity_timeout_cont(
    unsigned char *period_ptr
  )
{
  return 0;
}

int
  get_telnet_activity_timeout_std(
    unsigned char *period_ptr
  )
{
  return 0;
}

/*} LINUX */
#endif
