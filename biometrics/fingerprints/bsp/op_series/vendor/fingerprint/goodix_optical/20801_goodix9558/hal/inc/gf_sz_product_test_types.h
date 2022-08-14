/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _GF_SZ_PRODUCT_TEST_TYPES_H_
#define _GF_SZ_PRODUCT_TEST_TYPES_H_

#include <stdint.h>
#include "gf_sz_types.h"
#include "gf_product_test_types.h"

#define TEST_ALGO_VERSION_LEN 32

#define GF_SZ_PRODUCT_TEST_CROP_SIZE 50
#define GF_SZ_PRODUCT_TEST_3P_CROP_SIZE 90
#define GF_SZ_PRODUCT_TEST_DISCARD_MAX_VALUE_NUM 3
#define GF_SZ_PRODUCT_TEST_SELECT_MAX_VALUE_NUM 9
#define GF_SZ_PRODUCT_TEST_START_EXPOSURE_TIME 43
#define GF_SZ_PRODUCT_TEST_STEP_EXPOSURE_TIME 5
#define GF_SZ_PRODUCT_TEST_STOP_EXPOSURE_TIME 135
#define GF_SZ_PRODUCT_TEST_FIND_MAX_VALUE_STEP_EXPOSURE_TIME 50
#define GF_SZ_PRODUCT_TEST_FIND_BRIGHTEST_POINT_FRAME_AVG_NUM 4
#define GF_SZ_PRODUCT_TEST_FIND_BRIGHTEST_POINT_EXPOSURE_TIME 40
#define GF_SZ_PRODUCT_TEST_NORMAL_FRAME_AVG_NUM 1
#define GF_SZ_PRODUCT_TEST_NORMAL_PGA_GAIN 0
#define GF_SZ_PRODUCT_TEST_CALIBRATION_TARGET_VALUE 900
#define GF_SZ_PRODUCT_TEST_LIMIT_VALUE_PERCENTAGE (0.75)
#define GF_SZ_PRODUCT_TEST_CALIBRATION_TARGET_PGA_GAIN 3
#define GF_SZ_PRODUCT_TEST_CROP_DATA_LEN (GF_SZ_PRODUCT_TEST_CROP_SIZE * GF_SZ_PRODUCT_TEST_CROP_SIZE)
#define GF_SZ_PRODUCT_TEST_3P_CROP_DATA_LEN (GF_SZ_PRODUCT_TEST_3P_CROP_SIZE * GF_SZ_PRODUCT_TEST_3P_CROP_SIZE)
#define GF_SZ_PRODUCT_TEST_MAX_CALI_COUNT ((GF_SZ_PRODUCT_TEST_STOP_EXPOSURE_TIME - GF_SZ_PRODUCT_TEST_START_EXPOSURE_TIME) / GF_SZ_PRODUCT_TEST_STEP_EXPOSURE_TIME + 1)
#define GF_SZ_PRODUCT_TEST_MAX_VALUE_AVERAGE_COUNT 3
#define GF_SZ_PRODUCT_TEST_MAX_LINEAR_VALUE 600
#define GF_SZ_PRODUCT_TEST_AE_PARAM_THRESHOLD 30
#define GF_SZ_PRODUCT_TEST_AE_PARAM_CHECK_THRESHOLD (GF_SZ_PRODUCT_TEST_AE_PARAM_THRESHOLD + 10)
#define GF_SZ_PRODUCT_TEST_DE_DARK_START_COL 165
#define GF_SZ_PRODUCT_TEST_DE_DARK_STOP_COL 175

#define GF_SZ_PRODUCT_TEST_MAX_EXPOSURE_TIME 135
#define GF_SZ_PRODUCT_TEST_MIN_EXPOSURE_TIME 43
#define GF_SZ_PRODUCT_TEST_EXPOSURE_TIME_STEP 5
#define EXPOSURE_TIME_PARAM_SIZE ((GF_SZ_PRODUCT_TEST_MAX_EXPOSURE_TIME - GF_SZ_PRODUCT_TEST_MIN_EXPOSURE_TIME) / GF_SZ_PRODUCT_TEST_EXPOSURE_TIME_STEP + 1)
#define GF_SZ_PRODUCT_TEST_EXPOSURE_TIME_ACCURACY (0.1)
#define GF_SZ_PRODUCT_TEST_MAX_V_BLANK 9000
#define GF_SZ_PRODUCT_TEST_MIN_V_BLANK 4
#define GF_SZ_PRODUCT_TEST_MIN_H_BLANK 3000


