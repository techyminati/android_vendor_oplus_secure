#include "op_sensortest.h"

#include <stdlib.h>
#include <string.h>

#include "common_definition.h"
#include "device_int.h"
#include "egis_rbs_api.h"
#include "egis_save_to_bmp.h"
#include "fp_definition.h"
#include "fps_normal.h"
#include "op_manager.h"
#include "plat_file.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "plat_time.h"
#include "response_def.h"
#if defined(__ET7XX__) && !defined(__ET0XX__)
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "7xx_sensor_test_definition.h"
#include "captain.h"
#include "opt_file.h"
extern event_callbck_t g_event_callback;
#endif
#define FP_SENSORTEST_FINGERON_COUNT 3
#define SENSORTEST_DEFAULT_TIEM_OUT 6 * 1000
#define SENSOR_NEED_RESET 21
#define LITTLE_TO_BIG_ENDIAN(x) (((x >> 8) & 0xFF) + ((x << 8) & 0xFF00))
#define OPLUS_INLINE_TEST_ERROR -1
#define OPLUS_INLINE_TEST_SUCCESS 0
#define INLINE_CHECK_OVER_SPEC(val, max, min) ((val > max) ? 1 : ((val < min) ? 1 : 0))

extern int g_hdev;
extern app_instance_type_t g_app_type;

#if defined(__ET7XX__) && !defined(__ET0XX__)
float g_inline_siganl = 0;
float g_inline_noise = 0;
float g_inline_snr = 0;
char g_UID[9];
float g_expo_step1 = 0;
#endif

extern pwr_status_t g_status;
extern int do_power_on(pwr_status_t pwr);

unsigned long long g_inline_timestamp = 0;

#if defined(__ET7XX__) && !defined(__ET0XX__)
static int sensortest_send_cmd_reg_rw(int cmd, unsigned char* in_data, int in_data_size,
                                      unsigned char* out_data, int* out_data_size) {
    int retval = FP_LIB_ERROR_GENERAL;
    *(int*)in_data = cmd;
    ex_log(LOG_DEBUG, "%s enter! cmd = %d, in_data[0]=%d, in_data[1]=%d, in_data[2]=%d", __func__,
           cmd, *(int*)in_data, *(((int*)in_data) + 1), *(((int*)in_data) + 2));
    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, out_data,
                               out_data_size);
    ex_log(LOG_DEBUG, "%s opm_extra_command retval = %d", __func__, retval);
    if (FP_LIB_OK == retval || SENSOR_NEED_RESET == retval) {
        memcpy(&retval, out_data, sizeof(int));
    } else {
        retval = MMI_TEST_FAIL;
    }
    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
    return retval;
}
#endif

static int sensortest_send_cmd(int cmd) {
    ex_log(LOG_DEBUG, "%s enter!", __func__);
    int retval = FP_LIB_ERROR_GENERAL;
    int in_data[2];
    memset(in_data, 0, 2);
    in_data[0] = cmd;
    int in_data_size = 2;
    unsigned char out_data[64] = {0};
    int out_data_size = 64;

    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, out_data,
                               &out_data_size);
    ex_log(LOG_DEBUG, "%s opm_extra_command retval = %d", __func__, retval);
    if (FP_LIB_OK == retval || SENSOR_NEED_RESET == retval) {
        memcpy(&retval, out_data, sizeof(int));
    } else {
        retval = MMI_TEST_FAIL;
    }
    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
    return retval;
}

static int sensortest_check_finger_on(int idev) {
    int retval = MMI_TEST_FAIL;
    int test_count = 0;

    retval = sensortest_send_cmd(FP_MMI_FOD_TEST);
    if (MMI_TEST_SUCCESS != retval) {
        retval = MMI_TEST_FAIL;
        goto EXIT;
    }

    retval = FINGERPRINT_RES_TIMEOUT;
    do {
        retval = fp_device_interrupt_enable(idev, FALSE);
        retval = fp_device_interrupt_enable(idev, TRUE);
        retval = fp_device_interrupt_wait(idev, SENSORTEST_DEFAULT_TIEM_OUT);
        test_count++;
        if (FINGERPRINT_RES_SUCCESS == retval) break;
    } while (test_count < FP_SENSORTEST_FINGERON_COUNT);

    if (FINGERPRINT_RES_SUCCESS == retval) {
        retval = sensortest_send_cmd(FP_MMI_GET_FINGER_IMAGE);
        if (MMI_TEST_SUCCESS != retval) retval = MMI_TEST_FAIL;
    } else {
        retval = MMI_TEST_FAIL;
    }

EXIT:
    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
    return retval;
}

static int sensortest_wait_finger_on(int times_ms) {
    int retval = MMI_TEST_FAIL;
    int test_count = times_ms / 30;

    retval = sensortest_send_cmd(FP_MMI_FOD_TEST);
    if (MMI_TEST_SUCCESS != retval) {
        retval = MMI_TEST_FAIL;
        goto EXIT;
    }

    if (!wait_trigger(test_count, 30, TIMEOUT_WAIT_FOREVER))
        retval = MMI_TEST_FAIL;
    else
        retval = MMI_TEST_SUCCESS;

EXIT:
    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);
    return retval;
}

static int sensortest_wait_interrupt(trigger_info_t* trigger_info) {
    int retval = MMI_TEST_FAIL;
    int try_count = 0;
    int trigger_type = TRIGGER_RESET;
    int time_interval = 30;

    if (trigger_info != NULL) {
        try_count = trigger_info->wait_time / trigger_info->time_interval;
        trigger_type = trigger_info->trigger_type;
        time_interval = trigger_info->time_interval > 0 ? trigger_info->time_interval : 30;
    }
    ex_log(LOG_DEBUG, "%s start,try_count %d,type %d ,time_interval %d ", __func__, retval,
           trigger_type, time_interval);

    fp_set_interrupt_trigger_type(trigger_type);

    if (!wait_trigger(try_count, time_interval, TIMEOUT_WAIT_FOREVER))
        retval = MMI_TEST_FAIL;
    else
        retval = MMI_TEST_SUCCESS;

    fp_set_interrupt_trigger_type(TRIGGER_RESET);
    ex_log(LOG_DEBUG, "%s end ,wait result %d ", __func__, retval);
    return retval;
}

static int oplus_sensortest_get_image_qty(unsigned char* entry_data, unsigned char* buffer,
                                         int* buffer_size) {
    if (NULL == buffer || NULL == buffer_size) {
        ex_log(LOG_DEBUG, "%s invalid param", __func__);
        return MMI_TEST_FAIL;
    }

    int image_test_type = ((int*)entry_data)[1];
    int retval = FINGERPRINT_RES_FAILED;
    int in_data[2] = {0};
    in_data[0] = FP_OPLUS_QTY;
    in_data[1] = image_test_type;
    int in_data_size = sizeof(in_data);

    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, buffer,
                               buffer_size);
    if (FP_LIB_OK == retval) {
        retval = MMI_TEST_SUCCESS;
    } else {
        retval = MMI_TEST_FAIL;
    }

    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

    return retval;
}

