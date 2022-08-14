#define LOG_TAG "[ANC_COMMON][LogString]"

#include "anc_log_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "anc_log.h"


static char *gp_return_common_type_string[] = {
    "success",
    "fail",
    "fail memory",
    "fail buffer too small",
    "fail open file",
    "fail fstat file",
    "fail read file",
    "fail close file",
    "fail no dir",
    "fail spi open",
    "fail spi close",
    "fail spi read",
    "fail spi write",
    "fail spi write read",
    "fail hmac key",
    "fail load ta",
    "fail hw unavalable",
    "fail register sb",
    "fail prep sb secure read",
    "fail prep sb non secure read",
    "fail deregister sb",
    "fail wait time out",
    "fail cancel",
    "fail invalid command",
    "fail efuse read byte",
    "fail efuse write byte",
    "fail no calibration",
    "fail efuse empty",
    "fail efuse invalid",
    "fail crc check",
    "fail invalid param",
    "fail ta transmit",
    "fail load otp data",
};

static char *gp_return_capture_type_string[] = {
    "capture frame error",
    "capture packet error",
    "capture frame size not match",
    "capture read size too large",
    "capture buffer not enough",
    "capture finger move too fast",
    "capture abnormal exp",
    "capture low auto exp",
    "capture low env light",
    "capture high env light",
    "capture wait image timeout",
    "capture image crc error",
    "capture rd underflow",
    "capture finger up",
    "capture image check fail",
};

static char *gp_return_algo_common_type_string[] = {
    "algo no error",
    "algo invalid parameter",
    "algo state err",
    "algo alloc mem fail",
    "algo sdk timeout",
    "algo load model fail",
};

static char *gp_return_algo_extract_type_string[] = {
    "algo extract ok",
    "algo bad img",
    "algo low qty",
    "algo parital",
    "algo fake finger",
    "algo ghost finger",
    "algo extract fail",
};

static char *gp_return_algo_enroll_type_string[] = {
    "algo enroll finish",
    "algo enroll ok continue",
    "algo enroll same finger",
    "algo enroll duplicate",
    "algo enroll fail",
    "algo enroll NULL", // no 405
    "algo enroll low qty",
    "algo enroll get template fail",
    "enroll time out",
    "enroll fail",
};

static char *gp_return_algo_match_type_string[] = {
    "algo match ok",
    "algo match fail",
    "algo match fail stop retry",
    "algo match fail NULL", // no 503
    "algo match fail bad img",
    "algo match fail partial",
    "algo match fail fake",
    "algo match fail stop retry partial",
};

static char *gp_return_algo_db_type_string[] = {
    "db finger id invalid",
    "db cache incomplete",
    "db file incomplete",
    "db crc32 dismatch",
    "db enroll already full",
};

static char *gp_return_algo_ft_type_string[] = {
    "ft algo version dismatch",
    "ft transmit image fail",
    "ft save image fail",
    "ft lens high freq sinal test fail",
    "ft lens center offset test fail",
    "ft lens corner shading test fail",
    "ft lens for area test fail",
    "ft defect test fail",
    "ft install offset test fail",
    "ft prevent check fail",
    "ft screen signal check fail",
    "ft screen leak ratio check fail",
    "ft black exp time test fail",
    "ft signal noise test fail",
    "ft req mfactor test fail",
    "ft module test fail",
    "ft calibration test fail",
    "ft save calibration data fail",
    "ft install angle test fail",
    "ft chart angle check fail",
    "ft chart position check fail",
    "ft sw hw version match fail",
    "ft read chip version fail",
    "ft expo calibration fail",
    "ft detect distortion fail",
    "ft install tilt test fail",
    "ft lens id test fail",
};

static char *gp_return_algo_template_type_string[] = {
    "algo template load fail",
    "algo template save image fail",
    "algo template save feature fail",
    "algo template enroll full",
    "algo template state err",
    "algo template upgrade fail",
};

static char *gp_get_version_string = "GET_VERSION";
static char *gp_do_test_string = "DO_TEST";
static char *gp_test_share_buffer_string = "TEST_SHARE_BUFFER";
static char *gp_unknown_command_string = "Unknown";

