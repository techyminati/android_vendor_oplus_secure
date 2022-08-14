/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_COMMON_H_
#define _GF_COMMON_H_

#include <stdint.h>
#include "gf_type_define.h"
#include "gf_error.h"

#ifdef __cplusplus
extern "C"
{
#endif  // end ifdef __cplusplus

#define ALGO_VERSION_INFO_LEN    64
#define FW_VERSION_INFO_LEN      64
#define TEE_VERSION_INFO_LEN     72
#define TA_VERSION_INFO_LEN      64
#define PRODUCTION_DATE_LEN      32
#define GF_CHIP_ID_LEN           16
#define GF_VENDOR_ID_LEN         16
#define GF_SENSOR_ID_LEN         16

// milan an: 192,milan f/hv: 32,milan hu(GF5228): 64
#define GF_SENSOR_OTP_BUFFER_LEN   192


#define GF_HEART_BEAT_ALGO_VERSION_LEN 64
#define GF_MAX_SPI_RW_LEN        24
#define MAX_CALLBACK_RESULT_LEN  1024
// (milanBN) and Calibration data frames
#define BN_IMAGE_BUFFER_LEN      7680
#define DUMMY_PIXEL_LEN          120
#define CALIBRATION_MAX_FRAMES   17
#define CALIBRATION_MAX_INDEX    100
#define CALIBRATION_MAX_COUNT    5
#define CALIBRATION_PARAMS_LEN   238
#define CALIBRATION_PARAMS_EXCEPT_VCM_DAC_LEN 24
#define CALIBRATION_PARAMS_VCM_DAC_LEN 214

// MAX(oswego_m: 13824 + 22, milan_f_series: 22176 + 4, milan_a_series: 30000)
#define ORIGIN_RAW_DATA_LEN      30000

// MAX(96 * 96, 68 * 118, 88 * 108, 54 * 176, 64 * 176, 132 * 112, 64 * 120)
#define IMAGE_BUFFER_LEN         14784

// MAX(MILAN F:88 * 108, OSWEGO M:15 * 96 * 3)
#define NAV_BUFFER_LEN           9504
#define NAV_MAX_FRAMES           50
#define NAV_MAX_BUFFER_LEN       (9504*20)

#define FPC_KEY_DATA_LEN         4

#define DUMP_TEMPLATE_BUFFER_LEN 983040      // (960 * 1024)//for 160 templates

#define GSC_UNTOUCH_LEN          10
#define GSC_TOUCH_LEN            5
#define GF_LIVE_DATA_LEN         18
#define HBD_DISPLAY_DATA_LEN     5
#define HBD_RAW_DATA_LEN         12
#define HBD_BUFFER_LEN           24
#define GSC_RAW_DATA_BUFFER_LEN  24

/* fw and cfg binary length*/
// MAX(oswego_m:43022=(4+8+2+(32+4+2+4)*1024), milan_a_series:5120)
#define GF_FW_LEN                43022
// MAX(oswego_m:249, milan_a_series:256)
#define GF_CFG_LEN               418

// below is defined for fido, UVT means User Verify Token.
#define MAX_AAID_LEN             32
#define MAX_FINAL_CHALLENGE_LEN  32
#define MAX_UVT_LEN              512      // the size which fido suggest is 8k

/*operation array length is equal priority classes*/
#define MAX_OPERATION_ARRAY_SIZE (10)

#define PRIORITY_UNDEFINE        (10)
#define PRIORITY_TEST            (0)
#define PRIORITY_AUTHENTICATE    (1)
#define PRIORITY_PRIOR_TEST      (2)

#define MAX_RESCAN_TIME   (3)

#define MAX_CONTINUE_FRAME_NUM   (3)
#define MAX_BAD_POINT_TEST_FRAME_NUM   (5)
#define CRC32_SIZE               (4)

#define BROKEN_CHECK_MAX_FRAME_NUM  3
#define MAX_FINGER_PATH_NAME_SIZE        (128)

#define SWIPE_ENROLL_DUMP_RAW_NUM      (10)
#define SWIPE_ENROLL_DUMP_CALI_NUM     (2)

#define GF_TOKEN_LEN                                            (sizeof(uint8_t))

#define GF_CRC32_TOKEN                                          0xF0
#define GF_CRC32_CONTENT_LEN                                    (sizeof(uint32_t))
#define GF_CRC32_LEN                                            \
    (GF_TOKEN_LEN + GF_CRC32_CONTENT_LEN)

// macro definition bigdata info relatively
#define GF_BIGDATA_ANDROID_VERSION_LEN    20
#define GF_BIGDATA_SERVICE_VERSION_LEN    20
#define GF_BIGDATA_PLATFORM_LEN           20
#define GF_BIGDATA_ALGO_VERSION_LEN       ALGO_VERSION_INFO_LEN
#define GF_BIGDATA_PREPROCESS_VERSION_LEN ALGO_VERSION_INFO_LEN
#define GF_BIGDATA_FW_VERSION_LEN         FW_VERSION_INFO_LEN
#define GF_BIGDATA_TEE_VERSION_LEN        TEE_VERSION_INFO_LEN
#define GF_BIGDATA_TA_VERSION_LEN         TA_VERSION_INFO_LEN
#define GF_BIGDATA_CONFIG_VERSION_LEN     20
#define GF_BIGDATA_CHIP_ID_LEN            GF_CHIP_ID_LEN
#define GF_BIGDATA_VENDOR_ID_LEN          GF_VENDOR_ID_LEN
#define GF_BIGDATA_SENSOR_ID_LEN          GF_SENSOR_ID_LEN
#define GF_BIGDATA_SENSOR_OTP_VERSION_LEN 16
#define GF_BIGDATA_SENSOR_OTP_CHIPID_LEN  16
#define GF_BIGDATA_SENSOR_OTP_INFO_LEN    GF_SENSOR_OTP_BUFFER_LEN
#define GF_BIGDATA_PRODUCTION_DATE_LEN    PRODUCTION_DATE_LEN
#define GF_BIGDATA_MODULE_VERSION_LEN     GF_CHIP_ID_LEN
#define GF_BIGDATA_GSC_UNTOUCH_LEN        GSC_UNTOUCH_LEN
#define GF_BIGDATA_GSC_TOUCH_LEN          GSC_TOUCH_LEN
#define GF_BIGDATA_LIVE_DATA_LEN          GF_LIVE_DATA_LEN
#define GF_BIGDATA_HBD_DISPLAY_DATA_LEN   HBD_DISPLAY_DATA_LEN
#define GF_BIGDATA_HBD_RAW_DATA_LEN       HBD_RAW_DATA_LEN
#define GF_BIGDATA_HBD_BUFFER_LEN         HBD_BUFFER_LEN
#define GF_BIGDATA_GSC_RAW_DATA_BUFFER_LEN GSC_RAW_DATA_BUFFER_LEN
#define GF_BIGDATA_HBD_ALGO_VERSION_LEN   GF_HEART_BEAT_ALGO_VERSION_LEN
#define GF_BIGDATA_FW_DATA_LEN            GF_FW_LEN
#define GF_BIGDATA_CFG_DATA_LEN           GF_CFG_LEN
#define GF_BIGDATA_FPC_KEY_DATA_LEN       FPC_KEY_DATA_LEN

#define FULL_TEMPERATURE_CHECK (1)  // full check temperature
#define FAST_TEMPERATURE_CHECK (0)  // fast check temperature
#define CHICAGO_B_OFFSET (7095)

#define MAX_OTHER_RESERVE_LEN    (500 * 1024)

typedef enum
{
    MODE_IMAGE = 0,
    MODE_KEY,
    MODE_SLEEP,
    MODE_FF,
    MODE_NAV,
    MODE_NAV_BASE,
    MODE_DEBUG,
    MODE_FINGER_BASE,
    MODE_IDLE,
    MODE_HBD,
    MODE_HBD_DEBUG,
    MODE_IMAGE_CONTINUE,
    MODE_TEST_BAD_POINT,
    MODE_TEST_PIXEL_OPEN_DEFAULT,
    MODE_TEST_PIXEL_OPEN_POSITIVE,
    MODE_TEST_BAD_POINT_FINGER_BASE,
    MODE_TEST_CALIBRATION_PARAM_BASE,
    MODE_MAX,
    MODE_NONE = 1000,
} gf_mode_t;

typedef enum
{
    TEST_SNR_BASE_DATA = 0,
    TEST_SNR_RAW_DATE,
} gf_snr_type_t;

typedef enum
{
    GF_CMD_DETECT_SENSOR = 1000,
    GF_CMD_INIT,
    GF_CMD_EXIT,
    GF_CMD_DOWNLOAD_FW,
    GF_CMD_DOWNLOAD_CFG,
    GF_CMD_INIT_FINISHED,
    GF_CMD_PRE_ENROLL,
    GF_CMD_ENROLL,
    GF_CMD_POST_ENROLL,
    GF_CMD_CANCEL,
    GF_CMD_AUTHENTICATE,
    GF_CMD_GET_AUTH_ID,
    GF_CMD_SAVE,
    GF_CMD_REMOVE,
    GF_CMD_SET_ACTIVE_GROUP,
    GF_CMD_ENUMERATE,
    GF_CMD_IRQ,
    GF_CMD_SCREEN_ON,
    GF_CMD_SCREEN_OFF,
    GF_CMD_ESD_CHECK,
    GF_CMD_SET_SAFE_CLASS,
    GF_CMD_CAMERA_CAPTURE,
    GF_CMD_ENABLE_FINGERPRINT_MODULE,
    GF_CMD_ENABLE_FF_FEATURE,
    GF_CMD_TEST_BAD_POINT,
    GF_CMD_TEST_SENSOR_FINE,
    GF_CMD_TEST_SENSOR_FINE_FINISH,
    GF_CMD_TEST_PIXEL_OPEN,
    GF_CMD_TEST_PIXEL_OPEN_STEP1,
    GF_CMD_TEST_PIXEL_OPEN_FINISH,
    GF_CMD_TEST_PERFORMANCE,
    GF_CMD_TEST_SPI_PERFORMANCE,
    GF_CMD_TEST_SPI_TRANSFER,
    GF_CMD_TEST_PRE_SPI,
    GF_CMD_TEST_SPI,
    GF_CMD_TEST_SPI_RW,
    GF_CMD_TEST_PRE_GET_VERSION,
    GF_CMD_TEST_GET_VERSION,
    GF_CMD_TEST_FRR_FAR_INIT,
    GF_CMD_TEST_FRR_FAR_RECORD_CALIBRATION,
    GF_CMD_TEST_FRR_FAR_RECORD_ENROLL,
    GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE,
    GF_CMD_TEST_FRR_FAR_RECORD_AUTHENTICATE_FINISH,
    GF_CMD_TEST_FRR_FAR_PLAY_CALIBRATION,
    GF_CMD_TEST_FRR_FAR_PLAY_ENROLL,
    GF_CMD_TEST_FRR_FAR_PLAY_AUTHENTICATE,
    GF_CMD_TEST_FRR_FAR_ENROLL_FINISH,
    GF_CMD_TEST_FRR_FAR_SAVE_FINGER,
    GF_CMD_TEST_FRR_FAR_DEL_FINGER,
    GF_CMD_TEST_FRR_FAR_CANCEL,
    GF_CMD_TEST_RESET_PIN1,
    GF_CMD_TEST_RESET_PIN2,
    GF_CMD_TEST_INTERRUPT_PIN,
    GF_CMD_TEST_DOWNLOAD_FW,
    GF_CMD_TEST_DOWNLOAD_CFG,
    GF_CMD_TEST_DOWNLOAD_FWCFG,
    GF_CMD_TEST_RESET_FWCFG,
    GF_CMD_TEST_SENSOR_VALIDITY,
    GF_CMD_TEST_SET_CONFIG,
    GF_CMD_TEST_DRIVER_CMD,
    GF_CMD_TEST_UNTRUSTED_ENROLL,
    GF_CMD_TEST_UNTRUSTED_AUTHENTICATE,
    GF_CMD_TEST_DELETE_UNTRUSTED_ENROLLED_FINGER,
    GF_CMD_TEST_CHECK_FINGER_EVENT,
    GF_CMD_TEST_BIO_CALIBRATION,
    GF_CMD_TEST_HBD_CALIBRATION,
    GF_CMD_TEST_CANCEL,
    GF_CMD_TEST_REAL_TIME_DATA,
    GF_CMD_TEST_BMP_DATA,
    GF_CMD_TEST_READ_CFG,
    GF_CMD_TEST_READ_FW,
    GF_CMD_NAVIGATE,
    GF_CMD_DETECT_NAV_EVENT,
    GF_CMD_NAVIGATE_COMPLETE,
    GF_CMD_DUMP_NAV_DATA,
    GF_CMD_CHECK_FINGER_LONG_PRESS,
    GF_CMD_FDT_DOWN_TIMEOUT,
    GF_CMD_START_HBD,
    GF_CMD_AUTHENTICATE_FIDO,
    GF_CMD_DUMP_TEMPLATE,
    GF_CMD_DUMP_DATA,
    GF_CMD_DUMP_ORIGIN_DATA,
    GF_CMD_TEST_PRIOR_CANCEL,
    GF_CMD_TEST_STABLE_FACTOR,
    GF_CMD_TEST_RAWDATA_SATURATED,
    GF_CMD_UPDATE_STITCH,
    GF_CMD_AUTHENTICATE_FINISH,
    GF_CMD_DUMP_NAV_ENHANCE_DATA,
    GF_CMD_DUMP_CALIBRATION_DATA_FRAMES,
    GF_CMD_GET_DEV_INFO,
    GF_CMD_LOCKOUT,
    GF_CMD_PAUSE_ENROLL,
    GF_CMD_TEST_UNTRUSTED_PAUSE_ENROLL,
    GF_CMD_TEST_FPC_KEY_DETECT,
    GF_CMD_TEST_BAD_POINT_PRE_GET_BASE,
    GF_CMD_TEST_MEMORY_CHECK,
    GF_CMD_TEST_CALIBRATION_PARA_RETEST,
    GF_CMD_TEST_DATA_NOISE_BASE,
    GF_CMD_TEST_POLLING_IMAGE,
    GF_CMD_TEST_FRR_FAR_PREPROCESS_INIT,
    GF_CMD_TEST_RESET_CLEAR,
    GF_CMD_TEST_SENSOR_BROKEN,
    GF_CMD_TEST_STABLE_FACTOR_BASE,
    GF_CMD_TEST_STABLE_FACTOR_GET_BASE,
    GF_CMD_TEST_STABLE_FACTOR_RESULT,
    GF_CMD_TEST_SNR_GET_RESULT,
    GF_CMD_TEST_TWILL_BADPOINT_GET_RESULT,
    GF_CMD_TEST_PIXEL_SHORT_STREAK,
    GF_CMD_DUMP_DATA_RESERVE,
    GF_CMD_TEST_SYNCHRONOUS_PIXEL_OPEN, //for oppo
    GF_CMD_USER_SET_KEY_MODE,
    GF_CMD_USER_SET_CANCEL,
    GF_CMD_GET_QR_CODE,
    GF_CMD_SYNC_DSC_INFO,
    GF_CMD_MAX,
} gf_cmd_id_t;

typedef enum
{
    OPERATION_ENROLL = 0,/*0*/
    OPERATION_AUTHENTICATE_IMAGE,/*1*/
    OPERATION_AUTHENTICATE_FF,/*2*/
    OPERATION_AUTHENTICATE_SLEEP,/*3*/
    OPERATION_AUTHENTICATE_FIDO,/*4*/
    OPERATION_FINGER_BASE,/*5*/
    OPERATION_NAV,/*6*/
    OPERATION_NAV_BASE,/*7*/
    OPERATION_CHECK_FINGER_LONG_PRESS,/*8*/
    OPERATION_HOME_KEY,/*9*/
    OPERATION_POWER_KEY,/*10*/
    OPERATION_CAMERA_KEY,/*11*/
    OPERATION_HEARTBEAT_KEY,/*12*/
    OPERATION_TEST_IMAGE_MODE,/*13*/
    OPERATION_TEST_DEBUG_MODE,/*14*/
    OPERATION_TEST_FF_MODE,/*15*/
    OPERATION_TEST_KEY_MODE,/*16*/
    OPERATION_TEST_NAV_MODE,/*17*/
    OPERATION_TEST_NAV_BASE_MODE,/*18*/
    OPERATION_TEST_FINGER_BASE_MODE,/*19*/
    OPERATION_TEST_IDLE_MODE,/*20*/
    OPERATION_TEST_SLEEP_MODE,/*21*/
    OPERATION_TEST_HBD_DEBUG_MODE,/*22*/
    OPERATION_TEST_HBD_MODE,/*23*/
    OPERATION_TEST_PIXEL_OPEN_STEP1,/*24*/
    OPERATION_TEST_PIXEL_OPEN_STEP2,/*25*/
    OPERATION_TEST_PIXEL_OPEN_STEP3,/*26*/
    OPERATION_TEST_BAD_POINT_RECODE_BASE,/*27*/
    OPERATION_TEST_BAD_POINT,/*28*/
    OPERATION_TEST_PERFORMANCE,/*29*/
    OPERATION_TEST_SPI_PERFORMANCE,/*30*/
    OPERATION_TEST_SPI_TRANSFER,/*31*/
    OPERATION_TEST_FRR_FAR_RECORD_CALIBRATION,/*32*/
    OPERATION_TEST_FRR_FAR_RECORD_ENROLL,/*33*/
    OPERATION_TEST_FRR_FAR_RECORD_AUTHENTICATE,/*34*/
    OPERATION_TEST_UNTRUSTED_ENROLL,/*35*/
    OPERATION_TEST_UNTRUSTED_AUTHENTICATE,/*36*/
    OPERATION_TEST_CHECK_FINGER_EVENT,/*37*/
    OPERATION_TEST_BIO_CALIBRATION,/*38*/
    OPERATION_TEST_HBD_CALIBRATION,/*39*/
    OPERATION_TEST_REAL_TIME_DATA,/*40*/
    OPERATION_TEST_BMP_DATA,/*41*/
    OPERATION_TEST_SENSOR_VALIDITY,/*42*/
    OPERATION_TEST_RESET_PIN,/*43*/
    OPERATION_TEST_INTERRUPT_PIN,/*44*/
    OPERATION_TEST_PRE_SPI,/*45*/
    OPERATION_SCREEN_OFF_SLEEP,/*46*/
    OPERATION_TEST_STABLE_FACTOR, /*47*/
    OPERATION_TEST_RAWDATA_SATURATED, /*48*/
    OPERATION_TEST_SENSOR_FINE_STEP1,/*49*/
    OPERATION_TEST_SENSOR_FINE_STEP2,/*50*/
    OPERATION_TEST_FPC_KEY_DETECT,/*51*/
    OPERATION_NONE,/*52*/
    OPERATION_LOCKOUT,/*53*/
    OPERATION_PAUSE_ENROLL, /*54*/
    OPERATION_TEST_UNTRUSTED_PAUSE_ENROLL, /*55*/
    OPERATION_TEST_BAD_POINT_BASE_FRAME, /*56*/
    OPERATION_TEST_CALIBRATION_PARA_RETEST_RECODE_BASE,/*57*/
    OPERATION_TEST_CALIBRATION_PARA_RETEST,/*58*/
    OPERATION_TEST_STABLE_FACTOR_BASE,/*59*/
    OPERATION_TEST_DATA_NOISE,/*60*/
    OPERATION_TEST_DATA_NOISE_BASE,/*61*/
    OPERATION_TEST_PIXEL_SHORT_STREAK,/*62*/
    OPERATION_TEST_BOOT_CALIBRATION,/*63*/
    OPERATION_USER_KEY,/*64*/
    OPERATION_USER_NONE,/*65*/
    OPERATION_INVAILD,/*66*/
    OPERATION_MAX,
} gf_operation_type_t;

typedef enum
{
    GF_NAV_NONE = 0,
    GF_NAV_FINGER_UP,
    GF_NAV_FINGER_DOWN,
    GF_NAV_UP,
    GF_NAV_DOWN,
    GF_NAV_LEFT,
    GF_NAV_RIGHT,
    GF_NAV_CLICK,
    GF_NAV_HEAVY,
    GF_NAV_LONG_PRESS,
    GF_NAV_DOUBLE_CLICK,
    GF_NAV_MAX,
} gf_nav_code_t;

typedef enum
{
    GF_NAV_CLICK_STATUS_NONE = 0,
    GF_NAV_CLICK_STATUS_DOWN,
    GF_NAV_CLICK_STATUS_DOWN_UP,
} gf_nav_click_status_t;

typedef enum
{
    GF_KEY_NONE = 0,  //
    GF_KEY_HOME,
    GF_KEY_POWER,
    GF_KEY_MENU,
    GF_KEY_BACK,
    GF_KEY_CAMERA,
    GF_KEY_MAX,
} gf_key_code_t;

typedef enum
{
    GF_KEY_STATUS_UP = 0,  //
    GF_KEY_STATUS_DOWN,
} gf_key_status_t;

/* TODO: use bitmask, since several IRQs would occur the same time */
#define GF_IRQ_FINGER_DOWN_MASK     (1 << 1)
#define GF_IRQ_FINGER_UP_MASK       (1 << 2)
#define GF_IRQ_MENUKEY_DOWN_MASK    (1 << 3)
#define GF_IRQ_MENUKEY_UP_MASK      (1 << 4)
#define GF_IRQ_BACKKEY_DOWN_MASK    (1 << 5)
#define GF_IRQ_BACKKEY_UP_MASK      (1 << 6)
#define GF_IRQ_IMAGE_MASK           (1 << 7)
#define GF_IRQ_RESET_MASK           (1 << 8)
#define GF_IRQ_TMR_IRQ_MNT_MASK     (1 << 9)      // idle timeout int
#define GF_IRQ_ONE_FRAME_DONE_MASK  (1 << 10)
#define GF_IRQ_ESD_IRQ_MASK         (1 << 11)      // esd irq
#define GF_IRQ_ADC_FIFO_FULL_MASK   (1 << 12)      // ADC test irq
#define GF_IRQ_ADC_FIFO_HALF_MASK   (1 << 13)      // ADC test irq
#define GF_IRQ_FDT_REVERSE_MASK     (1 << 14)      // fdt_irq1 for milan f
#define GF_IRQ_NAV_MASK             (1 << 15)
#define GF_IRQ_GSC_MASK             (1 << 16)      // For MiLan A
#define GF_IRQ_TEMPERATURE_BAD_MASK (1 << 16)      // For Dubai B
#define GF_IRQ_HBD_MASK             (1 << 17)      // For MiLan A
#define GF_IRQ_FW_ERR_MASK          (1 << 18)      // For MiLan A
#define GF_IRQ_CFG_ERR_MASK         (1 << 19)      // For MiLan A
#define GF_IRQ_ESD_ERR_MASK         (1 << 20)      // For MiLan A
#define GF_IRQ_NAV_LEFT_MASK        (1 << 21)      // For MiLan A
#define GF_IRQ_NAV_RIGHT_MASK       (1 << 22)      // For MiLan A
#define GF_IRQ_NAV_UP_MASK          (1 << 23)      // For MiLan A
#define GF_IRQ_NAV_DOWN_MASK        (1 << 24)      // For MiLan A
#define GF_IRQ_PRESS_LIGHT_MASK     (1 << 25)      // For MiLan A
#define GF_IRQ_PRESS_HEAVY_MASK     (1 << 26)      // For MiLan A
#define GF_IRQ_UPDATE_BASE_MASK     (1 << 27)      // For MiLan A
#define GF_IRQ_FARGO_ERR_MASK       (1 << 16)          // For Milan HV Fargo
#define GF_IRQ_FARGO_ERR_NOT_RESOLVED_MASK (1 << 17)   // For Milan HV Fargo
#define GF_IRQ_RESET_FIRST_MASK     (1 << 18)          // For Milan HV Fargo
#define GF_IRQ_RESET_FAILED_MASK    (1 << 19)          // For Milan HV
#define GF_IRQ_TEMPERATURE_CHANGE_MASK  (1 << 28)      // For Milan HV
#define GF_IRQ_TCODE_CHANGE_MASK    (1 << 29)      // For MiLan An Tcode Change
#define GF_IRQ_INVALID_MASK         (1 << 30)      // For MiLan An
#define GF_IRQ_ERR_CHECKSUM_MASK    (1 << 31)      // For MiLan An

// helper macro for IRQ MASK
#define IS_FINGER_DOWN_MASK(irq_type) (0 != ((irq_type) & GF_IRQ_FINGER_DOWN_MASK))
#define IS_FINGER_UP_MASK(irq_type)   (0 != ((irq_type) & GF_IRQ_FINGER_UP_MASK))
#define IS_IMAGE_MASK(irq_type)       (0 != ((irq_type) & GF_IRQ_IMAGE_MASK))

typedef enum
{
    CONFIG_NORMAL = 0,  //
    CONFIG_TEST_PIXEL_OPEN_A,
    CONFIG_TEST_PIXEL_OPEN_B,
    CONFIG_TEST_HBD,
    CONFIG_TEST_FROM_FILE,
} gf_config_type_t;

typedef struct
{
    uint8_t reset_flag;

    gf_mode_t mode;
    gf_operation_type_t operation;

    /*compatible with different TEE*/
    gf_error_t result;

    gf_operation_type_t operation_array[MAX_OPERATION_ARRAY_SIZE];

    uint32_t operation_id;
    uint32_t cmd_id;
    uint8_t temp_check_level;  // 0:fast check 1:full check
    uint8_t rsv_data[31];
} gf_cmd_header_t;

typedef struct {
    gf_cmd_header_t cmd_header;
    uint8_t reset_flag;
    uint8_t dump_data_flag;
    uint16_t pixel_image1[IMAGE_BUFFER_LEN];
    uint16_t pixel_image2[IMAGE_BUFFER_LEN];
    uint32_t pixel_image_data_len;
    uint32_t bad_pixel_num;
} gf_cmd_test_pixel_open;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_config_t config;
    uint8_t retry_time;
} gf_detect_sensor_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t download_fw_flag;
    uint8_t download_cfg_flag;
    uint8_t esd_check_flag;
    uint32_t row;
    uint32_t col;
    uint32_t row_ee;
    uint32_t col_ee;
    uint32_t nav_row;
    uint32_t nav_col;
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t otp_info[GF_SENSOR_OTP_BUFFER_LEN];
    uint32_t otp_info_len;
    gf_chip_type_t chip_type;
    gf_chip_series_t chip_series;
} gf_init_t;

