/******************************************************************************
 * @file   tz_cmd.h
 * @brief  Contains CA/TA communication command IDs.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/4/2    0.1.0      Init version
 * Joe Li      2018/5/8    0.1.1      Add set nav mode command ID.
 * David Wang  2018/5/15   0.1.2      Add get finger status command ID.
 * John Zhang  2018/5/16   0.1.3      Add get config command ID.
 * Rich Li     2018/5/28   0.1.4      Add get enroll number command ID.
 * Davie Wang  2018/6/1    0.1.5      Add capture image sub command ID.
 * Davie Wang  2018/6/15   0.1.6      Add optics command ID.
 * Rich Li     2018/7/2    0.1.7      Add algo set param command ID.
 *
 *****************************************************************************/

/* This file must be the same between CA and TA APP */

#ifndef __SL_TZ_CMD_H__
#define __SL_TZ_CMD_H__

enum fp_tz_cmd {
    TZ_FP_CMD_MODE_DOWNLOAD           = 0x00000001,
    TZ_FP_CMD_CAPTURE_IMG             = 0x00000002,
    TZ_FP_CMD_NAV_CAPTURE_IMG         = 0x00000003,
    TZ_FP_CMD_AUTH_START              = 0x00000004,
    TZ_FP_CMD_AUTH_STEP               = 0x00000005,
    TZ_FP_CMD_AUTH_END                = 0x00000006,
    TZ_FP_CMD_ENROLL_START            = 0x00000007,
    TZ_FP_CMD_ENROLL_STEP             = 0x00000008,
    TZ_FP_CMD_ENROLL_END              = 0x00000009,
    TZ_FP_CMD_NAV_START               = 0x0000000A,
    TZ_FP_CMD_NAV_STEP                = 0x0000000B,
    TZ_FP_CMD_NAV_END                 = 0x0000000C,
    TZ_FP_CMD_NAV_SUPPORT             = 0x0000000D,
    TZ_FP_CMD_INIT                    = 0x0000000E,
    TZ_FP_CMD_DEINIT                  = 0x0000000F,
    TZ_FP_CMD_SET_GID                 = 0x00000010,
    TZ_FP_CMD_LOAD_USER_DB            = 0x00000011,
    TZ_FP_CMD_FP_REMOVE               = 0x00000012,
    TZ_FP_CMD_GET_DB_COUNT            = 0x00000013,
    TZ_FP_CMD_GET_FINGERPRINTS        = 0x00000014,
    TZ_FP_CMD_LOAD_ENROLL_CHALLENGE   = 0x00000015,
    TZ_FP_CMD_SET_ENROLL_CHALLENGE    = 0x00000016,
    TZ_FP_CMD_VERIFY_ENROLL_CHALLENGE = 0x00000017,
    TZ_FP_CMD_LOAD_AUTH_ID            = 0x00000018,
    TZ_FP_CMD_GET_AUTH_OBJ            = 0x00000019,
    TZ_FP_CMD_UPDATE_CFG              = 0x0000001A,
    TZ_FP_CMD_INIT2                   = 0x0000001B,
    TZ_FP_CMD_LOAD_TEMPLATE           = 0x0000001C,
    TZ_FP_CMD_SAVE_TEMPLATE           = 0x0000001D,
    TZ_FP_CMD_UPDATE_TEMPLATE         = 0x0000001E,
    TZ_FP_CMD_SET_LOG_MODE            = 0x0000001F,
    TZ_FP_CMD_SET_SPI_DEV             = 0x00000020,
    TZ_FP_CMD_SET_NAV_MODE            = 0x00000021,
    TZ_FP_CMD_GET_FINGER_STATUS       = 0x00000022,
    TZ_FP_CMD_GET_CONFIG              = 0x00000023,
    TZ_FP_CMD_GET_ENROLL_NUM          = 0x00000024,
    TZ_FP_CMD_CAPTURE_IMG_PRE         = 0x00000025,
    TZ_FP_CMD_CAPTURE_IMG_RAW         = 0x00000026,
    TZ_FP_CMD_CAPTURE_IMG_AFTER       = 0x00000027,

    TZ_FP_CMD_SEND_CMD_WITH_BUF       = 0x00000028,

