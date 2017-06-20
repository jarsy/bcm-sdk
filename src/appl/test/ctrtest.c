/*
 * $Id: ctrtest.c,v 1.41 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Counter Tests
 */

#include <sal/types.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>


#include <soc/debug.h>
#include <soc/counter.h>


#include "testlist.h"


/*
 * Given a counter type, return a port that can be used for it.
 * Actually returns the first port of the appropriate type
 */
static soc_port_t
find_ctype_port(int unit, soc_ctr_type_t ctype)
{
    switch (ctype) {
    case SOC_CTR_TYPE_FE:
	if (SOC_PORT_NUM(unit, fe) < 1) {
	    return -1;
	}
	return SOC_PORT(unit, fe, 0);
    case SOC_CTR_TYPE_GE:
    case SOC_CTR_TYPE_GFE:
	if (SOC_PORT_NUM(unit, ge) < 1) {
	    return -1;
	}
#ifdef BCM_GREYHOUND_SUPPORT
       if (SOC_IS_GREYHOUND(unit) && IS_XL_PORT(unit, SOC_PORT(unit, ge, 0))) {
           return -1;
       }
#endif /* BCM_GREYHOUND_SUPPORT */
	return SOC_PORT(unit, ge, 0);
    case SOC_CTR_TYPE_XE:
	if (SOC_PORT_NUM(unit, xe) < 1) {
	    return -1;
	}
	return SOC_PORT(unit, xe, 0);
    case SOC_CTR_TYPE_HG:
	if (SOC_PORT_NUM(unit, hg) < 1) {
	    return -1;
	}
	return SOC_PORT(unit, hg, 0);
    default:
	return -1;
    }
}

/*
 * Routine to test the width of a counter in bits, as verified against
 * the width listed in the regsfile, for counters up to 32 bits.
 */

static int
test_width32(int unit, soc_reg_t ctr_reg, soc_port_t port)
{
    uint32		val = 0;
    char		*name;
    int			regsfile_width, actual_width;
#if defined(BCM_ROBO_SUPPORT) || defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    int			r;
#endif  /* BCM_ROBO_SUPPORT || BCM_ESW_SUPPORT || defined(BCM_SIRIUS_SUPPORT) */
#ifdef BCM_ROBO_SUPPORT
    uint32		addr;
#endif

    name = NULL;
    regsfile_width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_feature(unit, soc_feature_counter_parity) ||
        soc_reg_field_valid(unit, ctr_reg, PARITYf) ) {
        int i = 0;
        while((SOC_REG_INFO(unit, ctr_reg).fields + i) != NULL) { 
            if (SOC_REG_INFO(unit, ctr_reg).fields[i].field == COUNTf) {
                regsfile_width = SOC_REG_INFO(unit, ctr_reg).fields[i].len;
                break;
            }
            i++;
        }
    }
#endif

    if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
        if (SOC_REG_IS_VALID(unit, ctr_reg)) {
            name = SOC_ROBO_REG_NAME(unit, ctr_reg);
            addr = (DRV_SERVICES(unit)->reg_addr)(unit, ctr_reg, port, 0);
            val = ~0;
            r = (DRV_SERVICES(unit)->reg_write)(unit, addr, &val, (regsfile_width/8));
            if ((r < 0) ||
                (r = (DRV_SERVICES(unit)->reg_read)(unit, addr, &val, (regsfile_width/8))) < 0) {
                test_error(unit,
                    "Can't access counter %s.%s: %s\n",
                    name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
                return -1;
            }
        }
#endif
    } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        name = SOC_REG_NAME(unit, ctr_reg);

        if (soc_feature(unit, soc_feature_prime_ctr_writes)) {
            r = soc_counter_set32(unit, port, ctr_reg, 0, ~0);
        } else {
            r = soc_reg32_set(unit, ctr_reg, port, 0, ~0);
        }

        if ((r < 0) ||
            (r = soc_reg32_get(unit, ctr_reg, port, 0, &val)) < 0) {
            test_error(unit,
                       "Can't access counter %s.%s: %s\n",
                       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
            return -1;
        }
        if (soc_feature(unit, soc_feature_counter_parity) ||
            soc_reg_field_valid(unit, ctr_reg, PARITYf) ) {
            int bits;
            bits = soc_reg_field_length(unit, ctr_reg, COUNTf);
            val &= ((bits == 32 ? 0 : 1 << bits) - 1);
        }
#endif
    }

    actual_width = _shr_popcount(val);

    if (regsfile_width != actual_width) {
	test_error(unit,
		   "Counter %s.%s width mismatch: "
		   "regsfile: %d bits, actual: %d bits\n",
		   name, SOC_PORT_NAME(unit, port), regsfile_width,
                   actual_width);
	return -1;
    }
    return 0;
}