typedef struct
{
    uint8_t vendor_id;
    uint8_t mode;
    uint8_t operation;
    uint8_t reserved[5];
} gf_ioc_chip_info_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint16_t orientation;
    uint16_t facing;
} gf_init_finished_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint64_t challenge;
} gf_pre_enroll_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint8_t dump_swipe_enroll_enable;
    uint8_t system_auth_token_version;
    gf_hw_auth_token_t hat;
} gf_enroll_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint64_t operation_id;
    int32_t screen_on_flag;
} gf_authenticate_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t aaid_len;
    uint8_t aaid[MAX_AAID_LEN];
    uint32_t final_challenge_len;
    uint8_t final_challenge[MAX_FINAL_CHALLENGE_LEN];
} gf_authenticate_fido_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint64_t auth_id;
} gf_get_auth_id_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_operation_type_t operation;
} gf_cancel_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t nav_mode;
} gf_nav_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint32_t algo_result;
} gf_save_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
} gf_update_stitch_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
    uint32_t finger_id;
    uint32_t removing_templates;
    uint32_t deleted_fids[MAX_FINGERS_PER_USER];
    uint32_t deleted_gids[MAX_FINGERS_PER_USER];
} gf_remove_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;
} gf_set_active_group_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t safe_class;
} gf_set_safe_class_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t size;
    uint32_t group_ids[MAX_FINGERS_PER_USER];
    uint32_t finger_ids[MAX_FINGERS_PER_USER];
} gf_enumerate_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t enable_flag;
} gf_enable_fingerprint_module_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t enable_flag;
} gf_enable_ff_feature_t;


typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t pause_enroll_flag;
} gf_pause_enroll_t;


typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t lockout_flag;
} gf_lockout_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t qr_code[20];
} gf_qr_code_t;

typedef struct
{
    int32_t image_quality;
    int32_t valid_area;
    uint32_t match_score;
    int32_t key_point_num;
    uint32_t increase_rate;
    uint32_t overlay;
    uint32_t polling_image_time;
    uint32_t get_raw_data_time;
    uint32_t broken_check_time;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t enroll_time;
    uint32_t authenticate_time;
    uint32_t authenticate_update_flag;
    uint32_t authenticate_finger_count;
    uint32_t authenticate_finger_time;
    uint32_t total_time;
    uint32_t get_gsc_data_time;
    uint32_t bio_assay_time;
    int32_t bio_assay_ret;
    uint32_t try_count;
    uint32_t is_final;
    uint32_t template_count;
    uint32_t bad_point_num;
    uint32_t fp_rawdata_max;
    uint32_t fp_rawdata_min;
    uint32_t fp_rawdata_average;
    uint32_t gsc_rawdata_max;
    uint32_t gsc_rawdata_min;
    uint32_t gsc_rawdata_average;
} gf_test_performance_t;

typedef struct
{
    uint32_t get_dr_timestamp_time;
    uint32_t get_mode_time;
    uint8_t fw_version[FW_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint32_t get_chip_id_time;
    uint32_t get_vendor_id_time;
    uint32_t get_sensor_id_time;
    uint32_t get_fw_version_time;
    uint32_t get_image_time;
    uint32_t raw_data_len;
} gf_test_spi_performance_t;

typedef struct
{
    uint16_t m_avg_diff_val;
    double m_noise;
    uint32_t m_bad_pixel_num;
    uint32_t m_local_small_bad_pixel_num;
    uint32_t m_local_big_bad_pixel_num;
    uint32_t m_flatness_bad_pixel_num;
    uint32_t m_is_bad_line;
    float m_all_tilt_angle;
    float m_block_tilt_angle_max;
} gf_bad_point_test_result_oswego_t;

// returned parameters
typedef struct
{
    uint16_t total;
    uint16_t local;
    uint16_t num_local;
    uint16_t local_worst;
    uint16_t coverage;
    uint32_t singular;
    uint8_t *p_bad_pixels;
    uint8_t *p_local_bad_num;
} gf_bad_point_test_result_milan_t;

typedef union
{
    gf_bad_point_test_result_oswego_t oswego;
    gf_bad_point_test_result_milan_t milan;
} gf_bad_point_test_result_t;

typedef struct
{
    int32_t total;
    int32_t local;
    int32_t local_worst;
    int32_t singular;
    int32_t bad_pixel_double_check;
    int32_t normal_pixel_double_check;
} gf_badpoint_parameter_threshold_t;


typedef struct
{
    int32_t center_width;
    int32_t edge_margin;
    int32_t center_diff;
    int32_t edge_diff;
    int32_t center_total;
    int32_t edge_total;
    int32_t center_local;
    int32_t edge_local;
    int32_t center_local_worst;
    int32_t edge_local_worst;
    int32_t center_singular;
    int32_t edge_singular;
}gf_badpoint_parameter_threshold_stbow_t;

typedef struct
{
    uint16_t max_total;
    uint16_t max_local;
    uint16_t max_num_local;
    uint16_t max_local_worst;
    // uint16_t min_coverage;
    // uint16_t coverage_flag;
}gf_badpoint_threshold_stbow_t;

typedef struct
{
    uint16_t max_total;
    uint16_t max_local;
    uint16_t max_num_local;
    uint16_t max_local_worst;
    uint16_t min_coverage;
    uint16_t coverage_flag;
}gf_badpoint_threshold_t;

typedef struct
{
    gf_bad_point_test_result_t result;
    uint8_t algo_processed_flag;
} gf_test_bad_point_t;

typedef struct
{
    gf_cmd_header_t cmd_header;

    // it is used to test heard beat
    uint32_t hbd_switch_flag;  // 0 : disable, 1: enable
    uint32_t electricity_value;  // LED0

    // virable below used to test both gsc and hear beat
    uint16_t hbd_base;
    uint16_t hbd_avg;

    uint8_t hdb_data[HBD_BUFFER_LEN];
    uint32_t hbd_data_len;
} gf_test_hbd_feature_t;

typedef struct
{
    uint8_t broken_checked;
    uint8_t disable_sensor;
    uint8_t disable_study;
} gf_sensor_broken_check_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t fw_version[FW_VERSION_INFO_LEN];
    uint8_t code_fw_version[FW_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint8_t sensor_otp_type;
} gf_test_spi_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t spi_cmd;
    uint32_t start_addr;
    uint32_t rw_len;
    uint8_t rw_content[GF_MAX_SPI_RW_LEN];
} gf_test_spi_rw_t;

