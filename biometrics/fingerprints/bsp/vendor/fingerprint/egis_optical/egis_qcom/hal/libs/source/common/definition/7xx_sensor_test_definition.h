#pragma once
#include "type_definition.h"
#include <stdint.h>

// copy form samsung branch, \sensor\jni\public\include\hw_definition.h
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

/* Version for sensor test */
#define SENSOR_TEST_BUILD 1
/* Log max size. It stores test result */
#define SENSOR_TEST_OUTPUT_LOG_SIZE 2048
#define IMG711_SIZE_BUFF (ET711_SENSOR_WIDTH*ET711_SENSOR_HEIGHT)
#define IMG713_SIZE_BUFF (ET713_SENSOR_WIDTH*ET713_SENSOR_HEIGHT)

#define IMG_MAX_BUFFER_SIZE IMG713_SIZE_BUFF
#define EFS_CALIB_DATA_SIZE 300000
#define DEMO_UI_LOG_BUF		512
#define EFUSE_BUF_SIZE 		32

#define DEFINE_FILE_PATH "/data/vendor/optical_fingerprint/et713/"
#define FILE_NAME_LEN 256
#define IMG8_BUF_SIZE IMG_MAX_BUFFER_SIZE

//Normal
#define TPT_CT_LOG_DBG DEFINE_FILE_PATH"picture_testpattern_ct16.bin"

//SNR
#define SNR_BK_LOG_DBG DEFINE_FILE_PATH"picture_snr_bk16.bin"
#define MF_BK_LOG_DBG  DEFINE_FILE_PATH"picture_mf_bk16.bin"
#define MF_CT_LOG_DBG  DEFINE_FILE_PATH"picture_mf_ct16.bin"
#define FEA_BK_LOG_DBG  DEFINE_FILE_PATH"picture_fea_bk16.bin"
#define SP_BK_LOG_DBG  DEFINE_FILE_PATH"picture_sp_bk16.bin"
#define INLINE_LOG_PATH DEFINE_FILE_PATH"inline_log.txt"
#define SNR_CT_LOG_DBG_1 DEFINE_FILE_PATH"picture_snr_ct16_1.bin"
#define SNR_CT_LOG_DBG_2 DEFINE_FILE_PATH"picture_snr_ct16_2.bin"
#define SNR_CT_LOG_DBG_3 DEFINE_FILE_PATH"picture_snr_ct16_3.bin"
#define SNR_CT_LOG_DBG_4 DEFINE_FILE_PATH"picture_snr_ct16_4.bin"
#define SNR_CT_LOG_DBG_5 DEFINE_FILE_PATH"picture_snr_ct16_5.bin"
#define SNR_CT_LOG_DBG_6 DEFINE_FILE_PATH"picture_snr_ct16_6.bin"
#define SNR_CT_LOG_DBG_7 DEFINE_FILE_PATH"picture_snr_ct16_7.bin"
#define SNR_CT_LOG_DBG_8 DEFINE_FILE_PATH"picture_snr_ct16_8.bin"
#define SNR_CT_LOG_DBG_9 DEFINE_FILE_PATH"picture_snr_ct16_9.bin"
#define SNR_CT_LOG_DBG_10 DEFINE_FILE_PATH"picture_snr_ct16_10.bin"
#define BP_CT_LOG_DBG_1 DEFINE_FILE_PATH"picture_bp_bk16_1.bin"
#define BP_CT_LOG_DBG_2 DEFINE_FILE_PATH"picture_bp_bk16_2.bin"
#define BP_CT_LOG_DBG_3 DEFINE_FILE_PATH"picture_bp_bk16_3.bin"
#define BP_CT_LOG_DBG_4 DEFINE_FILE_PATH"picture_bp_bk16_4.bin"
#define BP_CT_LOG_DBG_5 DEFINE_FILE_PATH"picture_bp_bk16_5.bin"
#define BP_CT_LOG_DBG_6 DEFINE_FILE_PATH"picture_bp_bk16_6.bin"
#define BP_CT_LOG_DBG_7 DEFINE_FILE_PATH"picture_bp_bk16_7.bin"
#define BP_CT_LOG_DBG_8 DEFINE_FILE_PATH"picture_bp_bk16_8.bin"
#define BP_CT_LOG_DBG_9 DEFINE_FILE_PATH"picture_bp_bk16_9.bin"
#define SBR_CT8_LOG_DBG DEFINE_FILE_PATH"picture_snr_ct8_dbg.bin"
#define EXP1_CT_LOG_DBG_1 DEFINE_FILE_PATH"picture_exp1_ct16_exp10.bin"
#define EXP1_CT_LOG_DBG_2 DEFINE_FILE_PATH"picture_exp1_ct16_exp11.bin"
#define EXP1_CT_LOG_DBG_3 DEFINE_FILE_PATH"picture_exp1_ct16_exp12.bin"
#define EXP1_CT_LOG_DBG_4 DEFINE_FILE_PATH"picture_exp1_ct16_exp13.bin"
#define EXP1_CT_LOG_DBG_5 DEFINE_FILE_PATH"picture_exp1_ct16_exp14.bin"
#define EXP1_CT_LOG_DBG_6 DEFINE_FILE_PATH"picture_exp1_ct16_exp15.bin"
#define EXP1_CT_LOG_DBG_7 DEFINE_FILE_PATH"picture_exp1_ct16_exp16.bin"
#define EXP1_CT_LOG_DBG_8 DEFINE_FILE_PATH"picture_exp1_ct16_exp17.bin"
#define EXP1_CT_LOG_DBG_9 DEFINE_FILE_PATH"picture_exp1_ct16_exp18.bin"
#define EXP1_CT_LOG_DBG_10 DEFINE_FILE_PATH"picture_exp1_ct16_exp19.bin"
#define EXP1_CT_LOG_DBG_11 DEFINE_FILE_PATH"picture_exp1_ct16_exp20.bin"
#define EXP1_CT_LOG_DBG_12 DEFINE_FILE_PATH"picture_exp1_ct16_exp21.bin"
#define EXP1_CT_LOG_DBG_13 DEFINE_FILE_PATH"picture_exp1_ct16_exp22.bin"
#define EXP1_CT_LOG_DBG_14 DEFINE_FILE_PATH"picture_exp1_ct16_exp23.bin"
#define EXP1_CT_LOG_DBG_15 DEFINE_FILE_PATH"picture_exp1_ct16_exp24.bin"
#define EXP1_CT_LOG_DBG_16 DEFINE_FILE_PATH"picture_exp1_ct16_exp25.bin"
#define EXP2_CT_LOG_DBG DEFINE_FILE_PATH"picture_exp2_ct16_exp"
#define INLINE_CSV_DBG DEFINE_FILE_PATH"oppo_inline_result.csv"

