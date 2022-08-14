/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _ALASKHEARTBEATRATEDEFINE_H_
#define _ALASKHEARTBEATRATEDEFINE_H_

#include "HeartbeatDefine.h"
#include "GoodixFingerprint.h"
#include "gf_delmar_types.h"
#include "gf_delmar_heartbeat_rate_types.h"

namespace goodix
{
    typedef enum
    {
        HEARTBEAT_RATE_SENSOR_DISPLAY_CONTROL = GF_FINGERPRINT_MSG_TYPE_MAX + 1,  // 1005
        HEARTBEAT_RATE_PREVIEW_DISPLAY_CONTROL,
    }
    DELMAR_HEARTBEAT_RATE_MSG;

    typedef enum
    {
        HEARTBEAT_RATE_CMD_PRE_HEART_RATE_DETECT = HEARTBEAT_RATE_CMD_MAX + 1,  // 400001
        HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_DOWN,
        HEARTBEAT_RATE_CMD_HEART_RATE_FINGER_UP,
        HEARTBEAT_RATE_CMD_POST_HEART_RATE_DETECT,
        HEARTBEAT_RATE_CMD_DUMP_HEART_RATE_RESULT,
        HEARTBEAT_RATE_CMD_CANCEL,
        HEARTBEAT_RATE_CMD_NOTIFY_UI_READY = 400100,
        HEARTBEAT_RATE_CMD_NOTIFY_UI_DISAPPEAR,
        HEARTBEAT_RATE_DELMAR_CMD_MAX,
    }
    DELMAR_HEARTBEAT_RATE_CMD_ID;

    enum
    {
        HEARTBEAT_RATE_TOKEN_HEART_RATE_RESULT = HEARTBEAT_RATE_TOKEN_MAX + 1,  // 400002
        HEARTBEAT_RATE_TOKEN_HEART_RATE_DATA_NUM,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_IS_HARD_PRESS,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_USER_NAME,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_USER_TEST_TIMES,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_RESULT_ARRAY,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_BLE_RESULT_ARRAY,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_RESULT_FLAG,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_PPG_DATA,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_ORI_PPG_DATA,
        HEARTBEAT_RATE_TOKEN_HEART_RATE_INTERVAL,
    };

    typedef enum
    {
        HEARTBEAT_RATE_AREA_SHOW_INDICATOR = 1,
        HEARTBEAT_RATE_AREA_HIDE,
    }
    DELMAR_HEARTBEAT_RATE_SENSOR_CONTROL_CMD;

    enum
    {
        HEARTBEAT_RATE_TOKEN_DISPLAY_CONTROL_STATE = 310039,  // same with debugtool for apk
    };

    typedef struct {
        int32_t finger_test_times;
        int32_t detect_finger_status;
    } gf_delmar_hb_msg_data_t;

    typedef struct {
        uint16_t is_hr_result;
        int32_t finger_test_times;
        uint8_t finger_names[COLLECT_FILE_PATH_MAX_LEN];
        uint32_t ble_hr_result[HEART_BEAT_RATE_MAX_CALC_TIME];
        uint32_t hr_result[HEART_BEAT_RATE_MAX_CALC_TIME];
    } gf_delmar_hb_result_msg_data_t;

};  // namespace goodix

#endif /* _ALASKHEARTBEATRATEDEFINE_H_ */
