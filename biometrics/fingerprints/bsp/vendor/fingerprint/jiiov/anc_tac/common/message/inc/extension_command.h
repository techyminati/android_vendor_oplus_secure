#ifndef __EXTENSION_COMMAND_H__
#define __EXTENSION_COMMAND_H__

#include "anc_type.h"


typedef enum {
    ANC_CMD_EXTENSION_NONE = 0,
    ANC_CMD_EXTENSION_GET_IMAGE,
    ANC_CMD_EXTENSION_SET_TIME,
    ANC_CMD_EXTENSION_GET_AUTH_TOKEN,

    ANC_CMD_EXTENSION_FT_INIT,
    ANC_CMD_EXTENSION_FT_MODULE_TEST,
    ANC_CMD_EXTENSION_FT_TRANSMIT_IMAGE,
    ANC_CMD_EXTENSION_FT_CALIBRATION_BASE_SUM,
    ANC_CMD_EXTENSION_FT_CAL_MEAN,
    ANC_CMD_EXTENSION_FT_LENS_TEST,
    ANC_CMD_EXTENSION_FT_DEFECT_TEST,
    ANC_CMD_EXTENSION_FT_CALIBRATION_BASE_IMAGE,
    ANC_CMD_EXTENSION_FT_MARK_POSITION,
    ANC_CMD_EXTENSION_FT_BLACK_TEST,
    ANC_CMD_EXTENSION_FT_SIGNAL_NOISE_TEST,
    ANC_CMD_EXTENSION_FT_REQ_MFACTOR_TEST,
    ANC_CMD_EXTENSION_FT_SAVE_CALIBRATION_DATA,
    ANC_CMD_EXTENSION_RESET,
    ANC_CMD_EXTENSION_FT_DEINIT,
    ANC_CMD_EXTENSION_INIT_DATA_COLLOCT,
    ANC_CMD_EXTENSION_AUTH_RESULT_COLLOCT,
    ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT,
    ANC_CMD_EXTENSION_ENROLL_END_COLLOCT,
    ANC_CMD_EXTENSION_HEART_BEAT_COLLOCT,

    ANC_CMD_LOAD_CALIBRAION_DATA,
#ifdef FP_JIIOV_TEMPLATE_UPDATE_ENABLE
    ANC_CMD_EXTENSION_UPDATE_FILE,
#endif
    /// add mmi new test item
    ANC_CMD_EXTENSION_FT_MODULE_TEST_V2,
    ANC_CMD_EXTENSION_AUTO_EXPO_CALIBRATION,
    ANC_CMD_EXTENSION_FIXED_EXPO_CALIBRATION,
    ANC_CMD_EXTENSION_FT_GET_CONFIG,
    ANC_CMD_EXTENSION_FT_CAPTURE_IMAGE_TO_CACHE,
    ANC_CMD_EXTENSION_FT_TRANSMIT_IMAGE_FROM_CACHE,
    ANC_CMD_EXTENSION_FACTORY_TEST,

    ANC_CMD_EXTENSION_MAX
}ANC_COMMAND_EXTENSION_TYPE;

typedef enum {
    ANC_CMD_EXTENSION_FTEO_NONE,
    ANC_CMD_EXTENSION_FTEO_STARTUP,
    ANC_CMD_EXTENSION_FTEO_SET_HBM_ON,
    ANC_CMD_EXTENSION_FTEO_SET_HBM_OFF,
    ANC_CMD_EXTENSION_FTEO_SET_CALIBRATION_SUCCEESS,
    ANC_CMD_EXTENSION_FTEO_SET_CALIBRATION_FAIL,
    ANC_CMD_EXTENSION_FTEO_INIT,
    ANC_CMD_EXTENSION_FTEO_DEINIT,
    ANC_CMD_EXTENSION_FTEO_MAX
}ANC_FACTORY_TEST_EXTERNAL_OPERATION_TYPE;

typedef struct {
    uint32_t is_need_adjust;
    uint32_t capture_image_count;
    uint32_t reserved[8];
}__attribute__((packed)) AncFTOprExtensionEntity;

typedef struct {
    uint32_t mmi_algo_version;
    AncFTOprExtensionEntity white_entity;
    AncFTOprExtensionEntity black_entity;
    AncFTOprExtensionEntity stripe_entity;
    uint32_t snr_capture_image_total_count;
    uint32_t snr_capture_white_image_count;
    uint32_t snr_capture_stripe_image_count;
    uint32_t reserved[40];
} __attribute__((packed)) AncFTConfig;

typedef struct {
    uint32_t data;
    uint32_t gid;
    uint32_t path_size;
    uint8_t path[256];
}__attribute__((packed)) AncExtensionUpdateFileInfo;

typedef struct {
    uint32_t gid;
    uint8_t path[256];
}__attribute__((packed)) AncMultiUserUpdateFileInfo;

typedef struct {
    uint32_t image_size;
    uint32_t width;
    uint32_t height;
    int32_t exposure_time;
    uint32_t exposure_reg_val;
    uint32_t image_serial_number;
    uint32_t env_light_value;
    uint32_t image_ready_time;
    uint8_t *p_image_data;
} __attribute__((packed)) AncExtensionImage;

