/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_DELMAR_TYPES_H_
#define _GF_DELMAR_TYPES_H_

#include "gf_sensor_types.h"
#include "gf_algo_types.h"
#include "gf_config_type.h"
#include "gf_delmar_config.h"
#include "gf_fpcore_types.h"

#define GF_EXT_UNTRUST_ENROLL_GID 0xFFFFFFFF
#ifndef CUSTOMIZE_TEMPLATE_PATH
#define CUSTOMIZE_TEMPLATE_PATH   "/data/vendor/goodix/fpdata/"
#endif  // CUSTOMIZE_TEMPLATE_PATH

#ifndef MAX_SENSOR_NUM
#define MAX_SENSOR_NUM                             (4)
#endif  // MAX_SENSOR_NUM

#define DELMAR_MAX_CONTINUE_FRAME_NUM              (5)
#define DELMAR_ALGO_VERSION_INFO_LEN               (64)
#define DELMAR_ALGO_VERSION_ID                     (4)
#define DELMAR_PRODUCTION_ALGO_VERSION_INFO_LEN    (64)
#define DELMAR_GF_SENSOR_ID_LEN                    (14)
#define DELMAR_SENSOR_OTP_INFO_LEN                 (14)
#define DELMAR_RUBBER_NUM                 (3)
#define DELMAR_VENDOR_ID_LEN        (1)
#define DELMAR_WAFER_LOT_ID_LEN        (8)
#define DELMAR_WAFER_NO_LEN        (1)
#define DELMAR_DIE_COORDINATE_LEN        (1)

#define DELMAR_FAKE_LOG_LNE                        (256 * 4)
// for delmar filter
#define DELMAR_FILTER_CSV_WIDTH                    (256)
#define DELMAR_FILTER_CSV_HEIGHT                   (256)
#define DELMAR_ORIGIN_BUFFER_LEN                   (183456)  // 364*252*sizeof(uint16_t)
#define DELMAR_RAW_BUFFER_LEN                      (40000)  // 200*200, word(2 bytes) len
#define DELMAR_RAW_CF_BUFFER_LEN                   (80000)  // 200*200*sizeof(uint16_t)
#define DELMAR_BMP_BUFFER_LEN                      (40000)  // 200*200, byte(1 bytes) len
#define DELMAR_CHIP_ID_BUFFER_LEN                  (32)
#define DELMAR_VENDOR_ID_BUFFER_LEN                (2)
#define DELMAR_PRODUCTION_DATE_BUFFER_LEN          (4)
#define DELMAR_SENSOR_ID_BUFFER_LEN                (14)
#define DELMAR_OTP_INFO_BUFFER_LEN                 (64)
#define DELMAR_MT_INFO_BUFFER_LEN                  (256)
#define DELMAR_MT_INFO_CRC_LEN                     (4)
#define DELMAR_MT_BACKUP_BUFFER_LEN                (DELMAR_SENSOR_ID_BUFFER_LEN + DELMAR_MT_INFO_BUFFER_LEN)
#define DELMAR_BAK_FILE_PATH_MAX_LEN               (256)
#define DELMAR_AUTOCALI_PARAM_BUFFER_LEN           (936989)
#define DELMAR_CF_POINT_BUFFER_LEN                 (768)  // 4*6*(4+6+6)*sizeof(uint16_t)
#define DELMAR_ALL_FINGER_ID_LEN                   (128)
#define DELMAR_CALI_TEST_STAGE                     (sizeof(uint32_t))
#define DELMAR_CALI_HEAD_CRC                       (sizeof(uint32_t))
#define DELMAR_CALI_LEN_KB                         (sizeof(uint32_t))
#define DELMAR_CALI_CRC_KB                         (sizeof(uint32_t))
#define DELMAR_CRC32_LEN_KB       \
    (DELMAR_CALI_TEST_STAGE + DELMAR_CALI_HEAD_CRC + DELMAR_CALI_LEN_KB + DELMAR_CALI_CRC_KB)
