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
	POWEROFF_MODE,
	NAVI_DETECT_MODE
} sensor_work_mode_t;

typedef enum opt_type {
	TYPE_SEND_CALIBRATION_DATA,
	TYPE_SEND_TEMPLATE,
	TYPE_RECEIVE_CALIBRATION_DATA,
	TYPE_RECEIVE_TEMPLATE,
	TYPE_RECEIVE_IMAGE,
	TYPE_RECEIVE_MULTIPLE_IMAGE,
	TYPE_DELETE_TEMPLATE,
	TYPE_TEST_TRANSPORTER,
	TYPE_SEND_INI_CONFIG,
	TYPE_DESTROY_INI_CONFIG,
	TYPE_RECEIVE_LIVE_IMAGE,
	TYPE_RECEIVE_TA_VERSION,
	TYPE_RECEIVE_ALGO_VERSION,
	TYPE_RECEIVE_IP_VERSION,

	TYPE_IC_SAVE_CALIBRATION_DATA,
	TYPE_IC_LOAD_CALIBRATION_DATA,
	TYPE_SEND_CALLBACK_FUNCTION,

	TYPE_RECEIVE_CALIBRATION_BKG_IMAGE,

	TYPE_SEND_HOST_DEVICE_INFO,
	TYPE_RECEIVE_INI_CONFIG,

	TYPE_SEND_RESET_IMAGE_FRAME_COUNT,
	TYPE_RECEIVE_IMAGE_OBJECT_ARRAY,

	TYPE_SEND_INI_CONFIG_PATH = 50,
	TYPE_RECEIVE_SENSOR_BUF,
	TYPE_EVTOOL_RECEIVE_MATCH_SCORE,
	TYPE_REMOVE_INI_FILE,
        TYPE_RECEIVE_SENSOR_INT_PARAM,
        TYPE_SEND_DB_INI_CONFIG,
        TYPE_SEND_CA_TIME = 60,
} data_transfer_type_t;

typedef enum {
	DUPLICATE_TEST,
	INCREMENTAL_TEST
} transporter_test_type_t;

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
	int dx;
	int dy;
	int swipe_dir;
	int similarity_score;
	int mergemap_hw_width;
	int mergemap_hw_height;
} algo_swipe_info_t;

typedef struct {
	unsigned int status;
	unsigned int percentage;
	algo_swipe_info_t swipe_info;
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
} cmd_identifyresult_t;
#define IDENTIFYRESULT_X_DATA_OFFSET (5 * sizeof(uint32_t))  //5 :[matched_id,status,is_tmpl_update,ext_feat_quality,x_len]

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
	EX_CMD_ENROLL_IDENTIFY,  //deprecated  10
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

	EX_CMD_NAVIGATION,  // 20
	EX_CMD_REMOVE,
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
	EX_CMD_NAVI_CONTROL,
	EX_CMD_OPEN_SPI,
	EX_CMD_CLOSE_SPI,
        EX_CMD_INIT_SENSOR_HW = 60,
        EX_CMD_UNINIT_SENSOR_HW,
} EX_CMD_T;

typedef enum NAVI_CONTROL_CMD {
	NAVI_CMD_START,
	NAVI_CMD_STOP,
} NAVI_CONTROL_CMD_T;

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
	FP_INLINE_7XX_SNR_GET_IMAGE = 712,
	FP_INLINE_7XX_BP_GET_IMAGE = 713,
	FP_INLINE_7XX_SNRCT_GET_IMAGE = 714,
	FP_INLINE_7XX_EXPSTEP1_GET_IMAGE = 715,
	FP_INLINE_7XX_EXPSTEP2_GET_IMAGE = 716,
	FP_INLINE_7XX_FLASH_TEST = 717,
        FP_INLINE_7XX_SNR_WKBOX_ON_IMAGE = 718,
        FP_INLINE_7XX_SNR_BKBOX_ON_2 = 719,
	FP_INLINE_7XX_FLASH_TEST_2 = 720,
        /* OPLUS Custom start */
	FP_INLINE_7XX_CALI_SPI_TEST = 790,
	/* OPLUS Custom end */
	FP_INLINE_7XX_CASE_UNINIT = 799,

} fp_mmi_test_type;