#define GF_SZ_FACTORY_TEST_FRAME_NUM 39
#define GF_SZ_FACTORY_TEST_FRAME_DATA_SIZE (GF_SZ_FACTORY_TEST_FRAME_NUM * GF_CHIP_RAWDATA_LEN)  // 38 rawdata
#define TABLE(cmdID) {cmdID, #cmdID}

enum {
    GF_SZ_CMD_TEST_RETRIEVE_IMAGE = GF_PRODUCT_TEST_CMD_MAX + 1,
    GF_SZ_CMD_TEST_INIT,
    GF_SZ_CMD_UNTRUST_ENROLL_ENABLE,
    GF_SZ_CMD_TEST_GET_BMD_DATA,
    GF_SZ_CMD_TEST_FIND_SENSOR,
    GF_SZ_CMD_TEST_SEND_CAPTURE_PARAM,
    GF_SZ_CMD_TEST_GET_VERSION,
    GF_SZ_CMD_TEST_ENROLL_TEMPLATE_COUNT,
    GF_SZ_CMD_TEST_GET_CONFIG,
    GF_SZ_CMD_TEST_FUSION,
    GF_SZ_CMD_TEST_RAWDATA_PREVIEW,
    GF_SZ_CMD_TEST_SLEEP,
    GF_SZ_CMD_TEST_K_B_CALIBRATION,
    GF_SZ_CMD_TEST_UPDATE_CFG,
    GF_SZ_CMD_TEST_UPDATE_FW,
    GF_SZ_CMD_FACTORY_TEST_GET_SENSOR_INFO,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_IMAGE,
    GF_SZ_CMD_FACTORY_TEST_FINSH_EXPO_AUTO_CALIBRATION,
    GF_SZ_CMD_FACTORY_TEST_INIT,
    GF_SZ_CMD_FACTORY_TEST_EXIT,
    GF_CMD_FACTORY_TEST_CAPTURE_DARK_BASE,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_H_DARK,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_L_DARK,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_H_FLESH,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_L_FLESH,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_CHART,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_CHECKBOX,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_LOCATION_IMAGE,
    GF_SZ_CMD_FACTORY_TEST_PERFORMANCE,
    GF_SZ_CMD_FACTORY_TEST_CALIBRATE,
    GF_SZ_CMD_FACTORY_TEST_SPI,
    GF_SZ_CMD_FACTORY_RESET,
    GF_SZ_CMD_FACTORY_MT_CHECK,
    GF_SZ_CMD_FACTORY_KPI,
    GF_SZ_CMD_FACTORY_TEST_CAPTURE_IMAGE_MANUAL,
    GF_SZ_CMD_FACTORY_TEST_PERFORMANCE_SIMPLE,
    GF_SZ_CMD_FACTORY_TEST_GET_MT_INFO,
    GF_SZ_CMD_FACTORY_TEST_LOCAL_AREA_SAMPLE,
    GF_SZ_CMD_FACTORY_TEST_SIMPLE_INIT,
    GF_SZ_CMD_FACTORY_TEST_SIMPLE_EXIT,
    GF_SZ_CMD_FACTORY_TEST_SIMPLE_CALI_BASE,
    GF_SZ_CMD_FACTORY_TEST_SIMPLE_CALI_PERFORMANCE,
    GF_SZ_CMD_FACTORY_TEST_SIMPLE_CALI_SAVE_DATA,
    GF_SZ_CMD_FACTORY_TEST_SAMPLE_CALI_CALIBRATE,
    GF_SZ_CMD_FACTORY_TEST_GET_EXPO_LEVEL,
    GF_SZ_CMD_FACTORY_TEST_CHECK_AE_PARAM,
    GF_SZ_CMD_FACTORY_TEST_FIND_BRIGHTEST_POINT3P,
    GF_SZ_CMD_FACTORY_TEST_FIND_BRIGHTEST_POINT,
    GF_SZ_CMD_FACTORY_TEST_FIND_TARGET_EXPO_TIME,
    GF_SZ_CMD_TEST_MAX
};

enum {
    DATA_TYPE_FUSION,
    DATA_TYPE_LONG_EXPO,
    DATA_TYPE_SHORT_EXPO,
};

enum {
    BASE_TYPE_CALISIG,
    BASE_TYPE_FILTERDATA,
    BASE_TYPE_KUCHHOLEMASK,
    BASE_TYPE_PREPROCESSPARAMS,
    BASE_TYPE_SPOSMAP,
    BASE_TYPE_BASEUNIT,
    BASE_TYPE_BLACK0,
    BASE_TYPE_KBCALIBRATA,
    BASE_TYPE_WTMAP,
    BASE_TYPE_BASE,
    BASE_HIGH_BLACK_BASE,
    BASE_TYPE_MPPARAMS,
    BASE_TYPE_FRAMENUM,
    BASE_TYPE_SLOPBASE,
    BASE_TYPE_STPBBUFFER
};

