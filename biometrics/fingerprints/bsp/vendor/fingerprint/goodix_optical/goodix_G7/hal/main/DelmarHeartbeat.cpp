/*
 * Copyright (C) 2013-2020, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][DelmarHeartbeatRate]"

#include <inttypes.h>
#include "DelmarHeartbeat.h"
#include "HalContext.h"
#include "DelmarHeartbeatDefine.h"
#include "gf_sensor_types.h"
#include "gf_delmar_config.h"
#include "DelmarSensor.h"
#include "TestUtils.hpp"
#include "DelmarHalUtils.h"
#include "gf_delmar_types.h"
#include "Timer.h"
#include "Device.h"
#include "Mutex.h"
#include "HalUtils.h"

static void * heartBeatDetectEntry(void *arg) {
    goodix::DelmarHeartbeatRate *self = static_cast<goodix::DelmarHeartbeatRate *>(arg);
    self->startHeartBeatDetect();
    return NULL;
}

namespace goodix {

    typedef enum {
        UNTRUST_STATE_IDLE = 0,
        UNTRUST_STATE_HEART_BEAT_DETECT,
    } UNTRUST_WORK_STATE;

    typedef enum {
        HEART_BEAT_FINGER_DOWN = 0,
        HEART_BEAT_FINGER_UP,
    } HEART_BEAT_FINGER_PRESS_STATUS;

    static DelmarHeartbeatRate *mpSelf = NULL;
    static int32_t g_heart_beat_detect_finger_status = HEART_BEAT_FINGER_UP;
    static uint8_t g_heart_beat_finger_names[COLLECT_FILE_PATH_MAX_LEN] = "NA";
    static int32_t g_heart_beat_finger_test_times = 1;
    static uint32_t g_heart_beat_ble_result[HEART_BEAT_RATE_MAX_CALC_TIME] = {0};
    static uint32_t g_heart_beat_result[HEART_BEAT_RATE_MAX_CALC_TIME] = {0};

    DelmarHeartbeatRate::DelmarHeartbeatRate(HalContext *context)
        : HeartbeatRate(context) {
        VOID_FUNC_ENTER();
        mpSelf = this;
        mWorkState = UNTRUST_STATE_IDLE;

        mContext->mMsgBus.addMsgListener(this);
        // print hal version
        LOG_I(LOG_TAG, "[%s] HAL_VERSION=%s", __func__, GF_HAL_VERSION);
        DelmarHalUtils::registerModuleVersion("ext_modules", GF_HAL_VERSION);

        VOID_FUNC_EXIT();
    }

    DelmarHeartbeatRate::~DelmarHeartbeatRate() {
        VOID_FUNC_ENTER();
        mContext->mMsgBus.removeMsgListener(this);
        VOID_FUNC_EXIT();
    }

    bool DelmarHeartbeatRate::isNeedLock(int32_t cmdId) {
        if (cmdId == HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_UP) {
            return false;
        } else {
            return HeartbeatRate::isNeedLock(cmdId);
        }
    }

    bool DelmarHeartbeatRate::isNeedCtlSensor(uint32_t cmdId) {
        bool ret = false;

        switch (cmdId) {

            default: {
                break;
            }
        }

        return ret;
    }

    gf_error_t DelmarHeartbeatRate::executeCommand(int32_t cmdId, const int8_t *in,
                                                 uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        gf_error_t ret = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor*) mContext->mSensor;
        int32_t sensorNum = (int32_t) sensor->getAvailableSensorNum();
#ifdef SUPPORT_DUMP_HEARTBEAT_RATE
        gf_delmar_hb_result_msg_data_t msg_data = { 0 };
#endif  // SUPPORT_DUMP_HEARTBEAT_RATE
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] cmdId=%d, sensorNum=%d", __func__, cmdId, sensorNum);

        if (isNeedCtlSensor(cmdId)) {
            err = sensor->wakeupSensor();
            if (err != GF_SUCCESS) {
                FUNC_EXIT(err);
                return err;
            }
        }

        switch (cmdId) {
            case HEARTBEAT_RATE_CMD_PRE_HEART_RATE_DETECT: {
                if (mWorkState == UNTRUST_STATE_IDLE) {
                    sensor->setInHBMode(true);
                    err = preHeartBeatDetect(in, inLen);
                }
                break;
            }
            case HEARTBEAT_RATE_CMD_POST_HEART_RATE_DETECT: {
                if (mWorkState == UNTRUST_STATE_HEART_BEAT_DETECT) {
                    sensor->setInHBMode(false);
                    err = postHeartBeatDetect(in, inLen);
                }
                break;
            }
            case HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_DOWN: {
                if (g_heart_beat_detect_finger_status == HEART_BEAT_FINGER_DOWN
                    || mWorkState == UNTRUST_STATE_IDLE) {
                    break;
                }
                memset(g_heart_beat_result, 0, sizeof(g_heart_beat_result));
                err = heartBeatParseTestArgs(in, inLen);
                GF_ERROR_BREAK(err);
                err = handleEvent(EVENT_FINGER_DOWN);
                break;
            }
            case HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_UP: {
                if (g_heart_beat_detect_finger_status == HEART_BEAT_FINGER_UP
                    || mWorkState == UNTRUST_STATE_IDLE) {
                    break;
                }
                g_heart_beat_detect_finger_status = HEART_BEAT_FINGER_UP;
                LOG_D(LOG_TAG, "[%s] g_heart_beat_detect_finger_status:%d", __func__, g_heart_beat_detect_finger_status);
                err = handleEvent(EVENT_FINGER_UP);
                break;
            }
            case HEARTBEAT_RATE_CMD_DUMP_HEART_RATE_RESULT: {
                memset(g_heart_beat_ble_result, 0, sizeof(g_heart_beat_ble_result));
                err = heartBeatParseResultArgs(in, inLen);
                GF_ERROR_BREAK(err);
#ifdef SUPPORT_DUMP_HEARTBEAT_RATE
                msg_data.is_hr_result = 0;
                strcpy((char *)msg_data.finger_names, (char *)g_heart_beat_finger_names);
                msg_data.finger_test_times = g_heart_beat_finger_test_times;
                memcpy(msg_data.ble_hr_result, g_heart_beat_ble_result, sizeof(g_heart_beat_ble_result));
                sendMessage(MSG_HB_RATE_DUMP_RESULT, &msg_data, sizeof(msg_data));
#endif  // SUPPORT_DUMP_HEARTBEAT_RATE
                break;
            }
            case HEARTBEAT_RATE_CMD_CANCEL: {
                sensor->setInHBMode(false);
                err = mContext->mDevice->enable_tp(GF_TP_DISENABLE);
                if (err != GF_SUCCESS) {
                    LOG_E(LOG_TAG, "[%s] enable_tp fail, err code : %d.", __func__, err);
                }
                if (g_heart_beat_detect_finger_status == HEART_BEAT_FINGER_DOWN) {
                    g_heart_beat_detect_finger_status = HEART_BEAT_FINGER_UP;
                    err = handleEvent(EVENT_FINGER_UP);
                    GF_ERROR_BREAK(err);
                }
                if (mWorkState == UNTRUST_STATE_HEART_BEAT_DETECT) {
                    mWorkState = UNTRUST_STATE_IDLE;
                }
                sendMessage(MsgBus::MSG_CANCELED);
                break;
            }

            default: {
                err = HeartbeatRate::executeCommand(cmdId, in, inLen, out, outLen);
                break;
            }
        }

        if (isNeedCtlSensor(cmdId)) {
            ret = sensor->sleepSensor();
            if (err == GF_SUCCESS && ret != GF_SUCCESS) {
                err = ret;
                LOG_E(LOG_TAG, "[%s] sleep sensor failed!", __func__);
            }
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarHeartbeatRate::handleEvent(gf_event_type_t e) {
        gf_error_t err = GF_SUCCESS;
        DelmarSensor *sensor = (DelmarSensor *)mContext->mSensor;
        FUNC_ENTER();

        do {
            switch (e) {
            case EVENT_FINGER_DOWN:
                LOG_D(LOG_TAG, "[%s] down event in heartbeat rate mode", __func__);
                err = sensor->wakeupSensor();
                GF_ERROR_BREAK(err);
                g_heart_beat_detect_finger_status = HEART_BEAT_FINGER_DOWN;
                LOG_D(LOG_TAG, "[%s] g_heart_beat_detect_finger_status:%d", __func__, g_heart_beat_detect_finger_status);
                if (mWorkState == UNTRUST_STATE_HEART_BEAT_DETECT) {
                    pthread_t thread;
                    if (pthread_create(&thread, NULL, heartBeatDetectEntry, (void *)this) != 0) {
                        LOG_E(LOG_TAG, "[%s] pthread_create failed", __func__);
                        err = GF_ERROR_GENERIC;
                    } else {
                        pthread_detach(thread);
                    }
                }
                break;
            case EVENT_FINGER_UP:
                LOG_D(LOG_TAG, "[%s] up event in heartbeat rate mode", __func__);
                if (mWorkState == UNTRUST_STATE_HEART_BEAT_DETECT) {
                    err = stopHeartBeatDetect();
                    GF_ERROR_BREAK(err);
                }
                err = sensor->sleepSensor();
                break;
            default:
                break;
            }
            if (err != GF_SUCCESS) {
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarHeartbeatRate::onMessage(const MsgBus::Message &msg) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] onMessage msg:%d", __func__, msg.msg);
        switch (msg.msg) {
            case MsgBus::MSG_WAIT_FOR_FINGER_INPUT: {
                if (mWorkState == UNTRUST_STATE_HEART_BEAT_DETECT) {
                    handleWaitForFingerInputMessage();
                }
                break;
            }

            case MsgBus::MSG_CANCELED: {
                handleCanceledMessage();
                break;
            }

            case MsgBus::MSG_SCREEN_STATE: {
                LOG_D(LOG_TAG, "[%s] screen off  in heartbeat rate mode", __func__);
                if (msg.params1 == 0) {
                    if (g_heart_beat_detect_finger_status == HEART_BEAT_FINGER_DOWN) {
                        g_heart_beat_detect_finger_status = HEART_BEAT_FINGER_UP;
                        err = handleEvent(EVENT_FINGER_UP);
                        GF_ERROR_BREAK(err);
                    }
                    if (mWorkState == UNTRUST_STATE_HEART_BEAT_DETECT) {
                        mWorkState = UNTRUST_STATE_IDLE;
                    }
                }
                break;
            }

            default: {
                break;
            }
        }
        FUNC_EXIT(err);
        return err;
    }

    void DelmarHeartbeatRate::handleWaitForFingerInputMessage() {
        notifySensorDisplayControl(0, HEARTBEAT_RATE_AREA_SHOW_INDICATOR, NULL, 0);
        return;
    }

    void DelmarHeartbeatRate::handleCanceledMessage() {
        notifySensorDisplayControl(0, HEARTBEAT_RATE_AREA_HIDE, NULL, 0);
        return;
    }

    void DelmarHeartbeatRate::notifySensorDisplayControl(int64_t devId, int32_t cmdId,
                                                       const int8_t *sensorData, int32_t dataLen) {
        int8_t *test_result = NULL;
        uint32_t len = 0;
        VOID_FUNC_ENTER();

        do {
            if (NULL == mpNotify) {
                LOG_D(LOG_TAG, "callback is not set");
                break;
            }

            // work_state
            len += HAL_TEST_SIZEOF_INT32;
            // dataLen
            len += sizeof(int32_t);

            if (dataLen > 0 && sensorData != NULL) {
                len += dataLen;
            }

            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_DISPLAY_CONTROL_STATE,
                                                     mWorkState);
                memcpy(current, &dataLen, sizeof(int32_t));
                current += sizeof(int32_t);

                if (dataLen > 0 && sensorData != NULL) {
                    memcpy(current, sensorData, dataLen);
                    current += dataLen;
                }

                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            mpNotify(devId, HEARTBEAT_RATE_SENSOR_DISPLAY_CONTROL, cmdId, test_result, len);
        }
        while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void DelmarHeartbeatRate::notifyHeartbeatRateCmd(int64_t devId, int32_t cmdId,
                                    const int8_t *result, int32_t resultLen) {
        VOID_FUNC_ENTER();

        if (mpNotify != NULL) {
            mpNotify(devId, GF_FINGERPRINT_TEST_CMD, cmdId, result, resultLen);
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t DelmarHeartbeatRate::preHeartBeatDetect(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(in);
        UNUSED_VAR(inLen);
        err = mContext->mDevice->enable_tp(GF_TP_ENABLE);
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] enable_tp fail, err code : %d.", __func__, err);
            return err;
        }
        mWorkState = UNTRUST_STATE_HEART_BEAT_DETECT;
        sendMessage(MsgBus::MSG_WAIT_FOR_FINGER_INPUT);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarHeartbeatRate::heartBeatParseTestArgs(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        uint32_t token = 0;
        //char userFingerName[COLLECT_FILE_PATH_MAX_LEN] = { 0 };
        //uint32_t g_heart_beat_finger_test_times = 0;
        FUNC_ENTER();
        do {
            const int8_t *in_buf = in;
            uint32_t arrayLen = 0;

            if (NULL != in_buf) {
                do {
                    in_buf = TestUtils::testDecodeUint32(&token, in_buf);
                    arrayLen = 0;

                    switch (token) {
                        case HEARTBEAT_RATE_TOKEN_HEART_RATE_USER_NAME: {
                            in_buf = TestUtils::testDecodeUint32(&arrayLen, in_buf);
                            if (arrayLen > 0) {
                                memset(g_heart_beat_finger_names, 0, COLLECT_FILE_PATH_MAX_LEN);
                                memcpy(g_heart_beat_finger_names, in_buf, arrayLen);
                                g_heart_beat_finger_names[arrayLen + 1] = '\0';
                            } else {
                                memset(g_heart_beat_finger_names, 0, COLLECT_FILE_PATH_MAX_LEN);
                                g_heart_beat_finger_names[0] = 'N';
                                g_heart_beat_finger_names[1] = 'A';
                                g_heart_beat_finger_names[2] = '\0';
                            }
                            in_buf += arrayLen;
                            LOG_D(LOG_TAG, "[%s] g_heart_beat_finger_names : %s", __func__, g_heart_beat_finger_names);
                            break;
                        }
                        case HEARTBEAT_RATE_TOKEN_HEART_RATE_USER_TEST_TIMES: {
                            in_buf = TestUtils::testDecodeUint32((uint32_t *)&g_heart_beat_finger_test_times, in_buf);
                            LOG_D(LOG_TAG, "[%s] g_heart_beat_finger_test_times: %d", __func__, g_heart_beat_finger_test_times);
                            break;
                        }
                        default: {
                            LOG_D(LOG_TAG, "[%s] invalid token: %d", __func__, token);
                            break;
                        }
                    }
                }
                while (in_buf < in + inLen);
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarHeartbeatRate::heartBeatParseResultArgs(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        uint32_t token = 0;
        const int8_t * buf = NULL;
        const int8_t * in_buf = in;
        uint32_t arrayLen = 0;
        uint32_t i = 0;
        FUNC_ENTER();
        do {
            if (NULL != in_buf) {
                do {
                    in_buf = TestUtils::testDecodeUint32(&token, in_buf);
                    arrayLen = 0;

                    switch (token) {
                        /*case HEARTBEAT_RATE_TOKEN_HEART_RATE_RESULT_ARRAY: {
                            in_buf = TestUtils::testDecodeUint32(&arrayLen, in_buf);
                            buf = in_buf;
                            if (arrayLen > 0 && arrayLen % sizeof(uint32_t) == 0) {
                                if (arrayLen / sizeof(uint32_t) <=  HEART_BEAT_RATE_MAX_CALC_TIME) {
                                    for (i = 0; i < arrayLen / sizeof(uint32_t); i++) {
                                        buf = TestUtils::testDecodeUint32(&(g_heart_beat_result[i]), buf);
                                    }
                                } else {
                                    for (i = 0; i < HEART_BEAT_RATE_MAX_CALC_TIME; i++) {
                                        buf = TestUtils::testDecodeUint32(&(g_heart_beat_result[i]), buf);
                                    }
                                }
                            }
                            in_buf += arrayLen;
                            break;
                        }*/
                        case HEARTBEAT_RATE_TOKEN_HEART_RATE_BLE_RESULT_ARRAY: {
                            in_buf = TestUtils::testDecodeUint32(&arrayLen, in_buf);
                            buf = in_buf;
                            if (arrayLen > 0 && arrayLen % sizeof(uint32_t) == 0) {
                                if (arrayLen / sizeof(uint32_t) <=  HEART_BEAT_RATE_MAX_CALC_TIME) {
                                    for (i = 0; i < arrayLen / sizeof(uint32_t); i++) {
                                        buf = TestUtils::testDecodeUint32(&(g_heart_beat_ble_result[i]), buf);
                                    }
                                } else {
                                    for (i = 0; i < HEART_BEAT_RATE_MAX_CALC_TIME; i++) {
                                        buf = TestUtils::testDecodeUint32(&(g_heart_beat_ble_result[i]), buf);
                                    }
                                }
                            }
                            in_buf += arrayLen;
                            break;
                        }
                        default: {
                            LOG_D(LOG_TAG, "[%s] invalid token: %d", __func__, token);
                            break;
                        }
                    }
                }
                while (in_buf < in + inLen);
            } else {
                err = GF_ERROR_BAD_PARAMS;
                LOG_E(LOG_TAG, "[%s] bad params", __func__);
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }


    gf_error_t DelmarHeartbeatRate::postHeartBeatDetect(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(in);
        UNUSED_VAR(inLen);
        err = mContext->mDevice->enable_tp(GF_TP_DISENABLE);
        if (err != GF_SUCCESS) {
            LOG_E(LOG_TAG, "[%s] enable_tp fail, err code : %d.", __func__, err);
        }
        mWorkState = UNTRUST_STATE_IDLE;
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DelmarHeartbeatRate::startHeartBeatDetect() {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_heart_beat_rate_cmd_t *cmd = NULL;
        uint64_t t0 = 0;
        uint64_t t1 = 0;
        uint64_t t2 = 0;
        gf_delmar_config_t *config = (gf_delmar_config_t *) mContext->mConfig;
        uint32_t sample_count = HEART_BEAT_RATE_MAX_SAMPLE_COUNT;
#ifdef SUPPORT_DUMP_HEARTBEAT_RATE
        gf_delmar_hb_msg_data_t msg_data = { 0 };
        gf_delmar_hb_result_msg_data_t result_msg_data = { 0 };
#endif  // SUPPORT_DUMP_HEARTBEAT_RATE
        FUNC_ENTER();

        do {
            if (config->hb_sample_count > 0) {
                sample_count = config->hb_sample_count * HEART_BEAT_RATE_SAMPLING_RATE;
            }
            t0 = HalUtils::getCurrentTimeMicrosecond();
            if (cmd == NULL) {
                cmd = (gf_delmar_heart_beat_rate_cmd_t*) malloc(sizeof(gf_delmar_heart_beat_rate_cmd_t));
                if (NULL == cmd) {
                    err = GF_ERROR_OUT_OF_MEMORY;
                    break;
                }
            }
            memset(cmd, 0, sizeof(gf_delmar_heart_beat_rate_cmd_t));
            // used to control continue capture 10 frames in ta or not when config->support_hb_continue_capture == 0
            // 1: capture 10 frames in ta per time
            // 0: capture 1 frame in ta per time
            cmd->i_continue_flag = 0;
            cmd->header.target = GF_TARGET_HEARTBEAT_RATE;
            cmd->header.cmd_id = GF_HEARTBEAT_RATE_CMD_START_HEART_BEAT_RATE_DETECT;
            strcpy((char *)cmd->finger_names, (char *)g_heart_beat_finger_names);
            LOG_D(LOG_TAG, "[%s] cmd->finger_names:%s", __func__, cmd->finger_names);

            err = invokeCommand(cmd, sizeof(gf_delmar_heart_beat_rate_cmd_t));

            t1 = HalUtils::getCurrentTimeMicrosecond();
            LOG_D(LOG_TAG, "[%s] one frame = %dms", __func__, (int32_t)((t1 - t0)/ 1000));

            LOG_D(LOG_TAG, "[%s] origin_ppg_data=%d; interval_time=%d", __func__, cmd->origin_ppg_data, cmd->interval_time);

            if (GF_SUCCESS != err || cmd->rawdata_count >= 5) {
                cmd->is_hard_press = 0;
                if (cmd->rawdata_count >= 4 * HEART_BEAT_RATE_SAMPLING_RATE
                    && cmd->rawdata_count % HEART_BEAT_RATE_SAMPLING_RATE == 0) {
                    g_heart_beat_result[cmd->rawdata_count / HEART_BEAT_RATE_SAMPLING_RATE - 4] = cmd->hb_rate;
                }
                notifyHeartBeatRateResult(err, cmd);
            } else {
#ifdef SUPPORT_OPEN_HB_PPG_DATA
                notifyHeartBeatRateResult(err, cmd);
                LOG_D(LOG_TAG, "[%s] SUPPORT_OPEN_HB_PPG_DATA", __func__);
#endif  // SUPPORT_OPEN_HB_PPG_DATA
            }
            LOG_D(LOG_TAG, "[%s] cmd->hb_rate: %d,  cmd->ppg_data: %d, cmd->rawdata_count:%d", __func__, cmd->hb_rate, cmd->ppg_data, cmd->rawdata_count);
            LOG_D(LOG_TAG, "[%s] g_heart_beat_detect_finger_status:%d", __func__, g_heart_beat_detect_finger_status);
#ifdef SUPPORT_DUMP_HEARTBEAT_RATE
            msg_data.detect_finger_status = g_heart_beat_detect_finger_status;
            msg_data.finger_test_times = g_heart_beat_finger_test_times;
            sendMessage(MSG_HB_RATE_CAPTURE_IMG_FINISHED, &msg_data, sizeof(msg_data));
#endif  // SUPPORT_DUMP_HEARTBEAT_RATE

            if (g_heart_beat_detect_finger_status == HEART_BEAT_FINGER_UP ||
                cmd->rawdata_count >= sample_count ||
                GF_SUCCESS != err) {
#ifdef SUPPORT_DUMP_HEARTBEAT_RATE
                sendMessage(MSG_HB_RATE_COLLECT_FINISHED, &msg_data, sizeof(msg_data));
                sendMessage(MSG_HB_RATE_COLLECT_PPG_FINISHED, &msg_data, sizeof(msg_data));
                result_msg_data.is_hr_result = 1;
                strcpy((char *)result_msg_data.finger_names, (char *)g_heart_beat_finger_names);
                result_msg_data.finger_test_times = g_heart_beat_finger_test_times;
                memcpy(result_msg_data.hr_result, g_heart_beat_result, sizeof(g_heart_beat_result));
                sendMessage(MSG_HB_RATE_DUMP_RESULT, &result_msg_data, sizeof(result_msg_data));
#endif  // SUPPORT_DUMP_HEARTBEAT_RATE
                break;
            }
            t2 = HalUtils::getCurrentTimeMicrosecond();
            LOG_D(LOG_TAG, "[%s] notify + dump = %dms; count = %d", __func__, (int32_t)((t2 - t1)/ 1000), cmd->rawdata_count );
            LOG_D(LOG_TAG, "[%s] one frame + notify + dump = %dms", __func__, (int32_t)((t2 - t0)/ 1000));
        } while (1);

        if (cmd != NULL) {
            free(cmd);
            cmd = NULL;
        }

        FUNC_EXIT(err);
        return GF_SUCCESS;
    }

    gf_error_t DelmarHeartbeatRate::stopHeartBeatDetect() {
        gf_error_t err = GF_SUCCESS;
        gf_delmar_clear_heart_beat_rate_data_cmd_t *cmd = NULL;
        FUNC_ENTER();

        do {
            cmd = (gf_delmar_clear_heart_beat_rate_data_cmd_t*) malloc(sizeof(gf_delmar_clear_heart_beat_rate_data_cmd_t));
            if (NULL == cmd) {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memset(cmd, 0, sizeof(gf_delmar_clear_heart_beat_rate_data_cmd_t));
            cmd->header.target = GF_TARGET_HEARTBEAT_RATE;
            cmd->header.cmd_id = GF_HEARTBEAT_RATE_CMD_STOP_HEART_BEAT_RATE_DETECT;

            err = invokeCommand(cmd, sizeof(gf_delmar_clear_heart_beat_rate_data_cmd_t));
        } while (0);

        if (cmd != NULL) {
            free(cmd);
            cmd = NULL;
        }

        FUNC_EXIT(err);
        return GF_SUCCESS;
    }

    void DelmarHeartbeatRate::notifyHeartBeatRateResult(gf_error_t err, gf_delmar_heart_beat_rate_cmd_t *result) {
        int8_t *test_result = NULL;
        uint32_t len = 0;
        VOID_FUNC_ENTER();

        // error_code
        len += HAL_TEST_SIZEOF_INT32;
        // heart rate result data
        len += HAL_TEST_SIZEOF_INT32;
        // heart rate data num
        len += HAL_TEST_SIZEOF_INT32;
        // heart rate ppg data
        len += HAL_TEST_SIZEOF_INT32;
#ifdef SUPPORT_OPEN_HB_PPG_DATA
        // heart rate original ppg data
        len += HAL_TEST_SIZEOF_INT32;
        // heart rate capture interval
        len += HAL_TEST_SIZEOF_INT32;
#endif  // SUPPORT_OPEN_HB_PPG_DATA

        test_result = new int8_t[len] {0};

        if (test_result != NULL) {
            memset(test_result, 0, len);
            int8_t *current = test_result;
            current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_ERROR_CODE,
                                err);
            current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_HEART_RATE_RESULT,
                                        result->hb_rate);
            current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_HEART_RATE_DATA_NUM,
                                        result->rawdata_count);
            current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_HEART_RATE_PPG_DATA,
                                        result->ppg_data);
#ifdef SUPPORT_OPEN_HB_PPG_DATA
            current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_HEART_RATE_ORI_PPG_DATA,
                                        result->origin_ppg_data);
            current = TestUtils::testEncodeInt32(current, HEARTBEAT_RATE_TOKEN_HEART_RATE_INTERVAL,
                                        result->interval_time);
#endif  // SUPPORT_OPEN_HB_PPG_DATA
        } else {
            len = 0;
        }

        LOG_D(LOG_TAG, "[%s] err = %d", __func__, err);
        LOG_D(LOG_TAG, "[%s] hb result = %d", __func__, result->hb_rate);
        notifyHeartbeatRateCmd(0, HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_DOWN, test_result, len);

        if (test_result != nullptr)
        {
            free(test_result);
            test_result = nullptr;
        }

        VOID_FUNC_EXIT();
    }
}  // namespace goodix
