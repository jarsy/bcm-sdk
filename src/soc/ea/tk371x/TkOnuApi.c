/*
 * $Id: TkOnuApi.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOnuApi.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsUtil.h>
#include <soc/ea/tk371x/TkOamMem.h>
#include <soc/ea/tk371x/Oam.h>
#include <soc/ea/tk371x/OamUtils.h>
#include <soc/ea/tk371x/TkOnuApi.h>
#include <soc/ea/tk371x/TkDebug.h>
#include <soc/ea/tk371x/OamUtilsCtc.h>

#define MAX_MSG_LENGTH              255

/*
 *  Function:
 *      TkExtOamOnuInfo
 * Purpose:
 *      Get the extended proprietary ONU information of the EPON MAC
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pRXBuf    - The ONU information
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamOnuInfo(uint8 pathId, uint8 LinkId, OamTkInfo * pRxBuf)
{
    OamTkExt       *tk = NULL;
    OamVarDesc     *pVarDesc = NULL;    /* Variable Descriptor */
    uint32          size,
                    RespLen;
    int32           DataSize;
    uint8          *pMsgBuf = NULL;
    uint16          flags = 0x0050;

    if ((LinkId > 7) || (NULL == pRxBuf)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (uint8 *) TkOamMemGet(pathId);  /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, pMsgBuf);
    tk->opcode = OamExtOpInfo;  /* 00 */

    /*
     * Variable Descriptor 
     */
    pVarDesc = (OamVarDesc *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pVarDesc->branch = 0;
    pVarDesc->leaf = soc_htons(0);

    size = sizeof(OamOuiVendorExt) + sizeof(OamTkExt);
    if (OK ==
        TkOamRequest(pathId, LinkId,
                     (OamFrame *) pMsgBuf,
                     size + 2, (OamPdu *) pMsgBuf, &RespLen)) {
        DataSize = sizeof(OamTkInfo);
        bcopy((int8 *) INT_TO_PTR(PTR_TO_INT(pMsgBuf) +
                        sizeof(OamOuiVendorExt)), (int8 *) pRxBuf,
              DataSize);
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */

        pRxBuf->firmwareVersion = soc_ntohs(pRxBuf->firmwareVersion);
        pRxBuf->oamId.product = soc_ntohs(pRxBuf->oamId.product);
        pRxBuf->oamId.version = soc_ntohs(pRxBuf->oamId.version);
        pRxBuf->upBuffer = soc_ntohs(pRxBuf->upBuffer);
        pRxBuf->dnBuffer = soc_ntohs(pRxBuf->dnBuffer);
        pRxBuf->jedecId = soc_ntohs(pRxBuf->jedecId);
        pRxBuf->chipId = soc_ntohs(pRxBuf->chipId);
        pRxBuf->chipVersion = soc_ntohl(pRxBuf->chipVersion);
        return OK;
    } else {
        TkOamMemPut(pathId,(void *) pMsgBuf);  /* free memory */
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * Function:
 *      TkExtOamGetFirmwareVer
 * Purpose:
 *      Get the ONU firmware version.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      FirmwareVer - Firmware version
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetFirmwareVer(uint8 pathId, uint8 LinkId, uint16 * FirmwareVer)
{
    uint32          DataLen;
    uint16          temp;
    int32           ret = OK;

    if ((LinkId > 7) || (NULL == FirmwareVer)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamGet(pathId, LinkId,
                    OamBranchAttribute,
                    OamExtAttrFirmwareVer, (uint8 *) & temp, &DataLen);

    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        *FirmwareVer = soc_ntohs(temp);
        return OK;
    }
}


/*
 * Function:
 *      TkExtOamGetVendorID
 * Purpose:
 *      Get the ONU Vendor ID.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pVendorID    - Vendor Id.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetVendorID(uint8 pathId, uint8 LinkId, uint8 * pVendorID)
{
    uint32          DataLen;
    OamObjIndex     index;

    if ((LinkId > 7) || (NULL == pVendorID)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = 1;

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrExtendedId, pVendorID, &DataLen))
        return (OK);
    else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * Function:
 *      TkExtOamGetJedecID
 * Purpose:
 *      Get the ONU jedec ID.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pJedecID    - JEDEC ID
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetJedecID(uint8 pathId, uint8 LinkId, uint16 * pJedecID)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint16          temp;

    if ((LinkId > 7) || (NULL == pJedecID)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = 1;

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrJedecId, (uint8 *) & temp, &DataLen)) {
        *pJedecID = soc_ntohs(temp);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * Function:
 *      TkExtOamGetChipID
 * Purpose:
 *      Get the ONU chip ID.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pChipID -   Chip Id
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      This value is read from the EPON MAC register.
 */
int32
TkExtOamGetChipID(uint8 pathId, uint8 LinkId, uint16 * pChipID)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint16          temp;

    if ((LinkId > 7) || (NULL == pChipID)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = 1;

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrChipId, (uint8 *) & temp, &DataLen)) {
        *pChipID = soc_ntohs(temp);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * Function:
 *      TkExtOamGetChipRev
 * Purpose:
 *      Get the chip revision from the ONU chipset.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pChipRev   - The chip revision information.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetChipRev(uint8 pathId, uint8 LinkId, uint32 * pChipRev)
{
    uint32          DataLen;
    OamObjIndex     index;
    uint32          temp;

    if ((LinkId > 7) || (NULL == pChipRev)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = 1;

    if (OK ==
        TkExtOamObjGet(pathId, LinkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrChipVersion, (uint8 *) & temp,
                       &DataLen)) {
        *pChipRev = soc_ntohl(temp);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}

/*
 * Function:
 *      TkExtOamSetResetOnu
 * Purpose:
 *      Reset the EPON chipset.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetResetOnu(uint8 pathId, uint8 LinkId)
{
    OamTkExt       *tk = NULL;
    OamVarContainer *pVarCont = NULL;   /* Variable Container */
    uint32          size;
    char           *pMsgBuf = NULL;
    uint16          flags = 0x0050;
    int32           ret = OK;

    if (LinkId > 7) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pMsgBuf = (char *) TkOamMemGet(pathId);   /* get memory */
    if (NULL == pMsgBuf) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    tk = (OamTkExt *) OamFillExtHeader(flags, &TeknovusOui, (uint8 *)
                                       pMsgBuf);
    tk->opcode = OamExtOpVarSet;    /* 03 */

    /*
     * Variable Container 
     */
    pVarCont = (OamVarContainer *) INT_TO_PTR(PTR_TO_INT(tk) + 1);
    pVarCont->branch = OamBranchAction; /* branch (09) */
    pVarCont->leaf = soc_htons(OamExtActResetOnu);  /* leaf */
    pVarCont->length = 0x80;

    size =
        sizeof(OamOuiVendorExt) + sizeof(OamTkExt) + OamContSize(pVarCont);
    if (OK !=
        TkOamNoResRequest(pathId, LinkId, (OamFrame *) pMsgBuf, size)) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        ret = OK;
    }

    TkOamMemPut(pathId,(void *) pMsgBuf);

    return ret;
}


/*
 * Function:
 *      TkExtOamSetEraseNvs
 * Purpose:
 *      Erase the ONU NVS
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message    
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetEraseNvs(uint8 pathId, uint8 LinkId)
{
    uint8           ret = OamVarErrNoError;

    if (LinkId > 7)
        return (ERROR);

    ret =
        TkExtOamSet(pathId, LinkId,
                    OamBranchAction, OamExtActEraseNvs, NULL, 0);

    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        return OK;
    }
}

#define TkExtOamSetGpioHigh( pathid, linkid, gpioMask ) \
            TkExtOamSetGpio( pathid, linkid, gpioMask, gpioMask )
#define TkExtOamSetGpioLow(  pathid, linkid, gpioMask ) \
            TkExtOamSetGpio( pathid, linkid, gpioMask, 0 )

/*
 * Function:
 *      TkExtOamSetGpio
 * Purpose:
 *      Set the GPIO values
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      GpioMask    - GPIO pin number indicated by bit number
 *      GpioVal    - GPIO value
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      This API allows customers to program output GPIO pin to control the external 
 *      hardware.
 */
int32
TkExtOamSetGpio(uint8 pathId, uint8 LinkId, uint32 GpioMask,
                uint32 GpioVal)
{
    OamGpioValue    GpioSet;
    uint8           ret;

    if (LinkId > 7) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    GpioSet.mask = soc_htonl(GpioMask);
    GpioSet.value = soc_htonl(GpioVal);
    ret =
        TkExtOamSet(pathId, LinkId,
                    OamBranchAction,
                    OamExtActSetGpioValue,
                    (uint8 *) & GpioSet, sizeof(OamGpioValue));
    if (ret != OamVarErrNoError)
        return ERROR;

    return (OK);
}


/*
 * Function:
 *      TkExtOamGetGpio
 * Purpose:
 *      Get the GPIO value 
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      gpio    - The GPIO pin vlaue
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetGpio(uint8 pathId, uint8 LinkId, uint32 * gpio)
{
    uint8           buf[8];
    uint32          len;
    uint8           ret;

    if (LinkId > 7 || gpio == NULL) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamGet(pathId, LinkId,
                    OamBranchAction, OamExtActGetGpioValue, buf, &len);
    if (ret != OK || len != 8)
        return ERROR;

    *gpio = soc_ntohl(*(uint32 *) (buf + 4));
    return (OK);
}

/*
 * Function:
 *      TkExtOamSetGpioConfig
 * Purpose:
 *      Set the GPIO configuration values
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      mask    - The configuration value
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      This API allows customers to program 
 */
int32
TkExtOamSetGpioConfig(uint8 pathId, uint8 LinkId, uint32 mask)
{   
    uint32  buf[8];
    uint8   ret;

    if (LinkId > 7) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    buf[0] = soc_htonl(mask);

    ret = TkExtOamSet(pathId, LinkId,OamBranchAction, OamExtActSetGpioConfig,
        (uint8 *)buf, sizeof(uint32));
    
    if (ret != OamVarErrNoError)
        return ERROR;

    return (OK);
}


/*
 * Function:
 *      TkExtOamGetGpioConfig
 * Purpose:
 *      Get the GPIO configuration mask value 
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      mask    - The GPIO configuration vlaue
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetGpioConfig(uint8 pathId, uint8 LinkId, uint32 * mask)
{
    uint32          buf[8];
    uint32          len;
    uint8           ret;

    if (LinkId > 7 || mask == NULL) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret = TkExtOamGet(pathId, LinkId,OamBranchAction, OamExtActGetGpioConfig, 
        (uint8 *)buf, &len);
    
    if (ret != OK || len != sizeof(uint32))
        return ERROR;

    *mask = soc_ntohl(buf[0]);
    return (OK);
}


/*
 * Function:
 *      TkExtOamGetLoadInfo
 * Purpose:
 *      Get ONU load information.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pLoadInfo    - The load information.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      Get the load information of the EPON MAC including the version and checksum of 
 *      personality, bootloader and firmware.
 */
int32
TkExtOamGetLoadInfo(uint8 pathId, uint8 LinkId, OamLoadInfo * pLoadInfo)
{
    uint32          DataLen;
    int32           ret = OK;

    if ((LinkId > 7) || (NULL == pLoadInfo)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamGet(pathId, LinkId,
                    OamBranchAction,
                    OamExtActGetLoadInfo, (uint8 *) pLoadInfo, &DataLen);

    if (OK == ret) {
        pLoadInfo->bootVer = soc_ntohs(pLoadInfo->bootVer);
        /* < Boot version */
        pLoadInfo->bootCrc = soc_ntohl(pLoadInfo->bootCrc);
        /* < Boot crc 32 */
        pLoadInfo->persVer = soc_ntohs(pLoadInfo->persVer);
        /* < Personality version */
        pLoadInfo->persCrc = soc_ntohl(pLoadInfo->persCrc);
        /* < Personality crc 32 */
        pLoadInfo->app0Ver = soc_ntohs(pLoadInfo->app0Ver);
        /* < App0 version */
        pLoadInfo->app0Crc = soc_ntohl(pLoadInfo->app0Crc);
        /* < App0 crc 32 */
        pLoadInfo->app1Ver = soc_ntohs(pLoadInfo->app1Ver);
        /* < App1 version */
        pLoadInfo->app1Crc = soc_ntohl(pLoadInfo->app1Crc);
        /* < App1 crc 32 */
        pLoadInfo->diagVer = soc_ntohs(pLoadInfo->diagVer);
        /* < Diagnostic version */
        pLoadInfo->diagCrc = soc_ntohl(pLoadInfo->diagCrc);
        /* < Diagnostic crc 32 */
    } else {
        TkDbgTrace(TkDbgErrorEnable);
    }

    return ret;
}


/*
 * Function:
 *      TkExtOamGetFECMode
 * Purpose:
 *      Get the FEC mode
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pFECMode    - The fec mode value.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetFECMode(uint8 pathId, uint8 LinkId, OamExtFECMode * pFECMode)
{
    uint32          DataLen;

    if ((LinkId > 7) || (NULL == pFECMode)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (OK ==
        TkExtOamGet(pathId, LinkId,
                    OamBranchAttribute,
                    OamExtAttrFECMode, (uint8 *) pFECMode, &DataLen))
        return (OK);
    else {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }
}


/*
 * Function:
 *      TkExtOamSetFECMode
 * Purpose:
 *      Set the FEC mode vlaue.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pFECMode    - The FEC mode value.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetFECMode(uint8 pathId, uint8 LinkId, OamExtFECMode * pFECMode)
{
    uint8           ret = OamVarErrNoError;

    if ((LinkId > 7) || (NULL == pFECMode)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamSet(pathId, LinkId,
                    OamBranchAttribute,
                    OamExtAttrFECMode,
                    (uint8 *) pFECMode, sizeof(OamExtFECMode));

    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        return OK;
    }
}


/*
 * Function:
 *      TkExtOamSetEponAdmin
 * Purpose:
 *      Set the PON port admin state.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      EponCtl    - Admin state, 0 for Enable while 1 for disable.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetEponAdmin(uint8 pathId, uint8 LinkId, uint8 EponCtl)
{
    uint8           ret = OamVarErrNoError;

    if ((LinkId > 7)
        || ((EponAdminEnable != EponCtl)
            && (EponAdminDisable != EponCtl))) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamSet(pathId, LinkId,
                    OamBranchAttribute,
                    OamExtAttrEponPortAdmin, (uint8 *) & EponCtl, 1);
    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamGetEponAdmin
 * Purpose:
 *      Get the PON port admin state.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      EponCtl    - The Admin state, 0 for enable while 1 for disable.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetEponAdmin(uint8 pathId, uint8 LinkId, uint8 * EponCtl)
{
    uint32          size = 0;

    if (NULL == EponCtl || LinkId > 7) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    return TkExtOamGet(pathId, LinkId,
                       OamBranchAttribute,
                       OamExtAttrEponPortAdmin, EponCtl, &size);
}

/*
 * Function:
 *      TkExtOamGetOnuLlid
 * Purpose:
 *      Get the LLID of the logic link.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      llid     - The LLID value
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetOnuLlid(uint8 pathId, uint8 linkId, uint16 * llid)
{
    uint32          size;
    uint8           ret;

    OamEponPortInfo eponPortInfo;

    /* Only a base link id of 0 is supported for this interface*/
    if ((linkId > SDK_MAX_NUM_OF_LINK)
        || (llid == NULL)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
    ret =
        TkExtOamGet(pathId, linkId,
                    OamBranchAttribute,
                    OamExtAttrLinkInfo, (uint8 *) & eponPortInfo, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    *llid = soc_ntohs(eponPortInfo.LinkInfo[linkId].PhyLLID);

    return OK;
}

/*
 * Function:
 *      TkExtOamSetEponBaseMac
 * Purpose:
 *      Set the ONU base MAC
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      baseMacAddr    - EPON base MAC and user MAC
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetEponBaseMac(uint8 pathId, OamEponBaseMac * baseMacAddr)
{
    uint8           ret = OamVarErrNoError;

    if (NULL == baseMacAddr) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamSet(pathId, 0, OamBranchAction,
                    OamExtActBaseMac, (uint8 *) baseMacAddr, 6 * 2);

    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamGetEponPortInfo
 * Purpose:
 *      Get the PON port information.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      EponPortInfo    - The PON port and associated logic link information.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetEponPortInfo(uint8 pathId,
                        uint8 linkId, OamEponPortInfo * EponPortInfo)
{
    uint8           ret;
    uint32          size;
    uint8           index;

    if (NULL == EponPortInfo) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamGet(pathId, linkId,
                    OamBranchAttribute,
                    OamExtAttrLinkInfo, (uint8 *) EponPortInfo, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (ret == OK) {
        for (index = 0; index <= SDK_MAX_NUM_OF_LINK; index++) {
            EponPortInfo->LinkInfo[index].PhyLLID =
                soc_ntohs(EponPortInfo->LinkInfo[index].PhyLLID);
        }
    }

    return ret;
}

/*
 * Function:
 *      TkExtOamSetBatteryBackup
 * Purpose:
 *      Set the EPON chipset battery back state
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state    - State, 0 for not supported while 1 means support
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetBatteryBackup(uint8 pathId, uint8 linkId, uint8 state)
{
    uint8           ret = OamVarErrNoError;

    if (linkId > SDK_MAX_NUM_OF_LINK || (TRUE != state && FALSE != state)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamSet(pathId, linkId,
                    OamBranchAttribute,
                    OamExtAttrBatteryBackupCap, &state, sizeof(uint8));
    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamGetBatteryBackup
 * Purpose:
 *      Get the battery backup sate
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state    - The battery backup state, 0 for not support while 1 means support.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetBatteryBackup(uint8 pathId, uint8 linkId, uint8 * state)
{
    uint8           ret;
    uint32          size;
    uint8           buff[MAX_MSG_LENGTH] = { 0X0 };

    if (linkId > SDK_MAX_NUM_OF_LINK || (NULL == state)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    ret =
        TkExtOamGet(pathId, linkId,
                    OamBranchAttribute,
                    OamExtAttrBatteryBackupCap, (uint8 *) buff, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    *state = buff[0];

    return OK;
}

/*
 * Function:
 *      TkExtOamGetCtcOamVersion
 * Purpose:
 *      Get the OAM version which is running in the EPON chipset
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      version    - Verison information.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetCtcOamVersion(uint8 pathId, OamVersion * version)
{
    uint8           ret = OamVarErrNoError;
    uint32          size;
    uint8           buff[MAX_MSG_LENGTH] = { 0X0 };

    ret =
        TkExtOamGet(pathId, 0,
                    OamBranchAttribute,
                    OamExtAttrCtcOamVerion, (uint8 *) buff, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    sal_memcpy(version, buff, sizeof(OamVersion));

    version->TKOAMVer = soc_ntohs(version->TKOAMVer);
    version->CTCOAMVer = soc_ntohs(version->CTCOAMVer);

    return OK;
}

/*
 * Function:
 *      TkExtOamSetLaserOn
 * Purpose:
 *      Set the laser always on or on/off based on grant start time of the PON port.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state    - State. 0 means laser on base grant start time, 1 means laser always on.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetLaserOn(uint8 pathId, EponTxLaserStatus state)
{
    uint8           ret = OK;
    uint8           laserState;

    if (!(EponTxLaserNormal <= state && EponTxLaserDisable >= state)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    laserState = state;

    ret =
        TkExtOamSet(pathId, 0,
                    OamBranchAttribute,
                    OamExtAttrLaserAlwaysOn, &laserState, sizeof(uint8));

    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    } else {
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamGetLaserOn
 * Purpose:
 *      Get the laser always on or on/off based on grant start time of the PON port.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state    - State. 0 means laser on base grant start time, 1 means laser always on.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetLaserOn(uint8 pathId, EponTxLaserStatus * state)
{
    uint8           ret = OK;
    uint8          *pBuff = NULL;
    uint32          size = 0;

    if (NULL == state) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamGet(pathId, 0,
                    OamBranchAttribute,
                    OamExtAttrLaserAlwaysOn, (uint8 *) pBuff, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        *state = pBuff[0];
        ret = OK;
    }
    TkOamMemPut(pathId, pBuff);
    return ret;
}

/*
 * Function:
 *      TkExtOamGetPonLosInfo
 * Purpose:
 *      Get PON port loss state
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state    - The state, 1 for loss while 0 for normal
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetPonLosInfo(uint8 pathId, uint8 * state)
{
    uint8           ret = OK;
    uint8          *pBuff = NULL;
    uint32          size = 0;

    if (NULL == state) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamGet(pathId, 0, OamBranchAction,
                    OamExtActPonLosInfo, (uint8 *) pBuff, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        *state = pBuff[0];
        ret = OK;
    }
    TkOamMemPut(pathId, pBuff);
    return ret;
}

/*
 * Function:
 *      TkExtOamGetPonRegStatus
 * Purpose:
 *      Get the EPON port resiger information including assocaited logic link.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      state    - The information including register state and all logic links' LLID value.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetPonRegStatus(uint8 pathId, uint8 linkId, TkEponRegState * state)
{
    uint8           ret = OK;
    uint8          *pBuff = NULL;
    uint32          size = 0;

    if (NULL == state) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamGet(pathId, linkId,
                    OamBranchAttribute,
                    OamExtAttrEponRegState, (uint8 *) pBuff, &size);
    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        sal_memcpy(state, pBuff, sizeof(TkEponRegState));
        state->ONU_LLID = soc_ntohs(state->ONU_LLID);
        ret = OK;
    }
    TkOamMemPut(pathId, pBuff);
    return ret;
}

/*
 * Function:
 *      TkExtOamGetEncryptKeyExpiryTime
 * Purpose:
 *      Get the timeout value of the encryption keys.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      time    - The timeout value for the encryption keys. 0 for disable encryption.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetEncryptKeyExpiryTime(uint8 pathId, uint8 linkId, uint16 * time)
{
    uint32          dataLen;
    OamObjIndex     index;

    if (linkId > 7) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    index.linkId = linkId;
    if (OK ==
        TkExtOamObjGet(pathId, linkId,
                       OamNameMacName, &index,
                       OamBranchAttribute,
                       OamExtAttrEncryptKeyExpiryTime,
                       (uint8 *) time, &dataLen)) {
        *time = soc_ntohs(*time);
        return (OK);
    } else {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }
}

/*
 * Function:
 *      TkExtOamSetEncryptKeyExpiryTime
 * Purpose:
 *      Set the timeout value of the encryption keys.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      time    - The timeout value for the encryption keys. 0 for disable encryption.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamSetEncryptKeyExpiryTime(uint8 pathId, uint8 linkId, uint16 time)
{
    uint16          buf;
    OamObjIndex     index;
    uint8           ret;
    
    if (linkId > 7) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    index.linkId = linkId;
    buf = soc_ntohs(time);

    ret = TkExtOamObjSet(pathId, linkId, OamNameMacName, &index, 
        OamBranchAttribute, OamExtAttrEncryptKeyExpiryTime, (uint8 *)&buf, 
        sizeof(uint16));

    if(ret != OamVarErrNoError){
        return ERROR;
    }else{
        return OK;
    }
}


/*
 * Function:
 *      TkExtOamSetVlanToLLIDMapping
 * Purpose:
 *      Set Vlan to LLID mapping.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      vlanMappingInfo - Mapping information.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      This feature only provided in the 10G FPAG board.
 */
int32
TkExtOamSetVlanToLLIDMapping(uint8 pathId,
                             uint8 linkId,
                             uint32 portId,
                             TkVlanToLLIDMapping * vlanMappingInfo)
{
    uint8           ret = OK;
    uint8          *pBuff = NULL;
    uint32          size = 0;
    uint32          numOfEntry = 0;
    TkOamVlanToLLIDMapping *pTkOamVlanToLLIDMapping;
    TkOamVlanToLLIDMappingCond *pTkOamVlanToLLIDMappingCond;
    OamObjIndex     index;

    if (NULL == vlanMappingInfo) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = portId;

    pTkOamVlanToLLIDMapping = (TkOamVlanToLLIDMapping *) pBuff;
    pTkOamVlanToLLIDMappingCond = pTkOamVlanToLLIDMapping->cond;

    pTkOamVlanToLLIDMapping->matchMode = vlanMappingInfo->matchMode;
     /**/ pTkOamVlanToLLIDMapping->flags = vlanMappingInfo->flags;
    pTkOamVlanToLLIDMapping->defaultLink = vlanMappingInfo->defaultLink;
    pTkOamVlanToLLIDMapping->defaultQueue = vlanMappingInfo->defaultQueue;
    pTkOamVlanToLLIDMapping->numOfVlanEntry =
        vlanMappingInfo->numOfVlanEntry;

    size =
        sizeof(TkOamVlanToLLIDMapping) -
        sizeof(TkOamVlanToLLIDMappingCond);

    for (numOfEntry = 0;
         numOfEntry < vlanMappingInfo->numOfVlanEntry; numOfEntry++) {
        pTkOamVlanToLLIDMappingCond[numOfEntry].vlan =
            soc_htons(vlanMappingInfo->cond[numOfEntry].vlan);
        pTkOamVlanToLLIDMappingCond[numOfEntry].link =
            vlanMappingInfo->cond[numOfEntry].link;
        pTkOamVlanToLLIDMappingCond[numOfEntry].queue =
            vlanMappingInfo->cond[numOfEntry].queue;

        size += sizeof(TkOamVlanToLLIDMappingCond);
    }

    ret =
        TkExtOamObjSet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrVlanDest, (uint8 *) pBuff, size);

    if (ret != OamVarErrNoError) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        ret = OK;
    }
    TkOamMemPut(pathId, pBuff);
    return ret;
}

/*
 * Function:
 *      TkExtOamGetVlanToLLIDMapping
 * Purpose:
 *      Get the VLAN to LLID mapping information
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      vlanMappingInfo    - The VLAN to LLID mapping information.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      This feature only support in 10G 4701 FPBA board.
 */
int32
TkExtOamGetVlanToLLIDMapping(uint8 pathId,
                             uint8 linkId,
                             uint32 portId,
                             TkVlanToLLIDMapping * vlanMappingInfo)
{
    int32           ret = OK;
    uint8          *pBuff = NULL;
    uint32          numOfEntry = 0;
    TkOamVlanToLLIDMapping *pTkOamVlanToLLIDMapping;
    TkOamVlanToLLIDMappingCond *pTkOamVlanToLLIDMappingCond;
    OamObjIndex     index;
    uint32          DataLen = 0;

    if (NULL == vlanMappingInfo) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = portId;
    pTkOamVlanToLLIDMapping = (TkOamVlanToLLIDMapping *) pBuff;
    pTkOamVlanToLLIDMappingCond = pTkOamVlanToLLIDMapping->cond;

    ret =
        TkExtOamObjGet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrVlanDest, (uint8 *) pBuff, &DataLen);

    if (ret != OK) {
        TkOamMemPut(pathId, pBuff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    vlanMappingInfo->matchMode = pTkOamVlanToLLIDMapping->matchMode;
     /**/ vlanMappingInfo->flags = pTkOamVlanToLLIDMapping->flags;
    vlanMappingInfo->defaultLink = pTkOamVlanToLLIDMapping->defaultLink;
    vlanMappingInfo->defaultQueue = pTkOamVlanToLLIDMapping->defaultQueue;
    vlanMappingInfo->numOfVlanEntry =
        pTkOamVlanToLLIDMapping->numOfVlanEntry;

    for (numOfEntry = 0;
         numOfEntry < pTkOamVlanToLLIDMapping->numOfVlanEntry;
         numOfEntry++) {
        vlanMappingInfo->cond[numOfEntry].vlan =
            soc_ntohs(pTkOamVlanToLLIDMappingCond[numOfEntry].vlan);
        vlanMappingInfo->cond[numOfEntry].link =
            pTkOamVlanToLLIDMappingCond[numOfEntry].link;
        vlanMappingInfo->cond[numOfEntry].queue =
            pTkOamVlanToLLIDMappingCond[numOfEntry].queue;
    }

    TkOamMemPut(pathId, pBuff);
    return ret;
}

/*
 * Function:
 *      TkExtOamSetVlan2LLIDMappingStripTag
 * Purpose:
 *      Set the strip VLAN tag action of VLAN to LLID mapping rule. All the rule share the 
 *      same action. 
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      action    - The action. 0 for keep the packet unchanged, 1 for Drop packets - unuseed
 *                     2 for strip VLAN tag.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      10G 4701 FPGA used only
 */
int32
TkExtOamSetVlan2LLIDMappingStripTag(uint8 pathId, uint8 linkId,
                                    uint8 action)
{
    int32           ret = OK;
    uint8          *pBuff = NULL;
    TkOamVlanToLLIDMapping *pTkOamVlanToLLIDMapping;
    OamObjIndex     index;
    uint32          DataLen = 0;

    if ((StripVlanTag != action)
        && (DropUnTaggedOrUnknown != action)
        && (DefaultAction != action)) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (DropUnTaggedOrUnknown == action) {
        TkDbgPrintf(("This action may make disconnect EPON mac.\n"));
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    index.portId = 0x1;
    pTkOamVlanToLLIDMapping = (TkOamVlanToLLIDMapping *) pBuff;

    ret =
        TkExtOamObjGet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrVlanDest, (uint8 *) pBuff, &DataLen);

    if (ret != OK) {
        TkOamMemPut(pathId, pBuff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    if (0 == pTkOamVlanToLLIDMapping->numOfVlanEntry) {
        TkOamMemPut(pathId, pBuff);
        return OK;
    }

    pTkOamVlanToLLIDMapping->flags = action;

    ret =
        TkExtOamObjSet(pathId, linkId,
                       OamNamePhyName, &index,
                       OamBranchAttribute,
                       OamExtAttrVlanDest, (uint8 *) pBuff, DataLen);

    if (ret != OamVarErrNoError) {
        TkOamMemPut(pathId, pBuff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    TkOamMemPut(pathId, pBuff);
    return OK;
}

/*
 * Function:
 *      TkExtOamGetMLLIDLinkStatus
 * Purpose:
 *      Get multiple link state if multiple link enabled
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      mLLIDLinkStatus    - The multiple logic link inforamtion.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetMLLIDLinkStatus(uint8 pathId,
                           uint8 linkId, TkEponStatus * mLLIDLinkStatus)
{
    int32           retVal;
    uint8          *pBuff = NULL;
    uint32          size = 0;
    OamExtEponStatus *pOamExtEponStatus = NULL;
    uint8           cntOfLLID;

    if ((MAX_NUM_OF_PON_CHIP < pathId) || (SDK_MAX_NUM_OF_LINK < linkId)
        || (NULL == mLLIDLinkStatus)) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pOamExtEponStatus = (OamExtEponStatus *) pBuff;

    retVal =
        TkExtOamGet(pathId, linkId,
                    OamBranchAttribute, OamExtAttrEponRegState, pBuff,
                    &size);

    if (OK != retVal) {
        TkOamMemPut(pathId, pBuff);
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    mLLIDLinkStatus->connection = pOamExtEponStatus->connection;
    mLLIDLinkStatus->rxOptState = pOamExtEponStatus->rxOptState;
    mLLIDLinkStatus->linkNum = pOamExtEponStatus->linkNum;
    sal_memcpy(mLLIDLinkStatus->Olt_MAC_addr, pOamExtEponStatus->Olt_MAC_addr,6);

    for (cntOfLLID = 0; cntOfLLID < pOamExtEponStatus->linkNum;
         cntOfLLID++) {
        mLLIDLinkStatus->linkInfo[cntOfLLID].oamLinkEstablished =
            pOamExtEponStatus->linkInfo[cntOfLLID].oamLinkEstablished;
        mLLIDLinkStatus->linkInfo[cntOfLLID].authorizationState =
            pOamExtEponStatus->linkInfo[cntOfLLID].authorizationState;
        mLLIDLinkStatus->linkInfo[cntOfLLID].loopBack =
            pOamExtEponStatus->linkInfo[cntOfLLID].loopBack;
        mLLIDLinkStatus->linkInfo[cntOfLLID].linkLlid =
            soc_ntohs(pOamExtEponStatus->linkInfo[cntOfLLID].linkLlid);
    }

    TkOamMemPut(pathId, pBuff);
    return OK;
}

/*
 * Function:
 *      TkExtOamSetLaserTxPwrOffTime
 * Purpose:
 *      Set TX power off time for the PON port.
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      powerOffTime    - Tx power off time in units of seconds. 0 means power on all the 
 *                     time, 65535 means power off all the time.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      10G 4701 support only
 */
int32
TkExtOamSetLaserTxPwrOffTime(uint8 pathId, uint16 powerOffTime)
{
    uint8           ret = OamVarErrNoError;

    uint16          cntOfSeconds = soc_ntohs(powerOffTime);

    ret = TkExtOamSet(pathId, 0, OamBranchAction,
                      OamExtActLaserPowerOffTime,
                      (uint8 *) & cntOfSeconds, sizeof(uint16));
    if (OamVarErrNoError != ret) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamSetFailSafeState
 * Purpose:
 *      Set the ONU fail safe mechanism states
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pTkFailSafeState    - Fail safe state.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      Provide the ability to reset the EPON MAC while the critical fail happens. Customer
 *      can enable/disable the state of the designated fail type.
 *      For the current EPON MAC, only one fail type is defined. Enable the MPCP fail type
 *      means if the ONU failed and retried the MPCP registeration to the OLT as the spec 
 *      defined retry time, the EPON MAC will reset automatically and restart the MPCP 
 *      registeration again.
 */
int32
TkExtOamSetFailSafeState(uint8 pathId, TkFailSafeState * pTkFailSafeState)
{
    uint8           ret = OamVarErrNoError;
    OamExtFailsafes *pOamExtFailsafes;
    uint32          size = 0;
    uint8           index;
    uint8          *pBuff = NULL;

    if (NULL == pTkFailSafeState) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    if (CNT_OF_FAIL_SAFE < pTkFailSafeState->failsafeCount) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    pOamExtFailsafes = (OamExtFailsafes *) pBuff;

    pOamExtFailsafes->failsafeCount = pTkFailSafeState->failsafeCount;

    for (index = 0; index < pTkFailSafeState->failsafeCount; index++) {
        pOamExtFailsafes->failsafes[index] =
            pTkFailSafeState->failSafeList[index];
    }

    size = sizeof(OamExtFailsafes) +
        sizeof(uint8) * (pTkFailSafeState->failsafeCount - 1);

    ret =
        TkExtOamSet(pathId, 0,
                    OamBranchAttribute,
                    OamExtAttrOnuFailsafe, (uint8 *) pBuff, size);

    if (OamVarErrNoError != ret) {
        TkOamMemPut(pathId,pBuff);
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    } else {
        TkOamMemPut(pathId,pBuff);
        return OK;
    }
}

/*
 * Function:
 *      TkExtOamGetFailSafeState
 * Purpose:
 *      Get the fail safe state
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      pTkFailSafeState    - The fail safe state.
 * Returns:
 *      ERROR code or OK 
 * Notes:
 *      Provide the ability to reset the EPON MAC while the critical fail happens. Customer
 *      can enable/disable the state of the designated fail type.
 *      For the current EPON MAC, only one fail type is defined. Enable the MPCP fail type
 *      means if the ONU failed and retried the MPCP registeration to the OLT as the spec 
 *      defined retry time, the EPON MAC will reset automatically and restart the MPCP 
 *      registeration again.
 */
int32
TkExtOamGetFailSafeState(uint8 pathId, TkFailSafeState * pTkFailSafeState)
{
    int32           ret = OK;
    OamExtFailsafes *pOamExtFailsafes;
    uint32          size = 0;
    uint8           index;
    uint8          *pBuff = NULL;

    if (NULL == pTkFailSafeState) {
        TkDbgTrace(TkDbgErrorEnable);
        return (ERROR);
    }

    pBuff = (uint8 *) TkOamMemGet(pathId);
    if (NULL == pBuff) {
        TkDbgTrace(TkDbgErrorEnable);
        return ERROR;
    }

    ret =
        TkExtOamGet(pathId, 0,
                    OamBranchAttribute,
                    OamExtAttrOnuFailsafe, (uint8 *) pBuff, &size);

    if (ret != OK) {
        TkDbgTrace(TkDbgErrorEnable);
        ret = ERROR;
    } else {
        pOamExtFailsafes = (OamExtFailsafes *) pBuff;
        if (CNT_OF_FAIL_SAFE > pOamExtFailsafes->failsafeCount) {
            ret = ERROR;
        } else {
            pTkFailSafeState->failsafeCount =
                pOamExtFailsafes->failsafeCount;

            for (index = 0; index < pOamExtFailsafes->failsafeCount;
                 index++) {
                pTkFailSafeState->failSafeList[index] =
                    pOamExtFailsafes->failsafes[index];
            }

            ret = OK;
        }
    }

    TkOamMemPut(pathId, pBuff);
    return ret;
}

/*
 * Function:
 *      TkExtOamSetCTCLoidAuthIfFail
 * Purpose:
 *      Notify EPON chipset authentication result
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      auth      - True faile, FALSE successfull
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32 
TkExtOamSetCTCLoidAuthIfFail(uint8 pathId, uint8 linkId, uint8 auth)
{
    int rv;
    uint8 result;
    
    if(linkId >= SDK_MAX_NUM_OF_LINK || (auth != TRUE && auth != FALSE)){
        return ERROR;
    }

    result = auth;
    rv = TkExtOamSet(pathId, linkId, OamBranchAttribute, 
        OamExtAttrCTCLoidAuthResult, &result, sizeof(result));

    if(OamVarErrNoError != rv){
        rv = ERROR;
    }else{
        rv = OK;
    }

    return rv;
}

/*
 * Function:
 *      TkExtOamGetCTCLoidAuthIfFail
 * Purpose:
 *      Get EPON chipset authentication result
 * Parameters:
 *      pathId    - The EPON chipset unit number.
 *      LinkId    - The link index which will be attached to the message
 *      auth      - True faile, FALSE successfull
 * Returns:
 *      ERROR code or OK 
 * Notes:
 */
int32
TkExtOamGetCTCLoidAuthIfFail(uint8 pathId, uint8 linkId, uint8 *auth)
{
    uint32 data_len;
    uint8 buff[128] = {0x0};
    int rv;

    if(linkId >= SDK_MAX_NUM_OF_LINK || NULL == auth){
        return ERROR;
    }

    rv = TkExtOamGet(pathId, linkId, OamBranchAttribute, 
        OamExtAttrCTCLoidAuthResult, buff, &data_len);

    if(rv != OK){
        return ERROR;
    }else{
        auth[0] = buff[0];
        return OK;
    }
}