typedef enum FRAME_TYPE_T {
    DARK_BASE_INDEX = 21,
    H_DARK_INDEX = 5,
    L_DARK_INDEX = 0,
    H_FLESH_INDEX = 15,
    L_FLESH_INDEX = 10,
    CHART_INDEX = 34,
    CHECKBOX_INDEX = 26,
    LOCATION_IMAGE_INDEX = 39
} FRAME_TYPE;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t i_enable;
} gf_sz_untrust_enroll_t;

typedef struct GF_SZ_TEST_BEGIN_ENROLL {
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint8_t system_auth_token_version;
    uint8_t hat[GF_HW_AUTH_TOKEN_LEN];
} gf_sz_begin_enroll_t;

typedef struct GF_SZ_TEST_PARAM {
    gf_cmd_header_t cmd_header;

    uint32_t scale_ratio;
    uint32_t long_frame_avg_num;
    uint32_t long_pga_gain;

    uint32_t short_exposure_time;
    uint32_t short_frame_avg_num;
    uint32_t short_pga_gain;

    uint32_t unlowerthresh;
    uint32_t unhighthresh;

    float fusionratio;

    uint32_t preprocess;

    uint32_t rect_x;
    uint32_t rect_y;
    uint32_t rect_width;
    uint32_t rect_height;
    uint32_t rect_bmp_col;
    uint32_t rect_bmp_row;

    uint32_t base_calibrate_switch;
    uint32_t ldc_switch;

    uint32_t tnr_thresh;
    uint32_t tnr_switch;
    uint32_t lpf_switch;
    uint32_t lsc_switch;
    uint32_t enhance_level;
    uint32_t rawdata_process_switch;
} gf_sz_capture_param_t;

typedef struct GF_SZ_TEST_RAWDATA {
    uint16_t data[IMAGE_BUFFER_LEN];
    uint32_t data_len;
    uint32_t capture_time;
    uint32_t row;
    uint32_t col;
    uint32_t expo_time;
    uint32_t short_pga_gain;
} gf_sz_test_rawdata_t;

typedef struct GF_SZ_FIND_SENSOR_IMAGE {
    gf_cmd_header_t cmd_header;
    gf_sz_test_rawdata_t raw_data;
    uint32_t row;
    uint32_t col;
} gf_sz_find_sensor_image_t;

typedef struct GF_SZ_TEST_BMP_DATA {
    gf_cmd_header_t cmd_header;
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
    uint32_t width;
    uint32_t height;
} gf_sz_test_bmp_data_t;

