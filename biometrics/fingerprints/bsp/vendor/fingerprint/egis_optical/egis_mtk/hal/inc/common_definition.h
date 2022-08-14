#ifndef __COMMON_DEFINITION_H__
#define __COMMON_DEFINITION_H__

#include <stdint.h>
#include "ini_definition.h"

typedef enum extra_type {
    PID_COMMAND,
    PID_INLINETOOL = 3,
    PID_FAETOOL,
    PID_DEMOTOOL,
    PID_BKG_IMG,
    PID_HOST_TOUCH,
    PID_SET_ENROLL_TIMEOUT,
    PID_HOST_TEMPERATURE,
    PID_7XX_INLINETOOL,
    PID_EVTOOL,
    PID_SYSUNLOCKTOOL_NAME,
    PID_SYSUNLOCKTOOL_TOTAL_COUNT,
    PID_SYSUNLOCKTOOL_MATCH_COUNT,
    PID_SYSUNLOCKTOOL_NOT_MATCH_COUNT,
} extra_type_t;

#define CMD_GET_CONFIG 2001
#define CMD_UPDATE_CONFIG 2002
#define CMD_SET_NAVI_CONFIG 2003
#define CMD_REMOVE_INI_FILE 2004
#define CMD_UPDATE_DB_CONFIG 2005
typedef enum sensor_work_mode {
    SENSOR_MODE = 0,
    DETECT_MODE,
    POWEROFF_MODE
} sensor_work_mode_t;

typedef enum opt_type {
    TYPE_SEND_CALIBRATION_DATA,  // 0
    TYPE_SEND_TEMPLATE_START,
    TYPE_SEND_TEMPLATE,
    TYPE_SEND_TEMPLATE_END,
    TYPE_RECEIVE_CALIBRATION_DATA,
    TYPE_RECEIVE_BACKUP_CALIBRATION_DATA,
    TYPE_RECEIVE_TEMPLATE_START,
    TYPE_RECEIVE_TEMPLATE,
    TYPE_RECEIVE_TEMPLATE_END,
    TYPE_RECEIVE_IMAGE,
    TYPE_RECEIVE_MULTIPLE_IMAGE,
    TYPE_DELETE_TEMPLATE,  // 10
    TYPE_TEST_TRANSPORTER,
    TYPE_SEND_INI_CONFIG,
    TYPE_DESTROY_INI_CONFIG,
    TYPE_RECEIVE_LIVE_IMAGE = 15,
    TYPE_RECEIVE_TA_VERSION,
    TYPE_RECEIVE_ALGO_VERSION,
    TYPE_RECEIVE_IP_VERSION,
    TYPE_RECEIVE_USER_INFO,
    TYPE_SEND_USER_INFO,
    TYPE_RECEIVE_SCRATCH,  // 20
    TYPE_SEND_SCRATCH,
    TYPE_DELETE_SCRATCH_FILE,
    TYPE_RECEIVE_BDS_START,
    TYPE_RECEIVE_BDS,
    TYPE_RECEIVE_BDS_END,
    TYPE_SEND_BDS_START,
    TYPE_SEND_BDS,
    TYPE_SEND_BDS_END,
    TYPE_DELETE_BDS_FILE,

    TYPE_IC_SAVE_CALIBRATION_DATA,  // 30
    TYPE_IC_LOAD_CALIBRATION_DATA,
    TYPE_SEND_CALLBACK_FUNCTION,

    TYPE_RECEIVE_CALIBRATION_BKG_IMAGE,

    TYPE_SEND_HOST_DEVICE_INFO,
    TYPE_RECEIVE_INI_CONFIG,

    TYPE_SEND_RESET_IMAGE_FRAME_COUNT,
    TYPE_RECEIVE_IMAGE_OBJECT_ARRAY,
    TYPE_SEND_CA_TIME,
    TYPE_RECEIVE_ENCRY_IMAGE,
    TYPE_DELETE_ENCRY_IMAGE,
    TYPE_RECEIVE_TEMPL_INFO,
    TYPE_SEND_ENCRY_IMAGE_START,
    TYPE_SEND_ENCRY_IMAGE,
    TYPE_SEND_ENCRY_IMAGE_END,
    TYPE_SEND_INI_CONFIG_PATH = 50,
    TYPE_RECEIVE_SENSOR_BUF,
    TYPE_RECEIVE_RAND_DATA,
    TYPE_RECEIVE_DEBASE,
    TYPE_SEND_DEBASE,
    TYPE_DELETE_DEBASE,
    TYPE_COPY_CALI_DATA,
    TYPE_SEND_UPGRADE,
    TYPE_RECEIVE_CONVICT_ID,
    TYPE_RECEIVE_TOTAL_EXP_TIME,
    TYPE_SEND_DB_INI_CONFIG,
    TYPE_REMOVE_INI_FILE,
    TYPE_RECEIVE_SENSOR_INT_PARAM = 62,
    TYPE_EVTOOL_RECEIVE_MATCH_SCORE,
    TYPE_SEND_USERID,
    TYPE_SEND_BACKUP_CALIBRATION_DATA,
} data_transfer_type_t;

