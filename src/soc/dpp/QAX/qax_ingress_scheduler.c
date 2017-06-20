/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>
#include <soc/mcm/memregs.h>

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS


/*************
 * INCLUDES  *
 *************/
#include <soc/mem.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/TMC/tmc_api_ingress_scheduler.h>
#include <soc/dpp/ARAD/arad_ingress_scheduler.h>
#include <soc/dpp/QAX/qax_ingress_scheduler.h>


/*************
 * TYPE DEFS *
 *************/
typedef struct qax_ingress_scheduler_mem_field_s {
    soc_mem_t   delay_cal_mem;
    soc_field_t delay_field;
    soc_field_t cal_field;
    int         delay_cal_index;
} qax_ingress_scheduler_mem_field_t;



/*********************************************************************
*     This procedure calculates max_rate value of the the shaper
*     from the suitable memory fields (ShaperDelay, ShaperCal).
*     The calculation is performed using an additional function.
*     shaper struct: using only the field shaper->max_rate.
*********************************************************************/
STATIC soc_error_t
  qax_ingress_scheduler_shaper_values_get(
    SOC_SAND_IN   int                       unit,
    SOC_SAND_IN   int                       is_delay_2_clocks,
    SOC_SAND_IN   qax_ingress_scheduler_mem_field_t *shaper_mem,
    SOC_SAND_OUT  ARAD_ING_SCH_SHAPER       *shaper
  )
{
  uint32 res, shaper_delay_2_clocks, shaper_cal;
  uint32 shaper_table_entry[4] = {0};
  SOCDNX_INIT_FUNC_DEFS;

  SOCDNX_NULL_CHECK(shaper_mem);
  SOCDNX_NULL_CHECK(shaper);

  /*read ShaperDelay and ShaperCal fields. Those fields are in the same memory and same index*/
  SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, shaper_mem->delay_cal_mem, MEM_BLOCK_ANY, shaper_mem->delay_cal_index, shaper_table_entry));
  shaper_delay_2_clocks = soc_mem_field32_get(unit, shaper_mem->delay_cal_mem, shaper_table_entry, shaper_mem->delay_field);
  shaper_cal = soc_mem_field32_get(unit, shaper_mem->delay_cal_mem, shaper_table_entry, shaper_mem->cal_field);

  /* if delay is in 2 clocks resolution */
  if (is_delay_2_clocks)
  {
      shaper_delay_2_clocks *=2;
  }

  res = arad_ingress_scheduler_delay_cal_to_max_rate_form(
          unit,
          shaper_delay_2_clocks,
          shaper_cal,
          &(shaper->max_rate)
        );
  SOCDNX_SAND_IF_ERR_EXIT(res);

exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*     This procedure writes the values of a given shaper
*     (ShaperDelay, ShaperCal) to the suitable memory fields.
*     The ShaperDelay & ShaperCal fields are retrieved using an additional
*     function that converts max_rate to the suitable values.
*     shaper struct: using only the field shaper->max_rate.
*********************************************************************/
STATIC soc_error_t
  qax_ingress_scheduler_shaper_values_set(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  int                  is_delay_2_clocks,
    SOC_SAND_IN  qax_ingress_scheduler_mem_field_t *shaper_mem,
    SOC_SAND_IN  ARAD_ING_SCH_SHAPER     *shaper
  )
{
    uint32 res, shaper_delay_2_clocks, shaper_cal, dummy, shaper_table_entry[4] = {0};
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_NULL_CHECK(shaper_mem);
    SOCDNX_NULL_CHECK(shaper);

    /* write ShaperDelay and ShaperCal fields */

    /* convert max_rate to delay and cal, delay is returned in 2 clocks resolution  */
    res = arad_ingress_scheduler_rate_to_delay_cal_form(
            unit,
            shaper->max_rate,
            is_delay_2_clocks,
            &shaper_delay_2_clocks,
            &shaper_cal,
            &dummy
    );
    SOCDNX_SAND_IF_ERR_EXIT(res);

    /* write delay and cal values to the memory */
    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, shaper_mem->delay_cal_mem, MEM_BLOCK_ANY, shaper_mem->delay_cal_index, shaper_table_entry));
    soc_mem_field32_set(unit, shaper_mem->delay_cal_mem, shaper_table_entry, shaper_mem->delay_field, shaper_delay_2_clocks);
    soc_mem_field32_set(unit, shaper_mem->delay_cal_mem, shaper_table_entry, shaper_mem->cal_field, shaper_cal);
    SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, shaper_mem->delay_cal_mem, MEM_BLOCK_ANY, shaper_mem->delay_cal_index, shaper_table_entry));

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configures the slow start phase0/phase1
*     rates (ShaperDelay, ShaperCal) of HP/LP MC
*     of the clos/mesh scheduling scheme.
*     1. shaper_info struct: using only the fields:
*     shaper_info->slow_start_rate_phase_0/1
*     2. full_rate_shaper struct: using only the fields:
*     full_rate_shaper->max_rate                                                                                                                                                                                            .
*********************************************************************/
STATIC soc_error_t
qax_ingress_scheduler_slow_start_phase_set (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ING_SCH_SHAPER *shaper_info,
    SOC_SAND_IN  ARAD_ING_SCH_SHAPER *full_rate_shaper,
    SOC_SAND_IN int                  is_hp,
    SOC_SAND_IN int                  is_phase_1
    )
{
    qax_ingress_scheduler_mem_field_t shaper_mem;
    ARAD_ING_SCH_SHAPER shaper_slow_phase_config;
    soc_error_t rv = SOC_E_NONE;
    uint32 factor;
    SOCDNX_INIT_FUNC_DEFS;

    shaper_mem.delay_cal_mem = PTS_SHAPER_FMC_CFGm;
    shaper_mem.delay_field = is_phase_1? SLOW_START_DELAY_1f : SLOW_START_DELAY_0f;
    shaper_mem.cal_field = is_phase_1? SLOW_START_CAL_1f : SLOW_START_CAL_0f;
    shaper_mem.delay_cal_index = is_hp? QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_SLOW_START_CONFIG_INDEX : QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_SLOW_START_CONFIG_INDEX;

    /* Set HP/LP Multicast slow rate phase0/phase1 */
    SOC_TMC_ING_SCH_SHAPER_clear(&shaper_slow_phase_config);
    factor = is_phase_1? shaper_info->slow_start_rate_phase_1 : shaper_info->slow_start_rate_phase_0;
    shaper_slow_phase_config.max_rate = full_rate_shaper->max_rate * factor / 100;
    /* Set ShaperDelay, ShaperCal of phase0/1 based on the calculated shaper_temp.max_rate */
    rv = qax_ingress_scheduler_shaper_values_set(
            unit,
            TRUE,
            &shaper_mem,
            &shaper_slow_phase_config
    );
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configures the slow start configuration
*     (rates, enable/disable) of the clos/mesh scheduling scheme.
*     shaper_info struct: using only the fields:
*     1. shaper_info->slow_start_enable
*     2. shaper_info->slow_start_rate_phase_0/1                                                                                                                                                                                              .
*********************************************************************/
STATIC soc_error_t
qax_ingress_scheduler_slow_start_configuration_set (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ING_SCH_SHAPER *shaper_info,
    SOC_SAND_IN int                  is_hp
    )
{
    qax_ingress_scheduler_mem_field_t shaper_mem;
    ARAD_ING_SCH_SHAPER full_rate_shaper;
    uint32 shaper_table_entry[4] = {0};
    soc_error_t rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    shaper_mem.delay_cal_mem = PTS_PER_SHAPER_CFGm;
    shaper_mem.delay_field = SHAPER_DELAYf;
    shaper_mem.cal_field = SHAPER_CALf;
    shaper_mem.delay_cal_index = is_hp? QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_INDEX : QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_INDEX;

    /* get full_rate_shaper.max_rate of the NORMAL (100%) rate, calculated from delay and cal fields (for HP/LP, depending on delay_cal_index) */
    rv = qax_ingress_scheduler_shaper_values_get(
            unit,
            TRUE,
            &shaper_mem,
            &full_rate_shaper
    );
    SOCDNX_IF_ERR_EXIT(rv);

    /* Set HP/LP Multicast values of slow start mechanism */

    /* 1. Enable slow start */
    if (shaper_info->slow_start_enable != SOC_TMC_ING_SCH_DONT_TOUCH)
    {
        int index = is_hp? QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_SLOW_START_CONFIG_INDEX : QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_SLOW_START_CONFIG_INDEX;
        int val = (shaper_info->slow_start_enable)? 1 : 0;
        SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, PTS_SHAPER_FMC_CFGm, MEM_BLOCK_ANY, index, shaper_table_entry));
        soc_mem_field32_set(unit, PTS_SHAPER_FMC_CFGm, shaper_table_entry, SLOW_START_ENf, val);
        SOCDNX_IF_ERR_EXIT(soc_mem_write(unit, PTS_SHAPER_FMC_CFGm, MEM_BLOCK_ANY, index, shaper_table_entry));
    }

    /* 2. Set HP/LP Multicast slow rate phase 0- delay and cal */
    if (shaper_info->slow_start_rate_phase_0 != SOC_TMC_ING_SCH_DONT_TOUCH)
    {
        rv = qax_ingress_scheduler_slow_start_phase_set (
                unit,
                shaper_info,
                &full_rate_shaper, /*contains normal max_rate*/
                is_hp,
                0
         );
        SOCDNX_IF_ERR_EXIT(rv);
    }

    /* 3. Set HP/LP Multicast slow rate phase 1- delay and cal */
    if (shaper_info->slow_start_rate_phase_1 != SOC_TMC_ING_SCH_DONT_TOUCH)
    {
        rv = qax_ingress_scheduler_slow_start_phase_set (
                unit,
                shaper_info,
                &full_rate_shaper, /*contains normal max_rate*/
                is_hp,
                1
        );
        SOCDNX_IF_ERR_EXIT(rv);
    }

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*     This procedure returns the slow start phase0/phase1
*     rates (ShaperDelay, ShaperCal) of HP/LP MC
*     of the clos/mesh scheduling scheme.
*     shaper_info struct: using only the fields:
*     1. shaper_info->max_rate
*     2. shaper_info->slow_start_rate_phase_0/1                                                                                                                                                                                        .
*********************************************************************/
STATIC soc_error_t
qax_ingress_scheduler_slow_start_phase_get (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT  ARAD_ING_SCH_SHAPER *shaper_info,
    SOC_SAND_IN int                  is_hp,
    SOC_SAND_IN int                  is_phase_1
    )
{
    qax_ingress_scheduler_mem_field_t shaper_mem;
    ARAD_ING_SCH_SHAPER shaper_slow_phase_config;
    soc_error_t rv = SOC_E_NONE;
    uint32 val = 0;
    SOCDNX_INIT_FUNC_DEFS;

    /* Get HP/LP Multicast slow rate phase0/phase1 */
    shaper_mem.delay_cal_mem = PTS_SHAPER_FMC_CFGm;
    shaper_mem.delay_field = is_phase_1? SLOW_START_DELAY_1f : SLOW_START_DELAY_0f;
    shaper_mem.cal_field = is_phase_1? SLOW_START_CAL_1f : SLOW_START_CAL_0f;
    shaper_mem.delay_cal_index = is_hp? QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_SLOW_START_CONFIG_INDEX : QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_SLOW_START_CONFIG_INDEX;

    /* get shaper_slow_phase_config.max_rate */
    SOC_TMC_ING_SCH_SHAPER_clear(&shaper_slow_phase_config);
    rv = qax_ingress_scheduler_shaper_values_get(
            unit,
            TRUE,
            &shaper_mem,
            &shaper_slow_phase_config
    );
    SOCDNX_IF_ERR_EXIT(rv);

    if (shaper_info->max_rate != 0) /* the NORMAL (100%) rate */
    {
        val = shaper_slow_phase_config.max_rate * 100 / shaper_info->max_rate;
    }
    if (is_phase_1)
    {
        shaper_info->slow_start_rate_phase_1 = val;
    } else
    {
        shaper_info->slow_start_rate_phase_0 = val;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     This procedure returns the whole slow start configuration
*     (rates, enable/disable) of HP/LP MC
*     of the clos/mesh scheduling scheme.
*     shaper_info struct: using only the fields:
*     1. shaper_info->max_rate, shaper_info->slow_start_enable,
*     2. shaper_info->slow_start_rate_phase_0/1 in
*     qax_ingress_scheduler_slow_start_phase_get                                                                                                                                                                                               .
*********************************************************************/
STATIC soc_error_t
qax_ingress_scheduler_slow_start_configuration_get (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT  ARAD_ING_SCH_SHAPER *shaper_info,
    SOC_SAND_IN int                  is_hp
    )
{
    qax_ingress_scheduler_mem_field_t shaper_mem;
    uint32 shaper_table_entry[4] = {0};
    soc_error_t rv = SOC_E_NONE;
    int index;
    SOCDNX_INIT_FUNC_DEFS;

    /* get HP/LP Multicast Shaper in order to calculate NORMAL (100%) shaper rate */
    shaper_mem.delay_cal_mem = PTS_PER_SHAPER_CFGm;
    shaper_mem.delay_field = SHAPER_DELAYf;
    shaper_mem.cal_field = SHAPER_CALf;
    shaper_mem.delay_cal_index = is_hp? QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_INDEX : QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_INDEX;

    /* get shaper_info->max_rate of the NORMAL (100%) rate, calculated from delay and cal fields (for HP/LP, depending on delay_cal_index) */
    rv = qax_ingress_scheduler_shaper_values_get(
            unit,
            TRUE,
            &shaper_mem,
            shaper_info
    );
    SOCDNX_IF_ERR_EXIT(rv);

    /* Get HP/LP Multicast values of slow start mechanism */
    /* 1. is slow start enabled */
    index = is_hp? QAX_INGRESS_SCHEDULER_HP_MC_SHAPER_SLOW_START_CONFIG_INDEX : QAX_INGRESS_SCHEDULER_LP_MC_SHAPER_SLOW_START_CONFIG_INDEX;
    SOCDNX_IF_ERR_EXIT(soc_mem_read(unit, PTS_SHAPER_FMC_CFGm, MEM_BLOCK_ANY, index, shaper_table_entry));
    shaper_info->slow_start_enable = soc_mem_field32_get(unit, PTS_SHAPER_FMC_CFGm, shaper_table_entry, SLOW_START_ENf);

    /* 2. Get HP/LP Multicast slow rate phase 0- delay and cal */
    rv = qax_ingress_scheduler_slow_start_phase_get (
            unit,
            shaper_info,
            is_hp,
            0
    );
    SOCDNX_IF_ERR_EXIT(rv);

    /* 3. Get HP/LP Multicast slow rate phase 1- delay and cal */
    rv = qax_ingress_scheduler_slow_start_phase_get (
            unit,
            shaper_info,
            is_hp,
            1
    );
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     This procedure returns the whole slow start configuration
*     (rates, enable/disable) of the clos scheduling scheme.
*     clos info struct: using only
*     clos_info->shapers.hp/lp.fabric_multicast structs.                                                                                                                                                                                                    .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_clos_slow_start_get (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_INFO   *clos_info
    )
{
    soc_error_t rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* get HP Multicast configuration */
    rv = qax_ingress_scheduler_slow_start_configuration_get (
            unit,
            &clos_info->shapers.hp.fabric_multicast,
            1
    );
    SOCDNX_IF_ERR_EXIT(rv);

    /* get LP Multicast configuration */
    rv = qax_ingress_scheduler_slow_start_configuration_get (
            unit,
            &clos_info->shapers.lp.fabric_multicast,
            0
    );
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*     This procedure configures the slow start configuration
*     (rates/enable/disable) of the clos scheduling scheme.
*     clos info struct: using only
*     clos_info->shapers.hp/lp.fabric_multicast structs.                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_clos_slow_start_set (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_INFO   *clos_info
    )
{
    soc_error_t rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* set HP Multicast configuration */
    rv = qax_ingress_scheduler_slow_start_configuration_set (
        unit,
        &clos_info->shapers.hp.fabric_multicast,
        1
    );
    SOCDNX_IF_ERR_EXIT(rv);

    /* set LP Multicast configuration */
    rv = qax_ingress_scheduler_slow_start_configuration_set (
        unit,
        &clos_info->shapers.lp.fabric_multicast,
        0
    );
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*     This procedure returns the whole slow start configuration
*     (rates, enable/disable) of the mesh scheduling scheme                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_mesh_slow_start_get (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_OUT SOC_TMC_ING_SCH_SHAPER   *shaper_info
    )
{
    soc_error_t rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* get HP Multicast configuration. HP and LP are configured the same for mesh */
    rv = qax_ingress_scheduler_slow_start_configuration_get (
            unit,
            shaper_info,
            1
    );
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configures the slow start configuration
*     (rates/enable/disable) of the mesh scheduling scheme                                                                                                                                                                                              .
*********************************************************************/
soc_error_t
qax_ingress_scheduler_mesh_slow_start_set (
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN SOC_TMC_ING_SCH_SHAPER   *shaper_info
    )
{
    soc_error_t rv = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* setting HP and LP Multicast slow start configuration to be exactly the same */
    /* set HP Multicast configuration */
    rv = qax_ingress_scheduler_slow_start_configuration_set (
            unit,
            shaper_info,
            1
    );
    SOCDNX_IF_ERR_EXIT(rv);

    /* set LP Multicast configuration */
    rv = qax_ingress_scheduler_slow_start_configuration_set (
            unit,
            shaper_info,
            0
    );
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}
