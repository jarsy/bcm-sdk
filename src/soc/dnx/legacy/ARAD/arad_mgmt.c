#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_mgmt.c,v 1.100 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*************
 * INCLUDES  *
 *************/
/* { */

#include <sal/core/stats.h>
#include <shared/swstate/access/sw_state_access.h>
#include <shared/shr_template.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_mem.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/mem.h>
#include <soc/drv.h>
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_link.h>

#include <soc/dnx/legacy/ARAD/arad_defs.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/SAND/Management/sand_device_management.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_params.h>

#include <soc/dnx/legacy/port_sw_db.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_PORT_NDX_MAX                                        (JER2_ARAD_NOF_FAP_PORTS-1)
#define JER2_ARAD_CONF_MODE_NDX_MAX                                   (JER2_ARAD_MGMT_NOF_PCKT_SIZE_CONF_MODES - 1)

/* System Frequency factor */
#define JER2_ARAD_MGMT_SYST_FREQ_RES_19 (1 << 19)

#define JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_ITERATIONS 1500
#define JER2_ARAD_MGMT_INIT_CTRL_RCH_STATUS_ITERATIONS  24
#define JER2_ARAD_MGMT_INIT_STANDALONE_ITERATIONS       16
#define JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_DELAY_MSEC 32

/* For port rates of 50G - 100G */
#define JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_XE     8
/* For port rates of 100G - 200G */
#define JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_CAUI   16
/* For port rates of 200G - 400G */
#define JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_ILKN   32

#define JER2_ARAD_MGMT_IQM_OCB_PRM_QUEUE_CATEGORY_START  (9)
#define JER2_ARAD_MGMT_IQM_OCB_PRM_QUEUE_CATEGORY_LENGTH (2)
#define JER2_ARAD_MGMT_IQM_OCB_PRM_RATE_CLASS_START      (3)
#define JER2_ARAD_MGMT_IQM_OCB_PRM_RATE_CLASS_LENGTH     (6)
#define JER2_ARAD_MGMT_IQM_OCB_PRM_TRAFFIC_CLASS_START   (0)
#define JER2_ARAD_MGMT_IQM_OCB_PRM_TRAFFIC_CLASS_LENGTH  (3)

#define JER2_ARAD_MGMT_IQM_OCBPRM_MANTISSA_NOF_BITS(unit) (SOC_IS_ARADPLUS_AND_BELOW(unit) ? 7 : 8)
#define JER2_ARAD_MGMT_IQM_OCBPRM_EXPONENT_NOF_BITS       (5)

#define JER2_ARAD_MGMT_IQM_OCBPRM_BUFF_SIZE_MANTISSA_NOF_BITS(unit) (SOC_IS_ARADPLUS_AND_BELOW(unit) ? 7 : 8)
#define JER2_ARAD_MGMT_IQM_OCBPRM_BUFF_SIZE_EXPONENT_NOF_BITS       (4)

/* value was calculated in the following way:
    (4k for every 10G of the port type) 
    10G  ports : (4 * 1024 * 1)   = 4096
    40G  ports : (4 * 1024 * 4)   = 16384
    100G ports : (4 * 1024 * 10)  = 40960
    200G ports : (4 * 1024 * 20)  = 81920
    biggest value was chosen as default.
    the logic behind this default is to allow for each voq a reasonable burst size for the maximu possible speed of each port without consuming unreasonable amount of OCB resources.
    the prefered approch should be to devide to the VOQs all of the available OCB resources, however doing so is too specific per user's application so those defaults were chosen and users can farther
    optimize the values per application according to thier needs */
#define JER2_ARAD_MGMT_IQM_OCB_PRM_DEFAULT_SIZE          (81920)
/* same as default size only devided by common ocb buff size (256) */
#define JER2_ARAD_MGMT_IQM_OCB_PRM_DEFAULT_BUFF_SIZE     (320)

/*
 * PVT monitor
 */
#define _SOC_JER2_ARAD_PVT_MON_NOF                             (2)
#define _SOC_JER2_ARAD_PVT_FACTOR                              (5660)
#define _SOC_JER2_ARAD_PVT_BASE                                (4283900)

#define _SOC_ARDON_ECI_PVT_MON_CONTROL_REG_POWERDOWN_BIT   (32)
#define _SOC_ARDON_ECI_PVT_MON_CONTROL_REG_RESET_BIT       (33)
#define _SOC_ARDON_PVT_MON_NOF                             (1)
#define _SOC_ARDON_PVT_FACTOR                              (48517)
#define _SOC_ARDON_PVT_BASE                                (41016000)

/*
 * The number of fields being enabled / disabled while enabling / disabling ctrl cells
 */
#define JER2_ARAD_MGMT_CTRL_CELLS_MAX_NOF_CONFS 100

#define JER2_ARAD_MGMT_DBG_ON                   0

#if JER2_ARAD_MGMT_DBG_ON
  #define JER2_ARAD_MGMT_TRACE(iter_index)                         \
  {                                                           \
    if (iter_index == 0)                                      \
    {                                                         \
      LOG_CLI( \
               (BSL_META_U(unit, \
                           " %s\r\n"), FUNCTION_NAME()));            \
    }                                                         \
  }
#else
  #define JER2_ARAD_MGMT_TRACE(iter_index)
#endif

extern char *_build_release;


/*
 *  Chip identification fields (Version Register).
 *  Needed by register_device, before access database is initialized
 */
/* } */

/*************
 *  MACROS   *
 *************/

/* { */


typedef uint32 (*JER2_ARAD_CTRL_CELL_POLL_FUNC) (DNX_SAND_IN  int unit, DNX_SAND_IN uint32 iter_index, DNX_SAND_OUT uint8 *success) ;
typedef uint32 (*JER2_ARAD_CTRL_CELL_FNLY_FUNC) (DNX_SAND_IN  int unit) ;

typedef struct
{
  soc_reg_t                reg;
  soc_field_t              field;

  uint32                  instance_id;

  JER2_ARAD_CTRL_CELL_POLL_FUNC   polling_func;

  JER2_ARAD_CTRL_CELL_FNLY_FUNC  failure_func;

  uint32                    val;

  /*
   *  In case a polling function is used,
   *  this is the number of iterations (at least one is performed).
   *  Otherwise - this is the delay in milliseconds.
   */
  uint32                    delay_or_polling_iters;

  uint32                    err_on_fail;

}JER2_ARAD_CTRL_CELL_DATA;

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

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


static uint32
  jer2_arad_mgmt_module_init(int unit)
{
  uint32
    res;

  DNX_SAND_INTERRUPT_INIT_DEFS;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MODULE_INIT);

  /* Stopping interrupts, since the following functions are global, and not device specific
     (device semaphore can't work here). Possible optimization - maintain another global mutex, in
     order not to stop interrupts for this */
  DNX_SAND_INTERRUPTS_STOP;

  res = jer2_arad_chip_defines_init(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = jer2_arad_sw_db_init(
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* Now add list of JER2_ARAD procedure descriptors to all-system pool.                                     */
  res = jer2_arad_procedure_desc_add() ;
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

exit:
  DNX_SAND_INTERRUPTS_START_IF_STOPPED;

  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_module_init()",0,0);
}

static uint32
  jer2_arad_mgmt_device_init(
    DNX_SAND_IN  int  unit
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_DEVICE_INIT);

  res = jer2_arad_sw_db_device_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_device_init()",0,0);
}

static uint32
  jer2_arad_mgmt_device_close(
    DNX_SAND_IN  int  unit
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_DEVICE_CLOSE);

  res = jer2_arad_sw_db_device_close(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_device_close()",0,0);
}


/*********************************************************************
*     This procedure registers a new device to be taken care
*     of by this device driver. Physical device must be
*     accessible by CPU when this call is made..
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_register_device_unsafe(
             uint32                  *base_address,
    DNX_SAND_IN  DNX_SAND_RESET_DEVICE_FUNC_PTR reset_device_ptr,
    DNX_SAND_INOUT int                 *unit_ptr
  )
{
  uint32
    res;
  int
    unit = *unit_ptr;
  DNX_SAND_DEV_VER_INFO
    ver_info;
  JER2_ARAD_REG_FIELD
    chip_type_fld,
    dbg_ver_fld,
    chip_ver_fld;
  uint32
    *base;
  uint32
    reg_val;
  soc_error_t
    rv = SOC_E_NONE;

  DNX_SAND_INTERRUPT_INIT_DEFS;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_REGISTER_DEVICE_UNSAFE);

  unit = (*unit_ptr);

  res = jer2_arad_mgmt_module_init(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 5, exit);


  dnx_sand_clear_SAND_DEV_VER_INFO(&ver_info);
  chip_type_fld.lsb = JER2_ARAD_MGMT_CHIP_TYPE_FLD_LSB;
  chip_type_fld.msb = JER2_ARAD_MGMT_CHIP_TYPE_FLD_MSB;
  dbg_ver_fld.lsb   = JER2_ARAD_MGMT_DBG_VER_FLD_LSB;
  dbg_ver_fld.msb   = JER2_ARAD_MGMT_DBG_VER_FLD_MSB;
  chip_ver_fld.lsb  = JER2_ARAD_MGMT_CHIP_VER_FLD_LSB;
  chip_ver_fld.msb  = JER2_ARAD_MGMT_CHIP_VER_FLD_MSB;

  ver_info.ver_reg_offset = JER2_ARAD_MGMT_VER_REG_BASE;
  ver_info.logic_chip_type= DNX_SAND_DEV_JER2_ARAD;
  ver_info.chip_type      = JER2_ARAD_EXPECTED_CHIP_TYPE;

  ver_info.chip_type_shift= JER2_ARAD_FLD_SHIFT_OLD(chip_type_fld);
  ver_info.chip_type_mask = JER2_ARAD_FLD_MASK_OLD(chip_type_fld);
  ver_info.dbg_ver_shift  = JER2_ARAD_FLD_SHIFT_OLD(dbg_ver_fld);
  ver_info.dbg_ver_mask   = JER2_ARAD_FLD_MASK_OLD(dbg_ver_fld);
  ver_info.chip_ver_shift = JER2_ARAD_FLD_SHIFT_OLD(chip_ver_fld);
  ver_info.chip_ver_mask  = JER2_ARAD_FLD_MASK_OLD(chip_ver_fld);

  /* Skip verification since Arad is CMIC-based */
  ver_info.cmic_skip_verif = TRUE;


  base = (uint32*)base_address;

  /*
   *  Validate ECI access to the Arad
   */
    reg_val = 0xaaff5500;
    SOC_DNX_ALLOW_WARMBOOT_WRITE(WRITE_ECI_TEST_REGISTERr(*unit_ptr, reg_val), rv);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 1000, exit);
    READ_ECI_TEST_REGISTERr(*unit_ptr, &reg_val);
#ifdef SAL_BOOT_PLISIM
    if ((SAL_BOOT_PLISIM) == 0)
    {
        if (reg_val != ~(0xaaff5500))
        {
          DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ECI_ACCESS_ERR, 49, exit);
        }
    }
    else 
#endif /* SAL_BOOT_PLISIM */
    {
        if (reg_val != (0xaaff5500))
        {
          DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ECI_ACCESS_ERR, 49, exit);
        }
    }


  /* Register the device in DNX_SAND.                                                                         */
  res = dnx_sand_device_register(
          base,
          JER2_ARAD_TOTAL_SIZE_OF_REGS,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          reset_device_ptr,
          NULL,
          &ver_info,
          NULL,
          0xFFFFFFFF,                         /* Given invalid address as the general mask bit.           */
                                              /* This bit do not exist on the JER2_ARAD                      */
          &unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  *unit_ptr = unit ;

  res = jer2_arad_mgmt_sw_ver_set_unsafe(unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 55, exit);

  /* Stopping interrupts, since error/proc desc is not device specific, but global */
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  DNX_SAND_INTERRUPTS_STOP; 
  {
      /* Add list of PB_PP errors to all-system errors pool                                                  */
      res = jer2_arad_pp_errors_desc_add();
      DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

      /* Now add list of PB_PP procedure descriptors to all-system pool.                                     */
      res = jer2_arad_pp_procedure_desc_add() ;
      DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  }
  DNX_SAND_INTERRUPTS_START_IF_STOPPED;
#endif 

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_device_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  SOC_DNX_WARMBOOT_RELEASE_HW_MUTEX(rv);
  if(rv != SOC_E_NONE) {
    LOG_ERROR(BSL_LS_SOC_INIT,
              (BSL_META_U(unit,
                          " Failed while executing the macro SOC_DNX_WARMBOOT_RELEASE_HW_MUTEX.\n")));
  }
  DNX_SAND_INTERRUPTS_START_IF_STOPPED;
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_register_device_unsafe()",0,0);
}

