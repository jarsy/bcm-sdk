/* $Id: ui_memory_hrp.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#if (defined(LINUX) || defined(UNIX)) || defined(__DUNE_GTO_BCM_CPU__)
/* { */
int
  type_mem_checker(
    void           *current_line_ptr,
    unsigned long  data_value,
    char           *err_msg,
    unsigned long  partial
  )
{
  return 0;
}

/*} LINUX */
#endif
