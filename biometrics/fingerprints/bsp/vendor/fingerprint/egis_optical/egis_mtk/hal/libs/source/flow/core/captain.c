/************************************************************************************
 ** File: - captain.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2009-2019, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for egis optical fingerprint
 **
 ** Version: 1.0
 ** Date : 17:18:20,28/12/2019
 ** Author: zhouqijia@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  Qijia.Zhou 2019/12/28           set module info
 ************************************************************************************/

#include "captain.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <sys/system_properties.h>
#include "7xx_sensor_test_definition.h"
#include "common_definition.h"
#include "constant_def.h"
#include "core_config.h"
#include "device_int.h"
#include "egis_definition.h"
#include "egis_rbs_api.h"
#include "ex_define.h"
#include "extra_def.h"
#include "fp_definition.h"
#include "fps_normal.h"
#include "liver_image.h"
#include "object_def_image.h"
#include "op_manager.h"
#include "op_sensortest.h"
#include "opt_file.h"
#include "plat_file.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "plat_thread.h"
#include "plat_time.h"
#include "response_def.h"
#include "thread_manager.h"
#include "type_def.h"
#include "version.h"
#include "ini_definition.h"
#include "save_image.h"
#ifdef __TRUSTONIC__
#include "opt_file_trustonic.h"
#endif
#define LOG_TAG "RBS-CAPTAIN"
#include "vendor_custom.h"

#ifdef __SDK_SAVE_IMAGE_V2__
#include "save_image_v2.h"
#endif
#ifdef EGIS_DEBUG_MEMORY
#include "egis_mem_debug.h"
#endif

#define FINGERPRINT_CALIPROP "vendor.fingerprint.cali"
#define PROPERTY_FP_FACTORY_SDK_ALGO_VERSION "oplus.fingerprint.algo.version"

#define AUTH_TOKEN_LEN 256
#define FPS_CALI_ET7XX_BKG 7
#define FPS_SAVE_ET7XX_CALI_DATA 8
#define FPS_LOAD_ET7XX_CALI_DATA 9
#define FPS_REMOVE_ET7XX_CALI_DATA 10
#define FPS_CALI_UNINIT 12
#define FPS_REMOVE_ET7XX_BDS 13
static pthread_cond_t cv;
static mutex_handle_t lock;
static mutex_handle_t pwr_lock;

static BOOL g_fingeroff_learning_flag = TRUE;
int g_user_info_value = 0;

#define PROPERTY_FINGERPRINT_FACTORY "oplus.fingerprint.factory"
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.algo.version"

static fp_module_info_t module_info_list[] = {
    /* module, lens, module_info_string */
#ifdef FP_EGIS_OPTICAL_FA118
    {0, 0, "E_Quote"},
    {1, 0, "E_Ofilm"},
#else
    {16, 0, "E_Quote"},
    {5, 0, "E_Ofilm"},
#endif //FP_EGIS_OPTICAL_FA118
    {2, 0, "E_Truly"},
    {3, 0, "E_Primax"},
};

cache_info_t g_cache_info;
fingerprint_enroll_info_t g_enroll_info;
fingerprint_verify_info_t g_verify_info;
fingerprint_verify_info_t g_save_debug_image_verify_info;
enroll_config_t g_enroll_config;
BigDataInfo_t g_verify_bigDataInfo;
static cut_img_config_t g_crop_config;

static unsigned int g_has_enroll_count = 0;
static int g_enroll_percentage_tmp = 0;

static BOOL g_need_cancel = FALSE;
static BOOL g_hardware_ready = FALSE;
static BOOL g_finger_on_detected_finger_off = FALSE;

BOOL g_use_sysunlock_imagetool = FALSE;
int  g_verify_flow_flag = 0;
extern int g_verify_touch_up_flag;
int g_oparate_type = DO_OTHER;

int g_hdev = 0;
int g_temp_remaining = 0;
char g_user_path[MAX_PATH_LEN] = {0};
int g_enroll_timeout = TIMEOUT_WAIT_FOREVER;

#ifdef TEMPLATE_UPGRADE_FROM_SAVED_IMAGE
int g_is_upgrade = 0;
int g_is_flag = 0;
static thread_handle_t g_upgrade_test_handle = {0};
static semaphore_handle_t g_upgrade_sem;
#endif

const int TIMEOUT_FOR_ONE_TRY = 500;
event_callbck_t g_event_callback;

typedef struct {
    pwr_status_t pwr_status;
    BOOL is_fast_finger_on;
} power_management_t;

static power_management_t g_power_managemen = {
    .pwr_status = SENSOR_PWR_OFF,
    .is_fast_finger_on = false,
};

host_device_info_t g_host_device = {
    .temperature_x10 = 250,
};
void set_host_device_info(host_device_info_t device) {
    g_host_device = device;
}
host_device_info_t get_host_device_info(void) {
    return g_host_device;
}

static void get_module_info(unsigned char* module_type, unsigned char* lens_type) {
    unsigned char buf[11] = {0};
    int ret = FP_LIB_ERROR_GENERAL;
    int buf_len = sizeof(buf);

    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_GET_UUID, 0, 0, 0, buf, &buf_len);
    if (ret == FP_LIB_OK) {
        ex_log(LOG_DEBUG, "buf[0] = 0x%02x, buf[10] = 0x%02x", buf[0], buf[10]);
#ifdef FP_EGIS_OPTICAL_FA118
        *module_type = buf[10];
#else
        *module_type = buf[10] >> 3;
#endif //FP_EGIS_OPTICAL_FA118
        *lens_type = buf[0];
    }
    ex_log(LOG_DEBUG, "module_type = 0x%02x, lens_type = 0x%02x", *module_type, *lens_type);
}

static int do_sensortest(int cid, unsigned char* in_data, int in_data_size,
                         unsigned char* out_buffer, int* out_buffer_size);

void captain_cancel(BOOL cancel_flag);
static void get_ca_version(unsigned char* version, int* len);
static int enroll_percentage_to_remaining(int percentage);
static long getCurrentTime_ms(void);
void get_time_now(char* str_time);
long get_ca_time_ms(void);
int set_brightness(int level);

int do_power_on(pwr_status_t pwr);
void sprint_algoinfo(BigDataInfo_t* bigDataInfo);

#define SKIP_FIST_TOUCH_INTERVAL 1000
#define FINGER_ON_DURATION 150

extern char g_username[256];
extern char g_fingerid[256];
extern char g_case[256];
extern char g_test_case_id[256];
extern oplus_trigger_case_t g_trigger_case;

void setSpiState(BOOL spi_on, BOOL forced);
void notify(int event_id, int first_param, int second_param, unsigned char* data, int data_size);

static mutex_handle_t pwr_lock;
static BOOL g_need_pause = FALSE;
static BOOL g_screen_state = FALSE;
unsigned long g_first_auth;

static void auth_notify(int event_id, int first_param, int second_param, unsigned char* data,
                        int data_size) {
    if (event_id == EVENT_FINGER_TOUCH_DOWN && g_screen_state == 0) {
        if (plat_get_diff_time(g_first_auth) < SKIP_FIST_TOUCH_INTERVAL) {
            egislog_d("skip first touch");
            return;
        }
    }

    notify(event_id, first_param, second_param, data, data_size);
}

static int touch_enroll_remaining_no_percentage(int percentage) {
    ex_log(LOG_DEBUG, "%s enter! percentage = %d", __func__, percentage);

    if (g_enroll_percentage_tmp != percentage) {
        g_has_enroll_count++;
        g_enroll_percentage_tmp = percentage;
    }

    int remaining;
    if (percentage >= 100) {
        g_enroll_percentage_tmp = 0;
        g_has_enroll_count = 0;
        remaining = 0;
    } else {
        int total = g_enroll_config.enroll_max_count;
        remaining = total - g_has_enroll_count;
        ex_log(LOG_DEBUG, "%s has_enroll_count=%d, total=%d, remaining=%d", __func__,
               g_has_enroll_count, total, remaining);
        if (remaining < 1) {
            remaining = 1;
        }
        if (remaining > g_enroll_config.enroll_max_count) {
            remaining = g_enroll_config.enroll_max_count;
        }
    }

    return remaining;
}

void toDCSstring(int value, char * str) {
    if (str == NULL) {
        return;
    }

    memset(str, 0, strlen(str));
    sprintf(str, "%d", value);
    ex_log(LOG_DEBUG, "str =  %s,len %d value = %d ", str, (int)strlen(str), value);
    return;
}

void get_algorithm_version(unsigned char* algo_ver) {
    ex_log(LOG_DEBUG, "%s enter", __func__);
    unsigned char buf[32] = {0};
    int ret = FP_LIB_ERROR_GENERAL;
    int buf_len = sizeof(buf);
    ret = flow_inline_legacy_cmd(FP_INLINE_GET_ALGO_VER, 0, 0, 0, buf, &buf_len);
    if (ret == FP_LIB_OK) {
        ex_log(LOG_DEBUG, "algo version= %s", buf);
        memcpy(algo_ver, buf, sizeof(buf));
    }
}

int get_sensor_id() {
    ex_log(LOG_DEBUG, "%s enter", __func__);
    unsigned char buf[4] = {0};
    int ret = FP_LIB_ERROR_GENERAL;
    int buf_len = sizeof(buf);
    ret = flow_inline_legacy_cmd(FP_INLINE_GET_SENSOR_ID, 0, 0, 0, buf, &buf_len);
    if (ret == FP_LIB_OK) {
        ex_log(LOG_DEBUG, "buf[0] = 0x%02x-%d, buf[10] = 0x%02x-%d", buf[0], buf[0], buf[1],
               buf[1]);
        return (int)buf[0] * 100 + buf[1];
    }
    return -1;
}

static void get_dcs_msg(key_value_t* msg, int size) {
    ex_log(LOG_DEBUG, "%s enter", __func__);
    unsigned char module = 0;
    unsigned char lens = 0;
    int chip_ic;
    char algo_ver[32] = {0};
    memset(msg, 0, size);

    if (msg != NULL) {
        msg[0].key = "auth_result";
        toDCSstring(g_verify_info.is_passed, msg[0].value);
        msg[1].key = "fail_reason";
        if (g_verify_info.is_passed) {
            toDCSstring(FP_LIB_VERIFY_MATCHED, msg[1].value);
        } else {
            toDCSstring(g_verify_info.dcsmsg.fail_reason, msg[1].value);
        }

        msg[2].key = "quality_score";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.finger_quality_score, msg[2].value);

        msg[3].key = "match_score";
        toDCSstring(g_verify_info.dcsmsg.match_score, msg[3].value);

        msg[4].key = "signal_value";
        toDCSstring(0, msg[4].value);

        msg[5].key = "img_area";
        toDCSstring(0, msg[5].value);

        msg[6].key = "retry_times";
        toDCSstring(g_verify_info.dcsmsg.retry_times, msg[6].value);

        msg[7].key = "algo_version";
        get_algorithm_version((unsigned char*)algo_ver);
        memcpy(msg[7].value, algo_ver, 32);

        msg[8].key = "chip_ic";
        chip_ic = get_sensor_id();
        toDCSstring(chip_ic, msg[8].value);

        get_module_info(&module, &lens);
        msg[9].key = "module_type";
        toDCSstring(module, msg[9].value);

        msg[10].key = "lens_type";
        toDCSstring(lens, msg[10].value);

        msg[11].key = "dsp_available";
        toDCSstring(g_verify_info.dcsmsg.dsp_available, msg[11].value);  // not support

        msg[12].key = "device_id";
        toDCSstring(g_user_info_value, msg[12].value); // not support

        msg[13].key = "time";
        //get_time_now(msg[13].value);
        memcpy(msg[13].value, g_verify_bigDataInfo.time, sizeof(g_verify_bigDataInfo.time));

        msg[14].key = "IBQI";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.in_bad_qty_img, msg[14].value);

        msg[15].key = "ClsCode";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.compress_cls_score, msg[15].value);

        msg[16].key = "TnumScore";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.compress_tnum_score, msg[16].value);

        msg[17].key = "study";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.is_studied, msg[17].value);

        msg[18].key = "Matd";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.mat_d, msg[18].value);

        msg[19].key = "Flivescore";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.finger_live_score, msg[19].value);

        msg[20].key = "IsLandScore";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.compress_island_score, msg[20].value);

        msg[21].key = "FLightscore";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.finger_light_score, msg[21].value);

        msg[22].key = "MTid";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.matched_finger_id, msg[22].value);

        msg[23].key = "ETcount";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.enrolled_template_count, msg[23].value);

        msg[24].key = "AreaScore";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.compress_area_score, msg[24].value);

        msg[25].key = "Egtype";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.type, msg[25].value);

        msg[26].key = "ImgVar";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.img_variance, msg[26].value);

        msg[27].key = "ImgContr";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.img_contrast, msg[27].value);

        msg[28].key = "MatS";
        toDCSstring(g_verify_bigDataInfo.ancAlgoFingerInfo.mat_s, msg[28].value);

        for (int i = 0; i < (int)(size / sizeof(key_value_t)); i++) {
            ex_log(LOG_DEBUG, "%s = %s", msg[i].key, msg[i].value);
        }
    } else
        ex_log(LOG_DEBUG, "NULL pointer");
}

int do_sensor_aging_test() {
    int retval = FINGERPRINT_RES_SUCCESS;
    ex_log(LOG_DEBUG, "%s enter !!!", __func__);

    if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power on sensor fail");
        retval = FINGERPRINT_RES_FAILED;
        return -1;
    }
    while (!(check_cancelable())) {
        host_touch_set_hbm(1);
#if defined(__ET7XX__) && !defined(__ET0XX__)
        retval = do_7XX_spi_test(FP_INLINE_7XX_CALI_SPI_TEST);
        plat_sleep_time(500);
#endif
        cmd_test_result_t notify_result = {};
        notify_result.test_result_data[0] = 100;
        notify_result.test_result_data[1] = (retval == FP_LIB_OK) ? 0 : 1;
        // notify_result.test_result_data[1] = 0;
        host_touch_set_hbm(0);
        notify(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, 513, 2 * sizeof(uint32_t),
               (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
    }
    if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power off sensor fail");
        retval = FINGERPRINT_RES_FAILED;
        return -1;
    }

    return 0;
}