typedef struct
{
    uint8_t bmp2_data[IMAGE_BUFFER_LEN];
    uint8_t touch_mask[IMAGE_BUFFER_LEN];
    uint16_t kr_data[IMAGE_BUFFER_LEN];
    uint16_t b_data[IMAGE_BUFFER_LEN];
    uint16_t reserved1[IMAGE_BUFFER_LEN];
    uint8_t reserved2[IMAGE_BUFFER_LEN];
} gf_test_frr_far_reserve_data_t;

typedef struct
{
    gf_cmd_header_t cmd_header;

    // if greater than zero, don't check enroll failed condition
    uint8_t check_flag;

    int32_t algo_index;

    uint16_t raw_data[IMAGE_BUFFER_LEN];
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    // TEST_TOKEN_RAW_DATA or TEST_TOKEN_BMP_DATA, using what kind of data to run test
    uint32_t data_type;
    int32_t gsc_data[GF_LIVE_DATA_LEN];
    uint32_t gsc_flag;
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
    uint32_t get_feature_time;
    uint32_t authenticate_time;

    // for consistency test
    uint32_t score;
    uint32_t update;
    uint32_t study_replase_index;
    uint32_t key_point_num;
    uint32_t cache_tmp_num;
    uint32_t extra_tmp_num;

    // for algo broken mask
    uint32_t have_broken_mask;
    uint8_t broken_mask[IMAGE_BUFFER_LEN];
} gf_test_frr_far_t;

