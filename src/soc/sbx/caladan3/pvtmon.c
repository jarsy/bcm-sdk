/*
 * $Id: pvtmon.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose: PVTmon funcitonality
 *   - Currently Implements temperature monitor
 */
#ifdef BCM_CALADAN3_SUPPORT

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/cm.h>
#include <soc/mem.h>


#define TO_F(c)  ((c) * 9/5 + 32)

#define TO_C(c)  (c)

#define SOC_SBX_CALADAN3_MAX_MONITORS 5

#define UNITS_C "C"
#define UNITS_F "F"

char *mon_loc[] = {
    "bot-left ",
    "mid-left ",
    "mid-right",
    "bot-right",
    "off mid  "
};

char *process[] = {
    "ss ",
    "tt ",
    "ff "
};

static int
soc_sbx_caladan3_temperature_monitor_read(int unit, int nsamples, int format, int celsius, int *avg_temperature);

int 
soc_sbx_caladan3_temperature_monitor_init(int unit) 
{
    uint32 rval;
    
    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_1r(unit, &rval));
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_1r, &rval, RESET_Nf, 0);
    SOC_IF_ERROR_RETURN(WRITE_CX_PVT_MON_CONTROL_1r(unit, rval));

    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_0r(unit, &rval));
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_0r, &rval, PROG_RESISTORf, 3);
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_0r, &rval, BJ_ADJf, 3);
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_0r, &rval, MODEf, 5);    
    SOC_IF_ERROR_RETURN(WRITE_CX_PVT_MON_CONTROL_0r(unit, rval));

    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_1r(unit, &rval));
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_1r, &rval, PVT_SELf, 0);
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_1r, &rval, RESET_Nf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CX_PVT_MON_CONTROL_1r(unit, rval));

    sal_usleep(1000);

    return SOC_E_NONE;
}    

int
soc_sbx_caladan3_temperature_monitor(int unit, int nsamples, int format, int celsius)
{
    return soc_sbx_caladan3_temperature_monitor_read(unit, nsamples, format, celsius, NULL);
}

