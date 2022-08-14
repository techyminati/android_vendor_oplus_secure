/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][CustomizedFingerprintCore]"

#include "CustomizedFingerprintCore.h"
#include "HalUtils.h"
#include "SZAlgo.h"
#include "HalContext.h"
#include "HalLog.h"

// 0 cycle, 1 ellipse for icon
#define OPTICAL_ENROLLICON  "persist.vendor.fingerprint.optical.enrollicon"
#define OPTICAL_AUTHICON       "persist.vendor.fingerprint.optical.authicon"
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"
#define PROPERTY_SCREEN_TYPE   "persist.vendor.fingerprint.optical.lcdtype"


#define TEMP_POWER_PATH "/sys/class/power_supply/battery/temp"

namespace goodix {
    CustomizedFingerprintCore::CustomizedFingerprintCore(HalContext *context) :
            SZFingerprintCore(context), mUpTime(0), mDownTime(0), mFastUp(false) {
    }

    CustomizedFingerprintCore::~CustomizedFingerprintCore() {
    }

    gf_error_t CustomizedFingerprintCore::init() {
        return SZFingerprintCore::init();
    }

    gf_error_t CustomizedFingerprintCore::onEnrollRequested(const void *hat, uint32_t gid,
            uint32_t timeoutSec) {
        return SZFingerprintCore::onEnrollRequested(hat, gid, timeoutSec);
    }

    gf_error_t CustomizedFingerprintCore::onEnrollUpEvt() {
        return SZFingerprintCore::onEnrollUpEvt();
    }

    gf_error_t CustomizedFingerprintCore::onEnrollStart(EnrollContext *context) {
        return SZFingerprintCore::onEnrollStart(context);
    }

    gf_error_t CustomizedFingerprintCore::onBeforeEnrollCapture(EnrollContext *context) {
        return SZFingerprintCore::onBeforeEnrollCapture(context);
    }

    gf_error_t CustomizedFingerprintCore::onAfterEnrollCapture(EnrollContext *context) {
        return SZFingerprintCore::onAfterEnrollCapture(context);
    }

    gf_error_t CustomizedFingerprintCore::onBeforeEnrollAlgo(EnrollContext *context) {
        gf_sz_algo_enroll_image_t* enroll_cmd = (gf_sz_algo_enroll_image_t*)(context->enroll_cmd);

        getTemp(TEMP_POWER_PATH, &enroll_cmd->i_temp);
        enroll_cmd->i_light = mLight;
        LOG_D(LOG_TAG, "[%s] temp    %d, light %d",  __func__, enroll_cmd->i_temp, enroll_cmd->i_light);
        return SZFingerprintCore::onBeforeEnrollAlgo(context);
    }

    gf_error_t CustomizedFingerprintCore::onAfterEnrollAlgo(EnrollContext *context) {
        return SZFingerprintCore::onAfterEnrollAlgo(context);
    }

    gf_error_t CustomizedFingerprintCore::onEnrollStop(EnrollContext *context) {
        return SZFingerprintCore::onEnrollStop(context);
    }

    void CustomizedFingerprintCore::onEnrollError(EnrollContext *context) {
        SZFingerprintCore::onEnrollError(context);
    }

    gf_error_t CustomizedFingerprintCore::notifyEnrollProgress(EnrollContext *context) {
        return SZFingerprintCore::notifyEnrollProgress(context);
    }

    gf_error_t CustomizedFingerprintCore::prepareAuthRequest() {
        return SZFingerprintCore::prepareAuthRequest();
    }

    gf_error_t CustomizedFingerprintCore::onAuthRequested(uint64_t operationId, uint32_t gid) {
        return SZFingerprintCore::onAuthRequested(operationId, gid);
    }

    gf_error_t CustomizedFingerprintCore::onAuthUpEvt() {
        return SZFingerprintCore::onAuthUpEvt();
    }

    gf_error_t CustomizedFingerprintCore::onAuthStart(AuthenticateContext *context) {
        mTotalTime = HalUtils::getCurrentTimeMicrosecond();
        mMDMTime = 0;
        return SZFingerprintCore::onAuthStart(context);
    }

