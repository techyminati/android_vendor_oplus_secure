/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _GF_DELMAR_PRODUCT_TEST_TYPES_H_
#define _GF_DELMAR_PRODUCT_TEST_TYPES_H_

#include <stdint.h>
#include "gf_product_test_types.h"
#include "gf_delmar_types.h"

// for delmar location circle pic length(200 * 200)
#define LOCATION_CIRCLE_PIC_BUFFER_LEN    (200 * 200)
#define CALI_KB 0x00 //普通kb校准
#define CALI_SUB_KB 0x01 //减basekb校准
#define CALI_ADJ_SUB_KB 0x02//自适应减base校准

enum
{
    GF_CMD_TEST_PERFORMANCE_TESTING = GF_PRODUCT_TEST_CMD_MAX + 1,
    GF_CMD_TEST_LOCATION_CIRCLE_TESTING,
    GF_CMD_TEST_SPI,
    GF_CMD_TEST_RESET_INTERRUPT_PIN,
    GF_CMD_TEST_OTP_FLASH,
    GF_CMD_TEST_GET_SENSOR_INFO,
    GF_CMD_TEST_GET_OTP_INFO,
    GF_CMD_TEST_GAIN_TARGET,
    GF_CMD_TEST_AGE_TEST,
    GF_CMD_TEST_IMAGE_QUALITY,
    GF_CMD_TEST_GET_VERSION,
    GF_CMD_TEST_CAPTURE_IMAGE,
    GF_CMD_TEST_MORPHOTYPE,
    GF_CMD_TEST_MAX,
};

typedef enum
{
    OPERATION_STEP_COLLECT_NONE = 100,
    OPERATION_STEP_BASEDATA_DARK_COLLECT,
    OPERATION_STEP_BASEDATA_MIN_DARK_COLLECT,
    OPERATION_STEP_BASEDATA_MID_DARK_COLLECT,
    OPERATION_STEP_BASEDATA_MAX_DARK_COLLECT,
    OPERATION_STEP_BASEDATA_MIN_COLLECT,
    OPERATION_STEP_BASEDATA_MID_COLLECT,
    OPERATION_STEP_BASEDATA_MAX_COLLECT,
    OPERATION_STEP_CIRCLEDATA_COLLECT,
    OPERATION_STEP_CHARTDATA_COLLECT,
    OPERATION_STEP_CALCULATE_RESULT,  // 110
    OPERATION_STEP_GET_LOCATION_CIRCLE,
    OPERATION_STEP_CALCULATE_SIMPLIFIED_PERFORMANCE_INDICATORS,
    OPERATION_STEP_GET_KB_CALIBRATION,
    OPERATION_STEP_CALCULATE_GAIN_COLLECT, // 114
    OPERATION_STEP_CALCULATE_GAIN,
    OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT, // 116
    OPERATION_STEP_CALCULATE_GAIN_TWO, // 117
    OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT, // 118
    OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN, // 119
    OPERATION_STEP_FINISHED = 120,
    OPERATION_STEP_CALCULATE_MID_BRIGHTNESS_GAIN_COLLECT = 600,
    OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_GAIN_COLLECT,
    OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_GAIN,
    OPERATION_STEP_MID_BRIGHTNESS_FRESH_COLLECT,
    OPERATION_STEP_LOW_BRIGHTNESS_FRESH_COLLECT,
    OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_DIGIT_GAIN,
    OPERATION_STEP_SAVE_KB_CALI = 800,
    OPERATION_STEP_SAVE_PGA_GAIN = 801,
    OPERATION_STEP_FLESH_MAX_COLLECT = 803,
    OPERATION_STEP_COLLECT_MAX = 999
} gf_test_performance_testing_operation_step_t;

typedef enum
{
    OPERATION_STEP_NOISE_BASEDATA_DARK_COLLECT = 1,
    OPERATION_STEP_NOISE_BASEDATA_MAX_COLLECT,
    OPERATION_STEP_NOISE_CALCULATE_RESULT
} gf_test_noise_operation_step_t;