/*********************************************************************
*     Undo jer2_arad_register_device()
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_unregister_device_unsafe(
    DNX_SAND_IN  int        unit
  )
{
  uint32
    res = DNX_SAND_OK;
  int32
    dnx_sand_ret;
  DNX_SAND_RET
    ret;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_UNREGISTER_DEVICE_UNSAFE);
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
/* #ifdef JER2_ARAD_PP */
  if (1/*jer2_arad_sw_db_pp_enable_get(unit)*/)
  {
    res = jer2_arad_pp_mgmt_device_close(
            unit
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
/* #endif */
#endif 
  res = jer2_arad_mgmt_device_close(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  /*
   * Take mutexes in the right order -> delta_list, device, rest of them
   */
  if (DNX_SAND_OK != dnx_sand_tcm_callback_delta_list_take_mutex())
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_SEM_TAKE_FAIL, 2, exit) ;
  }

  dnx_sand_ret = dnx_sand_take_chip_descriptor_mutex(unit) ;
  if (DNX_SAND_OK != dnx_sand_ret)
  {
    if (DNX_SAND_ERR == dnx_sand_ret)
    {
      dnx_sand_tcm_callback_delta_list_give_mutex();
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_SEM_TAKE_FAIL, 3, exit);
    }
    if (0 > dnx_sand_ret)
    {
      dnx_sand_tcm_callback_delta_list_give_mutex();
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ILLEGAL_DEVICE_ID, 4, exit) ;
    }
  }
  /* semaphore taken successfully */

  /*
   * Give device mutex back
   */
  if (DNX_SAND_OK != dnx_sand_give_chip_descriptor_mutex(unit))
  {
    dnx_sand_tcm_callback_delta_list_give_mutex();
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_SEM_GIVE_FAIL, 5, exit) ;
  }
  /*
   * Give list mutex back
   */
  if (DNX_SAND_OK != dnx_sand_tcm_callback_delta_list_give_mutex())
  {
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_SEM_GIVE_FAIL, 6, exit) ;
  }

  ret = dnx_sand_device_unregister(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(ret, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_unregister_device_unsafe()",0,0);
}
/*
   [31:20] - SDK version of the last regular init;
   [19:8] - if ISSU took place then set the SDK version after ISSU;
   [7:4] - Did WB take place?(=0x0,0x1) ;
   [3:0] - Did ISSU take place? (=0x0,0x1) 
*/
uint32
  jer2_arad_mgmt_sw_ver_set_unsafe(
    DNX_SAND_IN  int                      unit
  )
{
    int      ver_val[3] = {0,0,0};
    uint32   prev_ver_val[3] = {0,0,0};
    uint32   regval, i, prev_regval;
    char     *ver;
    int      wb, issu, bit_ndx ;
  soc_error_t
    rv;
  char
    *cur_number_ptr;
  soc_reg_t  ver_reg;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_SW_VER_UNSAFE);

  regval = 0;
  wb = 0;
  issu = 0;


  ver = _build_release;
  cur_number_ptr = sal_strchr(ver, '-');
  if(cur_number_ptr == NULL) {
      DNX_SAND_EXIT_AND_SEND_ERROR_DNX((_BSL_DNXC_MSG_STR("Invalid version format.")));
  }
  ++cur_number_ptr;
  ver_val[0] = _shr_ctoi (cur_number_ptr);
  cur_number_ptr = sal_strchr(cur_number_ptr, '.');
  if(cur_number_ptr == NULL) {
      DNX_SAND_EXIT_AND_SEND_ERROR_DNX((_BSL_DNXC_MSG_STR("Invalid version format.")));
  }
  ++cur_number_ptr;
  ver_val[1] = _shr_ctoi (cur_number_ptr);
  cur_number_ptr = sal_strchr(cur_number_ptr, '.');
  if(cur_number_ptr == NULL) {
      DNX_SAND_EXIT_AND_SEND_ERROR_DNX((_BSL_DNXC_MSG_STR("Invalid version format.")));
  }
  ++cur_number_ptr;
  ver_val[2] = _shr_ctoi (cur_number_ptr);
  
  ver_reg = SOC_IS_JERICHO(unit)? ECI_SW_VERSIONr: ECI_REG_0093r;

  /*	[31:20] - SDK version of the last regular init */
  if (SOC_WARM_BOOT(unit)) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(rv,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg32_get(unit, ver_reg, REG_PORT_ANY,  0, &prev_regval));

      wb = 1;
      bit_ndx = 28;

      for (i=0; i<3; i++) {
          /* Get last regular init version*/
          prev_ver_val[i]= ( prev_regval >> (bit_ndx - i*4)) & 0xf;

          if (prev_ver_val[i] != ver_val[i]) {
                issu = 1;
          }
          regval = (regval | (0xf & prev_ver_val[i])) << 4;
      }

  }else {
      for (i=0; i<3; i++) {
          regval = (regval | (0xf & ver_val[i])) << 4;
      }
  }

  /* If issu set current version in 19:8*/
  for (i=0; i<3; i++) {
      if (issu) {
          regval = (regval | (0xf & ver_val[i]));
      }
      regval = regval << 4;
  }

  /* 7:4 is wb */
  regval = (regval | (0xf & wb)) << 4;

  /* 3:0 is issu */
  regval = (regval | (0xf & issu));

  SOC_DNX_ALLOW_WARMBOOT_WRITE(soc_reg32_set(unit, ver_reg, REG_PORT_ANY, 0, regval), rv);
  
  DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 1000, exit);
 
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_sw_ver_set_unsafe()",0,0);
}
 

/***********************************************************************************
* Helper utility for *_mgmt_credit_worth_set():
* Calculate value for SCH_ASSIGNED_CREDIT_CONFIGURATION, SCH_REBOUNDED_CREDIT_CONFIGURATION,
* given input 'credit_worth'
* Output goes into *fld_val to be loaded into registers specified above, into 
* ASSIGNED_CREDIT_WORTH, REBOUNDED_CREDIT_WORTH fields, correspondingly
***********************************************************************************/
uint32
  jer2_arad_calc_assigned_rebounded_credit_conf(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth,
    DNX_SAND_OUT uint32              *fld_val
  )
{
  uint32
    res ;
  soc_port_t  
    port_i;
  bcm_pbmp_t      
    ports_map;
  soc_port_if_t 
    interface_type;
  uint32
    credit_div;
  uint8
    is_ilkn_used,
    is_caui_used;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_CALC_ASSIGNED_REBOUNDED_CREDIT_CONF);
  DNX_SAND_CHECK_NULL_INPUT(fld_val);
  *fld_val = 0 ;

  /* SCH { */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, dnx_port_sw_db_valid_ports_get(unit, DNX_PORT_FLAGS_NETWORK_INTERFACE, &ports_map));

  is_ilkn_used = FALSE;
  is_caui_used = FALSE;
  BCM_PBMP_ITER(ports_map, port_i) { /* Look for ILKN/CAUI interfaces that need value different than default */
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, dnx_port_sw_db_interface_type_get(unit, port_i, &interface_type));
      if(interface_type == SOC_PORT_IF_ILKN) 
      {
        is_ilkn_used = TRUE;
      } 
      else if(interface_type == SOC_PORT_IF_CAUI) 
      {
        is_caui_used = TRUE;
      }
      if(is_ilkn_used && is_caui_used)
      {
        break; /* No need to continue iterating */
      }
  }

  if(SOC_DNX_CONFIG(unit)->jer2_arad->init.credit.credit_worth_resolution == JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_AUTO || 
     SOC_DNX_CONFIG(unit)->jer2_arad->init.credit.credit_worth_resolution == JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_LOW)
  { 
    if(is_ilkn_used) 
    {
      credit_div = JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_ILKN; /* Allow max rate of 400G */
    } 
    else if(is_caui_used) 
    {
      credit_div = JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_CAUI; /* Allow max rate of 200G */
    }
    else
    {
      credit_div = JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_XE; /* Allow max rate of 100G */
    }
  }
  else
  {
    switch(SOC_DNX_CONFIG(unit)->jer2_arad->init.credit.credit_worth_resolution)
    {
    case JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_HIGH:
      credit_div = JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_XE; /* Always, regardless of interfaces used */
      break;
    case JER2_ARAD_MGMT_CREDIT_WORTH_RESOLUTION_MEDIUM:
      credit_div = JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_CAUI;
      break;
    default:
      /* Should only reach here in case of miss-configuration */
      credit_div = JER2_ARAD_MGMT_DIV_ASSIGNED_CREDIT_WORTH_XE; /* Assign default value */
      break;
    }
  }
  *fld_val = credit_worth / credit_div ;
  /* SCH } */
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_calc_assigned_rebounded_credit_conf()",0,0);
}


/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*     Details: in the H file. (search for prototype)
*     This procedure is for Arad and Arad+
*********************************************************************/
int
  jer2_arad_mgmt_credit_worth_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth
  )
{
  uint32
    fld_val = 0;
  soc_reg_t credit_value_reg;
  soc_field_t credit_value_lcl_field;
  
  DNXC_INIT_FUNC_DEFS ;

  DNXC_SAND_IF_ERR_EXIT(jer2_arad_mgmt_credit_worth_verify(unit,credit_worth)) ;

  fld_val = credit_worth;
  /*
   * Arad: Note that the Jericho register is different than in Arad and the fields
   * are marked '0' and '1' while in Arad, the same fields are marked '1' and '2'.
   */
  credit_value_reg = SOC_IS_ARADPLUS(unit)? IPS_IPS_CREDIT_CONFIG_1r: IPS_IPS_CREDIT_CONFIGr;
  credit_value_lcl_field = SOC_IS_ARADPLUS(unit)? CREDIT_VALUE_1f: CREDIT_VALUEf;

  DNXC_IF_ERR_EXIT(soc_reg_field32_modify(unit, credit_value_reg, REG_PORT_ANY, credit_value_lcl_field, fld_val)) ;

  /* SCH { */
  
  DNXC_SAND_IF_ERR_EXIT(jer2_arad_calc_assigned_rebounded_credit_conf(unit, credit_worth, &fld_val));

  {
    /*
     * This clause will probably change when two calendars feature is
     * added (SCH_ASSIGNED_CREDIT_CONFIGURATION_1, SCH_REBOUNDED_CREDIT_CONFIGURATION_1, ...)
     */
    DNXC_IF_ERR_EXIT(
        soc_reg_above_64_field32_modify(unit, SCH_ASSIGNED_CREDIT_CONFIGURATIONr, SOC_CORE_ALL, 0, ASSIGNED_CREDIT_WORTHf,  fld_val));
    /* Set Rebounded Credit worth equals to assigned credit worth */
    DNXC_IF_ERR_EXIT(
        soc_reg_above_64_field32_modify(unit, SCH_REBOUNDED_CREDIT_CONFIGURATIONr, SOC_CORE_ALL, 0, REBOUNDED_CREDIT_WORTHf,  fld_val));
  }
  /* SCH } */
exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_credit_worth_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              credit_worth
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_CREDIT_WORTH_VERIFY);

  DNX_SAND_ERR_IF_OUT_OF_RANGE(
    credit_worth, DNX_TMC_CREDIT_SIZE_BYTES_MIN, DNX_TMC_CREDIT_SIZE_BYTES_MAX,
    JER2_ARAD_CREDIT_SIZE_OUT_OF_RANGE_ERR, 10, exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_credit_worth_verify()",0,0);
}

/*********************************************************************
*     Bytes-worth of a single credit. It should be configured
*     the same in all the FAPs in the systems, and should be
*     set before programming the scheduler.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_mgmt_credit_worth_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32              *credit_worth
  )
{
    uint32
        reg_val,
        fld_val = 0;
    soc_reg_t credit_value_reg;
    soc_field_t credit_value_lcl_field;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(credit_worth);

    credit_value_reg = SOC_IS_ARADPLUS(unit)? IPS_IPS_CREDIT_CONFIG_1r: IPS_IPS_CREDIT_CONFIGr ;
    credit_value_lcl_field = SOC_IS_ARADPLUS(unit)? CREDIT_VALUE_1f: CREDIT_VALUEf ;
    /*
     * Read selected register and, then, extract the filed.
     */
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, credit_value_reg, REG_PORT_ANY, 0, &reg_val)) ;
    fld_val = soc_reg_field_get(unit, credit_value_reg, reg_val, credit_value_lcl_field) ;
    *credit_worth = fld_val;

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Arad+ only: set local and remote (1 and 2) credit worth values
 */