typedef enum { DUPLICATE_TEST, INCREMENTAL_TEST } transporter_test_type_t;

typedef struct {
    unsigned int fingerid;
    int header_len;
    unsigned int checksun;
} encry_image_header_t;
typedef struct {
    unsigned int fingerid;
    int max_enroll_count;
    int raw_image_size;
} templ_info_t;

typedef struct {
    int32_t type;
    int32_t step;
    int32_t in_data_size;
} transporter_test_in_head_t;

typedef struct {
    int32_t temperature_x10;
    int32_t reserve[3];
} host_device_info_t;

typedef struct {
    int32_t type;
    int32_t in_data_checksum;
    int32_t out_data_checksum;
    int32_t out_data_size;
} transporter_test_out_head_t;

typedef struct {
    unsigned int status;
    unsigned int percentage;
} cmd_enrollresult_t;

typedef struct {
    unsigned int status;
} cmd_imagestatus_t;

typedef struct {
    uint32_t matched_id;
    uint32_t status;
    uint32_t is_tmpl_update;
    int32_t ext_feat_quality;
    uint32_t x_len;
    int32_t quality_score;
    int32_t match_score;
} cmd_identifyresult_t;
#define IDENTIFYRESULT_X_DATA_OFFSET \
    (7 * sizeof(uint32_t))  // 5 :[matched_id,status,is_tmpl_update,ext_feat_quality,x_len]

#define SUPPORT_MAX_ENROLL_COUNT 5
typedef struct {
    uint32_t ids[SUPPORT_MAX_ENROLL_COUNT];
    uint32_t count;
} cmd_fingerprint_ids_t;

typedef enum EX_CMD {
    /* Initialize */
    EX_CMD_INIT_SDK = 0,
    EX_CMD_INIT_ALGO,
    EX_CMD_INIT_SENSOR,
    EX_CMD_UNINIT_SDK,
    EX_CMD_UNINIT_ALGO,
    EX_CMD_UNINIT_SENSOR,
    /* Common */
    EX_CMD_CALIBRATION,  // 6
    EX_CMD_DEL_CALIBRATION,
    EX_CMD_GET_IMAGE,
    EX_CMD_GET_ENROLLED_COUNT,
    /* Enrollment */
    EX_CMD_ENROLL_IDENTIFY,  // deprecated  10
    EX_CMD_ENROLL_INIT,
    EX_CMD_ENROLL,
    EX_CMD_ENROLL_UNINIT,
    EX_CMD_ENROLL_SAVE_TEMPLATE,
    /* Identify */
    EX_CMD_IDENTIFY_START,  // 15
    EX_CMD_IDENTIFY,
    EX_CMD_IDENTIFY_FINISH,
    EX_CMD_IDENTIFY_TMPL_UPDATE,
    EX_CMD_IDENTIFY_TMPL_SAVE,

    EX_CMD_REMOVE,  //20
    EX_CMD_GET_FP_IDS,
    EX_CMD_SET_WORK_MODE,
    EX_CMD_SET_ACTIVE_USER,
    EX_CMD_SET_DATA_PATH,
    EX_CMD_GET_DEV_HANDLE,
    EX_CMD_RE_CALIBRATION,
    EX_CMD_WRITE_REGISTER,
    EX_CMD_DETECT_SENSOR_TYPE,
    EX_CMD_CHECK_SID,
    EX_CMD_CHECK_AUTH_TOKEN,
    EX_CMD_GET_AUTH_ID,

    EX_CMD_SET_DATA,  // 33
    EX_CMD_GET_DATA,
    EX_CMD_SENSOR_TEST,
    EX_CMD_EXTRA,
    EX_CMD_CHECK_FINGER_LOST,
    EX_CMD_ENROLL_MERGE,
    EX_CMD_SET_FINGER_STATE,
    EX_CMD_OPEN_SPI,
    EX_CMD_CLOSE_SPI,
    EX_CMD_PRINT_MEMORY,
    EX_CMD_GET_RAW_IMAGE,
    EX_CMD_GET_IMAGE_IPP,
    EX_CMD_INIT_SENSOR_HW = 60,
    EX_CMD_UNINIT_SENSOR_HW,
    EX_CMD_IS_FINGER_OFF,
    EX_CMD_UPGRADE_TEMPLATE,
} EX_CMD_T;

