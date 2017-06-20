/* $Id: multicast.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/multicast.h>
#include <soc/dcmn/error.h>

/* flags for the flags field of dpp_mcds_common_t */
/*#define FLAG_MC_FIRST_FLAG 1 */

/* store a mcds per unit */
STATIC dpp_mcds_t *_dpp_mc_swds[SOC_SAND_MAX_DEVICE] = {0};

/* return the mcds of the given unit */
dpp_mcds_t *dpp_get_mcds(
    SOC_SAND_IN  int unit
  )
{
  return _dpp_mc_swds[unit];
}

/* Allocate the multicast data structure of a unit, using a given size */
uint32 dpp_alloc_mcds(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  unsigned   size_of_mcds, /* size of mcds to allocate in bytes */
    SOC_SAND_OUT dpp_mcds_t **mcds_out    /* output: allocated mcds */
)
{
    int need_cleaning = 0;
    dpp_mcds_t** mcds_p = _dpp_mc_swds + unit;
    SOCDNX_INIT_FUNC_DEFS;
    if (*mcds_p) { /* MCDS already allocated */
        SOCDNX_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_SOCDNX_MSG("multicast data structure already allocated")));
    }
    if (!(*mcds_p = sal_alloc(size_of_mcds, "multicast data structure"))) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("could not allocate multicast data structure")));
    }
    need_cleaning = 1;
    SOCDNX_IF_ERR_EXIT(soc_sand_os_memset(*mcds_p, 0, size_of_mcds));
    need_cleaning = 0;
    if (mcds_out) {
        *mcds_out = *mcds_p;
    }

exit:
    if (need_cleaning) {
        sal_free(*mcds_p);
        *mcds_p = NULL;
    }
    SOCDNX_FUNC_RETURN;
}

/* De-allocate the multicast data structure of a unit */
uint32 dpp_dealloc_mcds(
    SOC_SAND_IN  int        unit
)
{
    dpp_mcds_t** mcds_p = _dpp_mc_swds + unit;
    SOCDNX_INIT_FUNC_DEFS;
    if (*mcds_p == NULL) { /* MCDS not allocated */
        SOCDNX_EXIT_WITH_ERR(SOC_E_EXISTS, (_BSL_SOCDNX_MSG("multicast data structure not allocated")));
    }
    sal_free(*mcds_p);
    *mcds_p = NULL;

exit:
    SOCDNX_FUNC_RETURN;
}

/* Multicast assert mechanism */

STATIC uint32 nof_mc_asserts = 0;   /* number of MC asserts that happend till now */
STATIC int8 mc_asserts_enabled = 0; /* if MC asserts are to be enabled */
#if defined(_ARAD_SWDB_TEST_ASSERTS_REAL_ASSERT) && !defined(__KERNEL__)
extern char *getenv(const char*);
STATIC int8 mc_asserts_real = getenv("GDB") ? 1 : 0; /* if MC asserts are to call assert() */
#else
STATIC int8 mc_asserts_real = 0;
#endif

uint32 dpp_mcds_get_nof_asserts(void) {
    return nof_mc_asserts;
}
uint8 dpp_mcds_get_mc_asserts_enabled(void) {
    return mc_asserts_enabled;
}
void dpp_mcds_set_mc_asserts_enabled(uint8 enabled) {
    mc_asserts_enabled = enabled;
}
uint8 dpp_mcds_get_mc_asserts_real(void) {
    return mc_asserts_real;
}
void dpp_mcds_set_mc_asserts_real(uint8 real) {
    mc_asserts_real = real;
}

void dpp_perform_mc_assert(const char *file_name, unsigned line_number) { /* breakpoint can be set to this function to catch DPP_MC_ASSERT assertion failures */
    ++nof_mc_asserts;
    if (mc_asserts_enabled) {
        LOG_ERROR(BSL_LS_SOC_MULTICAST, (BSL_META("MCDS ASSERTED at %s:%u\n"), file_name, line_number));
#if !defined(__KERNEL__)
        if (mc_asserts_real) {
            assert(0);
        }
    #endif
    }
}

