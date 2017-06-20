/*
 * $Id: bm9600_diags.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        bm9600_diags.c
 * Purpose:     Polaris-specific diagnostics tests.
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/debug.h>
#include <sal/appl/io.h>
#include <soc/drv.h>

#ifdef BCM_BM9600_SUPPORT
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/hal_pl_auto.h>
#include <appl/test/bm9600_diags.h>
#include <appl/test/sbx_diags.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>
#include <bcm/error.h>
#include <sal/appl/sal.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <appl/diag/sbx/register.h>
#include <soc/sbx/bm9600_init.h>
#ifndef __KERNEL__
#include <time.h>
#include <signal.h>
#endif

extern cmd_result_t sbx_diag_write_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32 v);
extern cmd_result_t sbx_diag_read_reg(int unit, soc_sbx_reg_info_t *reg_info, uint32 *v);
extern int _mc_fpga_read8(int addr, uint8 *v);
extern int _mc_fpga_write8(int addr, uint8 data);
static uint8 gStopPrbsTest = 0;


#define DIAG_QE2000_SCI_PORT_BASE   68

/* SCI Mapping */

/* SHOULD BE MOVED  */

/* mapping SCI links to Polaris Ports */
uint32 sbx_qe_pl_port_map_rev1[2][2] = {
  { 25, 24 }, /* QE(Unit0),SCI0,SCI1 */
  { 26, 27 }, /* QE(Unit3),SCI0,SCI1 */
};

/* mapping SCI links to Polaris Ports */
uint32 sbx_qe_pl_port_map_rev0[2][2] = {
  { 23, 22 }, /* QE(Unit0),SCI0,SCI1 */
  { 56, 57 }, /* QE(Unit3),SCI0,SCI1 */
};

/* SCI0,SCI1 */
uint32 sbx_fe2k_pl_port_map_rev[1][2] = {
  { 25, 24 }, /* QE(Unit0),SCI0,SCI1 */
};

/* temp  -- my_dev_cmd_rec from sbx.c */
extern my_dev_cmd_rec polaris_lc_cmd_tbl[];
extern my_dev_cmd_rec fe2kxt_lc_cmd_tbl[];


/*
 *   PRBS (pseudorandom binary sequence) Tests
 *   QEXX <-----> Polaris 
 */

uint32 g_qexx_mask=0;
int g_qexx_lcm_ids[MAX_QE_NODES];
int g_brdtype;
int g_card_rev;

