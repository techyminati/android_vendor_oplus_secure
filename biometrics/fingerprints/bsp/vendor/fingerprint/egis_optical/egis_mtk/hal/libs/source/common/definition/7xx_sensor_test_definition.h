#pragma once
#include <stdint.h>
#include "type_definition.h"
#include "vendor_custom.h"

// copy form CS1 branch, \sensor\jni\public\include\hw_definition.h
/*
 * ET711 area size
 */
#define ET711_SENSOR_WIDTH (146)
#define ET711_SENSOR_HEIGHT (218)
#define ET711_SENSOR_WIDTH_HW (146)
#define ET711_SENSOR_HEIGHT_HW (218)

/*
 * ET713 area size
 */
#define ET713_SENSOR_WIDTH (200)
#define ET713_SENSOR_HEIGHT (200)
#define ET713_SENSOR_WIDTH_HW (202)
#define ET713_SENSOR_HEIGHT_HW (202)
//--------------------------------------------------------------
#ifdef FP_EGIS_OPTICAL_FA118
#define DBG_EXPO_END_WK 25.0f
#define FOVX1_MAX 5300.0f
#define FOVX1_MIN 3300.0f
#define FOVX2_MAX 5300.0f
#define FOVX2_MIN 3300.0f
#define FOVY1_MAX 5300.0f
#define FOVY1_MIN 3300.0f
#define FOVY2_MAX 5300.0f
#define FOVY2_MIN 3300.0f
#define PERIOD_MAX 9.3f
#define PERIOD_MIN 7.3f
#define INTENSITY_DIFF_MIN 1000
// BP
#define DBG_THRESHOLD_BP 3.0f
#else
#define DBG_EXPO_END_WK 23.0f
#define FOVX1_MAX 4300.0f
#define FOVX1_MIN 2600.0f
#define FOVX2_MAX 4300.0f
#define FOVX2_MIN 2600.0f
#define FOVY1_MAX 4300.0f
#define FOVY1_MIN 2600.0f
#define FOVY2_MAX 4300.0f
#define FOVY2_MIN 2600.0f
#define PERIOD_MAX 6.4f
#define PERIOD_MIN 5.4f
#define INTENSITY_DIFF_MIN 500
// BP
#endif //FP_EGIS_OPTICAL_FA118

/* Version for sensor test */
#define SENSOR_TEST_BUILD 1
/* Log max size. It stores test result */
#define SENSOR_TEST_OUTPUT_LOG_SIZE 2048
#define IMG711_SIZE_BUFF (ET711_SENSOR_WIDTH * ET711_SENSOR_HEIGHT)
#define IMG713_SIZE_BUFF (ET713_SENSOR_WIDTH * ET713_SENSOR_HEIGHT)

#define IMG_MAX_BUFFER_SIZE IMG713_SIZE_BUFF
#define EFS_CALIB_DATA_SIZE 300000
#define DEMO_UI_LOG_BUF 512
#define EFUSE_BUF_SIZE 32

#define DEFINE_FILE_PATH "/data/vendor/optical_fingerprint/et713/"
#define FILE_NAME_LEN 300
#define IMG8_BUF_SIZE IMG_MAX_BUFFER_SIZE
#define INLINE_LOG_PATH DEFINE_FILE_PATH "inline_log.txt"

#define INLINE_DATA_OK 0
#define INLINE_DATA_OUTOFRANGE 1
#define INLINE_CSV_DBG DEFINE_FILE_PATH "oppo_inline_result.csv"
#define HBM_STABLE_TIME 500 /* ms */

#define DBG_PITCH_WIDTH 400

#define DBG_CELL_WIDTH 6

#define DBG_HW_INT_WK 1
#define DBG_SHIFT_WK 5
#define DBG_EXPO_START_WK 3.0f

#define DBG_WK_TARGET_PERCENTAGE VENDOR_INLINE_WK_TARGET_PERCENTAGE

#define DBG_EXPO_START_WK_ET701 10.0f
#define DBG_EXPO_END_WK_ET701 200.0f
#define DBG_EXPO_STEP_WK 1.0f /* exp_increase_by = 0.01f; */
#define DBG_EXPO_STEP_WK_DD319 2.0f
#define DBG_HIGH_PTS_WK 50
#define DBG_TARGET_PERCENT_WK 60.0f
#define DBG_IMG_CNT_WK ((DBG_EXPO_END_WK - DBG_EXPO_START_WK) / DBG_EXPO_STEP_WK + 1)
#define DBG_IMG_CNT_STEP2_WK 21
#define DBG_IMG_EXPO_MIDDLE_WK (DBG_EXPO_END_WK + DBG_EXPO_START_WK) / 2

#define DBG_HW_INT_SNR 7
#define DBG_EXPO_TIME_SNR 14.0
#define DBG_SHIFT_SNR 5

#define DBG_ROI_HALF_WIDTH 52
#define DBG_ROI_HALF_HEIGHT 52

#define DBG_ANG_TRY_START 95
#define DBG_ANG_TRY_END 115

#define DBG_PERIOD_TRY_START 5
#define DBG_PERIOD_TRY_END 18

#define DBG_HW_INT_MF DBG_HW_INT_SNR
#define DBG_EXPO_TIME_MF DBG_EXPO_TIME_SNR
#define DBG_SHIFT_MF DBG_SHIFT_SNR
#define MAGIC_NUM 4
#define DBG_BKG TRUE
#define DBG_IMG_CNT_SNR 10

