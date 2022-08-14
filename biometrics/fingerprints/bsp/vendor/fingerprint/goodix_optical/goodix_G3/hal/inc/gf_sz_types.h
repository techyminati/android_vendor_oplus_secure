/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_SZ_TYPES_H_
#define _GF_SZ_TYPES_H_

#include "gf_sz_config.h"
#include "gf_sensor_types.h"
#include "gf_algo_types.h"
#include "gf_config_type.h"
#include "gf_sz_common.h"

#define MAX_CONTINUE_FRAME_NUM   (1)
#define ALGO_VERSION_INFO_LEN    (64)
#define PRODUCTION_ALGO_VERSION_INFO_LEN    (64)
#define GF_HW_AUTH_TOKEN_LEN     80

#define HAL_VERSION_INFO_LEN     64
#define SOFTWARE_VERSION_LEN     64
#define GF_CHIP_ID_LEN           16
#define GF_VENDOR_ID_LEN         20
#define GF_SENSOR_ID_LEN         16
#define GF_PMIC_ID_LEN           16
#define GF_FLASH_ID_LEN          16
#define GF_MCU_ID_LEN            16
#define GF_AD_VERSION_LEN        16
#define GF_LENS_TYPE_LEN         16
#define GF_SENSOR_UID_LEN        13


#define SHENZHEN_CHIP_ID          0x1302
#define ORIGIN_RAW_DATA_LEN       71680  // 224*(176*2+4)  90000

#define ORIGIN_BUFFER_LEN        (90000)        // 90000

#define MAX_CHIP_RAWDATA_LEN     (SHENZHEN_SE3_SENSOR_ROW * SHENZHEN_SE3_SENSOR_COL)

#define CHIP_ID_BUFFER_LEN       (32)  // 16
#define VENDOR_ID_BUFFER_LEN     (32)  // 16
#define SENSOR_ID_BUFFER_LEN     (32)  // 16
#define OTP_INFO_BUFFER_LEN      (256)  // 192
#define EFUSE_INFO_BUFFER_LEN      (256)  // 192

#define SHENZHEN_SE2_SENSOR_ROW   (224)
#define SHENZHEN_SE2_SENSOR_COL   (176)
#define SHENZHEN_SE3_SENSOR_ROW   (224)
#define SHENZHEN_SE3_SENSOR_COL   (192)
#define GF_CHIP_RAWDATA_LEN      (SHENZHEN_SE3_SENSOR_ROW * SHENZHEN_SE3_SENSOR_COL)
#define GF_SHENZHEN_CHIP_DFT_WIDTH    (256)
#define GF_SHENZHEN_CHIP_DFT_HEIGHT   (256)
#define IMAGE_BUFFER_LEN         GF_CHIP_RAWDATA_LEN        // 90000

// macro definition bigdata info relatively
#define MILAN_CONFIG_CONTENT_MAX_LEN 524
#define SENSOR_OTP_VERSION_LEN 16
#define SENSOR_OTP_CHIPID_LEN  16

#define GF_SENSOR_VDELAY_VALUE  (1)
#define GF_SE_K_MIN_H_BLANK_VALUE   (3000)
#define GF_SE_G_MIN_H_BLANK_VALUE   (3500)

#define DUMP_TEMPLATE_BUFFER_LEN 409600

#define WAFER_LOT_ID             (8)
#define OTP_RANDOM_NO    (2)
#define PRODUCT_ID    (2)
#define PROGRAM_VERSION    (2)
#define TEST_TIME    (4)
#define MODULE_VERSION_LEN    (20)
#define MAX_CFG_FILE_LEN (1024)
#define MAX_FW_FILE_LEN  (56*1024)
#define RAWDATA_EACH_PACKAGE_SIZE    (1024 * 7)

#define LENS_TYPE_1P_HEM        0x00
#define LENS_TYPE_1P_NM         0x01
#define LENS_TYPE_2P_SY         0x02
#define LENS_TYPE_2P_NM         0x03
#define LENS_TYPE_2P_OF         0x04
#define LENS_TYPE_3P_UM         0x05
#define LENS_TYPE_3P_NM         0x06
#define LENS_TYPE_3P_OF         0x07
#define LENS_TYPE_G24_NM        0x08
#define LENS_TYPE_G24_OF        0x09
#define LENS_TYPE_G24_UN        0x0A
#define LENS_TYPE_G24_YJ        0x0B
#define LENS_TYPE_MICROLENS     0x80

