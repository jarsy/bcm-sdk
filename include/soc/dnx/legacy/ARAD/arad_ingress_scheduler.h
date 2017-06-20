/* $Id: jer2_arad_ingress_scheduler.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_ARAD_INGRESS_SCHEDULER_INCLUDED__
/* { */
#define __JER2_ARAD_INGRESS_SCHEDULER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_ingress_scheduler.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_ING_SCH_MAX_DELAY_VAL   0xFFFF
#define JER2_ARAD_ING_SCH_MIN_DELAY_VAL   4

#define JER2_ARAD_ING_SCH_MAX_CAL_VAL     0xFFFF
#define JER2_ARAD_ING_SCH_MIN_CAL_VAL     0
#define JER2_ARAD_ING_SCH_FIRST_CAL_VAL   1024

#define JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS JER2_ARAD_ING_SCH_MESH_LAST

#define JER2_ARAD_ING_SCH_MAX_RATE_MIN 15
#define JER2_ARAD_ING_SCH_MAX_RATE_MAX 120000000

/* } */

/*************
 * MACROS    *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct {
    soc_reg_t   reg;
    soc_field_t field;
    int         index;
} reg_field;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_init
* FUNCTION:
*     Initialization of the JER2_ARAD blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
soc_error_t
  jer2_arad_ingress_scheduler_init(
    DNX_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_verify
* TYPE:
*   PROC
* DATE:
*   Sep 30 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Sep 30 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *exact_mesh_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Sep 30 2007
* FUNCTION:
*     MESH fabric (no FE) configure the ingress scheduler. The
*     configuration includes: [per-destination]-shaper-rates,
*     [per-destination]-weights )
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info -
*     mesh_info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_mesh_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_MESH_INFO   *mesh_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_verify
* TYPE:
*   PROC
* DATE:
*   Sep 30 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Sep 30 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Sep 30 2007
* FUNCTION:
*     This procedure configure the ingress scheduler when
*     working with DNX_SAND CLOS fabric (that is DNX_SAND_FE200/DNX_SAND_FE600). The
*     configuration includes: [local/fabric]-shaper-rates,
*     [local/fabric]-weights.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_weights_set
* TYPE:
*   PROC
* DATE:
*   Oct 02 2007
* FUNCTION:
*     This procedure writes all the weights in the clos_info structure
*     to the suitable registers fields.
* INPUT:
*     DNX_SAND_IN  int unit - for writing fields.
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   clos_info - pointer to the
*           ingress scheduler clos information, where the weights are
*           kept.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_ingress_scheduler_clos_weights_set(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
     );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_global_shapers_set
* TYPE:
*   PROC
* DATE:
*   Oct 02 2007
* FUNCTION:
*     This procedure writes the values of the global shapers (fabric and local)
*     in the clos_info structure to the suitable registers fields.
* INPUT:
*     DNX_SAND_IN  int unit - for writing fields.
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   clos_info - pointer to the
*           ingress scheduler clos information, where the shapers are
*           kept.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
   jer2_arad_ingress_scheduler_clos_global_shapers_set(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
     DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_shaper_values_set
* TYPE:
*   PROC
* DATE:
*   Oct 02 2007
* FUNCTION:
*     This procedure writes the values a given shapers (max_burst, max_rate)
*     in the clos_info structure to the suitable registers fields
*     (ShaperMaxCredit, ShaperDelay, ShaperCal). The ShaperDelay & ShaperCal
*     fields are retrieved using an additional function that converts max_rate
*     to the suitable values.
* INPUT:
*     DNX_SAND_IN  int unit - for writing fields.
*     DNX_SAND_IN  int is_delay_2_clocks - Does the delay represented in 2 clocks delay
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_SHAPER shaper - the shaper
*               structure which its parameters are to be written to the registers.
*     DNX_SAND_IN  reg_field max_credit_fld - the field in which to write the
*               max_burst value.
*     DNX_SAND_IN  reg_field delay_fld - the field in which to write the delay
*               value (in 2 clock cycles resolution) derived from the max_rate
*               value.
*     DNX_SAND_IN  reg_field cal_fld - the field in which to write the cal
*               value (in bytes resolution) derived from the max_rate value.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_shaper_values_set(
       DNX_SAND_IN  int                  unit,
       DNX_SAND_IN  int                     is_delay_2_clocks,
       DNX_SAND_IN  JER2_ARAD_ING_SCH_SHAPER     *shaper,
       DNX_SAND_IN  reg_field               *max_credit_fld,
       DNX_SAND_IN  reg_field               *delay_fld,
       DNX_SAND_IN  reg_field               *cal_fld,
       DNX_SAND_OUT uint32                  *exact_max_rate
      );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_rate_to_delay_cal_form
* TYPE:
*   PROC
* DATE:
*   Oct 02 2007
* FUNCTION:
*     This procedure converts the ingress scheduler shaper max_rate
*     given in kbps to the values of the register fields-
*     1. ShaperDelay: Time interval to add the credit.
*        in two clocks cycles resolution.
*     2. ShaperCal: Credit to add, in bytes resolution.
* INPUT:
*   DNX_SAND_IN  int            unit - for reading device ticks
*   DNX_SAND_IN  uint32             max_rate - the value to be converted.
*   DNX_SAND_OUT uint32*            shaper_delay - pointer for return.
*   DNX_SAND_OUT uint32*            shaper_cal - pointer for return.
* RETURNS:
*   DNX_SAND_INDIRECT:
*     shaper_delay - the ShaperDelay register field.
*     shaper_cal - the ShaperCal register field.
*   OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_ingress_scheduler_rate_to_delay_cal_form(
     DNX_SAND_IN  int            unit,
     DNX_SAND_IN  uint32         max_rate,
     DNX_SAND_IN  int            is_delay_2_clocks,
     DNX_SAND_OUT uint32*        shaper_delay_2_clocks,
     DNX_SAND_OUT uint32*        shaper_cal,
     DNX_SAND_OUT uint32*        exact_max_rate
     );
/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_shaper_values_get
* TYPE:
*   PROC
* DATE:
*   Oct 09 2007
* FUNCTION:
*     This procedure reads the values of the the shaper (max_burst, max_rate)
*     from the suitable registers fields (ShaperMaxCredit, ShaperDelay, ShaperCal).
*     The max_rate vallue of the Shaper structure is retrieved from the
*     ShaperDelay & ShaperCal fields, using an additional function that
*     converts the suitable values to max_rate.
* INPUT:
*      DNX_SAND_IN   int            unit - for reading fields.
*      DNX_SAND_IN   int            is_delay_2_clocks - Does the delay represented in 2 clocks delay
*      DNX_SAND_IN   reg_field        *max_credit_fld - the field in which to
*                                             read the max_burst value.
*      DNX_SAND_IN   reg_field        *delay_fld - the field in which to
*                                             read the delay value.
*      DNX_SAND_IN   reg_field        *cal_fld - the field in which to
*                                             read the cal value.
*      DNX_SAND_OUT  JER2_ARAD_ING_SCH_SHAPER *shaper - the shaper structure which its
*                           parameters are to be received from registers
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_shaper_values_get(
    DNX_SAND_IN   int                 unit,
    DNX_SAND_IN   int                 is_delay_2_clocks,
    DNX_SAND_IN   reg_field              *max_credit_fld,
    DNX_SAND_IN   reg_field              *delay_fld,
    DNX_SAND_IN   reg_field              *cal_fld,
    DNX_SAND_OUT  JER2_ARAD_ING_SCH_SHAPER    *shaper
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_delay_cal_to_max_rate_form
* TYPE:
*   PROC
* DATE:
*   Oct 09 2007
* FUNCTION:
*     This procedure converts the ingress scheduler shaper register field
*     values:
*     1. ShaperDelay: Time interval to add the credit.
*        in two clocks cycles resolution.
*     2. ShaperCal: Credit to add, in bytes resolution.
*
*     to the max_rate value of the shaper structure in the API (in kbps)
*
* INPUT:
*   DNX_SAND_IN  int         unit - for reading device ticks
*   DNX_SAND_IN  uint32          shaper_delay - the reg field value
*                                           (Time interval to add the credit).
*   DNX_SAND_IN  uint32          shaper_cal - the reg field value
*                                            (Credit to add, in bytes resolution).
*   DNX_SAND_OUT uint32          *max_rate - the value returned (in kbps)
* RETURNS:
*   DNX_SAND_INDIRECT:
*     shaper_delay - the ShaperDelay register field.
*     shaper_cal - the ShaperCal register field.
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_delay_cal_to_max_rate_form(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  uint32          shaper_delay,
    DNX_SAND_IN  uint32          shaper_cal,
    DNX_SAND_OUT uint32          *max_rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_mesh_reg_flds_db_get
* TYPE:
*   PROC
* DATE:
*   Oct 09 2007
* FUNCTION:
*     This procedure is used for the Mesh topology. This procedure initializes
*     arrays of shaper_max_crdts, shaper_delays and shaper_cals with the
*     appropriate fields. This is done fo0r easier implementation of the set and
*     get functions.
* INPUT:
*     DNX_SAND_IN  int       unit - for reading fields.
*     reg_field          *wfq_weights - an array of the wfq fields (to return).
*     reg_field          *shaper_max_crdts - an array of the maxCredits fields (to return).
*     reg_field          *shaper_delays - an array of the delay fields (to return).
*     reg_field          *shaper_cals - an array of the cal fields (to return).
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

uint32
  jer2_arad_ingress_scheduler_mesh_reg_flds_db_get(
    DNX_SAND_IN  int       unit,
    reg_field          wfq_weights[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    reg_field          shaper_max_crdts[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    reg_field          shaper_delays[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS],
    reg_field          shaper_cals[JER2_ARAD_ING_SCH_NUM_OF_CONTEXTS]
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_weights_get
* TYPE:
*   PROC
* DATE:
*   Oct 09 2007
* FUNCTION:
*     This procedure read all the weights in the suitable registers fields
*     to the clos_info structure.
* INPUT:
*     DNX_SAND_IN  int unit - for reading fields.
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   clos_info - pointer to the
*           ingress scheduler clos information, where the weights are
*           kept.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
   jer2_arad_ingress_scheduler_clos_weights_get(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_global_shapers_get
* TYPE:
*   PROC
* DATE:
*   Oct 09 2007
* FUNCTION:
*     This procedure reads the values of the global shapers (fabric and local)
*     to the clos_info structure from the suitable registers fields.
* INPUT:
*     DNX_SAND_IN  int unit - for reading fields.
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   clos_info - pointer to the
*           ingress scheduler clos information, where the shapers are
*           kept.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
   jer2_arad_ingress_scheduler_clos_global_shapers_get(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_exact_cal_value
* TYPE:
*   PROC
* DATE:
*   Oct 11 2007
* FUNCTION:
*     This procedure returns a more exact cal value.
*     in order to get an exact cal value
*     calculate:
*        cal_value * max_rate / exact_rate_in_kbits_per_sec;
* INPUT:
*   void.
* RETURNS:
*   DNX_SAND_DIRECT:
*
*
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_exact_cal_value(
    DNX_SAND_IN uint32  cal_value,
    DNX_SAND_IN uint32  max_rate,
    DNX_SAND_IN uint32  exact_rate_in_kbits_per_sec,
    DNX_SAND_OUT uint32 *exact_cal_value_long
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_conversion_test
* TYPE:
*   PROC
* DATE:
*   Oct 12 2007
* FUNCTION:
*     This procedure perform a test that compares the rate values to
*     the exact rate values that are received after the
*     conversion function.
*     The procedure tests 2 main criteria:
*     1. That the exact error percentage from the rate does not exceed limit.
*     2. That the exact is always larger than rate.
* INPUT:
*   DNX_SAND_IN uint8 is_regression - if true less values are checked.
*   DNX_SAND_IN uint8 silent - whether to perform prints (True/False)
* RETURNS:
*   DNX_SAND_DIRECT: uint8 pass - whether the test has passed or not.
*
*
*********************************************************************/

