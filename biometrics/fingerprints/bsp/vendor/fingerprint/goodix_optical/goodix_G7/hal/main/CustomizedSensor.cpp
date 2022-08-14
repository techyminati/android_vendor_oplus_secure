/************************************************************************************
 ** File: - CustomizedSensor.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix(android Q)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,15/08/2019
 ** Author: Bangxiong.Wu@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Bangxiong.Wu       2019/10/12       create file
 **  Bangxiong.Wu       2019/10/12       add lcdtype CC161_SAMSUNG for SM8250
 ************************************************************************************/

#define LOG_TAG "[GF_HAL][CustomizedSensor]"

#include "HalContext.h"
#include "CustomizedSensor.h"
#include "HalLog.h"
#include "CustomizedSensorConfigProvider.h"
#include "CustomizedHalConfig.h"

namespace goodix {

    CustomizedSensor::CustomizedSensor(HalContext *context) :
        DelmarSensor(context, new CustomizedSensorConfigProvider()),
        mProductScreenId(SCREEN_TYPE_BD187_SAMSUNG_ID) {
        memset(&mCustomizedConfig, 0, sizeof(gf_customized_config_t));
    }

    CustomizedSensor::~CustomizedSensor() {
    }

    gf_error_t CustomizedSensor::createInitCmd(gf_delmar_sensor_init_t** cmd, int32_t* size) {
        gf_error_t err = GF_SUCCESS;
        gf_customized_sensor_init_t* ptr = NULL;
        FUNC_ENTER();

        do {
            if (cmd == NULL || size == NULL) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            ptr = (gf_customized_sensor_init_t*)malloc(sizeof(gf_customized_sensor_init_t));
            if (ptr != NULL) {
                *size = sizeof(gf_customized_sensor_init_t);
                memset(ptr, 0, *size);
                if (checkScreenType((char *)SCREEN_TYPE_AD097_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AD097_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AD097_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AD097_BOE)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AD097_BOE_ID;
                    mProductScreenId = SCREEN_TYPE_AD097_BOE_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_BD187_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_BD187_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_BD187_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_CC151_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_CC151_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_CC151_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_CC161_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_CC161_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_CC161_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_DD306_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_DD306_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_DD306_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AD119_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AD119_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AD119_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AE009_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AE009_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AE009_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AE009_BOE)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AE009_BOE_ID;
                    mProductScreenId = SCREEN_TYPE_AE009_BOE_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_RA352_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_RA352_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_RA352_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AA200_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AA200_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AA200_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AA202_BOE)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AA202_BOE_ID;
                    mProductScreenId = SCREEN_TYPE_AA202_BOE_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AA262_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AA262_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AA262_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_FERRIRA_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_FERRIRA_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_FERRIRA_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AA437_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AA437_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AA437_SAMSUNG_ID;
                } else if (checkScreenType((char *)SCREEN_TYPE_AA439_SAMSUNG)) {
                    ptr->i_product_screen_id = SCREEN_TYPE_AA439_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_AA439_SAMSUNG_ID;
                } else {
                    ptr->i_product_screen_id = SCREEN_TYPE_BD187_SAMSUNG_ID;
                    mProductScreenId = SCREEN_TYPE_BD187_SAMSUNG_ID;
                }
                *cmd = (gf_delmar_sensor_init_t*)ptr;
            } else {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            LOG_D(LOG_TAG, "[%s] mProductScreenId=%d", __func__, mProductScreenId);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    int32_t CustomizedSensor::detectTemperature() {
        int32_t temperature = 0;
        return temperature;
    }

    /*terminal state. Indicates whether the device is root*/
    bool CustomizedSensor::isDeviceUnlocked(void) {
        return isTerminalUnlocked();
    }

    gf_error_t CustomizedSensor::handleInitResult(gf_delmar_sensor_init_t* cmd) {
        gf_error_t err = GF_SUCCESS;
        gf_customized_sensor_init_t* ptr = NULL;
        FUNC_ENTER();
        do {
            if (cmd == NULL) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            ptr = (gf_customized_sensor_init_t *) cmd;
            memcpy(&mCustomizedConfig, &ptr->o_customized_config, sizeof(gf_customized_config_t));
            LOG_D(LOG_TAG, "[%s] optical_type=%d", __func__, mCustomizedConfig.customized_optical_type);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