typedef struct GF_SZ_TEST_GET_VERSION {
    gf_cmd_header_t cmd_header;
    uint8_t algo_version[ALGO_VERSION_INFO_LEN];
    uint8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    uint8_t fake_version[ALGO_VERSION_INFO_LEN];
    uint8_t fw_version[FW_VERSION_INFO_LEN];
    uint8_t tee_version[TEE_VERSION_INFO_LEN];
    uint8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t package_version[TA_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    uint8_t factory_algo_version[TEST_ALGO_VERSION_LEN];
    uint8_t ad_version[GF_AD_VERSION_LEN];
    uint8_t lens_type[GF_LENS_TYPE_LEN];
} gf_sz_test_get_version_t;

typedef struct GF_SZ_TEST_ENROLL_TEMPLAE_COUNT {
    gf_cmd_header_t cmd_header;
    uint32_t template_count;
} gf_sz_test_enroll_template_count_t;

typedef struct GF_SZ_TEST_GET_CONFIG {
    gf_cmd_header_t cmd_header;
    uint32_t bmp_col;
    uint32_t bmp_row;
    uint32_t sensor_col;
    uint32_t sensor_row;
    uint32_t scale_ratio;
} gf_sz_test_get_config_t;

typedef struct GF_SZ_TEST_GET_MT {
    gf_cmd_header_t cmd_header;
    gf_sensor_mt_info_t mt_info;
} gf_sz_test_get_mt_t;

typedef struct GF_SZ_TEST_IMAGE_CROP {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} gf_sz_image_crop_rect_t;

typedef struct GF_SZ_TEST_FUSION_PREVIEW {
    gf_cmd_header_t cmd_header;
    uint8_t fusion_data[IMAGE_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    gf_sz_test_rawdata_t rawdata;
    uint8_t weak_process_data[IMAGE_BUFFER_LEN];
    uint16_t KBcalidate[IMAGE_BUFFER_LEN];
    uint8_t image_mask_data[IMAGE_BUFFER_LEN];
    uint32_t crop_width;
    uint32_t crop_height;
    uint32_t row;
    uint32_t col;
    uint32_t weak_data_row;
    uint32_t weak_data_col;
    gf_sz_image_crop_rect_t image_crop_rect;
    uint32_t expo_time;
    uint32_t short_pga_gain;
} gf_sz_fusion_preview_t;

typedef struct GF_SZ_ORIGIN_BMP_DATA {
    gf_cmd_header_t cmd_header;
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    uint32_t width;
    uint32_t height;
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t frame_index;
    uint16_t KBcalidate[IMAGE_BUFFER_LEN];
    uint8_t image_mask_data[IMAGE_BUFFER_LEN];
    uint32_t row;
    uint32_t col;
    uint32_t weak_data_row;
    uint32_t weak_data_col;
    gf_sz_image_crop_rect_t image_crop_rect;
} gf_sz_bmp_data_t;

typedef struct GF_SZ_IMAGE_KPI {
    gf_cmd_header_t cmd_header;
    uint32_t o_image_quality;
    uint32_t o_valid_area;
    uint32_t o_expo_time;
} gf_sz_image_kpi_t;

typedef struct GF_SZ_CAPUTURE_IMAGE_MANUAL {
    gf_cmd_header_t cmd_header;
    uint32_t image_quality;
    uint32_t valid_area;
} gf_sz_capture_image_manual;

typedef struct GF_SZ_TEST_RAWDATA_PREVIEW {
    gf_cmd_header_t cmd_header;
    gf_sz_test_rawdata_t rawdata_long;
    gf_sz_test_rawdata_t rawdata_short;
    uint32_t row;
    uint32_t col;
    uint32_t expo_time;
    uint32_t short_pga_gain;
} gf_sz_rawdata_preview_t;

typedef struct GF_SZ_TEST_SLEEP {
    gf_cmd_header_t cmd_header;
} gf_sz_test_sleep_t;

typedef struct GF_SZ_TEST_UPDATE_CFG {
    gf_cmd_header_t cmd_header;
    uint8_t update_mode;
    uint32_t cfg_file_len;
    uint8_t cfg_file_data[MAX_CFG_FILE_LEN];
} gf_sz_test_update_cfg_t;

typedef struct GF_SZ_TEST_UPDATE_FW {
    gf_cmd_header_t cmd_header;
    uint32_t fw_file_len;
    uint8_t fw_file_data[MAX_FW_FILE_LEN];
} gf_sz_test_update_fw_t;

typedef enum GF_SZ_K_B_CLIBRATION_STEP {
    recode_brightlight_dark_base_step = 0,
    recode_darklight_dark_base_step,
    recode_brightlight_bg_base_step,
    recode_darklight_bg_base_step,
} gf_sz_k_b_calibrate_step_t;

typedef struct GF_SZ_K_B_CALIBTATION {
    gf_cmd_header_t cmd_header;
    gf_sz_k_b_calibrate_step_t k_b_step;
} gf_sz_k_b_calibrate_t;

typedef struct GF_SZ_TEST_INIT {
    gf_cmd_header_t cmd_header;
} gf_sz_test_init_t;

typedef struct GF_FACTORY_CALIBRATE {
    gf_cmd_header_t cmd_header;
    uint8_t calicam_params[300 * 1024];  // big enough to hold struct ST_CALI_CAM_PARAM_Shenzhen

    // result params below
    int32_t nSNoise;
    int32_t nTNoise;
    int32_t nFleshTouchDiff;
    int32_t nLightLeakRatio;
    int32_t nFovLeft;
    int32_t nFovRight;
    int32_t nFovUp;
    int32_t nFovDown;
    int32_t nRelativeIlluminance[4];
    int32_t nIllumMaxX;
    int32_t nIllumMaxY;
    int32_t nScale;
    double rms;
    double cameraMatrix[9];
    double distCoeffs[5];
    int32_t nCropWidth;
    int32_t nCropHeight;
    int32_t nBadPointNum;
    int32_t nClusterNum;
    int32_t nPixelOfLargestBadCluster;
    int8_t reliability_test_flag;
} gf_factory_calibrate_t;

typedef struct GF_FACTORY_PERFORMANCE {
    gf_cmd_header_t cmd_header;
    uint32_t lpf_enabled;

    int32_t nP2P;
    int32_t nNoise;
    int32_t nSSNR;
    int32_t nMeanRidge;
    int32_t nMeanValley;
    int32_t nSharpness;
    int32_t nSharpnessAll;
    int32_t nChartTouchDiff;
    int32_t nChartContrast;
    // TODO add more result
} gf_factory_performance_t;

typedef struct GF_SZ_TEST_CAPTURE_IMAGE {
    gf_cmd_header_t cmd_header;
    uint8_t frame_avg_num;
    uint16_t pga_gain;
    uint16_t exposure_time;
    gf_sz_test_rawdata_t rawdata;
} gf_sz_test_capture_image_t;

typedef struct {
    int32_t nMeanFleshMaxBrightness;
    int32_t nMeanFleshLowBrightness;
    int32_t nMeanBlackMaxBrightness;
    int32_t nMeanBlackLowBrightness;
    int32_t nMeanDark;
    int32_t nMeanChart;
    int32_t nStdFleshMaxBrightness;
    int32_t nStdFleshLowBrightness;
    int32_t nStdBlackMaxBrightness;
    int32_t nStdBlackLowBrightness;
    int32_t nStdDark;
    int32_t nStdChart;
} gf_factory_rawdata_check_result_t;

typedef struct GF_SZ_CHIP_INFO {
    uint8_t uid[GF_SENSOR_ID_LEN];
    uint32_t col;
    uint32_t row;
    uint32_t dark_start_col;
    uint32_t dark_len;
    uint32_t expo_value;
    uint32_t lens_type;
    uint32_t factory_type;
    uint32_t expo_time;
    uint32_t ae_min_time;
    uint32_t ae_max_time;
    uint32_t ae_expo_start_time;
    uint8_t chip_type_otp;
} gf_sz_chip_info_t;

typedef struct GF_SZ_GET_SENSOR_INFO {
    gf_cmd_header_t cmd_header;
    gf_sz_chip_info_t chip_info;
} gf_sz_get_chip_info_t;

typedef struct {
    int32_t maxBadPointNum;
    int32_t maxClusterNum;
    int32_t maxPixelOfLargestBadCluster;
    float maxLightNoiseT;
    float maxLightNoiseS;

    float minFleshTouchDiff;
    int32_t minFovArea;
    float maxLightLeakRatio;
    float minRelativeIlluminance;
    float maxScaleRatio;
    float minScaleRatio;
    int32_t minMaskCropArea;

    float minSSNR;
    float minShapeness;
    float minP2P;
    float minChartConstrast;
    int32_t nEffRegRad;
    int32_t nEffRegRad2;
    int32_t nSPMTBadPixThd;
    int32_t nSPMTBadPixThd2;
    int32_t nSaturatPixHighThd;
    int32_t chartDirectionThd;
    int32_t chartDirectionTarget;
    float maxScreenStructRatio;
    int32_t max_expo_time;
    int32_t min_expo_time;
    float maxCenterOffset;
    float maxSimpleScaleRatio;
    float minSimpleScaleRatio;
    int32_t maxSimpleBadPointNum;
    int32_t maxSimplePixelOfLargestBadCluster;
    int32_t nSimpleSPMTBadPixThd;
    int32_t nSimpleSPMTBadPixThd2;
    float maxDarkNoiseT;
    float maxDarkNoiseS;
    int32_t fovAreaTooBigSwitch;
    float FOVAreaAdjustTargetValue;
    int32_t maxPixelOfLargestOrientBadCluster;
    int32_t maxTiltLevel;
    float simpleCaliScaleOffset;
    float max_raw_ratio;
    float min_raw_ratio;
    uint32_t min_raw_avg;
} factory_config_t;

typedef struct GF_SZ_FACTORY_TEST_BALIBRATE {
    // result params below
    int32_t nSNoise;
    int32_t nTNoise;
    int32_t nFleshTouchDiff;
    int32_t nLightLeakRatio;
    int32_t nFovLeft;
    int32_t nFovRight;
    int32_t nFovUp;
    int32_t nFovDown;
    int32_t nRelativeIlluminance[4];
    int32_t nIllumMaxX;
    int32_t nIllumMaxY;
    int32_t nScale;
    double rms;
    double cameraMatrix[9];
    double distCoeffs[5];
    int32_t nCropWidth;
    int32_t nCropHeight;
    int32_t nBadPointNum;
    int32_t nClusterNum;
    int32_t nPixelOfLargestBadCluster;
    uint8_t badpoint_data[IMAGE_BUFFER_LEN];
    int8_t reliability_test_flag;
    uint8_t center_point_x;
    uint8_t center_point_y;
    int32_t nStructRatio;
    int32_t centerOffsetLevel;
    gf_factory_rawdata_check_result_t check_result;
    uint8_t factory_algo_version[TEST_ALGO_VERSION_LEN];
    uint16_t ChartImg[GF_CHIP_RAWDATA_LEN];
    uint32_t ChartImg_Len;
    int32_t stcol;
    int32_t strow;
    uint8_t pkuchHoleMask[GF_CHIP_RAWDATA_LEN];
    gf_sz_preprocess_params_t preprocess_params;
    uint16_t pusbase[GF_CHIP_RAWDATA_LEN];
    int16_t pusBaseUnit[GF_CHIP_RAWDATA_LEN];
    int16_t pusKr[GF_CHIP_RAWDATA_LEN];
    gf_sz_chip_info_t sensor;
    gf_error_t errorcode;
    factory_config_t factory_config;
    int32_t nDarkSNoise;
    int32_t nDarkTNoise;
    int32_t nPixelOfLargestOrientBadCluster;
    uint8_t orient_badpoint_data[IMAGE_BUFFER_LEN];
    int32_t tiltLevel;
} gf_sz_factory_calibrate_t;

enum {
    EXPO_AUTO_CALIBRATION_SUCCESS = 0,
    EXPO_AUTO_CALIBRATION_FAIL,
    EXPO_AUTO_CALIBRATION_STOP,
    EXPO_AUTO_CALIBRATION_MAX,
};

typedef struct GF_FACTORY_CAPTURE_IMAGE {
    gf_cmd_header_t cmd_header;
    uint32_t width;
    uint32_t height;
    uint16_t img_data[512 * 1024];
    uint32_t img_data_size;
} gf_factory_capture_image_t;

typedef struct GF_SZ_GT_FACTORY_CALIBRATE {
    gf_cmd_header_t cmd_header;
    gf_sz_factory_calibrate_t factory_calibrate;
} gf_sz_get_factory_calibrate_t;

typedef struct GF_SZ_FACTORY_PERFORMANCE {
    uint32_t lpf_enabled;

    int32_t nP2P;
    int32_t nNoise;
    int32_t nSSNR;
    int32_t nMeanRidge;
    int32_t nMeanValley;
    int32_t nSharpness;
    int32_t nSharpnessAll;
    int32_t nChartTouchDiff;
    int32_t nChartContrast;
    int32_t nChartDirection;
    int32_t nChartDirectionDiff;
    uint16_t ChartImg[GF_CHIP_RAWDATA_LEN];
    uint32_t ChartImg_Len;
    uint32_t col;
    uint32_t row;
    // TODO add more result
    gf_factory_rawdata_check_result_t check_result;
    gf_sz_chip_info_t sensor;
    gf_error_t errorcode;
    factory_config_t factory_config;
} gf_sz_factory_performance_t;

typedef struct GF_SZ_GET_FACTORY_PERFOEMANCE {
    gf_cmd_header_t cmd_header;
    gf_sz_factory_performance_t factory_performance;
} gf_sz_get_factory_performance_t;

typedef struct {
    int32_t o_maxBadPointNum;
    int32_t o_maxClusterNum;
    int32_t o_maxPixelOfLargestBadCluster;
    float o_maxLightNoiseT;
    float o_maxLightNoiseS;

    float o_minFleshTouchDiff;
    int32_t o_minFovArea;
    float o_maxLightLeakRatio;
    float o_minRelativeIlluminance;
    float o_maxScaleRatio;
    float o_minScaleRatio;
    int32_t o_minMaskCropArea;

    float o_minSSNR;
    float o_minShapeness;
    float o_minP2P;
    float o_minChartConstrast;
    int32_t o_nEffRegRad;
    int32_t o_nEffRegRad2;
    int32_t o_nSPMTBadPixThd;
    int32_t o_nSPMTBadPixThd2;
    int32_t o_nSaturatPixHighThd;
    int32_t o_chartDirectionThd;
    int32_t o_chartDirectionTarget;
    float o_maxScreenStructRatio;
    int32_t o_max_expo_time;
    int32_t o_min_expo_time;
    float o_maxCenterOffset;
    float o_maxSimpleScaleRatio;
    float o_minSimpleScaleRatio;
    int32_t o_maxSimpleBadPointNum;
    int32_t o_maxSimplePixelOfLargestBadCluster;
    int32_t o_nSimpleSPMTBadPixThd;
    int32_t o_nSimpleSPMTBadPixThd2;
    float o_maxDarkNoiseT;
    float o_maxDarkNoiseS;
    int32_t o_fovAreaTooBigSwitch;
    float o_FOVAreaAdjustTargetValue;
    int32_t o_maxPixelOfLargestOrientBadCluster;
    int32_t o_maxTiltLevel;
    float o_simpleCaliScaleOffset;
} gf_sz_factory_config_t;

typedef struct {
    uint32_t i_display_vendor;
} gf_sz_factory_config_input_t;

typedef struct GF_FACTORY_TEST_RECORD {
    gf_sz_factory_calibrate_t factoryCaliData;
    gf_sz_factory_performance_t factoryPerformance;
    char TimeStamp[256];
    uint8_t sensor_uid[GF_SENSOR_UID_LEN];
    int32_t result;
    int32_t error;
} gf_factory_test_record_t;

typedef struct FACTORY_INIT {
    gf_cmd_header_t cmd_header;
    gf_sz_factory_config_t o_factory_config;
    gf_sz_factory_config_input_t i_factory_config;
} gf_sz_factory_init;

typedef struct FACTORY_EXIT {
    gf_cmd_header_t cmd_header;
} gf_sz_factory_exit;

typedef struct GF_SZ_IOC_CHIP_INFO {
    uint8_t vendor_id;
    uint8_t mode;
    uint8_t operation;
    uint8_t reserved[5];
} gf_sz_ioc_chip_info;

typedef struct GF_SZ_CROP_PIXEL {
    uint16_t x;
    uint16_t y;
    uint16_t value;
} gf_sz_crop_pixel_t;

typedef struct GF_SZ_EXPO_AUTO_CALIBRATION {
    gf_sz_chip_info_t sensor;
    gf_sz_crop_pixel_t select_pixel[GF_SZ_PRODUCT_TEST_SELECT_MAX_VALUE_NUM];
    uint16_t target_rawdata_value;
    uint16_t target_exposure_time;
} gf_sz_expo_auto_calibration_t;

typedef struct GF_SZ_TEST_EXPOSURE_PARAM {
    gf_cmd_header_t cmd_header;
    uint16_t exposure_time;
    uint32_t flag;
    uint32_t mode;
} gf_sz_test_exposure_param_t;

typedef struct GF_SZ_EXPOSURE_RESULT {
    uint16_t target_exposure_time;
    uint16_t target_average_value;
    uint16_t target_rawdata_value;
    uint16_t rawdata_threshold;
    uint16_t start_exposure_time;
    uint16_t start_average_value;
} gf_sz_exposure_result_t;

typedef struct GF_SZ_EXPOSURE_TEST_DATA {
    uint16_t rawdata;
    uint16_t row_dark_data;
} gf_sz_exposure_test_data_t;

typedef struct GF_SZ_EXPOSURE_DATA {
    gf_sz_chip_info_t sensor;
    gf_sz_exposure_test_data_t
    find_ae_data[GF_SZ_PRODUCT_TEST_SELECT_MAX_VALUE_NUM];
    gf_sz_exposure_test_data_t
    check_ae_data[GF_SZ_PRODUCT_TEST_SELECT_MAX_VALUE_NUM];
    gf_sz_exposure_result_t ae_result;
    uint16_t rawdata[IMAGE_BUFFER_LEN];
} gf_sz_exposure_data_t;

typedef struct GF_SZ_EXPOSURE_TIME_PARAM {
    uint32_t exposure_time;
    uint32_t H_Blank;
    uint32_t V_Blank;
    uint32_t V_Delay;
    uint32_t H_Valid;
    uint32_t V_Valid;
} gf_sz_exposure_time_param_t;

typedef struct GF_SZ_SPI_TEST {
    gf_cmd_header_t cmd_header;
    uint32_t mcu_id;
    uint32_t pmic_id;
    uint32_t sensor_id;
    uint32_t sensor_version;
    uint32_t flash_id;
    int8_t otp_info[13];
} gf_sz_spi_test_t;

typedef enum {
    STATE_INIT,
    STATE_ENROLL,
    STATE_AUTHENTICATE,
    FUSION_PREVIEW,
    RAWDATA_PREVIEW,
    STATE_CANCEL,
    STATE_FARR_ENROLL,
    STATE_FARR_AUTHENTICATE,
    FACTORY_RESET_INT,
    STATE_KPI,
    STATE_CALI_BASE,
    STATE_CALI_AUTO_CALIBRATION,
    STATE_CALI_STOP_AUTO_CALIBRATION,
} WORK_STATE;

typedef struct {
    int32_t cmdID;
    const char *cmdIDString;
} sz_product_test_cmd_table_t;

typedef enum {
    SAMPLE_CALI_MOVE_FINGER = 100,
    SAMPLE_CALI_DUPLICATE,
    SAMPLE_CALI_BASE_SUCCESS,
    SAMPLE_CALI_BASE_FAILED,
    SAMPLE_CALI_ALGO_ERROR,
    SAMPLE_CALI_RAW_DATA_CHECK_FAILED,
    SAMPLE_CALI_SIMILAR_FINGER_BASE,
} SAMPLE_CALI_STATE;

typedef enum {
    SAMPLE_PERFORMANCE_FAIL = 1000,
    SAMPLE_PERFORMANCE_RAW_DATA_CHECK_FAILED,
    SAMPLE_PERFORMANCE_ALL_SUCCESS,
    SAMPLE_PERFORMANCE_BAD_POINT_FAIL,
    SAMPLE_PERFORMANCE_SCALERATION_FAIL,
    SAMPLE_PERFORMANCE_SCREEN_STRUCT_FAIL,
} SAMPLE_PERFORMANCE_RESULT;

typedef enum {
    SAMPLE_CALI_HEAD_TYPE_FINGER = 0,
    SAMPLE_CALI_HEAD_TYPE_FIRST_FRAME = 10,
    SAMPLE_CALI_HEAD_TYPE_BLACK_SHELTER = 20,
}SAMPLE_CALI_HEAD_TYPE;

typedef enum {
    SAMPLE_CALI_DARK_IMAGE_START_IDNEX = 1,
    SAMPLE_CALI_DARK_IMAGE_END_IDNEX = 5,

    SAMPLE_CALI_CHART_IMAGE_START_IDNEX = 6,
    SAMPLE_CALI_CHART_IMAGE_END_IDNEX = 13,

    SAMPLE_CALI_DARK_BASE_START_IDNEX = 14,
    SAMPLE_CALI_DARK_BASE_END_IDNEX = 14,
} SAMPLE_CALI_INDEX;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_sz_test_rawdata_t finger_rawdata;
    SAMPLE_CALI_STATE state;
    uint32_t finger_base_remaining_num;
    uint16_t width;
    uint16_t height;
} gf_sample_cali_base_t;

typedef struct {
    uint32_t needTest;
    uint32_t head_type;
    uint32_t frame_start_index;
    uint32_t frame_end_index;
    SAMPLE_CALI_STATE cali_state;
} gf_sample_raw_check_param_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    SAMPLE_PERFORMANCE_RESULT test_result;
    uint16_t fingerAverage[GF_CHIP_RAWDATA_LEN];
    uint16_t pusBase[GF_CHIP_RAWDATA_LEN];
    uint16_t pusBaseUnit[GF_CHIP_RAWDATA_LEN];
    uint16_t chart_data[GF_CHIP_RAWDATA_LEN];
    uint8_t pkuchHoleMask[GF_CHIP_RAWDATA_LEN];
    uint16_t fakeHblack[GF_CHIP_RAWDATA_LEN];
    uint16_t width;
    uint16_t height;
    uint16_t nHoleWidth;
    uint16_t nHoleHeight;
    uint32_t chartDistance;
} gf_sample_cali_performance_t;