static int sensortest_get_image(unsigned char* entry_data, unsigned char* buffer,
                                int* buffer_size) {
    if (NULL == buffer || NULL == buffer_size) {
        ex_log(LOG_DEBUG, "%s invalid param", __func__);
        return MMI_TEST_FAIL;
    }

    int image_test_type = ((int*)entry_data)[1];
    int retval = FINGERPRINT_RES_FAILED;
    int in_data[2] = {0};
    in_data[0] = FP_CAPTURE_IMG;
    in_data[1] = image_test_type;
    int in_data_size = sizeof(in_data);

    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, buffer,
                               buffer_size);
    if (FP_LIB_OK == retval) {
        retval = MMI_TEST_SUCCESS;
    } else {
        retval = MMI_TEST_FAIL;
    }

    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

    return retval;
}

static int sensortest_set_crop_info(unsigned char* entry_data) {
    int retval = FINGERPRINT_RES_FAILED;
    unsigned char out_data[64] = {0};
    int out_data_size = 64;
    int in_data[4] = {0};
    in_data[0] = FP_MMI_SET_CROP_INFO;
    in_data[1] = ((int*)entry_data)[1];
    in_data[2] = ((int*)entry_data)[3];
    in_data[3] = ((int*)entry_data)[4];
    int in_data_size = sizeof(in_data);

    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, out_data,
                               &out_data_size);
    if (FP_LIB_OK == retval) {
        retval = MMI_TEST_SUCCESS;
    } else {
        retval = MMI_TEST_FAIL;
    }

    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

    return retval;
}

static int sensortest_get_nvm_uid(unsigned char* buffer, int* buffer_size) {
    if (NULL == buffer || NULL == buffer_size) {
        ex_log(LOG_DEBUG, "%s invalid param", __func__);
        return MMI_TEST_FAIL;
    }

    int retval = FINGERPRINT_RES_FAILED;
    int in_data[2] = {0};

    memset(in_data, 0, 2);
    in_data[0] = FP_MMI_GET_NVM_UID;
    int in_data_size = sizeof(in_data);

    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, buffer,
                               buffer_size);
    if (FP_LIB_OK == retval) {
        retval = MMI_TEST_SUCCESS;
    } else {
        retval = MMI_TEST_FAIL;
    }

    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

    return retval;
}

int sensor_test_opation(int cid, int idev, unsigned char* in_data, int in_data_size,
                        unsigned char* buffer, int* buffer_size) {
    ex_log(LOG_DEBUG, "%s enter, dev = %d, in_data_size %d", __func__, idev, in_data_size);
    int retval = MMI_TEST_FAIL;
    switch (cid) {
#if defined(__ET7XX__) && !defined(__ET0XX__)
        case SENSORTEST_READ_REG_TEST:
            retval =
                sensortest_send_cmd_reg_rw(FP_READ_REG, in_data, in_data_size, buffer, buffer_size);
            break;
        case SENSORTEST_WRITE_REG_TEST:
            retval = sensortest_send_cmd_reg_rw(FP_WRITE_REG, in_data, in_data_size, buffer,
                                                buffer_size);
            break;
        case SENSORTEST_CAPTURE_IMG_AE:
            *(int*)in_data = FP_CAPTURE_IMG_AE;
            retval = opm_extra_command(PID_INLINETOOL, in_data, in_data_size, buffer, buffer_size);
            printf("Try save bin file, out data len %d\n", *buffer_size);
            printf("out buff data 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buffer[0], buffer[1], buffer[2],
                   buffer[3], buffer[4]);
            break;
#endif
        case SENSORTEST_DIRTYDOTS_TEST:
            retval = sensortest_send_cmd(FP_MMI_DIRTYDOTS_TEST);
            break;

        case SENSORTEST_READ_REV_TEST:
            retval = sensortest_send_cmd(FP_MMI_READ_REV_TEST);
            break;

        case SENSORTEST_REGISTER_RW_TEST: {
            retval = sensortest_send_cmd(FP_MMI_REGISTER_RW_TEST);
            if (SENSOR_NEED_RESET == retval) {
                retval = fp_device_reset(idev);
                retval = sensortest_send_cmd(FP_MMI_REGISTER_RW_TEST);
            }
            retval = fp_device_reset(idev);
            retval = sensortest_send_cmd(FP_MMI_REGISTER_RECOVERY);
        } break;

        case SENSORTEST_CHECK_FINGER_ON:
            retval = sensortest_check_finger_on(idev);
            break;
        case SENSORTEST_IMAGE_QTY:
            retval = oplus_sensortest_get_image_qty(in_data, buffer, buffer_size);
            break;
        case SENSORTEST_GET_IMAGE:
            retval = sensortest_get_image(in_data, buffer, buffer_size);
            break;
        case SENSORTEST_WAIT_INTERRUTP: {
            // int in_0 = ((int *)in_data)[0];
            int in_1 = ((int*)in_data)[1];

            retval = sensortest_wait_finger_on(in_1);
        } break;
        case SENSORTEST_START_INTERRUTP:
            retval = sensortest_send_cmd(FP_MMI_FOD_TEST);
            break;
        case SENSORTEST_TEST_INTERRUTP: {
            trigger_info_t* trigger_info = (trigger_info_t*)&(((int*)in_data)[1]);

            retval = sensortest_wait_interrupt(trigger_info);
        } break;
        case SENSORTEST_STOP_INTERRUTP:

            break;
        case SENSORTEST_SET_CROP_INFO:
            retval = sensortest_set_crop_info(in_data);
            break;
        case SENSORTEST_GET_NVM_UID:
            retval = sensortest_get_nvm_uid(buffer, buffer_size);
            break;

        default:
            ex_log(LOG_DEBUG, "sensortest tool unkown cmd");
            break;
    }

    ex_log(LOG_DEBUG, "%s retval = %d", __func__, retval);

    return MMI_TEST_SUCCESS == retval ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_FAILED;
}

int flow_inline_legacy_cmd(int cmd, int param1, int param2, int param3, unsigned char* out_buf,
                           int* out_size) {
    int retval = FP_LIB_ERROR_GENERAL;
    int in_data_size;
    int in_data[4];
    in_data[0] = cmd;
    in_data[1] = param1;
    in_data[2] = param2;
    in_data[3] = param3;
    unsigned char* out_data = NULL;
    int out_data_size;
    in_data_size = sizeof(in_data);

    unsigned long long time_diff = 0;

    ex_log(LOG_DEBUG, "%s [%d] %d, %d, %d", __func__, cmd, param1, param2, param3);

    if (out_buf == NULL || out_size == NULL) {
        ex_log(LOG_DEBUG, "flow_inline_legacy_cmd out_data = malloc(4);");
        out_data = malloc(4);
        out_data_size = 4;
    } else {
        ex_log(LOG_DEBUG, "%s, out_size=%d", __func__, *out_size);
        out_data = out_buf;
        out_data_size = *out_size;
    }

    unsigned long long time_start = plat_get_time();
    ex_log(LOG_DEBUG, "flow_inline_legacy_cmd in");

    retval = opm_extra_command(PID_INLINETOOL, (unsigned char*)in_data, in_data_size, out_data,
                               &out_data_size);

    time_diff = plat_get_diff_time(time_start);
    ex_log(LOG_DEBUG, "flow_inline_legacy_cmd out");
    ex_log(LOG_DEBUG, "flow_inline_legacy_cmd out CMD = %d spent %d ms\r\n", cmd, (int)time_diff);

    if (FP_LIB_OK != retval) {
        ex_log(LOG_ERROR, "%s, retval = %d", __func__, retval);
    }
    ex_log(LOG_DEBUG, "%s, retval = %d", __func__, retval);
    if ((out_size != NULL) && (*out_size >= out_data_size)) {
        *out_size = out_data_size;
    }
    if (out_data != out_buf) {
        if (NULL != out_data) {
            free(out_data);
            out_data = NULL;
        }
    }
    return retval;
}
#if defined(__ET7XX__) && !defined(__ET0XX__)