int do_image_qty_test() {
    int in_data[2] = {0};
    static int image_score = 99;
    int buffer_size = (MAX_IMAGE_BUFFER_SIZE * 2) + sizeof(liver_image_out_header_t);
    int retval = FINGERPRINT_RES_SUCCESS;
    liver_image_out_header_t* pheader;
    unsigned char* buffer = NULL;
    engineer_info_t info[3] = {0};

    g_oparate_type = DO_INLINE;

    memset(in_data, 0x00, sizeof(in_data));
    in_data[1] = 4;

    g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;

    ex_log(LOG_DEBUG, "%s enter !!! buffer_size=%d, sizeof(liver_image_out_header_t)=%d", __func__,
           buffer_size, (int)sizeof(liver_image_out_header_t));
    if (check_cancelable()) {
        return FINGERPRINT_RES_CANCEL;
    }
    retval = opm_set_work_mode(DETECT_MODE);
    if (check_cancelable()) {
        return FINGERPRINT_RES_CANCEL;
    }

    buffer = (unsigned char*)malloc(buffer_size);
    if (buffer == NULL) {
        ex_log(LOG_ERROR, "buffer FINGERPRINT_RES_ALLOC_FAILED fail");
        return FINGERPRINT_RES_ALLOC_FAILED;
    }

    if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power on sensor fail");
        retval = FINGERPRINT_RES_FAILED;
        if (buffer != NULL) {
            free(buffer);
            buffer = NULL;
        }
        return -1;
    }
    // while (!(check_cancelable()))
    //{
    host_touch_set_finger_reset();
    memset(buffer, 0x00, buffer_size);
    if (wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) {
        host_touch_set_hbm(1);
        ex_log(LOG_DEBUG, "%s enter has finger on sensor", __func__);
        if (check_cancelable()) {
            retval = FINGERPRINT_RES_CANCEL;
            // break;
        }
    }

    ex_log(LOG_DEBUG, "%s image test finger touch !!!", __func__);

    // notify(EVENT_FINGER_TOUCH, 0, 0, NULL, 0);

    if (check_cancelable()) {
        retval = FINGERPRINT_RES_CANCEL;
        // break;
    }

    ex_log(LOG_DEBUG, "%s SENSORTEST_IMAGE_QTY Start", __func__);
    retval = sensor_test_opation(SENSORTEST_IMAGE_QTY, g_hdev, (unsigned char*)in_data,
                                 sizeof(in_data), buffer, &buffer_size);
    ex_log(LOG_DEBUG, "%s SENSORTEST_IMAGE_QTY End", __func__);

    pheader = (liver_image_out_header_t*)buffer;
    image_score = pheader->image_par_t.process_parameter.img_qty;
    // image_score = 100;
    ex_log(LOG_DEBUG, "%s image_score=%d, %p", __func__, image_score, pheader);
    egislog_d("test [%d], %p", __LINE__, &(pheader->image_par_t.process_parameter.img_qty));
    ex_log(LOG_DEBUG, "%p [%d][%d]", in_data, in_data[0], in_data[1]);
    ex_log(LOG_DEBUG, "%p [%d][%d][%d][%d][%d]", buffer, buffer[0], buffer[1], buffer[2], buffer[3],
           buffer[4]);
    ex_log(LOG_DEBUG, "%p [%d][%d][%d][%d][%d]", buffer, buffer[5], buffer[6], buffer[7], buffer[8],
           buffer[9]);
    ex_log(LOG_DEBUG, "%p [%d][%d][%d][%d][%d]", buffer, buffer[10], buffer[11], buffer[12],
           buffer[13], buffer[14]);
    ex_log(LOG_DEBUG, "%p [%d][%d][%d][%d][%d]", buffer, buffer[15], buffer[16], buffer[17],
           buffer[18], buffer[19]);
    ex_log(LOG_DEBUG, "%p [%d][%d][%d][%d][%d]", buffer, buffer[20], buffer[21], buffer[22],
           buffer[23], buffer[24]);
    ex_log(LOG_DEBUG, "%p [%d][%d][%d][%d][%d]", buffer, buffer[56], buffer[57], buffer[58],
           buffer[59], buffer[60]);
    info[0].key = 0;
    info[0].value = 1;

    info[1].key = 1;
    info[1].value = image_score;
    info[2].key = 9;
    info[2].value = 1;

    host_touch_set_hbm(0);
    if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power off sensor fail");
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

EXIT:
    plat_sleep_time(50);
    notify(EVENT_SENSOR_OPTICAL_CALI_IMAGE_QTY, retval, image_score--, (unsigned char*)info,
           3 * sizeof(engineer_info_t));

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    return retval;
}

static int convert_notify(int* event_id, int* second_param) {
    if (*event_id == EVENT_ENROLL_OK) {
        int remaining = enroll_percentage_to_remaining(*second_param);
        g_temp_remaining = remaining;

#ifndef __OPLUS_ON_A50__
        const int ENROLL_KTM = 1;
        const int ENROLL_KTM_START = 12;
        *event_id = EVENT_ENROLL_REMAINING;

        *second_param = remaining;
#else
        if (*second_param == 58) {
            *event_id = 1006;
        } else if (*second_param > 58) {
            *second_param -= 4;
        }
#endif
    } else if (*event_id == EVENT_CUSTOM_ENROLL_OK_FINAL) { /* Report remaining after templates.*/
#ifndef __OPLUS_ON_A50__
        int remaining = enroll_percentage_to_remaining(*second_param);
        *event_id = EVENT_ENROLL_REMAINING;
        *second_param = 0;
#else
        *event_id = EVENT_ENROLL_OK;
        *second_param = 100;
#endif
        plat_sleep_time(*second_param * 2);
    }
    return FINGERPRINT_RES_SUCCESS;
}

thread_handle_t g_thread_power_on_handle = {0};
mutex_handle_t g_thread_power_on_lock;

void thread_power_on() {
    ex_log(LOG_DEBUG, "%s, begin", __func__);
    int ret = 999;
    if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_DEBUG, "%s, do power on sensor fail when touch_down", __func__);
    }
    ret = plat_mutex_unlock(g_thread_power_on_lock);
    ex_log(LOG_DEBUG, "%s, end, ret = %d", __func__, ret);
}

void notify(int event_id, int first_param, int second_param, unsigned char* data, int data_size) {
    int retval = FINGERPRINT_RES_SUCCESS;
    int ret = 999;
    if (NULL != g_event_callback) {
        int id = event_id, param2 = second_param;
#ifndef EGIS_DBG
        if (event_id == EVENT_RETURN_IMAGE || event_id == EVENT_RETURN_LIVE_IMAGE) {
            return;
        }
#endif
        if (event_id == EVENT_FINGER_TOUCH_DOWN &&
            g_oparate_type == DO_VERIFY) {
            // ex_log(LOG_DEBUG, "do power on when touch_down");
            ret = plat_mutex_trylock(g_thread_power_on_lock);
            ex_log(LOG_DEBUG, "try lock g_thread_power_on_lock , ret = %d", ret);
            if (ret == THREAD_RES_OK) {
                ex_log(LOG_DEBUG,
                    "create thread g_thread_power_on_lock , hlinux = %d",
                    (int)g_thread_power_on_handle.hlinux);
                g_thread_power_on_handle.hlinux = 0;
                plat_thread_create_ex(&g_thread_power_on_handle, thread_power_on, NULL);
            }
        }

        ex_log(LOG_DEBUG, "In event_id=%d, (%d)(%d)", event_id, first_param, second_param);
        retval = convert_notify(&id, &param2);
        if (retval == FINGERPRINT_RES_SUCCESS) {
            ex_log(LOG_DEBUG, "out event_id=%d, (%d)(%d)", id, first_param, param2);
            g_event_callback(id, first_param, param2, data, data_size);
        }
    }
}
static int create_ini_config(BOOL need_reinit, unsigned char* in_data, int in_data_size) {
    int retval = FINGERPRINT_RES_SUCCESS;

    retval = opt_send_data(TYPE_SEND_INI_CONFIG, in_data, in_data_size);
    if (retval != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "create_ini_config return %d", retval);
        return FINGERPRINT_RES_FAILED;
    }

    g_enroll_config.enroll_method =
        core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_METHOD, ENROLL_METHOD_TOUCH);
    g_enroll_config.enroll_too_fast_rollback =
        core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLLEX_TOO_FAST_ROLLBACK, 0);

    g_enroll_config.capture_delay_enroll_enable =
        core_config_get_int(INI_SECTION_ENROLL, KEY_CAPTURE_DELAY_ENABLE, 0);
    g_enroll_config.capture_delay_enroll_start_progress = core_config_get_int(
        INI_SECTION_ENROLL, KEY_CAPTURE_DELAY_START_PROGRESS, 75);  // last 5 of total 24, ~= 20%

    g_enroll_config.enroll_extra_1st_enable = core_config_get_int(
        INI_SECTION_ENROLL, KEY_ENROLL_EXTRA_THE_FIRST_ENABLE, INID_ENROLL_EXTRA_THE_FIRST_ENABLE);
    g_enroll_config.enroll_extra_1st_before_progress =
        core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_EXTRA_THE_FIRST_BEFORE_PROGRESS, 40);
    g_enroll_config.enroll_max_count =
        core_config_get_int(INI_SECTION_ENROLL, KEY_MAX_ENROLL_COUNT, INID_MAX_ENROLL_COUNT);
    cut_img_config_t crop_config = {0};
    crop_config.enable_cut_img = core_config_get_int(INI_SECTION_SENSOR, KEY_CUT_IMAGE, 0);
    crop_config.algo_sensor_type =
        core_config_get_int(INI_SECTION_SENSOR, KEY_ALGO_INIT_SENSOR_TYPE, 0);
    crop_config.crop_width = core_config_get_int(INI_SECTION_SENSOR, KEY_CUT_IMAGE_WIDTH, 0);
    crop_config.crop_height = core_config_get_int(INI_SECTION_SENSOR, KEY_CUT_IMAGE_HEIGHT, 0);

    if (mem_compare(&crop_config, &g_crop_config, sizeof(cut_img_config_t))) {
        if (!need_reinit) {
            opm_uninitialize_sensor();
            retval = opm_initialize_sensor();
            if (FINGERPRINT_RES_SUCCESS != retval) {
                return FINGERPRINT_RES_FAILED;
            }

            opm_uninitialize_algo();
            retval = opm_initialize_algo();
            if (FINGERPRINT_RES_SUCCESS != retval) {
                return FINGERPRINT_RES_FAILED;
            }
        }

        mem_move(&g_crop_config, &crop_config, sizeof(cut_img_config_t));
    }

    ex_log(LOG_DEBUG, "enroll_method = 0x%x", g_enroll_config.enroll_method);
    return retval;
}

static int destroy_ini_config(void) {
    return opt_send_data(TYPE_DESTROY_INI_CONFIG, NULL, 0);
}
static BOOL check_fingerprint_id_available(unsigned int fingerprint_id) {
    unsigned int index;

    for (index = 0; index < g_cache_info.fingerprint_ids_count; index++) {
        if (g_cache_info.fingerprint_ids[index] == fingerprint_id) {
            ex_log(LOG_ERROR,
                   "check_fingerprint_id_available return false, "
                   "the same fingerprint id alredy exist");
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL check_enrollment_space_available() {
    return g_cache_info.fingerprint_ids_count < __SINGLE_UPPER_LIMITS__ ? TRUE : FALSE;
}

int sync_user_cache(unsigned int user_id) {
    int retval;
    unsigned int index;
    fingerprint_ids_t fps;
#ifdef TEMPLATE_UPGRADE_FROM_SAVED_IMAGE
    ex_log(LOG_INFO, "sync_user_cache enter g_is_upgrade=%d,%d,%d", g_is_upgrade, user_id,
           g_cache_info.user_id);
#else
    ex_log(LOG_INFO, "sync_user_cache enter user_id%d,%d", user_id, g_cache_info.user_id);
#endif
    if (user_id == g_cache_info.user_id) {
        return FINGERPRINT_RES_SUCCESS;
    }

    fps.fingerprint_ids_count = 0;
    retval = opm_get_fingerprint_ids(user_id, &fps);
    if (EX_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "opm_get_fingerprint_ids return = %d", retval);
        return FINGERPRINT_RES_FAILED;
    }

    mem_move(g_cache_info.fingerprint_ids, fps.fingerprint_ids,
             fps.fingerprint_ids_count * sizeof(unsigned int));
    g_cache_info.fingerprint_ids_count = fps.fingerprint_ids_count;
    g_cache_info.user_id = user_id;

    ex_log(LOG_DEBUG, "fingerprint_ids_count= %d", fps.fingerprint_ids_count);

    if (fps.fingerprint_ids_count > 0) {
        g_cache_info.authenticator_id = 123;  // hard code magic number
        for (index = 0; index < fps.fingerprint_ids_count; index++) {
            g_cache_info.authenticator_id += fps.fingerprint_ids[index];
        }
    } else {
        g_cache_info.authenticator_id = (unsigned long long)-1;
    }

    ex_log(LOG_DEBUG, "opm_get_fingerprint_ids return = %d", retval);
    return FINGERPRINT_RES_SUCCESS;
}

BOOL check_cancelable() {
    if (g_need_cancel) {
        host_touch_set_hbm_system(FALSE);
    }
    return g_need_cancel;
}

BOOL check_need_pause(void) {
    if (g_need_pause) {
        host_touch_set_hbm_system(FALSE);
    }
    return g_need_pause;
}

#if !defined(__ET7XX__)
static void power_off(void) {
    int ret = 0;
    ret = opm_set_work_mode(POWEROFF_MODE);
    if (ret != 0) {
        ex_log(LOG_ERROR, "power_off Fail = %d", ret);
    }
}
#endif

static BOOL g_spi_connected = FALSE;
void setSpiState(BOOL spi_on, BOOL forced) {
    if (!spi_on && (forced || g_spi_connected)) {
        opm_close_spi();
        g_spi_connected = FALSE;

    } else if (spi_on && (forced || !g_spi_connected)) {
        opm_open_spi();
        g_spi_connected = TRUE;
    } else {
        ex_log(LOG_DEBUG, "setSpiState do nothing, spi_on = %d, forced = %d, g_spi_connected = %d",
               spi_on, forced, g_spi_connected);
    }
    ex_log(LOG_VERBOSE, "setSpiState: spi_on = %d, forced = %d, g_spi_connected = %d", spi_on,
           forced, g_spi_connected);
}

static int touch_enroll(unsigned int* percentage, unsigned int enrolled_count) {
    plat_mutex_lock(lock);
    int retval, ret2 = 0;
    int enroll_option = ENROLL_OPTION_NORMAL;
    int enroll_retry_count = 0;
    unsigned int status = FP_LIB_FINGER_LOST;
    unsigned int estate = ESTATE_WAIT_INT;
    unsigned int get_image_result;
    cmd_enrollresult_t enroll_result;
    mem_set(&enroll_result, 0, sizeof(cmd_enrollresult_t));
    BigDataInfo_t bigDataInfo;
    g_need_pause = FALSE;

    host_touch_set_finger_reset();

    do {
        switch (estate) {
            case ESTATE_WAIT_INT:
                ex_log(LOG_INFO, "estate == ESTATE_WAIT_INT");
                enroll_retry_count = 0;
                g_enroll_info.enroll_retry_count = enroll_retry_count;
                opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);
#if !defined(__ET7XX__)
                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                if (check_need_pause()) {
                    estate = ESTATE_PAUSE;
                    break;
                }
                retval = opm_set_work_mode(DETECT_MODE);
                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }
                if (FINGERPRINT_RES_SUCCESS != retval) {
                    ex_log(LOG_ERROR, "ESTATE_WAIT_INT set detect mode failed return = %d", retval);
                }
#endif
                notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;

                ret2 = wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout);

                if (ret2 == FINGERPRINT_RES_SECURE_ENROLL_TIMEOUT) {
                    notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
                    ret2 = 0;
                }

                if (!ret2) {
                    ex_log(LOG_DEBUG, "wait_trigger cancel, %d", ret2);
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "power on sensor fail");
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }

                host_touch_set_hbm_evalute_always();

                if ((enroll_result.percentage >
                     (uint32_t)g_enroll_config.capture_delay_enroll_start_progress) &&
                    g_enroll_config.capture_delay_enroll_enable) {
                    ex_log(LOG_INFO, "percentage = %d , enroll still triggered at %d",
                           enroll_result.percentage, g_enroll_config.capture_delay_enroll_enable);
                    estate = ESTATE_CAPTURE_DELAY;
                    break;
                }  // else go down to case ESTATE_GET_IMG;

            case ESTATE_GET_IMG:
                ex_log(LOG_DEBUG, "estate == ESTATE_GET_IMG");
                g_enroll_info.is_redundant = FALSE;
                g_finger_on_detected_finger_off = FALSE;

                host_touch_set_hbm_system(TRUE);
                if (check_need_pause()) {
                    estate = ESTATE_PAUSE;
                    break;
                }

                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                if (host_touch_is_finger_off()) {
                    estate = ESTATE_FINGER_OFF;
                    notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                    get_image_result = FP_IMAGE_FINGER_OFF_0;
                    break;
                }

#ifdef FD_CAPTURE_IMAGE_FIRST_ONLY_GET_RAW
                retval = opm_get_raw_image(get_host_device_info());
#else
                retval = opm_get_image(&get_image_result, get_host_device_info());
#endif
                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }
#ifdef __SHOW_LIVEIMAGE__
                transfer_frames_to_client(TRANSFER_LIVE_IMAGE, get_image_result, FALSE, 0);
#endif

                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "get raw image failed retval = %d", retval);
                    break;
                }

                if (host_touch_is_finger_off()) {
                    estate = ESTATE_FINGER_OFF;
                    notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                    get_image_result = FP_IMAGE_FINGER_OFF_1;
                    break;
                }

#ifdef FD_CAPTURE_IMAGE_FIRST_ONLY_GET_RAW
                mem_set(&bigDataInfo, 0, sizeof(BigDataInfo_t));
                retval = opm_get_image_ipp(&get_image_result, &bigDataInfo);
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "opm_get_image_ipp failed retval = %d", retval);
                    break;
                }
#endif
                ex_log(LOG_INFO, "get image ipp img_quality %d", get_image_result);
#if !defined(__ET7XX__)
                if (get_image_result == FP_IMAGE_FAIL_LOW_QUALITY ||
                    get_image_result == FP_IMAGE_FAIL_LOW_QUALITY_AND_LOW_COVERAGE) {
                    estate = ESTATE_FINGER_OFF;
                    notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);

                } else if (get_image_result == FP_IMAGE_HELP_TOO_WET) {
                    estate = ESTATE_FINGER_OFF;
                    notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
                } else {
                    ex_log(LOG_DEBUG, "%s others", __func__);
                }
#endif
                if (g_finger_on_detected_finger_off == TRUE) {
                    ex_log(LOG_DEBUG, "%s finger on detected finger off, retry get image",
                           __func__);
                    estate = ESTATE_GET_IMG;
                    const uint16_t detect_vibration = 1;
                    int out_buffer_size = 0;
                    opm_extra_command(0, (uint8_t*)&detect_vibration, sizeof(uint16_t), NULL,
                                      &out_buffer_size);
                } else if (get_image_result == FP_IMAGE_FAIL_SPOOF_FINGER) {
                    estate = ESTATE_FINGER_OFF;
                    notify(EVENT_IMG_FAKE_FINGER, 0, 0, NULL, 0);
                    notify(EVENT_IMG_DIRTY_SCREEN, 0, 0, NULL, 0);
                } else if (get_image_result == FP_IMAGE_FAIL_LOW_COVERAGE ||
                    host_touch_is_low_coverage()) {
                    estate = ESTATE_FINGER_OFF;
                    host_touch_set_low_coverage(FALSE);
                    notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
                } else if (get_image_result == FP_IMAGE_SUCCESS) {
                    estate = ESTATE_ENROLL;
                    if (enroll_option != ENROLL_OPTION_REJECT_RETRY) {
                        notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
                    }
                } else if (get_image_result == FP_IMAGE_SPI_FAIL) {
                    estate = ESTATE_FINGER_OFF;
                    notify(EVENT_IMG_SPI_FAIL, 0, 0, NULL, 0);
                } else
                    estate = ESTATE_WAIT_INT;

                break;

            case ESTATE_ENROLL:
                ex_log(LOG_INFO, "estate == ESTATE_ENROLL");
                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                if (check_need_pause()) {
                    estate = ESTATE_PAUSE;
                    break;
                }
                retval = opm_do_enroll(&enroll_result, enroll_option, enrolled_count,
                                       g_enroll_info.fingerprint_info.fingerprint_id, &bigDataInfo);
                sprint_algoinfo(&bigDataInfo);
                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "opm_identify_enroll failed, return %d", retval);
                    break;
                }

                *percentage = enroll_result.percentage;

                estate = ESTATE_FINGER_OFF;

                g_enroll_info.enroll_retry_count = enroll_retry_count;
                enroll_retry_count++;
                ex_log(LOG_INFO, "estate == ESTATE_ENROLL retry = %d",
                       g_enroll_info.enroll_retry_count);

                if (enroll_result.status == FP_LIB_ENROLL_HELP_SAME_AREA) {
                    notify(EVENT_ENROLL_HIGHLY_SIMILAR, 0, 0, NULL, 0);
                    g_enroll_info.is_redundant = TRUE;

                } else if (enroll_result.status == FP_LIB_ENROLL_SUCCESS) {
#ifdef __ET7XX__  // ENROLL_THE_FIRST
                    if (enroll_result.percentage < 100) {
                        ex_log(LOG_INFO, "enroll_extra_1st (%d) %d",
                               g_enroll_config.enroll_extra_1st_enable,
                               g_enroll_config.enroll_extra_1st_before_progress);
                        if (g_enroll_config.enroll_extra_1st_enable) {
                            ex_log(LOG_INFO, "enroll_result.percentage %d",
                                   enroll_result.percentage);
#ifdef RBS_EVTOOL
                            // Skip the first reject-retry image of DQE flow
                            enroll_option = ENROLL_OPTION_REJECT_RETRY;
#endif
                            if (enroll_result.percentage >=
                                (uint32_t)g_enroll_config.enroll_extra_1st_before_progress) {
                                cmd_enrollresult_t enroll_result_temp;
                                opm_do_enroll(&enroll_result_temp, ENROLL_OPTION_STOP_DQE, 0,
                                              g_enroll_info.fingerprint_info.fingerprint_id, NULL);
                            } else if (enroll_option == ENROLL_OPTION_REJECT_RETRY) {
                                opm_do_enroll(&enroll_result, ENROLL_OPTION_ENROLL_THE_FIRST, 0,
                                              g_enroll_info.fingerprint_info.fingerprint_id, NULL);
                                /*notify(EVENT_ENROLL_LQ,
                                g_enroll_info.fingerprint_info.fingerprint_id,
                                enroll_result.percentage, NULL, 0);*/
                            }
                        }
                    }
#endif
                    if (enroll_option != ENROLL_OPTION_STOP_DQE) {
                        int encry_param[2];
                        encry_param[0] = g_enroll_info.fingerprint_info.fingerprint_id;
                        encry_param[1] = g_has_enroll_count;

                        // if(enroll_result.percentage != 100)
                        {
                            notify(EVENT_ENROLL_OK, g_enroll_info.fingerprint_info.fingerprint_id,
                                   enroll_result.percentage, NULL, 0);
                        }
                        ex_log(LOG_INFO, "encry_param %d,current percentage %d", encry_param[1],
                               enroll_result.percentage);
#ifdef __SUPPORT_SAVE_ENROLL_RAW__
                        opt_receive_data(TYPE_RECEIVE_ENCRY_IMAGE, (unsigned char*)encry_param,
                                         sizeof(encry_param), NULL, 0);
#endif
                    }
                } else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_QUALITY) {
                    if (enroll_retry_count <= VENDOR_FLOW_TRY_MATCH) {
                        estate = ESTATE_GET_IMG;
                    } else {
                        notify(EVENT_IMG_BAD_QLTY, g_enroll_info.fingerprint_info.fingerprint_id,
                               enroll_result.percentage, NULL, 0);
                    }
                    enroll_option = ENROLL_OPTION_REJECT_RETRY;
                    break;
                } else if (enroll_result.status ==
                           FP_LIB_ENROLL_FAIL_LOW_QUALITY_DYNAMIC_REJECT_LEVEL) {
                    /*notify(EVENT_IMG_BAD_QLTY,
                       g_enroll_info.fingerprint_info.fingerprint_id,
                           enroll_result.percentage, NULL, 0); */
                    estate = ESTATE_GET_IMG;
                    enroll_option = ENROLL_OPTION_REJECT_RETRY;
                    break;
                } else if (enroll_result.status == FP_LIB_ENROLL_FAIL_LOW_COVERAGE) {
                    notify(EVENT_IMG_PARTIAL, g_enroll_info.fingerprint_info.fingerprint_id,
                           enroll_result.percentage, NULL, 0);

                } else if (enroll_result.status == FP_LIB_ENROLL_TOO_FAST) {
                    // Todo
                } else if (enroll_result.status == FP_LIB_ENROLL_HELP_SCRATCH_DETECTED) {
                    notify(EVENT_ENROLL_SCRATCH_DETECTED, 0, 0, NULL, 0);
                } else if (enroll_result.status == FP_LIB_ENROLL_HELP_ALREADY_EXIST) {
                    notify(EVENT_ENROLL_DUPLICATE_FINGER, 0, 0, NULL, 0);
                } else if (enroll_result.status == FP_LIB_ENROLL_FAIL_SPOOF_FINGER) {
                    notify(EVENT_IMG_FAKE_FINGER, 0, 0, NULL, 0);
                    notify(EVENT_IMG_DIRTY_SCREEN, 0, 0, NULL, 0);
                } else {
                    retval = FINGERPRINT_RES_ALGORITHM_ERROR;
                }

                break;

            case ESTATE_CAPTURE_DELAY:
                ex_log(LOG_INFO, "estate == ESTATE_CAPTURE_DELAY");

                /*ESTATE_CAPTURE_DELAY will delay for awhile.*/
                setSpiState(FALSE, FALSE);

                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }
                event_check_fd check[2];
                check[0].checkerFunc = &check_cancelable;
                check[1].checkerFunc = &host_touch_is_finger_off;
                event_poll_wait(
                    check, 2,
                    core_config_get_int(INI_SECTION_ENROLL, KEY_CAPTURE_DELAY_WAITING_TIME, 1500));
                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                if (check[0].revent) {
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }

                if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "power on sensor fail");
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }

                setSpiState(TRUE, FALSE);
                estate = ESTATE_GET_IMG;

                if (check[1].revent) {
                    estate = ESTATE_WAIT_INT;
                    notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                    break;
                }

                break;

            case ESTATE_PAUSE:
                ex_log(LOG_INFO, "estate == ESTATE_PAUSE");
                /*
                if (check_need_pause() && !is_power_off_mode) {
                    opm_set_work_mode(POWEROFF_MODE);
                    is_power_off_mode = TRUE;
                }
                */

                if (!check_need_pause()) {
                    estate = ESTATE_WAIT_INT;
                }

                if (check_cancelable()) {
                    g_need_pause = FALSE;
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }

                plat_sleep_time(20);
                break;

            case ESTATE_FINGER_OFF:
                ex_log(LOG_INFO, "estate == ESTATE_FINGER_OFF");
#ifdef __SUPPORT_SAVE_IMAGE__
                transfer_frames_to_client(TRANSFER_ENROLL_IMAGE, get_image_result, FALSE, 0);
#endif
                enroll_option = ENROLL_OPTION_NORMAL;
                /*ESTATE_FINGER_OFF will be excuted many times until the finger leaving
                calling setSpiState(on) to keep the spi open for a while to avoid continual opening
                or closing the spi */
#ifdef __ET7XX__
                if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "power off sensor fail");
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }
#else
                setSpiState(TRUE, FALSE);
#endif

                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }
                if (host_touch_is_enable()) {
                    if (host_touch_is_using_oplus_flow()) {
                        g_trigger_case = TRIGGER_WAIT_TOUCH_UP;
                        ret2 = wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout);
                        ex_log(LOG_DEBUG, "enroll fingeroff wait_trigger ret2 = %d", ret2);
                        if (ret2 == FINGERPRINT_RES_SECURE_ENROLL_TIMEOUT) {
                            notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
                            ret2 = 0;
                        }
                        if (!ret2) {
                            retval = FINGERPRINT_RES_CANCEL;
                            goto EXIT;
                        }
                    }

                    status = FP_LIB_FINGER_LOST;
                }

                if (check_need_pause()) {
                    estate = ESTATE_PAUSE;
                    break;
                }

                if (check_cancelable()) {
                    retval = FINGERPRINT_RES_CANCEL;
                    goto EXIT;
                }
                if (enroll_result.percentage >= 100) {
                    goto EXIT;
                }

                // When finger is not lost, CORE change to detect mode already
                // opm_set_work_mode(DETECT_MODE);

                if (status == FP_LIB_FINGER_LOST) {
                    estate = ESTATE_WAIT_INT;
                    notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
#if !defined(__ET7XX__)
                    setSpiState(FALSE, FALSE);
#endif
                }
                break;
        }
    } while (retval == FINGERPRINT_RES_SUCCESS);
EXIT:
#ifdef __ET7XX__
    if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power off sensor fail");
        retval = FINGERPRINT_RES_FAILED;
    }
#else
    setSpiState(FALSE, FALSE);
#endif

    plat_mutex_unlock(lock);
    return retval;
}

int quick_enroll_uninitialize(void) {
    int ret2 = FINGERPRINT_RES_SUCCESS;
    ex_log(LOG_DEBUG, "quick_enroll_uninitialize= %d In", ret2);
    opm_enroll_uninitialize();
    /*ret2 = wait_trigger(0, TIMEOUT_FOR_ONE_TRY, g_enroll_timeout);

    if (ret2 == FINGERPRINT_RES_SECURE_ENROLL_TIMEOUT) {
        notify(EVENT_ENROLL_TIMEOUT, 0, 0, NULL, 0);
        return FINGERPRINT_RES_CANCEL;
    } else if (!ret2) {
        ex_log(LOG_DEBUG, "wait_trigger cancel, %d", ret2);
        return FINGERPRINT_RES_CANCEL;
    }*/

    ex_log(LOG_DEBUG, "quick_enroll_uninitialize= %d, Out", ret2);
    return FINGERPRINT_RES_SUCCESS;
}

static int do_enroll() {
    int retval;
    unsigned int percentage = 0, enrolled_count = 0;
    int fid_size;

    g_oparate_type = DO_ENROLL;

    g_has_enroll_count = 0;
    g_enroll_percentage_tmp = 0;
    g_temp_remaining = 0;
    g_enroll_config.enroll_max_count =
        core_config_get_int(INI_SECTION_ENROLL, KEY_MAX_ENROLL_COUNT, INID_MAX_ENROLL_COUNT);
#ifdef EGIS_DEBUG_MEMORY
    ex_log(LOG_ERROR, "opm_print_memory do_enroll start");
    retval = opm_print_memory(EGIS_MEM_LOG_LEVEL0);
    if (retval != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "opm_print_memory error");
        return retval;
    }