// match with ANC_COMMAND_SENSOR_TYPE
static char *gp_sensor_command_type_string[] = {
    "SENSOR_NONE",
    "SENSOR_INIT",
    "SENSOR_DEINIT",
    "SENSOR_SET_PARAM",
    "SENSOR_GET_PARAM",
    "SENSOR_CAPTURE_IMAGE",
    "SENSOR_GET_CHIP_ID",
    "SENSOR_SET_POWER_MODE",
};

// match with ANC_SENSOR_POWER_MODE
static char *gp_sensor_power_mode_string[] = {
    "SENSOR_NORMAL",
    "SENSOR_WAKEUP",
    "SENSOR_SLEEP",
    "SENSOR_POWER_ON",
    "SENSOR_POWER_ON_INIT",
    "SENSOR_POWER_OFF",
    "SENSOR_HW_RESET",
    "SENSOR_INIT",
    "SENSOR_WAKEUP_RESET",
    "SENSOR_HW_RESET_INIT",
    "SENSOR_WAKEUP_SCAN",
};

// match with ANC_SENSOR_CMD_TYPE
static char *gp_sensor_param_type_string[] = {
    "SET_EXPOSURE_TIME",
    "GET_EXPOSURE_TIME",
    "RESTORE_EXPOSURE_TIME",
    "GET_EXPOSURE_REG_VAL",
    "SET_IMAGE_SIZE",
    "GET_IMAGE_SIZE",
    "WRITE_REGISTER",
    "READ_REGISTER",
    "SET_FRAME_FUSION_NUM",
    "VC_SETTING",
    "SET_RETRY_EXPOSURE_TIME",
    "SET_EXPO_CONFIG_THRESHOLD",
    "GET_ENV_LIGHT_VAL",
    "RESTORE_DEFAULT_IMAGE_SIZE",
    "SET_CALIBRATION_MODE",
    "SELF_TEST",
    "GET_TOTAL_EXPOSURE_TIME",
    "LOAD_CALIBRATION_DATA",
    "SET_EXPO_TARGET_VALUE",
    "GET_EXPO_TARGET_VALUE",
    "GET_PRODUCT_ID",
};

// match with ANC_SENSOR_CMD_TYPE
static char *gp_sensor_param_efuse_type_string[] = {
    "EFUSE_READ_MT_OPTICAL_CENTER",
    "EFUSE_READ_MT_CUSTOM_MODULE_INFO",
    "EFUSE_READ_MT_MI_LENS_ID",
    "EFUSE_READ_FT_INFO",
    "EFUSE_WRITE_FT_INFO",
    "EFUSE_READ_CHIP_PROTO_INFO",
    "EFUSE_CHECK",
    "EFUSE_GET_MT1_OTP_DATA",
    "EFUSE_GET_CP_OTP_DATA",
};

// match with ANC_COMMAND_ALGORITHM_TYPE
static char *gp_algorithm_command_type_string[] = {
    "ALGORITHM_NONE",
    "ALGORITHM_INIT",
    "ALGORITHM_DEINIT",
    "ALGORITHM_INIT_ENROLL",
    "ALGORITHM_ENROLL",
    "ALGORITHM_DEINIT_ENROLL",
    "ALGORITHM_INIT_VERIFY",
    "ALGORITHM_VERIFY",
    "ALGORITHM_EXTRACT_FEATURE",
    "ALGORITHM_ENROLL_FEATURE",
    "ALGORITHM_COMPARE_FEATURE",
    "ALGORITHM_FEATURE_STUDY",
    "ALGORITHM_DEINIT_VERIFY",
    "TEMPLATE_LOAD_DATABASE",
    "TEMPLATE_SET_ACTIVE_GROUP",
    "TEMPLATE_GET_AUTHENTICATOR_ID",
    "TEMPLATE_DELETE_FINGERPRINT",
    "TEMPLATE_GET_ALL_FINGERPRINTS_IDS",
    "ALGORITHM_GET_ENROLL_TOTAL_TIMES",
    "ALGORITHM_GET_VERSION",
    "ALGORITHM_GET_IMAGE_QUALITY_SCORE",
    "ALGORITHM_GET_HEART_BEAT_RESULT",
};

