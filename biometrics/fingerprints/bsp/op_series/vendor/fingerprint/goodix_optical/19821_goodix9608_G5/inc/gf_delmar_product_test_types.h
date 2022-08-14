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

#define TEST_MAX_COMPUTE_FRAME_NUM (7)  // compute the position of auto find sensor and performance testing
// for delmar location circle pic length(200 * 200)
#define LOCATION_CIRCLE_PIC_BUFFER_LEN    (200 * 200)
#define KB_BUFFER_LEN    (182*132)
#define CALI_KB 0x00
#define CALI_SUB_KB 0x01
#define CALI_ADJ_SUB_KB 0x02

enum {
    GF_CMD_TEST_PERFORMANCE_TESTING = GF_PRODUCT_TEST_CMD_MAX + 1,
    GF_CMD_TEST_LOCATION_CIRCLE_TESTING,
    GF_CMD_TEST_SPI,
    GF_CMD_TEST_RESET_INTERRUPT_PIN,
    GF_CMD_TEST_OTP_FLASH,
    GF_CMD_TEST_GET_SENSOR_INFO,
    GF_CMD_TEST_GET_OTP_INFO,
    GF_CMD_TEST_GAIN_TARGET,
    GF_CMD_TEST_GET_IMAGE_QUALITY,
    GF_CMD_TEST_MAX,
};

typedef enum {
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
    OPERATION_STEP_CALCULATE_GAIN_COLLECT,  // 114
    OPERATION_STEP_CALCULATE_GAIN,
    OPERATION_STEP_CALCULATE_MID_BRIGHTNESS_GAIN_COLLECT,
    OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_GAIN_COLLECT,
    OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_GAIN,
    OPERATION_STEP_FINISHED = 120,
    OPERATION_STEP_CALCULATE_GAIN_ONE_COLLECT,
    OPERATION_STEP_CALCULATE_GAIN_TWO,
    OPERATION_STEP_CALCULATE_GAIN_TWO_COLLECT,
    OPERATION_STEP_CALCULATE_GAIN_TWO_LIGHT_MEAN,
    OPERATION_STEP_MID_BRIGHTNESS_FRESH_COLLECT = 603,
    OPERATION_STEP_LOW_BRIGHTNESS_FRESH_COLLECT,
    OPERATION_STEP_CALCULATE_LOW_BRIGHTNESS_DIGIT_GAIN,
    OPERATION_STEP_MAXFLESH_COLLECT,
    OPERATION_STEP_COLLECT_MAX = 999
} gf_test_performance_testing_operation_step_t;

typedef enum {
    OPERATION_STEP_NOISE_BASEDATA_DARK_COLLECT = 1,
    OPERATION_STEP_NOISE_BASEDATA_MAX_COLLECT,
    OPERATION_STEP_NOISE_CALCULATE_RESULT
} gf_test_noise_operation_step_t;



typedef struct {
    gf_cmd_header_t header;
    int32_t i_collect_phase;
    uint8_t i_sensor_index;
    int32_t o_frame_num;
} gf_delmar_performance_test_cmd_base_t;

typedef struct {
    gf_delmar_performance_test_cmd_base_t cmd_base;
    uint8_t o_sensor_id[DELMAR_SENSOR_ID_BUFFER_LEN];
    uint16_t o_raw_data[DELMAR_RAW_BUFFER_LEN * TEST_MAX_COMPUTE_FRAME_NUM * MAX_SENSOR_NUM];
    uint16_t o_raw_data_flesh[DELMAR_RAW_BUFFER_LEN * TEST_MAX_COMPUTE_FRAME_NUM * MAX_SENSOR_NUM];
#ifdef SUPPORT_DUMP_ORIGIN_DATA
    uint8_t origin_data[DELMAR_ORIGIN_BUFFER_LEN * TEST_MAX_COMPUTE_FRAME_NUM * 2];
    uint32_t frame_num;
    uint32_t origin_col;
    uint32_t origin_row;
#endif  // SUPPORT_DUMP_ORIGIN_DATA
    uint16_t crc_origin_data[256 * 520];
    uint32_t crc_origin_len;
} gf_delmar_collect_cmd_t;

