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

#if MAX_SENSOR_NUM == 1
#define MAX_VALID_SENSOR_NUM 1
#elif defined SUPPORT_MULTIPLE_FINGER_AUTH
#define MAX_VALID_SENSOR_NUM MAX_SENSOR_NUM
#elif MAX_SENSOR_NUM == 4  // MAX_VALID_SENSOR_NUM
#define MAX_VALID_SENSOR_NUM 2
#else   // MAX_VALID_SENSOR_NUM
#define MAX_VALID_SENSOR_NUM MAX_SENSOR_NUM
#endif  // MAX_VALID_SENSOR_NUM

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

#define DELMAR_FAKE_LOG_ARRAY_SIZE                 (20)
#define DELMAR_FAKE_LOG_BUFFER_LEN                 (1024)
// for delmar filter
#define DELMAR_FILTER_CSV_WIDTH                    (256)
#define DELMAR_FILTER_CSV_HEIGHT                   (256)
#define DELMAR_ORIGIN_BUFFER_LEN                   (183456)  // 364*252*sizeof(uint16_t)
#define DELMAR_RAW_BUFFER_LEN                      (40000)  // 200*200, word(2 bytes) len
#define DELMAR_RAW_CF_BUFFER_LEN                   (80000)  // 200*200*sizeof(uint16_t)
#define DELMAR_BMP_BUFFER_LEN                      (40000)  // 200*200, byte(1 bytes) len
#define DELMAR_RW_FLASH_BUFFER_LEN                 (4096)   // 4*1024, byte(1 bytes) len
#define DELMAR_CHIP_ID_BUFFER_LEN                  (32)
#define DELMAR_VENDOR_ID_BUFFER_LEN                (2)
#define DELMAR_PRODUCTION_DATE_BUFFER_LEN          (4)
#define DELMAR_SENSOR_ID_BUFFER_LEN                (14)
#define DELMAR_OTP_INFO_BUFFER_LEN                 (64)
#define DELMAR_MT_INFO_BUFFER_LEN                  (256)
#define DELMAR_MT_INFO_CRC_LEN                     (4)
#define DELMAR_MT_BACKUP_BUFFER_LEN                (DELMAR_SENSOR_ID_BUFFER_LEN + DELMAR_MT_INFO_BUFFER_LEN)
#define DELMAR_BAK_FILE_PATH_MAX_LEN               (256)
#define DELMAR_AUTOCALI_PARAM_BUFFER_LEN           (635505)
#define DELMAR_T_CF_POINT_BUFFER_LEN                768   // 4*6*(4+6+6)*sizeof(uint16_t)
#define DELMAR_S_CF_POINT_BUFFER_LEN                1728
#define DELMAR_CF_POINT_BUFFER_LEN                  DELMAR_S_CF_POINT_BUFFER_LEN    // max len delmarS(1728)/delmt(768)
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
#define DELMAR_AA_DATA_LEN      (g_sensor_data.aa_data_width * g_sensor_data.aa_data_height * sizeof(uint16_t))

#define GET_FEATURE_BUFFER_SIZE    (3 * 1024 * 1024 / 2)
#define MAX_QR_CODE_INFO_LEN                 (32)

#define MAX_PT_NUM               (2)
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define TEST_MAX_COMPUTE_FRAME_NUM (7)  // compute the position of auto find sensor and performance testing
#define KB_BUFFER_LEN    (182*132)

#define GF_DELMAR_FAKE_ERROR(err) ((err) == GF_ERROR_NOT_LIVE_FINGER)

#define DELMAR_TRACE(format, ...) \
    do { \
        char buf[1024] = { 0 }; \
        cpl_snprintf(buf, sizeof(buf), format, __VA_ARGS__); \
        GF_TRACE_STR_EXT(LOG_TAG, buf, 1); \
    } while (0)


#undef COMPILE_ASSERT
#define COMPILE_ASSERT(expr, msg) \
        typedef char msg[(expr) ? 1 : -1]

typedef gf_error_t (*user_data_checker)(uint8_t* data, uint32_t data_len);

