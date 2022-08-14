/******************************************************************************
 * @file   silead_const.h
 * @brief  Contains CA const header file.
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
 * David Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#ifndef __SILEAD_CONST_H__
#define __SILEAD_CONST_H__

#define NUM_ELEMS(a) (sizeof (a) / sizeof (a)[0])
#define BRIGHTNESS_ALL 750

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif /* UNUSED */

#ifndef SIL_FP_SENSOR_DEVICE
#define SIL_FP_SENSOR_DEVICE "/dev/silead_fp"
#endif

#ifndef SIL_STUB_DEV
#define SIL_STUB_DEV "/dev/silead_stub"
#endif

#define EXT_RESULT_PASS 0
#define EXT_RESULT_FAIL 1

#define MAX_PATH_LEN 256

typedef enum _module_mode {
    IRQ_UP = 0,
    IRQ_DOWN = 1,
    IRQ_NAV = 2,
    NORMAL = 3,
    IRQ_ESD = 4,
    DEEP_SLEEP = 5,
} module_mode_t;

// silfp_cmd_send_cmd_with_buf & silfp_cmd_send_cmd_with_buf_and_get cmd id
typedef enum _req_cmd_id {
    REQ_CMD_SUB_ID_SPI_CTRL                 = 0,        // spi control cmd (send)
    REQ_CMD_SUB_ID_TOUCH_INFO               = 1,        // send touch info cmd (send)
    REQ_CMD_SUB_ID_SNR_TEST_FINISH          = 0x0102,   // snr finish cmd (send)
    REQ_CMD_SUB_ID_SRAM_TEST                = 0,        // sram test cmd (send and get)
    REQ_CMD_SUB_ID_SNR_TEST                 = 1,        // snr test cmd (send and get)
    REQ_CMD_SUB_ID_OPTIC_FACTORY_QUALITY    = 2,        // optic factory quality test cmd (send and get)
    REQ_CMD_SUB_ID_DUMP_FILE_EXT            = 3,        // send dump data file ext info cmd (send and get)
    REQ_CMD_SUB_ID_FLASH_TEST               = 5,        // falsh test (send and get)
    REQ_CMD_SUB_ID_DUMP_TA_LOG              = 6,        // dump ta log (send and get)
    REQ_CMD_SUB_ID_TEST_IMAGE_INIT          = 0x1001,   // capture image test init (send)
    REQ_CMD_SUB_ID_DUMP_TA_LOG_ENABLE       = 0x1002,   // enable ta log dump to ca (send)
    REQ_CMD_SUB_ID_SYNC_EXT_INFO            = 0x1003,   // sync android time to ta (send)
    REQ_CMD_SUB_ID_DBG_CFG                  = 0x1004,   // dump chip config (send and get)
    REQ_CMD_SUB_ID_EXTEND_INFO_IN_FLASH     = 0x1005,   // read extend info from flash (send)
    REQ_CMD_SUB_ID_LOAD_CAL_IN_FLASH        = 0x1006,   // load cal data from flash (send)
    REQ_CMD_SUB_ID_SAVE_CAL_IN_FLASH        = 0x1007,   // save cal data from flash (send)
    REQ_CMD_SUB_ID_GET_TA_FEATURE           = 0x1013,
    REQ_CMD_SUB_ID_GET_BROKEN_INFO          = 0x1014,    //get broken info(send and get)
    REQ_CMD_SUB_ID_GET_AUTH_INFO            = 0x10001,   // save bmp data (send and get)
} req_cmd_id_t;

typedef enum _img_capture_type {
    IMG_CAPTURE_AUTH = 0,       // auth
    IMG_CAPTURE_ENROLL,         // enroll
    IMG_CAPTURE_CAPTURE_TEST,   // frrfar capture image
    IMG_CAPTURE_AGING_TEST,     // aging test
    IMG_CAPTURE_QUALITY_TEST,   // quality test
    IMG_CAPTURE_OTHER,
} img_capture_type_t;

/*
 * extend common id and subid, for factory test
*/
typedef enum _ext_cmd_id {
    EXT_CMD_BASE                        = 0x0000,
    EXT_CMD_SPI_TEST                    = EXT_CMD_BASE,
    EXT_CMD_TEST_RESET_PIN              = EXT_CMD_BASE + 1,
    EXT_CMD_TEST_DEAD_PIXEL             = EXT_CMD_BASE + 2,
    EXT_CMD_GET_VERSION                 = EXT_CMD_BASE + 3,
    EXT_CMD_GET_IMAGE                   = EXT_CMD_BASE + 4,
    EXT_CMD_SEND_IMAGE                  = EXT_CMD_BASE + 6,
    EXT_CMD_SEND_IMAGE_CLEAR            = EXT_CMD_BASE + 7,
    EXT_CMD_SELF_TEST                   = EXT_CMD_BASE + 8,
    EXT_CMD_TEST_SPEED                  = EXT_CMD_BASE + 9,
    EXT_CMD_TEST_FINISH                 = EXT_CMD_BASE + 10,
    EXT_CMD_CALIBRATE                   = EXT_CMD_BASE + 11,
    EXT_CMD_CALIBRATE_STEP              = EXT_CMD_BASE + 12,
    EXT_CMD_SEND_FINGER_DOWN            = EXT_CMD_BASE + 13,
    EXT_CMD_SEND_FINGER_UP              = EXT_CMD_BASE + 14,
    EXT_CMD_TEST_FLASH                  = EXT_CMD_BASE + 15,
    EXT_CMD_TEST_OTP                    = EXT_CMD_BASE + 16,
    EXT_CMD_ICON_READY                  = EXT_CMD_BASE + 17,

    EXT_CMD_IMAGE_QUALITY_GET           = EXT_CMD_BASE + 0x0102,
    EXT_CMD_IMAGE_CAPTURE_AGING         = EXT_CMD_BASE + 0x0103,
    EXT_CMD_OPTIC_TEST_FACTORY_QUALITY  = EXT_CMD_BASE + 0x0104,
} ext_cmd_id_t;

#define EXT_IMG_FEATURE_GEN_TPL_MASK    0x00000001
#define EXT_IMG_FEATURE_ORIG_MASK       0x00000002
#define EXT_IMG_FEATURE_QAULITY_MASK    0x00000004
#define EXT_IMG_FEATURE_DATA_MASK       0x00000008

typedef enum _ext_cal_sub_cmd_id {
    TEST_SUB_CMD_CAL_STEP_START = 0,
    TEST_SUB_CMD_CAL_STEP_1 = 1,
    TEST_SUB_CMD_CAL_STEP_2,
    TEST_SUB_CMD_CAL_STEP_3,
    TEST_SUB_CMD_CAL_STEP_4,
    TEST_SUB_CMD_CAL_STEP_5,
    TEST_SUB_CMD_CAL_STEP_6,
    TEST_SUB_CMD_CAL_STEP_7,
    TEST_SUB_CMD_CAL_STEP_8,
    TEST_SUB_CMD_CAL_STEP_9,
    TEST_SUB_CMD_CAL_STEP_10,
} ext_cal_sub_cmd_id_t;

#endif /* __SILEAD_CA_H__ */