uint8
  jer2_arad_ingress_scheduler_conversion_test(
    DNX_SAND_IN uint8 is_regression,
    DNX_SAND_IN uint8 silent
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_hp_shapers_set
* TYPE:
*   PROC
* DATE:
*   June 17 2012
* FUNCTION:
*     This procedure writes the values of the high priority shapers
*     (fabric and local) in the clos_info structure to the suitable
*     registers fields.
* INPUT:
*  DNX_SAND_IN  int                   unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info -
* RETURNS:
*   OK or ERROR indication.
*
*
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_hp_shapers_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_lp_shapers_set
* TYPE:
*   PROC
* DATE:
*   June 17 2012
* FUNCTION:
*     This procedure writes the values of the low priority shapers
*     in the clos_info structure to the suitable registers fields.
* INPUT:
*  DNX_SAND_IN  int                   unit -
*  DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info -
*     clos_info pointer to configuration structure.
*  DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info -
* RETURNS:
*   OK or ERROR indication.
*
*
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_lp_shapers_set(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info,
    DNX_SAND_OUT JER2_ARAD_ING_SCH_CLOS_INFO   *exact_clos_info
    );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_hp_shapers_get
* TYPE:
*   PROC
* DATE:
*   June 17 2012
* FUNCTION:
*     This procedure reads the values of the high priority shapers
*     (fabric and local) to the clos_info structure from the suitable
*     registers fields.
* INPUT:
*     DNX_SAND_IN  int unit - for reading fields.
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   clos_info - pointer to the
*           ingress scheduler clos information, where the shapers are
*           kept.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_hp_shapers_get(
    DNX_SAND_IN   int                   unit,
    DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ingress_scheduler_clos_lp_shapers_get
* TYPE:
*   PROC
* DATE:
*   June 17 2012 
* FUNCTION:
*     This procedure reads the values of the low priority shapers
*     to the clos_info structure from the suitable registers fields.
* INPUT:
*     DNX_SAND_IN  int unit - for reading fields.
*     DNX_SAND_IN  JER2_ARAD_ING_SCH_CLOS_INFO   clos_info - pointer to the
*           ingress scheduler clos information, where the shapers are
*           kept.
*
* OUTPUT:
*   void.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ingress_scheduler_clos_lp_shapers_get(
    DNX_SAND_IN   int                   unit,
    DNX_SAND_OUT  JER2_ARAD_ING_SCH_CLOS_INFO   *clos_info
  );

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_INGRESS_SCHEDULER_INCLUDED__*/
#endif
