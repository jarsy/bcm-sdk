/*
 * $Id: onu.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     onu.h
 * Purpose:
 *
 */
#ifndef _BCM_EA_ONU_H
#define _BCM_EA_ONU_H

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkDefs.h>
#include <soc/ea/tk371x/TkOnuApi.h>
#include <soc/ea/tk371x/common.h>
#include <soc/ea/tk371x/ea_drv.h>
#include <soc/macipadr.h>

#define GPIO_BANK0          0x00000001
#define GPIO_BANK1          0x00000002

#define GPIO_FLAGS_TK371X   0X00000000

#define SOC_EPON_FIRMWARE_VERSION(unit, version) \
    version = soc_ea_firmware_ver_get(unit)
    
#define SOC_EPON_EXTENSION_ID(unit, extension_id) \
    extension_id = soc_ea_extend_id_get(unit)

#define SOC_EPON_JEDEC_ID(unit, jedec_id) \
    jedec_id = soc_ea_jedecid_get(unit)

#define SOC_EPON_CHIP_ID(unit, chip_id)\
    chip_id = soc_ea_chip_id_get(unit)
    
#define SOC_EPON_CHIP_REVISION(unit, chip_rev) \
    chip_rev = soc_ea_chip_rev_get(unit)
    
typedef struct {
    sal_mac_addr_t pon_base_mac;
    sal_mac_addr_t uni_base_mac;
} soc_base_mac_t;

typedef enum{
    socEaLoopBackNone = OamLoopLocNone,
    socEaLoopbackPhy = OamLoopLocUniPhy,
    socEaLoopbackMac = OamLoopLocUniMac,
    socEaLoopback8023ah = OamLoopLoc8023ah,
    socEaLoopbackNowhere= OamLoopLoc8023ah+1,
    socEaLoopbackCount
} soc_ea_loopback_t;

typedef OamTkInfo		_soc_ea_oam_onu_info_t;
/* EPON Link Information */
typedef OamEponLinkInfo _soc_ea_oam_epon_link_info_t;
/* EPON Port Information */
typedef OamEponPortInfo _soc_ea_oam_epon_port_info_t;
/* EPON Base MAC, include pon base mac and user base mac*/
typedef OamEponBaseMac _soc_ea_oam_epon_base_mac_t;
/* EPON LOS STATE */
typedef EponLosState _soc_ea_oam_epon_los_state_t;
/* EPON LLID STATE */
typedef LlidState _soc_ea_llid_state_t;
/* OAM Version */
typedef OamVersion _soc_ea_oam_version_t;

/* VLAN TK link to llid mapping conde */
typedef TkVlanToLLIDMappingCond _soc_ea_tk_vlan_to_llid_mapping_cond_t;

typedef TkVlanToLLIDMapping	_soc_ea_tk_vlan_to_llid_mapping_t;

/* tk vlan action */
typedef TkVlanAction _soc_ea_tk_vlan_action_t;

/* TK VLAN Match mode */
typedef TkMatchMode _soc_ea_tk_match_mode_t;

typedef EponTxLaserStatus _soc_ea_epon_tx_laser_status_t;

typedef TkLinkRegInfo _soc_ea_tk_llid_reg_info_t;

typedef TkEponStatus _soc_ea_tk_epon_status_t;

typedef TkFailSafeState _soc_ea_tk_fail_safe_state_t;


#define _soc_ea_onu_info 			TkExtOamOnuInfo
#define _soc_ea_jedec_id_get		TkExtOamGetJedecID
#define _soc_ea_chip_id_get			TkExtOamGetChipID
#define _soc_ea_chip_rev_get		TkExtOamGetChipRev
#define _soc_ea_gpio_set			TkExtOamSetGpio
#define _soc_ea_gpio_get			TkExtOamGetGpio
#define _soc_ea_fec_mode_get		TkExtOamGetFECMode
#define _soc_ea_fec_mode_set		TkExtOamSetFECMode
#define _soc_ea_epon_admin_set		TkExtOamSetEponAdmin
#define _soc_ea_epon_admin_get		TkExtOamGetEponAdmin
#define _soc_ea_onu_llid_get		TkExtOamGetOnuLlid
#define _soc_ea_epon_base_mac_set	TkExtOamSetEponBaseMac
#define _soc_ea_epon_port_info_get	TkExtOamGetEponPortInfo
#define _soc_ea_battery_backup_set	TkExtOamSetBatteryBackup
#define _soc_ea_battery_backup_get	TkExtOamGetBatteryBackup
#define _soc_ea_ctc_oam_version_get	TkExtOamGetCtcOamVersion
#define _soc_ea_pon_los_info_get	TkExtOamGetPonLosInfo
#define _soc_ea_pon_reg_status_get	TkExtOamGetPonRegStatus
#define _soc_ea_encrypt_key_expiry_time_get \
									TkExtOamGetEncryptKeyExpiryTime
