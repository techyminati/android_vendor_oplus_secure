#ifndef _CUSTOMIZEDDELMARCOMMON_H_
#define _CUSTOMIZEDDELMARCOMMON_H_
#include "MsgBus.h"
#include "gf_event.h"
#include "gf_delmar_types.h"

#define CMD_TEST_SZ_TEST_BASE   0X600

#define BAD_PONIT_NUM        (350)
#define CLUSTER_NUM        (100)
#define PIXEL_OF_LARGEST_BAD_CLUSTER        (100)
#define LIGHT_HDEAD_LINE_NUM        (20)
#define LIGHT_VDEAD_LINE_NUM        (20)
#define MAX_HOT_CONNECTED_NUM        (100)
#define LOW_CORR_PITCH_LPF        (125)
#define MIN_VALID_AREA        (985)
#define MIN_CHART_DIRECTION        (80)
#define MAX_CHART_DIRECTION        (100)
#define AA_DARK_DIFF        (5)
#define MIN_ANGEL        (-2)
#define MAX_ANGEL        (2)
#define MIN_LIGHT_HIGH_MEAN        (1800)
#define MAX_LIGHT_HIGH_MEAN        (3000)
#define MIN_DIFF_FLESH_HM        (450)    // gap of Brightness L3 and Brightness L2 for fresh rubber
#define MIN_DIFF_FLESH_ML        (200)    // gap of Brightness L2 and Brightness L1 for fresh rubber
#define MIN_DIFF_BLACK_HM        (200)    // gap of Brightness L3 and Brightness L2 for black rubber
#define MIN_DIFF_BLACK_ML        (100)    // gap of Brightness L2 and Brightness L1 for black rubber
#define MAX_DIFF_OFFSET        (900)
#define DARK_TNOISE        (4)    // before LPF
#define LIGHT_TNOISE        (15)    // before LPF
#define DARK_SNOISE        (12)    // before LPF
#define LIGHT_SNOISE        (150)    // before LPF
#define FLAT_SNOISE_LPF        (3)    // after LPF
#define SIGNAL_LPF        (25)    // after LPF
#define SSNR_LPF        (10)    // after LPF
#define SHARPNESS_LPF        (0.3)    // after LPF

typedef  enum GF_BIG_DATA_MSG {
    MSG_BIG_DATA_UI_READY = goodix::MsgBus::MSG_MAX + 1000,
    MSG_BIG_DATA_GET_FEAURE_TIME,
    MSG_BIG_DATA_ENROLL_AUTH_END,
    MSG_BIG_DATA_FINGER_DOWN,
    MSG_BIG_DATA_FINGER_UP,
    MSG_BIG_DATA_DUMP_KPI,
}gf_big_data_msg_t;

typedef enum GF_ONEPLUS_NETLINK_EVENT {
    EVENT_UI_READY = 1<<4,
    EVENT_UI_DISAPPEAR = 1<<5,
}gf_oneplus_netlink_event;