#endif

    if (core_config_get_int(INI_SECTION_ENROLL, KEY_ENROLL_DUPLICATE,
                            VENDOR_ET7XX_ENROLL_DUPLICATE)) {
        retval = opm_get_enrolled_count(&enrolled_count);
        if (retval) {
            ex_log(LOG_ERROR, "opm_get_enrolled_count failed, return %d", retval);
            goto finish;
        }
    }

    retval = touch_enroll(&percentage, enrolled_count);

    ex_log(LOG_DEBUG, "ENROLL loop end!!!!!! percentage = %d, retval = %d", percentage, retval);

finish:
    retval = quick_enroll_uninitialize();
    ex_log(LOG_DEBUG, "quick_enroll_uninitialize, retval = %d", retval);
    if (percentage >= 100 && retval == FINGERPRINT_RES_SUCCESS) {
        retval = opm_save_enrolled_fingerprint(g_enroll_info);
        if (FINGERPRINT_RES_SUCCESS != retval) {
            ex_log(LOG_ERROR, "saved fingerprint failed");
            notify(EVENT_ENROLL_FAILED, 0, 0, NULL, 0);
        } else {
            fid_size = sizeof(g_enroll_info.fingerprint_info.fingerprint_id);
            opt_receive_data(TYPE_RECEIVE_USER_INFO,
                             (unsigned char*)&g_enroll_info.fingerprint_info.user_id,
                             sizeof(g_enroll_info.fingerprint_info.user_id), NULL, 0);
            opt_receive_data(TYPE_RECEIVE_TEMPLATE,
                             (unsigned char*)&g_enroll_info.fingerprint_info.fingerprint_id,
                             fid_size, NULL, 0);

            ex_log(LOG_DEBUG, "EVENT_CUSTOM_ENROLL_OK_FINAL");
            // notify(EVENT_CUSTOM_ENROLL_OK_FINAL, g_enroll_info.fingerprint_info.fingerprint_id,
            // 100,NULL, 0);
#ifdef ALGO_GEN_4
            opt_receive_data(TYPE_RECEIVE_DEBASE, NULL, 0, NULL, NULL);
#endif

            g_cache_info.user_id = -1;
            sync_user_cache(g_enroll_info.fingerprint_info.user_id);
            notify(EVENT_ENROLL_SUCCESS, 0, 0, NULL, 0);
        }
    } else {
        opt_receive_data(TYPE_DELETE_ENCRY_IMAGE,
                         (unsigned char*)&g_enroll_info.fingerprint_info.fingerprint_id,
                         sizeof(int), NULL, 0);
    }

    g_oparate_type = DO_OTHER;

    if (retval == FINGERPRINT_RES_CANCEL) {
        notify(EVENT_ENROLL_CANCELED, 0, 0, NULL, 0);
    } else if (retval != FINGERPRINT_RES_SUCCESS) {
        notify(EVENT_ERR_ENROLL, 0, 0, NULL, 0);
    }
#ifdef __ET7XX__
    retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_SAVE_ET7XX_CALI_DATA, 0, 0,
                                    NULL, NULL);
    if (retval != FP_LIB_OK) {
        ex_log(LOG_ERROR, "%s, Save cali data Fail %d", __func__, retval);
    }
#endif
    return FINGERPRINT_RES_SUCCESS;
}

static int do_verify() {
    const uint32_t LIVENESS_AUTHENTICATION = 0;
    int retval;
    int fid_size;
    unsigned int status = 0;
    unsigned int match_id;
    BOOL has_result = FALSE, stop_retry_flow = FALSE;
    unsigned int get_image_result;
    unsigned int vstate = VSTATE_FINGER_INIT;
    unsigned char auth_token[AUTH_TOKEN_LEN];
    unsigned int auth_token_len = AUTH_TOKEN_LEN;
    uint16_t is_get_image_finger_off = 0;
    unsigned long long start_wait_expo;
    int stable_expo;

    int flow_trymatch_count =
        core_config_get_int(INI_SECTION_VERIFY, KEY_FLOW_TRY_MATCH, INID_FLOW_TRY_MATCH);
    int flow_try_match = flow_trymatch_count;
    int ext_feat_quality_trymatch_th =
        core_config_get_int(INI_SECTION_VERIFY, KEY_EXT_FEAT_QUALITY_TRYMATCH_THRESHOLD,
                            INID_EXT_FEAT_QUALITY_TRYMATCH_THRESHOLD);
    int ext_feat_quality_badimg_th =
        core_config_get_int(INI_SECTION_VERIFY, KEY_EXT_FEAT_QUALITY_BADIMG_THRESHOLD, 25);
    unsigned char is_tmpl_updated = 0;
    int ext_feat_quality = 0;
    mem_set(&g_verify_bigDataInfo, 0, sizeof(BigDataInfo_t));

    key_value_t dcsmsg[29];
    mem_set(&dcsmsg, 0, sizeof(dcsmsg));
    mem_set(&(g_verify_info.dcsmsg), 0, sizeof(dcs_msg_t));
    g_verify_info.is_passed = FALSE;

    g_oparate_type = DO_VERIFY;

    //host_touch_set_finger_reset();
    opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);
#ifdef EGIS_DEBUG_MEMORY
    retval = opm_print_memory(EGIS_MEM_LOG_LEVEL0);
    if (retval != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "opm_print_memory error");
        return retval;
    }
#endif

    plat_mutex_lock(lock);

    do {
        switch (vstate) {
            case VSTATE_FINGER_INIT: {
                ex_log(LOG_DEBUG, "vstate == VSTATE_FINGER_INIT");
                g_verify_flow_flag = 1;
                retval = opm_identify_start(LIVENESS_AUTHENTICATION);
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }

                vstate = VSTATE_WAIT_INT;
                break;
            }
            case VSTATE_FINGER_UNINIT: {
                break;
            }
            case VSTATE_WAIT_INT: {
                ex_log(LOG_INFO, "vstate == VSTATE_WAIT_INT");
                flow_try_match = flow_trymatch_count;
                g_verify_info.flow_try_match = flow_try_match;
                g_verify_info.dcsmsg.dsp_available = FP_GET_IMAGE_NORMAL_FINGER_OFF;
                stop_retry_flow = FALSE;
                start_wait_expo = 0;
                stable_expo = 0;
                mem_set(&dcsmsg, 0, sizeof(dcsmsg));
                mem_set(&(g_verify_info.dcsmsg), 0, sizeof(dcs_msg_t));
                g_verify_info.is_passed = FALSE;

                ex_log(LOG_DEBUG, "wait init g_verify_flow_flag = %d", g_verify_flow_flag);
#if !defined(__ET7XX__)
                if (check_cancelable()) {
                    goto EXIT;
                }
                retval = opm_set_work_mode(DETECT_MODE);
                if (check_cancelable()) {
                    goto EXIT;
                }
                if (FINGERPRINT_RES_SUCCESS != retval) {
                    ex_log(LOG_ERROR, "VSTATE_WAIT_INT set detect mode failed, retval = %d",
                           retval);
                    break;
                }
#endif
                notify(EVENT_FINGER_WAIT, 0, 0, NULL, 0);

                if (check_cancelable()) {
                    goto EXIT;
                }
                if (host_touch_is_using_oplus_flow()) {
                    g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;
                }

                if (!wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) {
                    goto EXIT;
                }
                if (check_cancelable()) {
                    goto EXIT;
                }
                host_touch_set_hbm_evalute_always();
                if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "power on sensor fail");
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }
            }

            case VSTATE_GET_IMG:
                ex_log(LOG_INFO, "vstate == VSTATE_GET_IMG");
                is_get_image_finger_off = 0;
                g_verify_info.matched_fingerprint_id = 0;
                g_verify_info.bad_qty_threshold = ext_feat_quality_badimg_th;
                // g_verify_info.is_residual = FALSE;
                g_save_debug_image_verify_info = g_verify_info;
                g_finger_on_detected_finger_off = FALSE;
                g_fingeroff_learning_flag = TRUE;

                BOOL isretry = !(flow_try_match == flow_trymatch_count);
                get_image_check_start();

                if (start_wait_expo != 0) {
                    stable_expo = get_image_stable_expo(start_wait_expo);
                }

                host_touch_set_hbm_system(TRUE);
                ex_log(LOG_DEBUG, "get image g_verify_flow_flag = %d", g_verify_flow_flag);
                if (check_cancelable()) {
                    goto EXIT;
                }
                if (host_touch_is_finger_off()) {
                    vstate = VSTATE_FINGER_OFF;
                    stop_retry_flow = TRUE;
                    get_image_result = FP_IMAGE_FINGER_OFF_0;
                    if (abs(flow_try_match - flow_trymatch_count) < 1) {
                        notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        // break;  // don't output image
                    } else {
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
                        g_verify_info.dcsmsg.dsp_available = FP_GET_IMAGE_BEFORE_FINGER_OFF;
                        ex_log(LOG_DEBUG, "detected finger off before opm_get_image,dsp_available = %d", g_verify_info.dcsmsg.dsp_available);
                        get_dcs_msg(dcsmsg, sizeof(dcsmsg));
                        notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char*)dcsmsg, sizeof(dcsmsg));
                    }
                    break;
                }
                plat_sleep_time(20);
#ifdef FD_CAPTURE_IMAGE_FIRST_ONLY_GET_RAW
                retval = opm_get_raw_image(get_host_device_info());
#else
                retval = opm_get_image(&get_image_result, get_host_device_info());
#endif
                start_wait_expo = plat_get_time();
                if (check_cancelable()) {
                    goto EXIT;
                }
                ex_log(LOG_INFO, "opm_get_raw_image retval = %d ", retval);

                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "opm_get_image failed retval = %d", retval);
                    break;
                }

                ex_log(LOG_INFO,
                       "g_finger_on_detected_finger_off = %d,get_image_result = %d",
                       g_finger_on_detected_finger_off, get_image_result);

                if (host_touch_is_finger_off()) { /*&& get_image_is_too_fast(FINGER_ON_DURATION)*/
                    g_fingeroff_learning_flag = is_verify_finger_on_stable(isretry, stable_expo);
                    ex_log(LOG_DEBUG, "detected finger off after opm_get_image");
                    if (!g_fingeroff_learning_flag) {
                        ex_log(LOG_INFO,
                            "detected finger off after opm_get_image and finger on not stable");
                        get_image_result = FP_IMAGE_FINGER_OFF_1;
                        g_verify_info.dcsmsg.dsp_available = FP_GET_IMAGE_AFTER_FINGER_OFF;
                        ex_log(LOG_INFO,
                            "detected finger off after opm_get_image and finger on not stable dsp_available=%d", g_verify_info.dcsmsg.dsp_available);
#ifdef __AUTH0__
                        ex_log(LOG_DEBUG, "support auth 0");
                        is_get_image_finger_off = 1;
                        int out_buffer_size = 0;
                        opm_is_fingeroff_command(0, (uint8_t*)&is_get_image_finger_off,
                                                    sizeof(uint16_t), NULL, &out_buffer_size);
                        goto DO_IPP;
#else
                        vstate = VSTATE_FINGER_OFF;
                        stop_retry_flow = TRUE;
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_TOO_FAST;
                        get_dcs_msg(dcsmsg, sizeof(dcsmsg));
                        notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char*)dcsmsg, sizeof(dcsmsg));
                        if (abs(flow_try_match - flow_trymatch_count) < 1) {
                            notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                            notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                            // break;  // don't output image
                        } else {
                            notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                            g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
                            ex_log(LOG_DEBUG, "detected finger off after opm_get_image");
                        }
#endif
                    }
                break;
                }
            DO_IPP:
#ifdef FD_CAPTURE_IMAGE_FIRST_ONLY_GET_RAW
                retval = opm_get_image_ipp(&get_image_result, &g_verify_bigDataInfo);
                sprint_algoinfo(&g_verify_bigDataInfo);
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "opm_get_image_ipp failed retval = %d", retval);
                    break;
                }