typedef struct
{
    gf_test_frr_far_t frr_far;
    gf_test_frr_far_reserve_data_t frr_far_reserve;
} gf_test_frr_far_new_t;

typedef struct
{
    gf_cmd_header_t cmd_header;

    uint32_t calibration_type;
    uint32_t frame_num;
    uint16_t base_data[IMAGE_BUFFER_LEN];
    int16_t kr_data[IMAGE_BUFFER_LEN];
    int16_t b_data[IMAGE_BUFFER_LEN];
} gf_test_calibration_t;

typedef struct
{
    uint16_t avg_base_value;
    uint16_t avg_touch_value;
    uint16_t min_touch_value;
    uint32_t result;  // 0 SUCCESS  1  FAILED
} gf_test_calibration_para_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_test_calibration_para_t cmd_data;
} gf_test_cmd_calibration_para_t;

typedef struct
{
    gf_cmd_header_t cmd_header;

    uint32_t group_id;  // user num
    uint32_t finger_id;
    uint8_t finger_name[MAX_FINGER_PATH_NAME_SIZE];
} gf_test_frr_far_save_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t group_id;  // user num
    uint32_t finger_id;
} gf_test_frr_far_remove_t;

typedef struct gf_test_frr_far_finger gf_test_frr_far_finger_t;
struct gf_test_frr_far_finger
{
    uint32_t group_id;
    uint32_t finger_id;
    gf_test_frr_far_finger_t *next;
};