typedef enum {
    CMD_TEST_SZ_FINGER_DOWN = CMD_TEST_SZ_TEST_BASE,
    CMD_TEST_SZ_FINGER_UP,
    CMD_TEST_SZ_ENROLL,
    CMD_TEST_SZ_FIND_SENSOR,
    CMD_TEST_SZ_FUSION_PREVIEW,
    CMD_TEST_SZ_UNTRUSTED_ENROLL,
    CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE,
    CMD_TEST_SZ_DELETE_UNTRUSTED_ENROLLED_FINGER,
    CMD_TEST_SZ_RAWDATA_PREVIEW,
    CMD_TEST_SZ_LDC_CALIBRATE,
    CMD_TEST_SZ_ENROLL_TEMPLATE_COUNT,
    CMD_TEST_SZ_UPDATE_CAPTURE_PARM,
    CMD_TEST_SZ_CANCEL,
    CMD_TEST_SZ_GET_CONFIG,
    CMD_TEST_SZ_GET_VERSION,
    CMD_TEST_SZ_K_B_CALIBRATION,
    CMD_TEST_SZ_SET_GROUP_ID,
    CMD_TEST_SZ_UPDATE_CFG,
    CMD_TEST_SZ_UPDATE_FW,
    CMD_TEST_SZ_UNTRUSTED_ENUMERATE,
    CMD_TEST_SZ_FT_CAPTURE_DARK_BASE = CMD_TEST_SZ_TEST_BASE + 20,  // 1556
    CMD_TEST_SZ_FT_CAPTURE_H_DARK,
    CMD_TEST_SZ_FT_CAPTURE_L_DARK,
    CMD_TEST_SZ_FT_CAPTURE_H_FLESH,
    CMD_TEST_SZ_FT_CAPTURE_L_FLESH,
    CMD_TEST_SZ_FT_CAPTURE_CHART,
    CMD_TEST_SZ_FT_CAPTURE_CHECKBOX,
    CMD_TEST_SZ_FT_CAPTURE_LOCATION_IMAGE,
    CMD_TEST_SZ_FT_FACTORY_PERFORMANCE,
    CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION,
    CMD_TEST_SZ_FT_STOP_EXPO_AUTO_CALIBRATION,
    CMD_TEST_SZ_FT_RESET,
    CMD_TEST_SZ_FT_SPI_RST_INT,
    CMD_TEST_SZ_FT_SPI,
    CMD_TEST_SZ_FT_INIT,
    CMD_TEST_SZ_FT_EXIT,
    CMD_TEST_SZ_FT_CALIBRATE,
    CMD_TEST_SZ_FT_MT_CHECK,
    CMD_TEST_SZ_FT_KPI,
    CMD_TEST_SZ_FT_FACTORY_CAPTURE_IMAGE_MANUAL,
    CMD_TEST_SZ_DUMP_TEMPLATE = CMD_TEST_SZ_TEST_BASE + 60,
    CMD_TEST_SZ_SET_HBM_MODE,
    CMD_TEST_SZ_CLOSE_HBM_MODE,
    CMD_TEST_SZ_SET_HIGH_BRIGHTNESS,
    CMD_TEST_SZ_SET_LOW_BRIGHTNESS,
    CMD_TEST_SZ_SET_DUMP_ENABLE_FLAG,
    CMD_TEST_SZ_FACTORY_TEST_GET_MT_INFO,
    CMD_TEST_SZ_SET_MAX_BRIGHTNESS,
    CMD_TEST_SZ_LOCAL_AREA_SAMPLE,
    CMD_TEST_SZ_ENABLE_POWER,
    CMD_TEST_SZ_DISABLE_POWER,
    CMD_TEST_SZ_UI_READY,
    CMD_TEST_SZ_UI_DISAPPER,
    CMD_TEST_SZ_ENABLE_PAY_ENVIRONMENT_LEVEL,
    CMD_TEST_SZ_DISABLE_PAY_ENVIRONMENT_LEVEL,
    CMD_TEST_SZ_FT_CAPTURE_M_FLESH,
    CMD_TEST_SZ_FT_CAPTURE_M_DARK,
    CMD_TEST_SZ_FT_CAPTURE_CIRCLEDATA,
    CMD_TEST_SZ_FACTORY_TEST_GET_RELIABILTY_INFO,  // 1614
    CMD_TEST_SZ_FT_CAPTURE_FLESH_CIRCLEDATA,
    CMD_TEST_SZ_FT_CAPTURE_CHECKBOX_CAPTURE = CMD_TEST_SZ_TEST_BASE + 100,
    CMD_TEST_SZ_FT_CAPTURE_CHART_CAPTURE,
    CMD_TEST_SZ_FT_SAMPLE_CALIBRATE_CHART = CMD_TEST_SZ_TEST_BASE + 150,
    CMD_TEST_SZ_FT_SAMPLE_CALIBRATE,
    CMD_TEST_SZ_FT_SIMPLE_CALI_INIT,  // simple cali no test
    CMD_TEST_SZ_FT_SIMPLE_CALI_EXIT,
    CMD_TEST_SZ_FT_SIMPLE_CALI_BASE,
    CMD_TEST_SZ_FT_SIMPLE_CALI_AUTO_CALIBRATION,
    CMD_TEST_SZ_FT_SIMPLE_STOP_CALI_AUTO_CALIBRATION,
    CMD_TEST_SZ_FT_SIMPLE_CALI_PERFORMANCE,
    CMD_TEST_SZ_FT_SIMPLE_CALI_SCREEN_CHART,
    CMD_TEST_SZ_FT_SIMPLE_CANCLE,
    CMD_TEST_PRODUCT_CMD_MAX = CMD_TEST_SZ_TEST_BASE + 1000,
} CUSTOMIZED_DELMAR_PRODUCT_TEST_CMD_ID;
#endif  // _CUSTOMIZEDDELMARCOMMON_H_