    TZ_FP_CMD_SET_KEY_DATA            = 0x00000030,
    TZ_FP_CMD_INIT_UNK_0              = 0x00000031,
    TZ_FP_CMD_INIT_UNK_1              = 0x00000032,
    TZ_FP_CMD_DEINIT_UNK_1            = 0x00000033,
    TZ_FP_CMD_INIT_UNK_2              = 0x00000034,
    TZ_FP_CMD_DEINIT_UNK_2            = 0x00000035,
    TZ_FP_CMD_CALIBRATE               = 0x00000036,
    TZ_FP_CMD_CALIBRATE2              = 0x00000037,
    TZ_FP_CMD_CHECK_ESD               = 0x00000038,
    TZ_FP_CMD_GET_OTP                 = 0x00000039,

    TZ_FP_CMD_CALIBRATE_OPTIC         = 0x00000040,

    TZ_FP_CMD_GET_VERSIONS            = 0x00000050,
    TZ_FP_CMD_GET_CHIPID              = 0x00000051,
    TZ_FP_CMD_TEST_IMAGE_CAPTURE      = 0x00000052,
    TZ_FP_CMD_TEST_SEND_GP_IMG        = 0x00000053,
    TZ_FP_CMD_TEST_IMAGE_FINISH       = 0x00000054,
    TZ_FP_CMD_TEST_DEADPX             = 0x00000055,
    TZ_FP_CMD_TEST_GET_IMG_INFO       = 0x00000056,
    TZ_FP_CMD_TEST_DUMP_DATA          = 0x00000057,
    TZ_FP_CMD_TEST_SPEED              = 0x00000058,
    TZ_FP_CMD_SEND_CMD_WITH_BUF_GET   = 0x00000059,
    TZ_FP_CMD_CHECK_BROKEN            = 0x00000060,
};

typedef struct _cmd_desc {
    int32_t cmd;
    const char *cmdstr;
} cmd_desc_t;