#define CHIP_TYPE_G2       0
#define CHIP_TYPE_G2_1     1
#define CHIP_TYPE_G2_4     2
#define CHIP_TYPE_G2_4_PLUS     3
#define CHIP_TYPE_G2_5     4
#define CHIP_TYPE_G3      5
#define CHIP_TYPE_G3S      6

#define GF_SZ_DUMP_CALIBRATE_TEST_DIR "/gf_data/production_test/"
#define GF_SZ_FT_TIME 128
#define GF_SZ_USER_ENV 256

#define RAW_FEATUREST_LEN    (500 * 1024)
#define PCALI_SIG_LEN         (SHENZHEN_SE3_SENSOR_COL * SHENZHEN_SE3_SENSOR_ROW / 2)
#define USBASE_NM    (5)
#define HL_USBASE_NM    (2)

#define DUMP_HVX_SHARE_BUFFER_LEN     (205 * 1024 + 1 * 1024 * 1024)  // Close to 1.2M , Slightly larger than it.

#define FAKE_PARA_LEN     (64)
#define FAKE_RECOG_LEN     (60 * 3 * 4)
#define FAKE_CF_FEATURE_LEN    (9 * 7 * 3)
#define GF_SZ_EXTRA_SIZE 40000
#define SETTING_HBM_EXPO 1
#define SETTING_400_NIT_EXPO 2
#define SETTING_300_NIT_EXPO 3
#define SETTING_200_NIT_EXPO 4

#define SOFT_SCREEN_TYPE 1
#define HARD_SCREEN_TYPE 2

#define SCREEN_TYPE_ID_SDC 0x1
#define SCREEN_TYPE_ID_BOE 0x2
#define SCREEN_TYPE_ID_TM 0x3
#define SCREEN_TYPE_ID_LTPO 0x4

#define GF_TEMP_FAKE_PARA_POLAR_INDEX          (21)
#define GF_TEMP_FAKE_PARA_DL_INDEX          (41)
#define GF_TEMP_FAKE_PARA_CF_INDEX          (24)
#define GF_TEMP_FAKE_PARA_LIVE_INDEX        (25)

#define GF_FAKE_LOG_INFO_LEN (96)
#define GF_TEMP_LOG_INFO_LEN (20)

typedef enum
{
    GF_CHIP_SZ_SE2_K,
    GF_CHIP_SZ_SE2_ECO_K,
    GF_CHIP_SZ_SE3_K,
    GF_CHIP_SZ_SE3_G,
    GF_CHIP_SZ_SE3_ECO_K,
    GF_CHIP_SZ_SE3_ECO_G,
    GF_CHIP_SZ_TC2409,  //  G2.4.1
    GF_CHIP_SZ_TC2408,  //  G2.5
    GF_CHIP_SZ_G3,
    GF_CHIP_SZ_G3S,
    GF_CHIP_MAX,
} gf_sz_chip_type_t;

typedef enum
{
    IMG_DATA_PACK_MODE_1 = 0x01,
    IMG_DATA_PACK_MODE_2 = 0x02,
    IMG_DATA_PACK_MODE_3 = 0x03,
} gf_data_pack_mode_t;

typedef struct
{
    gf_sz_chip_type_t chip_type;
    uint16_t sys_version;
    uint16_t ad_version;
    uint8_t adc_bits;
    uint32_t col;
    uint32_t row;
    uint16_t dark_start_col;
    uint16_t dark_len;
    // dark info
    uint16_t dark_left;
    uint16_t dark_right;
    uint16_t dark_up;
    uint16_t dark_down;
    uint8_t reserve[32];
} gf_sensor_config_t;

typedef struct
{
    uint8_t info_0;
    uint8_t info_1;
    uint16_t exposure_time;
    uint8_t pga_gain;
    uint8_t rpg_gain;
    uint8_t force_param;
    uint8_t test_stage;
    uint8_t lens_type;
    uint8_t center_point_x;  // used by bad-point test
    uint8_t center_point_y;
    uint8_t chip_version;
    uint8_t flash_vendor_id;
    uint8_t module_QR_code[30];
    uint8_t filter_type_mask;
    uint8_t module_struct_type;
    uint8_t reserved[5];
}
__attribute__((__packed__))gf_sensor_mt_info_t;