static int
soc_sbx_caladan3_temperature_monitor_read(int unit, int nsamples, int format, int celsius, int *avg_temperature)
{
    uint32 rval;
    int index;
    int ns;
    int fval, avg_cur, cur, peak, max_peak;
    
    /* Check to see if the thermal monitor is initialized */
    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_1r(unit, &rval));
    if ((soc_reg_field_get(unit, CX_PVT_MON_CONTROL_1r, rval, RESET_Nf) == 0) ||
        (soc_reg_field_get(unit, CX_PVT_MON_CONTROL_1r, rval, PVT_SELf) != 0)) {
        SOC_IF_ERROR_RETURN(soc_sbx_caladan3_temperature_monitor_init(unit));
    }

    avg_cur = 0;
    max_peak = 0x80000000;

    ns = 0;
    do {
        for (index = 0; index < SOC_SBX_CALADAN3_MAX_MONITORS; index++) {

            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CX_PVT_MON_THERMAL_DATAr, REG_PORT_ANY, index, &rval));

            fval = soc_reg_field_get(unit, CX_PVT_MON_THERMAL_DATAr, rval, PVT_MON_THERMAL_DATAf);
            cur = (4180000 - (5556 * fval)) / 10000;

            fval = soc_reg_field_get(unit, CX_PVT_MON_THERMAL_DATAr, rval, PVT_MON_THERMAL_DATA_PEAKf);
            if (fval == 0) {
                fval = 752; /* to make peak 0 */
            }
            peak = (4180000 - (5556 * fval)) / 10000;
	    if (avg_temperature == NULL) {
		if (format == 2) {
		    /* show data in csv format */
		    if (ns==0 && index==0) {
			LOG_CLI((BSL_META_U(unit,
                                            "Sample, Temp monitor id, current(degree %s), peak(degree %s)\n"), 
                                 celsius ? UNITS_C : UNITS_F,
                                 celsius ? UNITS_C : UNITS_F));
		    }
		    if (celsius) 
			LOG_CLI((BSL_META_U(unit,
                                            "%d, %d, %3d, %3d\n"), ns, index, cur, peak));
		    else
			LOG_CLI((BSL_META_U(unit,
                                            "%d, %d, %3d, %3d\n"), ns, index, TO_F(cur), TO_F(peak)));
		} else {
		    LOG_CLI((BSL_META_U(unit,
                                        "Monitor %d "), index));
		    if (format==1) {
			/* show location in verbose mode */
			LOG_CLI((BSL_META_U(unit,
                                            "(%s)"), mon_loc[index]));
		    }
		    if (celsius)
			LOG_CLI((BSL_META_U(unit,
                                            ": current=%3d C, peak=%3d C\n"), cur, peak));
		    else
			LOG_CLI((BSL_META_U(unit,
                                            ": current=%3d F, peak=%3d F\n"), TO_F(cur), TO_F(peak)));
		}
	    }
            avg_cur += cur;
            if (max_peak < peak) {
                max_peak = peak;
            }
        }
    } while (++ns < nsamples);

    if (avg_temperature != NULL) {
	*avg_temperature = avg_cur / (SOC_SBX_CALADAN3_MAX_MONITORS * nsamples);
    }

    if (!format) {
        avg_cur /= (SOC_SBX_CALADAN3_MAX_MONITORS * nsamples);
        LOG_CLI((BSL_META_U(unit,
                            "Average current temperature is %d %s\n"),  celsius ? avg_cur : TO_F(avg_cur), celsius ? UNITS_C : UNITS_F ));
        LOG_CLI((BSL_META_U(unit,
                            "Maximum peak temperature is %d %s\n"), celsius ? max_peak : TO_F(max_peak), celsius ? UNITS_C : UNITS_F ));
    }

    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_process_monitor_init(int unit, int nmos) 
{
    uint32 rval;
    
    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_1r(unit, &rval));
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_1r, &rval, RESET_Nf, 0);
    SOC_IF_ERROR_RETURN(WRITE_CX_PVT_MON_CONTROL_1r(unit, rval));

    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_0r(unit, &rval));
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_0r, &rval, PROG_RESISTORf, 3);
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_0r, &rval, BJ_ADJf, 3);
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_0r, &rval, MODEf, nmos?5:7);    
    SOC_IF_ERROR_RETURN(WRITE_CX_PVT_MON_CONTROL_0r(unit, rval));

    SOC_IF_ERROR_RETURN(READ_CX_PVT_MON_CONTROL_1r(unit, &rval));
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_1r, &rval, PVT_SELf, 1);
    soc_reg_field_set(unit, CX_PVT_MON_CONTROL_1r, &rval, RESET_Nf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CX_PVT_MON_CONTROL_1r(unit, rval));

    sal_usleep(1000);

    return SOC_E_NONE;
}    