static inline const char *silfp_cmd_to_str(int32_t cmd)
{
    size_t i = 0;
    static const cmd_desc_t desc[] = {
        {TZ_FP_CMD_MODE_DOWNLOAD,           "MODE_DOWNLOAD"},
        {TZ_FP_CMD_CAPTURE_IMG,             "CAPTURE_IMG"},
        {TZ_FP_CMD_NAV_CAPTURE_IMG,         "NAV_CAPTURE_IMG"},
        {TZ_FP_CMD_AUTH_START,              "AUTH_START"},
        {TZ_FP_CMD_AUTH_STEP,               "AUTH_STEP"},
        {TZ_FP_CMD_AUTH_END,                "AUTH_END"},
        {TZ_FP_CMD_GET_ENROLL_NUM,          "GET_ENROLL_NUM"},
        {TZ_FP_CMD_ENROLL_START,            "ENROLL_START"},
        {TZ_FP_CMD_ENROLL_STEP,             "ENROLL_STEP"},
        {TZ_FP_CMD_ENROLL_END,              "ENROLL_END"},
        {TZ_FP_CMD_NAV_START,               "NAV_START"},
        {TZ_FP_CMD_NAV_STEP,                "NAV_STEP"},
        {TZ_FP_CMD_NAV_END,                 "NAV_END"},
        {TZ_FP_CMD_NAV_SUPPORT,             "NAV_SUPPORT"},
        {TZ_FP_CMD_INIT,                    "INIT"},
        {TZ_FP_CMD_DEINIT,                  "DEINIT"},
        {TZ_FP_CMD_SET_GID,                 "SET_GID"},
        {TZ_FP_CMD_LOAD_USER_DB,            "LOAD_USER_DB"},
        {TZ_FP_CMD_FP_REMOVE,               "FP_REMOVE"},
        {TZ_FP_CMD_GET_DB_COUNT,            "GET_DB_COUNT"},
        {TZ_FP_CMD_GET_FINGERPRINTS,        "GET_FINGERPRINTS"},
        {TZ_FP_CMD_LOAD_ENROLL_CHALLENGE,   "LOAD_ENROLL_CHALLENGE"},
        {TZ_FP_CMD_SET_ENROLL_CHALLENGE,    "SET_ENROLL_CHALLENGE"},
        {TZ_FP_CMD_VERIFY_ENROLL_CHALLENGE, "VERIFY_ENROLL_CHALLENGE"},
        {TZ_FP_CMD_LOAD_AUTH_ID,            "LOAD_AUTH_ID"},
        {TZ_FP_CMD_GET_AUTH_OBJ,            "GET_AUTH_OBJ"},
        {TZ_FP_CMD_UPDATE_CFG,              "UPDATE_CFG"},
        {TZ_FP_CMD_INIT2,                   "INIT2"},
        {TZ_FP_CMD_LOAD_TEMPLATE,           "LOAD_TEMPLATE"},
        {TZ_FP_CMD_SAVE_TEMPLATE,           "SAVE_TEMPLATE"},
        {TZ_FP_CMD_UPDATE_TEMPLATE,         "UPDATE_TEMPLATE"},
        {TZ_FP_CMD_SET_LOG_MODE,            "SET_LOG_MODE"},
        {TZ_FP_CMD_SET_SPI_DEV,             "SET_SPI_DEV"},
        {TZ_FP_CMD_SET_NAV_MODE,            "SET_NAV_MODE"},
        {TZ_FP_CMD_CAPTURE_IMG_PRE,         "CAPTURE_IMG_PRE"},
        {TZ_FP_CMD_CAPTURE_IMG_RAW,         "CAPTURE_IMG_RAW"},
        {TZ_FP_CMD_CAPTURE_IMG_AFTER,       "CAPTURE_IMG_AFTER"},
        {TZ_FP_CMD_CHECK_ESD,               "CHECK_ESD"},
        {TZ_FP_CMD_GET_FINGER_STATUS,       "GET_FINGER_STATUS"},
        {TZ_FP_CMD_GET_OTP,                 "GET_OTP"},
        {TZ_FP_CMD_SEND_CMD_WITH_BUF,       "SEND_CMD_WITH_BUF"},
        {TZ_FP_CMD_SEND_CMD_WITH_BUF_GET,   "SEND_CMD_WITH_BUF_GET"},
        {TZ_FP_CMD_SET_KEY_DATA,            "SET_KEY_DATA"},
        {TZ_FP_CMD_INIT_UNK_0,              "INIT_UNK_0"},
        {TZ_FP_CMD_INIT_UNK_1,              "INIT_UNK_1"},
        {TZ_FP_CMD_DEINIT_UNK_1,            "DEINIT_UNK_1"},
        {TZ_FP_CMD_INIT_UNK_2,              "INIT_UNK_2"},
        {TZ_FP_CMD_DEINIT_UNK_2,            "DEINIT_UNK_2"},
        {TZ_FP_CMD_CALIBRATE,               "CALIBRATE"},
        {TZ_FP_CMD_CALIBRATE2,              "CALIBRATE2"},
        {TZ_FP_CMD_GET_CONFIG,              "GET_CONFIG"},
        {TZ_FP_CMD_CALIBRATE_OPTIC,         "CALIBRATE_OPTIC"},
        {TZ_FP_CMD_GET_VERSIONS,            "GET_VERSIONS"},
        {TZ_FP_CMD_GET_CHIPID,              "GET_CHIPID"},
        {TZ_FP_CMD_TEST_IMAGE_CAPTURE,      "TEST_IMAGE_CAPTURE"},
        {TZ_FP_CMD_TEST_SEND_GP_IMG,        "TEST_SEND_GP_IMG"},
        {TZ_FP_CMD_TEST_IMAGE_FINISH,       "TEST_IMAGE_FINISH"},
        {TZ_FP_CMD_TEST_DEADPX,             "TEST_DEADPX"},
        {TZ_FP_CMD_TEST_GET_IMG_INFO,       "TEST_GET_IMG_INFO"},
        {TZ_FP_CMD_TEST_DUMP_DATA,          "TEST_DUMP_DATA"},
        {TZ_FP_CMD_TEST_SPEED,              "TEST_SPEED"},
        {TZ_FP_CMD_CHECK_BROKEN,            "CHECK_BROKEN"},
    };
    static uint32_t desc_count = sizeof(desc) / sizeof(desc[0]);

    for (i = 0; i < desc_count; i++) {
        if (desc[i].cmd == cmd) {
            return desc[i].cmdstr;
        }
    }

    return "unknow";
}

#endif /* __SL_TZ_CMD_H__ */