typedef struct
{
    uint32_t count;
    gf_test_frr_far_finger_t *finger;
} gf_test_frr_far_finger_list_t;

typedef struct
{
    gf_cmd_header_t cmd_header;

    uint32_t safe_class;
    uint32_t template_count;
    uint32_t support_bio_assay;
    uint32_t forbidden_duplicate_finger;
    uint32_t finger_group_id;
    uint32_t frr_far_para_flag;
    uint32_t increase_rate;
    uint32_t overlay;
} gf_test_frr_far_init_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    int16_t kr_data[IMAGE_BUFFER_LEN];
    int16_t b_data[IMAGE_BUFFER_LEN];
    uint16_t base_data[IMAGE_BUFFER_LEN];
    uint16_t raw_data[IMAGE_BUFFER_LEN];
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    uint8_t fpc_key_data[FPC_KEY_DATA_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
} gf_test_real_time_data_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t bmp_data[IMAGE_BUFFER_LEN];
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t preprocess_time;
} gf_test_bmp_data_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t data_type;   // base or rawdate
    uint32_t frame_num; /* n */
    uint32_t max_frame_num; /* N */
    uint16_t raw_data[IMAGE_BUFFER_LEN];
} gf_test_data_noise_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint16_t image[IMAGE_BUFFER_LEN];
} gf_polling_image_t;

typedef struct
{
    gf_nav_code_t nav_code;
    uint8_t finger_up_flag;
} gf_nav_result_t;

typedef struct
{
    uint32_t average_pixel_diff;
} gf_test_sensor_fine_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_nav_result_t nav_result;
} gf_detect_nav_event_t;

// keep the definition consistent to the algorithm implementation in gf_heart_beat.c
typedef enum
{
    GF_HEART_BEAT_UNSTABLE = 2,  //
    GF_HEART_BEAT_STABLE = 0
} gf_heart_beat_status_t;

typedef struct
{
    uint8_t heart_beat_rate;
    uint8_t status;
    uint8_t index;
    uint16_t raw_data[HBD_BUFFER_LEN];
    int32_t display_data[HBD_DISPLAY_DATA_LEN];
} gf_heart_beat_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    int32_t is_passed;  // < 0:fail, 1:pass>
} gf_test_sensor_validity_t;

typedef struct
{
    uint16_t untouch_data_len;
    uint16_t untouch_data[GSC_UNTOUCH_LEN];
    uint16_t touch_data_len;
    uint16_t touch_data[GSC_TOUCH_LEN];
    uint8_t gsc_flag;
} gf_gsc_t;

typedef enum {
    GF_ENROLL_STATE_NORMALL = 0,
    GF_ENROLL_STATE_PAUSED
} gf_enroll_state_t;

typedef enum
{
    GF_IRQ_STEP_IDLE = 0,  //
    GF_IRQ_STEP_GET_IRQ_TYPE,
    GF_IRQ_STEP_POLLING,
    GF_IRQ_STEP_PRE_GET_IMAGE,  // spi speed: 1M
    GF_IRQ_STEP_GET_IMAGE,  // spi speed: high
    GF_IRQ_STEP_POST_GET_IMAGE,  // spi speed: 1M
    GF_IRQ_STEP_CLEAR_IRQ,
    GF_IRQ_STEP_PROCESS,
    GF_IRQ_STEP_CONTINUE,
} gf_irq_step_t;

typedef struct
{
    uint32_t uvt_len;
    uint8_t uvt_buf[MAX_UVT_LEN];
} gf_uvt_t;

typedef struct
{
    uint32_t under_saturated_pixel_count;
    uint32_t over_saturated_pixel_count;
    uint32_t saturated_pixel_threshold;
} gf_test_rawdata_saturated_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint32_t irq_type;
    gf_operation_type_t operation;
    gf_mode_t mode;
    uint8_t too_fast_flag;
    uint8_t mistake_touch_flag;
    uint8_t report_authenticate_fail_flag;
    uint32_t dump_selete_index;
    uint32_t bad_pixel_num;
    uint32_t local_bad_pixel_num;
    uint32_t bad_pixel_num_dummy;
    uint32_t image_count;
    gf_nav_result_t nav_result;
    gf_test_performance_t test_performance;
    gf_test_spi_performance_t test_spi_performance;
    gf_test_frr_far_t test_frr_far;
    gf_test_bad_point_t test_bad_point;
    gf_test_hbd_feature_t test_hdb_feature;
    gf_test_rawdata_saturated_t test_rawdata_saturated;
    gf_sensor_broken_check_t broken_check;
    gf_heart_beat_t heart_beat;
    gf_gsc_t gsc;
    gf_test_sensor_validity_t test_sensor_validity;
    union
    {
        gf_test_real_time_data_t test_real_time_data;
        gf_test_frr_far_reserve_data_t test_frr_far_reserve_data;
    }
    share;
    gf_test_bmp_data_t test_bmp_data;
    gf_test_data_noise_t test_data_noise;
    gf_test_sensor_fine_t test_sensor_fine;
    gf_test_calibration_t test_calibration;
    gf_test_cmd_calibration_para_t test_calibration_para_retest;


    uint32_t group_id;
    uint32_t finger_id;
    uint16_t samples_remaining;
    uint32_t duplicate_finger_id;
    uint8_t save_flag;
    uint8_t update_stitch_flag;
    gf_test_performance_t dump_performance;

    gf_hw_auth_token_t auth_token;
    gf_uvt_t uvt;

    uint32_t speed;
    gf_irq_step_t step;
    uint32_t bad_pixel_short_streak_num;
} gf_irq_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_sensor_broken_check_t broken_check;
    gf_operation_type_t operation;
    uint32_t irq_type;
    uint32_t group_id;
    uint32_t finger_id;
    uint8_t save_flag;
    uint8_t update_stitch_flag;
} gf_authenticate_finish_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t download_fw_flag;
    uint8_t download_cfg_flag;
} gf_esd_check_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    int8_t algo_version[ALGO_VERSION_INFO_LEN];
    int8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    int8_t fw_version[FW_VERSION_INFO_LEN];
    int8_t tee_version[TEE_VERSION_INFO_LEN];
    int8_t ta_version[TA_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];
    uint8_t production_date[PRODUCTION_DATE_LEN];
    int8_t heart_beat_algo_version[GF_HEART_BEAT_ALGO_VERSION_LEN];
} gf_test_get_version_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t cmd_buf[64];
    uint8_t download_fw_flag;
    gf_mode_t mode;
    uint8_t product_cfg_idx;
    uint32_t secure_share_memory_count;
    uint32_t secure_share_memory_size[10];
    uint64_t secure_share_memory_time[10];
    uint32_t esd_exception_count;
    uint16_t address;
    uint8_t value;
    uint8_t ignore_irq_type;
} gf_test_driver_cmd_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t fw_data[GF_FW_LEN];
    uint32_t fw_data_len;
    uint8_t cfg_data[GF_CFG_LEN];
    uint32_t cfg_data_len;
    uint32_t fpc_key_dl_flag;
} gf_test_download_fw_cfg_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_config_t config;
    uint32_t token;
} gf_test_set_config_t;