typedef struct
{
    uint8_t info_0;
    uint8_t info_1;
    uint8_t force_param;
    uint8_t test_stage;
    uint8_t module_category;
    uint16_t center_contrast;
    uint16_t boarder_contrast;
    uint8_t reserved[5];
}
__attribute__((__packed__))gf_sensor_mt2_info_t;

typedef struct
{
    uint8_t info_0;
    uint8_t info_1;
    uint16_t exposure_time;
    uint8_t pga_gain;
    uint8_t rpg_gain;
    uint8_t force_param;
    uint8_t test_stage;
    uint8_t fov_left;
    uint8_t fov_right;
    uint8_t fov_up;
    uint8_t fov_down;
    uint8_t luminance_x;
    uint8_t luminance_y;
    // add for new flash plan
    uint16_t exposure_value;  //0:stand for 2400, others: real value
    uint16_t signal_value;  // default: 0

    uint16_t chart_signal_center;
    uint16_t chart_signal_topleft;
    uint16_t chart_signal_topright;
    uint16_t chart_signal_bottomleft;
    uint16_t chart_signal_bottomright;  // default:0

    uint16_t ri_topleft;
    uint16_t ri_topright;
    uint16_t ri_bottomleft;
    uint16_t ri_bottomright;  // default 0, multiple 1024 when write

    uint16_t screenlight_leak;  // default 0, multiple 1024 when write
    uint16_t flesh_touch_diff;  // default 0
    uint16_t scale;  // default 0, multiple 1024 when write

    uint16_t exposure_400_nit_time;
    uint16_t exposure_300_nit_time;
    uint16_t exposure_200_nit_time;

    uint8_t reserved[2];
    float cali_temp;
    float tsnr;  // use 4 bytes
    float acture_gain;
    float stripeT;
    uint8_t reserved2[20];  // total 32 bytes
}
__attribute__((__packed__))gf_sensor_spmt_info_t;

typedef struct
{
    gf_sz_chip_type_t chip_type;
    const char *str;
} gf_chip_type_str_t;