uint32
  jer2_arad_plus_mgmt_credit_worth_remote_set(
    DNX_SAND_IN  int    unit,
	DNX_SAND_IN  uint32    credit_worth_remote
  )
{
    uint32 res, reg_val;
    uint16 nof_remote_faps_with_remote_credit_value;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

	
	if (credit_worth_remote < DNX_TMC_CREDIT_SIZE_BYTES_MIN || credit_worth_remote > DNX_TMC_CREDIT_SIZE_BYTES_MAX) {
		LOG_ERROR(BSL_LS_SOC_MANAGEMENT, 
				  (BSL_META_U(unit,
							  "Remote size %d is not between %u..%u") , credit_worth_remote, DNX_TMC_CREDIT_SIZE_BYTES_MIN, DNX_TMC_CREDIT_SIZE_BYTES_MAX));
		DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);

	} else {
		uint32 arg_local, arg_remote;
		res = jer2_arad_plus_mgmt_credit_worth_remote_get(unit, &arg_remote);
		DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
		res = jer2_arad_mgmt_credit_worth_get(unit, &arg_local);
		DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
		if (credit_worth_remote != arg_remote) { /* are we changing the value? */
            res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.nof_remote_faps_with_remote_credit_value.get(unit, &nof_remote_faps_with_remote_credit_value);
    		DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
			if (nof_remote_faps_with_remote_credit_value) { /* is the current value being used (by remote FAPs)? */
				if (credit_worth_remote != arg_local) {
					LOG_ERROR(BSL_LS_SOC_MANAGEMENT,
							  (BSL_META_U(unit,"The Remote credit value is assigned to remote devices. To change the value you must first assign the local credit value to these devices.")));
					DNX_SAND_SET_ERROR_CODE(DNX_SAND_VALUE_OUT_OF_RANGE_ERR, 10, exit);
				} else { /* the local and (previous) remote values are equal, so we can just mark all FAPs as using the local value */
					res = jer2_arad_plus_mgmt_change_all_faps_credit_worth_unsafe(unit, DNX_TMC_FAP_CREDIT_VALUE_LOCAL); /*need to be in sand_error???*/
					DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
					}
				} else {
					DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, READ_IPS_IPS_CREDIT_CONFIG_1r(unit, &reg_val));
					soc_reg_field_set(unit, IPS_IPS_CREDIT_CONFIG_1r, &reg_val, CREDIT_VALUE_2f, credit_worth_remote);
					DNX_SAND_SOC_IF_ERROR_RETURN(res, 1100, exit, WRITE_IPS_IPS_CREDIT_CONFIG_1r(unit, reg_val));
				}
			}
		}
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_credit_worth_remote_set()", unit, 0);
}


/*
 * Arad+ only: set local and remote (1 and 2) credit worth values
 */
uint32
  jer2_arad_plus_mgmt_credit_worth_remote_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT uint32    *credit_worth_remote
  )
{
    uint32 res, reg_val;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
	DNX_SAND_CHECK_NULL_INPUT(credit_worth_remote);
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, READ_IPS_IPS_CREDIT_CONFIG_1r(unit, &reg_val));
	*credit_worth_remote = soc_reg_field_get(unit, IPS_IPS_CREDIT_CONFIG_1r, reg_val, CREDIT_VALUE_2f);
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_credit_worth_remote_get()", unit, 0);
}

/*
 * Arad+ only: Set the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
static uint32
  jer2_arad_plus_mgmt_per_module_credit_value_set(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* will be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  )
{
    soc_error_t rv;
    uint32 offset = fap_id % JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD;
    uint32 per_module_credit_value;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.per_module_credit_value.get(unit,
                                                                                     fap_id / JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD,
                                                                                     &per_module_credit_value);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);
    per_module_credit_value = (per_module_credit_value & ~(1 << offset)) | (credit_value_type << offset); 

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.per_module_credit_value.set(unit,
                                                                                     fap_id / JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD,
                                                                                     per_module_credit_value);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_module_to_credit_worth_map_get_unsafe()", unit, fap_id);
}


/*
 * Arad+ only: map the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_set_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_IN  uint32    credit_value_type /* should be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  )
{
    uint32 prev_type, res;
    uint16 nof_remote_faps_with_remote_credit_value;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    if (!SOC_IS_ARADPLUS(unit)) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 20, exit);
    }
    res = jer2_arad_plus_mgmt_module_to_credit_worth_map_get_unsafe(unit, fap_id, &prev_type);
    DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (credit_value_type != prev_type) {
        res = jer2_arad_plus_mgmt_per_module_credit_value_set(unit, fap_id, credit_value_type);
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.nof_remote_faps_with_remote_credit_value.get(unit, &nof_remote_faps_with_remote_credit_value);
        DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
        if (credit_value_type == DNX_TMC_FAP_CREDIT_VALUE_REMOTE) {
            ++nof_remote_faps_with_remote_credit_value;
        } else if (prev_type == DNX_TMC_FAP_CREDIT_VALUE_REMOTE) {
            --nof_remote_faps_with_remote_credit_value;
        }
        res = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.nof_remote_faps_with_remote_credit_value.set(unit, nof_remote_faps_with_remote_credit_value);
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_module_to_credit_worth_map_set_unsafe()", unit, fap_id);
}

/*
 * Arad+ only: Get the mapping the module (fap_id) to the given credit value (local, remote or non mapped).
 * Has no special handling of the local device (should not be used for the local device).
 */
uint32
  jer2_arad_plus_mgmt_module_to_credit_worth_map_get_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    fap_id,
    DNX_SAND_OUT uint32    *credit_value_type /* will be one of JER2_ARAD_PLUS_FAP_CREDIT_VALUE_* */
  )
{
    soc_error_t rv;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNX_SAND_CHECK_NULL_INPUT(credit_value_type);
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.per_module_credit_value.get(unit,
                                                                                     fap_id / JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD,
                                                                                     credit_value_type);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 130, exit);
    *credit_value_type = (*credit_value_type >> (fap_id % JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD)) & 0x1; 

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_module_to_credit_worth_map_get_unsafe()", unit, fap_id);
}

/*
 * Arad+ only: in case the local and remote credit values are equal, change all configure remote FAPS to use the local or remote value.
 * The credit_value_to_use selects to which value we should make the FAPS use:
 *   JER2_ARAD_PLUS_FAP_CREDIT_VALUE_LOCAL  - use the local credit value
 *   JER2_ARAD_PLUS_FAP_CREDIT_VALUE_REMOTE - use the local credit value
 */
uint32
  jer2_arad_plus_mgmt_change_all_faps_credit_worth_unsafe(
    DNX_SAND_IN  int    unit,
    DNX_SAND_OUT uint8     credit_value_to_use
  )
{
    uint32 reg_val;
    soc_error_t rv;
    uint32 credit_worth_local, credit_worth_remote;
    uint32 per_module_credit_idx;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);
    DNX_SAND_SOC_IF_ERROR_RETURN(rv, 100, exit, READ_IPS_IPS_CREDIT_CONFIG_1r(unit, &reg_val));
    credit_worth_local = soc_reg_field_get(unit, IPS_IPS_CREDIT_CONFIG_1r, reg_val, CREDIT_VALUE_1f);
    credit_worth_remote = soc_reg_field_get(unit, IPS_IPS_CREDIT_CONFIG_1r, reg_val, CREDIT_VALUE_2f);
    if (credit_worth_local != credit_worth_remote) {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 110, exit);
    } else if (credit_value_to_use == DNX_TMC_FAP_CREDIT_VALUE_LOCAL) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.nof_remote_faps_with_remote_credit_value.set(unit, 0);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 100, exit);
    } else if (credit_value_to_use == DNX_TMC_FAP_CREDIT_VALUE_REMOTE) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.nof_remote_faps_with_remote_credit_value.set(unit, JER2_ARAD_NOF_FAPS_IN_SYSTEM);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 100, exit);
    } else {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 120, exit);
    }

    /* change all fap modes to credit_value_to_use */
    for(per_module_credit_idx = 0; per_module_credit_idx < JER2_ARAD_PLUS_CREDIT_VALUE_MODE_WORDS; ++per_module_credit_idx) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_plus.per_module_credit_value.set(unit, per_module_credit_idx, credit_value_to_use ? 255 : 0);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(rv, 130, exit);
    }
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_plus_mgmt_change_all_faps_credit_worth_unsafe()", unit, credit_value_to_use);
}

/*********************************************************************
*     Set the fabric system ID of the device. Must be unique
*     in the system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 sys_fap_id
  )
{
  uint32
    res;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_SYSTEM_FAP_ID_SET_UNSAFE);

  
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_0r, REG_PORT_ANY, 0, PIPEIDf,  sys_fap_id));
  if (SOC_DNX_CONFIG(unit)->tdm.is_bypass &&
      SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.is_128_in_system &&
      SOC_DNX_CONFIG(unit)->tm.is_petrab_in_system) {
          DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_SOURCE_FAP_IDf,  sys_fap_id + SOC_DNX_CONFIG(unit)->jer2_arad->tdm_source_fap_id_offset));
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_system_fap_id_set_unsafe()", unit, sys_fap_id);
}

/*********************************************************************
*     Set the fabric system ID of the device. Must be unique
*     in the system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 sys_fap_id
  )
{
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_SYSTEM_FAP_ID_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    sys_fap_id, JER2_ARAD_NOF_FAPS_IN_SYSTEM-1,
    JER2_ARAD_FAP_FABRIC_ID_OUT_OF_RANGE_ERR, 10, exit
  );


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_system_fap_id_verify()",0,0);
}

/*********************************************************************
*     Set the fabric system ID of the device. Must be unique
*     in the system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_system_fap_id_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32              *sys_fap_id
  )
{
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(sys_fap_id);

  DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, ECI_GLOBAL_0r, REG_PORT_ANY, 0, PIPEIDf, sys_fap_id));

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set the device TM-Domain. Must be unique
*     in a stackable system.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_tm_domain_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tm_domain
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    res;
  
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, TM_DOMAINf,  tm_domain));

  res = jer2_arad_egr_prog_editor_stacking_lfems_set(unit, DNX_TMC_STACK_EGR_PROG_TM_PORT_PROFILE_STACK_ALL, 0x0);
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_tm_domain_set_unsafe()",tm_domain,0);
#endif 
    return -1;
}

uint32
  jer2_arad_mgmt_tm_domain_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tm_domain
  )
{
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_ERR_IF_ABOVE_MAX(tm_domain, JER2_ARAD_NOF_TM_DOMAIN_IN_SYSTEM-1, DNX_SAND_GEN_ERR, 10, exit);


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_tm_domain_verify()",0,0);
}

uint32
  jer2_arad_mgmt_tm_domain_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                 *tm_domain
  )
{
  uint32
    res,
    fld_val;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  DNX_SAND_CHECK_NULL_INPUT(tm_domain);

  

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, TM_DOMAINf, &fld_val));

  /* Get device internal field.                                                                           */
  *tm_domain = fld_val;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_tm_domain_get_unsafe()",0,0);
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 enable
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ENABLE_TRAFFIC_SET);
  DNX_SAND_PCID_LITE_SKIP(unit);
  DNX_SAND_CHECK_DRIVER_AND_DEVICE;

  res = jer2_arad_mgmt_enable_traffic_verify(
    unit,
    enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_TAKE_DEVICE_SEMAPHORE;

  res = jer2_arad_mgmt_enable_traffic_set_unsafe(
    unit,
    enable
  );
  DNX_SAND_CHECK_FUNC_RESULT(res, 100, exit_semaphore);

exit_semaphore:
  DNX_SAND_GIVE_DEVICE_SEMAPHORE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_enable_traffic_set()",0,0);
}