#endif
                ex_log(LOG_DEBUG, "opm_get_raw_image retval = %d, quality = %d", retval,
                       get_image_result);
                g_fingeroff_learning_flag &= (get_image_result == FP_IMAGE_SUCCESS ||
                    get_image_result == FP_IMAGE_FAIL_LOW_COVERAGE);

                g_verify_info.dcsmsg.retry_times =
                    (flow_trymatch_count - flow_try_match);  // start from 0

                if (FP_IMAGE_FAIL_LOW_QUALITY == get_image_result ||
                    FP_IMAGE_FAIL_LOW_QUALITY_AND_LOW_COVERAGE == get_image_result) {
                    vstate = VSTATE_VERIFY;
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_LOW_QTY;
                } else if (FP_IMAGE_HELP_TOO_WET == get_image_result) {
                    vstate = VSTATE_FINGER_OFF;
                    notify(EVENT_IMG_WATER, 0, 0, NULL, 0);
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_TOO_WET;
                } else if (FP_IMAGE_FAIL_RESIDUAL == get_image_result) {
                    vstate = VSTATE_FINGER_OFF;
                    notify(EVENT_IMG_RESIDUAL_FINGER, 0, 0, NULL, 0);
                    notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAKE;

                    get_dcs_msg(dcsmsg, sizeof(dcsmsg));
                    notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char*)dcsmsg, sizeof(dcsmsg));
                } else if (FP_IMAGE_FAIL_SPOOF_FINGER == get_image_result) {
                    vstate = VSTATE_FINGER_OFF;
                    notify(EVENT_IMG_FAKE_FINGER, 0, 0, NULL, 0);
                    notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAKE;
                    g_verify_info.dcsmsg.match_score = 0;

                    get_dcs_msg(dcsmsg, sizeof(dcsmsg));
                    notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char*)dcsmsg, sizeof(dcsmsg));
                } else if (g_finger_on_detected_finger_off == TRUE && abs(flow_try_match - flow_trymatch_count) >= 1) {
                    egislog_i("%s finger on detected finger off, retry get image", __func__);
                    int out_buffer_size = 0;
#ifdef __AUTH0__
                    is_get_image_finger_off = 1;
                    opm_is_fingeroff_command(0, (uint8_t*)&is_get_image_finger_off,
                                             sizeof(uint16_t), NULL, &out_buffer_size);
                    vstate = VSTATE_VERIFY;
#else
                    vstate = VSTATE_GET_IMG;
#endif
                    const uint16_t detect_vibration = 1;
                    opm_extra_command(0, (uint8_t*)&detect_vibration, sizeof(uint16_t), NULL,
                                      &out_buffer_size);

                } else if (FP_IMAGE_FAIL_LOW_COVERAGE == get_image_result) {
                    vstate = VSTATE_VERIFY;
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_LOW_COVERAGE;
                    break;
                } else if (FP_IMAGE_SUCCESS == get_image_result) {
                    vstate = VSTATE_VERIFY;
                    notify(EVENT_FINGER_READY, 0, 0, NULL, 0);
                } else if (FP_IMAGE_SPI_FAIL == get_image_result) {
                    vstate = VSTATE_FINGER_OFF;
                    notify(EVENT_IMG_SPI_FAIL, 0, 0, NULL, 0);
                    notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                } else {
                    vstate = VSTATE_FINGER_INIT;
                }

                break;

            case VSTATE_VERIFY: {
                ex_log(LOG_INFO, "vstate == VSTATE_VERIFY");

                if (check_cancelable()) {
                    goto EXIT;
                }
                auth_token_len = AUTH_TOKEN_LEN;
                mem_set(&g_verify_bigDataInfo, 0x00, sizeof(BigDataInfo_t));
                ex_log(LOG_DEBUG, "do verify g_verify_flow_flag = %d", g_verify_flow_flag);
                retval = opm_identify(&g_verify_info, &match_id, &status, auth_token,
                                      &auth_token_len, &ext_feat_quality, &g_verify_bigDataInfo);
                sprint_algoinfo(&g_verify_bigDataInfo);
                ex_log(LOG_INFO,
                       "opm_do_verify ret=%d, status=%u, match_id=%d, ext_feat_quality=%d", retval,
                       status, match_id, ext_feat_quality);
                if (check_cancelable()) {
                    goto EXIT;
                }
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "opm_identify fail, retval = %d", retval);
                    break;
                }

                int is_good_extract_qty = (ext_feat_quality > ext_feat_quality_trymatch_th) ? 1 : 0;
                if (FP_LIB_IDENTIFY_RETRYSTOP == status) {
                    ex_log(LOG_DEBUG, "status == FP_LIB_IDENTIFY_RETRYSTOP vstate = %d", vstate);
                    notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_RETRYSTOP;

                } else if (FP_LIB_IDENTIFY_NO_MATCH_PARTIAL == status) {
                    if ((is_get_image_finger_off) &&
                        abs(flow_try_match - flow_trymatch_count) < 1) {
                        ex_log(LOG_DEBUG, "EVENT_IMG_FAST is_get_image_finger_off = %d,status=%d",
                               is_get_image_finger_off, status);
                        get_image_result = FP_IMAGE_PARTIAL_FINGER_OFF_1;
                        notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_TOO_FAST;
                    } else {
                        get_image_result = FP_IMAGE_PARTIAL;
                        if ((flow_try_match-- > 0)) {
                            if (!is_good_extract_qty) {
                                vstate = VSTATE_GET_IMG;
                                break;
                            }
                        }
                        notify(EVENT_IMG_PARTIAL, 0, 0, NULL, 0);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_LOW_COVERAGE;
                    }
                } else if (FP_LIB_IDENTIFY_NO_MATCH_BADIMAGE == status) {
                    if ((is_get_image_finger_off) &&
                        abs(flow_try_match - flow_trymatch_count) < 1) {
                        ex_log(LOG_DEBUG, "EVENT_IMG_FAST is_get_image_finger_off = %d,status=%d",
                               is_get_image_finger_off, status);
                        get_image_result = FP_IMAGE_BAD_IMAGE_FINGER_OFF_1;
                        notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_TOO_FAST;
                    } else {
                        get_image_result = FP_IMAGE_BAD_IMAGE;
                        if ((flow_try_match-- > 0)) {
                            if (!is_good_extract_qty) {
                                vstate = VSTATE_GET_IMG;
                                break;
                            }
                        }
                        notify(EVENT_IMG_BAD_QLTY, 0, 0, NULL, 0);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_LOW_QTY;
                    }
                } else if (FP_LIB_IDENTIFY_NO_MATCH == status) {
                    if ((is_get_image_finger_off) &&
                        abs(flow_try_match - flow_trymatch_count) < 1) {
                        ex_log(LOG_DEBUG,
                               "EVENT_IMG_FAST is_get_image_finger_off = %d,status=%d",
                               is_get_image_finger_off, status);
                        notify(EVENT_IMG_FAST, 0, 0, NULL, 0);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        vstate = VSTATE_FINGER_OFF;
                        g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_TOO_FAST;
                        get_dcs_msg(dcsmsg, sizeof(dcsmsg));
                        notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char*)dcsmsg, sizeof(dcsmsg));
                        break;
                    }
                    g_save_debug_image_verify_info.flow_try_match = flow_try_match;
                    ex_log(LOG_INFO, "(%d) threshold %d, %d", is_good_extract_qty,
                           ext_feat_quality_trymatch_th, ext_feat_quality_badimg_th);
                    if ((flow_try_match-- > 0)) {
                        if (!is_good_extract_qty) {
                            ex_log(LOG_DEBUG, "opm_identify_finish");
                            opm_identify_finish();

                            ex_log(LOG_DEBUG, "opm_identify_start");
                            retval = opm_identify_start(LIVENESS_AUTHENTICATION);

                            vstate = VSTATE_GET_IMG;
                            break;
                        } else {
                            host_touch_set_hbm_system(FALSE);
                            notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
                        }
                    } else {
                        host_touch_set_hbm_system(FALSE);
                        notify(EVENT_VERIFY_NOT_MATCHED, 0, 0, NULL, 0);
#if defined(SIMULATE_NO_SENSOR) || defined(RBS_EVTOOL)
                        ex_log(LOG_DEBUG, "evtool has_result");
                        has_result = TRUE;
#endif
                        opm_identify_finish();
                    }
                    g_verify_info.dcsmsg.fail_reason = FP_LIB_VERIFY_FAIL;
                } else if (FP_LIB_IDENTIFY_MATCH == status ||
                    FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE == status) {
                    g_save_debug_image_verify_info.flow_try_match = flow_try_match;
                    // TIME_MEASURE_STOP(total_verify, "matched");
                    g_save_debug_image_verify_info.matched_fingerprint_id = (unsigned int)match_id;

                    host_touch_set_hbm_system(FALSE);

                    notify(EVENT_VERIFY_MATCHED, g_verify_info.user_id, match_id, auth_token,
                           auth_token_len);
                    has_result = TRUE;
                    g_verify_info.is_passed = TRUE;
                    if (status == FP_LIB_IDENTIFY_MATCH_UPDATED_TEMPLATE) {
                        retval = opm_identify_template_update(&is_tmpl_updated);
                        if (retval == FINGERPRINT_RES_SUCCESS && is_tmpl_updated) {
                            opm_identify_template_save();
                            fid_size = sizeof(match_id);
                            opt_receive_data(TYPE_RECEIVE_TEMPLATE, (unsigned char*)&match_id,
                                             fid_size, NULL, 0);
                        }
                    }
                } else {
                    ex_log(LOG_ERROR, "unknown identify result %d", status);
                }

                get_dcs_msg(dcsmsg, sizeof(dcsmsg));
                notify(EVENT_SEND_DCS_MSG, 0, 0, (unsigned char*)dcsmsg, sizeof(dcsmsg));

                vstate = VSTATE_FINGER_OFF;

                ex_log(LOG_INFO, "total_verify_retry = %d , match fid = %d",
                       (INID_FLOW_TRY_MATCH - g_save_debug_image_verify_info.flow_try_match),
                       g_save_debug_image_verify_info.matched_fingerprint_id);
            } break;

            case VSTATE_FINGER_OFF: {
                ex_log(LOG_INFO, "vstate == VSTATE_FINGER_OFF");
                ex_log(LOG_DEBUG, "fingeroff g_verify_flow_flag = %d", g_verify_flow_flag);
#ifdef __SUPPORT_SAVE_IMAGE__
                transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, get_image_result, FALSE,
                                          status);
#endif
#ifdef __ET7XX__
                if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, "power off sensor fail");
                    retval = FINGERPRINT_RES_FAILED;
                    goto EXIT;
                }
#else
                setSpiState(TRUE, FALSE);
#endif
                if (check_cancelable()) {
                    goto EXIT;
                }
                if (host_touch_is_enable()) {
                    status = FP_LIB_FINGER_LOST;
                    opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);
                }
                if (check_cancelable()) {
                    goto EXIT;
                }
                // When finger is not lost, CORE change to detect mode already
                // opm_set_work_mode(DETECT_MODE);
                if (host_touch_is_using_oplus_flow()) {
                    g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;
                }

                notify(EVENT_FINGER_TOUCH_UP, 0, 0, NULL, 0);

                if (host_touch_is_using_oplus_flow()) {
                    g_trigger_case = TRIGGER_WAIT_TOUCH_UP;
                    ex_log(LOG_INFO, "fingeroff g_verify_touch_up_flag = %d", g_verify_touch_up_flag);
                    if (g_verify_touch_up_flag || wait_trigger(0, TIMEOUT_FOR_ONE_TRY, TIMEOUT_WAIT_FOREVER)) {
                        vstate = VSTATE_FINGER_INIT;
                        g_verify_touch_up_flag = 0;
                        g_verify_flow_flag = 0;
                        notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
                        setSpiState(FALSE, FALSE);
                        if (has_result) {
                            goto EXIT;
                        }
                        opm_identify_finish();
                    }
                } else {
                    if (status == FP_LIB_FINGER_LOST ||
                        !wait_trigger(3, 30, TIMEOUT_WAIT_FOREVER)) {
                        vstate = VSTATE_FINGER_INIT;
                        if (stop_retry_flow) {
                            ex_log(LOG_INFO, "stop flow_try_match");
                            flow_try_match = 0;
                        }
                        notify(EVENT_FINGER_LEAVE, 0, 0, NULL, 0);
#if !defined(__ET7XX__)
                        setSpiState(FALSE, FALSE);
#endif
                        if (has_result) {
                            goto EXIT;
                        }
                    }
                }
                break;
            }
        }
    } while (retval == FINGERPRINT_RES_SUCCESS);
EXIT:
#ifdef __ET7XX__
    if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power off sensor fail");
        retval = FINGERPRINT_RES_FAILED;
    }
#else
    setSpiState(FALSE, FALSE);
#endif

    if (check_cancelable()) {
        host_touch_set_hbm_system(FALSE);
        host_touch_set_finger_on(FALSE);
        host_touch_set_finger_off(TRUE);
        host_touch_set_ui_ready(FALSE);
        ex_log(LOG_INFO, "detect called cancel");
#ifdef __SUPPORT_SAVE_IMAGE__
        get_image_result = FP_IMAGE_FAIL_CANCEL;
        transfer_frames_to_client(TRANSFER_VERIFY_IMAGE_V2, get_image_result, FALSE, status);
#endif
    }
    g_verify_touch_up_flag = 0;
    g_verify_flow_flag = 0;
    g_oparate_type = DO_OTHER;
    opm_identify_finish();
    opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);

#ifdef ALGO_GEN_4
    opt_receive_data(TYPE_RECEIVE_DEBASE, NULL, 0, NULL, NULL);
#endif

    if (retval != FINGERPRINT_RES_SUCCESS) {
        notify(EVENT_ERR_IDENTIFY, 0, 0, NULL, 0);
    }
    notify(EVENT_IDENTIFY_FINISH, 0, 0, NULL, 0);
#ifdef __ET7XX__
    retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_SAVE_ET7XX_CALI_DATA, 0, 0,
                                    NULL, NULL);
    if (retval != FP_LIB_OK) {
        ex_log(LOG_ERROR, "%s, Save BDS Pool Fail", __func__);
    }
#endif

    plat_mutex_unlock(lock);

    return FINGERPRINT_RES_SUCCESS;
}

#if defined(TEMPLATE_UPGRADE_FROM_SAVED_IMAGE) && defined(__SAVE_IN_REE__)

void do_revice_upgrade_template() {
    unsigned int index = 0;
    if (g_cache_info.fingerprint_ids_count > 0) {
        for (index = 0; index < g_cache_info.fingerprint_ids_count; index++) {
            ex_log(LOG_DEBUG, "fps.fingerprint_ids[%d]= %d", index, g_cache_info.fingerprint_ids[index]);
            opt_receive_data(TYPE_RECEIVE_TEMPLATE,
                             (unsigned char*)&g_cache_info.fingerprint_ids[index], sizeof(int), NULL,
                             0);
        }
    }
}

void do_updata_ree() {
    int retval = FINGERPRINT_RES_SUCCESS;
    retval = opt_send_data(TYPE_SEND_TEMPLATE, NULL, 0);
    ex_log(LOG_INFO, "%s, retval = %d", __func__, retval);
    if (retval == EGIS_TEMPL_NEED_UPGRADED) {
        g_is_upgrade = 0;
        g_is_flag = 1;
        opt_receive_data(TYPE_RECEIVE_CONVICT_ID, NULL, 0,
                         (unsigned char*)g_cache_info.fingerprint_ids,
                         (int*)&(g_cache_info.fingerprint_ids_count));

        ex_log(LOG_DEBUG, "%s, g_cache_info.fingerprint_ids_count = %d", __func__,
               g_cache_info.fingerprint_ids_count);

        ex_log(LOG_DEBUG, "%s, update begin", __func__);
        opt_send_data(TYPE_SEND_UPGRADE, NULL, 0);

        g_cache_info.user_id = -1;
        sync_user_cache(0);

        ex_log(LOG_DEBUG, "%s, recevie template begin", __func__);
        do_revice_upgrade_template();

        g_is_flag = 0;
        plat_semaphore_post(g_upgrade_sem);
    }
}
#else
#if defined(TEMPLATE_UPGRADE_FROM_SAVED_IMAGE)
void do_updata_tee() {
    int retval = FINGERPRINT_RES_SUCCESS;
    ex_log(LOG_INFO, "%s, retval = %d", __func__, retval);
    g_is_flag = 1;
    retval = opm_upgrade_template();
    ex_log(LOG_DEBUG, " %s, opm_upgrade_template retval = %d", __func__, retval);
    g_is_flag = 0;

    plat_semaphore_post(g_upgrade_sem);
}

#endif
#endif

static int do_empty_setting() {
    return FINGERPRINT_RES_SUCCESS;
}

int cpt_initialize(unsigned char* in_data, unsigned int in_data_len) {
    int retval, retval_cali;
    g_hardware_ready = FALSE;
    int retry_count = 0;
    const int retry_time = 3;
    uint64_t current_time = get_ca_time_ms();
    int ret_info = 0;
    __system_property_set(PROPERTY_FP_FACTORY_SDK_ALGO_VERSION, RELEASE_SDK_ALGO_VERSION);
    opm_set_data(TYPE_SEND_CA_TIME, (unsigned char*)&current_time, sizeof(uint64_t));

#ifdef HOST_TOUCH_CONTROL
    host_touch_set_enable(TRUE);
#endif

    if (g_hdev <= 0) {
        retval = fp_device_open(&g_hdev);
        if (FINGERPRINT_RES_SUCCESS != retval) {
            ex_log(LOG_ERROR, "cpt_initialize fp_device_open failed");
            retval = FINGERPRINT_RES_DEVICE_OPEN_FAIL;
            goto EXIT;
        }
    }

    retval = fp_device_power_control(g_hdev, TRUE);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize fp_device_power_control failed");
        retval = FINGERPRINT_RES_POWER_CONTROL_FAIL;
        goto EXIT;
    }
    g_power_managemen.pwr_status = SENSOR_PWR_ON;

    retval = fp_device_spipin_enable(g_hdev, TRUE);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "fp_device_spipin_enable TRUE failed");
        retval = FINGERPRINT_RES_HW_UNAVALABLE;
        goto EXIT;
    }

#ifdef __OPLUS_ON_A50__
    retval = fp_device_reset(g_hdev);
#else
    retval = fp_device_poweron_reset(g_hdev);