#define DELMAR_SUB_ABS(a, b) ((((a) - (b)) < 0) ? ((b) - (a)) : ((a) - (b)))

typedef enum {
    GF_CHIP_DELMAR_T_SE1,
    GF_CHIP_DELMAR_CS_SE1,
    GF_CHIP_DELMAR_UNKNOWN
} gf_delmar_chip_type_t;

typedef enum {
    DELMAR_EXPOSURE_DEFAULT_SHORT = 0x00,
    DELMAR_EXPOSURE_DEFAULT_LONG
} gf_delmar_exposure_mode_t;

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
    DELMAR_CALI_STATE_LB_DIGIT_GAIN_READY_BIT,  // low brightness digit gain
    DELMAR_CALI_STATE_MAX = 32,
} gf_delmar_cali_state_bit_t;

typedef enum {
    GF_CMD_DELMAR_SENSOR_BASE = 100,
    GF_CMD_DELMAR_SENSOR_READ_IMAGE = 101,
    GF_CMD_DELMAR_SENSOR_NOTIFY_RESET = 102,
    GF_CMD_DELMAR_SENSOR_UPDATE_SLOTS = 103,
    GF_CMD_DELMAR_GET_CALI_LOAD_ERROR = 104,
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
    DELMAR_MORPHOTYPE_UNKOWN,
};

enum {
    DELMAR_OPTICAL_TYPE_2 = 2,    // optical type 2.0
    DELMAR_OPTICAL_TYPE_3,   // optical type 3.0
};

typedef struct {
    uint8_t exposure_mode;
    uint8_t continue_frame_count;
    uint8_t black_screen_capture;
    uint8_t brightness_level;
    uint32_t operation;
    uint32_t retry_count;
    uint32_t is_last_highlight;
} gf_delmar_capture_info_t;

typedef struct {
    int16_t agl;
    uint32_t px;
    uint32_t py;
    uint16_t amp;
    uint8_t sigma;
} __attribute__((packed)) gf_delmar_preprocess_moire_info_t;

typedef struct {
    gf_cmd_header_t header;
    gf_delmar_config_t o_delmar_config;
    gf_delmar_sensor_type_t o_sensor_type;
    uint32_t o_cali_state;
    uint32_t o_sensor_num;
    uint8_t o_morphotype;
    uint8_t o_cf_mark;
    uint8_t o_cf_mask_type;
    uint8_t o_optical_type;
    uint8_t io_sdcard_otp[MAX_SENSOR_NUM][DELMAR_OTP_INFO_BUFFER_LEN];
    uint32_t io_sdcard_otp_len[MAX_SENSOR_NUM];
    uint8_t o_priv_data[1];
    uint8_t i_device_unlocked;
    int8_t o_qr_code[MAX_QR_CODE_INFO_LEN];
} gf_delmar_sensor_init_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t o_cali_state;
    uint32_t preprocess_col;
    uint32_t preprocess_row;
} gf_delmar_algo_init_t;

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
    uint32_t exposure_mode;  // exposure mode: short or long
    uint32_t continue_frame_count;  // capture count
    uint32_t clear_cache_image;
    uint8_t brightness_level;
} gf_delmar_capture_image_t;