/*
 * Routine to test the width of a counter in bits, as verified against
 * the width listed in the regsfile, for counters over 32 bits.
 */

static int
test_width64(int unit, soc_reg_t ctr_reg, soc_port_t port)
{
    uint64		val;
    char		*name;
    int			regsfile_width, actual_width;
#if defined(BCM_ROBO_SUPPORT) || defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    int			r;
#endif
#ifdef BCM_ROBO_SUPPORT
    uint32		addr;
#endif

    name = NULL;
    regsfile_width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_feature(unit, soc_feature_counter_parity) ||
        soc_reg_field_valid(unit, ctr_reg, PARITYf)) {
        int i = 0;
        while((SOC_REG_INFO(unit, ctr_reg).fields + i) != NULL) { 
            if (SOC_REG_INFO(unit, ctr_reg).fields[i].field == COUNTf) {
                regsfile_width = SOC_REG_INFO(unit, ctr_reg).fields[i].len;
                break;
            }
            i++;
        }
    }
#endif

    COMPILER_64_SET(val, 0xffffffff, 0xffffffff);

    if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
        if (SOC_REG_IS_VALID(unit, ctr_reg)) {
            name = SOC_ROBO_REG_NAME(unit, ctr_reg);
            addr = (DRV_SERVICES(unit)->reg_addr)(unit, ctr_reg, port, 0);
            if ((r = (DRV_SERVICES(unit)->reg_write)(unit, addr, &val, (regsfile_width/8))) < 0 ||
                (r = (DRV_SERVICES(unit)->reg_read)(unit, addr, &val, (regsfile_width/8))) < 0) {
                test_error(unit,
                   "Can't access counter %s.%s: %s\n",
                   name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
                return -1;
            }
        }
#endif
    } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        name = SOC_REG_NAME(unit, ctr_reg);
        if ((r = soc_reg64_set(unit, ctr_reg, port, 0, val)) < 0 ||
	    (r = soc_reg64_get(unit, ctr_reg, port, 0, &val)) < 0) {
	    test_error(unit,
		   "Can't access counter %s.%s: %s\n",
		   name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
	    return -1;
        }
        val = soc_reg64_field_get(unit, ctr_reg, val,
                                  SOC_REG_INFO(unit, ctr_reg).fields[0].field);
#endif
    }
    actual_width = (_shr_popcount(COMPILER_64_LO(val)) +
		    _shr_popcount(COMPILER_64_HI(val)));

    if (regsfile_width != actual_width) {
	test_error(unit,
		   "Counter %s.%s width mismatch: "
		   "regsfile: %d bits, actual: %d bits\n",
		   name, SOC_PORT_NAME(unit, port),
                   regsfile_width, actual_width);
	return -1;
    }
    return 0;
}

int
ctr_test_width(int unit, args_t *a, void *pa)
/*
 * Function: 	ctr_test_width
 * Purpose:	Test the widths of all counters as compared to regsfile
 * Parameters:	u - unit #.
 *		a - pointer to arguments.
 *		pa - ignored cookie.
 * Returns:	0
 */
{
    int			i;
    soc_ctr_type_t      ctype;
    soc_reg_t           reg;
    soc_port_t		port;

    COMPILER_REFERENCE(a);
    COMPILER_REFERENCE(pa);

#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        if (soc_reg_field32_modify(unit, ISW2_HW_CONTROLr, REG_PORT_ANY,
                                   XGS_COUNTER_COMPAT_MODEf, 1) < 0) {
            return -1;
        }
    }