static uint32
  jer2_arad_mgmt_all_ctrl_cells_fct_disable_polling(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 iter_index,
    DNX_SAND_OUT uint8                 *all_down
  )
{
  uint32
    res = DNX_SAND_OK;
  uint64 
      rtp_mask,
      links_up_bm;
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_FCT_DISABLE_POLLING);
  JER2_ARAD_MGMT_TRACE(iter_index);
  
  *all_down = TRUE;
  res = dnx_sand_os_memset(&rtp_mask, 0x0, sizeof(rtp_mask));
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = dnx_sand_os_memset(&links_up_bm, 0x0, sizeof(links_up_bm));
  DNX_SAND_CHECK_FUNC_RESULT(res, 11, exit);

  /*Get link down bitmap*/
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1110, exit, READ_RTP_LINK_ACTIVE_MASKr_REG64(unit, &rtp_mask));

  /*Convert to link up bitmap*/
  COMPILER_64_MASK_CREATE(links_up_bm, SOC_DNX_DEFS_GET(unit, nof_fabric_links), 0);
  COMPILER_64_NOT(rtp_mask);
  COMPILER_64_AND(links_up_bm, rtp_mask);

  if (!COMPILER_64_IS_ZERO(links_up_bm))
  {
    *all_down = FALSE;
    /* Exit with DNX_SAND_OK */
    DNX_SAND_SET_ERROR_CODE(DNX_SAND_OK, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_fct_disable_polling()",0,0);
}

static uint32
  jer2_arad_mgmt_all_ctrl_cells_standalone_polling(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 iter_index,
    DNX_SAND_OUT uint8                 *success
  )
{
  uint32
    res,
    buffer;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  JER2_ARAD_MGMT_TRACE(iter_index);


  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, MESH_TOPOLOGY_FAP_DETECT_CTRL_CELLS_CNTr, REG_PORT_ANY, 0, RCV_CTL_1f, &buffer));

  /*ignore first iteration in order to clear counter*/
  *success = DNX_SAND_NUM2BOOL(buffer) && (iter_index != 0);
  
   /*cfg not stand alone in success case */
  if (*success) {
        res = jer2_arad_fabric_stand_alone_fap_mode_set_unsafe(
          unit,
          FALSE
        );
        DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }


  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mngr_standalone_polling()",0,0);
}

static uint32
  jer2_arad_mgmt_all_ctrl_cells_standalone_failure(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = jer2_arad_fabric_stand_alone_fap_mode_set_unsafe(
          unit,
          TRUE
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_standalone_failure()",0,0);
}

static uint32
  jer2_arad_mgmt_all_ctrl_cells_status_polling(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 iter_index,
    DNX_SAND_OUT uint8                 *success
  )
{
  uint32
    res = DNX_SAND_OK;
  uint32
    /*buffer_1, buffer_2,*/ buffer;
  uint8
    stand_alone;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  JER2_ARAD_MGMT_TRACE(iter_index);
  res = jer2_arad_fabric_stand_alone_fap_mode_get_unsafe(
          unit,
          &stand_alone
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10,  exit, JER2_ARAD_REG_ACCESS_ERR,MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_mesh_check, (unit, stand_alone, success)));

  if (*success == FALSE) {
  
  

      /*recheck stand alone once every JER2_ARAD_MGMT_INIT_STANDALONE_ITERATIONS*/
      if ((iter_index % JER2_ARAD_MGMT_INIT_STANDALONE_ITERATIONS  == 0) && (iter_index != 0)) {
          
          DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, MESH_TOPOLOGY_FAP_DETECT_CTRL_CELLS_CNTr, REG_PORT_ANY, 0, RCV_CTL_1f, &buffer));

          /*cfg stand alone when the counter is zero*/
          if (!DNX_SAND_NUM2BOOL(buffer)) {
                res = jer2_arad_fabric_stand_alone_fap_mode_set_unsafe(
                  unit,
                  TRUE
                );
                DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

                *success = TRUE;

#if JER2_ARAD_MGMT_DBG_ON
                LOG_CLI(
                         (BSL_META_U(unit,
                                     "!!DBG: stand alone mode detected \n")));
#endif
          }

      }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_status_polling()",0,0);
}

static uint32
  jer2_arad_mgmt_all_ctrl_cells_fct_enable_polling(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 iter_index,
    DNX_SAND_OUT uint8                 *any_up
  )
{
  uint32
    res = DNX_SAND_OK;
  uint8
    all_down;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_FCT_ENABLE_POLLING);
  JER2_ARAD_MGMT_TRACE(iter_index);
  res = jer2_arad_mgmt_all_ctrl_cells_fct_disable_polling(
          unit,
          SAL_UINT32_MAX, /*no polling */
          &all_down
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *any_up = DNX_SAND_NUM2BOOL_INVERSE(all_down);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_fct_enable_polling()",0,0);
}

static uint32
  jer2_arad_mgmt_ctrl_cells_counter_clear(
    DNX_SAND_IN  int                  unit
  )
{
  uint32
    res,
    reg_val;
  uint64
    reg_val64;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_CTRL_CELLS_COUNTER_CLEAR);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_MESH_TOPOLOGY_FAP_DETECT_CTRL_CELLS_CNTr(unit, &reg_val));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_MESH_TOPOLOGY_STATUS_2r(unit, &reg_val64));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_ctrl_cells_counter_clear()",0,0);
}

static uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_write(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  JER2_ARAD_CTRL_CELL_DATA      *data,
    DNX_SAND_IN  uint8                  silent
  )
{
  uint32
    res = DNX_SAND_OK;
  uint8
    success = TRUE,
    is_low_sim_active;
  uint32
    wait_iter = 0;
   
    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_WRITE);

  DNX_SAND_CHECK_NULL_INPUT(data);

  

#ifndef SAND_LOW_LEVEL_SIMULATION
  is_low_sim_active = FALSE;
#else
  is_low_sim_active = dnx_sand_low_is_sim_active_get();
#endif

  /* Write the data */
  if (data->reg != INVALIDr)
  {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  25,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, data->reg,  data->instance_id,  0, data->field,  data->val));
  }

  if (data->polling_func)
  {
    /* If there polling field exists, refer to delay as the expected value */
#if JER2_ARAD_MGMT_DBG_ON
  if (data->polling_func)
  {
    LOG_CLI(
             (BSL_META_U(unit,
                         "!!DBG: Polling on ")));
  }
#endif
    do
    {
      if (is_low_sim_active)
      {
        success = data->failure_func ? FALSE : TRUE;
      }
      else
      {
        res = data->polling_func(
                unit,
                wait_iter,
                &success
              );
        DNX_SAND_CHECK_FUNC_RESULT(res, 27, exit);
      }

      if (success)
      {
        /* Exit with DNX_SAND_OK */
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_OK, 30, exit);
      }
      dnx_sand_os_task_delay_milisec(JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_DELAY_MSEC);

    } while ((wait_iter++) < data->delay_or_polling_iters);
  }
  else
  {
    /* If there is no polling field, refer to delay as delay */
    if (data->delay_or_polling_iters)
    {
      dnx_sand_os_task_delay_milisec(data->delay_or_polling_iters);
    }
  }

  if (!success)
  {
    if (data->failure_func)
    {
      res = data->failure_func(
              unit
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }

    if (data->err_on_fail)
    {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_ERR, 50, exit);
    }
  }

exit:
#if JER2_ARAD_MGMT_DBG_ON
  if (data->polling_func)
  {
    LOG_CLI(
             (BSL_META_U(unit,
                         "!!DBG: Polling Time: %u[ms], Success: %s\n\r"),
              wait_iter*JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_DELAY_MSEC,
              success?"TRUE":"FALSE"
              ));
    if (wait_iter >= data->delay_or_polling_iters)
    {
      LOG_CLI(
               (BSL_META_U(unit,
                           "!!DBG: Exceeded maximal polling time %u[ms] (%u * %u iterations)\n\r"),
                data->delay_or_polling_iters*JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_DELAY_MSEC,
                data->delay_or_polling_iters,
                JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_DELAY_MSEC
                ));
    }
  }
#endif
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_enable_write()",0,0);
}

uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_get_unsafe(
    DNX_SAND_IN   int  unit,
    DNX_SAND_OUT  uint8  *enable
  )
{
  uint32
    res,
    fld_val;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_GET_UNSAFE);

  /*
   *  Just read one register as a representative
   */
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, CONFIG_6f, &fld_val));
  *enable = DNX_SAND_NUM2BOOL(fld_val);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_enable_get_unsafe()",0,0);
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable,
    DNX_SAND_IN  uint32  flags

  )
{
  uint32
    res = DNX_SAND_OK;
  JER2_ARAD_CTRL_CELL_DATA
    *conf = NULL;
  uint32
    conf_idx = 0,
    nof_confs = 0,
    write_idx = 0,
    inst_idx = 0;
   

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_SET_UNSAFE);
  
  JER2_ARAD_ALLOC(conf, JER2_ARAD_CTRL_CELL_DATA, JER2_ARAD_MGMT_CTRL_CELLS_MAX_NOF_CONFS, "jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe.conf");
  res = dnx_sand_os_memset(
          conf,
          0x0,
          JER2_ARAD_MGMT_CTRL_CELLS_MAX_NOF_CONFS * sizeof(JER2_ARAD_CTRL_CELL_DATA)
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  for (inst_idx = 0; inst_idx < SOC_DNX_DEFS_GET(unit, nof_instances_fmac); inst_idx++)
  {
    conf[conf_idx].reg   = (enable == TRUE ? FMAC_LEAKY_BUCKET_CONTROL_REGISTERr:INVALIDr);
    conf[conf_idx].field = (enable == TRUE ? BKT_FILL_RATEf:INVALIDr);
    conf[conf_idx].val = dnx_sand_link_fap_bkt_fill_rate_get();
    conf[conf_idx].instance_id = inst_idx;
    conf[conf_idx].polling_func = NULL;
    conf[conf_idx].failure_func = NULL;
    conf[conf_idx].err_on_fail = FALSE;
    conf[conf_idx++].delay_or_polling_iters = 0;

    conf[conf_idx].reg = (enable == TRUE ? FMAC_LEAKY_BUCKET_CONTROL_REGISTERr:INVALIDr);
    conf[conf_idx].field = (enable == TRUE ? BKT_LINK_DN_THf:INVALIDr);
    conf[conf_idx].val = dnx_sand_link_fap_dn_link_th_get();
    conf[conf_idx].instance_id = inst_idx;
    conf[conf_idx].polling_func = NULL;
    conf[conf_idx].failure_func = NULL;
    conf[conf_idx].err_on_fail = FALSE;
    conf[conf_idx++].delay_or_polling_iters = 0;

    conf[conf_idx].reg = (enable == TRUE ? FMAC_LEAKY_BUCKET_CONTROL_REGISTERr:INVALIDr);
    conf[conf_idx].field = (enable == TRUE ? BKT_LINK_UP_THf:INVALIDr);
    conf[conf_idx].val = dnx_sand_link_fap_up_link_th_get();
    conf[conf_idx].instance_id = inst_idx;
    conf[conf_idx].polling_func = NULL;
    conf[conf_idx].failure_func = NULL;
    conf[conf_idx].err_on_fail = FALSE;
    conf[conf_idx++].delay_or_polling_iters = 0;
  }

  conf[conf_idx].reg = RTP_RTP_ENABLEr;
  conf[conf_idx].field = RMGRf;
  conf[conf_idx].val = (enable == TRUE ? dnx_sand_link_fap_reachability_rate_get(jer2_arad_chip_ticks_per_sec_get(unit), SOC_DNX_DEFS_GET(unit, fabric_rmgr_nof_links), SOC_DNX_DEFS_GET(unit, fabric_rmgr_units)) : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = RTP_RTP_ENABLEr;
  conf[conf_idx].field = RTPWPf;
  conf[conf_idx].val = (enable == TRUE ? dnx_sand_link_fap_reachability_watchdog_period_get(jer2_arad_chip_ticks_per_sec_get(unit)) : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = RTP_RTP_ENABLEr;
  conf[conf_idx].field = RTP_EN_MSKf;
  conf[conf_idx].val = (enable == TRUE ? 0x1 : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = RTP_RTP_ENABLEr;
  conf[conf_idx].field = RTP_UP_ENf;
  conf[conf_idx].val = (enable == TRUE ? 0x1 : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCT_FCT_ENABLER_REGISTERr;
  conf[conf_idx].field = DIS_STSf;
  conf[conf_idx].val = 0x1;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE; /* Do not throw error in case polling failed */
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCT_FCT_ENABLER_REGISTERr;
  conf[conf_idx].field = DIS_CRDf;
  conf[conf_idx].val = 0x1;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE; /* Do not throw error in case polling failed */
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCT_FCT_ENABLER_REGISTERr;
  conf[conf_idx].field = DIS_RCHf;
  conf[conf_idx].val = (enable == TRUE ? 0x0 : 0x1);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE; /* Do not throw error in case polling failed */
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr;
  conf[conf_idx].field = TRAP_ALL_CNTf;
  conf[conf_idx].val = (enable == TRUE ? 0x0 : 0x1);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_fct_enable_polling : NULL);
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE; /* Do not throw error in case polling failed */
  conf[conf_idx++].delay_or_polling_iters = (enable == TRUE ? JER2_ARAD_MGMT_INIT_CTRL_RCH_STATUS_ITERATIONS : 0);

  
  for (inst_idx = 0; inst_idx < SOC_DNX_DEFS_GET(unit, nof_instances_fmac); inst_idx++)
  {
      if ( !SOC_IS_JERICHO(unit) || (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode != DNX_TMC_FABRIC_CONNECT_MODE_MESH) ) 
      {
          conf[conf_idx].reg = FMAC_LINK_TOPO_MODE_REG_0r;
          conf[conf_idx].field = LINK_TOPO_MODE_0f;
          conf[conf_idx].val = (enable == TRUE ? 0xf : 0x0);
          conf[conf_idx].instance_id = inst_idx;
          conf[conf_idx].polling_func = NULL;
          conf[conf_idx].failure_func = NULL;
          conf[conf_idx].err_on_fail = FALSE;
          conf[conf_idx++].delay_or_polling_iters = 0;
      }

      conf[conf_idx].reg = FMAC_LINK_TOPO_MODE_REG_2r;
      conf[conf_idx].field = LINK_TOPO_MODE_2f;
      conf[conf_idx].val = (enable == TRUE ? 0xf : 0x0);
      conf[conf_idx].instance_id = inst_idx;
      conf[conf_idx].polling_func = NULL;
      conf[conf_idx].failure_func = NULL;
      conf[conf_idx].err_on_fail = FALSE;
      conf[conf_idx++].delay_or_polling_iters = 0;
  
  }
  
  conf[conf_idx].reg = MESH_TOPOLOGY_THRESHOLDSr;
  conf[conf_idx].field = THRESHOLD_2f;
  conf[conf_idx].val = (enable == TRUE ? 0x14 : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = (flags & JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET) == 0 ? MESH_TOPOLOGY_MESH_TOPOLOGYr : INVALIDr; /*Do not config as part of soft reset sequance*/
  conf[conf_idx].field = (flags & JER2_ARAD_MGMT_ALL_CTRL_CELLS_FLAGS_SOFT_RESET) == 0 ? CONFIG_6f : INVALIDf; /*Do not config as part of soft reset sequance*/
  conf[conf_idx].val = (enable == TRUE ? 0x1 : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = MESH_TOPOLOGY_INITr;
  conf[conf_idx].field = INITf;
  conf[conf_idx].val = (enable == TRUE ? 0xd : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_standalone_polling : NULL);
  conf[conf_idx].failure_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_standalone_failure : NULL);
  conf[conf_idx].err_on_fail = FALSE; /* Do not throw error in case polling failed */
  conf[conf_idx++].delay_or_polling_iters = (enable == TRUE ? JER2_ARAD_MGMT_INIT_STANDALONE_ITERATIONS : 0);

  conf[conf_idx].reg = INVALIDr;
  conf[conf_idx].field = INVALIDr;
  conf[conf_idx].val = 0;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_status_polling : NULL);
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = TRUE; /* Throw error in case polling failed */
  conf[conf_idx++].delay_or_polling_iters = (enable == TRUE ? JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_ITERATIONS : 0);

  conf[conf_idx].reg = MESH_TOPOLOGY_MESH_TOPOLOGYr;
  conf[conf_idx].field = CONFIG_5f;
  conf[conf_idx].val = (enable == TRUE ? 0x7 : 0x0);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCR_FCR_ENABLERS_AND_FILTER_MATCH_INPUT_LINKr;
  conf[conf_idx].field = TRAP_ALL_CNTf;
  conf[conf_idx].val = (enable == TRUE)?0x0 : 0x1;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = (enable == TRUE ? NULL : jer2_arad_mgmt_all_ctrl_cells_fct_disable_polling);
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = (enable == TRUE ? 0 : JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_ITERATIONS);

  conf[conf_idx].reg = FCT_FCT_ENABLER_REGISTERr;
  conf[conf_idx].field = DIS_STSf;
  conf[conf_idx].val = (enable == TRUE)?0x0 : 0x1;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCT_FCT_ENABLER_REGISTERr;
  conf[conf_idx].field = DIS_CRDf;
  conf[conf_idx].val = (enable == TRUE)?0x0 : 0x1;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  conf[conf_idx].reg = FCT_FCT_ENABLER_REGISTERr;
  conf[conf_idx].field = DIS_RCHf;
  conf[conf_idx].val = (enable == TRUE)?0x0 : 0x1;
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  for (inst_idx = 0; inst_idx < SOC_DNX_DEFS_GET(unit, nof_instances_fmac); inst_idx++)
  {
    conf[conf_idx].reg = FMAC_GENERAL_CONFIGURATION_REGISTERr;
    conf[conf_idx].field = ENABLE_SERIAL_LINKf;
    conf[conf_idx].val = (enable == TRUE ? 0x0 : 0x1);
    conf[conf_idx].instance_id = inst_idx;
    conf[conf_idx].polling_func = NULL;
    conf[conf_idx].failure_func = NULL;
    conf[conf_idx].err_on_fail = FALSE;
    conf[conf_idx++].delay_or_polling_iters = (((enable == TRUE) && (inst_idx == (SOC_DNX_DEFS_GET(unit, nof_instances_fmac) - 1))) || ((enable == FALSE) && (inst_idx == 0)) ? 16 : 0);
  }

  conf[conf_idx].reg = FDT_FDT_ENABLER_REGISTERr;
  conf[conf_idx].field = DISCARD_DLL_PKTSf;
  conf[conf_idx].val = (enable == TRUE ? 0x0 : 0x1);
  conf[conf_idx].instance_id = REG_PORT_ANY;
  conf[conf_idx].polling_func = NULL;
  conf[conf_idx].failure_func = NULL;
  conf[conf_idx].err_on_fail = FALSE;
  conf[conf_idx++].delay_or_polling_iters = 0;

  if (SOC_IS_JERICHO(unit)) {
      conf[conf_idx].reg = MESH_TOPOLOGY_REG_0117r; 
      conf[conf_idx].field = FIELD_0_2f;
      conf[conf_idx].val = (enable == TRUE ? 0x5 : 0x0);
      conf[conf_idx].instance_id = REG_PORT_ANY;
      conf[conf_idx].polling_func = NULL;
      conf[conf_idx].failure_func = NULL;
      conf[conf_idx].err_on_fail = FALSE;
      conf[conf_idx++].delay_or_polling_iters = 0;
  }
  nof_confs = conf_idx;

  res = jer2_arad_mgmt_ctrl_cells_counter_clear(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

#if JER2_ARAD_MGMT_DBG_ON
  LOG_CLI(
           (BSL_META_U(unit,
                       "\n\r!!DBG ON, print write-accesses (Addr,Mask,Value) and function calls\n\r")));
  if (enable){dnx_sand_set_print_when_writing(1, 1, 0);}
#endif
  for (conf_idx = 0; conf_idx < nof_confs; ++conf_idx)
  {
    write_idx = (enable == TRUE ? conf_idx : nof_confs - conf_idx - 1);

    res = jer2_arad_mgmt_all_ctrl_cells_enable_write(
            unit,
            &(conf[write_idx]),
            TRUE
         );
    DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }

#if JER2_ARAD_MGMT_DBG_ON
  if (enable){dnx_sand_set_print_when_writing(0, 0, 0);}
#endif

exit:
  JER2_ARAD_FREE(conf);
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_enable_set_unsafe()",conf_idx,DNX_SAND_BOOL2NUM(enable));
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_all_ctrl_cells_enable_verify(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ALL_CTRL_CELLS_ENABLE_VERIFY);

  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_all_ctrl_cells_enable_verify()",0,0);
}

/*********************************************************************
*     Enable / Disable the forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_set_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  int     enable
  )
{
    uint32 res;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 110, exit, JER2_ARAD_REG_ACCESS_ERR,
      soc_reg_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, DIS_LCLRTf, enable ? 1 : 0));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_force_tdm_bypass_traffic_to_fabric_set_unsafe()", enable, 0);
}
/*********************************************************************
*     Check if forcing of (bypass) TDM cells to fabric
*********************************************************************/
uint32
  jer2_arad_force_tdm_bypass_traffic_to_fabric_get_unsafe(
    DNX_SAND_IN  int     unit,
    DNX_SAND_OUT int     *enable
  )
{
    uint32 res, reg_val;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 110, exit, JER2_ARAD_REG_ACCESS_ERR,
      READ_FDT_FDT_ENABLER_REGISTERr_REG32(unit, &reg_val));
    *enable = (int)soc_reg_field_get(unit, FDT_FDT_ENABLER_REGISTERr, reg_val, DIS_LCLRTf);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_force_tdm_bypass_traffic_to_fabric_get_unsafe()", 0, 0);
}


/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 jer2_arad_mgmt_enable_traffic_set_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable)
{
    uint32
        res,
        enable_val,
        disable_val;
    uint64 
        reg_val_64,
        fld64_val;
    JER2_ARAD_CTRL_CELL_DATA
        *conf = NULL;
    uint32
        conf_idx = 0,
        write_idx = 0,
        nof_confs = 0,
        reg32_val;
    soc_reg_above_64_val_t 
        reg_above_64_val;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ENABLE_TRAFFIC_SET_UNSAFE);

    enable_val  = DNX_SAND_BOOL2NUM(enable);
    disable_val = DNX_SAND_BOOL2NUM_INVERSE(enable);
    COMPILER_64_ZERO(reg_val_64);
    /*
    *  Note: the comments are for disabling the traffic.
    *  For enabling the traffic, the action is the opposite of the described
    */

    /*
    * cfg MESH_TOPOLOGY status check 
    */
    if (enable) {
        JER2_ARAD_ALLOC(conf, JER2_ARAD_CTRL_CELL_DATA, JER2_ARAD_MGMT_CTRL_CELLS_MAX_NOF_CONFS, "jer2_arad_mgmt_enable_traffic_set_unsafe.conf");
        conf[conf_idx].reg = INVALIDr;
        conf[conf_idx].field = INVALIDr;
        conf[conf_idx].val = 0;
        conf[conf_idx].instance_id = REG_PORT_ANY;
        conf[conf_idx].polling_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_standalone_polling : NULL);
        conf[conf_idx].failure_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_standalone_failure : NULL);
        conf[conf_idx].err_on_fail = FALSE; /* Do not throw error in case polling failed */
        conf[conf_idx++].delay_or_polling_iters = (enable == TRUE ? JER2_ARAD_MGMT_INIT_STANDALONE_ITERATIONS : 0);

        conf[conf_idx].reg = INVALIDr;
        conf[conf_idx].field = INVALIDr;
        conf[conf_idx].val = 0;
        conf[conf_idx].instance_id = REG_PORT_ANY;
        conf[conf_idx].polling_func = (enable == TRUE ? jer2_arad_mgmt_all_ctrl_cells_status_polling : NULL);
        conf[conf_idx].failure_func = NULL;
        conf[conf_idx].err_on_fail = TRUE; /* Throw error in case polling failed */
        conf[conf_idx++].delay_or_polling_iters = (enable == TRUE ? JER2_ARAD_MGMT_INIT_CTRL_CELLS_TIMER_ITERATIONS : 0);

        nof_confs = conf_idx;


        for (conf_idx = 0; conf_idx < nof_confs; ++conf_idx) {
            write_idx = (enable == TRUE ? conf_idx : nof_confs - conf_idx - 1);

            res = jer2_arad_mgmt_all_ctrl_cells_enable_write(unit, &(conf[write_idx]), TRUE);
            DNX_SAND_CHECK_FUNC_RESULT(res, 100 + write_idx, exit);
        }
    }
    /*
    *  Disable data path at the ingress -
    *  I.e., if disabled - no packet will enter the ingress DRAM and fabric.
    *  IDR, IRR, IRE
    */
    if (enable == FALSE) {

        if (SOC_IS_ARDON(unit)) {
            reg32_val = 0;
            /* Disable traffic from NPU and signal NPU traffic is disabled */
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_INGRESS_DATA_PATH_READYf, 0x1);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_EGRESS_DATA_PATH_DRAINf, 0x1);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_EGRESS_DATA_PATH_FLOW_CONTROLf, 0x0);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ATMF_DATA_PATH_STATUSf, 0x0);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_EGRESS_DATA_PATH_MASKf, 0x0);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg32_set(unit, NBI_ENABLE_DATA_PATHr, REG_PORT_ANY, 0, reg32_val));            
        }

        /*
        *  Stop traffic from the NIFs
        */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRE_STATIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATHf,  enable_val));
        /*
        * Stop Data traffic from the fabric.
        */
         SOC_REG_ABOVE_64_ALLONES(reg_above_64_val);
         DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg_above_64_set(unit, FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_0r, REG_PORT_ANY, 0, reg_above_64_val));
         COMPILER_64_ALLONES(reg_val_64);
         DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  12,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_1r(unit,  reg_val_64));
         
         COMPILER_64_ZERO(reg_val_64);
         COMPILER_64_SET(fld64_val,0,0x2);
         JER2_ARAD_FLD_TO_REG64(FDR_FDR_ENABLERS_REGISTER_1r, FDR_MTCH_ACTf, fld64_val, reg_val_64, 14, exit);
         DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  14,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit,  reg_val_64)); 

         /* Stop Traffic from FDR */
         DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  16,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_FDR_ENABLERS_REGISTER_1r, REG_PORT_ANY, 0, FIELD_31_31f, 0x1));
         DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  18,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_FDR_ENABLERS_REGISTER_1r, REG_PORT_ANY, 0, FIELD_32_32f, 0x1));
      }
  
      /*
      *  Disable internal data-path - just in case
      */
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IDR_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATHf,  enable_val));
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  22,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRR_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATH_IDRf,  enable_val));
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  24,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRR_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATH_IQMf,  enable_val));
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  26,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_IHP_ENABLERSr, SOC_CORE_ALL, 0, ENABLE_DATA_PATHf,  enable_val));
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
     if (SOC_IS_JERICHO(unit) && SOC_DNX_CONFIG(unit)->jer2_arad->init.ocb.ocb_enable == OCB_ENABLED) {
         res = soc_reg_above_64_field32_modify(unit, OCB_GENERAL_CONFIGr, REG_PORT_ANY, 0, ENABLE_IDR_TRAFFICf, enable_val);
         DNX_SAND_CHECK_FUNC_RESULT(res, 28, exit);
     }
