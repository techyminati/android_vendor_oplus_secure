/********************************************************************************************
** Copyright (c) 2021 - 2029 OPLUS Mobile Comm Corp., Ltd.
**
** File: fp_type.h
** Description: define base type for fp
** Version: 1.0
** Date : 2021-3-29
** Author: wangzhi(Kevin) wangzhi12
** TAG: BSP.Fingerprint.Basic
** -----------------------------Revision History: -----------------------?
**  <author>      <date>            <desc>
**  Zhi.Wang   2021/03/29        create the file
*********************************************************************************************/

#ifndef FP_ENUM_H
#define FP_ENUM_H

#ifdef __cplusplus
extern "C" {
#endif

/*[vendor begin]*/
/****************************** define begin ******************************/
#ifndef NULL
#define NULL 0
#endif

#ifndef UNUSE
#define UNUSE(x) ((void)(x))
#endif

#ifndef _IN
#define _IN
#endif

#ifndef _OUT
#define _OUT
#endif

#ifndef MAX_FINGERS_PER_USER
#define MAX_FINGERS_PER_USER 5
#endif

#define PATH_LEN (128)
#define TA_ID_LEN (128)
#define TA_STR_LEN (32)
#define CMD_RESERVERD_LEN (32)
#define MAX_MODULE_NAME_LEN (64)
#define MAX_FILE_NAME_LENGTH (128)
#define SEND_TA_COMMAND_MAX_ERR_COUNT (5)
#define MAX_IO_BUFFER_LENGTH (32)
/*[vendor end]*/

#define ENROLL_TIMEOUT_MS (10*60*1000)
#define MAX_RETRY_INDEX (3)
#define MAX_FP_VALUE_LEN (32)
#define QSEE_HMAC_KEY_MAX_LEN (256)
#define MAX_DCS_TA_DATA_LEN (512)
#define SAFE_CAST_CMD_BUF(type, buf, size) (sizeof(type) == size ? (type*)buf : NULL)
/****************************** define end ******************************/

/****************************** enum begin ******************************/
typedef enum fp_module_id {
    FP_MODULE_BIO          = 0,
    FP_MODULE_SENSOR       = 1,
    FP_MODULE_FPCORE       = 2,
    FP_MODULE_ALGO         = 3,
    FP_MODULE_PRIVATE      = 4,
    FP_MODULE_PRODUCT_TEST = 5,
    FP_MODULE_DUMP         = 6,
    FP_MODULE_INJECT       = 7,
} fp_module_id_t;

typedef enum fp_cmd_id {
    FP_CMD_SENSOR_INIT,
    //init
    FP_CMD_FPCORE_INIT             = 2000,
    FP_CMD_FPCORE_DEINIT           = 2001,
    FP_CMD_FPCORE_SET_ACTIVE_GROUP = 2002,
    FP_CMD_FPCORE_GET_FEATURE      = 2003,
    FP_CMD_FPCORE_CAPTURE_IMG      = 2004,

    //enroll
    FP_CMD_FPCORE_PRE_ENROLL    = 2005,
    FP_CMD_FPCORE_ENROLL        = 2006,
    FP_CMD_FPCORE_ENROLL_DOWN   = 2007,
    FP_CMD_FPCORE_ENROLL_IMAGE  = 2008,
    FP_CMD_FPCORE_SAVE_TEMPLATE = 2009,
    FP_CMD_FPCORE_ENROLL_POST   = 2010,

    //auth
    FP_CMD_FPCORE_GET_AUTHENTICATOR_ID = 2011,
    FP_CMD_FPCORE_AUTHENTICATE         = 2012,
    FP_CMD_FPCORE_AUTHENTICATE_DOWN    = 2013,
    FP_CMD_FPCORE_AUTHENTICATE_COMPARE = 2014,
    FP_CMD_FPCORE_AUTHENTICATE_STUDY   = 2015,
    FP_CMD_FPCORE_AUTHENTICATE_FINISH  = 2016,

    //common
    FP_CMD_FPCORE_REMOVE          = 2017,
    FP_CMD_FPCORE_ENUMERATE       = 2018,
    FP_CMD_FPCORE_CANCEL          = 2019,
    FP_CMD_FPCORE_SET_HMACKEY     = 2020,
    FP_CMD_FPCORE_CUSTOMENC       = 2021,
    FP_CMD_FPCORE_GET_DCSINFO     = 2022,
    FP_CMD_FPCORE_SET_SENSOR_MODE = 2023,
    FP_CMD_PRODUCT_SEND_CMD       = 2024,

    // dump
    FP_CMD_DUMP_GET_DATA       = 2025,
    FP_CMD_DUMP_GET_FILE_COUNT = 2026,
    FP_CMD_DUMP_GET_IMG_SIZE   = 2027,

    // inject
    FP_CMD_INJECT_SWITCH_MODE = 2028,
    FP_CMD_INJECT_IMAGE       = 2029,
    FP_CMD_INJECT_CALI        = 2030,

    // config
    FP_CMD_FPCORE_INIT_TA_CONFIG = 2100,
    FP_CMD_FPCORE_GET_TA_CONFIG  = 2101,
    FP_CMD_FPCORE_ENROLL_FINISH  = 2102,
    FP_CMD_PRIVATE_INIT          = 3000,
} fp_cmd_id_t;

typedef enum fp_return_type {
    FP_ERROR   = -1,
    FP_SUCCESS = 0,

    FP_TA_ERR_NULL_PTR                = 100,
    FP_TA_ERR_NOT_INIT_TA             = 101,
    FP_TA_ERR_INVALID_TA_CMD          = 102,
    FP_TA_ERR_INVALID_MOUDULE_ID      = 103,
    FP_TA_ERR_INVALID_CMD_ID          = 104,
    FP_TA_ERR_INVALID_TOKEN_CHALLENGE = 105,
    FP_TA_ERR_INVALID_TOKEN_VERSION   = 106,
    FP_TA_ERR_INVALID_TOKEN_HMAC      = 107,
    FP_TA_ERR_UNTRUSTED_ENROLL        = 108,

    FP_TA_ERR_GLOBAL_DB_PATH           = 200,
    FP_TA_ERR_FILE_NOT_FOUND           = 201,
    FP_TA_ERR_GLOBAL_SIZE_FILE_PATH    = 202,
    FP_TA_ERR_GLOBAL_DB_FILE_NOT_FOUND = 203,
    FP_TA_ERR_OPEN_SIZE_FILE           = 204,
    FP_TA_ERR_READ_SIZE_FILE           = 205,
    FP_TA_ERR_OPEN_GLB_DB              = 206,
    FP_TA_ERR_MEMORY_NOT_ENOUGH        = 207,
    FP_TA_ERR_READ_GLB_DB              = 208,
    FP_TA_ERR_GLB_DB_SIZE_NOT_FOUND    = 209,
    FP_TA_ERR_GLB_DB_SIZE_FILE_PATH    = 210,

    FP_TA_ERR_GLB_DB_SIZE_NOT_CORRECT = 211,
    FP_TA_ERR_NOT_FOUND_GID_FINGER    = 212,
    FP_TA_ERR_FINGER_NUMBERS_IS_ZERO  = 213,
    FP_TA_ERR_OPEN_FINGER_DATA        = 214,
    FP_TA_ERR_LOAD_FINGER_SIZE_DATA   = 215,

    FP_TA_ERR_JOIN_TPL_PATH                = 216,
    FP_TA_ERR_JOIN_TPL_BAK_PATH            = 217,
    FP_TA_ERR_WRITE_TPL                    = 218,
    FP_TA_ERR_WRITE_BAK_TPL                = 219,
    FP_TA_ERR_CREAT_GLOBAL_DB_MEM          = 220,
    FP_TA_ERR_WRITE_SIZE_FILE              = 221,
    FP_TA_ERR_ADD_FINGER_GROUP             = 222,
    FP_TA_ERR_ADD_FINGER_LIST              = 223,
    FP_TA_ERR_GLB_SIZE                     = 224,
    FP_TA_ERR_SAVE_GLB_MEM                 = 225,
    FP_TA_ERR_PACK_GLB_DB_FAIL1            = 226,
    FP_TA_ERR_PACK_GLB_DB_FAIL2            = 227,
    FP_TA_ERR_MALLOC_FINGER_GROUP          = 228,
    FP_TA_ERR_FINGER_ALREADY_FULL          = 229,
    FP_TA_ERR_LOAD_GLB_DB_MEM              = 230,
    FP_TA_ERR_READ_GLB_DB_SIZE_NOT_CORRECT = 231,
    FP_TA_ERR_SAVE_FINGER1                 = 232,
    FP_TA_ERR_SAVE_FINGER2                 = 233,
    FP_TA_ERR_NOT_HAVE_FINGER_GROUP        = 234,
    FP_TA_ERR_NOT_MATCHED_FINGER_GROUP     = 235,
    FP_TA_ERR_COMPARE_CHECKSUM             = 236,
    FP_TA_ERR_MUTI_DATA_PARA               = 237,
    FP_TA_ERR_READ_MULTI_DATA              = 238,
    FP_TA_ERR_WRITE_MULTI_DATA             = 239,
    FP_TA_ERR_DIFF_GLB_DB_CHECKSUM         = 240,
    FP_TA_ERR_DIFF_TEMPLATE_CHECKSUM       = 241,
    FP_TA_ERR_SECURE_CHECKSUM              = 242,
    FP_TA_ERR_QS_BUFFER_NOT_ENOUGH         = 251,
    FP_TA_ERR_TA_CONFIG                    = 252,
    FP_TA_ERR_TA_VALUE_ID                  = 253,
    FP_TA_ERR_TA_VALUE_TYPE                = 254,
    FP_TA_ERR_TA_VALUE_LEN                 = 255,
    FP_TA_ERR_CONFIG_ITEM_SIZE             = 256,
    FP_TA_ERR_CONFIG_UNKNOW_TYPE           = 257,
    FP_TA_ERR_CP_CALI_DATA                 = 258,
    FP_TA_ERR_PACK_TEMPLATE                = 259,
    FP_TA_ERR_REMOVE_GID_NOT_MATCH         = 260,
    FP_TA_ERR_GENERATE_FID                 = 261,
    FP_TA_ERR_ENUMERATE_PARA               = 262,
    FP_TA_ERR_CALI_DATA_NOT_READY          = 263,
    FP_TA_ERR_CALI_ADDR_NULL               = 264,
    FP_TA_ERR_CALI_DATA_TO_VND             = 265,
    FP_TA_ERR_SWITCH_INJECT_MODE           = 266,
    FP_TA_ERR_INJECT_IMAGE                 = 267,
    FP_TA_ERR_ENROLL_TIMEOUT               = 268,

    //sdk api return vaule
    FP_SDK_RETURN_TYPE_START               = 5000,
    FP_SDK_RETURN_ERR_COMMON_FAILED        = 5001,
    FP_SDK_RETURN_ERR_MEMORY_NOT_ENOUGH    = 5002,
    FP_SDK_RETURN_ERR_SPI_CRC_FAIL         = 5003,
    FP_SDK_RETURN_ERR_INVALID_INPUT        = 5004,

    //sdk api result value
    FP_SDK_ERR_COMMON_FAILED               = 5050,
    //fp_capture_image
    FP_SDK_ERR_NO_NEED_TO_CAPTURE          = 5100,
    FP_SDK_ERR_NEED_RETRY_CAPTURE          = 5101,
    //getfeature
    FP_SDK_ERR_BRIGHTNESS_UNSTABLE         = 5104,
    //enroll process
    FP_SDK_ERR_ENROLL_TOO_SIMILAR          = 5200,
    FP_SDK_ERR_ENORLL_ALREADY_ENROLLED     = 5201,
    FP_SDK_ERR_ENORLL_NEED_CANCEL          = 5202,
    //auth compare
    FP_SDK_ERR_COMPARE_FAIL                = 5300,
    FP_SDK_ERR_COMPARE_FAIL_AND_NOT_RETRY  = 5301,
    FP_SDK_ERR_ACQUIRED_PARTIAL            = 5302,
    FP_SDK_ERR_ACQUIRED_IMAGER_DIRTY       = 5303,
    FP_SDK_ERR_ACQUIRED_IMAGER_LOW_QUALITY = 5304,
    //init
    FP_SDK_ERR_CALIDATA_NOT_EXIST          = 5400,
    FP_SDK_ERR_CALIDATA_CRC_FAIL           = 5401,
    FP_SDK_ERR_CALIDATA_SENSORID_NOT_MATCH = 5402,
    FP_SDK_ERR_HARDWARE_ID_NOT_MATCH       = 5403,
    FP_SDK_ERR_HARDWARE_BROKEN             = 5404,
    FP_SDK_ERR_ALGO_INIT_FAIL              = 5405,

    //hal
    FP_DEVICE_OPEN_ERROR             = 10000,
    FP_DEVICE_CLOSE_ERROR            = 10001,
    FP_DEVICE_FILE_DESCRIPTION_ERROR = 10002,
    FP_DEVICE_IOTCL_ERROR            = 10003,
    FP_DEVICE_OPEN_TP_ERROR          = 10004,
    FP_ERR_VND_CMD                   = 10005,
    FP_ERR_VND_FUNC                  = 10006,
    FP_ERR_SEND_CMD_TO_TA            = 10007,
    FP_ERR_FACTORY_TEST_INIT         = 10008,
    FP_ERR_READ_SENSOR_TYPE          = 10101,
    FP_ERR_READ_SPOT_CONFIG          = 10102,
    FP_ERR_CONFIG_CONTENT            = 10103,
    FP_ERR_CONFIG_NODE_PROJECT       = 10104,
    FP_ERR_CONFIG_NODE_FP_ID         = 10105,
    FP_ERR_CONFIG_NODE_LCD           = 10106,
    FP_ERR_NOT_REALIZATION           = 10107,
    FP_ERR_FINGERUP_TOO_FAST         = 10108,
    FP_HAL_AUTH_NEED_RETRY           = 10109,
    FP_ERR_TA_DEAD                   = 10110,
    FP_ERR_CONFIG_NODE_LOCKSTATE     = 10111,
    FP_ERR_NULL_PTR                  = 11000,

    //netlink error
    FP_NETLINK_ERROR_BASE   = 11001,
    FP_NETLINK_SENDMSG_FAIL = 11002,
    FP_NETLINK_WAIT_TIMEOUT = 11003,

    FP_DUMP_PARAM_INVALID     = 12001,
    FP_DUMP_MKDIR_FAIL        = 12002,
    FP_DUMP_FILE_OPEN_FAIL    = 12003,
    FP_DUMP_FILE_WRITE_FAIL   = 12004,
    FP_DUMP_FILE_FIND_FAIL    = 12005,
    FP_DUMP_DIR_OPEN_FAIL     = 12006,
    FP_DUMP_FILE_READ_FAIL    = 12007,
    FP_DUMP_FILE_MANAGER_FAIL = 12008,
    FP_CALI_DATA_PATH_FAIL    = 12009,
    FP_CALI_DATA_NAME_FAIL    = 12010,
    FP_CREATE_CALI_DIR_FAIL   = 12011,
    FP_WRITE_CALI_DATA_FAIL   = 12012,
    FP_CALI_DATA_NOT_EXIST    = 12013,
    FP_DUMP_OVER              = 12014,

    FP_SMOKING_Files_NOT_EXIST = 13000,
    FP_SMOKING_Files_IS_DAMAGE = 13001,

    FP_SELECT_CHIPID_ERROR       = 13002,
    FP_SELECT_LOAD_SPI_TA        = 13003,
    FP_SELECT_NOT_ADAPTER_SENSOR = 13004,
    FP_MAX,
} fp_return_type_t;

typedef enum fp_hw_authenticator_type {
    FP_HW_AUTH_NONE        = 0,
    FP_HW_AUTH_PASSWORD    = (int)(1 << 0),
    FP_HW_AUTH_FINGERPRINT = (int)(1 << 1),
    FP_HW_AUTH_ANY         = (int)(4294967295U),
} fp_hw_authenticator_type_t;

typedef enum fp_operation { FP_ENROLL = 0, FP_AUTH = 1 } fp_operation_t;

typedef enum finger_status_type {
    FP_FINGER_UP   = 0,
    FP_FINGER_DOWN = 1,
} finger_status_type_t;

/*[vendor begin]*/
typedef enum fp_enroll_type {
    FP_UNLOCK  = 0,
    FP_PAYMENT = 1,
    FP_AUTOSMOKING = 2,
    FP_DEFAULT,
} fp_enroll_type_t;
typedef fp_enroll_type_t fp_auth_type_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef enum dump_stage {
    DUMP_STAGE_1_GET_FILE_COUNT = 1,
    DUMP_STAGE_2_GET_DATA_INFO,
    DUMP_STAGE_3_GET_DATA_BUF,
    DUMP_STAGE_4_FREE_DATA,
} dump_stage_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef enum fp_mode {
    FP_MODE_NONE = 0,
    FP_MODE_CALI,
    FP_MODE_ENROLL,
    FP_MODE_AUTH,
    FP_MODE_FACTORY_TEST,
    FP_MODE_DEBUG,
    FP_MODE_FARFRR,
    FP_MODE_DUMP_TEST,
    FP_MODE_TEST,
    FP_MODE_CANCEL,
    FP_MODE_APP_DATA,
    FP_MODE_MAX,
} fp_mode_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef enum fp_screen_status {
    FP_SCREEN_ON,
    FP_SCREEN_OFF,
    FP_SCREEN_MAX,
} fp_screen_status_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef enum fp_sensor_mode {
    FP_SENSOR_IDLE = 0,
    FP_SENSOR_CAPTURE,
    FP_SENSOR_SLEEP,
    FP_SENSOR_DEEP_SLEEP,
    FP_SENSOR_DEAULT
} fp_sensor_mode_t;
/*[vendor end]*/

/*[vendor begin]*/
typedef enum lcd_type {
    SUMSANG_AD097 = 100,
    BOE_AD097     = 101,
    LCD_TYPE_MAX,
} lcd_type_t;
/*[vendor end]*/

typedef enum fp_sensor_type {
    FP_SENSOR_TYPE_G_G3   = 100,
    FP_SENSOR_TYPE_G_G5   = 101,
    FP_SENSOR_TYPE_J_0302 = 200,
    FP_SENSOR_TYPE_J_0301 = 201,
    FP_SENSOR_TYPE_MAX,
} fp_sensor_type_t;

typedef enum fp_dcs_auth_result_type {
    DCS_AUTH_FAIL                 = 0,
    DCS_AUTH_SUCCESS              = 1,
    DCS_AUTH_TOO_FAST_NO_IMGINFO  = 2,
    DCS_AUTH_TOO_FAST_GET_IMGINFO = 3,
    DCS_AUTH_OTHER_FAIL_REASON    = 4,
    DCS_DEFAULT_AUTH_RESULT_INFO,
} fp_dcs_auth_result_type_t;

typedef enum fp_dcs_event_type {
    DCS_INTI_EVENT_INFO         = 0,
    DCS_AUTH_EVENT_INFO         = 1,
    DCS_SINGLEENROLL_EVENT_INFO = 2,
    DCS_ENROLL_EVENT_INFO       = 3,
    DCS_SPECIAL_EVENT_INFO      = 4,
    DCS_DEFAULT_EVENT_INFO,
} fp_dcs_event_type_t;

/*[vendor begin]*/
typedef enum fp_value_id {
    FP_VALUE_UFF = 1000,
    FP_VALUE_TEMPLATE_VERSION,  // int
    FP_VALUE_GID,  // int
    FP_VALUE_FID,  // unsigned int
    FP_VALUE_LCD,  // int
    FP_VALUE_PROJECT,  // char [128]
    FP_VALUE_SENSOR_ID,  // char [128]
    FP_VALUE_TP_INFO,  // struct fp_tp_info_t
    FP_VALUE_VND = 20000,
    FP_VALUE_VND_ENROLL_TOTAL_NUMBER,
} fp_value_id_t;

typedef enum fp_value_type {
    FP_VALUE_TYPE_MIN = 10,
    FP_VALUE_TYPE_STRUCT,
    FP_VALUE_TYPE_INT8,
    FP_VALUE_TYPE_UINT8,
    FP_VALUE_TYPE_INT16,
    FP_VALUE_TYPE_UINT16,
    FP_VALUE_TYPE_INT32,
    FP_VALUE_TYPE_UINT32,
    FP_VALUE_TYPE_INT64,
    FP_VALUE_TYPE_UINT64,
    FP_VALUE_TYPE_MAX = 100,
} fp_value_type_t;

/*[vendor begin]*/
typedef enum fp_injection_mode {
    FP_INJECTION_OFF,
    FP_INJECTION_ON,
} fp_injection_mode_t;
/*[vendor end]*/

/* private init */
typedef enum {
    FP_SPI_UNKNOW = -1,
    FP_SPI_TA     = 0,
    FP_GOODIX_TA  = 1,
    FP_JV0301_TA  = 2,
    FP_JV0302_TA  = 3,
    FP_SILEAD_TA  = 4,
    FP_EGIS_TA    = 5,
    FP_MAX_VENDOR_TA
} fp_ta_name_t;

/*[vendor begin]*/
enum TEMPLATE_STUDY_FLAG {
    E_NO_NEED_STUDY_TEMPLATE = 0,
    E_NEED_STUDEY_TEMPLATE   = 1,
};

typedef enum {
    E_CODE_NONE                = -1,
    E_CODE_RESET               = 0,
    E_CODE_POWER_ON            = 1,
    E_CODE_POWER_OFF           = 2,
    E_CODE_HBM_ON              = 3,
    E_CODE_HBM_OFF             = 4,
    E_CODE_SET_LOW_BRIGHTNESS  = 5,
    E_CODE_SET_MID_BRIGHTNESS  = 6,
    E_CODE_SET_HIGH_BRIGHTNESS = 7,
    E_CODE_SAVE_CALI_DATA      = 8,
    E_CODE_GET_CAPTURE_IMG     = 9,
    E_CODE_FACTORY_TEST_DUMP   = 10,
    E_CODE_CALI_CONTINUE       = 11,
    E_MAX_CODE
} vnd_cmd_t;

typedef enum fp_send_cmdid {
    E_FACTORY_AUTO_TEST           = 0,
    E_FACTORY_QTY_TEST            = 1,
    E_FACTORY_GET_ALGO_INFO       = 2,
    E_FACTORY_GET_SENSOR_QRCODE   = 3,
    E_FACTORY_AGING_TEST          = 4,
    E_FACTORY_CALI_FLESH_BOX_TEST = 5,
    E_FACTORY_CALI_BLACK_BOX_TEST = 6,
    E_FACTORY_CALI_CHART_TEST     = 7,
    // reserved for factory test item
    E_FACTORY_MAX_NUM,
    E_FACTORY_CALI_ABORT          = 50,
    // must not to be changed
    E_CAPTURE_TOOL_GET_IMG = 100,
    //for inject mode
    E_INJECT_SWITCH_MODE = 200,
    E_INJECT_CALI_DATA   = 201,
    E_INJECT_IMG_DATA    = 202,

    //dcsinfo
    E_DCS_INTI_EVENT_INFO         = 300,
    E_DCS_AUTH_EVENT_INFO         = 301,
    E_DCS_SINGLEENROLL_EVENT_INFO = 302,
    E_DCS_ENROLL_EVENT_INFO       = 303,
    E_DCS_SPECIAL_EVENT_INFO      = 304,
    E_DCS_DEFAULT_EVENT_INFO,
}fp_send_cmdid_t;

/*[vendor end]*/

/****************************** enum end ******************************/
#ifdef __cplusplus
}
#endif
#endif  // FP_ENUM_H

