/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][Sensor]"

#include <sys/time.h>
#include "HalContext.h"
#include "Sensor.h"
#include "HalLog.h"
#include "gf_sensor_types.h"
#include "FingerprintCore.h"
#include "Device.h"

// config timeout by customized system
#ifndef WAIT_UIREADY_TIMEOUT_MS
#define WAIT_UIREADY_TIMEOUT_MS    500     // 500 ms
#endif  // #ifndef WAIT_UIREADY_TIMEOUT_MS

// battery
#define TEMPRATURE_DEVICE_PATH "/sys/class/thermal/thermal_zone74/temp"

namespace goodix {

    static int32_t doWait(sem_t* sem, uint32_t ms) {
        int32_t ret = -1;
        struct timespec t;
        struct timeval time;
        int32_t semvalue = -1;
        gettimeofday(&time, NULL);
        LOG_D(LOG_TAG, "[%s]Wait %u ms", __func__, ms);
        time.tv_usec += ms * 1000;
        if (time.tv_usec >= 1000000) {
            time.tv_sec += time.tv_usec / 1000000;
            time.tv_usec %= 1000000;
        }
        t.tv_sec = time.tv_sec;
        t.tv_nsec = time.tv_usec * 1000;
        sem_getvalue(sem, &semvalue);
        while (semvalue > 0) {
            LOG_D(LOG_TAG, "[%s]Consume sem value(%d).", __func__, semvalue);
            sem_timedwait(sem, &t);
            sem_getvalue(sem, &semvalue);
        }
        ret = sem_timedwait(sem, &t);
        LOG_D(LOG_TAG, "[%s]Wait exit, ret = %d.", __func__, ret);
        return ret;
    }

    static int32_t doPost(sem_t* sem) {
        int32_t ret = -1;
        int32_t semvalue = -1;
        sem_getvalue(sem, &semvalue);
        if (semvalue <= 0) {
            ret = sem_post(sem);
        }
        LOG_I(LOG_TAG, "[%s]sem value = %d", __func__, semvalue);
        return ret;
    }

    Sensor::SENSOR_STATUS Sensor::mSensorSleep = INIT;
    Sensor::Sensor(HalContext *context) : HalBase(context),
        mIsSensorUIReady(false) {
        sem_init(&mUiReadySem, 0, 0);
        mContext->mMsgBus.addMsgListener(this);
    }

    Sensor::~Sensor() {
        mContext->mMsgBus.removeMsgListener(this);
    }

    gf_error_t Sensor::init() {
        gf_error_t err = GF_SUCCESS;
        do {
            gf_sensor_init_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_INIT;
            err = invokeCommand(&cmd, sizeof(gf_sensor_init_t));
            GF_ERROR_BREAK(err);
            gf_config_t *config = new gf_config_t();

            if (config == nullptr) {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memcpy(config, &cmd.o_config, sizeof(gf_config_t));
            mContext->mConfig = (gf_config_t *) config;
        } while (0);
        return err;
    }

    gf_error_t Sensor::setStudyDisable(int32_t disable) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        // Child class override the method.
        UNUSED_VAR(disable);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Sensor::captureImage(int32_t op, uint32_t retry) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (mContext->mCenter->hasUpEvt()) {
                err = GF_ERROR_TOO_FAST;
                break;
            }

            if (mContext->mConfig->support_ui_ready
                                                  && !isSensorUIReady()) {
                err = GF_ERROR_UI_DISAPPEAR;
                break;
            }

            gf_capture_image_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_CAPTURE_IMG;
            cmd.i_operation = op;
            cmd.i_retry_count = retry;
            err = invokeCommand(&cmd, sizeof(cmd));
            GF_ERROR_BREAK(err);

            if (mContext->mCenter->hasUpEvt()) {
                err = GF_ERROR_TOO_FAST;
                break;
            }

            if (mContext->mConfig->support_ui_ready
                                                  && !isSensorUIReady()) {
                err = GF_ERROR_UI_DISAPPEAR;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_event_type_t Sensor::getIrqEventType() {
        gf_event_type_t event = EVENT_UNKNOWN;
        gf_error_t err = GF_SUCCESS;
        bool openFlag = false;
        FUNC_ENTER();

        do {
            if (mSensorSleep == SLEEP) {
                err = wakeupSensor();
                if (err != GF_SUCCESS) {
                    LOG_E(LOG_TAG, "[%s] wake sensor fail", __func__);
                    break;
                }
                openFlag = true;
            }

            gf_get_irq_type_cmd_t cmd = {{ 0 }};
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SENSOR_GET_IRQ_TYPE;
            err = invokeCommand(&cmd, sizeof(gf_get_irq_type_cmd_t));

            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] retrieve irq type fail", __func__);
                break;
            }

            event = (gf_event_type_t)cmd.o_irq_type;
        } while (0);

        if (openFlag == true) {
            err = sleepSensor();
            if (err != GF_SUCCESS) {
                LOG_E(LOG_TAG, "[%s] sleep sensor fail", __func__);
            }
        }
        FUNC_EXIT(err);
        return event;
    }