typedef struct GF_SZ_TEST_CHECK_AE_PARAM {
    gf_cmd_header_t cmd_header;
    gf_sz_expo_auto_calibration_t i_expo_auto_calibration;
    uint8_t i_frame_avg_num;
    uint16_t i_pga_gain;
    uint16_t o_average_value;
    uint32_t o_row_dark_average[500];
    gf_sz_test_rawdata_t o_rawdata;
}gf_sz_test_check_ae_param_t;

typedef struct GF_SZ_TEST_FIND_BRIGHTEST_POINT3P{
    gf_cmd_header_t cmd_header;
    gf_sz_expo_auto_calibration_t i_expo_auto_calibration;
    uint8_t i_frame_avg_num;
    uint16_t i_pga_gain;
    gf_sz_test_rawdata_t o_rawdata;
}gf_sz_test_find_brightest_point3p_t;

typedef struct GF_SZ_TEST_FIND_BRIGHTEST_POINT{
    gf_cmd_header_t cmd_header;
    gf_sz_expo_auto_calibration_t i_expo_auto_calibration;
    uint8_t i_frame_avg_num;
    uint16_t i_pga_gain;
    gf_sz_test_rawdata_t o_rawdata;
}gf_sz_test_find_brightest_point_t;

typedef struct GF_SZ_TEST_FIND_TARGET_EXPO_TIME{
    gf_cmd_header_t cmd_header;
    gf_sz_expo_auto_calibration_t i_expo_auto_calibration;
    uint8_t i_frame_avg_num;
    uint16_t i_pga_gain;
    uint16_t o_average_value;
    uint32_t o_row_dark_average[500];
    gf_sz_test_rawdata_t o_rawdata;
}gf_sz_test_find_target_expo_time_t;


#endif /* _GF_SZ_PRODUCT_TEST_TYPES_H_ */