#define DELMAR_CALI_DATE_INFO_LEN    (sizeof(uint16_t))
#define DELMAR_CALI_ALGO_VER_LEN    (sizeof(uint16_t))
#define DELMAR_CALIBRATION_HEADER_LEN       \
    (DELMAR_CALI_TEST_STAGE + DELMAR_CALI_DATE_INFO_LEN + DELMAR_CALI_ALGO_VER_LEN + DELMAR_CALI_HEAD_CRC + DELMAR_CALI_LEN_KB + DELMAR_CALI_CRC_KB + (sizeof(uint32_t) * 7))
#define DELMAR_CALIBRATION_DATA_LEN                (191813)
#define DELMAR_CALIBRATION_TOTAL_LEN               (DELMAR_CALIBRATION_HEADER_LEN + DELMAR_CALIBRATION_DATA_LEN)
#define DELMAR_CALI_SUB_KB_DATA_LEN                (116108)
#define DELMAR_ITO_DEPTH_BUFFER_LEN                (1600)
#define DELMAR_CALI_BACKUP_BUFFER_LEN              (DELMAR_CALIBRATION_HEADER_LEN + DELMAR_CALIBRATION_DATA_LEN + DELMAR_SENSOR_ID_BUFFER_LEN)

#define DELMAR_CF_DARK_PIXEL_AVG_LEN        (360)  // 180*2=360
#define DELMAR_CF_DATA_CRC_LEN        (sizeof(uint32_t))
#define DELMAR_CF_DATA_LEN_VALUE        (sizeof(uint32_t))
#define DELMAR_CF_HEADER_CRC_LEN        (sizeof(uint32_t))
#define DELMAR_CF_ALL_DATA_INFO_LEN        (DELMAR_CF_HEADER_CRC_LEN + DELMAR_CF_DATA_LEN_VALUE + DELMAR_CF_DATA_CRC_LEN + DELMAR_CF_DARK_PIXEL_AVG_LEN + DELMAR_CF_POINT_BUFFER_LEN)

#define SPLIT_UINT64_TO_UINT32_ARGS(x)   (uint32_t) ((x) >> 32), (uint32_t) (x)

#define DELMAR_SENSOR_UI_TYPE_CIRCLE            (0)
#define DELMAR_SENSOR_UI_TYPE_ELLIPSE            (1)

#define DELMAR_DL_DENOISE_DATA_LEN              (366 * 1024)
#define DELMAR_DL_FEATURE_DATA_LEN              (300 * 1024)
#define DELMAR_RAW_FEATURE_OLD_LEN              (500 * 1024)
#define DELMAR_BW_MASK_WIDTH                    (120)
#define DELMAR_BW_MASK_HEIGHT                   (120)
#define DELMAR_BW_MASK_LEN                      (DELMAR_BW_MASK_WIDTH*DELMAR_BW_MASK_HEIGHT)
#define WAFER_LOT_ID             (8)
#define OTP_RANDOM_NO    (2)
#define PRODUCT_ID    (2)
#define PROGRAM_VERSION    (2)
#define TEST_TIME    (4)
#define DELMAR_AA_COL 120
#define DELMAR_AA_ROW 180

#define GET_FEATURE_BUFFER_SIZE    (3 * 1024 * 1024 / 2)
#define DELMAR_SCREEN_VENDOR_PROPERTY "sys.panel.display"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define GF_DELMAR_FAKE_ERROR(err) ((err) == GF_ERROR_NOT_LIVE_FINGER)
typedef gf_error_t (*user_data_checker)(uint8_t* data, uint32_t data_len);

typedef enum {
    GF_CHIP_DELMAR_T_SE1,
    GF_CHIP_DELMAR_CS_SE1,
    GF_CHIP_DELMAR_UNKNOWN
} gf_delmar_chip_type_t;