#define FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_START 0x11000003
/* Output event to java AP. It means test is start. */
#define FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_END 0x11000004
/* Output event to java AP. It means test is end. */
#define FACTORY_TEST_EVT_SNSR_TEST_PUT_WKBOX 0x11000006
#define FACTORY_TEST_EVT_SNSR_TEST_PUT_BKBOX 0x11000007
#define FACTORY_TEST_EVT_SNSR_TEST_PUT_CHART 0x11000008

/* Output event to java AP. It means put the robber on sensor. */
#define EVT_ERROR 2020
// Copy from CS1 jni\common\platform\inc\egis_definition.h
#define SAVE_AND_PRINT_LOG(format, ...)                \
    do {                                               \
        ex_log(LOG_INFO, format, ##__VA_ARGS__);       \
        __inline_save_log_data(format, ##__VA_ARGS__); \
    } while (0)
/* Output event to java AP. It means test is error. */

static void notify_callback(int event_id, int first_param, int second_param, unsigned char* data,
                            int data_size) {
    ex_log(LOG_DEBUG, "%s, event_id = %d", __func__, event_id);

    if (NULL != g_event_callback) {
#ifndef EGIS_DBG
        if (event_id == EVENT_RETURN_IMAGE || event_id == EVENT_RETURN_LIVE_IMAGE) return;
#endif
        g_event_callback(event_id, first_param, second_param, data, data_size);
    }
}

static void __inline_save_log_data(const char* pImage, ...) {
    static char buf[1000] = {0};
    if (pImage == NULL) {
        ex_log(LOG_INFO, "inline_save_log_data pImage = NULL");
    }
    va_list arg;
    va_start(arg, pImage);
    unsigned int pImage_size = vsnprintf((char*)buf, 100, pImage, arg);
    va_end(arg);

    FILE* fn = fopen(INLINE_LOG_PATH, "ab");
    if (fn != NULL) {
        unsigned int size = fwrite(buf, 1, pImage_size, fn);
        fclose(fn);
        ex_log(LOG_INFO, "inline_save_log_data done size = %d", size);
    }
}
static void init_7xx_test(int event_id, int* result) {
    unsigned char path[11] = {0};
    unsigned char module_id[10] = {0};
    int ret = FP_LIB_ERROR_GENERAL, len = sizeof(int), path_len = sizeof(path), module_len = sizeof(module_id);
    char temp_path[FILE_NAME_LEN] = {0};

    mkdir(DEFINE_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    g_inline_timestamp = plat_get_time();
    ex_log(LOG_INFO, "init_7xx_test, inline_timestamp = %llu", g_inline_timestamp);
    ex_log(LOG_INFO, "init_7xx_test, ret = %d, %s", ret, DEFINE_FILE_PATH);
    __inline_save_log_data("GIT_SHA1 = OPLUS_0927_1.16.726.10_R_dbe8592ae");
    SAVE_AND_PRINT_LOG("\r\n");

    switch (event_id) {
        case FP_INLINE_7XX_NORMALSCAN:
            sprintf(temp_path, "%snormalscan", DEFINE_FILE_PATH);
            SAVE_AND_PRINT_LOG("==========INLINE_TEST_FOLDER_NAME = normalscan==========\r\n");
            break;
        case FP_INLINE_7XX_SNR_INIT:
            sprintf(temp_path, "%ssnr_%llu", DEFINE_FILE_PATH, g_inline_timestamp);
            SAVE_AND_PRINT_LOG("==========INLINE_TEST_FOLDER_NAME = snr_%llu==========\r\n",
                               g_inline_timestamp);
            break;
        default:
            sprintf(temp_path, "%s%llu", DEFINE_FILE_PATH, g_inline_timestamp);
            SAVE_AND_PRINT_LOG("==========INLINE_TEST_FOLDER_NAME = %llu==========\r\n",
                               g_inline_timestamp);
            break;
    }
    ret = mkdir(temp_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    ex_log(LOG_INFO, "init_7xx_test, ret = %d, %s", ret, temp_path);

    *result = 0;
    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_CASE_INIT, 0, 0, 0, (unsigned char*)result, &len);
    ex_log(LOG_INFO, "FP_INLINE_7XX_CASE_INIT ret=%d, buffer.result = %d", ret, *result);

    if (*result == FP_LIB_OK) {
        ret = flow_inline_legacy_cmd(FP_INLINE_7XX_GET_UUID, 0, 0, 0, path, &path_len);
        ex_log(LOG_INFO, "FP_INLINE_7XX_GET_UUID ret=%d, path_len = %d", ret, path_len);

        SAVE_AND_PRINT_LOG("ET713_UUID = %x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n", path[0],
                           path[1], path[2], path[3], path[4], path[5], path[6], path[7], path[8],
                           path[9]);
#ifdef FP_EGIS_OPTICAL_FA118
        SAVE_AND_PRINT_LOG("Module_type = 0x%x\r\n", path[10]);
#else
        ret = flow_inline_legacy_cmd(FP_INLINE_7XX_GET_MODULE_ID, 0, 0, 0, module_id, &module_len);
        ex_log(LOG_INFO, "FP_INLINE_7XX_GET_MODULE_ID ret=%d, module_len = %d", ret, module_len);

        SAVE_AND_PRINT_LOG(
            "ET713_Module_id = %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n", module_id[0],
            module_id[1], module_id[2], module_id[3], module_id[4], module_id[5], module_id[6],
            module_id[7], module_id[8], module_id[9]);

        ex_log(LOG_DEBUG,
               "ET713_Module_id_1 = "
               "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
               "%02x %02x %02x %02x",
               module_id[0] >> 3, module_id[0] & 0x07, module_id[1], module_id[2],
               module_id[3] >> 3, module_id[3] & 0x07, module_id[4] >> 5,
               (module_id[4] >> 1) & 0x0F, module_id[4] & 0x01, module_id[5] >> 4,
               (module_id[5] >> 2) & 0x03, module_id[5] & 0x03, module_id[6] >> 4,
               module_id[6] & 0x0F, module_id[7] >> 6, module_id[7] & 0x3F, module_id[8] >> 2,
               module_id[8] & 0x03, module_id[9] >> 4, module_id[9] & 0x0F);
        SAVE_AND_PRINT_LOG("Module_type = 0x%x\r\n", path[10] >> 3);
#endif //FP_EGIS_OPTICAL_FA118
        SAVE_AND_PRINT_LOG("Lens_type = 0x%x\r\n", path[0]);
    }
    ex_log(LOG_INFO, "init_7xx_test end, ret = %d", ret);

    // ret = set_hbm(TRUE); //customized by project
}

static void uninit_7xx_test(void) {
    int ret = FP_LIB_ERROR_GENERAL, len = sizeof(int);

    int result = 0;
    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_CASE_UNINIT, 0, 0, 0, (unsigned char*)&result, &len);
    ex_log(LOG_INFO, "FP_INLINE_7XX_CASE_UNINIT ret=%d, buffer.result = %d", ret, result);
    // ret = set_hbm(FALSE); //customized by project
}

static void __inline_print_log_file(struct sensor_test_output* buffer_out_data) {
    ex_log(LOG_INFO, "[inline_print_log_file]");
    ex_log(LOG_INFO, "===========[SNR_RESULT]========== start");
    SAVE_AND_PRINT_LOG("[SNR_RESULT] expo:%.3f\r\n[SNR_RESULT] hw_int:%d\r\n",
                       buffer_out_data->data.expo, buffer_out_data->data.hw_int);
    SAVE_AND_PRINT_LOG("[SNR_RESULT] Bkg_cx:%d\r\n[SNR_RESULT] Bkg_cy:%d\r\n",
                       buffer_out_data->data.bkg_cx, buffer_out_data->data.bkg_cy);
    SAVE_AND_PRINT_LOG("[SNR_RESULT] FEA_x1:%.1f\r\n[SNR_RESULT] FEA_x2:%.1f\r\n",
                       buffer_out_data->data.fov_x1_result, buffer_out_data->data.fov_x2_result);
    SAVE_AND_PRINT_LOG("[SNR_RESULT] FEA_y1:%.1f\r\n[SNR_RESULT] FEA_y2:%.1f\r\n",
                       buffer_out_data->data.fov_y1_result, buffer_out_data->data.fov_y2_result);
    SAVE_AND_PRINT_LOG(
        "[SNR_RESULT] MT_Lens_Centroid_cx:%d\r\n[SNR_RESULT] MT_Lens_Centroid_cy:%d\r\n",
        buffer_out_data->data.MT_centroid_x, buffer_out_data->data.MT_centroid_y);
    SAVE_AND_PRINT_LOG("[SNR_RESULT] LenMagnFactors:%.3f\r\n", buffer_out_data->data.period);
    SAVE_AND_PRINT_LOG(
        "[SNR_RESULT] signal:%.2f\r\n[SNR_RESULT] noise:%.2f\r\n[SNR_RESULT] snr:%.2f\r\n",
        buffer_out_data->data.signal, buffer_out_data->data.noise, buffer_out_data->data.snr);
    SAVE_AND_PRINT_LOG(
        "[SNR_RESULT] BadBlockNum:%d\r\n[SNR_RESULT] BadPixelMaxContinu:%d\r\n[SNR_RESULT] "
        "BadPixelNum:%d\r\n",
        buffer_out_data->data.bad_block_cnt, buffer_out_data->data.bad_pxl_max_continu_cnt,
        buffer_out_data->data.bad_pxl_cnt);
    SAVE_AND_PRINT_LOG("[SNR_RESULT] wk_intensity:%d\r\n", buffer_out_data->data.wk_avg_intensity);
    SAVE_AND_PRINT_LOG("[SNR_RESULT] bkg_avg_intensity:%d\r\n",
                       buffer_out_data->data.bkg_avg_intensity);
    ex_log(LOG_INFO, "===========[SNR_RESULT]========== end");
}
static int __send_7XX_sensortest_event(int event_id, unsigned char* test_result, int data_size) {
    ex_log(LOG_INFO, "event_id = %d", event_id);

    if (!host_touch_is_using_oplus_flow()) {
        switch (event_id) {
            case FP_INLINE_7XX_SNR_INIT:
                notify_callback(FACTORY_TEST_EVT_SNSR_TEST_PUT_WKBOX, 0, 0, 0, 0);
                break;
            case FP_INLINE_7XX_SNR_WKBOX_ON:
                notify_callback(FACTORY_TEST_EVT_SNSR_TEST_PUT_BKBOX, 0, 0, 0, 0);
                break;
            case FP_INLINE_7XX_SNR_BKBOX_ON:
                notify_callback(FACTORY_TEST_EVT_SNSR_TEST_PUT_CHART, 0, 0, 0, 0);
                break;
            case EVT_ERROR:
                SAVE_AND_PRINT_LOG("[SNR_RESULT] ERROR, algo fail \r\n");
                notify_callback(EVT_ERROR, 0, 0, test_result, data_size);
                break;
            default:
                notify_callback(event_id, 0, 0, test_result, data_size);
                break;
        }
    } else {
        int ret = FINGERPRINT_RES_SUCCESS;
        int len = 0;
        struct sensor_test_output cali_data;
        cmd_test_result_t notify_result;
        len = sizeof(cali_data);

        memset(&cali_data, 0, len);
        memset(&notify_result, 0, sizeof(notify_result));

        switch (event_id) {
            case FP_INLINE_7XX_SNR_WKBOX_ON:
                event_id = 1;
                ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_DATA, 0, 0, 0,
                                             (unsigned char*)&cali_data, &len);
                if (ret != 0) {
                    ex_log(LOG_ERROR, "get data error 1, %d", ret);
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 20,
                                    (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
                    return ret;
                }

                notify_result.test_result_data[0] = cali_data.data.expo;
                notify_result.test_result_data[1] = EXPO_MIN;
                notify_result.test_result_data[2] = EXPO_MAX;
                notify_result.test_result_data[4] = cali_data.data.hw_int;
                ex_log(LOG_DEBUG, "expo = %f, expo_min = %d, expo_max = %d, hw_int = %d",
                       cali_data.data.expo, EXPO_MIN, EXPO_MAX, cali_data.data.hw_int);
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.expo, EXPO_MAX, EXPO_MIN)) {
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_SUCCESS;
                }
                notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 20,
                                (unsigned char*)&notify_result, sizeof(cmd_test_result_t));

                return ret;
            case FP_INLINE_7XX_SNR_BKBOX_ON:
                event_id = 2;
                ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_DATA, 0, 0, 0,
                                             (unsigned char*)&cali_data, &len);
                if (ret != 0) {
                    ex_log(LOG_ERROR, "get data error 2, %d", ret);
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 32,
                                    (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
                    return ret;
                }

                notify_result.test_result_data[0] = cali_data.data.bkg_cx;
                notify_result.test_result_data[1] = BKG_CENTER_X_MIN;
                notify_result.test_result_data[2] = BKG_CENTER_X_MAX;
                ex_log(LOG_DEBUG, "bkg_cx = %d, bkg_center_x_min = %d, bkg_center_x_max = %d",
                       cali_data.data.bkg_cx, BKG_CENTER_X_MIN, BKG_CENTER_X_MAX);
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.bkg_cx, BKG_CENTER_X_MAX,
                                           BKG_CENTER_X_MIN)) {
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[4] = cali_data.data.bkg_cy;
                notify_result.test_result_data[5] = BKG_CENTER_Y_MIN;
                notify_result.test_result_data[6] = BKG_CENTER_Y_MAX;
                ex_log(LOG_DEBUG, "bkg_cy = %d, bkg_center_y_min = %d, bkg_center_y_max = %d",
                       cali_data.data.bkg_cy, BKG_CENTER_Y_MIN, BKG_CENTER_Y_MAX);
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.bkg_cy, BKG_CENTER_Y_MAX,
                                           BKG_CENTER_Y_MIN)) {
                    notify_result.test_result_data[7] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[7] = OPLUS_INLINE_TEST_SUCCESS;
                }
                notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 32,
                                (unsigned char*)&notify_result, sizeof(cmd_test_result_t));

                return ret;
            case FP_INLINE_7XX_FLASH_TEST:
                event_id = 3;
                break;
            case FP_INLINE_7XX_SNR_CHART_ON:
                event_id = 257;
                ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_DATA, 0, 0, 0,
                                             (unsigned char*)&cali_data, &len);
                if (ret != 0) {
                    ex_log(LOG_ERROR, "get data error 3, %d", ret);
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 196,
                                    (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
                    return ret;
                }
                ex_log(LOG_DEBUG,
                       "fov_x1_result = %f, fovx1_min = %f, fovx1_max = %f\n"
                       "fov_x2_result = %f, fovx2_min = %f, fovx2_max = %f\n"
                       "fov_y1_result = %f, fovy1_min = %f, fovy1_max = %f\n"
                       "fov_y2_result = %f, fovy2_min = %f, fovy2_max = %f\n"
                       "period = %f, period_min = %f, period_max = %f\n"
                       "signal = %f, signal_min = %f, signal_max = %f\n"
                       "noise = %f, noise_min = %f, noise_max = %f\n"
                       "snr = %f, snr_min = %f, snr_max = %f\n",
                       cali_data.data.fov_x1_result, FOVX1_MIN, FOVX1_MAX,
                       cali_data.data.fov_x2_result, FOVX2_MIN, FOVX2_MAX,
                       cali_data.data.fov_y1_result, FOVY1_MIN, FOVY1_MAX,
                       cali_data.data.fov_y2_result, FOVY2_MIN, FOVY2_MAX, cali_data.data.period,
                       PERIOD_MIN, PERIOD_MAX, cali_data.data.signal, SIGNAL_MIN, SIGNAL_MAX,
                       cali_data.data.noise, NOISE_MIN, NOISE_MAX, cali_data.data.snr, SNR_MIN,
                       SNR_MAX);
                ex_log(LOG_DEBUG,
                       "bad_block_cnt = %d, bad_block_cnt_min = %d, bad_block_cnt_max = %d\n"
                       "bad_pxl_max_continu_cnt = %d, bad_pxl_max_continu_cnt_min = %d, "
                       "bad_pxl_max_continu_cnt_max = %d\n"
                       "bad_pxl_cnt = %d, bad_pxl_cnt_min = %d, bad_pxl_cnt_max = %d\n"
                       "wk_avg_intensity = %d, intensity_diff_min = %d, intensity_diff_max = %d\n"
                       "bkg_avg_intensity = %d, intensity_diff_min = %d, intensity_diff_max = %d",
                       cali_data.data.bad_block_cnt, BAD_BLOCK_CNT_MIN, BAD_BLOCK_CNT_MAX,
                       cali_data.data.bad_pxl_max_continu_cnt, BAD_PIXEL_MAC_CONTINU_CNT_MIN,
                       BAD_PIXEL_MAC_CONTINU_CNT_MAX, cali_data.data.bad_pxl_cnt, BAD_PIXEL_CNT_MIN,
                       BAD_PIXEL_CNT_MAX, cali_data.data.wk_avg_intensity, INTENSITY_DIFF_MIN,
                       INTENSITY_DIFF_MAX, cali_data.data.bkg_avg_intensity, INTENSITY_DIFF_MIN,
                       INTENSITY_DIFF_MAX);

                notify_result.test_result_data[0] = cali_data.data.fov_x1_result;
                notify_result.test_result_data[1] = FOVX1_MIN;
                notify_result.test_result_data[2] = FOVX1_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.fov_x1_result, FOVX1_MAX, FOVX1_MIN)) {
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[4] = cali_data.data.fov_x2_result;
                notify_result.test_result_data[5] = FOVX2_MIN;
                notify_result.test_result_data[6] = FOVX2_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.fov_x2_result, FOVX2_MAX, FOVX2_MIN)) {
                    notify_result.test_result_data[7] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[7] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[8] = cali_data.data.fov_y1_result;
                notify_result.test_result_data[9] = FOVY1_MIN;
                notify_result.test_result_data[10] = FOVY1_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.fov_y1_result, FOVY1_MAX, FOVY1_MIN)) {
                    notify_result.test_result_data[11] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[11] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[12] = cali_data.data.fov_y2_result;
                notify_result.test_result_data[13] = FOVY2_MIN;
                notify_result.test_result_data[14] = FOVY2_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.fov_y2_result, FOVY2_MAX, FOVY2_MIN)) {
                    notify_result.test_result_data[15] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[15] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[16] = cali_data.data.period;
                notify_result.test_result_data[17] = PERIOD_MIN;
                notify_result.test_result_data[18] = PERIOD_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.period, PERIOD_MAX, PERIOD_MIN)) {
                    notify_result.test_result_data[19] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[19] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[20] = cali_data.data.signal;
                notify_result.test_result_data[21] = SIGNAL_MIN;
                notify_result.test_result_data[22] = SIGNAL_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.signal, SIGNAL_MAX, SIGNAL_MIN)) {
                    notify_result.test_result_data[23] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[23] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[24] = cali_data.data.noise;
                notify_result.test_result_data[25] = NOISE_MIN;
                notify_result.test_result_data[26] = NOISE_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.noise, NOISE_MAX, NOISE_MIN)) {
                    notify_result.test_result_data[27] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[27] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[28] = cali_data.data.snr;
                notify_result.test_result_data[29] = SNR_MIN;
                notify_result.test_result_data[30] = SNR_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.snr, SNR_MAX, SNR_MIN)) {
                    notify_result.test_result_data[31] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[31] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[32] = cali_data.data.bad_block_cnt;
                notify_result.test_result_data[33] = BAD_BLOCK_CNT_MIN;
                notify_result.test_result_data[34] = BAD_BLOCK_CNT_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.bad_block_cnt, BAD_BLOCK_CNT_MAX,
                                           BAD_BLOCK_CNT_MIN)) {
                    notify_result.test_result_data[35] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[35] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[36] = cali_data.data.bad_pxl_max_continu_cnt;
                notify_result.test_result_data[37] = BAD_PIXEL_MAC_CONTINU_CNT_MIN;
                notify_result.test_result_data[38] = BAD_PIXEL_MAC_CONTINU_CNT_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.bad_pxl_max_continu_cnt,
                                           BAD_PIXEL_MAC_CONTINU_CNT_MAX,
                                           BAD_PIXEL_MAC_CONTINU_CNT_MIN)) {
                    notify_result.test_result_data[39] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[39] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[40] = cali_data.data.bad_pxl_cnt;
                notify_result.test_result_data[41] = BAD_PIXEL_CNT_MIN;
                notify_result.test_result_data[42] = BAD_PIXEL_CNT_MAX;
                if (INLINE_CHECK_OVER_SPEC(cali_data.data.bad_pxl_cnt, BAD_PIXEL_CNT_MAX,
                                           BAD_PIXEL_CNT_MIN)) {
                    notify_result.test_result_data[43] = OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[43] = OPLUS_INLINE_TEST_SUCCESS;
                }

                notify_result.test_result_data[44] = cali_data.data.wk_avg_intensity;
                notify_result.test_result_data[45] = cali_data.data.bkg_avg_intensity;
                notify_result.test_result_data[46] = INTENSITY_DIFF_MIN;
                notify_result.test_result_data[47] = INTENSITY_DIFF_MAX;
                if (cali_data.data.check_order == OPLUS_INLINE_TEST_ERROR) {
                    notify_result.test_result_data[48] =
                        OPLUS_INLINE_TEST_ERROR;
                    ret = FINGERPRINT_RES_FAILED;
                }else{
                    notify_result.test_result_data[48] = OPLUS_INLINE_TEST_SUCCESS;
                }
                notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 196,
                                (unsigned char*)&notify_result, sizeof(cmd_test_result_t));

                return ret;
            case EVT_ERROR:
                event_id = 257;
                notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                break;
            case FP_INLINE_7XX_NORMALSCAN:
                if (*test_result != 0) {
                    ex_log(LOG_ERROR, "get data error 3, %d", ret);
                    event_id = 257;
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_ERROR;
                    notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 16,
                                    (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
                    ret = FINGERPRINT_RES_FAILED;
                    return ret;
                } else {
                    notify_result.test_result_data[3] = OPLUS_INLINE_TEST_SUCCESS;
                }

                break;
            case FP_INLINE_7XX_TEST_MFACTOR:
            case FP_INLINE_7XX_SNR_INIT:
            default:
                return ret;
        }

        ex_log(LOG_INFO, "event_id=%d, %f, %f, %f", event_id, notify_result.test_result_data[0],
               notify_result.test_result_data[1], notify_result.test_result_data[2],
               notify_result.test_result_data[3]);
        notify_callback(EVENT_SENSOR_OPTICAL_FINGERPRINT_CMD_RESULT, event_id, 16,
                        (unsigned char*)&notify_result, sizeof(cmd_test_result_t));
        return ret;
    }
    return FINGERPRINT_RES_SUCCESS;
}