#endif

    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize fp_device_reset failed");
        retval = FINGERPRINT_RES_DEVICE_RESET_FAIL;
        goto EXIT;
    }

    retval = fp_device_clock_enable(g_hdev, TRUE);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize fp_device_clock_enable TRUE failed");
        retval = FINGERPRINT_RES_HW_UNAVALABLE;
        goto EXIT;
    }

    retval = opm_initialize_sdk(in_data, in_data_len);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize opm_initialize_sdk return = %d", retval);
        retval = FINGERPRINT_RES_INIT_SDK_FAIL;
        goto EXIT;
    }

    __system_property_set(FINGERPRINT_CALIPROP, "0");
    retval_cali = opt_send_data(TYPE_SEND_CALIBRATION_DATA, NULL, 0);
    if (retval_cali == FINGERPRINT_RES_SUCCESS) {
        __system_property_set(FINGERPRINT_CALIPROP, "1");
#ifdef SAVE_BACKUP_CB
        opt_receive_data(TYPE_RECEIVE_BACKUP_CALIBRATION_DATA, NULL, 0, NULL, 0);
    } else {
        ex_log(LOG_ERROR, "CALIBRATION_DATA NOT EXIST");
        retval = opt_send_data(TYPE_SEND_BACKUP_CALIBRATION_DATA, NULL, 0);
        if (retval == FINGERPRINT_RES_SUCCESS) {
            __system_property_set(FINGERPRINT_CALIPROP, "1");
        }
#endif
    }

#ifndef ALGO_GEN_4
    retval = opt_send_data(TYPE_SEND_BDS, NULL, 0);
    retval = opt_send_data(TYPE_SEND_SCRATCH, NULL, 0);
#else
    retval = opt_send_data(TYPE_SEND_DEBASE, NULL, 0);
#endif
    retval = create_ini_config(TRUE, NULL, 0);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize create_ini_config return = %d", retval);
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    do {
        retval = opm_initialize_sensor();
        ex_log(LOG_DEBUG, "opm_initialize_sensor, ret =%d, retry_count =%d", retval, retry_count);
        retry_count++;
        if (FINGERPRINT_RES_SUCCESS != retval) {
            (void)do_power_on(SENSOR_PWR_OFF);
            plat_sleep_time(60);
            if (retry_count < retry_time) {
                (void)do_power_on(SENSOR_PWR_ON);
            }
        }
    } while (FINGERPRINT_RES_SUCCESS != retval && retry_count < retry_time);

    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize opm_initialize_sensor return = %d", retval);
        retval = FINGERPRINT_RES_SENSOR_INIT_FAIL;
        goto EXIT;
    } else if (retval_cali == FINGERPRINT_RES_OPEN_FILE_FAILED) {
        opt_receive_data(TYPE_RECEIVE_CALIBRATION_DATA, NULL, 0, NULL, 0);
    }

    retval = opm_initialize_algo();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_initialize opm_initialize_algo return = %d", retval);
        retval = FINGERPRINT_RES_ALGORITHM_NOTINIT;
        goto EXIT;
    }

    ex_log(LOG_DEBUG, "cpt_initialize opt_send_data return = %d\n", retval);

    retval = opm_calibration(0, PID_COMMAND, NULL, 0);
    if (EX_RES_SUCCESS == retval || EX_RES_NOT_NEED_RECALIBRATION == retval ||
        EX_RES_USE_OLD_CALIBRATION_DATA == retval) {
        g_hardware_ready = TRUE;
        retval = FINGERPRINT_RES_SUCCESS;
    } else {
        ex_log(LOG_ERROR, "cpt_initialize opm_calibration return = %d\n", retval);
        retval = FINGERPRINT_RES_NEED_CALIBRATION;
        goto EXIT;
    }

    ret_info = opt_send_data(TYPE_SEND_USERID, NULL, 0);

    plat_mutex_create(&lock);
    plat_mutex_create(&pwr_lock);
    plat_mutex_create(&g_thread_power_on_lock);

    host_touch_set_hbm_evalute_always();
    // host_touch_set_callback(notify);

    cpt_set_module_info();

    thread_manager_init();
    thread_manager_set_cancel_func((rbsCancelFunc)captain_cancel);
    thread_manager_set_idle_task((do_operation_callback)do_empty_setting);

    if (host_touch_is_using_oplus_flow()) {
        host_touch_set_callback(notify);
        init_oplus_event_listener(g_hdev);
    }

#if defined(TEMPLATE_UPGRADE_FROM_SAVED_IMAGE) && defined(__SAVE_IN_REE__)
    plat_semaphore_create(&g_upgrade_sem, 0, 1);
    ex_log(LOG_DEBUG, "begin thread for updata");
    plat_thread_create_ex(&g_upgrade_test_handle, do_updata_ree, NULL);
    ex_log(LOG_DEBUG, "end thread for updata");
#else
#if defined(TEMPLATE_UPGRADE_FROM_SAVED_IMAGE)
    plat_semaphore_create(&g_upgrade_sem, 0, 1);
    ex_log(LOG_DEBUG, "begin thread for updata");
    plat_thread_create_ex(&g_upgrade_test_handle, do_updata_tee, NULL);
    ex_log(LOG_DEBUG, "end thread for updata");
#endif
#endif

EXIT:
    if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "power off sensor fail");
        retval = FINGERPRINT_RES_FAILED;
    }
    return retval;
}

int cpt_uninitialize() {
    int retval;
    cpt_cancel();

    g_hardware_ready = FALSE;

    destroy_ini_config();
    retval = opm_uninitialize_sensor();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_uninitialize opm_uninitialize_sensor return = %d", retval);
    }
    retval = opm_uninitialize_algo();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_uninitialize opm_uninitialize_algo return = %d", retval);
    }
    retval = opm_uninitialize_sdk();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "cpt_uninitialize opm_uninitialize_sdk return = %d", retval);
    }

    if (0 != g_hdev) {
        fp_device_clock_enable(g_hdev, FALSE);
        fp_device_spipin_enable(g_hdev, FALSE);
        fp_device_reset_set(g_hdev, FALSE);
        fp_device_power_control(g_hdev, FALSE);
        fp_device_close(g_hdev);
        g_hdev = 0;
    }

    plat_mutex_release(&lock);
    plat_mutex_release(&pwr_lock);
    plat_mutex_release(&g_thread_power_on_lock);
    thread_manager_uninitialize();

    return 0;
}

int cpt_cancel() {
    g_has_enroll_count = 0;
    g_enroll_percentage_tmp = 0;
    g_temp_remaining = 0;
    thread_manager_cancel_task();
    ex_log(LOG_DEBUG, "cpt_cancel end");

    return FINGERPRINT_RES_SUCCESS;
}

void captain_cancel(BOOL cancel_flag) {
    ex_log(LOG_INFO, "captain_cancel enter [%d]", cancel_flag);

    g_need_cancel = cancel_flag;
    pthread_cond_signal(&cv);
}

int cpt_set_active_group(unsigned int user_id, const char* data_path) {
    int retval, result_sync_cache;

    ex_log(LOG_INFO, "%s, user_id = %u, data_path = %s", __func__, user_id, data_path);

#ifdef TEMPLATE_UPGRADE_FROM_SAVED_IMAGE
    if (g_is_flag) {
        ex_log(LOG_INFO, "%s, begin wait", __func__);
        retval = plat_semaphore_wait(g_upgrade_sem, 15 * 1000);
        ex_log(LOG_INFO, "%s, retval = %d", __func__, retval);
    }
#endif

    if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_set_active_group another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }

    retval = opm_set_active_group(user_id, data_path);
    if (FINGERPRINT_RES_SUCCESS == retval) {
        g_cache_info.user_id = -1;

        strncpy(g_user_path, data_path, MAX_PATH_LEN-1);
        g_user_path[MAX_PATH_LEN-1] = '\0';
        opt_send_data(TYPE_SEND_USER_INFO, (unsigned char*)&user_id, sizeof(user_id));
#ifdef TEMPLATE_UPGRADE_FROM_SAVED_IMAGE
        ex_log(LOG_DEBUG, "support upgrade");
#else
        opt_send_data(TYPE_SEND_TEMPLATE, NULL, 0);

#endif

        result_sync_cache = sync_user_cache(user_id);
        ex_log(LOG_DEBUG, "sync_user_cache return = %d", result_sync_cache);
    }
    thread_unlock_operation();
    return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_FAILED;
}

int cpt_set_data_path(unsigned int data_type, const char* data_path, unsigned int path_len) {
    int retval;

    ex_log(LOG_INFO, "cpt_set_data_path data_type = %u, data_path = %s, path_len = %u , %d",
           data_type, data_path, path_len, MAX_PATH_LEN);

    mem_set(g_user_path, 0, MAX_PATH_LEN);
    strncpy(g_user_path, data_path, path_len);

    if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_set_data_path another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }

    if (1 == data_type) {
        ex_log(LOG_DEBUG, "cpt_set_data_path set_log_path");
    }

    retval = opm_set_data_path(data_type, data_path, path_len);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "opm_set_data_path return = %d", retval);
    }
    thread_unlock_operation();

    return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_FAILED;
}

int cpt_chk_secure_id(unsigned int user_id, unsigned long long secure_id) {
    int retval = 0;
    retval = opm_chk_secure_id(user_id, secure_id);
    if (retval != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "check secure id failed!");
        return retval;
    }

    retval = opt_receive_data(TYPE_RECEIVE_USER_INFO, (unsigned char*)&g_cache_info.user_id,
                              sizeof(g_cache_info.user_id), NULL, 0);
    ex_log(LOG_DEBUG, "receive user info return %d", retval);
    return retval;
}

int cpt_pre_enroll(fingerprint_enroll_info_t enroll_info) {
    BOOL check_res;
    int retval;

    if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_pre_enroll another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }

#ifdef TEMPLATE_UPGRADE_FROM_SAVED_IMAGE
    plat_thread_release(&g_upgrade_test_handle);
#endif

    retval = sync_user_cache(enroll_info.fingerprint_info.user_id);
    if (retval != FINGERPRINT_RES_SUCCESS) {
        goto EXIT;
    }

    check_res = check_fingerprint_id_available(enroll_info.fingerprint_info.fingerprint_id);
    if (TRUE != check_res) {
        ex_log(LOG_ERROR, "fingerprint_id is not available");
        retval = FINGERPRINT_RES_DUPLICATE_ID;
        goto EXIT;
    }

    check_res = check_enrollment_space_available();
    if (TRUE != check_res) {
        ex_log(LOG_ERROR, "there's no space for enrollment");
        retval = FINGERPRINT_RES_NO_SPACE;
    }

    g_enroll_info = enroll_info;

EXIT:
    thread_unlock_operation();

    return retval;
}

int cpt_enroll() {
    if (TRUE != g_hardware_ready) {
        return FINGERPRINT_RES_HW_UNAVALABLE;
    }

#ifdef __NEED_COPY_CB_FILE__
    opt_send_data(TYPE_COPY_CALI_DATA, NULL, 0);
#endif

    uint64_t current_time = get_ca_time_ms();

    opm_set_data(TYPE_SEND_CA_TIME, (unsigned char*)&current_time, sizeof(uint64_t));

    plat_mutex_unlock(g_thread_power_on_lock);

    int retval = opm_enroll_initialize();
    if (retval != 0) {
        notify(EVENT_ENROLL_FAILED, 0, 0, NULL, 0);
        if (retval == EGIS_BKG_NOT_EXIST) {
            return retval;
        }
        return FINGERPRINT_RES_SUCCESS;
    }

    thread_manager_run_task((do_operation_callback)do_enroll, TASK_PROCESS);

    return FINGERPRINT_RES_SUCCESS;
}

int cpt_post_enroll() {
    if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_post_enroll another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }
    thread_unlock_operation();

    return FINGERPRINT_RES_SUCCESS;
}

int cpt_chk_auth_token(unsigned char* token, unsigned int len) {
    return opm_chk_auth_token(token, len);
}

int cpt_get_authenticator_id(unsigned long long* id) {
    int retval = opm_get_authenticator_id(id);
    if (retval != FINGERPRINT_RES_SUCCESS || *id == 0) {
        ex_log(LOG_ERROR, "opm_get_authenticator_id %d", retval);
        *id = g_cache_info.authenticator_id;
    }
    return FINGERPRINT_RES_SUCCESS;
}

int cpt_authenticate(fingerprint_verify_info_t verify_info) {
    if (g_cache_info.fingerprint_ids_count <= 0) {
        egislog_d("%s,g_cache_info.fingerprint_ids_count=%d", __func__,
                  g_cache_info.fingerprint_ids_count);
        return FINGERPRINT_RES_SUCCESS;
    }
    egislog_d("%s", __func__);
    if (TRUE != g_hardware_ready) {
        return FINGERPRINT_RES_HW_UNAVALABLE;
    }

#ifdef TEMPLATE_UPGRADE_FROM_SAVED_IMAGE
    plat_thread_release(&g_upgrade_test_handle);
#endif

#ifdef __NEED_COPY_CB_FILE__
    opt_send_data(TYPE_COPY_CALI_DATA, NULL, 0);
#endif

    uint64_t current_time = get_ca_time_ms();

    opm_set_data(TYPE_SEND_CA_TIME, (unsigned char*)&current_time, sizeof(uint64_t));

    plat_mutex_unlock(g_thread_power_on_lock);

    g_verify_info = verify_info;

    thread_manager_run_task((do_operation_callback)do_verify, TASK_PROCESS);
    if (g_cache_info.fingerprint_ids_count <= 0) {
        egislog_d("detect fingerprint_ids_count <= 0");
        thread_manager_cancel_task();
        return FINGERPRINT_RES_SUCCESS;
    }

    return FINGERPRINT_RES_SUCCESS;
}

int cpt_remove_fingerprint(fingerprint_remove_info_t remove_info) {
    int retval, result_sync_cache, fid_size;

    if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_remove_fingerprint another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }

    retval = opm_remove_fingerprint(remove_info);

    if ((retval == EX_RES_SUCCESS) || (retval == FP_LIB_ERROR_INVALID_FINGERID)) {
        retval = FINGERPRINT_RES_SUCCESS;
    } else {
        retval = FINGERPRINT_RES_FAILED;
    }
    /*
    ** reset cache user_id to default
    ** then sync the fingerprints info to the local cache
    */
    g_cache_info.user_id = -1;

    result_sync_cache = sync_user_cache(remove_info.fingerprint_info.user_id);
    if (FINGERPRINT_RES_SUCCESS != result_sync_cache) {
        ex_log(LOG_ERROR, "cpt_remove_fingerprint sync_user_cache return = %d", result_sync_cache);
    }

    fid_size = sizeof(remove_info.fingerprint_info.fingerprint_id);

    ex_log(LOG_DEBUG, "cpt_remove_fingerprint fingerid  %d",
           remove_info.fingerprint_info.fingerprint_id);
    opt_receive_data(TYPE_RECEIVE_USER_INFO, (unsigned char*)&remove_info.fingerprint_info.user_id,
                     sizeof(remove_info.fingerprint_info.user_id), NULL, 0);
    opt_receive_data(TYPE_DELETE_TEMPLATE,
                     (unsigned char*)&remove_info.fingerprint_info.fingerprint_id, fid_size, NULL,
                     0);
    thread_unlock_operation();

    return retval;
}