    gf_error_t CustomizedFingerprintCore::onBeforeAuthCapture(AuthenticateContext *context) {
        gf_error_t err = GF_SUCCESS;
        uint32_t time_cost = 0;
        mDownTime = HalUtils::getCurrentTimeMicrosecond();
        err = SZFingerprintCore::onBeforeAuthCapture(context);
        if (mMDMTime == 0) {
            time_cost = ((uint32_t)(HalUtils::getCurrentTimeMicrosecond() - mTotalTime)) / 1000;
            mTotalTime = HalUtils::getCurrentTimeMicrosecond();
            if (time_cost > 0x3FF) {
               time_cost = 0x3FF;
            }
            mMDMTime = time_cost & 0x3FF;
        }
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAfterAuthCapture(AuthenticateContext *context) {
        int32_t down_to_up_time = 0;
        int32_t expoTime = 0;
        int32_t valid_time = 0;

        do {
            mFastUp = false;
            if (GF_ERROR_TOO_FAST == context->result) {
                LOG_D(LOG_TAG, "[%s] too_fast, %lld, %lld", __func__, (unsigned long long int)mUpTime, (unsigned long long int)mDownTime);
                down_to_up_time = (mUpTime - mDownTime) / 1000;
                if (mDownTime > mUpTime) {  //  ui disappear && up finger before capturing image
                    break;
                }
                expoTime = static_cast<SZAlgo*>(mContext->mAlgo)->mExpoTime;
                valid_time = static_cast<SZAlgo*>(mContext->mAlgo)->mValidTime;
                LOG_D(LOG_TAG, "[%s] down_to_up_time %d expoTime %d valid_time %d", __func__, down_to_up_time, expoTime, valid_time);
                if (context->retry > 0) {   /*retry */
                    break;
                }  else if (down_to_up_time < expoTime + valid_time + 10) {
                    break;
                } else {
                    LOG_D(LOG_TAG, "[%s] right_to_find",  __func__);
                    context->result = GF_SUCCESS;
                    mFastUp = true;
                }
            }
        } while (0);
        mUpTime = 0;
        mDownTime = 0;
        return SZFingerprintCore::onAfterAuthCapture(context);
    }

    gf_error_t CustomizedFingerprintCore::onBeforeAuthAlgo(AuthenticateContext *context) {
        gf_sz_algo_auth_image_t* auth_cmd = (gf_sz_algo_auth_image_t*)(context->auth_cmd);


        getTemp(TEMP_POWER_PATH, &auth_cmd->i_temp);
        auth_cmd->i_light = mLight;
        LOG_D(LOG_TAG, "[%s] temp %d, light %d",  __func__, auth_cmd->i_temp, auth_cmd->i_light);
        return SZFingerprintCore::onBeforeAuthAlgo(context);
    }

    gf_error_t CustomizedFingerprintCore::onAfterAuthAlgo(AuthenticateContext *context) {
        return SZFingerprintCore::onAfterAuthAlgo(context);
    }

    gf_error_t CustomizedFingerprintCore::onAfterAuthRetry(AuthenticateContext *context) {
        notifyMDM(context);
        return SZFingerprintCore::onAfterAuthRetry(context);
    }

    gf_error_t CustomizedFingerprintCore::notifyMDM(AuthenticateContext *context){
        uint32_t time_cost = ((uint32_t)(HalUtils::getCurrentTimeMicrosecond()- mTotalTime)) / 1000;
        gf_error_t err = GF_SUCCESS;
        if (time_cost > 0xFFF) {
           time_cost = 0xFFF;
        }
        mMDMTime = (time_cost << 10) | mMDMTime;
        mMDMTime = (context->retry << 22) | mMDMTime;
        uint32_t error_code = 0;
        if (context->result != 0) {
            error_code = context->result - 1000 > 0xFF ? 0XFF : (uint32_t)(context->result - 1000) & 0xFF;
        }
        mMDMTime = (error_code << 24) | mMDMTime;
        gf_fingerprint_msg_t message;
        memset(&message, 0, sizeof(gf_fingerprint_msg_t));
        if (nullptr != mNotify) {
            LOG_D(LOG_TAG, "[%s] notifyMDM = 0x%x", __func__, mMDMTime);
            message.type = GF_FINGERPRINT_MDM;
            message.data.acquired.acquired_info = (gf_fingerprint_acquired_info_t) mMDMTime;
            mNotify(&message);
        }
        return err;
    }

    gf_error_t CustomizedFingerprintCore::onAfterAuthSuccess(AuthenticateContext *context) {
        return SZFingerprintCore::onAfterAuthSuccess(context);
    }

    gf_error_t CustomizedFingerprintCore::onAuthStop(AuthenticateContext *context) {
        return SZFingerprintCore::onAuthStop(context);
    }

    void CustomizedFingerprintCore::onAuthError(AuthenticateContext *context) {
        if (context->retry == 0 && mFastUp) {
            LOG_E(LOG_TAG, "[%s] auth_fail", __func__);
            context->result = GF_ERROR_TOO_FAST;
        }
        return SZFingerprintCore::onAuthError(context);
    }

    gf_error_t CustomizedFingerprintCore::notifyAuthSuccess(AuthenticateContext *context) {
        if (mFastUp) {
            LOG_E(LOG_TAG, "[%s] hahaha", __func__);
        }
        return SZFingerprintCore::notifyAuthSuccess(context);
    }

    void CustomizedFingerprintCore::notifyAuthNotMatched() {
        return SZFingerprintCore::notifyAuthNotMatched();
    }

    gf_error_t CustomizedFingerprintCore::onResetEvent() {
        return SZFingerprintCore::onResetEvent();
    }

    // notify message
    gf_error_t CustomizedFingerprintCore::notifyAcquiredInfo(gf_fingerprint_acquired_info_t info) {
        return SZFingerprintCore::notifyAcquiredInfo(info);
    }

    gf_error_t CustomizedFingerprintCore::notifyErrorInfo(gf_fingerprint_error_t err) {
        return SZFingerprintCore::notifyErrorInfo(err);
    }

    void CustomizedFingerprintCore::doCancel() {
        return SZFingerprintCore::doCancel();
    }

    gf_error_t CustomizedFingerprintCore::enrollPause() {
        return FingerprintCore::enrollPause();
    }
    gf_error_t CustomizedFingerprintCore::enrollResume() {
        return FingerprintCore::enrollResume();
    }

    gf_error_t CustomizedFingerprintCore::getTemp(const char *path, int32_t *temp) {
        int32_t fd = 0;
        char buffer[5] = {0};
        int32_t amt;

        fd = ::open(path, O_RDONLY);
        if (fd >= 0) {
            amt = read(fd, buffer, 5);
            ::close(fd);
            if (-1 == amt) {
                *temp = -1000;
                LOG_D(LOG_TAG, "[%s] read temperature device failed", __func__);
            } else {
                *temp = atoi(buffer)/10;
                LOG_D(LOG_TAG, "[%s] temperature is %d", __func__, *temp);
            }
        }
        return GF_SUCCESS;
    }

    void CustomizedFingerprintCore::setLight(uint32_t light) {
        mLight = light;
    }

}  // namespace goodix