typedef struct
{
    gf_cmd_header_t header;
    uint8_t i_sensor_index;
    int32_t o_image_quality;
} gf_delmar_image_quality_test_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    int8_t algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t preprocess_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t production_algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t fake_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t fw_version[FW_VERSION_INFO_LEN];
    int8_t tee_version[TEE_VERSION_INFO_LEN];
    int8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t chip_id[DELMAR_CHIP_ID_BUFFER_LEN];
    uint8_t vendor_id[DELMAR_VENDOR_ID_BUFFER_LEN];
    uint8_t sensor_id[DELMAR_SENSOR_ID_BUFFER_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    uint16_t pga_gain[MAX_SENSOR_NUM];
    uint16_t exposure_time[MAX_SENSOR_NUM];
} gf_test_get_version_t;

typedef struct
{
    gf_cmd_header_t header;
    int32_t i_collect_phase;
    uint8_t i_sensor_index;
    int32_t o_frame_num;
} gf_delmar_performance_test_cmd_base_t;

typedef struct
{
    gf_delmar_performance_test_cmd_base_t cmd_base;
} gf_delmar_collect_cmd_t;

typedef struct
{
    int8_t nProductAlgoVersion[MAX_SENSOR_NUM][DELMAR_PRODUCTION_ALGO_VERSION_INFO_LEN];
    uint8_t nModuleID[MAX_SENSOR_NUM][DELMAR_GF_SENSOR_ID_LEN];
    uint8_t nSensorID[MAX_SENSOR_NUM][DELMAR_GF_SENSOR_ID_LEN];
    char nLotID[MAX_SENSOR_NUM][DELMAR_WAFER_LOT_ID_LEN];
    uint8_t nWaferNo[MAX_SENSOR_NUM];
    uint8_t nDieX[MAX_SENSOR_NUM];
    uint8_t nDieY[MAX_SENSOR_NUM];
    uint8_t nSmt1_VendorID[MAX_SENSOR_NUM];
    uint8_t nSmt2_VendorID[MAX_SENSOR_NUM];
    uint32_t error_type[MAX_SENSOR_NUM];
    uint8_t nPgaGain[MAX_SENSOR_NUM];
    int16_t nExposureTime[MAX_SENSOR_NUM];
    uint32_t nFrameNumber[MAX_SENSOR_NUM];
    uint16_t nItoPatternCode[MAX_SENSOR_NUM];
    uint32_t nRubberNum[MAX_SENSOR_NUM];
    uint32_t nHotPixelNum[MAX_SENSOR_NUM];
    int32_t nHotLineNum[MAX_SENSOR_NUM];
    int32_t nBadPointNum[MAX_SENSOR_NUM];
    int32_t nClusterNum[MAX_SENSOR_NUM];
    int32_t nPixelOfLargestBadCluster[MAX_SENSOR_NUM];
    int32_t nBpnInClusters[MAX_SENSOR_NUM];
    int32_t nLightHBadLineNum[MAX_SENSOR_NUM];
    int32_t nLightVBadLineNum[MAX_SENSOR_NUM];
    int32_t nPeaks[MAX_SENSOR_NUM];
    int32_t nValleys[MAX_SENSOR_NUM];
    int32_t nMaxPeakValley[MAX_SENSOR_NUM];
    int32_t nMaxRateofChange[MAX_SENSOR_NUM];
    int32_t nMaxPeakAmp[MAX_SENSOR_NUM];
    int32_t nMinValleyAmp[MAX_SENSOR_NUM];
    int32_t nInValidArea[MAX_SENSOR_NUM];
    int32_t nLowCorrPitch_LPF[MAX_SENSOR_NUM];
    int32_t nDataNoiseFlat[MAX_SENSOR_NUM];
    int32_t nDataNoiseFlatLPF[MAX_SENSOR_NUM];
    int32_t nDataNoiseFlatLPF_MT[MAX_SENSOR_NUM];
    int32_t nChartDirection[MAX_SENSOR_NUM];
    int32_t nDirectionOffset[MAX_SENSOR_NUM];
    int32_t nUnorSignal[MAX_SENSOR_NUM];
    int32_t nSignal[MAX_SENSOR_NUM];
    int32_t nSSNR[MAX_SENSOR_NUM];
    int32_t nNoise[MAX_SENSOR_NUM];
    int32_t nShapeness[MAX_SENSOR_NUM];
    int32_t nUnorSignal_LPF[MAX_SENSOR_NUM];
    int32_t nSignal_LPF[MAX_SENSOR_NUM];
    int32_t nSSNR_LPF[MAX_SENSOR_NUM];
    int32_t nNoise_LPF[MAX_SENSOR_NUM];
    int32_t nShapeness_LPF[MAX_SENSOR_NUM];
    int32_t nDarkTNoise[MAX_SENSOR_NUM];
    int32_t nDarkSNoise[MAX_SENSOR_NUM];
    int32_t nLightTNoise[MAX_SENSOR_NUM];
    int32_t nLightSNoise[MAX_SENSOR_NUM];
    int32_t nLightHighMean[MAX_SENSOR_NUM];
    int32_t nTSNR[MAX_SENSOR_NUM];
    int32_t nDPAADarkDiff[MAX_SENSOR_NUM];
    int32_t nMinDiffFleshHM[MAX_SENSOR_NUM];
    int32_t nMinDiffFleshML[MAX_SENSOR_NUM];
    int32_t nMinDiffBlackHM[MAX_SENSOR_NUM];
    int32_t nMinDiffBlackML[MAX_SENSOR_NUM];
    int32_t nMaxDiffOffset[MAX_SENSOR_NUM];
    int32_t nBlackL1TNoise[MAX_SENSOR_NUM];
    int32_t nBlackL2TNoise[MAX_SENSOR_NUM];
    int32_t nBlackL3TNoise[MAX_SENSOR_NUM];
    int32_t nFleshL1TNoise[MAX_SENSOR_NUM];
    int32_t nFleshL2TNoise[MAX_SENSOR_NUM];
    int32_t nFleshL3TNoise[MAX_SENSOR_NUM];
    int32_t nMaxTNoise[MAX_SENSOR_NUM];
    int32_t nHAFBadPointNum[MAX_SENSOR_NUM];
    int32_t nHAFBadBlockNum[MAX_SENSOR_NUM];
    int32_t nHAFBadRatioNum[MAX_SENSOR_NUM];

    int16_t nLightLeakRatio[MAX_SENSOR_NUM];
    int32_t nMaxITODepth[MAX_SENSOR_NUM];
    int32_t nBWhitePixelNum[MAX_SENSOR_NUM];
    int32_t nBBlackPixelNum[MAX_SENSOR_NUM];
    int32_t nDpBadPointNum[MAX_SENSOR_NUM];
    int32_t nDpMaxBpnInRow[MAX_SENSOR_NUM];
    int32_t nDpMeanDiff[MAX_SENSOR_NUM];
    int32_t nDPSNoiseDark[MAX_SENSOR_NUM];
    int32_t nDPSNoiseLight[MAX_SENSOR_NUM];
    int16_t nMtDarkOp[MAX_SENSOR_NUM];
    uint16_t nUsLeftDistort[MAX_SENSOR_NUM];
    uint16_t nUsRightDistort[MAX_SENSOR_NUM];
    int32_t nMoireValue[MAX_SENSOR_NUM];

    int32_t nCollectPhase;
    int32_t nFrameNum;
    uint32_t nCenterXtoChip[MAX_SENSOR_NUM];
    uint32_t nCenterYtoChip[MAX_SENSOR_NUM];
    float nCenterXOffset[MAX_SENSOR_NUM];
    float nCenterYOffset[MAX_SENSOR_NUM];

    int32_t nAngletoChip[MAX_SENSOR_NUM];
    float nAngelOffset[MAX_SENSOR_NUM];
    int32_t nGhostNum[MAX_SENSOR_NUM];
} gf_calculate_cmd_result_t;