typedef enum {
    FP_MMI_TEST_NONE = 0,
    FP_MMI_AUTO_TEST = 1,  // for MMI1 and MMI2
    FP_MMI_TYPE_INTERRUPT_TEST = 2,
    FP_MMI_FAKE_FINGER_TEST = 3,  // juse for MMI1
    FP_MMI_SNR_SINGAL_IMAGE_TEST = 4,
    FP_MMI_SNR_WHITE_IMAGE_TEST = 5,
    FP_MMI_BUBBLE_TEST = 6,
    FP_MMI_SN_TEST = 7,

    FP_MMI_DIRTYDOTS_TEST = 100,
    FP_MMI_READ_REV_TEST = 101,
    FP_MMI_REGISTER_RW_TEST = 102,
    FP_MMI_REGISTER_RECOVERY = 103,
    FP_MMI_FOD_TEST = 104,
    FP_MMI_GET_FINGER_IMAGE = 105,

    FP_READ_REG = 200,
    FP_WRITE_REG = 201,
    FP_CAPTURE_IMG = 202,
    FP_MMI_SET_CROP_INFO = 203,
    FP_MMI_GET_NVM_UID = 204,
    FP_CAPTURE_IMG_AE = 205,
    FP_OPLUS_QTY = 206,

    FP_INLINE_SENSOR_CALIBRATE = 210,
    FP_INLINE_SENSOR_GET_CALI_IMAGE = 211,
    FP_INLINE_SENSOR_GET_IMAGE = 212,

    FP_INLINE_7XX_CASE_INIT = 700,
    FP_INLINE_7XX_NORMALSCAN = 701,
    FP_INLINE_7XX_SNR_INIT = 702,
    FP_INLINE_7XX_SNR_WKBOX_ON = 703,
    FP_INLINE_7XX_SNR_BKBOX_ON = 704,
    FP_INLINE_7XX_SNR_CHART_ON = 705,
    FP_INLINE_7XX_SNR_CHART_ON_WITHOUT_SAVE_EFS_SCRIPT_ID = 706,
    FP_INLINE_7XX_RESET = 707,
    FP_INLINE_7XX_GET_IMAGE = 708,
    FP_INLINE_7XX_SNR_GET_DATA = 709,
    FP_INLINE_7XX_TEST_MFACTOR = 710,
    FP_INLINE_7XX_NORMAL_GET_IMAGE = 711,
    FP_INLINE_7XX_SNR_GET_ALL_IMAGE_16_BIT = 712,
    FP_INLINE_7XX_SNR_GET_ALL_IMAGE_16_BIT_INFO = 713,
    FP_INLINE_7XX_SNR_GET_ALL_IMAGE_8_BIT = 714,
    FP_INLINE_7XX_SNR_GET_ALL_IMAGE_8_BIT_INFO = 715,
    FP_INLINE_7XX_SNR_SAVE_LOG_ENABLE_EXTRA_INFO = 716,
    FP_INLINE_7XX_FLASH_TEST = 717,
    FP_INLINE_7XX_GET_UUID = 718,
    FP_INLINE_7XX_SNR_BKBOX_ON_2 = 719,
    FP_INLINE_GET_SENSOR_ID = 721,
    FP_INLINE_GET_ALGO_VER = 722,
    FP_INLINE_7XX_GET_MODULE_ID = 723,
    FP_INLINE_7XX_CALI_SPI_TEST = 790,
    FP_INLINE_7XX_CASE_UNINIT = 799,
} fp_mmi_test_type;