int sbBme9600DiagsPrbsTest(sbxDiagsInfo_t *pDiagsInfo)
{

  int i,j;
  uint32 unit;
  my_dev_cmd_rec *pcmd = NULL;
  uint32 tmp = 0;
  uint8 fpga_data1;
  uint8 fpga_data2;
  int rv;
  uint8 bTestRun = FALSE;
  int status = 0;
  char *lsr_polynomial = NULL;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  uint8 bRestoreSCIMapping = FALSE;

  unit = pDiagsInfo->unit;
  assert(pDiagsInfo->nLSFR == 0 || pDiagsInfo->nLSFR == 1);

  if (pDiagsInfo->qe_prbs_link_mask == 0x0 ||
      (pDiagsInfo->qe_prbs_mask == 0x0)) {
    LOG_WARN(BSL_LS_APPL_COMMON,
             (BSL_META("Specify a non-zero mask\n")));
    return 0;
  }

  g_qexx_mask = pDiagsInfo->qe_prbs_mask;

  for (i=0;i<MAX_QE_NODES;i++) {
    g_qexx_lcm_ids[i] = -1;
  }

  g_brdtype = sbx_diag_get_board_type();
  g_card_rev = sbx_diag_get_board_rev();
  assert(g_brdtype >= 0);
  switch(g_brdtype) {
  case BOARD_TYPE_POLARIS_LC:
    /* be sure both SCI links are enabled */
    for (i=24;i<=27;i++) {
      if (( rv = bcm_port_enable_set(unit,i,1)) < 0) {
	cli_out("ERROR could not enable %s port %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      if (( rv = bcm_port_control_set(unit,i,bcmPortControlRxEnable,1)) < 0) {
	cli_out("ERROR could not set %s hypercore port control rx %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      if (( rv = bcm_port_control_set(unit,i,bcmPortControlTxEnable,1)) < 0) {
	cli_out("ERROR could not set %s hypercore port control tx %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      if (( rv = bcm_stk_module_protocol_set(unit,BCM_MODULE_FABRIC_BASE+i,bcmModuleProtocol1)) < 0) {
	cli_out("ERROR could not set %s protocol for link:%d (%s)\n",SOC_CHIP_STRING(unit),i,bcm_errmsg(rv));
	return (-1);
      }
      if (( rv = bcm_port_control_set(unit,i,bcmPortControlAbility,BCM_PORT_ABILITY_SCI)) < 0) {
	cli_out("ERROR could not set %s hypercore port control ability %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      
    }
    pcmd = polaris_lc_cmd_tbl;
    break;
  case BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC:
    for (i=24;i<=25;i++) {

      if (( rv = bcm_port_enable_set(unit,i,1)) < 0) {
	cli_out("ERROR could not enable %s port %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      if (( rv = bcm_port_control_set(unit,i,bcmPortControlRxEnable,1)) < 0) {
	cli_out("ERROR could not enable %s hypercore port control rx %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      if (( rv = bcm_port_control_set(unit,i,bcmPortControlTxEnable,1)) < 0) {
	cli_out("ERROR could not enable %s hypercore port control tx %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
      if (( rv = bcm_stk_module_protocol_set(unit,BCM_MODULE_FABRIC_BASE+i,bcmModuleProtocol1)) < 0) {
	cli_out("ERROR could not set %s protocol for link:%d (%s)\n",SOC_CHIP_STRING(unit),i,bcm_errmsg(rv));
	return (-1);
      }

      if (( rv = bcm_port_control_set(unit,i,bcmPortControlAbility,BCM_PORT_ABILITY_SCI)) < 0) {
	cli_out("ERROR could not set %s hypercore port control ability %d\n",SOC_CHIP_STRING(unit),i);
	return (-1);
      }
    }
    pcmd = fe2kxt_lc_cmd_tbl;
    break;
  default:
    LOG_WARN(BSL_LS_APPL_COMMON,
             (BSL_META("Unsupported board type(%d). "),g_brdtype));
    return (-1);
  }

  cli_out("Starting test.. \n");

  
  rv = sbBme9600DiagsFindAllQExx(unit, g_qexx_lcm_ids);
  if (rv != 0) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("No QExx type devices found.\n")));
    return rv;
  }


  /* Be sure both SCI links are Enabled at the QE's*/
  for (i=0;i<MAX_QE_NODES;i++) {
    if (g_qexx_lcm_ids[i] != -1) {
      for (j=DIAG_QE2000_SCI_PORT_BASE;j<=DIAG_QE2000_SCI_PORT_BASE+1;j++) {
	if (( rv = bcm_port_enable_set(g_qexx_lcm_ids[i],j,1)) < 0) {
	  cli_out("ERROR could not enable %s port %d\n",SOC_CHIP_STRING(unit),j);
	  return (-1);
	}
      }
    }
  }

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1 ) != CMD_OK) {
    cli_out("ERROR: Register info unknown for unit %d \n", unit);
    return (-1);
  }

  /* be sure all links are powered up */
  for(i=0;i< chip_info->nregs;i++) {
    reg_info = chip_info->regs[i];
    if (strstr(reg_info->name,"_sd_config")) {
      sbx_diag_read_reg(unit,reg_info,&tmp); 
      tmp &= ~(1<<0); /* rx_pwrdwn */
      tmp &= ~(1<<3); /* tx_pwrdwn */
      sbx_diag_write_reg(unit,reg_info,tmp);
    }
  }

  /* Run PRBS on each link between the polaris and QExx device, regardless of link state */
  for (i = 0; i < MAX_QE_NODES; i++ ) {
    if (g_qexx_lcm_ids[i] != -1 && (g_qexx_mask & (1 << g_qexx_lcm_ids[i]))) {
      lsr_polynomial = (pDiagsInfo->nLSFR == 0 ) ? "X7_X6_1" : "X15_X14_1";
      bTestRun = TRUE;
      cli_out("Testing using LSFR Polynomial %s\n",lsr_polynomial);
      rv = sbBme9600DiagsSFIPrbsTest(pDiagsInfo,
				     pcmd,
				     g_qexx_lcm_ids[i]);
      if (rv < 0) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("PRBS test found errors testing with QE[u:%d] using %s polynomial\n"),g_qexx_lcm_ids[i],lsr_polynomial));
	status = -1;
      }
    }
  }

  if (!bTestRun) {
    cli_out("No QE's found in mask:0x%x\n",g_qexx_mask);
    status = -1;
    goto done;
  }

  /* Test SCI links between QE<-->PL */
  if (g_brdtype == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
    _mc_fpga_read8(17,&fpga_data1);
    if ((fpga_data1 & 0xf) != 0xa) {
      LOG_WARN(BSL_LS_APPL_COMMON,
               (BSL_META("Setting SCI links to go to polaris for test\n")));
      bRestoreSCIMapping = TRUE;
      _mc_fpga_write8(17, 0xa); /* be sure SCI links go to polaris */
    }
  } else {
    _mc_fpga_read8(16,&fpga_data1);
    if ((fpga_data1 & 0xf0) != 0xa0) {
      LOG_WARN(BSL_LS_APPL_COMMON,
               (BSL_META("Setting QE0 SCI links to go to polaris for test\n")));
      bRestoreSCIMapping = TRUE;
      _mc_fpga_write8(16,0xa0);
    }
    _mc_fpga_read8(17,&fpga_data2);
    if ((fpga_data2 & 0xf) != 0xa) {
      LOG_WARN(BSL_LS_APPL_COMMON,
               (BSL_META("Setting QE1 SCI links to go to polaris for test\n")));
      bRestoreSCIMapping = TRUE;
      _mc_fpga_write8(17, 0x0A);
    }
  }

  rv = sbBme9600DiagsSCIPrbsTest(pDiagsInfo);
  if (rv < 0) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("PRBS test found errors testing SCI links\n")));
    status = -1;
  }

  /* restore orginal SCI mapping */
  if (bRestoreSCIMapping) {
    cli_out("Restoring SCI Mapping to backplane.\n");
    if (g_brdtype == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
      _mc_fpga_write8(17,fpga_data1);
    } else {
      _mc_fpga_write8(16,fpga_data1);
      _mc_fpga_write8(17,fpga_data2);
    }
  }

 done:
  return status;
}