typedef struct {
    int32_t image_quality[MAX_SENSOR_NUM][MAX_PT_NUM];
    int32_t valid_area[MAX_SENSOR_NUM][MAX_PT_NUM];
    int32_t match_score[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t authenticate_study_flag[MAX_SENSOR_NUM][MAX_PT_NUM];
    int32_t key_point_num[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t overlay[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t broken_check_time[MAX_SENSOR_NUM];
    uint32_t preprocess_time[MAX_SENSOR_NUM];
    uint32_t get_feature_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    // dsp get feature time, valid excpet speed up auth
    uint32_t fast_auth_get_feature1to3_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t fast_auth_get_feature4_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t fast_auth_get_feature5_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t get_feature1_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t get_feature2_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t get_feature3_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t get_feature4_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t get_feature5_time[MAX_SENSOR_NUM][MAX_PT_NUM];

    uint32_t enroll_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t authenticate_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t speed_up_authenticate_time[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t total_time[MAX_SENSOR_NUM];
    uint32_t study_flag[MAX_SENSOR_NUM][MAX_PT_NUM];
    int32_t update[MAX_SENSOR_NUM];
    uint32_t weak_update[MAX_SENSOR_NUM];
    uint32_t increase_rate[MAX_SENSOR_NUM][MAX_PT_NUM];
    uint32_t read_image_time[MAX_SENSOR_NUM];
    uint32_t detect_fake_time;
    uint32_t capture_image_time;
    uint32_t get_raw_data_time;
    uint64_t valid_sensor_ids;
    uint32_t works_image_counts;
    uint32_t algo_order[MAX_SENSOR_NUM];
} gf_delmar_algo_performance_t;

typedef struct {
    gf_cmd_header_t header;
    uint64_t i_sensor_ids;
    uint8_t i_use_full_mask;
    uint8_t i_ui_type;
    uint8_t i_has_press_area_info;
    gf_delmar_coordinate_info_t i_coordinate_info;
    gf_delmar_press_area_info_t i_press_area_info;
} gf_delmar_read_image_t;

typedef struct {
    gf_cmd_header_t header;
    gf_delmar_algo_performance_t o_performance;
} gf_delmar_algo_kpi_t;

typedef struct {
    gf_algo_enroll_image_t common;
    uint64_t i_enroll_sensor_ids;
    uint8_t i_is_first_sensor;
    uint8_t i_is_first_part;
    uint8_t i_retry_count;
    uint8_t i_overlay_check;
    uint32_t i_dsp_get_feature2_time;
    uint32_t i_get_feature4_time;
    uint8_t o_is_templates_full;
    int32_t o_error[MAX_SENSOR_NUM][MAX_PT_NUM];
#ifdef SUPPORT_DUMP_DSP_GET_FEATURE
    uint8_t o_getfeature_ret[2 * GET_FEATURE_BUFFER_SIZE];
#else  // SUPPORT_DUMP_DSP_GET_FEATURE
    uint8_t* o_getfeature_ret;  // should not be used
#endif  // SUPPORT_DUMP_DSP_GET_FEATURE
} gf_delmar_algo_enroll_image_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_gid;
    uint32_t i_finger_id;
    uint16_t i_samples_remaining;
} gf_delmar_finish_enroll_t;

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
} gf_delmar_algo_auth_image_t;

typedef enum {
    GF_DELMAR_CMD_UPDATE_SENSOR_IDS = GF_CMD_ALGO_MAX + 1,
    GF_DELMAR_CMD_ALGO_AUTHENTICATE_END,
    GF_DELMAR_CMD_ALGO_FINISH_ENROLL,
    GF_DELMAR_CMD_ALGO_AUTHENTICATE_HVX_SKIP_TEMPLATE,
    GF_DELMAR_CMD_AUTH_POST_AUTHENTICATE,
    GF_DELMAR_CMD_AUTH_FAST_AUTHENTICATE,
    GF_DELMAR_CMD_ALGO_MAX
} gf_delmar_algo_cmd_id_t;

enum {
    DELMAR_CMD_AUTH_POST_AUTHENTICATE_STAGE_ONE = 1,
    DELMAR_CMD_AUTH_POST_AUTHENTICATE_STAGE_TWO,
    DELMAR_CMD_AUTH_POST_AUTHENTICATE_STAGE_END
};

typedef struct {
    gf_auth_post_auth_t common;
    uint8_t i_stage;
    uint8_t i_auth_success_sensor_id;
    uint8_t i_study_sensor_id;
    uint8_t i_reauthenticate_for_study;
} gf_delmar_auth_post_auth_t;

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
    GF_DELMAR_OPERATION_AGE_TEST,
    GF_DELMAR_OPERATION_FLESH_MAX,
    GF_DELMAR_OPERATION_TEST_IMAGE_QUALITY,
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
    gf_cmd_header_t header;
    uint64_t i_sensor_ids;
} gf_delmar_calc_sensor_mem_slots;

#endif /* _GF_DELMAR_TYPES_H_ */