#endif 
    /*
    *  Stop credit reception from the fabric
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, SOC_CORE_ALL, 0, DISABLE_FABRIC_MSGSf,  disable_val));

    /*
    *  Stop credit generation
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_DVS_CONFIG_1r, REG_PORT_ANY, 0, FORCE_PAUSEf,  disable_val));


    /*
    *  Stop dequeue from all queues
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, SOC_CORE_ALL, 0, DIS_DEQ_CMDSf,  disable_val));

    /*
    *  Discard all credits in the IPS
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  80,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, SOC_CORE_ALL, 0, DISCARD_ALL_CRDTf,  disable_val));

    /*
    *  Discard all packets currently in the queues
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  90,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IQM_IQM_ENABLERSr, SOC_CORE_ALL, 0, DSCRD_ALL_PKTf,  disable_val));

    if (enable == TRUE) {
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  100,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRE_STATIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATHf,  enable_val));

        if (SOC_IS_ARDON(unit)) {
            /* Enable traffic from NPU and signal NPU traffic is enabled*/
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_INGRESS_DATA_PATH_READYf, 0x1);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_EGRESS_DATA_PATH_DRAINf, 0x0);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_EGRESS_DATA_PATH_FLOW_CONTROLf, 0x0);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ATMF_DATA_PATH_STATUSf, 0x1);
            soc_reg_field_set(unit, NBI_ENABLE_DATA_PATHr, &reg32_val, ENABLE_EGRESS_DATA_PATH_MASKf, 0x0);
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  105,  exit, JER2_ARAD_REG_ACCESS_ERR, soc_reg32_set(unit, NBI_ENABLE_DATA_PATHr, REG_PORT_ANY, 0, reg32_val));
        }
        /*
        * Set the device to receive from the fabric.
        * Note: may be overridden later by "fixes_apply" API.
        */
        COMPILER_64_SET(reg_val_64, JER2_ARAD_MGMT_FDR_TRFC_ENABLE_VAR_CELL_MSB, JER2_ARAD_MGMT_FDR_TRFC_ENABLE_VAR_CELL_LSB);
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit,  reg_val_64));
        /*
         * Reconfigure ALDWP
         */
        res = jer2_arad_fabric_aldwp_config(unit);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 120, exit);

        SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
        DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg_above_64_set(unit, FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_0r, REG_PORT_ANY, 0, reg_above_64_val));
        COMPILER_64_ZERO(reg_val_64);
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_FDR_PROGRAMMABLE_DATA_CELL_COUNTER_MASK_1r(unit,  reg_val_64)); 
    }

exit:
    JER2_ARAD_FREE(conf);
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_enable_traffic_set_unsafe()",0,0);
}


/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_enable_traffic_verify(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint8  enable
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ENABLE_TRAFFIC_VERIFY);

  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_enable_traffic_verify()",0,0);
}

/*********************************************************************
*     Enable / Disable the device from receiving and
*     transmitting traffic.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 jer2_arad_mgmt_enable_traffic_get_unsafe(
    DNX_SAND_IN  int  unit,
    DNX_SAND_OUT uint8  *enable)
{
    uint32
        res,
        enable_val;
    uint8
        enable_curr = FALSE,
        enable_all = TRUE;
    uint64
        enable_val64;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_ENABLE_TRAFFIC_GET_UNSAFE);

    DNX_SAND_CHECK_NULL_INPUT(enable);

    /*
    *  Check the ingress data path at the ingress -
    *  I.e., if disabled - no packet will enter the ingress DRAM and fabric.
    *  IDR, IRR, IRE
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IDR_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATHf, &enable_val));
    enable_curr = DNX_SAND_NUM2BOOL(enable_val);
    enable_all = (enable_all && enable_curr);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IRR_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATH_IDRf, &enable_val));
    enable_curr = DNX_SAND_NUM2BOOL(enable_val);
    enable_all = (enable_all && enable_curr);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IRR_DYNAMIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATH_IQMf, &enable_val));
    enable_curr = DNX_SAND_NUM2BOOL(enable_val);
    enable_all = (enable_all && enable_curr);

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IRE_STATIC_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_DATA_PATHf, &enable_val));
    enable_curr = DNX_SAND_NUM2BOOL(enable_val);
    enable_all = (enable_all && enable_curr);

    /*
    * Check fabric receive.
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field64_read(unit, FDR_FDR_ENABLERS_REGISTER_1r, REG_PORT_ANY, 0, FDR_MTCH_ACTf, &enable_val64));
    enable_curr = DNX_SAND_NUM2BOOL_INVERSE(COMPILER_64_IS_ZERO(enable_val64));
    enable_all = (enable_all && enable_curr);


    /*
    * Check SMP messages are enabled (scheduler).
    */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_SCHEDULER_CONFIGURATION_REGISTERr, SOC_CORE_ALL, 0, DISABLE_FABRIC_MSGSf, &enable_val));
    enable_curr = !(DNX_SAND_NUM2BOOL(enable_val));
    enable_all = (enable_all && enable_curr);

    *enable = enable_all;

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_enable_traffic_get_unsafe()",0,0);
}

/*********************************************************************
*     Set the maximal allowed packet size. The limitation can
 *     be performed based on the packet size before or after
 *     the ingress editing (external and internal configuration
 *     mode, accordingly). Packets above the specified value
 *     are dropped.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  uint32                       max_size
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    max_size_lcl,
    res = DNX_SAND_OK;
  JER2_ARAD_IDR_CONTEXT_MRU_TBL_DATA
    tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET_UNSAFE);

  res = jer2_arad_idr_context_mru_tbl_get_unsafe(
          unit,
          port_ndx,
          &tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  switch(conf_mode_ndx) {
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN:
    max_size_lcl = (max_size)-JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL;
    tbl_data.size = max_size_lcl;
    break;

  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN:
    max_size_lcl = (max_size)-JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL;
    tbl_data.org_size = max_size_lcl;
    break;

  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 10, exit);
  }

  res = jer2_arad_idr_context_mru_tbl_set_unsafe(
          unit,
          port_ndx,
          &tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_max_pckt_size_set_unsafe()", port_ndx, 0);
#endif 
    return -1;
}

uint32
  jer2_arad_mgmt_max_pckt_size_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  uint32                       max_size
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MAX_PCKT_SIZE_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_PORT_NDX_MAX, JER2_ARAD_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(conf_mode_ndx, JER2_ARAD_CONF_MODE_NDX_MAX, JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 20, exit);
  switch(conf_mode_ndx) {
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN:
    DNX_SAND_ERR_IF_ABOVE_MAX(max_size, JER2_ARAD_MGMT_PCKT_MAX_SIZE_INTERNAL_MAX, JER2_ARAD_MGMT_PCKT_MAX_SIZE_INTERNAL_OUT_OF_RANGE_ERROR, 30, exit);
    break;
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN:
        DNX_SAND_ERR_IF_ABOVE_MAX(max_size, JER2_ARAD_MGMT_PCKT_MAX_SIZE_EXTERNAL_MAX, JER2_ARAD_MGMT_PCKT_MAX_SIZE_EXTERNAL_OUT_OF_RANGE_ERROR, 40, exit);
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 50, exit);
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_max_pckt_size_set_verify()", port_ndx, 0);
#endif 
    return -1;
}

uint32
  jer2_arad_mgmt_max_pckt_size_get_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(port_ndx, JER2_ARAD_PORT_NDX_MAX, JER2_ARAD_PORT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  DNX_SAND_ERR_IF_ABOVE_MAX(conf_mode_ndx, JER2_ARAD_CONF_MODE_NDX_MAX, JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 20, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_max_pckt_size_get_verify()", port_ndx, 0);
#endif 
    return -1;
}

/*********************************************************************
*     Set the maximal allowed packet size. The limitation can
 *     be performed based on the packet size before or after
 *     the ingress editing (external and internal configuration
 *     mode, accordingly). Packets above the specified value
 *     are dropped.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_max_pckt_size_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      port_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_OUT uint32                       *max_size
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    max_size_lcl,
    res = DNX_SAND_OK;
  JER2_ARAD_IDR_CONTEXT_MRU_TBL_DATA
    tbl_data;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_MAX_PCKT_SIZE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(max_size);

  res = jer2_arad_idr_context_mru_tbl_get_unsafe(
          unit,
          port_ndx,
          &tbl_data
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  switch(conf_mode_ndx) {
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN:
    max_size_lcl = tbl_data.size;
    *max_size = max_size_lcl + JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL;
    break;

  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN:
    max_size_lcl = tbl_data.org_size;
    *max_size = max_size_lcl + JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL;
    break;

  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 70, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_max_pckt_size_get_unsafe()", port_ndx, 0);
#endif 
    return -1;
}


/*********************************************************************
 * Set the MTU (maximal allowed packet size) for any packet,
 * according to the buffer size.
 *********************************************************************/
uint32
  jer2_arad_mgmt_set_mru_by_dbuff_size(
    DNX_SAND_IN  int     unit
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    uint32 mru = SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.dbuff_size * JER2_ARAD_MGMT_MAX_BUFFERS_PER_PACKET;
    uint32 entry = 0;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    if (mru > JER2_ARAD_MGMT_PCKT_SIZE_BYTES_VSC_MAX) {
         mru = JER2_ARAD_MGMT_PCKT_SIZE_BYTES_VSC_MAX;
    }
    soc_IDR_CONTEXT_MRUm_field32_set(unit, &entry, MAX_ORG_SIZEf, mru + 1);
    soc_IDR_CONTEXT_MRUm_field32_set(unit, &entry, MAX_SIZEf, mru - 1);
    DNX_SAND_CHECK_FUNC_RESULT( jer2_arad_fill_table_with_entry(unit, IDR_CONTEXT_MRUm, MEM_BLOCK_ANY, &entry), 100, exit);
exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_set_mru_by_dbuff_size()", mru, entry);
#endif 
    return -1;
}