typedef struct
{
    uint32_t row;
    uint32_t col;
    uint32_t raw_data_len;

    uint8_t sensor_uid[GF_SENSOR_ID_LEN];
    uint32_t sensor_id;
    uint32_t sensor_version;

    uint32_t enroll_min_templates;
    uint32_t max_templates;

    int32_t flag_chip_info;
    int32_t thr_select_bmp;

    // createFusionMap config
    uint32_t Lowerthresh;
    uint32_t Highthresh;

    // exposure config
    uint8_t long_frame_avg_num;
    uint16_t long_pga_gain;
    uint16_t long_exposure_time;
    uint8_t short_frame_avg_num;
    uint16_t short_pga_gain;
    float acture_gain;
    uint16_t rpg_gain;
    uint16_t short_exposure_time_and_fov_change_flag;
    uint16_t short_exposure_time;
    uint16_t short_exposure_temporary_time;
    uint16_t short_exposure_hbm_time;
    uint16_t short_exposure_400_nit_time;
    uint16_t short_exposure_300_nit_time;
    uint16_t short_exposure_200_nit_time;
    uint16_t gain;
    uint16_t ae_min_time;
    uint16_t ae_max_time;
    uint16_t ae_thres_time;
    uint16_t ae_expo_start_time;
    uint32_t row_time;
    uint32_t frame_time;

    // expo time info
    uint32_t hblank;
    uint32_t hvalid;
    uint32_t vdelay;
    uint32_t vblank;
    uint32_t vvalid;

    // rect info
    uint8_t fov_left;
    uint8_t fov_right;
    uint8_t fov_up;
    uint8_t fov_down;

    uint8_t luminance_x;
    uint8_t luminance_y;

    // fusion config 1.0~8.0
    float ratio;
    // algorKey ex.0x07f7
    uint32_t g_preprocess_key;
    // Rect crop bmp
    uint32_t preprocess_crop_x0;
    uint32_t preprocess_crop_y0;
    uint32_t preprocess_row;
    uint32_t preprocess_col;
    // basecalidate switch
    uint32_t base_calibrate_switch;
    // ldc switch
    uint32_t ldc_switch;
    uint32_t tnr_switch;
    uint32_t tnr_th;
    uint32_t tnr_reset_flag;
    uint32_t enhance_level;
    uint32_t lpf_switch;
    uint32_t lsc_switch;
    uint32_t isUseLocalFilter;
    uint32_t nHoleRatio;
    uint32_t nLocalStren;
    uint32_t nMeanRaw;
    uint32_t nSelectBaseID;
    uint32_t nLQuanStren;
    uint32_t bmp_col;
    uint32_t bmp_row;
    uint32_t nScaleRatio;
    uint32_t nResizeM;
    uint32_t nResizeN;
    uint32_t nDilateErrod;
    int8_t support_continuous_sampling;
    uint8_t continuous_sampling_number;
    uint32_t dark_start_col;
    uint32_t dark_len;
    uint32_t expo_value;
    uint32_t adc_bits;
    uint8_t lens_type;
    uint8_t ulens_value;

    gf_data_pack_mode_t pack_mode;
    uint32_t original_data_size;
    uint32_t factory_type;
    gf_sz_chip_type_t chip_type;
    uint8_t mask_type;
    uint8_t mask;
    uint8_t center_point_x;  // used by bad-point test
    uint8_t center_point_y;

    // crop info
    uint32_t crop_x;
    uint32_t crop_y;
    uint32_t crop_width;
    uint32_t crop_height;
    uint32_t crop_row;
    uint32_t crop_col;

    // dark info
    uint8_t dark_left;
    uint8_t dark_right;
    uint8_t dark_up;
    uint8_t dark_down;

    // crop info backup
    uint32_t crop_x_backup;
    uint32_t crop_y_backup;
    uint32_t crop_width_backup;
    uint32_t crop_height_backup;
    uint32_t crop_row_backup;
    uint32_t crop_col_backup;

    // flash vensor id
    uint8_t flash_vendor_id;
    uint8_t module_struct_type;
    uint32_t nValidRawWidth;

    // chip type record in otp
    uint8_t chip_type_otp;
    // add for new flash plan
    uint16_t exposure_value;  //0:stand for 2400, others: real value
    uint16_t signal_value;  // default: 0

    uint16_t chart_signal_center;
    uint16_t chart_signal_topleft;
    uint16_t chart_signal_topright;
    uint16_t chart_signal_bottomleft;
    uint16_t chart_signal_bottomright;  // default:0

    uint16_t ri_topleft;
    uint16_t ri_topright;
    uint16_t ri_bottomleft;
    uint16_t ri_bottomright;  // default 0, multiple 1024 when write

    uint16_t screenlight_leak;  // default 0, multiple 1024 when write
    uint16_t flesh_touch_diff;  // default 0
    uint16_t scale;  // default 0, multiple 1024 when write
    uint16_t flash_expo_time;

    // mt2 info
    uint16_t center_contrast;
    uint16_t boarder_contrast;

#ifdef __TRUSTONIC
    uint16_t hardware_type;
#endif // __TRUSTONIC

    uint32_t factory_cf_mask_type;
    // thermal meter para
    float thermal_meter_para_c;
    float thermal_meter_para_d;
    float thermal_meter_para_e;
    float thermal_meter_para_f;
    float cali_temp;

    // ae
    uint16_t ae_expo_light_one_time;
    uint16_t ae_expo_light_two_time;
    uint16_t ae_expo_light_three_time;
    uint16_t ae_expo_min_value;
    uint16_t ae_expo_max_value;

    uint8_t screen_id;  // use 1 reserve byte
    uint8_t dummy1;  // for reserve
    uint8_t dummy2;  // for reserve
    uint8_t dummy3;  // for reserve
    int32_t current_temp;  // use 4 reserve bytes
    int32_t stime;         // use 4 reserve bytes
    float tsnr;            // use 4 reserve bytes
    float flash_gain;      // use 4 reserve bytes
    int32_t stripeT;
    uint8_t reserve[8];  // total 32 bytes
} gf_sz_sensor_data_t;

