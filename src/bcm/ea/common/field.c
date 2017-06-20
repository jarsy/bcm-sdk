/*
 * $Id: field.c,v 1.16 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     field.c
 * Purpose:
 *
 */
#include <soc/feature.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <bcm_int/ea/field.h>
#include <bcm_int/control.h>
#include <bcm_int/common/field.h>
#include <bcm/field.h>
#include <bcm/error.h>

#ifdef BCM_FIELD_SUPPORT
#define OPERATOR_TRUE		1
#define OPERATOR_FLASE		0

STATIC int
_bcm_ea_field_operator_equal(
		uint8 *data, uint8 *mask, int len){
	int i = 0;
	int iflag = 0;
	int mflag = 0;
	
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_ea_field_operator_equal\n")));
	while (i < len){
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META("mask[%d]=%02X, data[%d]=%02X\n"),
                       i, mask[i], i, data[i]));
		if (mask[i] == data[i]){
			iflag++;
		}
		if (mask[i] == 0){
			mflag++;
		}
		i++;
	}
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("iflag=%d, len=%d\n"),
                   iflag, len));
	if ((iflag == len) && (mflag != len)){
		return OPERATOR_TRUE;
	}
	return OPERATOR_FLASE;
}

STATIC int
_bcm_ea_field_operator_equal_v2(
		uint8 *data, uint8 *mask, int len){
	int i = 0;
	int mflag;

        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_ea_field_operator_equal_v2\n")));
	mflag = 0;
	while (i < len){
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META("mask[%d]=%02X, data[%d]=%02X\n"),
                       i, mask[i], i, data[i]));
		if (mask[i] == 0xff){
			mflag++;
		}
		i++;
	}
	if (mflag == len){
		return OPERATOR_TRUE;
	}
	return OPERATOR_FLASE;
}

STATIC int
_bcm_ea_field_operator_nevermatch(
		uint8 *data, uint8 *mask, int len){
	int i = 0;
	int dflag = 0;
	int mflag = 0;  
	int fflag = 0;
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_ea_field_operator_nevermatch\n")));
	while (i < len){
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META("mask[%d]=%02X, data[%d]=%02X\n"),
                       i, mask[i], i, data[i]));
		if (data[i] != 0){
			dflag = 1;
		}         
		if (data[i] == 0xff){
			fflag++;
		}
		if (mask[i] == 0){
			mflag++;
		}
		i++;
	}
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("mflag=%d, dflag=%d, len=%d\n"),
                   mflag, dflag, len));
	if (fflag == len && mflag == len){
		 return OPERATOR_FLASE;
	}
	if (dflag && mflag == len){
		return OPERATOR_TRUE;
	}
	return OPERATOR_FLASE;
}

#if 0
STATIC int
_bcm_ea_field_operator_notequal(
		uint8 *data, uint8 *mask, int len){
	int i = 0;
	int iflag = 0;

        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_ea_field_operator_notequal\n")));
	while (i < len){
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META("mask[%d]=%02X, data[%d]=%02X\n"),
                       i, mask[i], i, data[i]));
		if ((uint8)data[i] == (uint8)(~mask[i])){
			iflag++;
		}
		i++;
	}
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("iflag=%d, len=%d\n"),
                   iflag, len));
	if (iflag == len){
		return OPERATOR_TRUE;
	}
	return OPERATOR_FLASE;
}
#endif

STATIC int
_bcm_ea_field_operator_notequal_v2(
		uint8 *data, uint8 *mask, int len){
	int i = 0;
	int mflag = 0;

        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_ea_field_operator_notequal_v2\n")));
	while (i < len){
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META("mask[%d]=%02X, data[%d]=%02X\n"),
                       i, mask[i], i, data[i]));
		if ((uint8)mask[i] == 0x55){
			mflag++;
		}
		i++;
	}
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("mflag=%d, len=%d\n"), mflag, len));
	if (mflag == len){
		return OPERATOR_TRUE;
	}
	return OPERATOR_FLASE;
}

STATIC int
_bcm_ea_field_operator_alwaysmatch(
		uint8 *data, uint8 *mask, int len){
	int i = 0;
	int dflag = 0;
	int mflag = 0;

        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("_bcm_ea_field_operator_alwaysmatch\n")));
	while (i < len) {
            LOG_DEBUG(BSL_LS_BCM_FP,
                      (BSL_META("mask[%d]=%02X, data[%d]=%02X\n"),
                       i, mask[i], i, data[i]));
		if (data[i] == 0){
			dflag++;
		}
		if (mask[i] == 0){
			mflag++;
		}
		i++;
	}
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META("mflag=%d, dflag=%d, len=%d\n"),
                   mflag, dflag, len));
	if (dflag == len && mflag == len){
		return OPERATOR_TRUE;
	}
	return OPERATOR_FLASE;
}


int
_bcm_ea_field_operator_cond(uint8 *data, uint8 *mask, int len){
	if(OPERATOR_TRUE ==
			_bcm_ea_field_operator_notequal_v2(data, mask, len)){
		return _BCM_EA_FIELD_MATCH_NOTEQUAL;
	}else if (OPERATOR_TRUE ==
			_bcm_ea_field_operator_nevermatch(data, mask, len)){
		return _BCM_EA_FIELD_MATCH_NEVER_MATCH;
	}else if (OPERATOR_TRUE ==
			_bcm_ea_field_operator_equal(data, mask, len)){
		return _BCM_EA_FIELD_MATCH_EQUAL;
	}else if (OPERATOR_TRUE ==
			_bcm_ea_field_operator_alwaysmatch(data, mask, len)){
		return _BCM_EA_FIELD_MATCH_ALWAYSMATCH;
	}else if (OPERATOR_TRUE ==
			_bcm_ea_field_operator_equal_v2(data, mask, len)){
		return _BCM_EA_FIELD_MATCH_EQUAL;
	}else{
		return _BCM_EA_FIELD_MATCH_ERROR;
	}
}

int
_bcm_ea_field_operator_cond_mask_get(
		uint8 match, uint8 *data, uint8 *mask, int len){
	int i = 0;

	switch (match){
		case _BCM_EA_FIELD_MATCH_NOTEQUAL:
			while (i < len){
				mask[i] = 0x55;
				i++;
			}
			break;
		case _BCM_EA_FIELD_MATCH_ALWAYSMATCH:
		case _BCM_EA_FIELD_MATCH_NEVER_MATCH:
			while (i < len){
				mask[i] = 0x00;
				i++;
			}
			break;
		case _BCM_EA_FIELD_MATCH_EQUAL:
			while (i < len){
				mask[i] = 0xff;
				i++;
			}
			break;
		default:
			return BCM_E_INTERNAL;
	}
	return BCM_E_NONE;
}
#endif