typedef struct
{
    // location circle param
    uint32_t nSensorX;
    uint32_t nSensorY;
    uint32_t nSensorWidth;
    uint32_t nSensorHeight;
    uint32_t nSensorOffset;
    uint32_t nScreenWidth;
    uint32_t nScreenHeight;
    uint32_t nSensor_Center_X[MAX_SENSOR_NUM];
    uint32_t nSensor_Center_Y[MAX_SENSOR_NUM];
    float nSensorITORotation;
    uint32_t nIs_Whole_Pic;
} gf_location_circle_params_t;

typedef struct
{
    gf_delmar_performance_test_cmd_base_t cmd_base;
    gf_location_circle_params_t i_loc_params;
    gf_calculate_cmd_result_t o_result;
    uint8_t o_pga_gain[MAX_SENSOR_NUM];
    uint8_t o_pga_gain2[MAX_SENSOR_NUM];
    uint32_t o_mid_digit_gain[MAX_SENSOR_NUM];
    uint32_t o_low_digit_gain[MAX_SENSOR_NUM];
    uint32_t o_dark_h_avg[MAX_SENSOR_NUM];
    uint32_t feature_type;
    uint32_t disable_save_flag;
    uint32_t err_code;
} gf_delmar_calculate_cmd_t;

typedef struct
{
    gf_cmd_header_t header;
    uint32_t o_mcu_chip_id;
    uint16_t o_sensor_chip_id[MAX_SENSOR_NUM];
    uint16_t o_flash_id;
    uint16_t o_random_number[MAX_SENSOR_NUM];
} gf_delmar_test_spi_cmd_t;