typedef struct
{
    int32_t nCropX;
    int32_t nCropY;
    int32_t nCropWidth;
    int32_t nCropHeight;
    int32_t nIsReCalibrate;
    int32_t nRetry;
    int32_t nHoleWidth;
    int32_t nHoleHeight;
    int32_t nInWidth;
    int32_t nInHeight;
    int32_t nSimpleCaliMode;
    int32_t nIsCropFOV;
    int32_t nMeanBase;
    int32_t nScaleRatio;
    int32_t nUseReadMask;
    int32_t nDilateErrod;
    int32_t nMeanRaw;
    int32_t nSelectBaseID;
    int32_t nAlgKey;
    int32_t nResizeM;
    int32_t nResizeN;
    int32_t nRawFrmNum;
    int32_t nAddRawNum;
    int32_t nDarkStart;
    int32_t nDarkLen;
    int32_t nBitType;
    int32_t nKBVersion;
    int32_t nResidualFingerEnable;
    int32_t nBrokenDetEnable;
    int32_t uchFlipParam;
    int32_t nDarkSize;
    int32_t nDataUpdateKB;
    int32_t nCenterX;
    int32_t nCenterY;
    uint8_t reserve[32];
} gf_sz_preprocess_params_t;

typedef struct
{
    uint16_t ct;

    uint16_t ev;
    uint16_t sva;
    uint16_t svc;
    uint16_t svtl;
    uint16_t svtr;
    uint16_t svbl;
    uint16_t svbr;
    uint16_t ritl;
    uint16_t ritr;
    uint16_t ribl;
    uint16_t ribr;
    uint16_t sll;
    uint16_t td;
    uint16_t sc;

    uint16_t st;
    uint16_t lc;
    uint16_t illu;
    uint16_t bm;
} gf_sz_mp_params_t;


typedef struct
{
    gf_cmd_header_t header;
    gf_sensor_ids_t o_sensor_ids;
    uint8_t o_flash_vendor_id;
    uint8_t o_module_type;
    uint8_t o_lens_type;
    gf_sz_config_t o_shenzhen_config;
    uint8_t screen_id;  // use 1 reserve byte
    uint8_t reserve1;  // reserve 1 byte
    uint16_t gainvalue;  // use 2 bytes
    uint16_t expotime;  // use 2 bytes
    uint16_t reserve2;  // reserve 2 bytes
    uint8_t reserve[24];  // total 32 bytes
} gf_sz_sensor_init_t;

typedef struct
{
    gf_sensor_set_config_t header;
    gf_sz_config_t i_config;
} gf_sz_set_config_cmd_t;

typedef struct
{
    gf_sensor_info_t sensor_info;
    gf_sensor_ids_t sensor_ids;
} gf_sz_sensor_info_t;

typedef struct
{
    gf_cmd_header_t header;
    uint32_t preprocess_col;
    uint32_t preprocess_row;
    uint32_t spmt_pass;
    uint8_t android_version[ANDROID_VERSION_LEN];
    uint8_t service_version[SERVICE_VERSION_LEN];
    uint8_t platform[PLATFORM_LEN];
    uint8_t algo_version[ALGO_VERSION_INFO_LEN];
    uint8_t fake_version[ALGO_VERSION_INFO_LEN];
    uint8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    uint8_t factory_algo_version[ALGO_VERSION_INFO_LEN];
    uint8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    uint8_t reserve[32];
} gf_sz_algo_init_t;

typedef struct
{
    gf_sz_config_t config_info;
} gf_sz_config_info_t;

typedef struct
{
    uint8_t wafer_lot_id[WAFER_LOT_ID];
    uint8_t wafer_no;
    uint8_t x_coordinate;
    uint8_t y_coordinate;
    uint8_t random_no[OTP_RANDOM_NO];
    uint8_t trim_param_osc80m;
    uint8_t trim_param_wdt32k;
    uint8_t vbg_trim;
    uint8_t room_temp;
    uint8_t reserve_1[5];
    uint8_t cp_check_sum;
    uint8_t high_temp;
    uint8_t reserve_2[5];
    uint8_t cp_ht_check_sum;
    uint8_t flag;
    uint8_t product_id[PRODUCT_ID];
    uint8_t lens_type;
    uint8_t substract_type;
    uint8_t top_or_open;
    uint8_t program_version[PROGRAM_VERSION];
    uint8_t test_time[TEST_TIME];
    uint8_t screen_version;
    uint8_t reserve_3[20];
    uint8_t ft_check_sum;
    uint8_t reserve_4[32];
} gf_sz_sensor_otp_info_t;