#define _soc_ea_encrypt_key_expiry_time_set \
                                    TkExtOamSetEncryptKeyExpiryTime
#define _soc_ea_vlan_to_llid_mapping_set \
									TkExtOamSetVlanToLLIDMapping
#define _soc_ea_vlan_to_llid_mapping_get \
									TkExtOamGetVlanToLLIDMapping
#define _soc_ea_vlan_to_llid_mapping_strip_tag_set \
									TkExtOamSetVlan2LLIDMappingStripTag
#define _soc_ea_mllid_link_status_get	\
									TkExtOamGetMLLIDLinkStatus
#define _soc_ea_laser_tx_pwr_off_time_set \
									TkExtOamSetLarseTxPwrOffTime
#define _soc_ea_fail_safe_state_set TkExtOamSetFailSafeState
#define _soc_ea_fail_safe_state_get	TkExtOamGetFailSafeState

#define _soc_ea_phy_control_opt_tran_diag_get(unit, info) \
    CtcExtOamGetOptTransDiag(unit, 0, info)

#define _soc_ea_phy_control_if_set(unit, if_no) \
    CtcExtOamSetPonIfAdmin(unit, 0, if_no)

#define _soc_ea_phy_control_if_get(unit, if_no) \
    CtcExtOamGetPonIfAdmin(unit, 0, if_no)

int 
_soc_ea_load_info_get(int32 unit, soc_ea_load_info_t *load_info);

/* here are the onu information part */
int
_soc_ea_firmware_ver_get(int unit, uint16 *ver);

int 
_soc_ea_revision_id_get(int unit, uint32 *rev);

int 
_soc_ea_vendor_id_get(int unit, soc_ea_extend_id_t id);

int
_soc_ea_ctc_oam_ver_get(int unit, uint32 *ver);

int
_soc_ea_real_llid_count_get(int unit, uint32 *llid_count);

int
_soc_ea_info_get(int unit, soc_ea_onu_info_t *info);

/*
 * Function:
 *      _soc_ea_mllid_set
 * Description:
 *      Enable, disable multiple llid or notify pon firmware the authentication 
 *      successful or not.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      count - (IN) 0 authentication fail, 1 to the count of max count of llid 
 *            the pon chipset support means enable multiple llid, max count add 1
 *            means authentication sucessfull.
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_mllid_set(int unit, uint32 count);

/*
 * Function:
 *      _soc_ea_mllid_get
 * Description:
 *      Get state of Enable, disable multiple llid or notify pon firmware the 
 *      authentication successful or not.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      count - (IN) 0 authentication fail, 1 to the count of max count of llid 
 *            the pon chipset support means enable multiple llid
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_mllid_get(int unit, uint32 *count);

/*
 * Function:
 *      _soc_ea_gpio_write
 * Description:
 *      Write a GPIO value belonged to the bank, only the bits masked by the mask will be 
 *      write to the gpio register.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 *      data - (IN) Value to be write to the GPIO
 * Returns     : SOC_E_XXX
 */
int
_soc_ea_gpio_write(int unit, uint32 flags, uint32 mask, uint32 data);

/*
 * Function:
 *      _soc_ea_gpio_read
 * Description:
 *      Read a GPIO value belonged to the bank
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      data - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_gpio_read(int unit, uint32 flags, uint32 *data);

/*
 * Function:
 *      _soc_ea_gpio_config_write
 * Description:
 *      Wirte the gpio configuration
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 * Returns     : SOC_E_XXX
 */
int
_soc_ea_gpio_config_write(int unit, uint32 flags, uint32 mask);

/*
 * Function:
 *      _soc_ea_port_info_get
 * Description:
 *     get the port information from the soc data structure
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port - (IN) port
 *      type - (IN) targe will be gotten from the data structue
 *      value - (OUT) the data read out 
 * Returns     : SOC_E_XXX
 */
int
_soc_ea_port_info_get(int unit, soc_port_t port, soc_ea_port_control_t type, 
    int *value);

/*
 * Function:
 *      _soc_ea_port_info_set
 * Description:
 *      change the information of the port
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port - (IN) BANK flag
 *      type - (IN) target will be changed
 *      value - (IN) the value will be set to data structure
 * Returns     : SOC_E_XXX
 */
int
_soc_ea_port_info_set(int unit, soc_port_t port, soc_ea_port_control_t type, 
    int value);