#define ENROLL_METHOD_BIT 0x000F
#define ENROLL_METHOD_TOUCH 0x0001

typedef struct {
    int enroll_method;
    int enroll_max_count;
    int switch_horizontal;
    int switch_vertical;
    int switch_x_y;
    int enroll_select_threshold;
    int enroll_too_fast_level;
    int enroll_redundant_image_policy;
    int enroll_too_fast_rollback;
    int enroll_redundant_level;
    int capture_delay_enroll_enable;
    int capture_delay_enroll_start_progress;
    int enroll_extra_1st_enable;
    int enroll_extra_1st_before_progress;
    int enroll_redundant_method;
} enroll_config_t;

typedef struct {
    int enable_learning_update;
    int enable_try_match;
    int append_img_into_template;
    int enable_delay_learning;
    int skip_identiy_ext_failure;
    int flow_trymatch_count;
    int ext_feat_quality_trymatch_th;
    int ext_feat_quality_lqmatch_th;  // G2
    int enable_sratch_mask;
} verify_config_t;

typedef struct {
    int sensing_mode;
    int sensor_gain;
    int enable_gain2;
    int enable_boost_mode;
} sensor_config_t;

typedef struct {
    int burst_mode_img_count;
} finger_detect_config_t;

typedef struct {
    int enable_cut_img;
    int algo_sensor_type;
    int crop_width;
    int crop_height;
} cut_img_config_t;

typedef struct {
    enroll_config_t enroll;
    verify_config_t verify;
    sensor_config_t sensor;
    finger_detect_config_t finger_detect;
    cut_img_config_t cut_img;
} rbs_config_t;

#define MMI_TEST_SUCCESS 1
#define MMI_TEST_FAIL 0

#define ENROLL_OPTION_NORMAL 0
#define ENROLL_OPTION_FINGERON 1
#define ENROLL_OPTION_MERGE 2

// For Optical flow
#define ENROLL_OPTION_REJECT_RETRY 3
#define ENROLL_OPTION_CLEAR_IMAGES 4
#define ENROLL_OPTION_ENROLL_THE_FIRST 5
#define ENROLL_OPTION_STOP_DQE 6

struct EnrollConf {
    int enroll_mode;
    int swipe_mode;
    int swipe_swim_count;
};

#define EGIS_TRANSFER_FRAMES_PER_TIME 10

typedef enum image_type {
    TRANSFER_ENROLL_IMAGE,
    TRANSFER_VERIFY_IMAGE_OBSOLETE,
    TRANSFER_LIVE_IMAGE_OBSOLETE,
    TRANSFER_LIVE_IMAGE,
    TRANSFER_VERIFY_IMAGE_V2,
    TRANSFER_ENROLL_RAW,
    TRANSFER_VERIFY_RAW,
} transfer_image_type_t;

#define TRANSFER_MORE_NONE 0
#define TRANSFER_MORE_TRUE 1
#define TRANSFER_MORE_NEXT_RAW 16

typedef struct {
    int width;
    int height;
    int bpp;
} image_format_t;

typedef enum reset_mode {
    FRAMES_RESET_NEVER,
    FRAMES_RESET_AUTO,
    FRAMES_RESET_ALWAYS
} frames_reset_mode_t;

typedef struct {
    uint16_t image_type;
    uint16_t image_index_start;
    uint16_t image_count_request;
    frames_reset_mode_t reset_mode;
} receive_images_in_t;

typedef struct {
    int try_count;
    int match_score;
    int save_index;
    int extract_qty;
} identify_info_t;