uint32
  jer2_arad_mgmt_pckt_size_range_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE          *size_range
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    max_size,
    res;
   
    
  uint32
    port_ndx;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_PCKT_SIZE_RANGE_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(size_range);

  

  /*
   *    Minimum size configuration
   */
  switch(conf_mode_ndx) {
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN:
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IDR_PACKET_SIZESr, REG_PORT_ANY, 0, MIN_PACKET_SIZEf,  (size_range->min)-JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_MAXIMUM_AND_MINIMUM_PACKET_SIZEr, SOC_CORE_ALL, 0, FABRIC_MIN_PKT_SIZEf,  size_range->min));
    break;
 
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN:
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IDR_PACKET_SIZESr, REG_PORT_ANY, 0, MIN_ORG_PACKET_SIZEf,  (size_range->min)-JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL));
    break;

  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 40, exit);
  }

  /*
   *    Maximum size configuration
   */

  max_size = size_range->max;

  for (port_ndx = 0; port_ndx < JER2_ARAD_NOF_FAP_PORTS; ++port_ndx)
  {
    res = jer2_arad_mgmt_max_pckt_size_set_unsafe(
            unit,
            port_ndx,
            conf_mode_ndx,
            max_size
          );
    DNX_SAND_CHECK_FUNC_RESULT(res, 60, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_pckt_size_range_set_unsafe()",0,0);
#endif 
    return -1;
}

uint32
  jer2_arad_mgmt_pckt_size_range_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE conf_mode_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_PCKT_SIZE          *size_range
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    port_ndx,
    max_size,
    idr_min,
    egq_min,
    fld_val,
    res;
   
    


  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_PCKT_SIZE_RANGE_GET_UNSAFE);
  DNX_SAND_CHECK_NULL_INPUT(size_range);

  

  jer2_arad_JER2_ARAD_MGMT_PCKT_SIZE_clear(size_range);

  switch(conf_mode_ndx) {
  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_INTERN:
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IDR_PACKET_SIZESr, REG_PORT_ANY, 0, MIN_PACKET_SIZEf, &idr_min));
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, EGQ_MAXIMUM_AND_MINIMUM_PACKET_SIZEr, SOC_CORE_ALL, 0, FABRIC_MIN_PKT_SIZEf, &egq_min));

    if ((idr_min + JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL) != egq_min)
    {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MIN_PCKT_SIZE_INCONSISTENT_ERR, 30, exit);
    }

    size_range->min = idr_min + JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_INTERNAL;
    break;

  case JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_EXTERN:

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IDR_PACKET_SIZESr, REG_PORT_ANY, 0, MIN_ORG_PACKET_SIZEf, &fld_val));
    if (fld_val == 0x0)
    {
      size_range->min = JER2_ARAD_MGMT_PCKT_SIZE_EXTERN_NO_LIMIT;
    }
    else
    {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IDR_PACKET_SIZESr, REG_PORT_ANY, 0, MIN_ORG_PACKET_SIZEf, &idr_min));
       size_range->min = idr_min + JER2_ARAD_MGMT_PCKT_RNG_CORRECTION_EXTERNAL;
    }
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MGMT_PCKT_SIZE_CONF_MODE_OUT_OF_RANGE_ERR, 60, exit);
  }

  /*
   *    Get the max size from port 0
   */
  port_ndx = 0;
  res = jer2_arad_mgmt_max_pckt_size_get_unsafe(
          unit,
          port_ndx,
          conf_mode_ndx,
          &max_size
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 70, exit);

  size_range->max = max_size;
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mgmt_pckt_size_range_get_unsafe()",0,0);
#endif 
    return -1;
}

/* 
 * To disable the Multicast range, it is the user's responsibility to set both Multicast min & max to -1.
 * In case of -1, the Driver configures the min value to 64K-1 and the max value to 0.
 */
uint32
  jer2_arad_mgmt_ocb_mc_range_set_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    res,
    reg_val;
  const static soc_reg_t
    idr_ocb_multicast_range[] = {IDR_OCB_MULTICAST_RANGE_0r, IDR_OCB_MULTICAST_RANGE_1r};
  const static soc_field_t
    ocb_multicast_range_low[] = {OCB_MULTICAST_RANGE_0_LOWf, OCB_MULTICAST_RANGE_1_LOWf},
    ocb_multicast_range_high[] = {OCB_MULTICAST_RANGE_0_HIGHf, OCB_MULTICAST_RANGE_1_HIGHf};
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_MC_RANGE_SET_UNSAFE);

  if (SOC_IS_JERICHO(unit)) 
  {
      uint64 reg_64;
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg64_get(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, REG_PORT_ANY, range_ndx, &reg_64)); 
      soc_reg64_field32_set(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, &reg_64, OCB_ELIGIBLE_MULTICAST_RANGE_N_LOWf, range->min);
      soc_reg64_field32_set(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, &reg_64, OCB_ELIGIBLE_MULTICAST_RANGE_N_HIGHf, range->max);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg64_set(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, REG_PORT_ANY, range_ndx, reg_64));
  } else 
  {
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, soc_reg32_get(unit, idr_ocb_multicast_range[range_ndx], REG_PORT_ANY, 0, &reg_val)); 
      soc_reg_field_set(unit, idr_ocb_multicast_range[range_ndx], &reg_val, ocb_multicast_range_low[range_ndx], range->min);
      soc_reg_field_set(unit, idr_ocb_multicast_range[range_ndx], &reg_val, ocb_multicast_range_high[range_ndx], range->max);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_reg32_set(unit, idr_ocb_multicast_range[range_ndx], REG_PORT_ANY, 0, reg_val));
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_mc_range_set_unsafe()", 0, 0);
#endif 
    return -1;
}

/*
 * In case min value is 64K-1 or max value is 0, the Driver returns value '-1'. 
 */ 
uint32
  jer2_arad_mgmt_ocb_mc_range_get_unsafe(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  uint32                      range_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_MC_RANGE         *range
  )
{
  
  DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
  uint32
    res,
    reg_val;
  const static soc_reg_t
    idr_ocb_multicast_range[] = {IDR_OCB_MULTICAST_RANGE_0r, IDR_OCB_MULTICAST_RANGE_1r};
  const static soc_field_t
    ocb_multicast_range_low[] = {OCB_MULTICAST_RANGE_0_LOWf, OCB_MULTICAST_RANGE_1_LOWf},
    ocb_multicast_range_high[] = {OCB_MULTICAST_RANGE_0_HIGHf, OCB_MULTICAST_RANGE_1_HIGHf};
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_MC_RANGE_GET_UNSAFE);
  
  if(SOC_IS_JERICHO(unit))
  {
      uint64 reg_64;
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg64_get(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, REG_PORT_ANY, range_ndx, &reg_64)); 
      range->min = soc_reg64_field32_get(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, reg_64, OCB_ELIGIBLE_MULTICAST_RANGE_N_LOWf );
      range->max = soc_reg64_field32_get(unit, IDR_OCB_ELIGIBLE_MULTICAST_RANGEr, reg_64, OCB_ELIGIBLE_MULTICAST_RANGE_N_HIGHf );
  } else
  {
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg32_get(unit, idr_ocb_multicast_range[range_ndx], REG_PORT_ANY, 0, &reg_val)); 
      range->min = soc_reg_field_get(unit, idr_ocb_multicast_range[range_ndx], reg_val, ocb_multicast_range_low[range_ndx]);
      range->max = soc_reg_field_get(unit, idr_ocb_multicast_range[range_ndx], reg_val, ocb_multicast_range_high[range_ndx]);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_mc_range_get_unsafe()", 0, 0);
#endif 
    return -1;
}

        
#ifdef FIXME_DNX_LEGACY
/* This function get the value we would like to insert and retuen the field value should be inserted to the files
   OCB_QUE_BUFF_SIZE_EN_TH_[0/1]f and OCB_QUE_SIZE_EN_TH_[0/1]f
*/
static uint32
    jer2_arad_mgmt_ocb_prm_value_to_field_value(
        DNX_SAND_IN  int unit,
        DNX_SAND_IN  uint32 value,
        DNX_SAND_OUT uint32 *field_value
    )
{
    uint32
        res;
    uint32
        mantissa,
        exponent;
        
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_PRM_VALUE_TO_FIELD_VALUE);


    DNXC_LEGACY_FIXME_ASSERT;

    res = dnx_sand_break_to_mnt_exp_round_up(value,
            JER2_ARAD_MGMT_IQM_OCBPRM_MANTISSA_NOF_BITS(unit),
            JER2_ARAD_MGMT_IQM_OCBPRM_EXPONENT_NOF_BITS,
            0,
            &mantissa,
            &exponent
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  jer2_arad_iqm_mantissa_exponent_set(unit,
    mantissa,
    exponent,
    JER2_ARAD_MGMT_IQM_OCBPRM_MANTISSA_NOF_BITS(unit),
    field_value
  );
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_prm_value_to_field_value()", 0, 0);
    return -1;
}

static uint32
    jer2_arad_mgmt_ocb_prm_buff_size_value_to_field_value(
        DNX_SAND_IN  int unit,
        DNX_SAND_IN  uint32 value,
        DNX_SAND_OUT uint32 *field_value
    )
{
    
    uint32
        res;
    uint32
        mantissa,
        exponent;
        
    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_PRM_VALUE_TO_FIELD_VALUE);
    DNXC_LEGACY_FIXME_ASSERT;

    res = dnx_sand_break_to_mnt_exp_round_up(value,
            JER2_ARAD_MGMT_IQM_OCBPRM_BUFF_SIZE_MANTISSA_NOF_BITS(unit),
            JER2_ARAD_MGMT_IQM_OCBPRM_BUFF_SIZE_EXPONENT_NOF_BITS,
            0,
            &mantissa,
            &exponent
          );
  DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  jer2_arad_iqm_mantissa_exponent_set(unit,
    mantissa,
    exponent,
    JER2_ARAD_MGMT_IQM_OCBPRM_BUFF_SIZE_MANTISSA_NOF_BITS(unit),
    field_value
  );
  
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_prm_buff_size_value_to_field_value()", 0, 0);
    return -1;
}
/* This function takes the field value of OCB_QUE_SIZE_EN_TH_[0/1]f,
   and give back the exact value */
static uint32
    jer2_arad_mgmt_ocb_prm_field_value_to_exact_value(
        DNX_SAND_IN int unit,
        DNX_SAND_IN uint32 field_value
    )
{
    uint32
        exponent,
        mantissa;
    
    DNXC_LEGACY_FIXME_ASSERT;
            
    jer2_arad_iqm_mantissa_exponent_get(unit,
      field_value,
      JER2_ARAD_MGMT_IQM_OCBPRM_MANTISSA_NOF_BITS(unit),
      &mantissa,
      &exponent
    );
    
    return mantissa * (1 << exponent);
}

/* This function takes the field value of OCB_QUE_BUFF_SIZE_EN_TH_[0/1]f,
   and give back the exact value */
static uint32
    jer2_arad_mgmt_ocb_prm_buff_size_field_value_to_exact_value(
        DNX_SAND_IN int unit,
        DNX_SAND_IN uint32 field_value
    )
{
    uint32
        exponent,
        mantissa;
    
    DNXC_LEGACY_FIXME_ASSERT;
            
    jer2_arad_iqm_mantissa_exponent_get(unit,
      field_value,
      JER2_ARAD_MGMT_IQM_OCBPRM_BUFF_SIZE_MANTISSA_NOF_BITS(unit),
      &mantissa,
      &exponent
    );
    
    return mantissa * (1 << exponent);
}

uint32 jer2_arad_mgmt_ocb_voq_info_defaults_set(
    DNX_SAND_IN     int                         unit,
    DNX_SAND_OUT    JER2_ARAD_MGMT_OCB_VOQ_INFO      *ocb_info
    )
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_LEGACY_FIXME_ASSERT;


    DNXC_NULL_CHECK(ocb_info);

    ocb_info->th_words[0] = JER2_ARAD_MGMT_IQM_OCB_PRM_DEFAULT_SIZE;
    ocb_info->th_words[1] = JER2_ARAD_MGMT_IQM_OCB_PRM_DEFAULT_SIZE;
    ocb_info->th_buffers[0] = JER2_ARAD_MGMT_IQM_OCB_PRM_DEFAULT_BUFF_SIZE;
    ocb_info->th_buffers[1] = JER2_ARAD_MGMT_IQM_OCB_PRM_DEFAULT_BUFF_SIZE;

    exit:
        DNXC_FUNC_RETURN;
}
 