#define ENROLL_METHOD_BIT 0x000F
#define ENROLL_METHOD_TOUCH 0x0001
#define ENROLL_METHOD_SWIPE 0x0002
#define ENROLL_METHOD_PAINT 0x0004

#define SWIPE_DIRECTION_X 0x0010
#define SWIPE_DIRECTION_Y 0x0020
#define SWIPE_DIRECTION_AUTO 0x0040

#define DEFAULT_SWIPE_COUNT_X 1
#define DEFAULT_SWIPE_COUNT_Y 1

#define MERGE_MAP_WIDTH 160
#define MERGE_MAP_HEIGHT 256
#define BACKGROUND_HEIGHT_SWIPE 320
#define BACKGROUND_HEIGHT_PAINT 256

typedef struct {
	int enroll_method;
	int swipe_dir;
	int enroll_max_count;
	int swipe_count_x;
	int swipe_count_y;
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
	int ext_feat_quality_lqmatch_th; //G2
        int enable_check_residual;
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
	int enable_too_fast;
	int too_fast_threshold;
} swipe_config_t;

typedef struct {
	int navi_mode;
	int change_x_y;
	int change_up_down;
	int change_left_right;
} navigation_config_t;

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
	swipe_config_t swipe;
	navigation_config_t navi;
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

typedef enum NAVI_MODE {
	NAVI_MODE_NORMAL = 1,
	NAVI_MODE_PAINT,
	NAVI_MODE_OLD,
} NAVI_MODE_T;

struct NaviStatusInfo {
	int navi_score;
	int navi_dx;
	int navi_dy;
	int is_finger;
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
} identify_info_t;

typedef struct {
	uint16_t image_type;
	uint16_t image_index_end;
	uint16_t image_count_included;
	uint16_t has_more_image;
	image_format_t format;
	identify_info_t identify_info;
} receive_images_out_t;

typedef struct{
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
}param_imageprocessing_t;

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
}param_imageqmlib_t;

typedef struct {
	int max;
	int min;
	int mean;
}param_image_statistics_data_t;

typedef struct {
	int exp_time_x10;
	int hw_integrate_count;
	int bkg_cx;
	int bkg_cy;
	int reserved1;
	int reserved2;
}param_cali_image_data_t;

typedef struct {
	int partial_score;
	int qty_score;
	int fake_score;
	int ext_qty_score;
}param_optical_image_quality_t;

#define LIVIMG_IMAGE_VERSION 0xA1

#define LIVIMG_IMAGETYPE_NONE 0
#define LIVIMG_IMAGETYPE_FPIMAGELITE 1
#define LIVING_IMAGETYPE_QMEXTRACT 2
#define LIVING_IMAGETYPE_STATISTICSDATA 3
#define LIVING_IMAGETYPE_CALI_IMAGE 4
#define LIVING_IMAGETYPE_OPT_QUALITY 5

typedef struct {						//if struct changed ,modify LIVIMG_IMAGE_VERSION please
	uint32_t live_image_header_ver;
	int framecount;
	int	img_width;
	int img_height;
	int raw_width;
	int raw_height;
	int bpp;
	int live_image_type;
	union image_par{
		param_imageqmlib_t qm_parameter;
		param_imageprocessing_t process_parameter;
		param_image_statistics_data_t image_statistics_data;
		param_cali_image_data_t cali_image_data;
		param_optical_image_quality_t optical_quality_data;
	}image_par_t;
} liver_image_out_header_t;

typedef struct _cmd_test_result_t
{
	union {
	int32_t data[32];
	int32_t test_result_data[32];
	};
} cmd_test_result_t;

/* oplus fingerprint hal sensortest customize */

typedef struct __optical_cali_result_t
{
	int32_t cmd_id;
	int32_t result_length;
	cmd_test_result_t test_result;
} optical_cali_cmd_result_t;

typedef optical_cali_cmd_result_t \
	optical_cali_result_t;

typedef struct _engineer_info_t_
{
	int32_t key;
	int32_t value;
} engineer_info_t;

#endif