int
soc_sbx_caladan3_process_monitor(int unit, int nsamples)
{
    uint32 rval;
    int rv, index;
    int ns;
    int fval, code, peak, avg_nmos_code, max_nmos_peak, avg_pmos_code, max_pmos_peak, temperature;
    int nmos_tt, nmos_ff, pmos_tt, pmos_ff;

    /* find out temperature and calculate thresholds */
    rv = soc_sbx_caladan3_temperature_monitor_read(unit, 1000, 0, TRUE, &temperature);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "Unit %d failed to read die temperature %d:%s\n"), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    if (temperature < 25) {
	temperature = 25;
    } else if (temperature > 75) {
	temperature = 75;
    }

    temperature -= 25;
    nmos_tt = (((9809*temperature*temperature + 258900*temperature + 12330000)/1000)*3)/2;
    if (nmos_tt >= 1023000) {
	nmos_tt = 1023000;
    }
    nmos_tt = (nmos_tt * 1980) / 1023000;

    nmos_ff = (((32760*temperature*temperature + 1445000*temperature + 55860000)/1000)*3)/2;
    if (nmos_ff >= 1023000) {
	nmos_ff = 1023000;
    }
    nmos_ff = (nmos_ff * 1980) / 1023000;

    pmos_tt = (((4578*temperature*temperature + 97190*temperature + 4885000)/1000)*38)/20;
    if (pmos_tt >= 1023000) {
	pmos_tt = 1023000;
    }
    pmos_tt = (pmos_tt * 1980) / 1023000;

    pmos_ff = (((17720*temperature*temperature + 762900*temperature + 28650000)/1000)*665)/400;
    if (pmos_ff >= 1023000) {
	pmos_ff = 1023000;
    }
    pmos_ff = (pmos_ff * 1980) / 1023000;

    /* setup NMOS monitors */
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_process_monitor_init(unit, TRUE));

    /* take NMOS samples */
    nsamples += 10;
    avg_nmos_code = 0;
    max_nmos_peak = 0;

    ns = 0;
    do {
        for (index = 0; index < SOC_SBX_CALADAN3_MAX_MONITORS; index++) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CX_PVT_MON_THERMAL_DATAr, REG_PORT_ANY, index, &rval));

            fval = soc_reg_field_get(unit, CX_PVT_MON_THERMAL_DATAr, rval, PVT_MON_THERMAL_DATAf);
            code = fval;

            fval = soc_reg_field_get(unit, CX_PVT_MON_THERMAL_DATAr, rval, PVT_MON_THERMAL_DATA_PEAKf);
            peak = fval;

	    if (ns >= 10) {
		/* skip first 10 sample */
		avg_nmos_code += code;
		if (max_nmos_peak < peak) {
		    max_nmos_peak = peak;
		}
	    }
        }
    } while (++ns < nsamples);

    /* setup PMOS monitor */
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_process_monitor_init(unit, FALSE));

    /* take PMOS samples */
    avg_pmos_code = 0;
    max_pmos_peak = 0;

    ns = 0;
    do {
        for (index = 0; index < SOC_SBX_CALADAN3_MAX_MONITORS; index++) {
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, CX_PVT_MON_THERMAL_DATAr, REG_PORT_ANY, index, &rval));

            fval = soc_reg_field_get(unit, CX_PVT_MON_THERMAL_DATAr, rval, PVT_MON_THERMAL_DATAf);
            code = fval;

            fval = soc_reg_field_get(unit, CX_PVT_MON_THERMAL_DATAr, rval, PVT_MON_THERMAL_DATA_PEAKf);
            peak = fval;

	    if (ns >= 10) {
		/* skip first 10 sample */
		avg_pmos_code += code;
		if (max_pmos_peak < peak) {
		    max_pmos_peak = peak;
		}
	    }
        }
    } while (++ns < nsamples);

    /* report NMOS/PMOS process */
    avg_nmos_code /= (SOC_SBX_CALADAN3_MAX_MONITORS * (nsamples-10));
    LOG_CLI((BSL_META_U(unit,
                        "Average NMOS output code is %d\n"), avg_nmos_code));

    avg_pmos_code /= (SOC_SBX_CALADAN3_MAX_MONITORS * (nsamples-10));
    LOG_CLI((BSL_META_U(unit,
                        "Average PMOS output code is %d\n"), avg_pmos_code));

    LOG_CLI((BSL_META_U(unit,
                        "At temperature %d nmos_tt %d nmos_ff %d pmos_tt %d pmos_ff %d\n"),
             temperature+25, nmos_tt, nmos_ff, pmos_tt, pmos_ff));
    
    if (avg_nmos_code <= nmos_tt) {
	avg_nmos_code = 0;
    } else if (avg_nmos_code <= nmos_ff) {
	avg_nmos_code = 1;
    } else {
	avg_nmos_code = 2;
    }

    if (avg_pmos_code <= pmos_tt) {
	avg_pmos_code = 0;
    } else if (avg_pmos_code <= pmos_ff) {
	avg_pmos_code = 1;
    } else {
	avg_pmos_code = 2;
    }

    LOG_CLI((BSL_META_U(unit,
                        "NMos process %s, PMos process %s\n"),
             process[avg_nmos_code], process[avg_pmos_code]));
    return SOC_E_NONE;
}

#endif