typedef struct {
    int32_t dx;
    int32_t dy;
    int32_t len;
}__attribute__((packed)) AncFTSensorTiltResult;

typedef struct {
    uint32_t x;
    uint32_t y; 
}__attribute__((packed)) AncFTMarkPositioningResult;

typedef struct {
    uint32_t percentage;
    uint32_t mean;
} __attribute__((packed)) AncFTImageMeanInfoResult;

typedef struct {
    AncFTImageMeanInfoResult mean_info;
    int32_t exp_time;
} __attribute__((packed)) AncFTWhitePreventInfo;

typedef struct {
    uint32_t x;
    uint32_t y;
} __attribute__((packed)) AncFTOpticalCenterResult;

typedef struct {
    uint32_t ratio;
} __attribute__((packed)) AncFTHiFreqPeakResult;

typedef struct {
    uint32_t top_left;
    uint32_t top_right;
    uint32_t bottom_left;
    uint32_t bottom_right;
    uint32_t highest;
    uint32_t lowest;
} __attribute__((packed)) AncFTShadingResult;

typedef struct {
    uint32_t size;
} __attribute__((packed)) AncFTFovAreaResult;

typedef struct {
    AncFTOpticalCenterResult oc_result;
    AncFTHiFreqPeakResult hf_result;
    AncFTShadingResult sh_result;
    AncFTFovAreaResult fov_result;
}__attribute__((packed)) AncFTLensTestResult;

typedef struct {
    uint32_t total;
    uint32_t lines;
    uint32_t clusters;
    uint32_t max_cluster_size;
}__attribute__((packed)) AncFTDefectResult;

typedef struct {
    AncFTImageMeanInfoResult mean_info;
    int32_t screen_signal;
    uint32_t screen_leak_ratio;
    int32_t exp_time;
}__attribute__((packed)) AncFTBlackAlgoResult;

typedef struct {
    int32_t coef_k0;
    int32_t coef_k1;
    int32_t coef_k2;
    int32_t coef_k3;
}__attribute__((packed)) AncFTDistortionResult;

// typedef struct {
//     int32_t exp_time;
// }__attribute__((packed)) AncFTBlackExpDataResult;

typedef struct {
    int32_t snr[9], highest, lowest;
    int32_t signal[9], highest_signal, lowest_signal;
    int32_t noise[9], highest_noise, lowest_noise;
}__attribute__((packed)) AncFTSnrResult;

typedef struct {
    uint32_t line_pair_count;
    int32_t rotation_degree;
    uint32_t frequency_difference;
} __attribute__((packed)) AncFTChartAnalyseResult;

typedef struct {
    int32_t expo_time;
    int32_t expo_max_val;
} __attribute__((packed)) AncFTExpoResult;

typedef struct {
    long long capture_time[3];
    long long extract_time[3];
    long long enroll_time;
    long long verify_time[3];
    long long verify_time_all;
    long long study_time;
    long long UIready_time;
    long long authtotaltime;
} AncFTAlgoTimeInfo;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t image_size;
    uint8_t *p_image_data;
} __attribute__((packed)) AncFTImageInfo;

typedef enum {
    FACTORY_TEST_START = 96,
    FACTORY_TEST_TRANSMIT_IMAGE_GET_COUNT = 97,
    FACTORY_TEST_TRANSMIT_IMAGE = 98,
    FACTORY_TEST_SETUP = 99,
} FACTORY_TEST_COMMAND_TYPE;

typedef enum {
    ANC_SENSOR_AUTO_EXPOSURE,
    ANC_SENSOR_FIXED_EXPOSURE,
} ANC_SENSOR_EXPOSURE_TYPE;

typedef struct {
    uint32_t remain_count;
    uint32_t image_size;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t image_bit;
    uint32_t image_valid_bit;
    uint8_t image_path[100];
    uint8_t image_name[100];
    uint8_t *p_image_buffer;
} __attribute__((packed)) AncFtTransmitImageInfo;

typedef struct {
    uint32_t command;
    uint32_t buffer_length;
    uint8_t *p_buffer;
} __attribute__((packed)) AncFactoryTestCommand;

typedef struct {
    uint32_t command;
    uint32_t operation_code;
    uint32_t need_transmit_image;
    uint32_t buffer_length;
    uint8_t *p_buffer;
} __attribute__((packed)) AncFactoryTestCommandRespond;

typedef struct {
    uint32_t command;
    uint32_t data;
    int64_t ca_time_ms;
    uint32_t array_size;
    uint8_t array[256];
    int32_t response;
    uint32_t image_index;
    uint32_t base_sum_arr[7];
    ANC_BOOL load_base_image_flag;
} __attribute__((packed)) AncExtensionCommand;


typedef struct {
    uint32_t data;
    uint32_t array_size;
    uint8_t array[256];
} __attribute__((packed)) AncExtensionCommandRespond;

#endif