typedef enum
{
    MCU_RESET_STEP,
    MCU_WAIT_READY_STEP,
} gf_rst_int_step;

typedef struct
{
    gf_cmd_header_t header;
    gf_rst_int_step i_step;
} gf_delmar_rst_int_pin_cmd_t;

typedef struct
{
    gf_cmd_header_t header;
    // morphotype detected by pin
    uint8_t o_morphotype;
} gf_delmar_morphotype_cmd_t;

typedef gf_cmd_header_t gf_delmar_test_otp_flash_cmd_t;

typedef struct {
    // Bad-Pixel/Cluster
    int32_t maxBadPointNum;
    int32_t maxClusterNum;
    int32_t maxPixelOfLargestBadCluster;
    int32_t maxBpnInClusters;
    int32_t maxLightHBadLineNum;
    int32_t maxLightVBadLineNum;
    int32_t maxBBlackPixelNum;
    int32_t maxBWhitePixelNumLow;
    uint32_t maxHotConnected;
    // DarkPixel
    int32_t maxDPBadPointNum;
    int32_t maxDPBpnInRow;
    int32_t maxDPDiffMean;
    int32_t maxDPSNoiseDark;
    int32_t maxDPAADarkDiff;
    // HAF
    int32_t maxHAFBadPointNum;
    int32_t maxHAFBadBlockNum;
    // Tnoise/Snoise
    int32_t maxTNoiseDark;
    int32_t maxTNoiseLight;
    int32_t maxSNoiseDark;
    int32_t maxSNoiseLight;
    // LightHighMean
    int32_t minLightHighMean;
    int32_t maxLightHighMean;
    // LightLeakRatio
    float maxLightLeakRatio;
    // Signal/FlatSNoise/TSNR/Sharpness
    int32_t minSignal;  // no normalization
    int32_t maxFlatSNoise;  // after LPF dataNoiseFlatLPF
    float minTSNR;
    double minSharpness;  // after LPFsharpnessLPF
    // AssemblyAngle/CenterXOffset/CenterYOffset
    float maxAssemblyAngle;
    float maxCenterXOffset;
    float maxCenterYOffset;
    // DiffFleshHM/DiffFleshML/DiffBlackHM/DiffBlackML/DiffOffset/LightStability/LowCorrPitch/InValidArea/ChartDirection
    int32_t minDiffFleshHM;  // gap of Brightness L3 and Brightness L2 for fresh rubber
    int32_t minDiffFleshML;  // gap of Brightness L2 and Brightness L1 for fresh rubber
    int32_t minDiffBlackHM;  // gap of Brightness L3 and Brightness L2 for black rubber
    int32_t minDiffBlackML;  // gap of Brightness L2 and Brightness L1 for black rubber
    int32_t maxDiffOffset;
    float maxLightStability;  // maxTNoise
    int32_t maxLowCorrPitch;
    int32_t maxInValidArea;
    int32_t maxChartDirection;
    // standard param
    float standardAngle;
    float standardCenterX;
    float standardCenterY;
    int32_t maxGhostNum;
} gf_delmar_product_test_config_t;

typedef struct {
    uint8_t o_sensor_id[DELMAR_SENSOR_ID_BUFFER_LEN];
    uint8_t o_vendor_id[DELMAR_VENDOR_ID_BUFFER_LEN];
} gf_delmar_sensor_info_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_delmar_sensor_info_t chip_info[MAX_SENSOR_NUM];
} gf_delmar_get_sensor_info_cmd_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_delmar_sensor_otp_info_t otp_info[MAX_SENSOR_NUM];
} gf_delmar_test_get_otp_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    int8_t flag;  // 0 get, 1 set
    int32_t io_value;
} gf_delmar_gain_target_cmd_t;


#endif /* _GF_DELMAR_PRODUCT_TEST_TYPES_H_ */