#define INLINE_IMAGE_NUMBER 5
#define INLINE_IMAGE_SNR_NUMBER 10
#define INLINE_IMAGE_BP_NUMBER 9
#define INLINE_IMAGE_BP_NUMBER 9
#define INLINE_IMAGE_EXPSTEP1_NUMBER 16
#define INLINE_IMAGE_EXPSTEP2_NUMBER 21


#define INLINE_DATA_OK 0
#define INLINE_DATA_OUTOFRANGE 1



#define HBM_STABLE_TIME 500 /* ms */

#define DBG_PITCH_WIDTH 400
#define DBG_CELL_WIDTH 6

#define DBG_HW_INT_WK 1
#define DBG_SHIFT_WK 5
#define DBG_GAIN_WK 0
#define DBG_EXPO_START_WK 10.0f
#define DBG_EXPO_END_WK 25.0f
#define DBG_EXPO_STEP_WK 1.0f /* exp_increase_by = 0.01f; */
#define DBG_HIGH_PTS_WK 50
#define DBG_TARGET_PERCENT_WK 60.0f
#define DBG_IMG_CNT_WK ((DBG_EXPO_END_WK - DBG_EXPO_START_WK) / DBG_EXPO_STEP_WK + 1)
#define DBG_IMG_CNT_STEP2_WK 21

#define DBG_HW_INT_SNR 7
#define DBG_EXPO_TIME_SNR 14.0
#define DBG_SHIFT_SNR 5
#define DBG_GAIN_SNR 0