    gf_error_t Sensor::sleepSensor() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            Mutex::Autolock _l(mContext->mWakeUpLock);
            err = doSleep(mContext);
        } while (0);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Sensor::wakeupSensor() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        do {
            Mutex::Autolock _l(mContext->mWakeUpLock);
            bool needReset = 0;
            err = doWakeup(mContext, &needReset);
            if (needReset) {
                err = mContext->mDevice->reset();
            }
            if (err != GF_SUCCESS) {
                break;
            }
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }

    void Sensor::setSensorUIReady() {
        mIsSensorUIReady = true;
        LOG_D(LOG_TAG, "[%s] UI ready", __func__);
        doPost(&mUiReadySem);
    }

    bool Sensor::waitSensorUIReady() {
        FUNC_ENTER();
        do {
            if (mIsSensorUIReady) {
                break;
            }
            doWait(&mUiReadySem, WAIT_UIREADY_TIMEOUT_MS);
        } while (0);
        usleep(18*1000);
        LOG_D(LOG_TAG, "[%s] UI readyn 18 delay end", __func__);
        VOID_FUNC_EXIT();
        return mIsSensorUIReady;
    }

    bool Sensor::waitSensorUIDisappear() {
        int32_t iRetry = 200;
        while (iRetry > 0) {
            if (!mIsSensorUIReady) {
                return true;
            }
            iRetry--;
            usleep(100 * 25);  // 2.5ms
        }
        iRetry--;
        if (iRetry < 0) {
            LOG_D(LOG_TAG, "Wait ui disappear timeout");
        }
        return false;
    }

    void Sensor::clearSensorUIReady() {
        mIsSensorUIReady = false;
        doPost(&mUiReadySem);
    }

    bool Sensor::isSensorUIReady() {
        return mIsSensorUIReady;
    }

    uint8_t Sensor::getLogoTimes() {
        gf_get_logo_times_t cmd = {{ 0 }};
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        cmd.header.target = GF_TARGET_SENSOR;
        cmd.header.cmd_id = GF_CMD_SENSOR_GET_LOGO_TIMES;
        cmd.o_logo_times = -1;
        err = invokeCommand(&cmd, sizeof(cmd));
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] failed", __func__);
        }

        FUNC_EXIT(err);
        return cmd.o_logo_times;
    }

    void Sensor::resetWakeupFlag() {
        mSensorSleep = INIT;
    }

    gf_error_t Sensor::doWakeup(HalContext* halContext, bool* needReset) {
        gf_error_t err = GF_SUCCESS;
        gf_sensor_wakeup_t cmd = {{ 0 }};
        FUNC_ENTER();
        if (mSensorSleep != WAKEUP) {
            cmd.header.target = GF_TARGET_SENSOR;
            cmd.header.cmd_id = GF_CMD_SNESOR_WAKEUP;
            err = halContext->invokeCommand(&cmd, sizeof(gf_sensor_wakeup_t));
            if (err == GF_SUCCESS) {
                mSensorSleep = WAKEUP;
                if (needReset) {
                    *needReset = cmd.o_need_reset;
                    LOG_D(LOG_TAG, "[%s] ### wakeup ###", __func__);
                }
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Sensor::doSleep(HalContext* halContext) {
        gf_error_t err = GF_SUCCESS;
        gf_sensor_sleep_t cmd = { 0 };
        FUNC_ENTER();
        if (mSensorSleep != SLEEP) {
            cmd.target = GF_TARGET_SENSOR;
            cmd.cmd_id = GF_CMD_SNESOR_SLEEP;
            err = halContext->invokeCommand(&cmd, sizeof(gf_sensor_sleep_t));
            if (err == GF_SUCCESS) {
                mSensorSleep = SLEEP;
                LOG_D(LOG_TAG, "[%s] ### sleep ###", __func__);
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Sensor::onMessage(const MsgBus::Message &msg) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(msg);
        return err;
    }

    int32_t Sensor::detectTemperature() {
        FILE* fp = NULL;
        char buffer[80] = { 0 };

        int32_t temperature = 0;
        do {
            LOG_E(LOG_TAG, "[%s] Open %s.", __func__, TEMPRATURE_DEVICE_PATH);
            if ((fp = fopen(TEMPRATURE_DEVICE_PATH, "rb")) == NULL) {
                LOG_E(LOG_TAG, "[%s] Open %s error.", __func__, TEMPRATURE_DEVICE_PATH);
                break;
            }
            int32_t file_len = fread(buffer, sizeof(uint8_t), sizeof(buffer), fp);
            if (file_len <= 0) {
                LOG_E(LOG_TAG, "[%s] Read %s error, file len = %d.", __func__, TEMPRATURE_DEVICE_PATH, file_len);
                break;
            }
            temperature = atoi(buffer) / 1000;
            LOG_D(LOG_TAG, "[%s] temperature = %d", __func__, temperature);
        } while (0);
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        return temperature;
    }
}  // namespace goodix