#endif
    for (ctype = SOC_CTR_TYPE_FE; ctype < SOC_CTR_NUM_TYPES; ctype++) {
        if (SOC_HAS_CTR_TYPE(unit, ctype)) {
	    port = find_ctype_port(unit, ctype);
	    if (port < 0) {
		continue;
	    }
            for (i = 0; i < SOC_CTR_MAP_SIZE(unit, ctype); i++) {
                reg = SOC_CTR_TO_REG(unit, ctype, i);
                if (!SOC_COUNTER_INVALID(unit, reg) && 
                      !(SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_RO)) {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
                    soc_regaddrinfo_t   ainfo;
                    uint64		mask;
                    uint32              addr;
                    int                 block;
                    uint8               at;

                    if (!SOC_REG_PORT_VALID(unit, reg, port)) {
                        continue;
                    }
                    addr = soc_reg_addr_get(unit, reg, port, 0,
                                            SOC_REG_ADDR_OPTION_NONE,
                                            &block, &at);
                    soc_regaddrinfo_extended_get(unit, &ainfo, block, at, addr);
                    if (SOC_COUNTER_INVALID(unit, ainfo.reg)) {
                        continue;
                    }
                    if (reg_mask_subset(unit, &ainfo, &mask)) {
                        /* Unimplemented in HW, skip */
                        continue;
                    }
#endif

		    if (SOC_REG_IS_64(unit, reg)) {
			test_width64(unit, reg, port);
		    } else {
			test_width32(unit, reg, port);
		    }
                }
            }
        }
    }
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        if (soc_reg_field32_modify(unit, ISW2_HW_CONTROLr, REG_PORT_ANY,
                                   XGS_COUNTER_COMPAT_MODEf, 0) < 0) {
            return -1;
        }
    }
#endif

    return 0;
}

/*
 * Routine to write a counter with several patterns and compare.
 * Works on counters <= 32 bits.
 */

static int
test_rw32(int unit, soc_reg_t ctr_reg, soc_port_t port)
{
    char              *name;
    volatile uint32   pattern;  /* SiByte1250 workaround */
    uint32            mask, val;
#if defined(BCM_ROBO_SUPPORT) || defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    uint32            pattern_mask;
    int               r;
#endif
#ifdef BCM_ROBO_SUPPORT
    uint32	      addr = 0;
#endif
    int               bits, patno;

    name = NULL;
    bits = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_feature(unit, soc_feature_counter_parity) ||
        soc_reg_field_valid(unit, ctr_reg, PARITYf)) {
        int i = 0;
        while((SOC_REG_INFO(unit, ctr_reg).fields + i) != NULL) { 
            if (SOC_REG_INFO(unit, ctr_reg).fields[i].field == COUNTf) {
                bits = SOC_REG_INFO(unit, ctr_reg).fields[i].len;
                break;
            }
            i++;
        }
    }