// match with ANC_COMMAND_TOKEN_TYPE
static char *gp_token_command_type_string[] = {
    "TOKEN_NONE",
    "TOKEN_SET_HMAC_KEY",
    "TOKEN_GET_ENROLL_CHALLENGE",
    "TOKEN_AUTHORIZE_ENROLL",
    "TOKEN_SET_AUTHENTICATE_CHALLENGE",
    "TOKEN_GET_AUTHENTICATE_RESULT",
};

// match with ANC_COMMAND_EXTENSION_TYPE
static char *gp_extension_command_type_string[] = {
    "EXTENSION_NONE",
    "EXTENSION_GET_IMAGE",
    "EXTENSION_SET_TIME",
    "EXTENSION_GET_AUTH_TOKEN",
    "EXTENSION_FT_INIT",
    "EXTENSION_FT_MODULE_TEST",
    "EXTENSION_FT_TRANSMIT_IMAGE",
    "EXTENSION_FT_CALIBRATION_BASE_SUM",
    "EXTENSION_FT_CAL_MEAN",
    "EXTENSION_FT_LENS_TEST",
    "EXTENSION_FT_DEFECT_TEST",
    "EXTENSION_FT_CALIBRATION_BASE_IMAGE",
    "EXTENSION_FT_MARK_POSITION",
    "EXTENSION_FT_BLACK_TEST",
    "EXTENSION_FT_SIGNAL_NOISE_TEST",
    "EXTENSION_FT_REQ_MFACTOR_TEST",
    "EXTENSION_FT_SAVE_CALIBRATION_DATA",
    "EXTENSION_RESET",
    "EXTENSION_FT_DEINIT",
    "EXTENSION_INIT_DATA_COLLOCT",
    "EXTENSION_AUTH_RESULT_COLLOCT",
    "EXTENSION_SINGLE_ENROLL_COLLOCT",
    "EXTENSION_ENROLL_END_COLLOCT",
    "EXTENSION_HEART_BEAT_COLLOCT",
    "EXTENSION_LOAD_CALIBRAION_DATA",
    "EXTENSION_FT_MODULE_TEST_V2",
    "EXTENSION_AUTO_EXPO_CALIBRATION",
    "EXTENSION_FIXED_EXPO_CALIBRATION",
    "EXTENSION_FT_GET_CONFIG",
    "EXTENSION_FT_CAPTURE_IMAGE_TO_CACHE",
    "EXTENSION_FT_TRANSMIT_IMAGE_FROM_CACHE",
    "EXTENSION_FACTORY_TEST",
    "EXTENSION_UPDATE_FILE",
};


// match with ANC_RETURN_TYPE
char *AncConvertReturnTypeToString(ANC_RETURN_TYPE return_type) {
    char *p_string = NULL;
    int string_array_size = 0;
    int array_index = return_type / 100;
    int inner_index = return_type % 100;

    switch (array_index) {
        case 0 :
            string_array_size = sizeof(gp_return_common_type_string)/sizeof(gp_return_common_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_common_type_string[inner_index];
            }
            break;
        case 1 :
            string_array_size = sizeof(gp_return_capture_type_string)/sizeof(gp_return_capture_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_capture_type_string[inner_index];
            }
            break;
        case 2 :
            string_array_size = sizeof(gp_return_algo_common_type_string)/sizeof(gp_return_algo_common_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_common_type_string[inner_index];
            }
            break;
        case 3 :
            string_array_size = sizeof(gp_return_algo_extract_type_string)/sizeof(gp_return_algo_extract_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_extract_type_string[inner_index];
            }
            break;
        case 4 :
            string_array_size = sizeof(gp_return_algo_enroll_type_string)/sizeof(gp_return_algo_enroll_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_enroll_type_string[inner_index];
            }
            break;
        case 5 :
            string_array_size = sizeof(gp_return_algo_match_type_string)/sizeof(gp_return_algo_match_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_match_type_string[inner_index];
            }
            break;
        case 6 :
            string_array_size = sizeof(gp_return_algo_db_type_string)/sizeof(gp_return_algo_db_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_db_type_string[inner_index];
            }
            break;
        case 7 :
            string_array_size = sizeof(gp_return_algo_ft_type_string)/sizeof(gp_return_algo_ft_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_ft_type_string[inner_index];
            }
            break;
        case 8 :
            string_array_size = sizeof(gp_return_algo_template_type_string)/sizeof(gp_return_algo_template_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_return_algo_template_type_string[inner_index];
            }
            break;
        default :
            break;
    }

    if (NULL == p_string) {
        p_string = "NULL";
        ANC_LOGE("return_type:%d, array_index:%d, string_array_size:%d, inner_index:%d",
                  return_type, array_index, string_array_size, inner_index);
    }

    return p_string;
}