typedef struct
{
    uint8_t data[ORIGIN_RAW_DATA_LEN * MAX_CONTINUE_FRAME_NUM * MAX_RESCAN_TIME];
    uint32_t data_len;
} gf_image_origin_data_t;

typedef struct
{
    uint8_t data[5][ORIGIN_RAW_DATA_LEN];
    uint32_t data_len;
} gf_nav_origin_data_t;

typedef struct
{
    gf_operation_type_t operation;
    union
    {
        gf_image_origin_data_t origin_image;
        gf_nav_origin_data_t origin_nav;
    }
    data;
} gf_dump_origin_data_t;

typedef struct
{
    uint16_t bad_point_test_raw_data[MAX_BAD_POINT_TEST_FRAME_NUM][IMAGE_BUFFER_LEN];
} gf_dump_bad_point_data_t;

typedef struct
{
    uint8_t touch_mask[IMAGE_BUFFER_LEN];
    uint8_t dataf_bmp[IMAGE_BUFFER_LEN];
} gf_dump_reserve_data_t;

typedef struct
{
    uint8_t origin_data[ORIGIN_RAW_DATA_LEN];
    uint32_t origin_data_len;
    uint16_t raw_data[IMAGE_BUFFER_LEN * MAX_CONTINUE_FRAME_NUM * MAX_RESCAN_TIME];
    uint32_t raw_data_len;
    int32_t enroll_select_index;

    uint8_t broken_check_origin_data[2][ORIGIN_RAW_DATA_LEN];
    uint16_t broken_check_raw_data[BROKEN_CHECK_MAX_FRAME_NUM][IMAGE_BUFFER_LEN];
    uint16_t broken_check_base[IMAGE_BUFFER_LEN];
    uint32_t broken_check_frame_num;
    uint32_t broken_check_base_len;

    union
    {
        gf_dump_bad_point_data_t bad_point;
        gf_dump_reserve_data_t reserve;
    }
    share;

    uint8_t gsc_untouch_data[GSC_RAW_DATA_BUFFER_LEN];
    uint32_t gsc_untouch_data_len;
    uint8_t gsc_touch_data[GSC_RAW_DATA_BUFFER_LEN];
    uint32_t gsc_touch_data_len;
    uint16_t gsc_base;

    uint8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];

    uint32_t frame_num;
    int16_t kr[IMAGE_BUFFER_LEN];
    int16_t b[IMAGE_BUFFER_LEN];
    uint16_t cali_res[IMAGE_BUFFER_LEN];
    uint8_t data_bmp[IMAGE_BUFFER_LEN];
    uint8_t data_bmp_before_ee[IMAGE_BUFFER_LEN];
    uint8_t data_bmp_ee[IMAGE_BUFFER_LEN];
    uint8_t data_bmp_temp[IMAGE_BUFFER_LEN];
    uint8_t sito_bmp[IMAGE_BUFFER_LEN];
    uint32_t select_index;  // 0: no sito; 1: sito
    // for algo broken mask
    uint8_t broken_mask[IMAGE_BUFFER_LEN];

    uint32_t image_quality;
    uint32_t valid_area;

    uint32_t increase_rate_between_stitch_info;
    uint32_t overlap_rate_between_last_template;
    uint32_t enrolling_finger_id;
    uint32_t duplicated_finger_id;

    uint32_t match_score;
    uint32_t match_finger_id;
    uint32_t study_flag;

    uint32_t authenticate_retry_count;

    gf_test_performance_t dump_performance;
    uint32_t ee_w;
    uint32_t ee_h;
    uint32_t pl_w;
    uint32_t pl_h;
    uint8_t data2_bmp[IMAGE_BUFFER_LEN];
} gf_image_data_t;

typedef struct
{
    uint32_t polling_index;
    int32_t overlap_error;
    uint32_t overlap_rate;
    uint8_t register_flag;  // 1 success, 2 fail, 3 not registered
    uint32_t raw_data_len;
    uint16_t raw_data[IMAGE_BUFFER_LEN];
} gf_dump_swipe_enroll_raw_t;

typedef struct
{
    uint32_t polling_index;
    uint32_t image_quality;
    uint32_t valid_area;
    uint8_t record_flag;
    uint8_t preprocess_version[ALGO_VERSION_INFO_LEN];
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];

    int16_t kr[IMAGE_BUFFER_LEN];
    int16_t b[IMAGE_BUFFER_LEN];
    uint16_t cali_res[IMAGE_BUFFER_LEN];
    uint8_t data_bmp[IMAGE_BUFFER_LEN];
    uint8_t data2_bmp[IMAGE_BUFFER_LEN];
    uint8_t sito_bmp[IMAGE_BUFFER_LEN];
    // for algo broken mask
    uint8_t broken_mask[IMAGE_BUFFER_LEN];
    uint8_t touch_mask[IMAGE_BUFFER_LEN];
} gf_dump_swipe_enroll_cali_t;

typedef struct
{
    gf_operation_type_t operation;
    // other
    uint32_t image_quality;
    uint32_t valid_area;
    uint32_t match_score;
    uint32_t match_finger_id;
    uint32_t authenticate_retry_count;
    uint8_t is_only_dump_broken_check;
    // asp
    uint8_t asp_level;
    float asp_probability;
    uint32_t asp_timeinterval;
    uint32_t asp_updateflag;
    uint32_t asp_sensortype;
    uint16_t diff_use;
    uint16_t default_dac[4];
    uint16_t asp_tcode;
    int8_t asp_version[ALGO_VERSION_INFO_LEN];
    int8_t algo_version[ALGO_VERSION_INFO_LEN];
    float asp_template_feature[80];
    float asp_sample_feature[80];
    int32_t asp_stesting_flag;
    int32_t asp_testing_flag;
    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t reserve[MAX_OTHER_RESERVE_LEN];
} gf_dump_reserve_t;

typedef struct
{
    uint32_t polling_index;
    uint8_t register_flag;
} gf_swipe_enroll_register_result_t;