#endif
    mask = (bits == 32 ? 0 : 1 << bits) - 1;

    /*
     * The following loop sets pattern to:
     *
     *	0x00000000
     *	0x55555555
     *	0xaaaaaaaa
     *	0xffffffff
     */

    pattern = 0x00000000;
    val = 0;

    for (patno = 0; patno < 4; patno++) {
        if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
            if (SOC_REG_IS_VALID(unit, ctr_reg)) {
                name = SOC_ROBO_REG_NAME(unit, ctr_reg);
                pattern_mask = pattern & mask;
                addr = (DRV_SERVICES(unit)->reg_addr)(unit, ctr_reg, port, 0);
                r = (DRV_SERVICES(unit)->reg_write)(unit, addr, &pattern_mask, (bits/8));
                if (r < 0) {
                    val = pattern_mask;
                    test_error(unit,
                       "Can't write counter %s.%s: %s\n",
                       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
                } else if ((r = (DRV_SERVICES(unit)->reg_read)(unit, addr, &val, (bits/8))) < 0) {
                    test_error(unit,
                       "Can't access counter %s.%s: %s\n",
                       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
                }
            }
#endif
        } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
            name = SOC_REG_NAME(unit, ctr_reg);
             pattern_mask = pattern & mask;
            if (soc_feature(unit, soc_feature_prime_ctr_writes)) {
                r = soc_counter_set32(unit, port, ctr_reg, 0, pattern_mask);
            } else {
                r = soc_reg32_set(unit, ctr_reg, port, 0, pattern_mask);
            }
            if (r < 0) {
                val = pattern_mask;
	        test_error(unit,
		       "Can't write counter %s.%s: %s\n",
		       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
            } else if ((r = soc_reg32_get(unit, ctr_reg, port, 0, &val)) < 0) {
	        test_error(unit,
		       "Can't access counter %s.%s: %s\n",
		       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
            }
#endif
        }
        if (((val ^ pattern) & mask) != 0) {
	    uint32 reread = 0;

           if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
               (void) (DRV_SERVICES(unit)->reg_read)(unit, addr, &reread, (bits/8));
#endif
           } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
	    (void) soc_reg32_get(unit, ctr_reg, port, 0, &reread);
#endif
           }

	    test_error(unit,
		       "Counter %s.%s miscompare: \n"
                       "    wrote     0x%08x\n"
                       "    read      0x%08x\n"
                       "    re-read   0x%08x\n",
		       name, SOC_PORT_NAME(unit, port),
		       pattern & mask,
		       val & mask,
		       reread & mask);
	}

	pattern += 0x55555555;
    }
    return 0;
}

/*
 * Routine to write a counter with several patterns and compare.
 * Works on counters over 32 bits.
 */

static int
test_rw64(int unit, soc_reg_t ctr_reg, soc_port_t port)
{
    char		*name;
    uint64		mask, pattern, tmp;
    uint64		val, incr;
    int			bits, patno;
#if defined(BCM_ROBO_SUPPORT) || defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    int                 r;
#endif /* BCM_ROBO_SUPPORT || BCM_ESW_SUPPORT || defined(BCM_SIRIUS_SUPPORT) */
#ifdef BCM_ROBO_SUPPORT
    uint32              addr = 0;
#endif

    name = NULL;
    bits = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (soc_feature(unit, soc_feature_counter_parity) ||
        soc_reg_field_valid(unit, ctr_reg, PARITYf) ) {
        int i = 0;
        while((SOC_REG_INFO(unit, ctr_reg).fields + i) != NULL) { 
            if (SOC_REG_INFO(unit, ctr_reg).fields[i].field == COUNTf) {
                bits = SOC_REG_INFO(unit, ctr_reg).fields[i].len;
                break;
            }
            i++;
        }
    }
#endif

    COMPILER_64_MASK_CREATE(mask, bits, 0);
    COMPILER_64_SET(incr, 0x55555555, 0x55555555);
    COMPILER_64_SET(val, 0x0, 0x0);

    /*
     * The following loop sets pattern to:
     *
     *	0x00000000
     *	0x55555555
     *	0xaaaaaaaa
     *	0xffffffff
     */

    COMPILER_64_ZERO(pattern);

    for (patno = 0; patno < 4; patno++) {
	tmp = pattern;
        if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
            if (SOC_REG_IS_VALID(unit, ctr_reg)) {
                name = SOC_ROBO_REG_NAME(unit, ctr_reg);
                addr = (DRV_SERVICES(unit)->reg_addr)(unit, ctr_reg, port, 0);
                if ((r = (DRV_SERVICES(unit)->reg_write)(unit, addr, &tmp, (bits/8))) < 0) {
                   val = tmp;
                   test_error(unit,
                       "Can't write counter %s.%s: %s\n",
                       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
                } else if ((r = (DRV_SERVICES(unit)->reg_read)(unit, addr, &val, (bits/8))) < 0) {
                   test_error(unit,
                       "Can't access counter %s.%s: %s\n",
                       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
                }
            }
#endif
        } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
            name = SOC_REG_NAME(unit, ctr_reg);
	if ((r = soc_reg64_set(unit, ctr_reg, port, 0, tmp)) < 0) {
            val = tmp;
	    test_error(unit,
		       "Can't write counter %s.%s: %s\n",
		       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
	} else if ((r = soc_reg64_get(unit, ctr_reg, port, 0, &val)) < 0) {
	    test_error(unit,
		       "Can't access counter %s.%s: %s\n",
		       name, SOC_PORT_NAME(unit, port), soc_errmsg(r));
             }
#endif
        }
	    tmp = val;
	    COMPILER_64_XOR(tmp, pattern);
	    COMPILER_64_AND(tmp, mask);

	    if (!COMPILER_64_IS_ZERO(tmp)) {
		uint64 reread;

                COMPILER_64_ZERO(reread);

           if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
               (void) (DRV_SERVICES(unit)->reg_read)(unit, addr, &reread, (bits/8));
#endif
           } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
		(void) soc_reg64_get(unit, ctr_reg, port, 0, &reread);
#endif
           }

		COMPILER_64_AND(pattern, mask);
		COMPILER_64_AND(val, mask);

		test_error(unit,
			   "Counter %s.%s miscompare:\n"
			   "    wrote     0x%08x%08x\n"
                           "    read      0x%08x%08x\n"
			   "    re-read   0x%08x%08x\n",
			   name, SOC_PORT_NAME(unit, port),
			   COMPILER_64_HI(pattern),
			   COMPILER_64_LO(pattern),
			   COMPILER_64_HI(val),
			   COMPILER_64_LO(val),
			   COMPILER_64_HI(reread),
			   COMPILER_64_LO(reread));
	    }

	COMPILER_64_ADD_64(pattern, incr);
    }
    return 0;
}