typedef struct {
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
    int32_t nValidArea[MAX_SENSOR_NUM];
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
    int32_t nTargetLightHighMean[MAX_SENSOR_NUM];
    int32_t nLightHighMean[MAX_SENSOR_NUM];
    int32_t nTSNR[MAX_SENSOR_NUM];
    int32_t nAADarkDiff[MAX_SENSOR_NUM];
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
    int16_t k[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    int16_t b[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t black_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t flesh_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t chart_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t kr_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t br_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t k_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t b_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t ito_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t push_mask_bmp_data[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint8_t puchMask[MAX_SENSOR_NUM][KB_BUFFER_LEN];
    uint32_t del_dark_pixel_width;
    uint32_t del_dark_pixel_height;
    uint32_t nCenterXtoChip[MAX_SENSOR_NUM];
    uint32_t nCenterYtoChip[MAX_SENSOR_NUM];
    float nCenterXOffset[MAX_SENSOR_NUM];
    float nCenterYOffset[MAX_SENSOR_NUM];

    int32_t nAngletoChip[MAX_SENSOR_NUM];
    float nAngelOffset[MAX_SENSOR_NUM];
    // 155k data
    uint8_t cali_moire_data[MAX_SENSOR_NUM][DELMAR_CALIBRATION_DATA_LEN];
    int32_t cali_moire_data_len[MAX_SENSOR_NUM];
    int32_t nCaliMode;
    uint8_t highlight_flesh_cf_base[MAX_SENSOR_NUM][DELMAR_RAW_CF_BUFFER_LEN];
    uint8_t highlight_dark_cf_base[MAX_SENSOR_NUM][DELMAR_RAW_CF_BUFFER_LEN];
    uint32_t highlight_base_len;
#ifdef SUPPORT_DUMP_ORIGIN_DATA
    uint8_t origin_data_dark_max[MAX_SENSOR_NUM][DELMAR_ORIGIN_BUFFER_LEN];
    uint8_t origin_data_max[MAX_SENSOR_NUM][DELMAR_ORIGIN_BUFFER_LEN];
    uint32_t origin_col;
    uint32_t origin_row;
#endif  // SUPPORT_DUMP_ORIGIN_DATA
    uint16_t highlight_flesh_no_cf_base[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint16_t highlight_dark_no_cf_base[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint32_t base_col;
    uint32_t base_row;
    int32_t nGhostNum[MAX_SENSOR_NUM];
} gf_calculate_cmd_result_t;

typedef struct {
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

typedef struct {
    gf_delmar_performance_test_cmd_base_t cmd_base;
    gf_location_circle_params_t i_loc_params;
    gf_calculate_cmd_result_t o_result;
    uint8_t o_pga_gain[MAX_SENSOR_NUM];
    uint8_t o_pga_gain2[MAX_SENSOR_NUM];
    uint32_t feature_type;
    uint32_t err_code;
} gf_delmar_calculate_cmd_t;



typedef struct {
    gf_cmd_header_t header;
    uint32_t i_temperature;
    uint32_t i_sensor_index;
    uint32_t o_image_quality;
    uint32_t o_valid_area;
    uint8_t o_is_fake_finger;
    uint8_t mask_bw[MAX_SENSOR_NUM][2][DELMAR_BW_MASK_LEN];
    uint16_t raw_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint32_t raw_data_len[MAX_SENSOR_NUM];
    uint8_t raw_data_cf[MAX_SENSOR_NUM][DELMAR_RAW_CF_BUFFER_LEN];
    uint32_t raw_data_cf_len;
    uint8_t data_bmp[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint8_t data_decrypt_bmp[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    int16_t diff_image[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint8_t color_point_data[MAX_SENSOR_NUM][DELMAR_CF_POINT_BUFFER_LEN];
    uint32_t color_point_data_len;
    uint8_t fake_mask_data[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint32_t fake_mask_data_len[MAX_SENSOR_NUM];
    uint32_t preprocess_col[MAX_SENSOR_NUM];
    uint32_t preprocess_row[MAX_SENSOR_NUM];
    uint32_t mask_bw_len;
    int32_t nFakeParamV1Arr[2][20];
    int32_t nFakeParamV2Arr[2][20];
    int32_t fFakeParamV1Arr[2][20];
    int32_t  fFakeParamV2Arr[2][20];
    uint32_t o_part;
    gf_delmar_coordinate_info_t i_coordinate_info;
}gf_delmar_image_quality_test_t;

typedef struct {
    uint32_t nCenterXtoChip[MAX_SENSOR_NUM];
    uint32_t nCenterYtoChip[MAX_SENSOR_NUM];
    uint32_t nAngletoChip[MAX_SENSOR_NUM];
    uint32_t nCenterXtoScreen[MAX_SENSOR_NUM];
    uint32_t nCenterYtoScreen[MAX_SENSOR_NUM];
    uint32_t nAngletoScreen[MAX_SENSOR_NUM];
    uint32_t nWidth;
    uint32_t nHeight;
    uint32_t nOperationStep;
    uint32_t nFrameNum;
    uint16_t raw_data[MAX_SENSOR_NUM * DELMAR_RAW_BUFFER_LEN * TEST_MAX_COMPUTE_FRAME_NUM];
    uint8_t circle_bmp_data[MAX_SENSOR_NUM * DELMAR_RAW_BUFFER_LEN];
} gf_location_circle_test_result_t;

typedef struct {
    gf_cmd_header_t header;
    gf_location_circle_test_result_t o_result;
} gf_delmar_location_circle_test_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t o_mcu_chip_id;
    uint16_t o_sensor_chip_id[MAX_SENSOR_NUM];
    uint16_t o_flash_id;
    uint16_t o_random_number[MAX_SENSOR_NUM];
} gf_delmar_test_spi_cmd_t;

typedef enum {
    MCU_RESET_STEP,
    MCU_WAIT_READY_STEP,
} gf_rst_int_step;

typedef struct {
    gf_cmd_header_t header;
    gf_rst_int_step i_step;
} gf_delmar_rst_int_pin_cmd_t;

typedef gf_cmd_header_t gf_delmar_test_otp_flash_cmd_t;

typedef struct {
    int32_t badPointNum;
    int32_t clusterNum;
    int32_t pixelOfLargestBadCluster;
    int32_t bpnInClusters;
    int32_t lightHBadLineNum;
    int32_t lightVBadLineNum;
    uint32_t maxHotConnectedNum;
    int32_t lowCorrPitchLPF;
    int32_t maxValidArea;
    int32_t minChartDirection;
    int32_t maxChartDirection;
    int32_t aaDarkDiff;
    float minAngle;
    float maxAngle;
    float chipCenterOffsetX;
    float chipCenterOffsetY;
    int32_t minLightHighMean;
    int32_t maxLightHighMean;
    int32_t minDiffFleshHM;  // gap of Brightness L3 and Brightness L2 for fresh rubber
    int32_t minDiffFleshML;  // gap of Brightness L2 and Brightness L1 for fresh rubber
    int32_t minDiffBlackHM;  // gap of Brightness L3 and Brightness L2 for black rubber
    int32_t minDiffBlackML;  // gap of Brightness L2 and Brightness L1 for black rubber
    int32_t maxDiffOffset;
    int32_t darkTNoise;  // before LPF
    int32_t lightTNoise;  // before LPF
    int32_t darkSNoise;  // before LPF
    int32_t lightSNoise;  // before LPF
    int32_t dataNoiseFlatLPF;  // after LPF
    float unorSignalLPF;  // after LPF
    int32_t signalLPF;  // after LPF
    int32_t ssnrLPF;  // after LPF
    double sharpnessLPF;  // after LPF
    float tSNR;
    float maxTNoise;
    float standardAngle;
    float standardCenterX;
    float standardCenterY;
    int32_t blackPixelNum;
    int32_t whitePixelNum;
    float lightLeakRatio;
    float maxITODepth;
    int32_t dpBadPointNum;
    int32_t dpMaxBpnInRow;
    int32_t dpMeanDiff;
    int32_t dPSNoiseDark;
    int32_t dPSNoiseLight;
    int32_t maxHAFBadPointNum;
    int32_t maxHAFBadBlockNum;
    int32_t maxHAFBadRatioNum;
    int32_t realChartDirection;  // 0 is parallel to the short side; 90 is parallel to the long side
    int32_t maxHotLineNum;
    int32_t maxDarkLightLeak;
    int32_t maxGhostNum;
} gf_delmar_product_test_config_t;

typedef struct {
    uint8_t o_sensor_id[DELMAR_SENSOR_ID_BUFFER_LEN];
    uint8_t o_vendor_id[DELMAR_VENDOR_ID_BUFFER_LEN];
    uint8_t o_optical_type;
    uint8_t o_otp_version;
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