typedef struct
{
    uint8_t wafer_lot_id[WAFER_LOT_ID];
    uint8_t wafer_no;
    uint8_t x_coordinate;
    uint8_t y_coordinate;
    uint8_t random_no[OTP_RANDOM_NO];
    uint8_t trim_param_osc10m;
    uint8_t trim_param_wdt32k;
    uint8_t vbg_trim;
    uint8_t dvdd18_trim;
    uint8_t white_led_dac;
    uint8_t ir_led_dac;
    uint8_t cp_check_sum;
} gf_sz_pmic_otp_info_t;

typedef struct
{
    uint64_t timestamp_keydown;
    uint64_t timestamp_irq;
    uint64_t timestamp_notify;
    uint32_t screen_status;
    uint32_t result;
    uint32_t tryCount;
    uint32_t is_final;
    uint8_t reserve[32];
} gf_sz_dump_hal_data_t;

typedef struct
{
    // todo
} gf_sz_template_info_t;


typedef enum
{
    DUMP_SZ_RESET = DUMP_OP_MAX + 1,
    DUMP_SZ_CAPTURE,
    DUMP_SZ_TEMPLATE,
    DUMP_SZ_FACTORY_IMAGE,
    DUMP_SZ_AUTO_EXPOSURE,
    DUMP_SZ_CALIBRATE,
    DUMP_SZ_PERFORMANCE,
    DUMP_SZ_MAX
} gf_sz_dump_t;

typedef enum
{
    GF_SZ_CMD_STOP_CAPTURE_IMAGE = GF_CMD_SENSOR_MAX + 1,
    GF_SZ_CMD_SLEEP,
    GF_SZ_CMD_RESET,
    GF_SZ_CMD_SET_EXPO,
    GF_SZ_CMD_SENSOR_MAX
} gf_sz_sensor_cmd_id_t;

typedef enum
{
    GF_SZ_CMD_FORCE_STUDY = GF_CMD_ALGO_MAX + 1,
    GF_SZ_CMD_SET_ENVIRONMENT_LEVEL,
    GF_CMD_ALGO_DATA_INIT,
    GF_CMD_AUTH_CUSTOMENC,
    GF_CMD_GET_DUMP_LEVEL,
    GF_SZ_CMD_ALGO_MAX
} gf_sz_algo_cmd_id_t;

typedef enum
{
    GF_SZ_OPERATION = GF_OPERATION_MAX + 1,
    GF_SZ_OPERATION_FACTORY,
    GF_SZ_OPERATION_FACTORY_LOCAL_AREA_SAMPLE,
    GF_SZ_OPERATION_MAX
} gf_sz_operation_t;

typedef enum {
    GF_FINGERPRINT_DISMATCH_FAKE_FINGER = 1,
    GF_FINGERPRINT_DISMATCH_DRY_FINGER,
    GF_FINGERPRINT_DISMATCH_IMAGE_PARTIAL,
    GF_FINGERPRINT_DISMATCH_IMAGE_DIRTY,
    GF_FINGERPRINT_DISMATCH_WET_FINGER,
    GF_FINGERPRINT_DISMATCH_ANTI_SPOOF,
    GF_FINGERPRINT_DISMATCH_HIGH_LIGHT,
    GF_FINGERPRINT_DISMATCH_UNMATCH_FAKE_FINGER,
    GF_FINGERPRINT_DISMATCH_IMAGE_QUALITY,
    GF_FINGERPRINT_DISMATCH_IMAGE_MEANRAW,
    GF_FINGERPRINT_DISMATCH_FLOC,
    GF_FINGERPRINT_DISMATCH_RESIDUAL_FINGER,
    GF_FINGERPRINT_DISMATCH_COVER_BROKEN,
    GF_FINGERPRINT_DISMATCH_RESIDUAL_FINGER_NEW,
    GF_FINGERPRINT_DISMATCH_SIMILARITY,
    GF_FINGERPRINT_DISMATCH_FEATURE_POINT,
    GF_FINGERPRINT_DISMATCH_OVERLAPPING_AREA,
    GF_FINGERPRINT_DISMATCH_OTHER,
    GF_FINGERPRINT_DISMATCH_RESIDUAL_FINGER_NEW_TWO
} gf_sz_fingerprint_dismatch_info_t;

#endif /* _GF_SZ_TYPES_H_ */