int cpt_get_fingerprint_ids(unsigned int user_id, fingerprint_ids_t* fps) {
    egislog_d("%s", __func__);
    if (FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_get_fingerprint_ids another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }

    /* checks if local cache is updated
    ** if it isnt updated, make it synchronized*/
    int retval = sync_user_cache(user_id);
    if (retval != FINGERPRINT_RES_SUCCESS) {
        ex_log(LOG_ERROR, "cpt_get_fingerprint_ids sync_user_cache return = %d", retval);
        goto EXIT;
    }

    fps->fingerprint_ids_count = g_cache_info.fingerprint_ids_count;
    mem_move(fps->fingerprint_ids, g_cache_info.fingerprint_ids,
             g_cache_info.fingerprint_ids_count * sizeof(unsigned int));
EXIT:
    thread_unlock_operation();
    egislog_i("%s fingercount %d %d", __func__, retval, g_cache_info.fingerprint_ids_count);
    return FINGERPRINT_RES_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_FAILED;
}

void cpt_set_event_callback(event_callbck_t on_event_callback) {
    g_event_callback = on_event_callback;
    ex_log(LOG_DEBUG, "cpt_set_event_callback = %p, size = %lu", g_event_callback,
           sizeof(event_callbck_t));
#ifdef __OTG_SENSOR__
    opm_set_data(TYPE_SEND_CALLBACK_FUNCTION, (unsigned char*)&g_event_callback,
                 sizeof(event_callbck_t));
#endif
}

static void get_ca_version(unsigned char* version, int* len) {
    if (version == NULL || len == NULL) {
        return;
    }
    int version_len = 0;
    version_len = sizeof(VENDOR_ET7XX_SDK_VERSION);
    if (version_len <= *len) {
        *len = version_len;
        mem_move(version, VENDOR_ET7XX_SDK_VERSION, version_len);
    }

    ex_log(LOG_DEBUG, "version_len = %d , version = %s", *len, version);
}

int cpt_extra_api(int type, unsigned char* in_data, int in_data_size, unsigned char* out_buffer,
                  int* out_buffer_size) {
    int retval = FINGERPRINT_RES_SUCCESS, cid = 0;

    if (in_data == NULL || in_data_size < 1) {
        // Bypass to  opm_extra_command();
        ex_log(LOG_DEBUG, "%s [%d] in_data_size %d", __func__, type, in_data_size);
        retval = thread_try_lock_operation();
        if (retval != 0) {
            ex_log(LOG_ERROR, "Failed try lock. retval=%d", retval);
            return FINGERPRINT_RES_NOT_IDLE;
        }
        retval = opm_extra_command(type, in_data, in_data_size, out_buffer, out_buffer_size);
        thread_unlock_operation();
        ex_log(retval == 0 ? LOG_DEBUG : LOG_ERROR, "cpt_extra_api end, cid = %d, retval=%d", cid,
               retval);
        return retval;
    }

    uint8_t* extra_data = NULL;
    int extra_data_size = 0;
    if (in_data_size < 4) {
        ex_log(LOG_ERROR, "%s, in_data_size %d < 4", __func__, in_data_size);
        cid = in_data[0];
    } else {
        cid = *(int*)in_data;
        if (in_data_size > 4) {
            extra_data = in_data + 4;
            extra_data_size = in_data_size - 4;
        }
    }
    ex_log(LOG_DEBUG, "cpt_extra_api [%d], cid = %d, in_data_size = %d", type, cid, in_data_size);

    BOOL needLockThread;
    switch (type) {
        case PID_HOST_TOUCH:
            needLockThread = FALSE;
            break;

        case PID_INLINETOOL:
            if (SENSORTEST_IMAGE_QTY == cid || cid == SENSORTEST_AGING_TEST) {
                needLockThread = FALSE;
            } else {
                needLockThread = TRUE;
            }
            break;

        default:
            needLockThread = TRUE;
            break;
    }

    if (needLockThread && FINGERPRINT_RES_SUCCESS != thread_try_lock_operation()) {
        ex_log(LOG_ERROR, "cpt_extra_api another operation is doing ,return not idle");
        return FINGERPRINT_RES_NOT_IDLE;
    }

    switch (type) {
#if defined(__ET7XX__) && !defined(__ET0XX__)
        case PID_7XX_INLINETOOL: {
            if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_ERROR, "power on sensor fail");
                retval = FINGERPRINT_RES_FAILED;
                goto EXIT;
            }

            retval = do_7XX_sensortest(cid, 0, 0, 0, out_buffer, out_buffer_size);
            ex_log(LOG_DEBUG, "cid=%d, retval=%d", cid, retval);

            if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_ERROR, "power off sensor fail");
                retval = FINGERPRINT_RES_FAILED;
                goto EXIT;
            }
            usleep(50*1000);
            goto EXIT;
        }
#endif
        case PID_INLINETOOL:
            ex_log(LOG_DEBUG, "cid=%d", cid);
            if (SENSORTEST_IMAGE_QTY == cid) {
                ex_log(LOG_DEBUG, "SENSORTEST_IMAGE_QTY, skip open power");
                thread_manager_run_task((do_operation_callback)do_image_qty_test, TASK_PROCESS);
                retval = FINGERPRINT_RES_SUCCESS;
                goto EXIT;
            } else if (SENSORTEST_AGING_TEST == cid) {
                ex_log(LOG_DEBUG, "SENSORTEST_AGING_TEST, skip open power");
                thread_manager_run_task((do_operation_callback)do_sensor_aging_test, TASK_PROCESS);
                retval = FINGERPRINT_RES_SUCCESS;
                goto EXIT;
            }
        case PID_FAETOOL: {
            if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_ERROR, "power on sensor fail");
                retval = FINGERPRINT_RES_FAILED;
                goto EXIT;
            }

            retval = do_sensortest(cid, in_data, in_data_size, out_buffer, out_buffer_size);

            if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_ERROR, "power off sensor fail");
                retval = FINGERPRINT_RES_FAILED;
                goto EXIT;
            }
            goto EXIT;
        }

        case PID_COMMAND: {
            if (CMD_UPDATE_CONFIG == cid) {
                destroy_ini_config();
                if ((uint32_t)in_data_size > sizeof(int)) {
                    retval =
                        create_ini_config(FALSE, in_data + sizeof(int), in_data_size - sizeof(int));
                } else {
                    retval = create_ini_config(FALSE, NULL, 0);
                }

                goto EXIT;
            } else if (CMD_GET_CONFIG == cid) {
                if (out_buffer != NULL && out_buffer_size != NULL) {
                    retval = opt_receive_data(TYPE_RECEIVE_INI_CONFIG, NULL, 0, out_buffer,
                                              out_buffer_size);
                }
                goto EXIT;
            } else if (CMD_UPDATE_DB_CONFIG == cid) {
                if ((uint32_t)in_data_size > sizeof(int)) {
                    retval = opt_send_data(TYPE_SEND_DB_INI_CONFIG, in_data + sizeof(int),
                                           in_data_size - sizeof(int));
                } else {
                    retval = FINGERPRINT_RES_FAILED;
                    ex_log(LOG_ERROR, "Failed to update db config ini");
                }
                goto EXIT;
            }

            if (CMD_SET_NAVI_CONFIG == cid) {
                /*g_navi_config.navi_mode = ((int*)in_data)[0];
                g_navi_config.change_x_y = ((int*)in_data)[1];
                g_navi_config.change_up_down = ((int*)in_data)[2];
                g_navi_config.change_left_right = ((int*)in_data)[3];
                goto EXIT;*/
            }

            if (CMD_REMOVE_INI_FILE == cid) {
                retval = opt_send_data(TYPE_REMOVE_INI_FILE, NULL, 0);
                goto EXIT;
            }
        }

        case PID_DEMOTOOL: {
            switch (cid) {
                case CMD_VERSION_CA:
                    get_ca_version(out_buffer, out_buffer_size);
                    break;
                case CMD_VERSION_TA:
                    retval =
                        opm_get_data(TYPE_RECEIVE_TA_VERSION, NULL, 0, out_buffer, out_buffer_size);
                    break;
                case CMD_VERSION_ALGO:
                    retval = opm_get_data(TYPE_RECEIVE_ALGO_VERSION, NULL, 0, out_buffer,
                                          out_buffer_size);
                    break;
                case CMD_VERSION_IP:
                    retval =
                        opm_get_data(TYPE_RECEIVE_IP_VERSION, NULL, 0, out_buffer, out_buffer_size);
                    break;
                case EXTRA_DT_SET_INI_CONFIG_PATH:
                    retval = opt_send_data(TYPE_SEND_INI_CONFIG_PATH, extra_data, extra_data_size);
                    break;
                default:
                    break;
            }
            goto EXIT;
        }
        case PID_BKG_IMG: {
            switch (cid) {
                case CMD_BKG_IMG_RUN_CALI_PROC:
                    if (do_power_on(SENSOR_PWR_ON) != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "power on sensor fail");
                        retval = FINGERPRINT_RES_FAILED;
                        goto EXIT;
                    }
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_RUN_CALI_PROC");
                    retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE, FPS_CALI_ET7XX_BKG,
                                                    0, 0, NULL, NULL);
                    if (retval == FP_LIB_OK) {
                        retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE,
                                                        FPS_SAVE_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
                        ex_log(LOG_DEBUG, "FPS_SAVE_ET7XX_CALI_DATA, retval=%d", retval);
                        retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE,
                                                        FPS_LOAD_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
                        ex_log(LOG_DEBUG, "FPS_LOAD_ET7XX_CALI_DATA, retval=%d", retval);
                        ex_log(LOG_DEBUG, "TYPE_RECEIVE_CALIBRATION_DATA Start");
                        retval = opt_receive_data(TYPE_RECEIVE_CALIBRATION_DATA, NULL, 0, NULL, 0);
                        ex_log(LOG_DEBUG, "TYPE_RECEIVE_CALIBRATION_DATA, retval =%d", retval);
                    }
                    if (do_power_on(SENSOR_PWR_OFF) != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "power off sensor fail");
                        retval = FINGERPRINT_RES_FAILED;
                        goto EXIT;
                    }
                    break;
                case CMD_BKG_IMG_SAVE_CALI_DATA:
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_SAVE_CALI_DATA file path = %s",
                           in_data + sizeof(int));
                    retval = opm_set_data(TYPE_IC_SAVE_CALIBRATION_DATA, in_data + sizeof(int),
                                          in_data_size - sizeof(int));
                    break;
                case CMD_BKG_IMG_LOAD_CALI_DATA:
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_LOAD_CALI_DATA file path = %s",
                           in_data + sizeof(int));
                    retval = opm_set_data(TYPE_IC_LOAD_CALIBRATION_DATA, in_data + sizeof(int),
                                          in_data_size - sizeof(int));
                    break;
                case CMD_BKG_IMG_GET_CALI_IMAGE: {
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_GET_CALI_IMAGE");
                    int cali_image_size =
                        sizeof(liver_image_out_header_t) + MAX_IMAGE_BUFFER_SIZE * 2;
                    unsigned char* cali_image = malloc(cali_image_size);
                    if (cali_image != NULL) {
                        retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_GET_CALI_IMAGE, 0, 0, 0,
                                                        cali_image, &cali_image_size);
                        notify(EVENT_RETURN_LIVE_IMAGE_OUT, 0, 0, cali_image, cali_image_size);
                        if (cali_image != NULL) {
                            free(cali_image);
                            cali_image = NULL;
                        }
                    } else {
                        ex_log(LOG_ERROR, "failed to allocate %d", cali_image_size);
                        retval = FINGERPRINT_RES_ALLOC_FAILED;
                    }
                    break;
                }
                case CMD_BKG_IMG_RETURN_IMAGE: {
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_RETURN_IMAGE");
                    retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_GET_IMAGE, 0, 0, 0, out_buffer,
                                                    out_buffer_size);
                    break;
                }
                case CMD_BKG_IMG_REMOVE_CALI_DATA:
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_REMOVE_CALI_DATA");
                    retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE,
                                                    FPS_REMOVE_ET7XX_CALI_DATA, 0, 0, NULL, NULL);
                    break;
                case CMD_BKG_IMG_REMOVE_BDS:
                    ex_log(LOG_DEBUG, "CMD_BKG_IMG_REMOVE_BDS");
                    retval = flow_inline_legacy_cmd(FP_INLINE_SENSOR_CALIBRATE,
                                                    FPS_REMOVE_ET7XX_BDS, 0, 0, NULL, NULL);
                    break;
                default:
                    ex_log(LOG_ERROR, "unsupported cid : %d", cid);
                    break;
            }
            goto EXIT;
        }
        case PID_HOST_TOUCH: {
            switch (cid) {
                case CMD_HOST_TOUCH_ENABLE:
                    host_touch_set_enable(TRUE);
                    break;
                case CMD_HOST_TOUCH_PARTIAL_TOUCHING:
                    host_touch_set_finger_on(TRUE);
                    host_touch_set_low_coverage(TRUE);
                    break;
                case CMD_HOST_TOUCH_SET_TOUCHING: {
                    host_touch_set_finger_on(TRUE);
                    break;
                }
                case CMD_HOST_TOUCH_RESET_TOUCHING:
                    host_touch_set_finger_off(TRUE);
                    break;
                case CMD_HOST_TOUCH_SEND_TEMPERATURE: {
                    host_device_info_t info;
                    info.temperature_x10 = *((int*)in_data + 1);
                    ex_log(LOG_DEBUG, "ignore app temperature: %d", info.temperature_x10);
                    // set_host_device_info(info);
                    break;
                }

                case CMD_HOST_TOUCH_SET_UI_READY: {
                    host_touch_set_ui_ready(TRUE);
                    break;
                }
                case CMD_HOST_TOUCH_CUSTOM_SCREEN_STATE_ON: {
                    g_screen_state = TRUE;
                    ex_log(LOG_DEBUG, "g_screen_state %d", g_screen_state);
                    break;
                }
                case CMD_HOST_TOUCH_CUSTOM_SCREEN_STATE_OFF: {
                    g_screen_state = FALSE;
                    ex_log(LOG_DEBUG, "g_screen_state %d", g_screen_state);
                    break;
                }

                case CMD_DEMOTOOL_SET_SCREEN_BRIGHTNESS: {
#if defined(__ET7XX__) && !defined(__ET0XX__)
                    int value = *((int*)in_data + 1);
                    ex_log(LOG_DEBUG, "set brightness: %d", value);
                    retval = set_brightness(value);
#endif
                    goto EXIT;
                }
                default:
                    ex_log(LOG_ERROR, "unsupported cid : %d", cid);
                    break;
            }
            retval = FINGERPRINT_RES_SUCCESS;
            goto EXIT;
        }
        case PID_SET_ENROLL_TIMEOUT: {
            g_enroll_timeout = cid;
            ex_log(LOG_DEBUG, "set enroll timeout = %d ", g_enroll_timeout);
            goto EXIT;
        }
        case PID_EVTOOL: {
            const int PARAM_BUF_ENROLLED_PATH = 1000;
            const int PARAM_BUF_VERIFIED_PATH = 1001;
            const int PARAM_INT_DB_TOTAL_FINGER = 730;
            const int PARAM_BUF_DB_TOTAL_FINGERPRINT = 731;
            switch (cid) {
                case CMD_GET_ENROLL_FILE_NAME: {
                    int param = PARAM_BUF_ENROLLED_PATH;
                    retval = opm_get_data(TYPE_RECEIVE_SENSOR_BUF, (uint8_t*)&param, sizeof(param),
                                          out_buffer, out_buffer_size);
                } break;
                case CMD_GET_VERIFY_FILE_NAME: {
                    int param = PARAM_BUF_VERIFIED_PATH;
                    retval = opm_get_data(TYPE_RECEIVE_SENSOR_BUF, (uint8_t*)&param, sizeof(param),
                                          out_buffer, out_buffer_size);
                } break;
                case CMD_GET_MATCH_SCORE: {
                    retval = opm_get_data(TYPE_EVTOOL_RECEIVE_MATCH_SCORE, NULL, 0, out_buffer,
                                          out_buffer_size);
                } break;
                case CMD_INT_DB_TOTAL_FINGER: {
                    int param = PARAM_INT_DB_TOTAL_FINGER;
                    retval = opm_get_data(TYPE_RECEIVE_SENSOR_INT_PARAM, (uint8_t*)&param,
                                          sizeof(param), out_buffer, out_buffer_size);
                } break;
                case CMD_BUF_DB_TOTAL_FINGERPRINT: {
                    int param = PARAM_BUF_DB_TOTAL_FINGERPRINT;
                    retval = opm_get_data(TYPE_RECEIVE_SENSOR_BUF, (uint8_t*)&param, sizeof(param),
                                          out_buffer, out_buffer_size);
                } break;
                default:
                    ex_log(LOG_ERROR, "unsupported pid : %d, cid : %d", type, cid);
                    break;
            }
            goto EXIT;
        }