#define DBG_HW_INT_BP DBG_HW_INT_SNR
#define DBG_EXPO_TIME_BP 8.0f
#define DBG_SHIFT_BP DBG_SHIFT_SNR
#define DBG_ROI_H_BP 120
#define DBG_ROI_W_BP 120
#define DBG_STAR_IDX_BP 0
#define DBG_STEP_BP 1
#define DBG_THRESHOLD_1_BP 1.7f
#define DBG_THRESHOLD_2_BP 3.0f
#define DBG_THRESHOLD_3_BP 5.0f

#define DBG_HW_INT_FEA 1
#define DBG_EXPO_TIME_FEA 300.0f
#define DBG_SHIFT_FEA DBG_SHIFT_SNR

#define EGIS_7XX_TEST_RESET 33
#define EGIS_7XX_TEST_SENSOR_TEST_FAIL 34
#define EGIS_7XX_TEST_REGISTER_TEST_FAIL 35
#define EGIS_7XX_TEST_OTP_TEST_FAIL 36
#define EGIS_7XX_TEST_TESTPATTERN_TEST_FAIL 37

#define EXPO_MAX 23
#define EXPO_MIN 3
#define HW_INT_MAX 30
#define HW_INT_MIN 2
#define CENTROIDX_MAX 115
#define CENTROIDX_MIN 85
#define CENTROIDY_MAX 115
#define CENTROIDY_MIN 85

#define SIGNAL_MAX FLT_MAX
#define SIGNAL_MIN 100.0f
#define NOISE_MAX 80.0f
#define NOISE_MIN 0.0f
#define SNR_MAX FLT_MAX
#define SNR_MIN 3.0f
#define BAD_BLOCK_CNT_MAX 4
#define BAD_BLOCK_CNT_MIN 0
#define BAD_PIXEL_MAC_CONTINU_CNT_MAX 40
#define BAD_PIXEL_MAC_CONTINU_CNT_MIN 0
#define BAD_PIXEL_CNT_MAX 100
#define BAD_PIXEL_CNT_MIN 0
#define BKG_CENTER_X_MAX 115
#define BKG_CENTER_X_MIN 85
#define BKG_CENTER_Y_MAX 115
#define BKG_CENTER_Y_MIN 85
#define INTENSITY_DIFF_MAX INT_MAX
#define DBG_THRESHOLD_BP1 2.0f
#define DBG_THRESHOLD_BP2 1.7f
#define DBG_W_STRIDE_BP 1
#define DBG_ROI_PCT_BP 70
#define DBG_roi_radius 120
#define DBG_BP_MT_SIZE 8192

#define DBG_SNR_CTR8_WIDTH 105
#define DBG_SNR_CTR8_HEIGHT 105

struct sensor_test_input {
    int script_id;
    int image_num;
};

typedef struct egfps_final_MT_cali_data {
    //==W KBOX==
    float expo;
    int hw_int;
    int centroid_x;
    int centroid_y;
    int MT_centroid_x;
    int MT_centroid_y;
    //==B BOX==
    int bkg_cx;
    int bkg_cy;
    // CHART
    float signal;
    float noise;
    float snr;
    // FEA_TEST
    float fov_x1_result;
    float fov_x2_result;
    float fov_y1_result;
    float fov_y2_result;
    // PERIOD_TEST
    float period;
    // BP test
    int bad_block_cnt;
    int bad_pxl_max_continu_cnt;
    int bad_pxl_cnt;
    // intensity
    int wk_avg_intensity;
    int bkg_avg_intensity;

    int check_order;
} final_MT_cali_data_t;

#define INLINE_IMAGE_NAME_LENGH 36

#define MAGIC_NUM_INLINE 0x55AA

struct sensor_test_output {
    int result;
    int picture_remaining_count;
    final_MT_cali_data_t data;
    // image debug
    uint16_t picture_buffer_16[IMG_MAX_BUFFER_SIZE * 2];
    uint8_t picture_buffer_8[IMG_MAX_BUFFER_SIZE];
    uint8_t name[INLINE_IMAGE_NAME_LENGH];
};

typedef enum inline_7xx_err_index {
    REGISTER_TEST_SHIFT_BIT,
    OTP_TEST_SHIFT_BIT,
    TESTPATTERN_TEST_SHIFT_BIT,
    EXPO_OVER_SPEC_SHIFT_BIT,
    BKG_X_OVER_SPEC_SHIFT_BIT,
    BKG_Y_OVER_SPEC_SHIFT_BIT,
    FOV_X1_OVER_SPEC_SHIFT_BIT,
    FOV_X2_OVER_SPEC_SHIFT_BIT,
    FOV_Y1_OVER_SPEC_SHIFT_BIT,
    FOV_Y2_OVER_SPEC_SHIFT_BIT,
    PERIOD_OVER_SPEC_SHIFT_BIT,
    SIGNAL_OVER_SPEC_SHIFT_BIT,
    NOISE_OVER_SPEC_SHIFT_BIT,
    SNR_OVER_SPEC_SHIFT_BIT,
    BAD_BLOCK_CNT_OVER_SPEC_SHIFT_BIT,
    BAD_PXL_MAX_CNT_OVER_SPEC_SHIFT_BIT,
    BAD_PXL_CNT_OVER_SPEC_SHIFT_BIT,
} inline_7xx_err_index_t;