static int __inline_check_result(struct sensor_test_output* buffer_out_data, int* err_index) {
    ex_log(LOG_INFO, "[inline_check_result]");
    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.expo, EXPO_MAX, EXPO_MIN))
        *err_index |= (1 << EXPO_OVER_SPEC_SHIFT_BIT);
    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.bkg_cx, BKG_CENTER_X_MAX, BKG_CENTER_X_MIN))
        *err_index |= (1 << BKG_X_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.bkg_cy, BKG_CENTER_Y_MAX, BKG_CENTER_Y_MIN))
        *err_index |= (1 << BKG_Y_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.fov_x1_result, FOVX1_MAX, FOVX1_MIN))
        *err_index |= (1 << FOV_X1_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.fov_x2_result, FOVX2_MAX, FOVX2_MIN))
        *err_index |= (1 << FOV_X2_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.fov_y1_result, FOVY1_MAX, FOVY1_MIN))
        *err_index |= (1 << FOV_Y1_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.fov_y2_result, FOVY2_MAX, FOVY2_MIN))
        *err_index |= (1 << FOV_Y2_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.period, PERIOD_MAX, PERIOD_MIN))
        *err_index |= (1 << PERIOD_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.signal, SIGNAL_MAX, SIGNAL_MIN))
        *err_index |= (1 << SIGNAL_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.noise, NOISE_MAX, NOISE_MIN))
        *err_index |= (1 << NOISE_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.snr, SNR_MAX, SNR_MIN))
        *err_index |= (1 << SNR_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.bad_block_cnt, BAD_BLOCK_CNT_MAX,
                               BAD_BLOCK_CNT_MIN))
        *err_index |= (1 << BAD_BLOCK_CNT_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.bad_pxl_max_continu_cnt,
                               BAD_PIXEL_MAC_CONTINU_CNT_MAX, BAD_PIXEL_MAC_CONTINU_CNT_MIN))
        *err_index |= (1 << BAD_PXL_MAX_CNT_OVER_SPEC_SHIFT_BIT);

    if (INLINE_CHECK_OVER_SPEC(buffer_out_data->data.bad_pxl_cnt, BAD_PIXEL_CNT_MAX,
                               BAD_PIXEL_CNT_MIN))
        *err_index |= (1 << BAD_PXL_CNT_OVER_SPEC_SHIFT_BIT);

    if (*err_index != 0) {
        ex_log(LOG_ERROR, "[SNR_RESULT] fail, test overspec, *err_index = %d", *err_index);
        SAVE_AND_PRINT_LOG("[SNR_RESULT] fail, test overspec = %d \r\n", *err_index);
    }
    return FP_LIB_OK;
}