/* Requires updating to locate QExx's LCM96  on different linecards */
int sbBme9600DiagsFindAllQExx(int unit,int g_qexx_lcm_ids[])
{
  int i,j;
  int status = -1;
  i = j = 0;
  for (i=0;i<soc_ndev;i++) {
    if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i))) {
      g_qexx_lcm_ids[j++] = SOC_NDEV_IDX2DEV(i);
      status = 0;
    } else if (0/*SOC_IS_SBX_QE50(SOC_NDEV_IDX2DEV(i))*/) {
      g_qexx_lcm_ids[j++] = SOC_NDEV_IDX2DEV(i);
      status = 0;
    } else if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i))) {
      if (SOC_NDEV_IDX2DEV(i) == unit) continue;
      g_qexx_lcm_ids[j++] = SOC_NDEV_IDX2DEV(i);
      status = 0;
    }
  }
  return status;
}



int sbBme9600DiagsSFIPrbsTest(sbxDiagsInfo_t *p,
			      my_dev_cmd_rec *pcmd,
			      int qexx_unit)
{
  
  int rv,qe_modid,i,dir;
  bcm_port_t nQePort;
  int nQePhysicalPort;
  int nXbPort;
  int prbs_stat = 0x0;
  bcm_port_prbs_t prbs_type;
  int status = 0;
  int pl_unit;
  char *direction = NULL;
  int start, end;
  int rx_unit = 0;
  int tx_unit = 0;
  int rx_port = 0;
  int tx_port = 0;
  uint32 qe = 0;
  uint32 qexx_sfi_link_mask=0;
  int failedCount = 0;

#ifndef __KERNEL__
  signal(SIGINT,polaris_sigcatcher);
#else
  
#endif

  qexx_sfi_link_mask = p->qe_prbs_link_mask & 0x3ffff;
  pl_unit = p->unit;
  /* by default test in both directions: KA ---> PL, KA <----- PL */
  if (p->prbs_direction == -1 ) {
    start = PRBS_TO_PL;
    end = PRBS_TO_QEXX;
  } else {
    start = end = p->prbs_direction;
  }

  if (p->nLSFR == 0 ) {
    prbs_type = BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1;
  } else {
    prbs_type = BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1;
  }

  rv = bcm_stk_modid_get(qexx_unit,&qe_modid);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("bcm_stk_modid_get failed(%s) for UNIT:%d\n"),bcm_errmsg(rv),qexx_unit));
    return (-1);
  }

  for (dir = start; dir <= end; dir++) {
    for(i = 0; i < SB_FAB_DEVICE_QE2000_SFI_LINKS ; i++ ) {
      if (!(qexx_sfi_link_mask & ( 1 << i))) continue;

      rv = bcm_fabric_crossbar_mapping_get(qexx_unit,qe_modid, 0,i,&nQePort);
      if (BCM_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("bcm_fabric_crossbar_mapping_get (%d)(%s)\n"),rv,bcm_errmsg(rv)));
	return (-1);
      }

      nQePhysicalPort = nQePort - 50; /* QE2000_SFI_PORT_BASE, defined in sbx.c */
      /* get the xbar port for this QE modid, link. This info is fixed, based on board type */

      nXbPort = sbBme9600DiagsGetQExxXbarMapping(pcmd, qe_modid, nQePhysicalPort);

      if (nXbPort == -1 ) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("Could not find xbPort mapping for QE[mod:%d,u:%d,link:%d]\n"),
                   qe_modid,qexx_unit,i));
	status = -1;
	continue;
      }

      if ( dir == PRBS_TO_PL) {
	direction = "-->";
	tx_unit = qexx_unit; 
	tx_port = nQePort;
	rx_unit = pl_unit; 
	rx_port = nXbPort;
      } else if (dir == PRBS_TO_QEXX) {
	direction = "<--";
	tx_unit = pl_unit; 
	tx_port = nXbPort;
	rx_unit = qexx_unit; 
	rx_port = nQePort;
      }

    LoopOnPrbsError:
      cli_out("Testing PRBS %s[MOD:%d,U:%d,SFI:%02d] %s %s[U:%d,port:%02d HC%d_lane%d] .. ",SOC_CHIP_STRING(qexx_unit),
              qe_modid,qexx_unit,i,direction,
              SOC_CHIP_STRING(pl_unit),
              pl_unit,nXbPort,
              nXbPort/PL_SI_PER_HC,
              nXbPort%PL_SI_PER_HC);

    start:
      prbs_stat = 0xff;

      rv = sbBme9600DiagsStartPrbs(rx_unit,rx_port,tx_unit,tx_port,
				   p->bForcePRBSError,prbs_type,i,&prbs_stat);

      if (rv != 0) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META("(%s) Errors starting prbs test.\n"),FUNCTION_NAME()));
	return rv;
      }

      if (prbs_stat == 0 ) {
	if (!p->bForcePRBSError) {
	  cli_out("PASSED \n");
	  failedCount = 0;
	} else {
	  cli_out("PASSED, but should have failed with force_error=1\n");
	}
      } else {
	if (p->bForcePRBSError) {
	  cli_out("FAILED (expected), status = 0x%x\n",prbs_stat);
	} else {
	  failedCount++;
	  if (failedCount == 1) {
	    prbs_stat = 0;
	    sbBme9600DiagsStopPrbs(rx_unit,rx_port,tx_unit,tx_port);
	    goto start; /* retest to be sure link is bad,and not some ufo */
	  }
	  status = -1;
	  cli_out("FAILED status = 0x%x\n",prbs_stat);
	  /* show command to retest the failing link */
	  if (SOC_IS_SBX_QE2000(rx_unit)) {
	    qe = rx_unit;
	  } else {
	    qe = tx_unit;
	  }
	  if (!p->bLoopOnError) { /* do not show if looping on this error */
	    cli_out("To re-test this link TR 110 qe_mask=0x%x qe_link_mask=0x%x dir=%d Lsfr=%d LoopOnError=[0|1]\n",
                    (1<<qe), (1<<nQePhysicalPort),dir,p->nLSFR);
	  }

	}
      }

      /* if a link fails allow option to continue testing it to probe board during failure */
      if (status == -1 && p->bLoopOnError && gStopPrbsTest != 1) {
	goto LoopOnPrbsError;
      }
    }
    cli_out("------------------------------------------------------------------------------------------\n");
  }
  return status;
}