typedef enum {
    DELMAR_MODE_UNKOWN = 0,
    DELMAR_MODE_SLEEP = 1,
    DELMAR_MODE_IDLE,
    DELMAR_MODE_CAPTURE,
    DELMAR_MODE_MAX,
} gf_delmar_mode_t;

typedef enum {
    DELMAR_CALI_STATE_KB_READY_BIT = 0,
    DELMAR_CALI_STATE_PGA_GAIN_READY_BIT,
    DELMAR_CALI_STATE_LB_PGA_GAIN_READY_BIT,  // low brightness pga gain
    DELMAR_CALI_STATE_MAX = 32,
} gf_delmar_cali_state_bit_t;

typedef enum {
    GF_CMD_DELMAR_SENSOR_BASE = 100,
    GF_CMD_DELMAR_SENSOR_READ_IMAGE = 101,
    GF_CMD_DELMAR_SENSOR_CALCULATE_MASK = 102,
    GF_CMD_DELMAR_SENSOR_NOTIFY_RESET = 103,
    GF_CMD_DELMAR_SENSOR_BRIGHTNESS_LEVEL = 104,
} gf_delmar_sensor_cmd_id_t;

enum {
    MSG_UNTRUSTED_ENROLL_START = 1000,
    MSG_UNTRUSTED_AUTHENTICATE_START,
};

enum {
    DELMAR_MORPHOTYPE_SINGLE = 0,    // 1x1
    DELMAR_MORPHOTYPE_TWO_MUL_TWO,   // 2x2
    DELMAR_MORPHOTYPE_ONE_MUL_FOUR,  // 1x4
    DELMAR_MORPHOTYPE_TWO_MUL_FOUR,  // 2x4
};

typedef struct {
    gf_cmd_header_t header;
    gf_delmar_config_t o_delmar_config;
    gf_delmar_sensor_type_t o_sensor_type;
    uint32_t o_cali_state;
    uint32_t o_sensor_num;
    uint8_t o_morphotype;
    uint8_t o_cf_mark;
    uint8_t o_cf_mask_type;
    uint8_t io_sdcard_otp[MAX_SENSOR_NUM][DELMAR_OTP_INFO_BUFFER_LEN];
    uint32_t io_sdcard_otp_len[MAX_SENSOR_NUM];
    int32_t file_unlock_en;
} gf_delmar_sensor_init_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t o_cali_state;
    uint32_t preprocess_col;
    uint32_t preprocess_row;
} gf_delmar_algo_init_t;

typedef struct {
    uint8_t android_version[ANDROID_VERSION_LEN];
    uint8_t service_version[SERVICE_VERSION_LEN];
    uint8_t platform[TARGET_PLATFORM_LEN];
    uint8_t fw_version[FW_VERSION_INFO_LEN];
    uint8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    uint8_t algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    uint8_t preprocess_version[DELMAR_ALGO_VERSION_INFO_LEN];
    uint8_t production_algo_version[DELMAR_PRODUCTION_ALGO_VERSION_INFO_LEN];
    uint8_t chip_id[MAX_SENSOR_NUM][DELMAR_CHIP_ID_BUFFER_LEN];
    uint32_t chip_id_len[MAX_SENSOR_NUM];
    uint8_t vendor_id[MAX_SENSOR_NUM][DELMAR_VENDOR_ID_BUFFER_LEN];
    uint32_t vendor_id_len[MAX_SENSOR_NUM];
    uint8_t sensor_id[MAX_SENSOR_NUM][DELMAR_SENSOR_ID_BUFFER_LEN];
    uint32_t sensor_id_len[MAX_SENSOR_NUM];
    uint16_t ito_id[MAX_SENSOR_NUM];
    uint16_t pga_gain[MAX_SENSOR_NUM];
    uint16_t exposure_time[MAX_SENSOR_NUM];
    uint32_t chip_type;
    uint32_t sensor_row;
    uint32_t sensor_col;
    uint32_t preprocess_col[MAX_SENSOR_NUM];
    uint32_t preprocess_row[MAX_SENSOR_NUM];
    uint8_t anti_proofing_version[DELMAR_ALGO_VERSION_INFO_LEN];
} gf_delmar_dump_base_info_t;