typedef struct {
    uint16_t image_type;
    uint16_t image_index_end;
    uint16_t image_count_included;
    uint16_t has_more_image;
    image_format_t format;
    identify_info_t identify_info;
} receive_images_out_t;

typedef struct {
    int time_interval;
    int wait_time;
    int trigger_type;
} trigger_info_t;

typedef struct {
    int isFinger;
    int percentage;
    int corner_count;
    int img_qty;
    int cover_count;
    int img_level;
    int bsd2;
} param_imageprocessing_t;

typedef struct {
    int isFinger;
    int qty;
    int size;
    int percentage;
    int img_level;
    int gap;
    int qm_score;
    int dx;
    int dy;
} param_imageqmlib_t;

typedef struct {
    int max;
    int min;
    int mean;
} param_image_statistics_data_t;

typedef struct {
    int exp_time_x10;
    int hw_integrate_count;
    int bkg_cx;
    int bkg_cy;
    int reserved1;
    int reserved2;
} param_cali_image_data_t;

#define LIVIMG_IMAGETYPE_NONE 0
#define LIVIMG_IMAGETYPE_FPIMAGELITE 1
#define LIVING_IMAGETYPE_QMEXTRACT 2
#define LIVING_IMAGETYPE_STATISTICSDATA 3
#define LIVING_IMAGETYPE_CALI_IMAGE 4

#define LIVIMG_IMAGE_VERSION 0xA1
#define LIVING_IMAGETYPE_OPT_QUALITY 5

#define MAX_TIME_LENGTH 50

typedef struct {
    int partial_score;
    int qty_score;
    int fake_score;
    int ext_qty_score;
} param_optical_image_quality_t;

typedef struct _cmd_test_result_t {
    union {
        float data[64];
        float test_result_data[64];
    };
} cmd_test_result_t;

typedef struct __optical_cali_result_t {
    int32_t cmd_id;
    int32_t result_length;
    cmd_test_result_t test_result;
} optical_cali_cmd_result_t;

typedef optical_cali_cmd_result_t optical_cali_result_t;

typedef struct _engineer_info_t_ {
    int32_t key;
    int32_t value;
} engineer_info_t;

typedef enum {
    ANC_CMD_UNDEF,
    ANC_CMD_ENROLL,
    ANC_CMD_EXTRACT,
    ANC_CMD_VERIFY,
    ANC_CMD_NUM
} Anc_algo_cmd_type_t;

typedef struct AncAlgoFingerInfo {
    Anc_algo_cmd_type_t type;  // 0: no use, 1: enroll 2: extract, 3:verify
    uint32_t cmd_status;
    // verify data
    uint32_t enrolled_template_count;
    uint32_t matched_template_idx;
    uint32_t matched_feature_idx;
    uint32_t is_studied;
    int compare_final_score;
    int finger_glb_score;
    int area_ratio;
    int cur_rot_angle;
    int classify_score;
    int align_num;
    //extract & enroll
    int finger_quality_score;
    int finger_live_score;
    int finger_light_score;
    int finger_status_score;
    int finger_strange_score;
    int temperature;
    int in_bad_qty_img;
    uint32_t retry_count;
    uint32_t source_id;
    uint32_t img_variance;
    uint32_t img_contrast;
    uint32_t seg_feat_mean;
    uint32_t finger_seg_score;
    uint32_t matched_user_id;
    uint32_t matched_finger_id;
    int mat_s;
    int mat_d;
    int mat_c;
    int mat_cs;
    int compress_cls_score;
    int compress_tnum_score;
    int compress_area_score;
    int compress_island_score;
    uint32_t tnum;
    uint32_t cache_tnum;
} AncAlgoFingerInfo_t;

typedef struct BigDataInfo {
    AncAlgoFingerInfo_t ancAlgoFingerInfo;
    int user_info;
    unsigned char time[MAX_TIME_LENGTH];
} BigDataInfo_t;

typedef enum operate_type{
    DO_ENROLL,
    DO_VERIFY,
    DO_INLINE,
    DO_OTHER,
}operate_type_t;

#endif