typedef struct
{
    // one swipe data/status
    uint16_t* raw_data_record;
    gf_swipe_enroll_register_result_t* register_results;
    uint32_t recorded_raw_data_num;
    uint32_t total_polling_frame_num;
    uint32_t overlap_frame_num;
    uint32_t dirty_finger_num;
    uint8_t  finger_is_up;
    int32_t  error_code;
    uint64_t swipe_start_time;
    uint64_t swipe_end_time;

    // swipe enroll status
    uint32_t enroll_progress;
    uint32_t last_swipe_enroll_progress;
    uint8_t  dump_enable;
    uint32_t dump_raw_frame_num;
    uint32_t dump_cali_frame_num;
    uint16_t dump_raw_file_num;
    uint16_t dump_cali_file_num;
    uint8_t  unsaved_raw_num;
    uint8_t  unsaved_cali_num;
    uint16_t raw_file_read_index;
    uint16_t cali_file_read_index;
    gf_dump_swipe_enroll_raw_t raw_dump[SWIPE_ENROLL_DUMP_RAW_NUM];
    gf_dump_swipe_enroll_cali_t cali_dump[SWIPE_ENROLL_DUMP_CALI_NUM];
} gf_swipe_enroll_status_t;

typedef struct
{
    uint8_t origin_data[5][ORIGIN_RAW_DATA_LEN];
    uint16_t raw_data[NAV_MAX_BUFFER_LEN];
    uint32_t raw_data_len;
    uint8_t frame_num[NAV_MAX_FRAMES];
    uint8_t finger_up_flag[NAV_MAX_FRAMES];
    uint32_t nav_times;
    uint32_t nav_frame_index;
    uint32_t nav_frame_count;
    uint32_t origin_data_len;
    gf_nav_code_t nav_code;
    gf_nav_config_t nav_config;
} gf_nav_data_t;

typedef struct
{
    uint16_t nav_raw_data_enhance[NAV_MAX_BUFFER_LEN];
    /***********bigdata end***********/
    uint8_t frame_num[NAV_MAX_FRAMES];
    uint32_t nav_times;
    uint32_t nav_frame_index;
    uint32_t nav_frame_count;
} gf_nav_enhance_data_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint16_t raw_data[CALIBRATION_MAX_FRAMES][BN_IMAGE_BUFFER_LEN];
    uint8_t calibration_params[CALIBRATION_MAX_COUNT][CALIBRATION_PARAMS_LEN];
    uint8_t is_base_valid[CALIBRATION_MAX_INDEX];
    uint8_t base_chosen[CALIBRATION_MAX_INDEX];
    uint32_t calibration_frame_num;
    uint32_t calibration_index;
    uint8_t key_status[CALIBRATION_MAX_INDEX];
} gf_calibration_data_frames_t;

typedef struct
{
    uint8_t hbd_data[HBD_BUFFER_LEN];
    uint32_t hbd_data_len;
} gf_hbd_data_t;

typedef struct
{
    uint8_t template_data[DUMP_TEMPLATE_BUFFER_LEN];
    uint32_t template_len;
    uint32_t group_id;
    uint32_t finger_id;
} gf_dump_template_t;

typedef struct
{
    uint32_t enrolling_finger_id;
    uint32_t data_len;
    uint8_t  data_type;  // 0 raw data, 1 cali data
    uint8_t  finish_flag;
    union
    {
        gf_dump_swipe_enroll_raw_t swipe_raw[SWIPE_ENROLL_DUMP_RAW_NUM*3];
        gf_dump_swipe_enroll_cali_t swipe_cali[SWIPE_ENROLL_DUMP_CALI_NUM*3];
    }
    swipe_data;
} gf_dump_swipe_enroll_t;

typedef struct
{
    gf_operation_type_t operation;
    uint8_t swipe_enroll_flag;
    union
    {
        gf_image_data_t image;
        gf_nav_data_t nav;
        gf_hbd_data_t hbd;
        gf_nav_enhance_data_t nav_enhance;
        gf_calibration_data_frames_t calibration_data_frames;
        gf_dump_swipe_enroll_t swipe;
    }
    data;
    int64_t down_irq_time;
    uint8_t screen_flag;
    uint8_t is_only_dump_broken_check;
} gf_dump_data_t;

typedef struct
{
    gf_key_code_t key;
    gf_key_status_t status;
} gf_key_event_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t key_code;
    uint8_t can_test;
    uint8_t fail_stat;
    uint8_t fpc_key_en;
    uint8_t key_rawdata[6];
    uint8_t fpc_key_cancel[6];
} gf_test_fpc_key_data_t;

enum gf_netlink_cmd
{
    GF_NETLINK_TEST = 0,
    GF_NETLINK_IRQ = 1,
    GF_NETLINK_SCREEN_OFF,
    GF_NETLINK_SCREEN_ON,
    GF_NETLINK_DETECT_BROKEN,
};

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_irq_t irq;
    uint32_t speed;
    gf_irq_step_t next_step;
} gf_process_irq_t;


typedef struct
{
    uint16_t finger_base_rawdata[IMAGE_BUFFER_LEN];
    uint32_t finger_base_rawdata_len;
} gf_base_data_t;

typedef struct gf_memory_info
{
    uint32_t ca_leak_count;
    uint32_t ta_leak_count;
} gf_memory_info_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    gf_memory_info_t memory_info;
} gf_memory_check_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t algo_result;
    float algo_signal;
    float algo_noise;
    float algo_snr;
    float algo_select_percentage;
    uint16_t product_id;
}snr_result_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t algo_result;
    float algo_stable_result;
}stable_result_t;

typedef struct
{
    gf_cmd_header_t cmd_header;
    uint8_t algo_result;
    uint32_t algo_badpoint_total_result;
    uint32_t algo_badpoint_local_result;
    uint32_t algo_badpoint_numlocal_result;
    uint32_t algo_badpoint_line_result;
    uint8_t algo_badpoint_mat_result[IMAGE_BUFFER_LEN];
    uint8_t algo_badpoint_localmat_result[IMAGE_BUFFER_LEN];
}twill_badpoint_result_t;

const char *gf_strmode(gf_mode_t mode);
const char *gf_strcmd(gf_cmd_id_t cmd_id);
const char *gf_stroperation(gf_operation_type_t opera);
const char *gf_strnav(gf_nav_code_t nav);
const char *gf_strkey(gf_key_code_t key);
const char *gf_strirq(uint32_t irq_type);

#ifndef SUPPORT_FUNCTIONAL_TEST
#define TEST_STUB_NO_ARGS(stub_function_name)
#define TEST_STUB(stub_function_name, ...)
#define TEST_STUB_VOID(stub_function_name, ...)
#define TEST_HOOK(hook_function_name, ...)
#endif  // SUPPORT_FUNCTIONAL_TEST

#define SUPPORT_IMAGE_EE_CONDITION(config)          \
        ((config).support_image_ee_net && (config).support_swipe_enroll == 0 \
            && ((config).chip_type == GF_CHIP_5658ZN1 \
            || (config).chip_type == GF_CHIP_5658ZN2 \
            || (config).chip_type == GF_CHIP_3626ZS1 \
            || (config).chip_type == GF_CHIP_3636ZS1 \
            || (config).chip_type == GF_CHIP_3988 \
            || (config).chip_type == GF_CHIP_3956 \
            || (config).chip_type == GF_CHIP_3216 \
            || (config).chip_type == GF_CHIP_3668DN1))


#ifdef __cplusplus
}
#endif  // end ifdef __cplusplus

#endif  // _GF_COMMON_H_