uint32
  jer2_arad_mgmt_ocb_voq_eligible_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_IN  JER2_ARAD_MGMT_OCB_VOQ_INFO       *info,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO    *exact_info
  )
{
  uint32
    res;
  uint32
    index = 0,
    q_category_ndx_lcl = q_category_ndx,
    rate_class_ndx_lcl = rate_class_ndx,
    tc_ndx_lcl = tc_ndx,
    voq_eligible_lcl = info->voq_eligible,
    field_val;
  uint64
    reg_val;
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_SET_UNSAFE);

  DNXC_LEGACY_FIXME_ASSERT;


  if (!SOC_IS_QAX(unit)) {  
  /* calculating the index in the table */
  SHR_BITCOPY_RANGE(&index, JER2_ARAD_MGMT_IQM_OCB_PRM_QUEUE_CATEGORY_START, &q_category_ndx_lcl, 0, JER2_ARAD_MGMT_IQM_OCB_PRM_QUEUE_CATEGORY_LENGTH);
  SHR_BITCOPY_RANGE(&index, JER2_ARAD_MGMT_IQM_OCB_PRM_RATE_CLASS_START, &rate_class_ndx_lcl, 0, JER2_ARAD_MGMT_IQM_OCB_PRM_RATE_CLASS_LENGTH);
  SHR_BITCOPY_RANGE(&index, JER2_ARAD_MGMT_IQM_OCB_PRM_TRAFFIC_CLASS_START, &tc_ndx_lcl, 0, JER2_ARAD_MGMT_IQM_OCB_PRM_TRAFFIC_CLASS_LENGTH);
  
  /* set the information in the table */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IQM_OCBPRMm(unit, MEM_BLOCK_ANY, index, &reg_val));
  
  if (SOC_IS_JERICHO(unit)) {
      soc_mem_field32_set(unit, IQM_OCBPRMm, &reg_val, DRAM_ADMISSION_EXEMPTf, info->dram_admission_exempt & 0x1);
  } else {
      soc_mem_field32_set(unit, IQM_OCBPRMm, &reg_val, OCB_ENf, voq_eligible_lcl & 0x1);
  }

  jer2_arad_JER2_ARAD_MGMT_OCB_VOQ_INFO_clear(exact_info);

  res =  jer2_arad_mgmt_ocb_prm_buff_size_value_to_field_value(unit, info->th_buffers[0], &field_val );
  DNX_SAND_CHECK_FUNC_RESULT(res, 12, exit);
  soc_mem_field32_set(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_BUFF_SIZE_EN_TH_0f, field_val);  
  exact_info->th_buffers[0] = jer2_arad_mgmt_ocb_prm_buff_size_field_value_to_exact_value(unit, field_val);

  res =  jer2_arad_mgmt_ocb_prm_buff_size_value_to_field_value(unit, info->th_buffers[1], &field_val );
  DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  soc_mem_field32_set(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_BUFF_SIZE_EN_TH_1f, field_val);
  exact_info->th_buffers[1] = jer2_arad_mgmt_ocb_prm_buff_size_field_value_to_exact_value(unit, field_val);

  res =  jer2_arad_mgmt_ocb_prm_value_to_field_value(unit, info->th_words[0], &field_val );
  DNX_SAND_CHECK_FUNC_RESULT(res, 17, exit);
  soc_mem_field32_set(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_SIZE_EN_TH_0f, field_val);
  exact_info->th_words[0] = jer2_arad_mgmt_ocb_prm_field_value_to_exact_value(unit, field_val);

  res =  jer2_arad_mgmt_ocb_prm_value_to_field_value(unit, info->th_words[1], &field_val );
  DNX_SAND_CHECK_FUNC_RESULT(res, 19, exit);
  soc_mem_field32_set(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_SIZE_EN_TH_1f, field_val);
  exact_info->th_words[1] = jer2_arad_mgmt_ocb_prm_field_value_to_exact_value(unit, field_val);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IQM_OCBPRMm(unit, MEM_BLOCK_ANY, index, &reg_val));
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_voq_eligible_set_unsafe()", 0, 0);
}

uint32
  jer2_arad_mgmt_ocb_voq_eligible_dynamic_set_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    qid,
    DNX_SAND_IN  uint32                    enable
  )
{
  uint32
    res,
    mem_val,
    line,
    bit_in_line;   
    
  soc_reg_above_64_val_t
     old_wr_mask_val,
     wr_mask_val; 
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  

  DNXC_LEGACY_FIXME_ASSERT;
  
  sal_memset(wr_mask_val, 0,sizeof(wr_mask_val));
    
  line = qid / 32;
  bit_in_line = qid % 32;

  /*
   * Set indirect wr mask to 1 to allowe writing
   */

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IDR_INDIRECT_WR_MASKr(unit, old_wr_mask_val));
  
  wr_mask_val[0] = 0xffffffff;
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IDR_INDIRECT_WR_MASKr(unit, wr_mask_val)); 

  /*
   * Set relevent bit in IDR_MEM_1F0000 table.
   */
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IDR_MEM_1F0000m(unit, MEM_BLOCK_ANY, line, &mem_val));
  DNX_SAND_SET_BIT(mem_val, enable, bit_in_line);
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IDR_MEM_1F0000m(unit, MEM_BLOCK_ANY, line, &mem_val));  

  /* return to the old indirect wr mask value*/
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, READ_IDR_INDIRECT_WR_MASKr(unit, old_wr_mask_val)); 

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_voq_eligible_dynamic_set_unsafe()", 0, 0);
}



uint32
  jer2_arad_mgmt_ocb_voq_eligible_get_unsafe(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  uint32                    q_category_ndx,
    DNX_SAND_IN  uint32                    rate_class_ndx,
    DNX_SAND_IN  uint32                    tc_ndx,
    DNX_SAND_OUT JER2_ARAD_MGMT_OCB_VOQ_INFO       *info
  )
{
  uint32
    res;
  uint32
    index = 0,
    q_category_ndx_lcl = q_category_ndx,
    rate_class_ndx_lcl = rate_class_ndx,
    tc_ndx_lcl = tc_ndx,
    field_val;
  uint64
    reg_val;
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MGMT_OCB_VOQ_ELIGIBLE_GET_UNSAFE);


  DNXC_LEGACY_FIXME_ASSERT;

  if (!SOC_IS_QAX(unit)) {  
  /* calculating the index in the table */
  SHR_BITCOPY_RANGE(&index, JER2_ARAD_MGMT_IQM_OCB_PRM_QUEUE_CATEGORY_START, &q_category_ndx_lcl, 0, JER2_ARAD_MGMT_IQM_OCB_PRM_QUEUE_CATEGORY_LENGTH);
  SHR_BITCOPY_RANGE(&index, JER2_ARAD_MGMT_IQM_OCB_PRM_RATE_CLASS_START, &rate_class_ndx_lcl, 0, JER2_ARAD_MGMT_IQM_OCB_PRM_RATE_CLASS_LENGTH);
  SHR_BITCOPY_RANGE(&index, JER2_ARAD_MGMT_IQM_OCB_PRM_TRAFFIC_CLASS_START, &tc_ndx_lcl, 0, JER2_ARAD_MGMT_IQM_OCB_PRM_TRAFFIC_CLASS_LENGTH);

  /* get the information from the table */  
  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IQM_OCBPRMm(unit, MEM_BLOCK_ANY, index, &reg_val));
  if (SOC_IS_JERICHO(unit)) {
      info->voq_eligible = 0;
      info->dram_admission_exempt = soc_mem_field32_get(unit, IQM_OCBPRMm, &reg_val, DRAM_ADMISSION_EXEMPTf);
  } else {
      info->voq_eligible = soc_mem_field32_get(unit, IQM_OCBPRMm, &reg_val, OCB_ENf);
      info->dram_admission_exempt = 0;
  }
  
  field_val = soc_mem_field32_get(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_BUFF_SIZE_EN_TH_0f);
  info->th_buffers[0] = jer2_arad_mgmt_ocb_prm_buff_size_field_value_to_exact_value(unit, field_val);;

  field_val = soc_mem_field32_get(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_BUFF_SIZE_EN_TH_1f);
  info->th_buffers[1] = jer2_arad_mgmt_ocb_prm_buff_size_field_value_to_exact_value(unit, field_val);;
  
  field_val = soc_mem_field32_get(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_SIZE_EN_TH_0f);
  info->th_words[0] = jer2_arad_mgmt_ocb_prm_field_value_to_exact_value(unit, field_val);;

  field_val = soc_mem_field32_get(unit, IQM_OCBPRMm, &reg_val, OCB_QUE_SIZE_EN_TH_1f);
  info->th_words[1] = jer2_arad_mgmt_ocb_prm_field_value_to_exact_value(unit, field_val);;
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_mgmt_ocb_voq_eligible_get_unsafe()", 0, 0);
}
#endif 
/*********************************************************************
 *     Set core clock frequency according to given information in
 *     JER2_ARAD_MGMT_INIT struct.
 *     Details: in H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mgmt_init_set_core_clock_frequency (
    DNX_SAND_IN int unit,
    DNX_SAND_IN JER2_ARAD_MGMT_INIT* init
    ) 
{    
    uint32 /*res,*/ freq_jer2_arad_khz, intg;
    DNX_SAND_U64 frac, frac_x, frac_x2;

    DNXC_INIT_FUNC_DEFS;

    jer2_arad_chip_kilo_ticks_per_sec_set(
        unit,
        init->core_freq.frequency 
        );
    /* System Ref clock to set */
    if(!SOC_IS_QUX(unit))
    {
        /* Enable the fraction mode over cal. */
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, MULTI_FAP_3f,  0x1));

        freq_jer2_arad_khz = jer2_arad_chip_kilo_ticks_per_sec_get(unit);
        intg =  init->core_freq.system_ref_clock / freq_jer2_arad_khz;

        dnx_sand_u64_clear(&frac);
        dnx_sand_u64_clear(&frac_x);

        dnx_sand_u64_multiply_longs(init->core_freq.system_ref_clock, JER2_ARAD_MGMT_SYST_FREQ_RES_19, &frac_x);
        dnx_sand_u64_devide_u64_long(&frac_x, freq_jer2_arad_khz, &frac);

        if (freq_jer2_arad_khz <= init->core_freq.system_ref_clock) {
            dnx_sand_u64_multiply_longs(intg, JER2_ARAD_MGMT_SYST_FREQ_RES_19, &frac_x2);
            dnx_sand_u64_subtract_u64(&frac, &frac_x2);
        }

        /* Set the System clock frequency */
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_SYS_CONFIG_2r, REG_PORT_ANY, 0, SYS_CONFIG_21f,  frac.arr[0]));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_SYS_CONFIG_2r, REG_PORT_ANY, 0, SYS_CONFIG_22f,  intg));
    }
exit:
    DNXC_FUNC_RETURN;
}

int jer2_arad_mgmt_nof_block_instances(int unit, soc_block_types_t block_types, int *nof_block_instances) 
{
    int block_index;
    uint32 nof_instances = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(nof_block_instances);
    DNXC_NULL_CHECK(block_types);

    SOC_BLOCK_ITER_ALL(unit, block_index, *block_types)
    {
        ++nof_instances;
    }

    *nof_block_instances = nof_instances;

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Get the AVS - Adjustable Voltage Scaling value of the Arad
*********************************************************************/
int jer2_arad_mgmt_avs_value_get(
            int       unit,
            uint32*      avs_val)
{
    uint32
        reg_val;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(avs_val);

    *avs_val = 0;

    if (SOC_IS_ARADPLUS(unit)) {
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, ECI_AVS_STATUSr, REG_PORT_ANY, 0, &reg_val));
        *avs_val = soc_reg_field_get(unit, ECI_AVS_STATUSr, reg_val, AVS_STATUS_VALUEf);
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Device not supported.")));
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32
soc_jer2_arad_cache_table_update_all(int unit)
{
    return soc_dnxc_cache_table_update_all(unit);
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88690_A0) */