int sbBme9600DiagsGetQExxXbarMapping(my_dev_cmd_rec *pcmd,
				     int qexx_modid, 
				     int nQePhysicalPort)  
{

  my_dev_cmd_rec *top = NULL;
  int xb_port = -1;
  top = pcmd;
  while(top->cmd != -1) {
    if (top->cmd == 18 || top->cmd == 22 /*XBAR_CONNECT_ADD || XBAR_LGL_CONNECT_ADD*/) {
      /* find the one-to-one mapping for this link and qe modid */
      if ((*((int*)(top->arg2)) == qexx_modid) && 
	  ((top->arg4 == g_card_rev) || top->arg4 == 0 /* CMD_CARD_ALL */ ) &&
	  (top->arg1 == nQePhysicalPort)) {
	xb_port = top->arg3;
	break;
      }
    }
    top++;
  }
  return xb_port;
}


int sbBme9600DiagsStopPrbs(int rx_unit,
			   int rx_port,
			   int tx_unit,
			   int tx_port)

{

  int rv = 0;

  /* PL/QE to stop sending PRBS */
  rv = bcm_port_control_set(tx_unit,tx_port,bcmPortControlPrbsTxEnable,0);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("Failed to stop %s from sending prbs\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(tx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }

  /* Call bcm_port_control_set on QE/PL to stop receiving PRBS */
  rv = bcm_port_control_set(rx_unit,rx_port,bcmPortControlPrbsRxEnable,0);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("Failed to stop %s from recieving prbs\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(rx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }
  
  return rv;
}

int sbBme9600DiagsStartPrbs(int rx_unit,
			    int rx_port,
			    int tx_unit,
			    int tx_port,
			    uint8 bForceError,
			    bcm_port_prbs_t prbs_type,
			    int link,
			    int *prbs_status)

{

  int rv = 0;
  int force_error = 0;
  int bError = 1;
  uint uData = 0;

  /* set prbs mode on Polaris to 1 PRBS in SI BLOCK */
  if (SOC_IS_SBX_BM9600(rx_unit)) {
    rv = bcm_port_control_set(rx_unit,rx_port,bcmPortControlPrbsMode,1);
    if (BCM_FAILURE(rv)) {
      LOG_ERROR(BSL_LS_APPL_COMMON,
                (BSL_META("\nFailed to set %s prbs mode for port:%d \n bcm_port_control_set (%d)(%s)\n"),
                 SOC_CHIP_STRING(rx_unit),rx_port,rv,bcm_errmsg(rv)));
      return (-1);
    }  
  }

  /* set PRBS polynomial on transmitting unit */
  rv = bcm_port_control_set(tx_unit,tx_port,bcmPortControlPrbsPolynomial,prbs_type);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("\nFailed to set %s prbs polynomial\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(tx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }


  /* set PRBS polynomial on recieving unit */
  rv = bcm_port_control_set(rx_unit,rx_port,bcmPortControlPrbsPolynomial,prbs_type);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("\nFailed to set %s prbs polynomial\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(rx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }  

  /* Call bcm_port_control_set on QE/PL to send PRBS */
  rv = bcm_port_control_set(tx_unit,tx_port,bcmPortControlPrbsTxEnable,1);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("\nFailed to set %s to send PRBS\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(tx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }  

  /* Call bcm_port_control_set on QE/PL to recieve PRBS */
  rv = bcm_port_control_set(rx_unit,rx_port,bcmPortControlPrbsRxEnable,1);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("\nFailed to set %s to rx PRBS\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(rx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }  

  force_error = (bForceError) ? 1 : 0;
  rv = bcm_port_control_set(tx_unit,tx_port,bcmPortControlPrbsForceTxError,force_error);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("Failed to set %s to force PRBS Error\n bcm_port_control_set (%d)(%s)\n"),
               SOC_CHIP_STRING(tx_unit),rv,bcm_errmsg(rv)));
    return (-1);
  }  

  /* run for two seconds */
  thin_delay(2E9);

  /* Setup QE/PL to retrieve PRBS status */
  rv = bcm_port_control_get(rx_unit,rx_port,bcmPortControlPrbsRxStatus,prbs_status);
  if (BCM_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META("Failed to get %s prbs status\n bcm_port_control_get (%d)(%s)\n"),
               SOC_CHIP_STRING(rx_unit),rv,bcm_errmsg(rv)));
    *prbs_status = -1;
    return (-1);
  }

  /* read the HW error status register, this is set by hw if an error occurs (RO) */
  if (SOC_IS_SBX_QE2000(rx_unit)) {
    if (IS_SCI_PORT(rx_unit,rx_port)) { /* control link */
      if (link == 0) {
	uData = SAND_HAL_READ((sbhandle)rx_unit, KA,SC_SI0_PRBS_STATUS);
	bError = SAND_HAL_GET_FIELD(KA,SC_SI0_PRBS_STATUS,PRBS_ERROR_OCCURED,uData);
      } else {
	uData = SAND_HAL_READ((sbhandle)rx_unit, KA,SC_SI1_PRBS_STATUS);
	bError = SAND_HAL_GET_FIELD(KA,SC_SI1_PRBS_STATUS,PRBS_ERROR_OCCURED,uData);
      }
    } else { /* sfi link */
      uData = SAND_HAL_READ_STRIDE((sbhandle)rx_unit, KA, SF, link, SF0_SI_PRBS_STATUS);
      bError = SAND_HAL_GET_FIELD(KA, SF0_SI_PRBS_STATUS, PRBS_ERROR_OCCURED, uData);
    }
  } else if (SOC_IS_SBX_BM9600(rx_unit)) {
    uData = SAND_HAL_READ_STRIDE((sbhandle)rx_unit, PL, SI, rx_port, SI0_PRBS_STATUS);
    bError = SAND_HAL_GET_FIELD(PL, SI0_PRBS_STATUS, PRBS_ERROR_OCCURED, uData);
  }

  if (bError == 0) {
    /* no error detected by HW */
    *prbs_status = 0;
  }
  return 0;
}

int sbBme9600DiagsSCIPrbsTest(sbxDiagsInfo_t *pDiagsInfo) 
{

  int i,sci;
  int start,end,dir;
  int pl_unit=0;
  char *direction = NULL;
  int nXbPort=-1 ;
  int prbs_status = 0;
  int rx_unit = 0;
  int tx_unit = 0;
  int rx_port = 0;
  int tx_port = 0;
  int status = 0;
  int qe;
  uint32 qexx_sci_link_mask = 0;

  pl_unit = pDiagsInfo->unit;
#ifndef __KERNEL__
  signal(SIGINT,polaris_sigcatcher);
#else
  
#endif

  if (pDiagsInfo->prbs_direction == -1 ) {
    start = PRBS_TO_PL;
    end = PRBS_TO_QEXX;
  } else {
    start = end = pDiagsInfo->prbs_direction;
  }

  qexx_sci_link_mask = pDiagsInfo->qe_prbs_link_mask & 0xc0000;
  for (i = 0; i < MAX_QE_NODES; i++ ) {
    if (g_qexx_lcm_ids[i] != -1 && (g_qexx_mask & (1 << g_qexx_lcm_ids[i]))) {
      for (dir = start ; dir <= end; dir++ ) {
	for (sci = 0; sci < SB_FAB_DEVICE_QE2000_SCI_LINKS; sci++ ) {
	  if (!(qexx_sci_link_mask & ( 1 << (sci+SB_FAB_DEVICE_QE2000_SFI_LINKS)))) continue;
	  switch(g_brdtype) {
	  case BOARD_TYPE_POLARIS_LC:
	      if (g_card_rev == 0x2) {
		nXbPort = sbx_qe_pl_port_map_rev1[g_qexx_lcm_ids[i] >> 1][ sci ];
	      } else if (g_card_rev == 0x1) {
		nXbPort = sbx_qe_pl_port_map_rev1[g_qexx_lcm_ids[i] >> 1][ sci ];
	      }
	    break;
	  case BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC:
	    nXbPort = sbx_fe2k_pl_port_map_rev[g_qexx_lcm_ids[i] >> 1][ sci ];
	    break;
	  default:
	    nXbPort = -1;
	  }

	  if (nXbPort == -1 ) {
	    cli_out("No SCI mapping found for this board\n");
	    return (-1);
	  }

	  if (dir==0) {
	    rx_unit = pl_unit;
	    direction = "-->";
	    rx_port = nXbPort;
	    tx_unit = g_qexx_lcm_ids[i];
	    tx_port = (sci == 0 ) ? DIAG_QE2000_SCI_PORT_BASE+1 : DIAG_QE2000_SCI_PORT_BASE;
	  } else {
	    rx_unit = g_qexx_lcm_ids[i];
	    rx_port = (sci == 0 ) ? DIAG_QE2000_SCI_PORT_BASE+1 : DIAG_QE2000_SCI_PORT_BASE;
	    tx_unit = pl_unit;
	    direction = "<--";
	    tx_port = nXbPort;
	  }

	LoopOnPrbsError:
	  cli_out("Testing PRBS %s[U:%d,SCI:%d] %s %s[U:%d,port:%02d HC%d_lane%d] .. ",
                  SOC_CHIP_STRING(g_qexx_lcm_ids[i]),
                  g_qexx_lcm_ids[i],sci,direction,
                  SOC_CHIP_STRING(pl_unit),
                  pl_unit,nXbPort,
                  nXbPort/PL_SI_PER_HC,
                  nXbPort%PL_SI_PER_HC);


	  prbs_status = 0xff;

	  /* stop any running prbs */
	  sbBme9600DiagsStopPrbs(rx_unit,rx_port,tx_unit,tx_port);
	  thin_delay(1E9);

	  sbBme9600DiagsStartPrbs(rx_unit,rx_port,tx_unit,tx_port,
				  pDiagsInfo->bForcePRBSError,pDiagsInfo->nLSFR,
				  sci, &prbs_status);
	  if (prbs_status != 0) {
	    cli_out("FAILED status = 0x%x\n",prbs_status);
	    status = -1;
	    if (SOC_IS_SBX_QE2000(rx_unit)) {
	      qe = rx_unit;
	    } else {
	      qe = tx_unit;
	    }
	    if (!pDiagsInfo->bLoopOnError) { /* do not show if looping on this error */
	      cli_out("To re-test this link TR 110 qe_mask=0x%x qe_link_mask=0x%x dir=%d Lsfr=%d LoopOnError=[0|1]\n",
                      (1<<qe), (1<<(sci+SB_FAB_DEVICE_QE2000_SFI_LINKS)),dir,pDiagsInfo->nLSFR);
	    }
	  } else {
	    cli_out("PASSED\n");
	  }

	  if (status == -1 && pDiagsInfo->bLoopOnError && gStopPrbsTest != 1) {
	    goto LoopOnPrbsError;
	  }
	}
      }
    }
  }
  return status;
}			    
			    

/* simple register read/write test. Polaris is present and pci is working */
int sbBme9600DiagsRegTest(sbxDiagsInfo_t *pDiagsInfo)
{

  int unit;
  cmd_result_t rv;
  soc_sbx_chip_info_t *chip_info = NULL;
  soc_sbx_reg_info_t *reg_info = NULL;
  soc_sbx_reg_info_list_t *pl_test_regs_l = NULL;
  int i,j,idx;
  uint32 regval;
  uint32 tmp;
  int stat = -1;

  unit = pDiagsInfo->unit;
  pl_test_regs_l = sal_alloc(sizeof (soc_sbx_reg_info_list_t), "reginfo_l");
  if (!pl_test_regs_l) {
    LOG_ERROR(BSL_LS_APPL_COMMON,
              (BSL_META_U(unit,
                          "%s ERROR:  Out of Memory \n"),FUNCTION_NAME()));
    return CMD_FAIL;
  }

  if (sbx_chip_info_get(SOC_INFO(unit).chip_type, &chip_info,1) != CMD_OK) {
    cli_out("ERROR: Register info unknown for unit %d \n", unit);
    sal_free(pl_test_regs_l);
    return (-1);
  }

  pl_test_regs_l->count = 0;

  for(idx = 0; idx < chip_info->nregs;idx++) {
    reg_info = chip_info->regs[idx];
    if(strstr(reg_info->name,"revision")) {
      pl_test_regs_l->idx[pl_test_regs_l->count++] = idx;
    }
    if(strstr(reg_info->name,"pi_unit_interrupt8_mask")) { /* arbitrary R/W reg */
      pl_test_regs_l->idx[pl_test_regs_l->count++] = idx;
    }
  }

  for(i = 0; i < pl_test_regs_l->count; i++) {
    idx = pl_test_regs_l->idx[i];
    reg_info = chip_info->regs[idx];
    if (strstr(reg_info->name,"pi_revision")) {
      rv = sbx_diag_read_reg(unit,reg_info,&regval);
      if (rv != CMD_OK) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "sbx_diag_read_reg failed(%d)\n"),rv));
	sal_free(pl_test_regs_l);
	return (-1);
      }
      for(j=0;j<reg_info->nfields;j++) {
	cli_out("%s.%s = 0x%x\n",SOC_CHIP_STRING(unit),
                reg_info->fields[j]->name,
                (regval & reg_info->fields[j]->mask));
      }
    } else if (strstr(reg_info->name,"pi_unit_interrupt8_mask")) {
      /* be sure we can read/modify/write */
      rv = sbx_diag_read_reg(unit,reg_info,&regval);
      if (rv != CMD_OK) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "sbx_diag_read_reg failed(%d)\n"),rv));
	sal_free(pl_test_regs_l);
	return (-1);
      }

      rv = sbx_diag_write_reg(unit,reg_info,~regval);
      if (rv != CMD_OK) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "sbx_diag_write_reg failed(%d)\n"),rv));
	sal_free(pl_test_regs_l);
	return (-1);
      }
      
      sbx_diag_read_reg(unit,reg_info,&tmp);
      if (tmp != ~regval) {
	LOG_ERROR(BSL_LS_APPL_COMMON,
                  (BSL_META_U(unit,
                              "Write failed exp:0x%x got:0x%x\n"),~regval,tmp));
      } else {
	cli_out("Simple Read/Modify/Write test passed.\n");
	stat = 0;
      }
      /* restore orginal value */
      sbx_diag_write_reg(unit,reg_info,regval);
    }
  }

  if (stat < 0) {
    cli_out("Test Failed.\n");
  }

  sal_free(pl_test_regs_l);
  return stat;
}

void polaris_sigcatcher(int signum)
{
  sal_printf("\n");
  sal_printf("setting gStopPrbsTest to one\n");
  gStopPrbsTest = 1;
  return;
}

#endif /* BCM_BM9600_SUPPORT */
