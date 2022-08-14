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
#include "gf_base_types.h"

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
    GF_CMD_SET_SAMPLING_CFG,
    GF_CMD_GET_SAMPLING_CFG,
    GF_CMD_TEST_MAX,
};

typedef enum
{
    OPERATION_STEP_COLLECT_NONE = 83,
        OPERATION_STEP_DEC_EXPO_BASEDATA_MAX_COLLECT,
        OPERATION_STEP_DEC_EXPO_BASEDATA_MAX_DARK_COLLECT,

    OPERATION_STEP_AUTO_EXPO_COLLECT,
    OPERATION_STEP_CALCULATE_AUTO_EXPO,
    OPERATION_STEP_ADAPTED_EXPO_FOR_PWM_COLLECT,
    OPERATION_STEP_CALCULATE_ADAPTE_EXPO_FOR_PWM,
    OPERATION_STEP_LOW_BR_BASEDATA_DARK_COLLECT = 90,
    OPERATION_STEP_LOW_BR_BASEDATA_MIN_DARK_COLLECT = 91,
    OPERATION_STEP_LOW_BR_BASEDATA_MED_DARK_COLLECT = 92,
    OPERATION_STEP_LOW_BR_BASEDATA_MAX_DARK_COLLECT = 93,
    OPERATION_STEP_LOW_BR_BASEDATA_MIN_COLLECT = 94,
    OPERATION_STEP_LOW_BR_BASEDATA_MED_COLLECT = 95,
    OPERATION_STEP_LOW_BR_BASEDATA_MAX_COLLECT = 96,
    OPERATION_STEP_LOW_BR_CIRCLEDATA_COLLECT = 97,
    OPERATION_STEP_LOW_BR_CHARTDATA_COLLECT = 98,
    OPERATION_STEP_LOW_BR_CIRCLE_CHARTDATA_COLLECT = 99,
    OPERATION_STEP_CIRCLE_CHARTDATA_COLLECT,
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
    uint8_t reserve[MAX_RESERVE_SIZE];
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
    uint8_t sensor_id[DELMAR_SENSOR_ID_MAX_BUFFER_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    uint16_t pga_gain[MAX_SENSOR_NUM];
    gf_delmar_exposure_time_t exposure_time[MAX_SENSOR_NUM];
    int8_t hb_algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    int8_t hb_version[HB_VERSION_INFO_LEN];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_test_get_version_t;

typedef struct
{
    gf_cmd_header_t header;
    int32_t i_collect_phase;
    uint8_t i_sensor_index;
    int32_t o_frame_num;
    int32_t enable_scan_flag;
    int32_t scan_step_index;
    int32_t total_scan_count;
    float base_expo_time;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_performance_test_cmd_base_t;

typedef struct
{
    gf_delmar_performance_test_cmd_base_t cmd_base;
    uint32_t feature_type;
    uint8_t reserve[MAX_RESERVE_SIZE - 4];
} gf_delmar_collect_cmd_t;

typedef struct
{
    int8_t nProductAlgoVersion[MAX_SENSOR_NUM][DELMAR_PRODUCTION_ALGO_VERSION_INFO_LEN];
    uint8_t nModuleID[MAX_SENSOR_NUM][DELMAR_SENSOR_ID_MAX_BUFFER_LEN];
    uint8_t nSensorID[MAX_SENSOR_NUM][DELMAR_SENSOR_ID_MAX_BUFFER_LEN];
    char nLotID[MAX_SENSOR_NUM][DELMAR_WAFER_LOT_ID_LEN];
    uint8_t nWaferNo[MAX_SENSOR_NUM];
    uint8_t nDieX[MAX_SENSOR_NUM];
    uint8_t nDieY[MAX_SENSOR_NUM];
    uint8_t nSmt1_VendorID[MAX_SENSOR_NUM];
    uint8_t nSmt2_VendorID[MAX_SENSOR_NUM];
    uint32_t error_type[MAX_SENSOR_NUM];
    uint8_t nPgaGain[MAX_SENSOR_NUM];
    uint8_t nSecondPgaGain[MAX_SENSOR_NUM];
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
    int32_t nPressAreaRatio[MAX_SENSOR_NUM];
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
    int32_t nUnorLightTNoise[MAX_SENSOR_NUM];
    int32_t nLightTNoise[MAX_SENSOR_NUM];
    int32_t nLightSNoise[MAX_SENSOR_NUM];
    int32_t nMeanValueLight[MAX_SENSOR_NUM];
    int32_t nMeanValueDark[MAX_SENSOR_NUM];
    int32_t nTargetLightHighMean[MAX_SENSOR_NUM];
    int32_t nLightHighMean[MAX_SENSOR_NUM];
    int32_t nTSNR[MAX_SENSOR_NUM];
    int32_t nTSNR_LPF[MAX_SENSOR_NUM];
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
    float nMtDarkOp[MAX_SENSOR_NUM];
    uint16_t nUsLeftDistort[MAX_SENSOR_NUM];
    uint16_t nUsRightDistort[MAX_SENSOR_NUM];
    int32_t nMoireValue[MAX_SENSOR_NUM];
    int32_t nGhostNum[MAX_SENSOR_NUM];
    int32_t nItoK[MAX_SENSOR_NUM];
    int32_t nItoChart[MAX_SENSOR_NUM];

    int32_t nRedAvg[MAX_SENSOR_NUM];
    int32_t nGreenAvg[MAX_SENSOR_NUM];
    int32_t nBlueAvg[MAX_SENSOR_NUM];
    int32_t nWhiteAvg[MAX_SENSOR_NUM];
    int32_t nRedRatio[MAX_SENSOR_NUM];
    int32_t nGreenRatio[MAX_SENSOR_NUM];
    int32_t nBlueRatio[MAX_SENSOR_NUM];

    int32_t nCollectPhase;
    int32_t nFrameNum;
    uint32_t nCenterXtoChip[MAX_SENSOR_NUM];
    uint32_t nCenterYtoChip[MAX_SENSOR_NUM];
    float nCenterXOffset[MAX_SENSOR_NUM];
    float nCenterYOffset[MAX_SENSOR_NUM];

    int32_t nAngletoChip[MAX_SENSOR_NUM];
    float nAngelOffset[MAX_SENSOR_NUM];
    int16_t nTemperatureAdcBase[MAX_SENSOR_NUM];

    int32_t nScanSPISpeed[MAX_SENSOR_NUM];
    int32_t nScanExpoTimeStep[MAX_SENSOR_NUM];
    int32_t nScanExpoTime[MAX_SENSOR_NUM][SCAN_EXPO_TIME_MAX_FRAME_NUM];
    int32_t nScanTnoise[MAX_SENSOR_NUM][SCAN_EXPO_TIME_MAX_FRAME_NUM];
    int32_t nScanSnoise[MAX_SENSOR_NUM][SCAN_EXPO_TIME_MAX_FRAME_NUM];
    uint64_t align_reserve;  // used for make struct align by 8bytes
    uint8_t reserve[MAX_SENSOR_NUM * MAX_RESERVE_SIZE];
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
    gf_cmd_header_t header;
    uint32_t o_mcu_chip_id;
    uint16_t o_sensor_chip_id[MAX_SENSOR_NUM];
    uint16_t o_flash_id;
    uint16_t o_random_number[MAX_SENSOR_NUM];
    uint8_t reserve[MAX_RESERVE_SIZE];
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
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_rst_int_pin_cmd_t;

typedef struct
{
    gf_cmd_header_t header;
    // morphotype detected by pin
    uint8_t o_morphotype;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_morphotype_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t otp_info[DELMAR_OTP_INFO_BUFFER_LEN];
    uint32_t otp_info_len;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_test_otp_cmd_t;

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
    int32_t maxHotLineNum;
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
    int32_t maxDarkLightLeak;
    int32_t maxLowCorrPitch;
    int32_t maxInValidArea;
    int32_t maxChartDirection;
    int16_t minTemperatureAdcBase;
    int16_t maxTemperatureAdcBase;
    int32_t maxDiamondK;
    // standard param
    float standardAngle;
    float standardCenterX;
    float standardCenterY;
    uint64_t align_reserve;  // used for make struct align by 8bytes
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_product_test_config_t;

typedef struct {
    uint8_t o_sensor_id[DELMAR_SENSOR_ID_MAX_BUFFER_LEN];
    uint8_t o_vendor_id[DELMAR_VENDOR_ID_BUFFER_LEN];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_sensor_info_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_delmar_sensor_info_t chip_info[MAX_SENSOR_NUM];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_get_sensor_info_cmd_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    gf_delmar_sensor_otp_info_t otp_info[MAX_SENSOR_NUM];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_test_get_otp_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    int8_t flag;  // 0 get, 1 set
    int32_t io_value;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_gain_target_cmd_t;

typedef struct
{
    gf_cmd_header_t header;
    gf_delmar_sampling_cfg_t io_cfg;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_sampling_cfg_cmd_t;

typedef struct
{
    uint32_t nErrorType[MAX_SENSOR_NUM];
    uint8_t nSensorID[MAX_SENSOR_NUM][DELMAR_SENSOR_ID_MAX_BUFFER_LEN];
    // Bad-Pixel/Cluster
    int32_t nBadPixelNum[MAX_SENSOR_NUM];
    int32_t nBadBlockNum[MAX_SENSOR_NUM];
    int32_t nBadBlockLargest[MAX_SENSOR_NUM];
    int32_t nBadPixelAllBlocks[MAX_SENSOR_NUM];
    int32_t nBadLineNum[MAX_SENSOR_NUM];
    int32_t nHotPixelNum[MAX_SENSOR_NUM];
    int32_t nHotBlockLargest[MAX_SENSOR_NUM];
    int32_t nHotLineNum[MAX_SENSOR_NUM];
    // DP param
    int32_t nDPBadPixelNum[MAX_SENSOR_NUM];
    int32_t nDPBadPixelInRow[MAX_SENSOR_NUM];
    int32_t nDPSNoiseDarkN[MAX_SENSOR_NUM];
    int32_t nDPAADiffDark[MAX_SENSOR_NUM];
    // HAF
    int32_t nHAFBadPixelNum[MAX_SENSOR_NUM];
    int32_t nHAFBadBlockNum[MAX_SENSOR_NUM];
    // AF
    int32_t nAntiFakeDx[MAX_SENSOR_NUM];
    int32_t nAntiFakeDy[MAX_SENSOR_NUM];
    // Tnoise/Snoise
    int32_t nTNoiseDarkN[MAX_SENSOR_NUM];
    int32_t nTNoiseLightN[MAX_SENSOR_NUM];
    int32_t nSNoiseDarkN[MAX_SENSOR_NUM];
    int32_t nSNoiseLightN[MAX_SENSOR_NUM];
    // LightHighMean
    int32_t nLightHighMean[MAX_SENSOR_NUM];
    // screen param
    int16_t nLightLeakRatio[MAX_SENSOR_NUM];
    // performance param
    int32_t nSignalL[MAX_SENSOR_NUM];
    int32_t nSignalLLow[MAX_SENSOR_NUM];
    int32_t nSignalLHigh[MAX_SENSOR_NUM];
    int32_t nSNoiseFlatL[MAX_SENSOR_NUM];
    int32_t nTSNR[MAX_SENSOR_NUM];
    int32_t nTSNRLow[MAX_SENSOR_NUM];
    int32_t nTSNRHigh[MAX_SENSOR_NUM];
    // location circle param
    float nAssemblyXOffset[MAX_SENSOR_NUM];
    float nAssemblyYOffset[MAX_SENSOR_NUM];
    float nAssemblyAngelOffset[MAX_SENSOR_NUM];
    // fool proof parm
    int32_t nDiffFleshHM[MAX_SENSOR_NUM];
    int32_t nDiffFleshML[MAX_SENSOR_NUM];
    int32_t nDiffBlackHM[MAX_SENSOR_NUM];
    int32_t nDiffBlackML[MAX_SENSOR_NUM];
    int32_t nDiffBlackLD[MAX_SENSOR_NUM];
    int32_t nDiffOffset[MAX_SENSOR_NUM];
    int32_t nLightStability[MAX_SENSOR_NUM];
    int32_t nLightLeakDark[MAX_SENSOR_NUM];
    int32_t nValidAreaRatio[MAX_SENSOR_NUM];
    int32_t nLowCorrPitch[MAX_SENSOR_NUM];
    int32_t nChartDirection[MAX_SENSOR_NUM];
    int32_t nChartGhostShadow[MAX_SENSOR_NUM];
    int32_t nChartDirty[MAX_SENSOR_NUM];
    int32_t nPolarDegree[MAX_SENSOR_NUM];

    int32_t nRedRatio[MAX_SENSOR_NUM];
    float nHRDNoise[MAX_SENSOR_NUM];
    int32_t nFlesh3DFool[MAX_SENSOR_NUM];
    float nHblackNarrowRange[MAX_SENSOR_NUM];
    // float nLocationPValue[MAX_SENSOR_NUM];
    int32_t nTestHeadStatusDepth[MAX_SENSOR_NUM];

    float nHfleshDiffRatio[MAX_SENSOR_NUM];
    float nHblackDiffRatio[MAX_SENSOR_NUM];
    uint64_t align_reserve;  // used for make struct align by 8bytes
    uint8_t reserve[MAX_SENSOR_NUM * MAX_RESERVE_SIZE - 12 * MAX_SENSOR_NUM];
} gf_new_product_report_result_t;

typedef struct {
    // Bad-Pixel/Cluster
    int32_t maxBadPixelNum;
    int32_t maxBadBlockNum;
    int32_t maxBadBlockLargest;
    int32_t maxBadPixelAllBlocks;
    int32_t maxBadLineNum;
    int32_t maxHotPixelLowNum;
    int32_t maxHotBlockLargest;
    int32_t maxHotLineNum;
    // DarkPixel
    int32_t maxDPBadPixelNum;
    int32_t maxDPBadPixelInRow;
    int32_t maxDPDiffMean;
    float maxDPSNoiseDarkN;
    int32_t maxDPAADiffDark;
    // HAF
    int32_t maxHAFBadPixelNum;
    int32_t maxHAFBadBlockNum;
    // AF
    int32_t maxAntiFakeDx;
    int32_t minAntiFakeDx;
    int32_t maxAntiFakeDy;
    int32_t minAntiFakeDy;
    // Tnoise/Snoise
    float maxTNoiseDarkN;
    float maxTNoiseLightN;
    float maxSNoiseDarkN;
    int32_t maxSNoiseLightN;
    // LightHighMean
    int32_t minLightHighMean;
    int32_t maxLightHighMean;
    // LightLeakRatio
    float maxLightLeakRatio;
    // PolarDegree
    float minPolarDegree;
    // Signal/FlatSNoise/TSNR/Sharpness
    float minSignalL;  // no normalization
    // Signal for op7.0
    float minSignalLLow;
    float minSignalLHigh;
    int32_t maxSNoiseFlatL;  // after LPF dataNoiseFlatLPF
    float minTSNR;
    // TSNR for op7.0
    float minTSNRLow;
    float minTSNRHigh;
    double minSharpnessL;  // after LPFsharpnessLPF
    // AssemblyAngle/CenterXOffset/CenterYOffset
    float maxAssemblyAngleOffset;
    float maxAssemblyXOffset;
    float maxAssemblyYOffset;
    // DiffFleshHM/DiffFleshML/DiffBlackHM/DiffBlackML/DiffOffset/LightStability/LowCorrPitch/InValidArea/ChartDirection
    int32_t minDiffFleshHM;  // gap of Brightness L3 and Brightness L2 for fresh rubber
    int32_t minDiffFleshML;  // gap of Brightness L2 and Brightness L1 for fresh rubber
    int32_t minDiffBlackHM;  // gap of Brightness L3 and Brightness L2 for black rubber
    int32_t minDiffBlackML;  // gap of Brightness L2 and Brightness L1 for black rubber
    int32_t minDiffBlackLD;  // gap of Brightness L1 and Darkbase for black rubber
    int32_t maxDiffOffset;
    float maxLightStability;
    int32_t maxLightLeakDark;
    int32_t maxLowCorrPitch;
    float minValidAreaRatio;
    int32_t maxChartDirection;
    int32_t maxChartGhostShadow;
    int32_t maxChartDirty;
    float maxPValue;
    // standard param
    float standardAngle;
    float standardCenterX;
    float standardCenterY;
    float maxHRDNoise;
    int32_t maxFlesh3DFool;
    float maxHblackNarrowRange;
    float maxTestHeadStatusDepth;
    float maxDiffRatioForDecExpo;
    uint64_t align_reserve;  // used for make struct align by 8bytes
    uint8_t reserve[MAX_RESERVE_SIZE - 8];
} gf_delmar_new_product_test_threshold_t;

typedef struct
{
    gf_delmar_performance_test_cmd_base_t cmd_base;
    gf_location_circle_params_t i_loc_params;
    gf_calculate_cmd_result_t o_result;
    gf_new_product_report_result_t o_new_product_report_result;
    gf_delmar_new_product_test_threshold_t o_new_product_threshold;
    uint8_t o_pga_gain[MAX_SENSOR_NUM];
    uint8_t o_pga_gain2[MAX_SENSOR_NUM];
    uint8_t o_second_pga_gain[MAX_SENSOR_NUM];
    uint8_t o_second_pga_gain2[MAX_SENSOR_NUM];
    uint32_t o_mid_digit_gain[MAX_SENSOR_NUM];
    uint32_t o_low_digit_gain[MAX_SENSOR_NUM];
    uint32_t o_dark_h_avg[MAX_SENSOR_NUM];
    uint8_t i_time_str[DATE_TIME_STR_LEN];
    uint32_t feature_type;
    uint32_t disable_save_flag;
    uint32_t check_before_kb;
    uint32_t err_code;
    int32_t err_algo_interface[MAX_SENSOR_NUM][MAX_SG_NUM];
    uint8_t io_extend[MAX_RESERVE_SIZE];
    uint8_t o_lb_pga_gain[MAX_SENSOR_NUM];
    uint8_t o_lb_pga_gain2[MAX_SENSOR_NUM];
    uint8_t o_lb_second_pga_gain[MAX_SENSOR_NUM];
    uint8_t o_lb_second_pga_gain2[MAX_SENSOR_NUM];
    uint8_t reserve[MAX_SENSOR_NUM * MAX_RESERVE_SIZE - 4 * MAX_SENSOR_NUM];
} gf_delmar_calculate_cmd_t;

#endif /* _GF_DELMAR_PRODUCT_TEST_TYPES_H_ */