#if defined(__INLINE_SAVE_LOG_ENABLE_DEFAULT__) || defined(__INLINE_SAVE_LOG_ENABLE_EXTRA_INFO__)
static void __inline_get_snr_images_v2(void) {
    char temp_path[FILE_NAME_LEN] = {0};
    int ret = 0;
    struct sensor_test_output* buffer_out_data;
    int buffer_out_length = sizeof(struct sensor_test_output);
    buffer_out_data = (struct sensor_test_output*)malloc(sizeof(struct sensor_test_output));
    if (buffer_out_data == NULL) {
        ex_log(LOG_ERROR, "malloc ERROR");
        return;
    }
    // 16bit
    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_ALL_IMAGE_16_BIT_INFO, 0, 0, 0,
                                 (unsigned char*)buffer_out_data, &buffer_out_length);

    while (buffer_out_data->picture_remaining_count) {
        ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_ALL_IMAGE_16_BIT, 0, 0, 0,
                                     (unsigned char*)buffer_out_data, &buffer_out_length);
        if (ret != 0) ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);

        for (int j = 0; j < IMG_MAX_BUFFER_SIZE; j++) {
            buffer_out_data->picture_buffer_16[j] =
                LITTLE_TO_BIG_ENDIAN(buffer_out_data->picture_buffer_16[j]);
        }
        sprintf(temp_path, "%ssnr_%llu/%s", DEFINE_FILE_PATH, g_inline_timestamp,
                buffer_out_data->name);
        ex_log(LOG_DEBUG, "path=%s, count=%d", temp_path, buffer_out_data->picture_remaining_count);
        plat_save_raw_image(temp_path, (unsigned char*)buffer_out_data->picture_buffer_16,
                            IMG_MAX_BUFFER_SIZE * 2, 1);