int
ctr_test_rw(int unit, args_t *a, void *pa)
/*
 * Function: 	ctr_test_rw
 * Purpose:	Run test values through all counter registers
 * Parameters:	u - unit #.
 *		a - pointer to arguments.
 *		pa - ignored cookie.
 * Returns:	0
 */
{
    int			i;
    soc_reg_t           reg;
    soc_ctr_type_t      ctype;
    soc_port_t		port;

    COMPILER_REFERENCE(a);
    COMPILER_REFERENCE(pa);

#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        if (soc_reg_field32_modify(unit, ISW2_HW_CONTROLr, REG_PORT_ANY,
                                   XGS_COUNTER_COMPAT_MODEf, 1) < 0) {
            return -1;
        }
    }
#endif

    for (ctype = SOC_CTR_TYPE_FE; ctype < SOC_CTR_NUM_TYPES; ctype++) {
        if (SOC_HAS_CTR_TYPE(unit, ctype)) {
	    port = find_ctype_port(unit, ctype);
	    if (port < 0) {
		continue;
	    }
            for (i = 0; i < SOC_CTR_MAP_SIZE(unit, ctype); i++) {
                reg = SOC_CTR_TO_REG(unit, ctype, i);
                if (!SOC_COUNTER_INVALID(unit, reg) && 
                      !(SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_RO)) {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
                    soc_regaddrinfo_t   ainfo;
                    uint64		mask;
                    uint32              addr;
                    int                 block;
                    uint8               at;
                    
                    if (!SOC_REG_PORT_VALID(unit, reg, port)) {
                        continue;
                    }
                    addr = soc_reg_addr_get(unit, reg, port, 0,
                                            SOC_REG_ADDR_OPTION_NONE,
                                            &block, &at);
                    soc_regaddrinfo_extended_get(unit, &ainfo, block, at, addr);
                    if (SOC_COUNTER_INVALID(unit, ainfo.reg)) {
                        continue;
                    }
                    if (reg_mask_subset(unit, &ainfo, &mask)) {
                        /* Unimplemented in HW, skip */
                        continue;
                    }
#endif
		    if (SOC_REG_IS_64(unit, reg)) {
			test_rw64(unit, reg, port);
		    } else {
			test_rw32(unit, reg, port);
		    }
                }
            }
        }
    }
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        if (soc_reg_field32_modify(unit, ISW2_HW_CONTROLr, REG_PORT_ANY,
                                   XGS_COUNTER_COMPAT_MODEf, 0) < 0) {
            return -1;
        }
    }
#endif

    return 0;
}