// only dump one time in enroll or authenticate because of the data is not changed
typedef struct {
    uint8_t sensor_index;
    int16_t k[DELMAR_RAW_BUFFER_LEN];
    uint8_t k_bmp[DELMAR_BMP_BUFFER_LEN];
    int16_t b[DELMAR_RAW_BUFFER_LEN];
    uint8_t b_bmp[DELMAR_BMP_BUFFER_LEN];
    uint8_t cali_moire_data[DELMAR_CALIBRATION_DATA_LEN];
    int32_t cali_moire_data_len;
    uint8_t cali_mode;
    uint16_t black_base[DELMAR_RAW_BUFFER_LEN];
    uint32_t black_base_len;
    int16_t moire_filter[DELMAR_FILTER_CSV_WIDTH *
        DELMAR_FILTER_CSV_HEIGHT];
    uint32_t moire_filter_len;
    int32_t flesh_data[DELMAR_RAW_BUFFER_LEN];
    uint32_t flesh_data_len;
    int16_t kr[DELMAR_RAW_BUFFER_LEN];
    uint32_t kr_len;
    uint8_t kr_bmp[DELMAR_BMP_BUFFER_LEN];
    int16_t br[DELMAR_RAW_BUFFER_LEN];
    uint32_t br_len;
    uint8_t br_bmp[DELMAR_BMP_BUFFER_LEN];
    uint8_t ito[DELMAR_RAW_BUFFER_LEN];
    uint32_t ito_len;
    uint8_t ito_bmp[DELMAR_BMP_BUFFER_LEN];
    int32_t ito_row_depth[DELMAR_ITO_DEPTH_BUFFER_LEN];
    uint32_t ito_row_depth_len;
    int32_t ito_col_depth[DELMAR_ITO_DEPTH_BUFFER_LEN];
    uint32_t ito_col_depth_len;
    uint16_t highlight_flesh_base_cf[DELMAR_RAW_BUFFER_LEN];
    uint16_t highlight_black_base_cf[DELMAR_RAW_BUFFER_LEN];
    uint32_t highlight_base_cf_len;
    uint32_t preprocess_col;
    uint32_t preprocess_row;
    uint16_t highlight_flesh_base[DELMAR_RAW_BUFFER_LEN];
    uint16_t highlight_black_base[DELMAR_RAW_BUFFER_LEN];
    uint32_t base_col;
    uint32_t base_row;
    int32_t signal_LPF[MAX_SENSOR_NUM];
} gf_delmar_dump_cali_data;

typedef struct {
    uint8_t sensor_index;
    uint8_t autocali_param[DELMAR_AUTOCALI_PARAM_BUFFER_LEN];
    uint32_t autocali_param_len;
} gf_delmar_dump_auto_cali_params;