#ifdef EGIS_SAVE_BMP
        if (NULL != strstr(temp_path, "picture_sp_bk16") ||
            NULL != strstr(temp_path, "picture_sp_wk16"))
            save_16bit_to_bitmap(ET713_SENSOR_WIDTH, ET713_SENSOR_HEIGHT,
                                 (unsigned short*)buffer_out_data->picture_buffer_16, temp_path);
#endif
        memset(temp_path, 0, sizeof(temp_path));
    }
    // 8bit
    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_ALL_IMAGE_8_BIT_INFO, 0, 0, 0,
                                 (unsigned char*)buffer_out_data, &buffer_out_length);

    while (buffer_out_data->picture_remaining_count) {
        ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_ALL_IMAGE_8_BIT, 0, 0, 0,
                                     (unsigned char*)buffer_out_data, &buffer_out_length);
        if (ret != 0) ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);

        sprintf(temp_path, "%ssnr_%llu/%s", DEFINE_FILE_PATH, g_inline_timestamp,
                buffer_out_data->name);
        ex_log(LOG_DEBUG, "path=%s, count=%d", temp_path, buffer_out_data->picture_remaining_count);
        plat_save_raw_image(temp_path, (unsigned char*)buffer_out_data->picture_buffer_8,
                            IMG_MAX_BUFFER_SIZE, 1);