#define DBG_ROI_HALF_WIDTH 52
#define DBG_ROI_HALF_HEIGHT 52
#define DBG_ANG_TRY_START 104
#define DBG_ANG_TRY_END 106
#define DBG_PERIOD_TRY_START 5
#define DBG_PERIOD_TRY_END 18

#define DBG_HW_INT_MF DBG_HW_INT_SNR
#define DBG_EXPO_TIME_MF DBG_EXPO_TIME_SNR
#define DBG_SHIFT_MF DBG_SHIFT_SNR
#define DBG_GAIN_MF DBG_GAIN_SNR
#define MAGIC_NUM 4
#define DBG_BKG TRUE
#define DBG_IMG_CNT_SNR 10

#define DBG_HW_INT_BP DBG_HW_INT_SNR
#define DBG_EXPO_TIME_BP 8.0f
#define DBG_SHIFT_BP DBG_SHIFT_SNR
#define DBG_GAIN_BP DBG_GAIN_SNR
#define DBG_IMG_CNT_BP 9
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
#define DBG_GAIN_FEA DBG_GAIN_SNR

#define DBG_GAIN_MOIRE 0

#define	EGIS_7XX_TEST_RESET 33
#define	EGIS_7XX_TEST_READ_SENSOR_INFO_START 34
#define	EGIS_7XX_TEST_READ_SENSOR_INFO_END 35
#define	EGIS_7XX_TEST_SENSOR_TEST_FAIL 37



#define	EXPO_MAX 30
#define	EXPO_MIN 5
#define	HW_INT_MAX 30
#define	HW_INT_MIN 5
#define	FOVX1_MAX 6500.0f
#define	FOVX1_MIN 2000.0f
#define	FOVX2_MAX 6500.0f
#define	FOVX2_MIN 2000.0f
#define	FOVY1_MAX 6500.0f
#define	FOVY1_MIN 2000.0f
#define	FOVY2_MAX 6500.0f
#define	FOVY2_MIN 2000.0f
#define	PERIOD_MAX 11.0f
#define	PERIOD_MIN 3.0f
#define	SIGNAL_MAX 600.0f
#define	SIGNAL_MIN 50.0f
#define	NOISE_MAX 800.0f
#define	NOISE_MIN 0.0f
#define	SNR_MAX 1000.0f
#define	SNR_MIN 0.0f
#define	BAD_BLOCK_CNT_MAX 1
#define	BAD_BLOCK_CNT_MIN 0
#define	BAD_PIXEL_MAC_CONTINU_CNT_MAX 10
#define	BAD_PIXEL_MAC_CONTINU_CNT_MIN 0
#define	BAD_PIXEL_CNT_MAX 100
#define	BAD_PIXEL_CNT_MIN 0
#define	BKG_CENTER_X_MAX 115
#define	BKG_CENTER_X_MIN 85
#define	BKG_CENTER_Y_MAX 115
#define	BKG_CENTER_Y_MIN 85



struct sensor_test_input {
	int script_id;
	int image_num;
};

typedef struct egfps_final_MT_cali_data{
//==W KBOX==
	float expo;
        float expo_step1;
	int hw_int;
	int centroid_x;
	int centroid_y;
//==B BOX==
	//uint16_t * picture_sp_bk16;// get from calibration
//CHART
	float signal;
	float noise;
	float snr;
// MAG 
	int ridge;
	int valley;
	double mag_val;
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
//Flash 	
	int  calib_result;
//bkg center
	int  bkg_cx;
	int  bkg_cy;
} final_MT_cali_data_t;

typedef struct _EGIS_SVAE_INLINE_DATA {
    double inline_expo_time;
    int inline_hw_int;
    int bkg_cx;
    int bkg_cy;
    int magic_num;
} EGIS_SVAE_INLINE_DATA;

#define MAGIC_NUM_INLINE 0x55AA

struct sensor_test_output {
    int result;
    int size;	
    final_MT_cali_data_t data;
    uint16_t picture_buffer_16[IMG_MAX_BUFFER_SIZE * 2];
    uint8_t  picture_buffer_8[IMG_MAX_BUFFER_SIZE];
    uint8_t  UID[9];
};