char *AncConvertCommandIdToString(ANC_COMMAND_TYPE type, uint32_t id) {
    char *p_string = NULL;
    uint32_t string_array_size = 0;

    switch (type) {
        case ANC_CMD_GET_VERSION :
            p_string = gp_get_version_string;
            break;
        case ANC_CMD_SENSOR :
            string_array_size = sizeof(gp_sensor_command_type_string)/sizeof(gp_sensor_command_type_string[0]);
            if (id < string_array_size) {
                p_string = gp_sensor_command_type_string[id];
            }
            break;
        case ANC_CMD_ALGORITHM :
            string_array_size = sizeof(gp_algorithm_command_type_string)/sizeof(gp_algorithm_command_type_string[0]);
            if (id < string_array_size) {
                p_string = gp_algorithm_command_type_string[id];
            }
            break;
        case ANC_CMD_AUTH_TOKEN :
            string_array_size = sizeof(gp_token_command_type_string)/sizeof(gp_token_command_type_string[0]);
            if (id < string_array_size) {
                p_string = gp_token_command_type_string[id];
            }
            break;
        case ANC_CMD_EXTENSION :
            string_array_size = sizeof(gp_extension_command_type_string)/sizeof(gp_extension_command_type_string[0]);
            if (id < string_array_size) {
                p_string = gp_extension_command_type_string[id];
            }
            break;
        case ANC_CMD_DO_TEST :
            p_string = gp_do_test_string;
            break;
        case ANC_CMD_TEST_SHARE_BUFFER :
            p_string = gp_test_share_buffer_string;
            break;
        default :
            break;
    }

    if (NULL == p_string) {
        p_string = gp_unknown_command_string;
        ANC_LOGW("unknown command: type: %d, id: %d, string_array_size: %d", type, id, string_array_size);
    }

    return p_string;
}

static char *AncConvertSensorPowerModeToString(int32_t mode) {
    char *p_string = NULL;
    int32_t string_array_size = 0;

    string_array_size = sizeof(gp_sensor_power_mode_string)/sizeof(gp_sensor_power_mode_string[0]);
    if (mode < string_array_size) {
        p_string = gp_sensor_power_mode_string[mode];
    } else {
        p_string = gp_unknown_command_string;
        ANC_LOGW("unknown sensor power mode: %d, string_array_size: %d", mode, string_array_size);
    }

    return p_string;
}

static char *AncConvertSensorParamTypeToString(int32_t type) {
    char *p_string = NULL;
    int32_t string_array_size = 0;
    int32_t array_index = type / 100;
    int32_t inner_index = type % 100;

    switch (array_index) {
        case 0 :
            string_array_size = sizeof(gp_sensor_param_type_string)/sizeof(gp_sensor_param_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_sensor_param_type_string[inner_index];
            }
            break;
        case 1 :
            string_array_size = sizeof(gp_sensor_param_efuse_type_string)/sizeof(gp_sensor_param_efuse_type_string[0]);
            if (inner_index < string_array_size) {
                p_string = gp_sensor_param_efuse_type_string[inner_index];
            }
            break;
        default :
            break;
    }

    if (NULL == p_string) {
        p_string = gp_unknown_command_string;
        ANC_LOGE("unknown sensor param: type:%d, array_index:%d, string_array_size:%d, inner_index:%d",
                  type, array_index, string_array_size, inner_index);
    }

    return p_string;
}

char *AncConvertSensorParamToString(ANC_COMMAND_SENSOR_TYPE type, int32_t param) {
    char *p_string = NULL;

    switch (type) {
        case ANC_CMD_SENSOR_SET_POWER_MODE :
            p_string = AncConvertSensorPowerModeToString(param);
            break;
        case ANC_CMD_SENSOR_SET_PARAM :
        case ANC_CMD_SENSOR_GET_PARAM :
            p_string = AncConvertSensorParamTypeToString(param);
            break;
        default :
            p_string = gp_unknown_command_string;
            break;
    }

    return p_string;
}