#ifdef EGIS_SAVE_BMP
        if (NULL != strstr(temp_path, "picture_snr_ct8"))
            save_8bit_to_bitmap(DBG_SNR_CTR8_WIDTH, DBG_SNR_CTR8_HEIGHT,
                                (unsigned char*)buffer_out_data->picture_buffer_8, temp_path);
#endif
        memset(temp_path, 0, sizeof(temp_path));
    }

    if (NULL != buffer_out_data) {
        free(buffer_out_data);
        buffer_out_data = NULL;
    }
}
#endif

static void __inline_get_snr_MT_cali_data(unsigned char* buffer, int* err_index) {
    int ret = 0;
    struct sensor_test_output* buffer_out_data;
    int buffer_out_length = sizeof(struct sensor_test_output);

    buffer_out_data = (struct sensor_test_output*)malloc(sizeof(struct sensor_test_output));
    if (buffer_out_data == NULL) {
        ex_log(LOG_ERROR, "malloc ERROR");
        return;
    }
    ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_GET_DATA, 0, 0, 0,
                                 (unsigned char*)buffer_out_data, &buffer_out_length);
    if (ret != 0) ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);

    __inline_check_result(buffer_out_data, err_index);
    __inline_print_log_file(buffer_out_data);
    if (buffer != NULL) {
        ex_log(LOG_DEBUG, "copy to out buffer");
        memcpy(buffer, (char*)&buffer_out_data->data, sizeof(final_MT_cali_data_t));
    }

    if (NULL != buffer_out_data) {
        free(buffer_out_data);
        buffer_out_data = NULL;
    }
}

static void __inline_get_snr_data(int* err_index) {
    ex_log(LOG_INFO, "[inline_get_snr_data]GET_SNR_IMAGE_V2 ITEM");
    __inline_get_snr_MT_cali_data(NULL, err_index);
#if defined(__INLINE_SAVE_LOG_ENABLE_DEFAULT__) || defined(__INLINE_SAVE_LOG_ENABLE_EXTRA_INFO__)
    __inline_get_snr_images_v2();
#endif
}

#ifdef __INLINE_SAVE_LOG_ENABLE_DEFAULT__
static void __inline_get_norscan_data(int cmd) {
    ex_log(LOG_INFO, "[inline_get_norscan_data]");
    char temp_path[FILE_NAME_LEN] = {0};
    int ret = 0;
    struct sensor_test_output* buffer_out_data = NULL;
    int buffer_out_length = sizeof(struct sensor_test_output);
    buffer_out_data = (struct sensor_test_output*)malloc(sizeof(struct sensor_test_output));
    if (buffer_out_data == NULL) {
        ex_log(LOG_ERROR, "malloc ERROR");
        return;
    }
    ret = flow_inline_legacy_cmd(cmd, 0, 0, 0, (unsigned char*)buffer_out_data, &buffer_out_length);
    if (ret != 0) ex_log(LOG_ERROR, "flow_inline_legacy_cmd ERROR, %d", ret);

    for (int j = 0; j < IMG_MAX_BUFFER_SIZE; j++) {
        buffer_out_data->picture_buffer_16[j] =
            LITTLE_TO_BIG_ENDIAN(buffer_out_data->picture_buffer_16[j]);
    }
    sprintf(temp_path, "%snormalscan/%s", DEFINE_FILE_PATH, buffer_out_data->name);
    char* pPath = strstr(temp_path, ".bin");
    if (pPath == NULL) {
        ex_log(LOG_DEBUG, "path is error.");
        goto out;
    }
    sprintf(pPath, "_%llu.bin", g_inline_timestamp);
    plat_save_raw_image(temp_path, (unsigned char*)buffer_out_data->picture_buffer_16,
                        IMG_MAX_BUFFER_SIZE * 2, 1);
    memset(temp_path, 0, sizeof(temp_path));
out:
    if (NULL != buffer_out_data) {
        free(buffer_out_data);
        buffer_out_data = NULL;
    }
}
#endif