typedef struct {
    uint16_t raw_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint32_t raw_data_len[MAX_SENSOR_NUM];
    uint8_t raw_data_cf[MAX_SENSOR_NUM][DELMAR_RAW_CF_BUFFER_LEN];
    uint32_t raw_data_cf_len;
#ifdef SUPPORT_DUMP_ORIGIN_DATA
    uint8_t origin_data[MAX_SENSOR_NUM][DELMAR_ORIGIN_BUFFER_LEN];
    uint32_t origin_col;
    uint32_t origin_row;
#endif  //  SUPPORT_DUMP_ORIGIN_DATA
    uint8_t data_bmp[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint8_t data_decrypt_bmp[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint8_t black_set[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint32_t black_set_len[MAX_SENSOR_NUM];
    uint32_t preprocess_col[MAX_SENSOR_NUM];
    uint32_t preprocess_row[MAX_SENSOR_NUM];
    int32_t sig_rate[MAX_SENSOR_NUM];
    uint32_t frame_num[MAX_SENSOR_NUM];
    uint16_t pga_gain[MAX_SENSOR_NUM];
    uint16_t exposure_time[MAX_SENSOR_NUM];
    uint32_t get_raw_data_time[MAX_SENSOR_NUM];
    uint32_t preprocess_time[MAX_SENSOR_NUM];
    uint32_t get_feature_time[MAX_SENSOR_NUM][2];
    uint32_t key_point_num[MAX_SENSOR_NUM][2];
    uint32_t overlay[MAX_SENSOR_NUM][2];
    uint32_t image_quality[MAX_SENSOR_NUM][2];
    uint32_t valid_area[MAX_SENSOR_NUM][2];
    uint32_t broken_check_time[MAX_SENSOR_NUM];
    uint32_t bad_rawdata_flag[MAX_SENSOR_NUM];
    uint8_t mask_data[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint32_t mask_data_len[MAX_SENSOR_NUM];
    uint8_t particular_result[MAX_SENSOR_NUM][2];
    uint32_t sensor_x[MAX_SENSOR_NUM];
    uint32_t sensor_y[MAX_SENSOR_NUM];
    float sensor_rotation[MAX_SENSOR_NUM];
    uint32_t press_x;
    uint32_t press_y;
    uint32_t press_radius;
    uint32_t press_major;  // ellipse major axis
    uint32_t press_minor;  // ellipse minor axis
    int32_t press_rotation;  // ellipse press rotation
    uint32_t group_id;

    // follow ST_BDNAME_STANDARD
    int32_t dark_preprocess[MAX_SENSOR_NUM];
    int32_t cali_state[MAX_SENSOR_NUM];
    int32_t raw_data_mean[MAX_SENSOR_NUM];
    int32_t broken_level[MAX_SENSOR_NUM];
    int32_t spoof_level[MAX_SENSOR_NUM];
    int32_t bio_assay_ret[MAX_SENSOR_NUM];
    int32_t moire_detect[MAX_SENSOR_NUM];
    int32_t partial_touch[MAX_SENSOR_NUM];
    int32_t compensation_mode[MAX_SENSOR_NUM];
    int32_t reverse_recog[MAX_SENSOR_NUM];
    int32_t safe_level[MAX_SENSOR_NUM];
    int32_t weak_level[MAX_SENSOR_NUM];
    int32_t motion_x[MAX_SENSOR_NUM];
    int32_t motion_y[MAX_SENSOR_NUM];
    int32_t motion_cost[MAX_SENSOR_NUM];
#ifdef DEBUG_DUMP_CALIED_IMAGE_DATA
    uint8_t puch_image_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
    uint16_t ps_cali_data[MAX_SENSOR_NUM][DELMAR_RAW_BUFFER_LEN];
#endif  // DEBUG_DUMP_CALIED_IMAGE_DATA
    uint8_t raw_feature_old_arr[MAX_SENSOR_NUM][DELMAR_RAW_FEATURE_OLD_LEN];
    uint32_t raw_feature_old_arr_len[MAX_SENSOR_NUM];
    int16_t diff_image[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint8_t color_point_data[MAX_SENSOR_NUM][DELMAR_CF_POINT_BUFFER_LEN];
    uint32_t color_point_data_len;
    uint8_t fake_mask_data[MAX_SENSOR_NUM][DELMAR_BMP_BUFFER_LEN];
    uint32_t fake_mask_data_len[MAX_SENSOR_NUM];
    uint8_t is_highlight[MAX_SENSOR_NUM];
} gf_delmar_dump_base_data_t;

typedef struct {
    uint32_t match_score[MAX_SENSOR_NUM][2];
    uint32_t match_finger_id[MAX_SENSOR_NUM][2];
    uint32_t study_flag[MAX_SENSOR_NUM][2];
    uint32_t authenticate_time[MAX_SENSOR_NUM][2];
    uint8_t auth_order[MAX_SENSOR_NUM];
    uint8_t mask_bw[MAX_SENSOR_NUM][2][DELMAR_BW_MASK_LEN];
    uint8_t enrolled_finger_ids[DELMAR_ALL_FINGER_ID_LEN];  // separated with '#'
    uint32_t mask_bw_len;
    uint64_t valid_sensor_ids;
    uint32_t temp_flag;
    int32_t nFakeParamV1Arr[2][20];
    int32_t nFakeParamV2Arr[2][20];
    int32_t fFakeParamV1Arr[2][20];
    int32_t  fFakeParamV2Arr[2][20];
    gf_delmar_dump_base_data_t base_data;
    uint32_t fake_result;
    uint32_t nfv20[2];
    uint32_t nfv21[2];
} gf_delmar_dump_auth_t;

typedef struct {
    uint32_t increase_rate_between_stitch_info[MAX_SENSOR_NUM][2];
    uint32_t overlap_rate_between_last_template[MAX_SENSOR_NUM][2];
    uint32_t enrolling_finger_id;
    uint32_t duplicated_finger_id[2];
    uint32_t enroll_time[2];
    uint64_t valid_sensor_ids;
    gf_delmar_dump_base_data_t base_data;
    int32_t nFakeParamV1Arr[2][20];
    int32_t nFakeParamV2Arr[2][20];
    int32_t fFakeParamV1Arr[2][20];
    int32_t  fFakeParamV2Arr[2][20];
    uint8_t mask_bw[MAX_SENSOR_NUM][2][DELMAR_BW_MASK_LEN];
    uint32_t mask_bw_len;
    uint32_t fake_result;
} gf_delmar_dump_enroll_t;


typedef enum {
    DUMP_OP_BK_TOMBSTONE = DUMP_OP_MAX + 1,
    DUMP_OP_CIRCLE_BMP,
    DUMP_OP_PERFORMACE_RAW_DATA,
    DUMP_OP_PERFORMACE_PARAMETERS,
    DUMP_OP_PGA_GAIN_RECALCULATE,
    DUMP_OP_TEST_DATA_COLLECT,
    DUMP_OP_FRRFAR_CALI,
    DUMP_OP_FRRFAR_RAW_DATA,
    DUMP_OP_FRRFAR_FFD_DATA,
    DUMP_OP_FRRFAR_PREPROC_DATA,
    DUMP_OP_RGB_DATA_COLLECT,
    DUMP_OP_CALI,
    DUMP_OP_AUTO_CALI,
    DUMP_OP_PERFORMACE_ORIGIN_DATA,
    DUMP_OP_SAMPLING_DATA,
    DUMP_OP_LB_PGA_GAIN_RECALCULATE,
    DUMP_OP_BK_MAX
} gf_delmar_dump_op_t;

typedef struct {
    int32_t press_x;
    int32_t press_y;
    int32_t sensor_x[MAX_SENSOR_NUM];
    int32_t sensor_y[MAX_SENSOR_NUM];
    float sensor_rotation[MAX_SENSOR_NUM];
    int32_t radius;
} gf_delmar_coordinate_info_t;

typedef struct {
    int32_t touch_major;
    int32_t touch_minor;
    int32_t touch_orientation;
} gf_delmar_press_area_info_t;

typedef struct {
    gf_capture_image_t common;
    uint64_t valid_sensor_ids;
    uint32_t black_screen_capture;  // 0: light capure 1: black capture
} gf_delmar_capture_image_t;

typedef struct {
    int32_t image_quality[MAX_SENSOR_NUM][2];
    int32_t valid_area[MAX_SENSOR_NUM][2];
    int32_t match_score[MAX_SENSOR_NUM][2];
    uint32_t authenticate_study_flag[MAX_SENSOR_NUM][2];
    int32_t key_point_num[MAX_SENSOR_NUM][2];
    uint32_t overlay[MAX_SENSOR_NUM][2];
    uint32_t broken_check_time[MAX_SENSOR_NUM];
    uint32_t preprocess_time[MAX_SENSOR_NUM];
    uint32_t get_feature_time[MAX_SENSOR_NUM][2];
    // dsp get feature time, valid excpet speed up auth
    uint32_t fast_auth_get_feature1to3_time[MAX_SENSOR_NUM][2];
    uint32_t fast_auth_get_feature4_time[MAX_SENSOR_NUM][2];
    uint32_t fast_auth_get_feature5_time[MAX_SENSOR_NUM][2];
    uint32_t get_feature1_time[MAX_SENSOR_NUM][2];
    uint32_t get_feature2_time[MAX_SENSOR_NUM][2];
    uint32_t get_feature3_time[MAX_SENSOR_NUM][2];
    uint32_t get_feature4_time[MAX_SENSOR_NUM][2];
    uint32_t get_feature5_time[MAX_SENSOR_NUM][2];

    uint32_t enroll_time[MAX_SENSOR_NUM][2];
    uint32_t authenticate_time[MAX_SENSOR_NUM][2];
    uint32_t speed_up_authenticate_time[MAX_SENSOR_NUM][2];
    uint32_t total_time[MAX_SENSOR_NUM];
    uint32_t study_flag[MAX_SENSOR_NUM][2];
    int32_t update[MAX_SENSOR_NUM];
    uint32_t weak_update[MAX_SENSOR_NUM];
    uint32_t increase_rate[MAX_SENSOR_NUM][2];
    uint32_t read_image_time[MAX_SENSOR_NUM];
    uint32_t capture_image_time;
    uint32_t get_raw_data_time;
    uint64_t valid_sensor_ids;
    uint32_t works_image_counts;
    uint32_t auth_order[MAX_SENSOR_NUM];
} gf_delmar_algo_performance_t;

typedef struct {
    gf_cmd_header_t header;
    uint64_t i_sensor_ids;
    uint8_t i_ui_type;
    uint8_t i_has_press_area_info;
    gf_delmar_coordinate_info_t coordinate_info;
    gf_delmar_press_area_info_t press_area_info;
} gf_delmar_read_image_t;


typedef struct {
    gf_cmd_header_t header;
    uint64_t i_sensor_ids;
    uint8_t i_ui_type;
    uint8_t i_has_press_area_info;
    gf_delmar_coordinate_info_t coordinate_info;
    gf_delmar_press_area_info_t press_area_info;
} gf_delmar_calculate_mask_t;

typedef struct {
    gf_cmd_header_t header;
    gf_delmar_algo_performance_t o_performance;
} gf_delmar_algo_kpi_t;

typedef struct {
    gf_algo_enroll_image_t common;
    int32_t o_error[MAX_SENSOR_NUM][2];
    uint8_t i_temp;
} gf_delmar_algo_enroll_image_t;

typedef struct {
    gf_algo_auth_image_t common;
    uint64_t i_auth_sensor_ids;
    uint8_t i_is_first_auth;
    uint8_t o_auth_success_sensor_id;
    uint8_t i_is_first_part;
    uint32_t i_temp_flag;
    uint32_t i_dsp_get_feature2_time;
    uint32_t i_get_feature4_time;
#ifdef SUPPORT_DUMP_DSP_GET_FEATURE
    uint8_t o_getfeature_ret[2 * GET_FEATURE_BUFFER_SIZE];
#else  // SUPPORT_DUMP_DSP_GET_FEATURE
    uint8_t* o_getfeature_ret;  // should not be used
#endif  // SUPPORT_DUMP_DSP_GET_FEATURE
    uint64_t auth_time_flag;
} gf_delmar_algo_auth_image_t;

typedef enum {
    GF_DELMAR_CMD_UPDATE_SENSOR_IDS = GF_CMD_ALGO_MAX + 1,
    GF_DELMAR_CMD_ALGO_AUTHENTICATE_END,
    GF_DELMAR_CMD_ALGO_MAX
} gf_delmar_algo_cmd_id_t;

typedef struct {
    gf_cmd_header_t header;
    uint64_t study_sensor_ids;  // for study
    uint64_t valid_sensor_ids;  // for dump
} gf_delmar_sensor_ids_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t finger_id;
} gf_delmar_auth_end_t;

typedef enum {
    GF_DELMAR_OPERATION_FIND_SENSOR = GF_OPERATION_MAX + 1,
    GF_DELMAR_OPERATION_MAX
} gf_delmar_operation_t;

typedef struct {
    uint8_t wafer_lot_id[WAFER_LOT_ID];         // 0x00~0x07
    uint8_t wafer_no;                                         // 0x08
    uint8_t x_coordinate;                                   // 0x09
    uint8_t y_coordinate;                                   // 0x0A
    uint8_t random_no[OTP_RANDOM_NO];       // 0x0B~0x0C
    uint8_t cp_crc;                                              // 0x0D
    uint8_t trim_param_osc80m;                      // 0x0E
    uint8_t trim_param_wdt32k;                      // 0x0F
    uint8_t room_temp;                                      // 0x10
    uint8_t vbg_trim;                                           // 0x11
    uint8_t chip_type_otp_ver;                          // 0x12
    uint8_t lens_type_cp_info;                           // 0x13
    uint8_t cp_reserve[5];                                   // 0x14~0x16, 0x17~0x18
    uint8_t cp2_crc;                                             // 0x19
    uint8_t substrate_type;                              // 0x1A
    uint8_t program_version[PROGRAM_VERSION];  // 0x1B~0x1C
    uint8_t test_time[TEST_TIME];                       // 0x1D~0x20
    uint8_t ic_type;                                                // 0x21
    uint8_t ft_reserve;                                          // 0x22
    uint8_t ft_crc;                                                  // 0x23
    uint8_t smt1_info[2];                                       // 0x24~0x25
    uint8_t hw_qr_code[11];                                 // 0x26~0x30
    uint8_t smt1_reserve[3];                                // 0x31~0x33
    uint8_t smt1_crc;                                             // 0x34
    uint8_t smt2_info[2];                                       // 0x35~0x36
    uint8_t smt2_reserve[2];                                // 0x37~0x38
    uint8_t smt2_crc;                                            // 0x39
    uint8_t mt_info[2];                                           // 0x3A~0x3B
    uint8_t mt_pga_gain;                                        // 0x3C
    uint8_t mt_reserve[2];                                     // 0x3D~0x3E
    uint8_t mt_crc;                                                 // 0x3F
} gf_delmar_sensor_otp_info_t;

typedef struct {
    float    normal_expo_time;
    uint16_t normal_expo_gain;
    float    long_expo_time;
    uint16_t long_expo_gain;
    uint16_t osc;
} gf_delmar_sampling_cfg_t;

typedef struct {
    int32_t agl;
    uint32_t px;
    uint32_t py;
    uint16_t amp;
    uint8_t sigma;
} __attribute__((packed)) gf_delmar_preprocess_burlap_info_t;
typedef struct {
    gf_delmar_preprocess_burlap_info_t cali_burlap_info;
    gf_delmar_preprocess_burlap_info_t autocali_burlap_info;
    uint32_t data_process_version;  // keep it the same with data_process
    int32_t signal_LPF[MAX_SENSOR_NUM];
} __attribute__((packed)) gf_delmar_burlap_info_t;
#endif /* _GF_DELMAR_TYPES_H_ */