/*
 * Function:
 *      _soc_ea_gpio_config_read
 * Description:
 *      Read the gpio configuration
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
_soc_ea_gpio_config_read(int unit, uint32 flags, uint32 *mask);

void 
soc_ea_dbg_level_set(uint32 lvl);

void 
soc_ea_dbg_level_dump(void);

uint16 
soc_ea_firmware_ver_get(int unit);

uint16 
soc_ea_jedecid_get(int unit);

uint16 
soc_ea_chip_id_get(int unit);

uint32 
soc_ea_chip_rev_get(int unit);

uint8* 
soc_ea_extend_id_get(int unit);

/*
 * Function:
 *      soc_gpio_write
 * Description:
 *      Write a GPIO value belonged to the bank, only the bits masked by the mask will be 
 *      write to the gpio register.
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 *      data - (IN) Value to be write to the GPIO
 * Returns     : SOC_E_XXX
 */
int
soc_gpio_write(int unit, uint32 flags, uint32 mask, uint32 data);

/*
 * Function:
 *      soc_gpio_read
 * Description:
 *      Read a GPIO value belonged to the bank
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      data - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
soc_gpio_read(int unit, uint32 flags, uint32 *data);

/*
 * Function:
 *      soc_gpio_config_write
 * Description:
 *      Set Bitmask of GPIO ports for which to generate alert.  Allow the host to be notified 
 *      of changes at the GPIO ports of an ONU through alerts
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (IN) Bits mask
 * Returns     : SOC_E_XXX
 */
int
soc_gpio_config_write(int unit, uint32 flags, uint32 mask);

/*
 * Function:
 *      soc_gpio_config_read
 * Description:
 *      Get Bitmask of GPIO ports for which to generate alert.  Allow the host to be notified 
 *      of changes at the GPIO ports of an ONU through alerts
 * Parameters:
 *      unit  - (IN) BCM device number
 *      flags - (IN) BANK flag
 *      mask - (OUT) Value read out
 * Returns     : SOC_E_XXX
 */
int 
soc_gpio_config_read(int unit, uint32 flags, uint32 *mask);

/*
 * Function:
 *      soc_base_mac_set
 * Description:
 *      Allow user to change the PON base mac which is used form MPCP registering and OAM 
 *      source MAC address. 
 *      If enable multiple llid feature, the llids' mac address will generated from this mac 
 *      address by +1.
 *      The uni base mac address is used for uni port such as the pause frame which come 
 *      from the uni port will pack this mac as the source mac. uni0 will have this mac while
 *      uni1 use uni_base_mac +1, so do other unis
 * Parameters:
 *      unit  - (IN) BCM device number
 *      mac - (IN) Including the PON base mac and UNI base mac
 * Returns     : SOC_E_XXX
 */
int 
soc_base_mac_set(int unit, soc_base_mac_t mac);

/*
 * Function:
 *      soc_base_mac_get
 * Description:
 *      Allow user to get the PON base mac which is used form MPCP registering and OAM 
 *      source MAC address. 
 *      If enable multiple llid feature, the llids' mac address will generated from this mac 
 *      address by +1.
 *      The uni base mac address is used for uni port such as the pause frame which come 
 *      from the uni port will pack this mac as the source mac. uni0 will have this mac while
 *      uni1 use uni_base_mac +1, so do other unis
 * Parameters:
 *      unit  - (IN) BCM device number
 *      mac - (IN) Including the PON base mac and UNI base mac
 * Returns     : SOC_E_XXX
 */
int 
soc_base_mac_get(int unit, soc_base_mac_t *mac);

/*
 * Function:
 *      soc_chip_tk371x_reset
 * Description:
 *      Reset the EPON chipset by sending OAM message
 * Parameters:
 *      unit  - (IN) BCM device number
 * Returns     : SOC_E_XXX
 */
int 
soc_chip_tk371x_reset(int unit);

/*
 * Function:
 *      soc_nvs_erase
 * Description:
 *      Erase the EPON configuration that stored in the EPON chipset's NVS by 
 *      sending OAM message
 * Parameters:
 *      unit  - (IN) BCM device number
 * Returns     : SOC_E_XXX
 */
int
soc_nvs_erase(int unit);

/*
 * Function:
 *      soc_auth_result_set
 * Description:
 *      Commit the LOID authentication result to EPON chipset
 * Parameters:
 *      unit  - (IN) BCM device number
 *      state - (IN) TRUE means fail while FALSE means success
 * Returns     : SOC_E_XXX
 */
int
soc_auth_result_set(int unit, int state);

/*
 * Function:
 *      soc_auth_result_get
 * Description:
 *      Get the LOID authentication result from EPON chipset
 * Parameters:
 *      unit  - (IN) BCM device number
 *      state - (IN) TRUE means fail while FALSE means success
 * Returns     : SOC_E_XXX
 */
int 
soc_auth_result_get(int unit, int *state);

int 
soc_ea_firmware_upgrade(int unit, int type, int length, uint8* buf);


#endif /* _BCM_EA_ONU_H */