#if defined(__ET7XX__)
        case PID_SYSUNLOCKTOOL_NAME: {
            char *str, *delim = "/";
            char tmp[64];
            g_use_sysunlock_imagetool = TRUE;

            memset(g_username, 0, sizeof(g_username));
            memset(g_fingerid, 0, sizeof(g_fingerid));
            memset(g_case, 0, sizeof(g_case));
            memset(g_test_case_id, 0, sizeof(g_test_case_id));
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, in_data, sizeof(tmp));
            ex_log(LOG_DEBUG, "tmp = %s", tmp);

            str = strtok(tmp, delim);
            ex_log(LOG_DEBUG, "str = %s", str);
            if (str) {
                memcpy(g_test_case_id, str, strlen(str) + 1);
                ex_log(LOG_DEBUG, "g_test_case_id = %s", g_test_case_id);
                str = strtok(NULL, delim);

                memcpy(g_username, str, strlen(str) + 1);
                str = strtok(NULL, delim);

                memcpy(g_fingerid, str, strlen(str) + 1);
                ex_log(LOG_DEBUG, "g_fingerid = %s", g_fingerid);
                str = strtok(NULL, delim);

                memcpy(g_case, str, strlen(str) + 1);
                ex_log(LOG_DEBUG, "g_case = %s", g_case);
                reset_finger_count();
            }

            goto EXIT;
        }
        case PID_SYSUNLOCKTOOL_TOTAL_COUNT: {
            retval = get_total_count();
            goto EXIT;
        }
        case PID_SYSUNLOCKTOOL_MATCH_COUNT: {
            retval = get_match_count();
            goto EXIT;
        }
        case PID_SYSUNLOCKTOOL_NOT_MATCH_COUNT: {
            retval = get_not_match_count();
            goto EXIT;
        }
#endif
    }

    ex_log(LOG_VERBOSE, "opm_extra_command");
    retval = opm_extra_command(type, in_data, in_data_size, out_buffer, out_buffer_size);

EXIT:
    if (needLockThread) {
        thread_unlock_operation();
    }
    ex_log(LOG_DEBUG, "cpt_extra_api end, cid = %d, retval=%d", cid, retval);

    return retval;
}
#if 0
static int touch_enroll_remaining(int percentage)
{
    ex_log(LOG_DEBUG, "%s enter! percentage = %d", __func__, percentage);
    if (g_enroll_percentage_tmp != percentage) {
        g_has_enroll_count++;
        g_enroll_percentage_tmp = percentage;
    }
    int remaining;
    if (percentage >= 100) {
        g_enroll_percentage_tmp = 0;
        g_has_enroll_count = 0;
        remaining = 0;
    } else if (percentage > 0) {
        int total = (int)(100 / (float)percentage * g_has_enroll_count);
        remaining = total - g_has_enroll_count;
        ex_log(LOG_DEBUG, "%s has_enroll_count=%d, total=%d, remaining=%d", __func__, g_has_enroll_count, total, remaining);
        if (remaining < 1) {
            remaining = 1;
        }
    } else {
        remaining = g_enroll_config.enroll_max_count;
    }
    return remaining;
}
#endif
static int enroll_percentage_to_remaining(int percentage) {
    int retval = -1;
    retval = touch_enroll_remaining_no_percentage(percentage);
    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
    return retval;
}

static int do_sensortest(int cid, unsigned char* in_data, int in_data_size,
                         unsigned char* out_buffer, int* out_buffer_size) {
    ex_log(LOG_DEBUG, "%s enter! cid = %d", __func__, cid);
    int retval = FINGERPRINT_RES_FAILED;
    int buffer_size = MAX_IMAGE_BUFFER_SIZE * 2;

    ex_log(LOG_DEBUG, "%s, out buffer (%p) %d", __func__, out_buffer,
           out_buffer_size ? *out_buffer_size : 0);

    unsigned char* buffer;
    BOOL use_local_buffer;
    if (out_buffer != NULL && out_buffer_size != NULL && *out_buffer_size > 0) {
        buffer = out_buffer;
        buffer_size = *out_buffer_size;
        use_local_buffer = FALSE;
    } else {
        buffer = malloc(MAX_IMAGE_BUFFER_SIZE * 2);
        use_local_buffer = TRUE;
    }

    if (buffer != NULL) {
        g_need_cancel = FALSE;

        retval = sensor_test_opation(cid, g_hdev, in_data, in_data_size, buffer, &buffer_size);
        if (SENSORTEST_GET_IMAGE == cid && FINGERPRINT_RES_SUCCESS == retval) {
            if (use_local_buffer) {
                notify(EVENT_RETURN_IMAGE, 0, 0, buffer, buffer_size);
            }
        }

        if (SENSORTEST_GET_NVM_UID == cid && FINGERPRINT_RES_SUCCESS == retval) {
            notify(EVENT_SENSROR_TEST_NVM_UID, buffer_size, 0, buffer, buffer_size);
        }
    }

    if (use_local_buffer) {
        if (buffer != NULL) {
            free(buffer);
            buffer = NULL;
        }
    }
    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

    return retval;
}

int cpt_pause(void) {
    g_need_pause = TRUE;
    return FINGERPRINT_RES_SUCCESS;
}

int cpt_continue(void) {
    host_touch_set_ui_ready(FALSE);
    host_touch_set_finger_off(TRUE);
    host_touch_set_finger_on(FALSE);
    g_need_pause = FALSE;
    return FINGERPRINT_RES_SUCCESS;
}

int cpt_set_module_info(void)
{
    uint32_t i = 0;
    uint32_t moduleType = 0;
    uint32_t lensType = 0;
    uint32_t moduleInfoSize = 0;
    char lensTypeStr[4] = { 0 };
    char moduleInfoStr[100] = { 0 };

    do {
        get_module_info(&moduleType, &lensType);
        moduleInfoSize = sizeof(module_info_list)/sizeof(module_info_list[0]);

        for (i = 0; i < moduleInfoSize; i++) {
            if (moduleType == module_info_list[i].module) {
                break;
            }
        }

        if (i == moduleInfoSize) {
            property_set(PROPERTY_FINGERPRINT_FACTORY, "Unknown");
            ex_log(LOG_ERROR, "[%s] unkown module", __func__);
            break;
        }
        sprintf(lensTypeStr, "%X", lensType);
        strncpy(moduleInfoStr, module_info_list[i].module_info_string, strlen(module_info_list[i].module_info_string));
        strcat(moduleInfoStr, lensTypeStr);
        strcat(moduleInfoStr, "_ET713");
        property_set(PROPERTY_FINGERPRINT_FACTORY, moduleInfoStr);
    } while (0);

    return 0;
}

static long getCurrentTime_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void get_time_now(char* str_time) {
    time_t timep;
    struct tm* p;

    time(&timep);
    p = localtime(&timep);
    sprintf(str_time, "%02d-%02d-%02d.%02d.%02d.%03d", (1 + p->tm_mon), p->tm_mday, p->tm_hour,
            p->tm_min, p->tm_sec, (int)(getCurrentTime_ms() % 1000));
}

long get_ca_time_ms(void) {
    time_t timep;
    struct tm* p;
    long time_ms;
    time(&timep);
    p = localtime(&timep);
    time_ms = ((p->tm_hour) * 60 * 60 * 1000) + (p->tm_min * 60 * 1000) + (p->tm_sec * 1000) +
        (getCurrentTime_ms() % 1000);
    return time_ms;
}

int do_power_on(pwr_status_t pwr) {
    plat_mutex_lock(pwr_lock);
    int ret = FINGERPRINT_RES_SUCCESS;

    ex_log(LOG_DEBUG, "set pwr = %s", pwr == SENSOR_PWR_ON ? "SENSOR_PWR_ON" : "SENSOR_PWR_OFF");

    if (pwr == g_power_managemen.pwr_status) {
        ex_log(LOG_DEBUG, "already %s go exit!!",
               pwr == SENSOR_PWR_ON ? "SENSOR_PWR_ON" : "SENSOR_PWR_OFF");
        ret = FINGERPRINT_RES_SUCCESS;
        goto EXIT;
    }

    if (pwr == SENSOR_PWR_ON) {
        ret = fp_device_power_control(g_hdev, true);

        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_power_control on failed");
            goto EXIT;
        }

        setSpiState(TRUE, FALSE);

        ret = fp_device_spipin_enable(g_hdev, TRUE);
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_spipin_enable TRUE failed");
            fp_device_power_control(g_hdev, false);
            setSpiState(FALSE, FALSE);
            goto EXIT;
        }

        fp_device_poweron_reset(g_hdev); //2,2,13,8,

        ret = fp_device_clock_enable(g_hdev, TRUE);
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_clock_enable TRUE failed");
            fp_device_power_control(g_hdev, false);
            setSpiState(FALSE, FALSE);
            goto EXIT;
        }
        ret = opm_initialize_sensor_hw();

        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "opm_initialize_sensor_hw return = %d", ret);
            fp_device_power_control(g_hdev, false);
            setSpiState(FALSE, FALSE);
            goto EXIT;
        }
    } else {
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "opm_uninitialize_sensor_hw return = %d", ret);
            goto EXIT;
        }

        setSpiState(FALSE, FALSE);

        fp_device_clock_enable(g_hdev, FALSE);

        ret = fp_device_reset_set(g_hdev, 0); /* reset Low*/
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_reset_set LOW failed");
            goto EXIT;
        }

        ret = fp_device_spipin_enable(g_hdev, FALSE);
        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_spipin_enable FALSE failed");
            goto EXIT;
        }

        ret = fp_device_power_control(g_hdev, false);

        if (FINGERPRINT_RES_SUCCESS != ret) {
            ex_log(LOG_ERROR, "fp_device_power_control off failed");
            goto EXIT;
        }
    }

    g_power_managemen.pwr_status = pwr;

EXIT:
    ex_log(LOG_DEBUG, "g_power_managemen.pwr_status = %s",
           g_power_managemen.pwr_status == SENSOR_PWR_ON ? "SENSOR_PWR_ON" : "SENSOR_PWR_OFF");
    plat_mutex_unlock(pwr_lock);
    return ret;
}

/* not merge to vivo server*/
void sprint_algoinfo(BigDataInfo_t* bigDataInfo) {
    if (bigDataInfo != NULL) {
        ex_log(LOG_INFO, "bigDataInfo->type = %d,bigDataInfo->cmd_status=%d, userid = %d, time = %s", bigDataInfo->ancAlgoFingerInfo.type,
               bigDataInfo->ancAlgoFingerInfo.cmd_status, g_user_info_value, bigDataInfo->time);
        ex_log(LOG_INFO,
               "bigDataInfo->enrolled_template_count = %d, bigDataInfo->matched_template_idx = %d, "
               "bigDataInfo->compress_cls_score = %d, bigDataInfo->is_studied = %d, "
               "bigDataInfo->compare_final_score = %d, bigDataInfo->compress_tnum_score = %d, "
               "bigDataInfo->compress_area_score = %d, bigDataInfo->cur_rot_angle = %d, bigDataInfo->classify_score "
               "= %d, bigDataInfo->compress_island_score = %d",
               bigDataInfo->ancAlgoFingerInfo.enrolled_template_count,
               bigDataInfo->ancAlgoFingerInfo.matched_template_idx,
               bigDataInfo->ancAlgoFingerInfo.compress_cls_score,
               bigDataInfo->ancAlgoFingerInfo.is_studied,
               bigDataInfo->ancAlgoFingerInfo.compare_final_score,
               bigDataInfo->ancAlgoFingerInfo.compress_tnum_score,
               bigDataInfo->ancAlgoFingerInfo.compress_area_score,
               bigDataInfo->ancAlgoFingerInfo.cur_rot_angle,
               bigDataInfo->ancAlgoFingerInfo.classify_score,
               bigDataInfo->ancAlgoFingerInfo.compress_island_score);
        ex_log(
            LOG_INFO,
            "bigDataInfo->finger_quality_score = %d, bigDataInfo->mat_d = %d, bigDataInfo->finger_live_score = %d, "
            "bigDataInfo->finger_light_score = %d, bigDataInfo->finger_status_score = %d, "
            "bigDataInfo->finger_strange_score = %d, bigDataInfo->temperature = %d, "
            "bigDataInfo->in_bad_qty_img = %d, bigDataInfo->retry_count = %d, bigDataInfo->source_id = %d "
            "bigDataInfo->img_variance = %d, bigDataInfo->img_contrast = %d, bigDataInfo->mat_s = %d, bigDataInfo->matched_finger_id = %d",
            bigDataInfo->ancAlgoFingerInfo.finger_quality_score,
            bigDataInfo->ancAlgoFingerInfo.mat_d,
            bigDataInfo->ancAlgoFingerInfo.finger_live_score,
            bigDataInfo->ancAlgoFingerInfo.finger_light_score,
            bigDataInfo->ancAlgoFingerInfo.finger_status_score,
            bigDataInfo->ancAlgoFingerInfo.finger_strange_score,
            bigDataInfo->ancAlgoFingerInfo.temperature,
            bigDataInfo->ancAlgoFingerInfo.in_bad_qty_img,
            bigDataInfo->ancAlgoFingerInfo.retry_count,
            bigDataInfo->ancAlgoFingerInfo.source_id,
            bigDataInfo->ancAlgoFingerInfo.img_variance,
            bigDataInfo->ancAlgoFingerInfo.img_contrast,
            bigDataInfo->ancAlgoFingerInfo.mat_s,
            bigDataInfo->ancAlgoFingerInfo.matched_finger_id);
    } else {
        ex_log(LOG_INFO, "bigDataInfo = NULL");
    }
}
/* not merge to vivo server*/