int do_7XX_spi_test(int cmd) {
    int ret = FP_LIB_ERROR_GENERAL;
    int buffer_out_length = sizeof(int), test_result = 0;

    mkdir(DEFINE_FILE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    ex_log(LOG_INFO, "cmd = %d", cmd);

    ex_log(LOG_INFO, "FP_INLINE_7XX_SPI(NORSCAN) Start");

    ret = do_7XX_sensortest(FP_INLINE_7XX_NORMALSCAN, 0, 0, 0, (unsigned char*)&test_result,
                            &buffer_out_length);

    ex_log(LOG_INFO, "FP_INLINE_7XX_SPI(NORSCAN) End");

    ex_log(LOG_INFO, "[FP_INLINE_7XX] spi_test_result = %d", ret);
    return ret;
}

int do_7XX_sensortest(int cmd, int param1, int param2, int param3, unsigned char* out_buf,
                      int* out_size) {
    int ret = FP_LIB_ERROR_GENERAL;
    static unsigned long long time_diff_total = 0;
    int buffer_out_length = sizeof(int), test_result = 0, inline_err_index = 0;
    unsigned long long time_start = plat_get_time();
    int retry_count = 0;
    const int retry_time = 3;
    ex_log(LOG_INFO, "cmd = %d", cmd);
    switch (cmd) {
        case FP_INLINE_7XX_NORMALSCAN:
            retry_count = 0;
            do {
                init_7xx_test(FP_INLINE_7XX_NORMALSCAN, &test_result);
                ex_log(LOG_INFO, "FP_INLINE_7XX_NORMALSCAN test_result = %d, retry_count = %d",
                       test_result, retry_count);
                retry_count++;
                if (test_result != FP_LIB_OK) {
                    do_power_on(SENSOR_PWR_OFF);
                    plat_sleep_time(60);
                    do_power_on(SENSOR_PWR_ON);
                }
            } while (test_result != FP_LIB_OK && retry_count < retry_time);

            if (test_result != FP_LIB_OK) {
                __send_7XX_sensortest_event(FP_INLINE_7XX_NORMALSCAN,
                                            (unsigned char*)&(test_result), sizeof(test_result));
                break;
            }

            usleep(500 * 000);
            ex_log(LOG_INFO, "FP_INLINE_7XX_NORMALSCAN Start");
#if defined(__OPLUS_ON_A50__)
            __send_7XX_sensortest_event(FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_START, 0, 0);
#endif
            do {
                ret = flow_inline_legacy_cmd(cmd, param1, param2, param3,
                                             (unsigned char*)&test_result, &buffer_out_length);
                ex_log(LOG_INFO, "ret=%d, test_result = %d", ret, test_result);
                if (ret != FP_LIB_OK)
                    ex_log(LOG_ERROR, "ret=%d, test_result = %d", ret, test_result);

                switch (test_result) {
                    case EGIS_7XX_TEST_RESET:
                        ex_log(LOG_INFO, "No hardware reset for case EGIS_7XX_TEST_RESET");
                        break;
                    case FP_LIB_OK:
                        __send_7XX_sensortest_event(FACTORY_TEST_EVT_SNSR_TEST_SCRIPT_END,
                                                    (unsigned char*)&(inline_err_index),
                                                    sizeof(inline_err_index));
                        break;
                    case EGIS_7XX_TEST_REGISTER_TEST_FAIL:
                        inline_err_index |= (1 << REGISTER_TEST_SHIFT_BIT);
                        ex_log(LOG_ERROR, "EGIS_7XX_TEST_REGISTER_TEST_FAIL");
                        break;
                    case EGIS_7XX_TEST_OTP_TEST_FAIL:
                        inline_err_index |= (1 << OTP_TEST_SHIFT_BIT);
                        ex_log(LOG_ERROR, "EGIS_7XX_TEST_OTP_TEST_FAIL");
                        break;
                    case EGIS_7XX_TEST_TESTPATTERN_TEST_FAIL:
                        inline_err_index |= (1 << TESTPATTERN_TEST_SHIFT_BIT);
                        ex_log(LOG_ERROR, "EGIS_7XX_TEST_TESTPATTERN_TEST_FAIL");
                        break;
                }

                if (inline_err_index != FP_LIB_OK) {
                    __send_7XX_sensortest_event(EVT_ERROR, (unsigned char*)&(inline_err_index),
                                                sizeof(inline_err_index));
                }
            } while (!(test_result == FP_LIB_OK || inline_err_index != FP_LIB_OK));
            __inline_get_norscan_data(FP_INLINE_7XX_NORMAL_GET_IMAGE);
            uninit_7xx_test();
            opm_initialize_sensor();
            host_touch_set_hbm(0);
            ex_log(LOG_INFO, "FP_INLINE_7XX_NORMALSCAN End");
            break;
        case FP_INLINE_7XX_SNR_INIT:
            init_7xx_test(FP_INLINE_7XX_SNR_INIT, &test_result);
            if (g_app_type == APP_IS_USING_BINDER)
                __send_7XX_sensortest_event(FP_INLINE_7XX_SNR_INIT, 0, 0);

#ifdef __INLINE_SAVE_LOG_ENABLE_EXTRA_INFO__
            ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_SAVE_LOG_ENABLE_EXTRA_INFO, param1,
                                         param2, param3, NULL, 0);
#endif
            break;
        case FP_INLINE_7XX_FLASH_TEST:
#ifndef __OPLUS_ON_A50__
            ret = FP_LIB_OK;
            __send_7XX_sensortest_event(cmd, (unsigned char*)&ret, sizeof(ret));
            break;
#endif
        case FP_INLINE_7XX_SNR_WKBOX_ON:
        case FP_INLINE_7XX_SNR_BKBOX_ON:
        case FP_INLINE_7XX_SNR_CHART_ON:
            host_touch_set_hbm(1);
            plat_sleep_time(100);
            ret = flow_inline_legacy_cmd(cmd, param1, param2, param3, (unsigned char*)&test_result,
                                         &buffer_out_length);

            if (cmd == FP_INLINE_7XX_SNR_BKBOX_ON) {
                ret = flow_inline_legacy_cmd(FP_INLINE_7XX_SNR_BKBOX_ON_2, param1, param2, param3,
                                             (unsigned char*)&test_result, &buffer_out_length);
            }
            if (ret != FP_LIB_OK)
                ex_log(LOG_ERROR, "FAIL, ret=%d, test_result=%d", ret, test_result);
            unsigned long long time_diff = plat_get_diff_time(time_start);
            SAVE_AND_PRINT_LOG("CMD = %d spent %d ms\r\n", cmd, (int)time_diff);
            time_diff_total += time_diff;
            ret = __send_7XX_sensortest_event(cmd, (unsigned char*)&(inline_err_index),
                                        sizeof(inline_err_index));

#if defined(__OPLUS_ON_A50__)
            if (
#else
            if ((test_result == FP_LIB_OK && cmd == FP_INLINE_7XX_SNR_CHART_ON) ||
#endif
                test_result == FP_LIB_ERROR_GENERAL) {
                __inline_get_snr_data(&inline_err_index);
                uninit_7xx_test();
                time_diff = plat_get_diff_time(time_start);
                time_diff_total += time_diff;
                SAVE_AND_PRINT_LOG("get inline data, spent %d ms\r\n", (int)time_diff);
                SAVE_AND_PRINT_LOG("Total Test spent %d ms\r\n", (int)time_diff_total);
                time_diff_total = 0;
                if (ret == FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_DEBUG, "TYPE_RECEIVE_CALIBRATION_DATA Start");
                    ret = opt_receive_data(TYPE_RECEIVE_CALIBRATION_DATA, NULL, 0, NULL, 0);
                    ex_log(LOG_DEBUG, "TYPE_RECEIVE_CALIBRATION_DATA, ret =%d", ret);
                    if (FINGERPRINT_RES_SUCCESS == ret) {
#ifndef ALGO_GEN_4
                        opt_delete_data(TYPE_DELETE_SCRATCH_FILE);
                        opt_delete_data(TYPE_DELETE_BDS_FILE);
                        opt_receive_data(TYPE_RECEIVE_BDS, NULL, 0, NULL, 0);
                        opt_receive_data(TYPE_RECEIVE_SCRATCH, NULL, 0, NULL, 0);
#else
                        opt_delete_data(TYPE_DELETE_DEBASE);
#endif
                    }
                }else{
                    ex_log(LOG_DEBUG, "inline test fail, ret =%d", ret);
                }
            }
            break;
        case FP_INLINE_7XX_SNR_GET_DATA:  // specific for demo apk calibration.
            ex_log(LOG_DEBUG, "FP_INLINE_7XX_SNR_GET_DATA Start");
            if (out_size != NULL) {
                ex_log(LOG_DEBUG, "out_size=%d", *out_size);
                if ((unsigned long)*out_size < sizeof(final_MT_cali_data_t)) {
                    ex_log(LOG_ERROR, "out_size is NULL");
                    break;
                }
            } else {
                ex_log(LOG_ERROR, "out_size is NULL");
                break;
            }
            ex_log(LOG_DEBUG, "__inline_get_snr_MT_cali_data");
            __inline_get_snr_MT_cali_data(out_buf, &inline_err_index);
            return FP_LIB_OK;
        case FP_INLINE_7XX_CASE_UNINIT:
            uninit_7xx_test();
            break;
        default:
            __send_7XX_sensortest_event(EVT_ERROR, (unsigned char*)&test_result, buffer_out_length);
            uninit_7xx_test();
            ex_log(LOG_ERROR, "not support, cmd = %d", cmd);
            break;
    }

    ex_log(LOG_INFO, "ret = %d", ret);
    return ret;
}
#endif
