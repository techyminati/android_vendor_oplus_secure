/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <cutils/fs.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>

#include "gf_ca_entry.h"
#include "gf_dump_data.h"
#include "gf_fingerprint.h"
#include "gf_hal_common.h"
#include "gf_common.h"
#include "gf_hal.h"
#include "gf_hal_log.h"
#include "gf_hal_device.h"
#include "gf_hal_milan_a_series.h"
#include "gf_hal_milan_an_series.h"
#include "gf_hal_milan.h"
#include "gf_hal_oswego_m.h"
#ifdef __SIMULATOR
#include "gf_hal_simulator.h"
#endif  // __SIMULATOR
#include "gf_hal_timer.h"
#include "gf_hal_frr_database.h"
#include "IsScreenInteractive.h"
#include "gf_dump_bigdata.h"
#include "gf_hal_mem.h"
#include "gf_dump_data_utils.h"
#include "fingerprint_type.h"
#include "gf_hal_test.h"

#define LOG_TAG "[GF_HAL][gf_hal_common]"
#define GF_OTP_INFO_FILEPATH    "/data/vendor_de/0/fpdata/gf_otp_info"

pthread_mutex_t g_hal_mutex = PTHREAD_MUTEX_INITIALIZER;  // hal mutex
pthread_mutex_t g_sensor_mutex = PTHREAD_MUTEX_INITIALIZER;  // sensor mutex

uint32_t g_hal_inited = 0;  // hal inited flag

uint8_t g_enable_fingerprint_module = 1;  // enable fingerprint module

uint32_t g_failed_attempts = 0;  // failed attempts

uint32_t g_sensor_row = 0;  // sensor row
uint32_t g_sensor_col = 0;  // sensor rol
uint32_t g_sensor_nav_row = 0;  // sensor nav row
uint32_t g_sensor_nav_col = 0;  // sensor nav col

gf_nav_click_status_t g_nav_click_status = GF_NAV_CLICK_STATUS_NONE;  // nav click status
timer_t g_nav_double_click_timer_id = 0;  // id of nav double click timer
timer_t g_nav_long_press_timer_id = 0;  // id of nav long press timer
pthread_mutex_t g_nav_click_status_mutex = PTHREAD_MUTEX_INITIALIZER;  // nav click status mutex

gf_mode_t g_mode = MODE_NONE;  // mode
gf_operation_type_t g_operation = OPERATION_NONE;  // operation

timer_t g_enroll_timer_id = 0;  // id of enroll timer
timer_t g_enroll_timer_id_pre = 0; //for ppo
uint32_t g_enroll_timer_sec = 0;  // enroll timer second
uint16_t g_enroll_invalid_template_num = 0;  // number of enroll invalid template

timer_t g_long_pressed_timer_id = 0;  // id of long pressed timer

uint8_t g_key_down_flag = 0;  // flag of ley down

timer_t g_esd_timer_id = 0;  // id of esd timer
#define HAL_ESD_TIMER_SEC 2
uint8_t g_esd_check_flag = GF_CHECK_DISABLE;  // flag of esd check

uint32_t g_nav_times = 0;  // nav times
uint32_t g_nav_frame_index = 0;  // nav frame index

int64_t g_down_irq_time = 0;  // down irq time
int64_t g_image_irq_time = 0;  // image irq time

static uint16_t g_orientation = 0;  // nav code convert orientation
static uint16_t g_facing = GF_SENSOR_FACING_BACK;  // sensor facing back

static gf_key_code_t g_long_press_keycode = GF_KEY_NONE;  // keycode of long press

uint32_t g_spi_speed = GF_SPI_LOW_SPEED;  // spi speed

uint8_t g_screen_status = 0;  // screen status
uint8_t g_is_only_dump_broken_check = 0;  // flag of only dump broken check

volatile uint8_t g_set_active_group_done = 0;  // flag of active group done
uint8_t g_enum_fingers = 0;
static gf_screen_state_t gf_screen_status = GF_SCREEN_OFF;

uint32_t g_auth_retry_times = 0;  // retry times
uint32_t g_enroll_finger_counts = 0;

#ifdef FP_HYPNUSD_ENABLE
void gf_set_hypnus(int32_t action_type, int32_t action_timeout)
{
    LOG_E(LOG_TAG, "[%s] entry, action_type is %d, action_timeout is %d", __func__, action_type, action_timeout);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));

    message.type = FINGERPRINT_HYPNUSDSETACION;
    message.data.hypnusd_setting.action_type = action_type;
    message.data.hypnusd_setting.action_timeout = action_timeout;
    if (g_fingerprint_device->notify != NULL)
    {
        g_fingerprint_device->notify(&message);
    }
    return;
}
#endif

void gf_bind_bigcore_bytid() {
    LOG_D(LOG_TAG, "[%s] bind_bigcore_bytid", __func__);
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));

    message.type = FINGERPRINT_BINDCORE;
    message.data.bindcore_setting.tid = gettid();
    if (g_fingerprint_device->notify != NULL)
    {
        LOG_D(LOG_TAG, "[%s] bindcore_tid = %u", __func__, message.data.bindcore_setting.tid);
        g_fingerprint_device->notify(&message);
    }
    return;
}

static const uint8_t OTP_CRC8_TAB[256] =  // otp crc8 table
{
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36,
    0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F,
    0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8,
    0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5,
    0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC,
    0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E, 0x3B,
    0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E,
    0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87,
    0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE,
    0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4, 0x69,
    0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33,
    0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D,
    0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A,
    0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F,
    0x8A, 0x8D, 0x84, 0x83, 0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8,
    0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

/**
 * Function: hal_common_otp_crc8
 * Description: Calculate a crc value of otp data.
 * Input: buf, buf_len
 * Output: None
 * Return: uint8_t
 */
static uint8_t hal_common_otp_crc8(uint8_t *buf, uint8_t buf_len)
{
    uint8_t crc8 = 0;

    while (buf_len--)
    {
        uint8_t index = crc8 ^ (*buf++);
        crc8 = OTP_CRC8_TAB[index];
    }

    return (~crc8);
}

void gf_hal_notify_send_auth_dcsmsg(fingerprint_auth_dcsmsg_t auth_context) {
    fingerprint_msg_t message;
    char algo_version[PROP_VALUE_MAX] = { 0 };
    memset(&message, 0, sizeof(message));
    property_get(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, algo_version, "Milan_v_3.02.00.35");

    message.type = FINGERPRINT_AUTHENTICATED_DCSSTATUS;
    message.data.auth_dcsmsg.auth_result = auth_context.auth_result;
    message.data.auth_dcsmsg.fail_reason = auth_context.fail_reason;
    message.data.auth_dcsmsg.quality_score = auth_context.quality_score;
    message.data.auth_dcsmsg.match_score = auth_context.match_score;
    message.data.auth_dcsmsg.signal_value = 0;    //UNUSED
    message.data.auth_dcsmsg.img_area = auth_context.img_area;
    message.data.auth_dcsmsg.retry_times = auth_context.retry_times;
    //memcpy(message.data.auth_dcsmsg.algo_version, ALGO_VERSION, strlen(ALGO_VERSION));
    memcpy(message.data.auth_dcsmsg.algo_version, algo_version, sizeof(message.data.auth_dcsmsg.algo_version));
    message.data.auth_dcsmsg.chip_ic = fp_config_info_init.fp_ic_type;
    message.data.auth_dcsmsg.module_type = g_hal_config.chip_series;
    message.data.auth_dcsmsg.lense_type = 0;//UNUSED
    message.data.auth_dcsmsg.dsp_availalbe = 0;    //side fp sure be 0

    LOG_D(LOG_TAG, "[%s] Auth, auth_result = %d", __func__, message.data.auth_dcsmsg.auth_result);
    LOG_D(LOG_TAG, "[%s] Auth, fail_reason = %d", __func__, message.data.auth_dcsmsg.fail_reason);
    LOG_D(LOG_TAG, "[%s] Auth, quality_score = %d", __func__, message.data.auth_dcsmsg.quality_score);
    LOG_D(LOG_TAG, "[%s] Auth, match_score = %d", __func__,  message.data.auth_dcsmsg.match_score);
    LOG_D(LOG_TAG, "[%s] Auth, signal_value = %d", __func__,  message.data.auth_dcsmsg.signal_value);
    LOG_D(LOG_TAG, "[%s] Auth, img_area = %d", __func__, message.data.auth_dcsmsg.img_area);
    LOG_D(LOG_TAG, "[%s] Auth, retry_times = %d", __func__, message.data.auth_dcsmsg.retry_times);
    LOG_D(LOG_TAG, "[%s] Auth, algo_version = %s", __func__, message.data.auth_dcsmsg.algo_version);
    LOG_D(LOG_TAG, "[%s] Auth, chip_ic = %d", __func__,  message.data.auth_dcsmsg.chip_ic);
    LOG_D(LOG_TAG, "[%s] Auth, module_type = %d", __func__, message.data.auth_dcsmsg.module_type);
    LOG_D(LOG_TAG, "[%s] Auth, lense_type = %d", __func__, message.data.auth_dcsmsg.lense_type);
    LOG_D(LOG_TAG, "[%s] Auth, dsp_available = %d", __func__, message.data.auth_dcsmsg.dsp_availalbe);

    if (g_fingerprint_device->notify != NULL) {
        g_fingerprint_device->notify(&message);
    }
}

void gf_hal_notify_image_info(fingerprint_acquired_info_t acquired_info)
{
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));

    if (NULL == g_fingerprint_device || NULL == g_fingerprint_device->notify) {
        LOG_E(LOG_TAG, "[%s] NULL device or notify", __func__);
        return;
    }
    message.type = FINGERPRINT_IMAGE_INFO;
    message.data.image_info.type = acquired_info;
    g_fingerprint_device->notify(&message);
}

void gf_hal_notify_msg_info(fingerprint_msg_type_t msg_info)
{
    fingerprint_msg_t message;
    memset(&message, 0, sizeof(fingerprint_msg_t));

    if (NULL == g_fingerprint_device || NULL == g_fingerprint_device->notify) {
        LOG_E(LOG_TAG, "[%s] NULL device or notify", __func__);
        return;
    }
    message.type = msg_info;
    LOG_D(LOG_TAG,"%s send %d message", __func__, msg_info);
    g_fingerprint_device->notify(&message);
}

/**
 * Function: gf_hal_notify_acquired_info
 * Description: Report acquired information in authentication process.
 * Input: acquired_info
 * Output: None
 * Return: void
 */
void gf_hal_notify_acquired_info(gf_fingerprint_acquired_info_t acquired_info)
{
    fingerprint_msg_t message = { 0 };

    if (NULL == g_fingerprint_device || NULL == g_fingerprint_device->notify)
    {
        LOG_E(LOG_TAG, "[%s] NULL device or notify", __func__);
        return;
    }

    message.type = FINGERPRINT_ACQUIRED;
    message.data.acquired.acquired_info = (fingerprint_acquired_info_t)acquired_info;
    LOG_E(LOG_TAG, "[%s] acquired_info = %d, acquired.acquired_info = %d", __func__, acquired_info, message.data.acquired.acquired_info);
    g_fingerprint_device->notify(&message);
}

/**
 * Function: gf_hal_notify_error_info
 * Description: Report errot information.
 * Input: err_code
 * Output: None
 * Return: void
 */
void gf_hal_notify_error_info(gf_fingerprint_error_t err_code)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL)
    {
        fingerprint_msg_t message = { 0 };
        message.type = FINGERPRINT_ERROR;
        message.data.error = (fingerprint_error_t)err_code;
        LOG_E(LOG_TAG, "[%s] err code : %d.", __func__, message.data.error);
        g_fingerprint_device->notify(&message);
    }
}

/**
 * Function: gf_hal_notify_enrollment_progress
 * Description: Report the progress of  enrollment.
 * Input: group_id, finger_id, samples_remaining
 * Output: None
 * Return: void
 */
void gf_hal_notify_enrollment_progress(uint32_t group_id, uint32_t finger_id,
                                       uint32_t samples_remaining)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL)
    {

        if (samples_remaining == 0) {
            fingerprint_msg_t message = { 0 };
            uint32_t err = 0;
            int32_t indices_count = MAX_FINGERS_PER_USER;
            uint32_t indices[MAX_FINGERS_PER_USER] = {0};
            memset(&message, 0 , sizeof(fingerprint_msg_t));
            err = gf_hal_common_get_id_list(g_fingerprint_device, group_id, indices, &indices_count);
            if (err) {
                LOG_E(LOG_TAG, "[%s] gf_ha_common_get_id_list error~~~", __func__);
                return;
            }
            LOG_I(LOG_TAG, "[%s] ids count = %d", __func__, indices_count);

            message.data.enroll.fingers_count = indices_count;
            for (uint32_t i = 0; i < indices_count; i++) {
                LOG_I(LOG_TAG, "[%s] ids[%d] = %u", __func__, i, indices[i]);
                message.data.enroll.total_fingers[i].fid = indices[i];
                message.data.enroll.total_fingers[i].gid = group_id;
            }
            message.type = FINGERPRINT_TEMPLATE_ENROLLING;
            message.data.enroll.finger.gid = group_id;
            message.data.enroll.finger.fid = finger_id;
            message.data.enroll.samples_remaining = samples_remaining;
            g_fingerprint_device->notify(&message);
        } else {
            fingerprint_msg_t message = { 0 };
            message.type = FINGERPRINT_TEMPLATE_ENROLLING;
            message.data.enroll.finger.gid = group_id;
            message.data.enroll.finger.fid = finger_id;
            message.data.enroll.samples_remaining = samples_remaining;
            g_fingerprint_device->notify(&message);
        }
    }
}

/**
 * Function: gf_hal_get_screen_state
 * Description: Get the status of screen.
 * Input: screen_state
 * Output: screen_state
 * Return: void
 */
void gf_hal_get_screen_state(int32_t *screen_state)
{
    if (NULL == screen_state)
    {
        LOG_E(LOG_TAG, "[%s] screen_state is null", __func__);
    }
    else
    {
#if ((defined __ANDROID_M) || (defined __ANDROID_N))
        *screen_state = gfIsScreenInteractive();
        LOG_D(LOG_TAG, "[%s] get screen_state=%d", __func__, *screen_state);
#endif  // __ANDROID_O
    }
}

/**
 * Function: gf_hal_reinit
 * Description: Reinitialize hal.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_reinit()
{
    gf_error_t err = GF_SUCCESS;
    gf_init_t *cmd_init = NULL;
    gf_detect_sensor_t *cmd_common = NULL;
    uint32_t size = sizeof(gf_detect_sensor_t);
    gf_init_finished_t *cmd_init_finished = NULL;
    uint8_t retry_time = 5;
    FUNC_ENTER();
    err = gf_hal_control_spi_clock(1);
    if (err != GF_SUCCESS)
    {
        LOG_E(LOG_TAG, "[%s] spi clock enable failed", __func__);
        return err;
    }

    do
    {
        gf_ca_close_session();
        err = gf_ca_open_session();

        if (err != GF_SUCCESS)
        {
            break;
        }

        cmd_common = (gf_detect_sensor_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd_common)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        do
        {
            memset(cmd_common, 0, size);
            err = gf_ca_invoke_command(GF_OPERATION_ID, GF_CMD_DETECT_SENSOR, cmd_common,
                                   size);
            if (err == GF_SUCCESS)
            {
                LOG_D(LOG_TAG, "[%s] detect sensor success", __func__);
                break;
            }
            if (--retry_time > 0)
            {
                // detect sensor failed to reset chip and retry
                gf_hal_reset_chip();
                LOG_E(LOG_TAG, "[%s] detect sensor failed , need to retry, retry_time = %d",
                    __func__, retry_time);
                usleep(5000);
            }
        }
        while (retry_time > 0);
        if (err != GF_SUCCESS)
        {
            break;
        }

        memcpy(&g_hal_config, &cmd_common->config, sizeof(gf_config_t));
        size = sizeof(gf_init_t);
        cmd_init = (gf_init_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd_init)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd_init, 0, size);
        err = gf_ca_invoke_command(GF_OPERATION_ID, GF_CMD_INIT, cmd_init, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        g_sensor_row = cmd_init->row;
        g_sensor_col = cmd_init->col;
        g_sensor_nav_row = cmd_init->nav_row;
        g_sensor_nav_col = cmd_init->nav_col;
        gf_dump_init(g_sensor_row, g_sensor_col, g_sensor_nav_row, g_sensor_nav_col,
                     g_hal_config.chip_type, g_hal_config.chip_series);
        gf_hal_judge_delete_frr_database(g_hal_config.chip_type,
                                         g_hal_config.chip_series);
        g_esd_check_flag = cmd_init->esd_check_flag;

        if (GF_OSWEGO_M == g_hal_config.chip_series)
        {
            size = sizeof(gf_cmd_header_t);
            memset(cmd_common, 0, size);
            err = gf_ca_invoke_command(GF_OPERATION_ID, GF_CMD_DOWNLOAD_CFG, cmd_common,
                                       size);

            if (err != GF_SUCCESS)
            {
                break;
            }
        }

        size = sizeof(gf_init_finished_t);
        cmd_init_finished = (gf_init_finished_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd_init_finished)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd_init_finished, 0, size);
        err = gf_ca_invoke_command(GF_OPERATION_ID, GF_CMD_INIT_FINISHED,
                                   cmd_init_finished, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }  // do hal_reinit
    while (0);

    gf_hal_control_spi_clock(0);

    if (cmd_init_finished != NULL)
    {
        GF_MEM_FREE(cmd_init_finished);
    }

    if (cmd_common != NULL)
    {
        GF_MEM_FREE(cmd_common);
    }

    if (cmd_init != NULL)
    {
        GF_MEM_FREE(cmd_init);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_invoke_command
 * Description: Execute a command in TA.
 * Input: cmd_id, buffer, len
 * Output: buffer
 * Return: gf_error_t
 */
gf_error_t gf_hal_invoke_command(gf_cmd_id_t cmd_id, void *buffer, int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    if (NULL == buffer)
    {
        LOG_E(LOG_TAG, "[%s] buffer is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        err = g_hal_function.invoke_command(GF_OPERATION_ID, cmd_id, buffer, len);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_invoke_command_ex
 * Description: This is a external interface, create command and send it to TA.
 * Input: cmd_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_invoke_command_ex(gf_cmd_id_t cmd_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(cmd_id, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_nav_code_convert
 * Description: Convert navigation code according to the rotation angle of sensor.
 * Input: nav_code, converted_nav_code
 * Output: None
 * Return: gf_error_t
 */
void gf_hal_nav_code_convert(gf_nav_code_t nav_code,
                             gf_nav_code_t *converted_nav_code)
{
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] g_orientation=%u, g_facing=%u, nav=%s, nav_code=%d",
          __func__,
          g_orientation, g_facing, gf_strnav(nav_code), nav_code);

    if (NULL == converted_nav_code)
    {
        LOG_E(LOG_TAG, "[%s] converted_nav_code is null", __func__);
        return;
    }

    do
    {
        if (nav_code == GF_NAV_CLICK || nav_code == GF_NAV_HEAVY
            || nav_code == GF_NAV_DOUBLE_CLICK
            || nav_code == GF_NAV_LONG_PRESS)
        {
            *converted_nav_code = nav_code;
            break;
        }

        switch (g_orientation)
        {
            case 0:
            {
                *converted_nav_code = nav_code;
                break;
            }

            case 90:
            {
                if (GF_NAV_UP == nav_code)
                {
                    *converted_nav_code = GF_NAV_RIGHT;
                }
                else if (GF_NAV_RIGHT == nav_code)
                {
                    *converted_nav_code = GF_NAV_DOWN;
                }
                else if (GF_NAV_DOWN == nav_code)
                {
                    *converted_nav_code = GF_NAV_LEFT;
                }
                else if (GF_NAV_LEFT == nav_code)
                {
                    *converted_nav_code = GF_NAV_UP;
                }

                break;
            }

            case 180:
            {
                if (GF_NAV_UP == nav_code)
                {
                    *converted_nav_code = GF_NAV_DOWN;
                }
                else if (GF_NAV_RIGHT == nav_code)
                {
                    *converted_nav_code = GF_NAV_LEFT;
                }
                else if (GF_NAV_DOWN == nav_code)
                {
                    *converted_nav_code = GF_NAV_UP;
                }
                else if (GF_NAV_LEFT == nav_code)
                {
                    *converted_nav_code = GF_NAV_RIGHT;
                }

                break;
            }

            case 270:
            {
                if (GF_NAV_UP == nav_code)
                {
                    *converted_nav_code = GF_NAV_LEFT;
                }
                else if (GF_NAV_RIGHT == nav_code)
                {
                    *converted_nav_code = GF_NAV_UP;
                }
                else if (GF_NAV_DOWN == nav_code)
                {
                    *converted_nav_code = GF_NAV_RIGHT;
                }
                else if (GF_NAV_LEFT == nav_code)
                {
                    *converted_nav_code = GF_NAV_DOWN;
                }

                break;
            }

            default:
            {
                break;
            }
        }  // switch sensor orientation

        if (GF_SENSOR_FACING_BACK == g_facing)
        {
            if (GF_NAV_LEFT == *converted_nav_code)
            {
                *converted_nav_code = GF_NAV_RIGHT;
            }
            else if (GF_NAV_RIGHT == *converted_nav_code)
            {
                *converted_nav_code = GF_NAV_LEFT;
            }
        }
    }  // do gf_hal_nav_code_convert
    while (0);

    LOG_D(LOG_TAG,
          "[%s] nav=%s, nav_code=%d, converted_nav=%s, converted_nav_code=%d", __func__,
          gf_strnav(nav_code), nav_code, gf_strnav(*converted_nav_code),
          *converted_nav_code);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_download_fw
 * Description: Download firmware to sensor.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_download_fw()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_invoke_command_ex(GF_CMD_DOWNLOAD_FW);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_download_cfg
 * Description: Download configuration information to sensor.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_download_cfg()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    err = gf_hal_invoke_command_ex(GF_CMD_DOWNLOAD_CFG);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_esd_timer_thread
 * Description: The thread of esd check timer.
 * Input: v
 * Output: None
 * Return: void
 */
static void hal_esd_timer_thread(union sigval v)
{
    gf_error_t err = GF_SUCCESS;
    gf_esd_check_t *cmd = NULL;
    uint32_t size = sizeof(gf_esd_check_t);
    FUNC_ENTER();
    UNUSED_VAR(v);
    pthread_mutex_lock(&g_hal_mutex);

    do
    {
        if (0 == g_hal_inited)
        {
            err = GF_ERROR_GENERIC;
            LOG_E(LOG_TAG, "[%s] hal is not initialized", __func__);
            break;
        }

        cmd = (gf_esd_check_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_ESD_CHECK, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        if (cmd->download_fw_flag > 0 || cmd->download_cfg_flag > 0)
        {
            gf_hal_destroy_timer(&g_esd_timer_id);
        }

        if (cmd->download_fw_flag > 0)
        {
            err = gf_hal_download_fw();

            if (err != GF_SUCCESS)
            {
                break;
            }
        }

        if ((cmd->cmd_header.reset_flag > 0)
            && (g_hal_config.chip_series == GF_OSWEGO_M))
        {
            LOG_D(LOG_TAG, "[%s] failed", __func__);
            err = gf_hal_download_cfg();

            if (err != GF_SUCCESS)
            {
                break;
            }
        }
    }  // do hal_esd_timer_thread
    while (0);

    if (GF_CHECK_NORMAL == g_esd_check_flag)
    {
        if ((NULL != cmd) && (cmd->download_fw_flag > 0 || cmd->download_cfg_flag > 0))
        {
            gf_hal_create_and_set_esd_timer();
        }
    }
    else
    {
        gf_hal_destroy_timer(&g_esd_timer_id);
    }

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    pthread_mutex_unlock(&g_hal_mutex);
    FUNC_EXIT(err);
}

/**
 * Function: gf_hal_create_and_set_esd_timer
 * Description: Create and initialize esd check timer.
 * Input: None
 * Output: None
 * Return: void
 */
void gf_hal_create_and_set_esd_timer()
{
    VOID_FUNC_ENTER();

    do
    {
        if (GF_CHECK_NORMAL != g_esd_check_flag)
        {
            break;
        }

        if (gf_hal_create_timer(&g_esd_timer_id, hal_esd_timer_thread) != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] hal_create_timer failed", __func__);
            break;
        }

        if (gf_hal_set_timer(&g_esd_timer_id, HAL_ESD_TIMER_SEC, HAL_ESD_TIMER_SEC, 0) != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] hal_set_timer failed", __func__);
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_nav_listener
 * Description: Report navigation event.
 * Input: nav_code
 * Output: None
 * Return: void
 */
void gf_hal_nav_listener(gf_nav_code_t nav_code)
{
    VOID_FUNC_ENTER();
    LOG_I(LOG_TAG, "[%s] nav=%s, nav_code=%d", __func__, gf_strnav(nav_code),
          nav_code);
    gf_nav_code_t read_nav = GF_NAV_NONE;
    gf_hal_nav_reset();
    gf_hal_nav_code_convert(nav_code, &read_nav);
    gf_hal_send_nav_event(read_nav);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_nav_reset
 * Description: Reset click status and releated timers.
 * Input: None
 * Output: None
 * Return: void
 */
void gf_hal_nav_reset()
{
    VOID_FUNC_ENTER();
    g_nav_click_status = GF_NAV_CLICK_STATUS_NONE;
    gf_hal_destroy_timer(&g_nav_double_click_timer_id);
    gf_hal_destroy_timer(&g_nav_long_press_timer_id);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_nav_long_press_timer_thread
 * Description: The thread of checking long press.
 * Input: v
 * Output: None
 * Return: void
 */
void gf_hal_nav_long_press_timer_thread(union sigval v)
{
    UNUSED_VAR(v);
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] get a long press", __func__);
    pthread_mutex_lock(&g_nav_click_status_mutex);
    gf_hal_send_nav_event(GF_NAV_LONG_PRESS);
    gf_hal_nav_reset();
    pthread_mutex_unlock(&g_nav_click_status_mutex);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_nav_double_click_timer_thread
 * Description: The thread of checking double click.
 * Input: v
 * Output: None
 * Return: void
 */
void gf_hal_nav_double_click_timer_thread(union sigval v)
{
    UNUSED_VAR(v);
    VOID_FUNC_ENTER();
    pthread_mutex_lock(&g_nav_click_status_mutex);

    if (GF_NAV_CLICK_STATUS_DOWN_UP == g_nav_click_status)
    {
        LOG_D(LOG_TAG, "[%s] get a click", __func__);
        gf_hal_nav_reset();
        gf_hal_send_nav_event(GF_NAV_CLICK);
    }
    else if (GF_NAV_CLICK_STATUS_DOWN == g_nav_click_status)
    {
        gf_hal_destroy_timer(&g_nav_double_click_timer_id);
    }

    pthread_mutex_unlock(&g_nav_click_status_mutex);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_nav_complete
 * Description: Notify finger lifted event to TA.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_nav_complete(void)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    uint32_t size = sizeof(gf_cmd_header_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_NAVIGATE_COMPLETE, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_nav_assert_config_interval
 * Description: Set interval time for long press and double click .
 * Input: None
 * Output: None
 * Return: void
 */
void gf_hal_nav_assert_config_interval()
{
    VOID_FUNC_ENTER();

    if (g_hal_config.nav_double_click_interval_in_ms > 0)
    {
        if (g_hal_config.nav_double_click_interval_in_ms > GF_NAV_DOUBLE_CLICK_MAX_INTERVAL_IN_MS
            || g_hal_config.nav_double_click_interval_in_ms < GF_NAV_DOUBLE_CLICK_MIN_INTERVAL_IN_MS)
        {
            g_hal_config.nav_double_click_interval_in_ms =
                GF_NAV_DOUBLE_CLICK_DEFAULT_INTERVAL_IN_MS;
        }
    }

    if (g_hal_config.nav_long_press_interval_in_ms > 0)
    {
        if (g_hal_config.nav_long_press_interval_in_ms > GF_NAV_LONG_PRESS_MAX_INTERVAL_IN_MS
            || g_hal_config.nav_long_press_interval_in_ms < GF_NAV_LONG_PRESS_MIN_INTERVAL_IN_MS)
        {
            g_hal_config.nav_long_press_interval_in_ms =
                GF_NAV_LONG_PRESS_DEFAULT_INTERVAL_IN_MS;
        }

        if (g_hal_config.nav_long_press_interval_in_ms <=
            g_hal_config.nav_double_click_interval_in_ms)
        {
            g_hal_config.nav_double_click_interval_in_ms =
                GF_NAV_DOUBLE_CLICK_DEFAULT_INTERVAL_IN_MS;
            g_hal_config.nav_long_press_interval_in_ms =
                GF_NAV_LONG_PRESS_DEFAULT_INTERVAL_IN_MS;
        }
    }

    LOG_D(LOG_TAG, "[%s] g_hal_config.nav_double_click_interval_in_ms=%u, ",
          __func__,
          g_hal_config.nav_double_click_interval_in_ms);
    LOG_D(LOG_TAG, "[%s] g_hal_config.nav_long_press_interval_in_ms=%u", __func__,
          g_hal_config.nav_long_press_interval_in_ms);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_long_pressed_timer_thread
 * Description: Report GF_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG, and destroy timer.
 * Input: v
 * Output: None
 * Return: void
 */
void gf_hal_long_pressed_timer_thread(union sigval v)
{
    UNUSED_VAR(v);
    gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG);
    gf_hal_destroy_timer(&g_long_pressed_timer_id);
}

/**
 * Function: gf_hal_current_time_microsecond
 * Description: Get current time.
 * Input: None
 * Output: None
 * Return: int64_t
 */
int64_t gf_hal_current_time_microsecond(void)
{
    struct timeval now = { 0 };
    gettimeofday(&now, 0);
    return now.tv_sec * 1000000L + now.tv_usec;
}

/**
 * Function: gf_hal_current_time_second
 * Description: Get current time.
 * Input: None
 * Output: None
 * Return: int64_t
 */
int64_t gf_hal_current_time_second(void)
{
    struct timeval now = { 0 };
    gettimeofday(&now, 0);
    return now.tv_sec;
}

/**
 * Function: gf_hal_notify_authentication_succeeded
 * Description: Notify GF_FINGERPRINT_AUTHENTICATED when authentication succeeded.
 * Input: None
 * Output: None
 * Return: void
 */
void gf_hal_notify_authentication_succeeded(uint32_t group_id,
                                            uint32_t finger_id,
                                            gf_hw_auth_token_t *auth_token)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL && auth_token != NULL)
    {
        fingerprint_msg_t message = { 0 };
        message.type = FINGERPRINT_AUTHENTICATED;
        message.data.authenticated.finger.gid = group_id;
        message.data.authenticated.finger.fid = finger_id;
        memcpy(&message.data.authenticated.hat, auth_token, sizeof(gf_hw_auth_token_t));
        g_fingerprint_device->notify(&message);
    }
}

/**
 * Function: gf_hal_notify_authentication_failed
 * Description: Notify GF_FINGERPRINT_AUTHENTICATED when authentication failed.
 * Input: None
 * Output: None
 * Return: void
 */
void gf_hal_notify_authentication_failed()
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL)
    {
        fingerprint_msg_t message = { 0 };
        message.type = FINGERPRINT_AUTHENTICATED;
        message.data.authenticated.finger.gid = 0;
        message.data.authenticated.finger.fid = 0;  // 0 is authenticate failed
        g_fingerprint_device->notify(&message);
    }

    g_failed_attempts++;

    if (g_hal_config.max_authenticate_failed_attempts > 0
        && g_failed_attempts >= g_hal_config.max_authenticate_failed_attempts)
    {
        LOG_E(LOG_TAG, "[%s] too many attempts. please try again later", __func__);
        g_hal_function.cancel((void *) g_fingerprint_device);
#if defined(__ANDROID_O) || defined(__ANDROID_P)
        gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_LOCKOUT);
#endif  // __ANDROID_O
    }
}

/**
 * Function: gf_hal_notify_authentication_fido_failed
 * Description: Notify GF_FINGERPRINT_AUTHENTICATED_FIDO when fido authentication failed.
 * Input: None
 * Output: None
 * Return: void
 */
void gf_hal_notify_authentication_fido_failed()
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
    {
        gf_fingerprint_msg_t message = { 0 };
        message.type = GF_FINGERPRINT_AUTHENTICATED_FIDO;
        message.data.authenticated_fido.finger_id = 0;
        message.data.authenticated_fido.uvt_len = 0;
        //g_fingerprint_device->test_notify(&message);
    }

    g_failed_attempts++;

    if (g_hal_config.max_authenticate_failed_attempts > 0
        && g_failed_attempts >= g_hal_config.max_authenticate_failed_attempts)
    {
        LOG_E(LOG_TAG, "[%s] too many attempts. please try again later", __func__);
        g_hal_function.cancel((void *) g_fingerprint_device);
    }
}

/**
 * Function: gf_hal_notify_remove_succeeded
 * Description: Notify GF_FINGERPRINT_TEMPLATE_REMOVED when the template has been removed.
 * Input: group_id, finger_id, remaining_templates
 * Output: None
 * Return: void
 */
void gf_hal_notify_remove_succeeded(uint32_t group_id, uint32_t finger_id, uint32_t remaining_templates)
{
    if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL)
    {
        fingerprint_msg_t message = { 0 };
        uint32_t err = 0;
        int32_t indices_count = MAX_FINGERS_PER_USER;
        uint32_t indices[MAX_FINGERS_PER_USER] = {0};

        memset(&message, 0, sizeof(fingerprint_msg_t));
        err = gf_hal_common_get_id_list(g_fingerprint_device, group_id, indices, &indices_count);
        if (err) {
            LOG_E(LOG_TAG, "[%s] gf_ha_common_get_id_list error~~~", __func__);
            return;
        }
        LOG_I(LOG_TAG, "[%s] ids count = %d", __func__, indices_count);

        for (uint32_t i = 0; i < indices_count; i++) {
            LOG_I(LOG_TAG, "[%s] ids[%d] = %u", __func__, i, indices[i]);
            message.data.removed.total_fingers[i].fid = indices[i];
            message.data.removed.total_fingers[i].gid = group_id;
        }

        message.type = FINGERPRINT_TEMPLATE_REMOVED;
        message.data.removed.finger.gid = group_id;
        message.data.removed.finger.fid = finger_id;
        message.data.removed.fingers_count = remaining_templates;
//#if defined(__ANDROID_O) || defined(__ANDROID_P)
        //message.data.removed.fingers_count = remaining_templates; //for oppo
        LOG_I(LOG_TAG, "[%s] remove finger, fid = %u, gid = %u", __func__, message.data.removed.finger.fid, message.data.removed.finger.gid);
        g_fingerprint_device->notify(&message);
    }
}

/**
 * Function: gf_hal_dump_performance
 * Description: Dump performance data.
 * Input: func_name, operation, dump_performance
 * Output: None
 * Return: void
 */
void gf_hal_dump_performance(const char *func_name,
                             gf_operation_type_t operation,
                             gf_test_performance_t *dump_performance)
{
    int64_t time = 0;

    if (NULL == func_name || NULL == dump_performance)
    {
        LOG_E(LOG_TAG, "[%s] func_name or dump_performance is null", __func__);
        return;
    }

    if (0 == g_hal_config.support_performance_dump)
    {
        return;
    }

    switch (operation)
    {
        case OPERATION_ENROLL:
        case OPERATION_TEST_UNTRUSTED_ENROLL:
        {
            LOG_I(LOG_TAG, "[%s]     image_quality=%d", func_name,
                  dump_performance->image_quality);
            LOG_I(LOG_TAG, "[%s]        valid_area=%d", func_name,
                  dump_performance->valid_area);
            LOG_I(LOG_TAG, "[%s]     key_point_num=%d", func_name,
                  dump_performance->key_point_num);
            LOG_I(LOG_TAG, "[%s]     increase_rate=%d", func_name,
                  dump_performance->increase_rate);
            LOG_I(LOG_TAG, "[%s]           overlay=%d", func_name,
                  dump_performance->overlay);
            LOG_I(LOG_TAG, "[%s] get_raw_data_time=%dms", func_name,
                  dump_performance->get_raw_data_time / 1000);

            if (GF_MILAN_A_SERIES == g_hal_config.chip_series)
            {
                LOG_I(LOG_TAG, "[%s] get_gsc_data_time=%dms", func_name,
                      dump_performance->get_gsc_data_time / 1000);
            }

            if (g_hal_config.support_sensor_broken_check > 0)
            {
                LOG_I(LOG_TAG, "[%s] broken_check_time=%dms", func_name,
                      dump_performance->broken_check_time / 1000);
            }

            LOG_I(LOG_TAG, "[%s]   preprocess_time=%dms", func_name,
                  dump_performance->preprocess_time / 1000);
            LOG_I(LOG_TAG, "[%s]  get_feature_time=%dms", func_name,
                  dump_performance->get_feature_time / 1000);
            LOG_I(LOG_TAG, "[%s]       enroll_time=%dms", func_name,
                  dump_performance->enroll_time / 1000);
            break;
        }  // case OPERATION_TEST_UNTRUSTED_ENROLL

        case OPERATION_AUTHENTICATE_FF:
        case OPERATION_AUTHENTICATE_IMAGE:
        case OPERATION_TEST_UNTRUSTED_AUTHENTICATE:
        case OPERATION_AUTHENTICATE_FIDO:
        {
            LOG_I(LOG_TAG, "[%s]     image_quality=%d", func_name,
                  dump_performance->image_quality);
            LOG_I(LOG_TAG, "[%s]        valid_area=%d", func_name,
                  dump_performance->valid_area);
            LOG_I(LOG_TAG, "[%s]     key_point_num=%d", func_name,
                  dump_performance->key_point_num);
            LOG_I(LOG_TAG, "[%s]       match_score=%d", func_name,
                  dump_performance->match_score);

            if (GF_MILAN_A_SERIES == g_hal_config.chip_series)
            {
                LOG_I(LOG_TAG, "[%s]     bio_assay_ret=%d", func_name,
                      dump_performance->bio_assay_ret);
            }
            LOG_I(LOG_TAG, "[%s] polling_image_time=%dms", func_name,
                  dump_performance->polling_image_time / 1000);

            LOG_I(LOG_TAG, "[%s] get_raw_data_time=%dms", func_name,
                  dump_performance->get_raw_data_time / 1000);

            if (GF_MILAN_A_SERIES == g_hal_config.chip_series)
            {
                LOG_I(LOG_TAG, "[%s] get_gsc_data_time=%dms", func_name,
                      dump_performance->get_gsc_data_time / 1000);
            }

            if (g_hal_config.support_sensor_broken_check > 0)
            {
                LOG_I(LOG_TAG, "[%s] broken_check_time=%dms", func_name,
                      dump_performance->broken_check_time / 1000);
            }

            LOG_I(LOG_TAG, "[%s]   preprocess_time=%dms", func_name,
                  dump_performance->preprocess_time / 1000);
            LOG_I(LOG_TAG, "[%s]  get_feature_time=%dms", func_name,
                  dump_performance->get_feature_time / 1000);
            LOG_I(LOG_TAG, "[%s] authenticate_time=%dms", func_name,
                  dump_performance->authenticate_time / 1000);
            LOG_I(LOG_TAG, "[%s] KPI time=%dms", func_name,
                  (dump_performance->polling_image_time
                          + dump_performance->get_raw_data_time
                          + dump_performance->preprocess_time
                          + dump_performance->get_feature_time
                          + dump_performance->authenticate_time) / 1000);
            break;
        }  // case OPERATION_AUTHENTICATE_FIDO

        default:
        {
            break;
        }
    }  // switch operation

    time = gf_hal_current_time_microsecond() - g_down_irq_time;
    LOG_I(LOG_TAG, "[%s]        total time=%dms", func_name, (int32_t)(time / 1000));
}

/**
 * Function: gf_hal_save
 * Description: Save template to file when enrollment complete.
 * Input: group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_save(uint32_t group_id, uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_save_t *cmd = NULL;
    uint32_t size = sizeof(gf_save_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_save_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->finger_id = finger_id;
        cmd->group_id = group_id;
        err = gf_hal_invoke_command(GF_CMD_SAVE, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_update_stitch
 * Description: Update stitch.
 * Input: group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_update_stitch(uint32_t group_id, uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_update_stitch_t *cmd = NULL;
    uint32_t size = sizeof(gf_update_stitch_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_update_stitch_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->finger_id = finger_id;
        cmd->group_id = group_id;
        err = gf_hal_invoke_command(GF_CMD_UPDATE_STITCH, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_init_finished
 * Description: Finish initializtion and dump initial data.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_init_finished()
{
    gf_error_t err = GF_SUCCESS;
    gf_init_finished_t *cmd = NULL;
    uint32_t size = sizeof(gf_init_finished_t);
    FUNC_ENTER();

    do
    {
        cmd = (gf_init_finished_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_INIT_FINISHED, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        g_orientation = cmd->orientation;
        g_facing = cmd->facing;
        g_hal_function.dump_chip_init_data();
    }
    while (0);

    if (g_hal_config.support_detect_sensor_temperature)
    {
        g_sensor_broken_check_mode = 1;//full check
        gf_hal_post_sem_detect_broken();
    }

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_close
 * Description: Close device and free resource.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_close(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_cmd_header_t *cmd = NULL;
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        uint32_t size = 0;
        gf_hal_destroy_timer(&g_enroll_timer_id);
        gf_hal_destroy_timer(&g_long_pressed_timer_id);
        gf_hal_device_disable();
        size = sizeof(gf_cmd_header_t);
        cmd = (gf_cmd_header_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_EXIT, cmd, sizeof(gf_cmd_header_t));
        gf_ca_close_session();
        gf_hal_disable_power();
        gf_hal_device_close();
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_cancel
 * Description: Destroy timers, stop checking long press.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_cancel(void *dev)
{
    FUNC_ENTER();
    UNUSED_VAR(dev);
    gf_hal_destroy_timer(&g_enroll_timer_id);
    gf_hal_destroy_timer(&g_long_pressed_timer_id);
    FUNC_EXIT(GF_SUCCESS);
    return GF_SUCCESS;
}

/**
 * Function: gf_hal_common_pre_enroll
 * Description: Generate 64 bit rand number, and prepare for enrollment.
 * Input: dev
 * Output: None
 * Return: uint64_t
 */
uint64_t gf_hal_common_pre_enroll(void *dev)
{
    uint64_t ret = 0;
    gf_error_t err = GF_SUCCESS;
    gf_pre_enroll_t *cmd = NULL;
    uint32_t size = sizeof(gf_pre_enroll_t);
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_pre_enroll_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_PRE_ENROLL, cmd, size);
        ret = cmd->challenge;
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return ret;
}

/**
 * Function: hal_enroll_timer_thread
 * Description: Report GF_FINGERPRINT_ERROR_TIMEOUT message when enroll timeout.
 * Input: v
 * Output: None
 * Return: void
 */
void hal_enroll_timer_thread(union sigval v)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(v);
    gf_hal_destroy_timer(&g_enroll_timer_id);

    gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_TIMEOUT);

    if (g_hal_function.cancel((void *) g_fingerprint_device) != GF_SUCCESS)
    {
        LOG_D(LOG_TAG, "[%s] hal_cancel failed", __func__);
    }

    VOID_FUNC_EXIT();
}

/**
 * Function: hal_enroll_timer_thread_pre
 * Description: Report GF_FINGERPRINT_ERROR_TIMEOUT message when enroll timeout.
 * Input: v
 * Output: None
 * Return: void
 */
void hal_enroll_timer_thread_t(union sigval v)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(v);
    gf_hal_destroy_timer(&g_enroll_timer_id_pre);

    gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_TIMEOUT);

    if (g_hal_function.cancel((void *) g_fingerprint_device) != GF_SUCCESS)
    {
        LOG_D(LOG_TAG, "[%s] hal_cancel failed", __func__);
    }

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_common_enroll
 * Description: Wait finger down, and start enrollment operation.
 * Input: dev, hat, group_id, timeout_sec
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_enroll(void *dev, const void *hat, uint32_t group_id,
                                uint32_t timeout_sec)
{
    gf_error_t err = GF_SUCCESS;
    gf_enroll_t *cmd = NULL;
    uint32_t size = sizeof(gf_enroll_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, timeout_sec=%u", __func__, group_id,
          timeout_sec);
    UNUSED_VAR(dev);

    do
    {
        if ((NULL == hat) || (NULL == dev))
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG, "[%s] bad paramters", __func__);
            break;
        }

        gf_hal_destroy_timer(&g_long_pressed_timer_id);

        if (timeout_sec != 0)
        {
            g_enroll_timer_sec = timeout_sec;
            gf_hal_create_timer(&g_enroll_timer_id, hal_enroll_timer_thread);
            gf_hal_set_timer(&g_enroll_timer_id, g_enroll_timer_sec, g_enroll_timer_sec, 0);
        }

        cmd = (gf_enroll_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_ENROLL;
        cmd->group_id = group_id;
        cmd->system_auth_token_version = GF_HW_AUTH_TOKEN_VERSION;
        if (g_hal_config.support_swipe_enroll > 0)
        {
            cmd->dump_swipe_enroll_enable = gf_is_dump_enabled();
            LOG_D(LOG_TAG, "[%s] cmd->dump_swipe_enroll_enable=%u", __func__, cmd->dump_swipe_enroll_enable);
        }
        memcpy(&cmd->hat, hat, sizeof(gf_hw_auth_token_t));
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] err = %u", __func__, err);
            if (GF_ERROR_ENROLL_TIMEOUT == err) {
               gf_hal_create_timer(&g_enroll_timer_id_pre, hal_enroll_timer_thread_t); 
               gf_hal_set_timer(&g_enroll_timer_id_pre, 0, 0, 100000);
               err = GF_SUCCESS;
            }

#if defined(__ANDROID_O) || defined(__ANDROID_P)
            // support android o vts
            if (GF_ERROR_INVALID_CHALLENGE == err || GF_ERROR_INVALID_HAT_VERSION == err)
            {
                LOG_E(LOG_TAG, "[%s] hardware unavailable", __func__);
                gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_HW_UNAVAILABLE);
                err = GF_SUCCESS;
            }
#endif  // __ANDROID_O
            break;
        }
        g_work_state = STATE_ENROLL;

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
        g_enroll_invalid_template_num = 0;
    }  // do gf_hal_common_enroll
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_post_enroll
 * Description: Reset g_challenge after enroll finger.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_post_enroll(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    err = gf_hal_invoke_command_ex(GF_CMD_POST_ENROLL);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_authenticate
 * Description: Wait finger down, and start authentication operation.
 * Input: dev, hat, group_id, timeout_sec
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_authenticate(void *dev, uint64_t operation_id,
                                      uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_authenticate_t *cmd = NULL;
    uint32_t size = sizeof(gf_authenticate_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    int32_t screen_state = -1;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u", __func__, group_id);
    UNUSED_VAR(dev);

    do
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        gf_hal_destroy_timer(&g_long_pressed_timer_id);
        gf_hal_get_screen_state(&screen_state);
        g_failed_attempts = 0;
        cmd = (gf_authenticate_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_AUTHENTICATE;
        cmd->group_id = group_id;
        cmd->operation_id = operation_id;
        cmd->screen_on_flag = screen_state;
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
    }  // do gf_hal_common_authenticate
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_get_auth_id
 * Description: Get auth id.
 * Input: dev
 * Output: None
 * Return: uint64_t
 */
uint64_t gf_hal_common_get_auth_id(void *dev)
{
    uint64_t ret = 0;
    gf_error_t err = GF_SUCCESS;
    gf_get_auth_id_t *cmd = NULL;
    uint32_t size = sizeof(gf_get_auth_id_t);
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_get_auth_id_t *) GF_MEM_MALLOC(sizeof(gf_get_auth_id_t));

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, sizeof(gf_get_auth_id_t));
        err = gf_hal_invoke_command(GF_CMD_GET_AUTH_ID, cmd, size);
        ret = cmd->auth_id;
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return ret;
}

/**
 * Function: gf_hal_common_remove_without_callback
 * Description: Remove finger template without callback.
 * Input: group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_remove_without_callback(uint32_t group_id,
                                                 uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_remove_t *cmd = NULL;
    uint32_t size = sizeof(gf_remove_t);
    uint32_t i = 0;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);

    do
    {
        cmd = (gf_remove_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->group_id = group_id;
        cmd->finger_id = finger_id;
        err = gf_hal_invoke_command(GF_CMD_REMOVE, cmd, size);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] something wrong happens", __func__);
        }

        for (i = 0; i < MAX_FINGERS_PER_USER; i++)
        {
            if (0 != cmd->deleted_fids[i])
            {
                LOG_D(LOG_TAG, "[%s] remove finger. gid=%u, fid=%u", __func__,
                      cmd->deleted_gids[i], cmd->deleted_fids[i]);
            }
        }
    }  // do gf_hal_common_remove_without_callback
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_remove_for_sync_list
 * Description: Remove finger template without callback.
 * Input: group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_remove_for_sync_list(void *dev, uint32_t group_id,
                                              uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    err = gf_hal_common_remove_without_callback(group_id, finger_id);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_remove
 * Description: Remove finger template.
 * Input: group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_remove(void *dev, uint32_t group_id,
                                uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_remove_t *cmd = NULL;
    uint32_t size = sizeof(gf_remove_t);
    uint32_t i = 0;
    uint32_t remaining_templates = 0;

    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, group_id, finger_id);
    //UNUSED_VAR(dev);

    do
    {
        cmd = (gf_remove_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->group_id = group_id;
        cmd->finger_id = finger_id;
        err = gf_hal_invoke_command(GF_CMD_REMOVE, cmd, size);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] something wrong happens", __func__);
        }

        LOG_D(LOG_TAG, "[%s] removing templates : %u.", __func__, cmd->removing_templates);
        if (0 == cmd->removing_templates)
        {
            LOG_D(LOG_TAG, "[%s] no fingers are removed.", __func__);
#if defined(__ANDROID_O) || defined(__ANDROID_P)
            remaining_templates = 0;
            gf_hal_notify_remove_succeeded(group_id, finger_id, remaining_templates);
#endif  // __ANDROID_O
        }
        else
        {
            for (i = 0; i < MAX_FINGERS_PER_USER && 0 != cmd->deleted_fids[i]; i++)
            {
                LOG_D(LOG_TAG, "[%s] remove finger. gid=%u, fid=%u, removing_templates=%u", __func__,
                      cmd->deleted_gids[i], cmd->deleted_fids[i], cmd->removing_templates);
                remaining_templates = cmd->removing_templates - i -1;
                gf_hal_notify_remove_succeeded(cmd->deleted_gids[i], cmd->deleted_fids[i], remaining_templates);
            }
        }

#ifdef __ANDROID_N
        gf_hal_notify_remove_succeeded(0, 0, remaining_templates);
#endif  // __ANDROID_N

        err = do_enumerate(dev, group_id, finger_id);
        if (0 != err) {
            LOG_E(LOG_TAG, "[%s] do enumerate failed with status = %d", __func__, err);
        }
    }  // do gf_hal_common_remove
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_set_active_group
 * Description: Set active user group.
 * Input: group_id, dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_set_active_group(void *dev, uint32_t group_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_set_active_group_t *cmd = NULL;
    uint32_t size = sizeof(gf_set_active_group_t);
    UNUSED_VAR(dev);
    FUNC_ENTER();

    do
    {
        cmd = (gf_set_active_group_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->group_id = group_id;
        pthread_mutex_lock(&g_hal_mutex);
        err = gf_hal_invoke_command(GF_CMD_SET_ACTIVE_GROUP, cmd, size);
        g_set_active_group_done = 1;
        LOG_D(LOG_TAG, "[%s] set active group done, g_set_active_group_done=%u.", __func__,
                g_set_active_group_done);
        pthread_mutex_unlock(&g_hal_mutex);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_enumerate
 * Description: Enumerate finger list.
 * Input: dev, result, max_size
 * Output: result
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_enumerate(void *dev, void *result, uint32_t *max_size)
{
    gf_fingerprint_finger_id_t *results = (gf_fingerprint_finger_id_t *) result;
    gf_error_t err = GF_SUCCESS;
    gf_enumerate_t *cmd = NULL;
    uint32_t size = sizeof(gf_enumerate_t);
    uint32_t i = 0;
    uint32_t len = 0;
    uint32_t count = 0;
    FUNC_ENTER();
    UNUSED_VAR(dev);

    if ((NULL == results) || (NULL == max_size))
    {
        LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    len = g_hal_config.max_fingers_per_user < *max_size ?
          g_hal_config.max_fingers_per_user :
          *max_size;
    LOG_D(LOG_TAG, "[%s] size=%u", __func__, len);

    do
    {
        cmd = (gf_enumerate_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_ENUMERATE, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        for (i = 0; i < len; i++)
        {
            LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u", __func__, i,
                  cmd->group_ids[i], i, cmd->finger_ids[i]);

            if (cmd->finger_ids[i] != 0)
            {
                results[count].gid = cmd->group_ids[i];
                results[count].fid = cmd->finger_ids[i];
                count++;
            }
        }

        LOG_D(LOG_TAG, "[%s] size=%u", __func__, count);
    }  // do gf_hal_common_enumerate
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }
    g_enum_fingers = count;
    *max_size = count;
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_enumerate_with_callback
 * Description: Enumerate finger list without callback.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_enumerate_with_callback(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    gf_enumerate_t *cmd = NULL;
    uint32_t size = sizeof(gf_enumerate_t);
    uint32_t i = 0;
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_enumerate_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_ENUMERATE, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        LOG_D(LOG_TAG, "[%s] size=%u", __func__, cmd->size);
        g_enum_fingers = cmd->size;

        if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL)
        {
            fingerprint_msg_t message = { 0 };
            message.type = FINGERPRINT_TEMPLATE_ENUMERATING;

            if (0 == cmd->size)
            {
                g_fingerprint_device->notify(&message);
                break;
            }

            for (i = 0; i < cmd->size; i++)
            {
                LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u, remains=%u",
                    __func__, i, cmd->group_ids[i], i, cmd->finger_ids[i], cmd->size - i - 1);

                message.data.enumerated.fingers[i].gid = cmd->group_ids[i];
                message.data.enumerated.fingers[i].fid = cmd->finger_ids[i];
                //message.data.enumerated.finger.gid = cmd->group_ids[i];
                //message.data.enumerated.finger.fid = cmd->finger_ids[i];
                //message.data.enumerated.remaining_templates = cmd->size - i - 1;
                //g_fingerprint_device->notify(&message);
            }
            message.data.enumerated.remaining_templates = cmd->size;
            g_fingerprint_device->notify(&message);
            /*
            message.data.enumerated.gid = cmd->group_ids[0];
            message.data.enumerated.remaining_templates = 5 - cmd->size;
            g_fingerprint_device->notify(&message);
            */
        }
    }  // do gf_hal_common_enumerate_with_callback
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_screen_on
 * Description: Notify screen status to TA.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_screen_on()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    g_screen_status = 1;
    err = gf_hal_invoke_command_ex(GF_CMD_SCREEN_ON);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_screen_off
 * Description: Notify screen status to TA.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_screen_off()
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    g_screen_status = 0;
    err = gf_hal_invoke_command_ex(GF_CMD_SCREEN_OFF);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_set_safe_class
 * Description: Set safe class authentication.
 * Input: dev ,safe_class
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_set_safe_class(void *dev, gf_safe_class_t safe_class)
{
    gf_error_t err = GF_SUCCESS;
    gf_set_safe_class_t *cmd = NULL;
    uint32_t size = sizeof(gf_set_safe_class_t);
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] safe_class=%d", __func__, safe_class);
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_set_safe_class_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->safe_class = safe_class;
        err = gf_hal_invoke_command(GF_CMD_SET_SAFE_CLASS, cmd, size);
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_navigate
 * Description: Set navigation mode and start navigation operation.
 * Input: dev ,nav_mode
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_navigate(void *dev, gf_nav_mode_t nav_mode)
{
    gf_error_t err = GF_SUCCESS;
    gf_nav_t *cmd = NULL;
    uint32_t size = sizeof(gf_nav_t);
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        if (nav_mode >= GF_NAV_MODE_MAX)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters, nav_mode = %d", __func__, nav_mode);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        cmd = (gf_nav_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->nav_mode = nav_mode;
        err = gf_hal_invoke_command(GF_CMD_NAVIGATE, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] GF_CMD_NAVIGATE fail, err = %d", __func__, err);
            break;
        }
    }  // do gf_hal_common_navigate
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_enable_fingerprint_module
 * Description: Enable or disable fingerprint module.
 * Input: dev ,enable_flag
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_enable_fingerprint_module(void *dev,
                                                   uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    gf_enable_fingerprint_module_t *cmd = NULL;
    uint32_t size = sizeof(gf_enable_fingerprint_module_t);
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] enable_flag=%u", __func__, enable_flag);
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_enable_fingerprint_module_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->enable_flag = enable_flag;

        if (0 == enable_flag)
        {
            if (0 == g_enable_fingerprint_module)
            {
                break;
            }

            err = gf_hal_invoke_command(GF_CMD_ENABLE_FINGERPRINT_MODULE, cmd, size);

            if (err != GF_SUCCESS)
            {
                break;
            }

            err = gf_hal_disable_power();

            if (err != GF_SUCCESS)
            {
                break;
            }

            g_enable_fingerprint_module = 0;
        }
        else
        {
            if (g_enable_fingerprint_module > 0)
            {
                break;
            }

            err = gf_hal_enable_power();

            if (err != GF_SUCCESS)
            {
                break;
            }

            g_enable_fingerprint_module = 1;
            err = gf_hal_invoke_command(GF_CMD_ENABLE_FINGERPRINT_MODULE, cmd, size);
        }
    }  // do gf_hal_common_enable_fingerprint_module
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_camera_capture
 * Description: Execute OPERATION_CAMERA_KEY, wait finger down.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_camera_capture(void *dev)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    err = gf_hal_invoke_command_ex(GF_CMD_CAMERA_CAPTURE);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_enable_ff_feature
 * Description: Enable or disable fingerflash feature.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_enable_ff_feature(void *dev, uint8_t enable_flag)
{
    gf_error_t err = GF_SUCCESS;
    gf_enable_ff_feature_t *cmd = NULL;
    uint32_t size = sizeof(gf_enable_ff_feature_t);
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] enable_flag=%u", __func__, enable_flag);
    UNUSED_VAR(dev);

    do
    {
        cmd = (gf_enable_ff_feature_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd->enable_flag = enable_flag;
        err = gf_hal_invoke_command(GF_CMD_ENABLE_FF_FEATURE, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        g_hal_config.support_ff_mode = enable_flag;
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_reset_lockout
 * Description: Reset the lock of operation priority.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_reset_lockout()
{
    gf_error_t err = GF_SUCCESS;
    gf_lockout_t *cmd = NULL;
    uint32_t size = sizeof(gf_lockout_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();

    do
    {
        cmd = (gf_lockout_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_LOCKOUT;
        cmd->lockout_flag = 0;
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_lockout
 * Description: Lock the priority of current operation.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_lockout()
{
    gf_error_t err = GF_SUCCESS;
    gf_lockout_t *cmd = NULL;
    uint32_t size = sizeof(gf_lockout_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();

    do
    {
        cmd = (gf_lockout_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_LOCKOUT;
        cmd->lockout_flag = 1;
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_sync_finger_list
 * Description: Synchronize finger list.
 * Input: dev, list, count
 * Output: list
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_sync_finger_list(void *dev, uint32_t *list,
                                          int32_t count)
{
    gf_error_t err = GF_SUCCESS;
    gf_enumerate_t *enumerate_cmd = NULL;
    uint32_t size = sizeof(gf_enumerate_t);
    gf_remove_t *remove_cmd = NULL;
    int32_t i = 0;
    int32_t j = 0;
    int32_t num_of_fingers = 0;
    int32_t callback_finger_ids[MAX_FINGERS_PER_USER] = { 0 };
    gf_fingerprint_msg_t message = { 0 };
    FUNC_ENTER();
    UNUSED_VAR(dev);

    do
    {
        /* enumerate native framework fingers */
        enumerate_cmd = (gf_enumerate_t *) GF_MEM_MALLOC(size);

        if (NULL == enumerate_cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(enumerate_cmd, 0, size);
        err = gf_hal_invoke_command(GF_CMD_ENUMERATE, enumerate_cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }

        if (enumerate_cmd->size > MAX_FINGERS_PER_USER
            || enumerate_cmd->size > g_hal_config.max_fingers_per_user)
        {
            LOG_E(LOG_TAG, "[%s] enumerate size out of bounds", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s] native framework fingerprint size: %u", __func__,
              enumerate_cmd->size);
        LOG_D(LOG_TAG, "[%s] java framework fingerprint size: %d", __func__, count);
        /* init remove_cmd */
        size = sizeof(gf_remove_t);
        remove_cmd = (gf_remove_t *) GF_MEM_MALLOC(size);

        if (NULL == remove_cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(remove_cmd, 0, size);
        remove_cmd->group_id = enumerate_cmd->group_ids[0];

        if (count > 0)
        {
            for (i = 0; i < enumerate_cmd->size; i++)
            {
                LOG_D(LOG_TAG,
                      "[%s] native framework fingerprint list: group_id[%d]=%u, finger_id[%d]=%u",
                      __func__, i,
                      enumerate_cmd->group_ids[i], i, enumerate_cmd->finger_ids[i]);

                if (enumerate_cmd->finger_ids[i] != 0)
                {
                    for (j = 0; j < count; j++)
                    {
                        LOG_D(LOG_TAG, "[%s] java framework fingerprint list: finger_id[%d]=%u",
                              __func__, j, list[j]);

                        if (enumerate_cmd->finger_ids[i] == list[j])
                        {
                            break;
                        }
                    }

                    if (j == count) /* remove invalid fingers */
                    {
                        remove_cmd->finger_id = enumerate_cmd->finger_ids[i];
                        LOG_D(LOG_TAG, "[%s] delete native framework finger: finger_id=%u", __func__,
                              enumerate_cmd->finger_ids[i]);
                        err = gf_hal_invoke_command(GF_CMD_REMOVE, remove_cmd, size);

                        if (err != GF_SUCCESS)
                        {
                            LOG_E(LOG_TAG, "[%s] something wrong happens.", __func__);
                        }
                    }
                    else /* get fingerIds not removed */
                    {
                        LOG_D(LOG_TAG, "[%s] match native framework finger: finger_id=%u", __func__,
                              enumerate_cmd->finger_ids[i]);
                        callback_finger_ids[num_of_fingers] = enumerate_cmd->finger_ids[i];
                        num_of_fingers++;
                    }
                }  // end if: enumerate_cmd->finger_ids[i] != 0
            }  // end for: enumerate_cmd
             g_enum_fingers = num_of_fingers;
            /* callback fingerIds */
            if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL)
            {
                LOG_D(LOG_TAG, "[%s] match native framework finger: count=%d", __func__,
                      num_of_fingers);
                message.type = GF_FINGERPRINT_SYNCED;
                message.data.sync.groupId = enumerate_cmd->group_ids[0];
                message.data.sync.count = num_of_fingers;
                memset(message.data.sync.fingerIds, 0, sizeof(message.data.sync.fingerIds));

                for (i = 0; i < num_of_fingers; i ++)
                {
                    message.data.sync.fingerIds[i] = (int32_t)callback_finger_ids[i];
                }

                //g_fingerprint_device->notify(&message);
            }  // end if: callback fingerIds
        }  // end if: if (count > 0)
        else
        {
            LOG_D(LOG_TAG, "[%s] remove all native framework fingerIds", __func__);
            remove_cmd->finger_id = 0;
            err = gf_hal_invoke_command(GF_CMD_REMOVE, remove_cmd, size);
            g_enum_fingers = 0;
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] something wrong happens.", __func__);
            }
        }
    }  // do gf_hal_common_sync_finger_list
    while (0);

    if (NULL != enumerate_cmd)
    {
        GF_MEM_FREE(enumerate_cmd);
        enumerate_cmd = NULL;
    }

    if (NULL != remove_cmd)
    {
        GF_MEM_FREE(remove_cmd);
        remove_cmd = NULL;
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_user_invoke_command
 * Description: Execute ta command in mutex lock.
 * Input: cmd_id, buffer, len
 * Output: buffer
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_user_invoke_command(uint32_t cmd_id, void *buffer,
                                             int32_t len)
{
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    pthread_mutex_lock(&g_sensor_mutex);

    do
    {
        if (NULL == buffer)
        {
            LOG_E(LOG_TAG, "[%s] buffer is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if(g_sensor_power_down == 1) {
            LOG_E(LOG_TAG, "[%s] power-off, exit", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_hal_control_spi_clock(1);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] spi clock enable failed", __func__);
            break;
        }
        err = gf_ca_invoke_command(GF_USER_OPERATION_ID, cmd_id, buffer, len);
        gf_hal_control_spi_clock(0);
    }
    while (0);

    pthread_mutex_unlock(&g_sensor_mutex);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_authenticate_fido
 * Description: Wait finger down, and start authentication fido operation.
 * Input: dev, group_id, aaid, aaid_len, challenge, challenge_len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_authenticate_fido(void *dev, uint32_t group_id,
                                           uint8_t *aaid,
                                           uint32_t aaid_len, uint8_t *challenge, uint32_t challenge_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_authenticate_fido_t *cmd = NULL;
    uint32_t size = sizeof(gf_authenticate_fido_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] group_id=%u", __func__, group_id);
    UNUSED_VAR(dev);

    do
    {
        if (NULL == aaid)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, aaid is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (aaid_len == 0 || aaid_len > MAX_AAID_LEN)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, aaid_len=%u", __func__, aaid_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (challenge_len == 0 || challenge_len > MAX_AAID_LEN)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, challenge_len=%u", __func__,
                  challenge_len);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (NULL == challenge)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter, challenge is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        gf_hal_destroy_timer(&g_enroll_timer_id);
        g_failed_attempts = 0;
        cmd = (gf_authenticate_fido_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memery", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_AUTHENTICATE_FIDO;
        cmd->group_id = group_id;
        cmd->aaid_len = aaid_len;
        memcpy(cmd->aaid, aaid, aaid_len);
        cmd->final_challenge_len = challenge_len;
        memcpy(cmd->final_challenge, challenge, challenge_len);
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (GF_SUCCESS != err)
        {
            LOG_E(LOG_TAG, "[%s] gf_hal_invoke_command.", __func__);
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT);
    }  // do gf_hal_common_authenticate_fido
    while (0);

    if (NULL != cmd)
    {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_is_id_valid
 * Description: Check finger id.
 * Input: dev, group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_is_id_valid(void *dev, uint32_t group_id,
                                     uint32_t finger_id)
{
    gf_error_t err = GF_SUCCESS;
    gf_fingerprint_finger_id_t fp_list[MAX_FINGERS_PER_USER] = { { 0 } };
    uint32_t fp_count = MAX_FINGERS_PER_USER;
    uint32_t i = 0;
    uint8_t is_id_valid = 0;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "finger_id=%u", finger_id);

    do
    {
        if (NULL == dev)
        {
            LOG_E(LOG_TAG, "[%s] dev is null", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_hal_common_enumerate(dev, fp_list, &fp_count);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] gf_hal_common_enumerate.", __func__);
            break;
        }

        for (i = 0; i < fp_count; i++)
        {
            LOG_D(LOG_TAG, "[%s] finger_id=%u, fid[%d]=%u", __func__, finger_id, i,
                  fp_list[i].fid);

            if (fp_list[i].fid == finger_id && fp_list[i].gid == group_id)
            {
                LOG_D(LOG_TAG, "[%s] finger_id is valid", __func__);
                is_id_valid = 1;
                break;
            }
        }

        err = is_id_valid == 1 ? GF_SUCCESS : GF_ERROR_INVALID_FP_ID;
    }  // do gf_hal_common_is_id_valid
    while (0);

    FUNC_EXIT(err);
    return is_id_valid;
}

/**
 * Function: gf_hal_common_get_id_list
 * Description: get finger id list by group id.
 * Input: dev, group_id, list, count
 * Output: list, count
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_get_id_list(void *dev, uint32_t group_id,
                                     uint32_t *list, int32_t *count)
{
    gf_fingerprint_finger_id_t fp_list[MAX_FINGERS_PER_USER] = { { 0 } };
    int32_t fp_count = MAX_FINGERS_PER_USER;
    gf_error_t err = GF_SUCCESS;
    int32_t i = 0;
    int32_t num = 0;
    FUNC_ENTER();

    do
    {
        if (NULL == dev || list == NULL || count == NULL)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter: dev or list or count is NULL", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_hal_common_enumerate(dev, fp_list, (uint32_t *) &fp_count);

        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] gf_hal_common_enumerate.", __func__);
            break;
        }

        if (*count < fp_count)
        {
            LOG_E(LOG_TAG, "[%s] count is smaller than fp_count, *count=%d, fp_count=%d",
                  __func__,
                  *count, fp_count);
            break;
        }

        for (i = 0; i < fp_count; i++)
        {
            if (fp_list[i].gid == group_id)
            {
                list[i] = fp_list[i].fid;
                num++;
            }
        }

        *count = num;
    }  // do gf_hal_common_get_id_list
    while (0);

    if (count != NULL && GF_SUCCESS != err)
    {
        *count = 0;
    }
    g_enroll_finger_counts = *count;
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_pause_enroll
 * Description: Pause enroll.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_pause_enroll()
{
    gf_error_t err = GF_SUCCESS;
    gf_pause_enroll_t *cmd = NULL;
    uint32_t size = sizeof(gf_pause_enroll_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();

    do
    {
        cmd = (gf_pause_enroll_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_PAUSE_ENROLL;
        cmd->pause_enroll_flag = 1;
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (err == GF_SUCCESS)
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_resume_enroll
 * Description: Resume enroll.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_resume_enroll()
{
    gf_error_t err = GF_SUCCESS;
    gf_pause_enroll_t *cmd = NULL;
    uint32_t size = sizeof(gf_pause_enroll_t);
    gf_cmd_id_t cmd_id = GF_CMD_MAX;
    FUNC_ENTER();

    do
    {
        cmd = (gf_pause_enroll_t *) GF_MEM_MALLOC(size);

        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(cmd, 0, size);
        cmd_id = GF_CMD_PAUSE_ENROLL;
        cmd->pause_enroll_flag = 0;
        err = gf_hal_invoke_command(cmd_id, cmd, size);

        if (err != GF_SUCCESS)
        {
            break;
        }
    }
    while (0);

    if (cmd != NULL)
    {
        GF_MEM_FREE(cmd);
    }

    if (err == GF_SUCCESS)
    {
        gf_hal_destroy_timer(&g_enroll_timer_id);
        gf_hal_create_timer(&g_enroll_timer_id, hal_enroll_timer_thread);
        gf_hal_set_timer(&g_enroll_timer_id, g_enroll_timer_sec, g_enroll_timer_sec, 0);
    }

    FUNC_EXIT(err);
    return err;
}

gf_screen_state_t gf_hal_common_get_screen_status()
{
    LOG_D(LOG_TAG, "[%s] gf_screen_status = %d", __func__, gf_screen_status);
    return gf_screen_status;
}

gf_error_t gf_hal_common_set_screen_status(uint32_t screen_t)
{
    if(screen_t) {
        gf_screen_status = GF_SCREEN_ON;
    } else {
        gf_screen_status = GF_SCREEN_OFF;
    }
    LOG_D(LOG_TAG, "[%s] gf_screen_status = %d", __func__, gf_screen_status);

    if (g_hal_config.support_detect_sensor_temperature)
    {
        gf_hal_post_sem_detect_broken();
    }

    return GF_SUCCESS;
}

// ----------------------------------------------------------------------------//
/**
 * Function: hal_init_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_init_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_close_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_close_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_cancel_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_cancel_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_test_cancel_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_test_cancel_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_test_prior_cancel_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_test_prior_cancel_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_pre_enroll_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static uint64_t hal_pre_enroll_stub(void *dev)
{
    UNUSED_VAR(dev);
    return 0;
}

/**
 * Function: hal_enroll_stub
 * Description: stub code.
 * Input: dev, hat, group_id, timeout_sec
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enroll_stub(void *dev, const void *hat, uint32_t group_id,
                                  uint32_t timeout_sec)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(hat);
    UNUSED_VAR(group_id);
    UNUSED_VAR(timeout_sec);
    return GF_SUCCESS;
}

/**
 * Function: hal_post_enroll_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_post_enroll_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_authenticate_stub
 * Description: stub code.
 * Input: dev, operation_id, group_id
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_authenticate_stub(void *dev, uint64_t operation_id,
                                        uint32_t group_id)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(operation_id);
    UNUSED_VAR(group_id);
    return GF_SUCCESS;
}

/**
 * Function: hal_get_auth_id_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static uint64_t hal_get_auth_id_stub(void *dev)
{
    UNUSED_VAR(dev);
    return 0;
}

/**
 * Function: hal_remove_stub
 * Description: stub code.
 * Input: dev, group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_remove_stub(void *dev, uint32_t group_id,
                                  uint32_t finger_id)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(group_id);
    UNUSED_VAR(finger_id);
    return GF_SUCCESS;
}

/**
 * Function: hal_set_active_group_stub
 * Description: stub code.
 * Input: dev, group_id
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_set_active_group_stub(void *dev, uint32_t group_id)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(group_id);
    return GF_SUCCESS;
}

/**
 * Function: hal_enumerate_stub
 * Description: stub code.
 * Input: dev, results, max_size
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enumerate_stub(void *dev, void *results,
                                     uint32_t *max_size)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(results);
    UNUSED_VAR(max_size);
    return GF_SUCCESS;
}

/**
 * Function: hal_enumerate_with_callback_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enumerate_with_callback_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_irq_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_irq_stub(void)
{
    return GF_SUCCESS;
}

/**
 * Function: hal_screen_on_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_screen_on_stub(void)
{
    return GF_SUCCESS;
}

/**
 * Function: hal_screen_off_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_screen_off_stub(void)
{
    return GF_SUCCESS;
}

/**
 * Function: hal_set_safe_class_stub
 * Description: stub code.
 * Input: dev, safe_class
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_set_safe_class_stub(void *dev,
                                          gf_safe_class_t safe_class)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(safe_class);
    return GF_SUCCESS;
}

/**
 * Function: hal_navigate_stub
 * Description: stub code.
 * Input: dev, nav_mode
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_navigate_stub(void *dev, gf_nav_mode_t nav_mode)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(nav_mode);
    return GF_SUCCESS;
}

/**
 * Function: hal_enable_fingerprint_module_stub
 * Description: stub code.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enable_fingerprint_module_stub(void *dev,
                                                     uint8_t enable_flag)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(enable_flag);
    return GF_SUCCESS;
}

/**
 * Function: hal_camera_capture_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_camera_capture_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_enable_ff_feature_stub
 * Description: stub code.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enable_ff_feature_stub(void *dev, uint8_t enable_flag)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(enable_flag);
    return GF_SUCCESS;
}

/**
 * Function: hal_enable_bio_assay_feature_stub
 * Description: stub code.
 * Input: dev, enable_flag
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_enable_bio_assay_feature_stub(void *dev,
                                                    uint8_t enable_flag)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(enable_flag);
    return GF_SUCCESS;
}

/**
 * Function: hal_start_hbd_stub
 * Description: stub code.
 * Input: dev
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_start_hbd_stub(void *dev)
{
    UNUSED_VAR(dev);
    return GF_SUCCESS;
}

/**
 * Function: hal_sync_finger_list_stub
 * Description: stub code.
 * Input: dev, list, count
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_sync_finger_list_stub(void *dev, uint32_t *list,
                                            int32_t count)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(list);
    UNUSED_VAR(count);
    return GF_SUCCESS;
}

/**
 * Function: hal_invoke_command_stub
 * Description: stub code.
 * Input: operation_id, cmd_id, buffer, len
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_invoke_command_stub(uint32_t operation_id,
                                          gf_cmd_id_t cmd_id, void *buffer,
                                          int32_t len)
{
    UNUSED_VAR(operation_id);
    UNUSED_VAR(cmd_id);
    UNUSED_VAR(buffer);
    UNUSED_VAR(len);
    return GF_SUCCESS;
}

/**
 * Function: hal_user_invoke_command_stub
 * Description: stub code.
 * Input: cmd_id, buffer, len
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_user_invoke_command_stub(uint32_t cmd_id, void *buffer,
                                               int32_t len)
{
    UNUSED_VAR(cmd_id);
    UNUSED_VAR(buffer);
    UNUSED_VAR(len);
    return GF_SUCCESS;
}

/**
 * Function: hal_dump_invoke_command_stub
 * Description: stub code.
 * Input: cmd_id, buffer, len
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_dump_invoke_command_stub(uint32_t cmd_id, void *buffer,
                                               int32_t len)
{
    UNUSED_VAR(cmd_id);
    UNUSED_VAR(buffer);
    UNUSED_VAR(len);
    return GF_SUCCESS;
}

/**
 * Function: hal_authenticate_fido_stub
 * Description: stub code.
 * Input: dev, group_id, aaid, aaid_len, challenge, challenge_len
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_authenticate_fido_stub(void *dev, uint32_t group_id,
                                             uint8_t *aaid,
                                             uint32_t aaid_len, uint8_t *challenge, uint32_t challenge_len)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(group_id);
    UNUSED_VAR(aaid);
    UNUSED_VAR(aaid_len);
    UNUSED_VAR(challenge);
    UNUSED_VAR(challenge_len);
    return GF_SUCCESS;
}

/**
 * Function: hal_is_id_valid_stub
 * Description: stub code.
 * Input: dev, group_id, finger_id
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_is_id_valid_stub(void *dev, uint32_t group_id,
                                       uint32_t finger_id)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(group_id);
    UNUSED_VAR(finger_id);
    return GF_SUCCESS;
}

/**
 * Function: hal_get_id_list_stub
 * Description: stub code.
 * Input: dev, group_id, list, count
 * Output: None
 * Return: gf_error_t
 */
static int32_t hal_get_id_list_stub(void *dev, uint32_t group_id,
                                    uint32_t *list, int32_t *count)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(group_id);
    UNUSED_VAR(list);
    UNUSED_VAR(count);
    return 0;
}

/**
 * Function: hal_reset_lockout_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_reset_lockout_stub()
{
    return GF_SUCCESS;
}

/**
 * Function: hal_lockout_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_lockout_stub()
{
    return GF_SUCCESS;
}

/**
 * Function: hal_pause_enroll_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_pause_enroll_stub()
{
    return GF_SUCCESS;
}

/**
 * Function: hal_resume_enroll_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_resume_enroll_stub()
{
    return GF_SUCCESS;
}

/**
 * Function: dump_chip_init_data_stub
 * Description: stub code.
 * Input: None
 * Output: None
 * Return: bool
 */
bool dump_chip_init_data_stub()
{
    // return true if chip dump finished, no need common dump, false continue common dump
    return false;
}

/**
 * Function: dump_chip_operation_data_stub
 * Description: stub code.
 * Input: dump_data, operation, tv, error_code, chip_type
 * Output: None
 * Return: bool
 */
bool dump_chip_operation_data_stub(gf_dump_data_t *dump_data,
                                       gf_operation_type_t operation,
                                       struct timeval* tv,
                                       gf_error_t error_code,
                                       gf_chip_type_t chip_type)
{
    // return true if chip dump finished, no need common dump, false continue common dump
    UNUSED_VAR(dump_data);
    UNUSED_VAR(operation);
    UNUSED_VAR(tv);
    UNUSED_VAR(error_code);
    UNUSED_VAR(chip_type);
    return false;
}


// ----------------------------------------------------------------------------//

/**
 * Function: gf_hal_function_init
 * Description: Initialize function point.
 * Input: hal_function, chip_series
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_function_init(gf_hal_function_t *hal_function,
                                gf_chip_series_t chip_series)
{
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == hal_function)
        {
            err = GF_ERROR_BAD_PARAMS;
            LOG_E(LOG_TAG "[%s] invalid param", __func__);
            break;
        }

        hal_function->init = hal_init_stub;
        hal_function->close = hal_close_stub;
        hal_function->cancel = hal_cancel_stub;
        hal_function->test_cancel = hal_test_cancel_stub;
        hal_function->test_prior_cancel = hal_test_prior_cancel_stub;
        hal_function->pre_enroll = hal_pre_enroll_stub;
        hal_function->enroll = hal_enroll_stub;
        hal_function->post_enroll = hal_post_enroll_stub;
        hal_function->authenticate = hal_authenticate_stub;
        hal_function->get_auth_id = hal_get_auth_id_stub;
        hal_function->remove = hal_remove_stub;
        hal_function->set_active_group = hal_set_active_group_stub;
        hal_function->enumerate = hal_enumerate_stub;
        hal_function->enumerate_with_callback = hal_enumerate_with_callback_stub;
        hal_function->irq = hal_irq_stub;
        hal_function->screen_on = hal_screen_on_stub;
        hal_function->screen_off = hal_screen_off_stub;
        hal_function->set_safe_class = hal_set_safe_class_stub;
        hal_function->navigate = hal_navigate_stub;
        hal_function->enable_fingerprint_module = hal_enable_fingerprint_module_stub;
        hal_function->camera_capture = hal_camera_capture_stub;
        hal_function->enable_ff_feature = hal_enable_ff_feature_stub;
        hal_function->enable_bio_assay_feature = hal_enable_bio_assay_feature_stub;
        hal_function->start_hbd = hal_start_hbd_stub;
        hal_function->reset_lockout = hal_reset_lockout_stub;
        hal_function->sync_finger_list = hal_sync_finger_list_stub;
        hal_function->invoke_command = hal_invoke_command_stub;
        hal_function->user_invoke_command = hal_user_invoke_command_stub;
        hal_function->dump_invoke_command = hal_dump_invoke_command_stub;
        hal_function->authenticate_fido = hal_authenticate_fido_stub;
        hal_function->is_id_valid = hal_is_id_valid_stub;
        hal_function->get_id_list = hal_get_id_list_stub;
        hal_function->lockout = hal_lockout_stub;
        hal_function->pause_enroll = hal_pause_enroll_stub;
        hal_function->resume_enroll = hal_resume_enroll_stub;
        hal_function->dump_chip_init_data = dump_chip_init_data_stub;
        hal_function->dump_chip_operation_data = dump_chip_operation_data_stub;
        hal_function->set_finger_screen = gf_hal_common_set_screen_status;

        /* customize hal functions */
        switch (chip_series)
        {
            case GF_OSWEGO_M:
            {
                err = gf_hal_oswego_m_function_customize(hal_function);
                break;
            }

            case GF_MILAN_HV:
            case GF_MILAN_F_SERIES:
            case GF_DUBAI_A_SERIES:
            {
                err = gf_hal_milan_function_customize(hal_function);
                break;
            }

            case GF_MILAN_A_SERIES:
            {
                err = gf_hal_milan_a_series_function_customize(hal_function);
                break;
            }

            case GF_MILAN_AN_SERIES:
            {
                err = gf_hal_milan_an_series_function_customize(hal_function);
                break;
            }

#ifdef __SIMULATOR
            case GF_SIMULATOR:
            {
                err = gf_hal_simulator_function_customize(hal_function);
                break;
            }
#endif  // __SIMULATOR
            default:
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
        }  // switch chip_series
    }  // do gf_hal_function_init
    while (0);

    return err;
}

/**
 * Function: gf_hal_common_load_otp_info_from_sdcard
 * Description: Load otp data from sdcard.
 * Input: otp_buf, otp_len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_load_otp_info_from_sdcard(uint8_t *otp_buf,
                                                   uint32_t *otp_len)
{
    gf_error_t err = GF_SUCCESS;
    FILE *file = NULL;
    uint8_t *read_buf = NULL;
    FUNC_ENTER();

    do
    {
        uint8_t calculate_crc8 = 0;
        uint8_t backup_crc8 = 0;
        int32_t file_len = 0;
        uint32_t actual_otp_len = 0;
        // milan bn otp len is GF_SENSOR_OTP_BUFFER_LEN, check sum is 1 byte.
        uint32_t read_buf_len = GF_SENSOR_OTP_BUFFER_LEN + 1;

        if (NULL == otp_buf || NULL == otp_len)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        read_buf = (uint8_t *) malloc(read_buf_len);
        if (NULL == read_buf)
        {
            LOG_E(LOG_TAG, "[%s] out of memory when malloc read_buf", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }

        memset(read_buf, 0, read_buf_len);

        file = fopen(GF_OTP_INFO_FILEPATH, "rb");

        if (NULL == file)
        {
            LOG_E(LOG_TAG, "[%s] OTP file(%s) is not exist", __func__,
                  GF_OTP_INFO_FILEPATH);
            err = GF_ERROR_FILE_NOT_EXIST;
            break;
        }

        LOG_D(LOG_TAG, "[%s] open file success", __func__);

        file_len = fread(read_buf, sizeof(uint8_t), read_buf_len, file);
        if (file_len <= 0)
        {
            LOG_E(LOG_TAG, "[%s] read OTP info fail", __func__);
            err = GF_ERROR_FILE_READ_FAILED;
            break;
        }

        // the otp file save the otp data and crc(1 byte), so need -1
        actual_otp_len = file_len -1;
        LOG_D(LOG_TAG, "[%s] read data success", __func__);

        memcpy(otp_buf, read_buf, GF_SENSOR_OTP_BUFFER_LEN);

        backup_crc8 = read_buf[file_len - 1];

        calculate_crc8 = hal_common_otp_crc8(otp_buf, actual_otp_len);

        if (calculate_crc8 != backup_crc8)
        {
            LOG_E(LOG_TAG,
                  "[%s] CRC8 check fail, backup_crc8=0x%02X, calcualte_crc8=0x%02X",
                  __func__, backup_crc8, calculate_crc8);
            err = GF_ERROR_HAL_GENERAL_ERROR;
            break;
        }

        LOG_I(LOG_TAG, "[%s] CRC8 check success", __func__);
        *otp_len = actual_otp_len;
    }  // do gf_hal_common_load_otp_info_from_sdcard
    while (0);

    if (NULL != read_buf)
    {
        free(read_buf);
    }

    if (NULL != file)
    {
        fclose(file);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_save_otp_info_into_sdcard
 * Description: Save otp data to sdcard.
 * Input: otp_buf, otp_len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_save_otp_info_into_sdcard(uint8_t *otp_buf,
                                                   uint32_t otp_len)
{
    gf_error_t err = GF_SUCCESS;
    FILE *file = NULL;
    FUNC_ENTER();

    do
    {
        uint8_t calculate_crc8 = 0;

        if (NULL == otp_buf)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        file = fopen(GF_OTP_INFO_FILEPATH, "wb");

        if (NULL == file)
        {
            LOG_E(LOG_TAG, "[%s] save OTP file(%s) is not exist", __func__,
                  GF_OTP_INFO_FILEPATH);
            err = GF_ERROR_FILE_NOT_EXIST;
            break;
        }

        LOG_D(LOG_TAG, "[%s] open file(%s) success", __func__, GF_OTP_INFO_FILEPATH);

        if (otp_len != fwrite(otp_buf, sizeof(uint8_t),
                                             otp_len, file))
        {
            LOG_E(LOG_TAG, "[%s] write OTP INFO fail", __func__);
            err = GF_ERROR_FILE_WRITE_FAILED;
            break;
        }

        calculate_crc8 = hal_common_otp_crc8(otp_buf, otp_len);

        if (1 != fwrite(&calculate_crc8, sizeof(uint8_t), 1, file))
        {
            LOG_E(LOG_TAG, "[%s] write CRC8 for OTP INFO fail", __func__);
            err = GF_ERROR_FILE_WRITE_FAILED;
            break;
        }

        LOG_D(LOG_TAG, "[%s] save backup OTP info success", __func__);
    }  // do gf_hal_common_save_otp_info_into_sdcard
    while (0);

    if (NULL != file)
    {
        fclose(file);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_common_enroll_success
 * Description: Send GF_FINGERPRINT_ACQUIRED_GOOD message when enroll success.
 * Input: cmd, err_code
 * Output: None
 * Return: gf_error_t
 */
void gf_hal_common_enroll_success(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd || NULL == err_code)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u, samples_remaining=%u", __func__,
              cmd->group_id, cmd->finger_id, cmd->samples_remaining);
        if (0 == g_hal_config.support_swipe_enroll)
        {
            gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);
        }

        if (0 == cmd->samples_remaining)
        {
            gf_hal_destroy_timer(&g_enroll_timer_id);
            gf_hal_destroy_timer(&g_long_pressed_timer_id);
            *err_code = gf_hal_save(cmd->group_id, cmd->finger_id);
            if (GF_SUCCESS != *err_code)
            {
                gf_hal_notify_error_info(GF_FINGERPRINT_ERROR_NO_SPACE);
            }
            else
            {
                if (g_work_state != STATE_ENROLL)
                {
                    LOG_D(LOG_TAG, "[%s] canceled enroll", __func__);
                    gf_hal_common_remove_without_callback(cmd->group_id, cmd->finger_id);
                }
                else
                {
                    gf_hal_notify_enrollment_progress(cmd->group_id, cmd->finger_id,
                                                      cmd->samples_remaining);
                    if (g_fingerprint_device != NULL) {
                        do_enumerate(g_fingerprint_device, cmd->group_id, cmd->finger_id);
                    }
                }
            }
        }
        else
        {
            gf_hal_notify_enrollment_progress(cmd->group_id, cmd->finger_id,
                                              cmd->samples_remaining);
            gf_hal_set_timer(&g_enroll_timer_id, g_enroll_timer_sec, g_enroll_timer_sec, 0);
        }
    }  // do gf_hal_common_enroll_success
    while (0);

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_common_authenticate_success
 * Description: Send GF_FINGERPRINT_ACQUIRED_GOOD message when authentication success.
 * Input: cmd, err_code
 * Output: None
 * Return: gf_error_t
 */
void gf_hal_common_authenticate_success(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd || NULL == err_code)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);

        gf_hal_notify_authentication_succeeded(cmd->group_id, cmd->finger_id,
                                               &cmd->auth_token);
        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
              cmd->finger_id);
        LOG_D(LOG_TAG, "[%s] save_flag=%u, update_stitch_flag=%u", __func__,
              cmd->save_flag,
              cmd->update_stitch_flag);
        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(*err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }

        g_failed_attempts = 0;

        if (cmd->save_flag > 0)
        {
            *err_code = gf_hal_save(cmd->group_id, cmd->finger_id);
        }

        if (cmd->update_stitch_flag > 0)
        {
            gf_hal_update_stitch(cmd->group_id, cmd->finger_id);
        }
    }  // do gf_hal_common_authenticate_success
    while (0);

    VOID_FUNC_EXIT();
}


/**
 * Function: gf_hal_common_authenticate_fido_success
 * Description: Send GF_FINGERPRINT_ACQUIRED_GOOD message when authentication fido success.
 * Input: cmd, err_code
 * Output: None
 * Return: gf_error_t
 */
void gf_hal_common_authenticate_fido_success(gf_irq_t *cmd,
                                             gf_error_t *err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd || NULL == err_code)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);

        if (g_fingerprint_device != NULL && g_fingerprint_device->test_notify != NULL)
        {
            gf_fingerprint_msg_t message = { 0 };
            message.type = GF_FINGERPRINT_AUTHENTICATED_FIDO;
            message.data.authenticated_fido.finger_id = cmd->finger_id;
            message.data.authenticated_fido.uvt_len = cmd->uvt.uvt_len;
            memcpy(&message.data.authenticated_fido.uvt_data, &cmd->uvt.uvt_buf,
                   sizeof(message.data.authenticated_fido.uvt_data));
            //g_fingerprint_device->test_notify((fingerprint_msg_t *)&message);
        }

        LOG_D(LOG_TAG, "[%s] group_id=%u, finger_id=%u", __func__, cmd->group_id,
              cmd->finger_id);
        LOG_D(LOG_TAG, "[%s] save_flag=%u, update_stitch_flag=%u", __func__,
              cmd->save_flag,
              cmd->update_stitch_flag);
        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(*err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }

        g_failed_attempts = 0;

        if (cmd->save_flag > 0)
        {
            *err_code = gf_hal_save(cmd->group_id, cmd->finger_id);
        }

        if (cmd->update_stitch_flag > 0)
        {
            gf_hal_update_stitch(cmd->group_id, cmd->finger_id);
        }
    }  // do gf_hal_common_authenticate_fido_success
    while (0);

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_common_authenticate_not_match
 * Description: Send message when authentication failed.
 * Input: cmd, err_code
 * Output: None
 * Return: gf_error_t
 */
void gf_hal_common_authenticate_not_match(gf_irq_t *cmd, gf_error_t err_code)
{
    VOID_FUNC_ENTER()

    fingerprint_auth_dcsmsg_t auth_context;
    memset(&auth_context, 0, sizeof(auth_context));
    auth_context.fail_reason = (uint32_t)err_code;

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] cmd is null", __func__);
            break;
        }

        if (cmd->too_fast_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] press too fast", __func__);
            if (OPERATION_TEST_UNTRUSTED_ENROLL == cmd->operation
                || OPERATION_TEST_UNTRUSTED_AUTHENTICATE == cmd->operation) {
                gf_hal_notify_test_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
            } else {
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOO_FAST);
            }
            auth_context.fail_reason = GF_FINGERPRINT_ACQUIRED_TOO_FAST;
        } else if (cmd->mistake_touch_flag > 0)
        {
            LOG_I(LOG_TAG, "[%s] touch by mistake : %d", __func__, cmd->mistake_touch_flag);
            if (2 == cmd->mistake_touch_flag) {
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_PARTIAL);
                auth_context.fail_reason = GF_FINGERPRINT_ACQUIRED_PARTIAL;
                gf_hal_notify_authentication_failed();
            } else if (1 == cmd->mistake_touch_flag) {
                if (GF_SCREEN_ON == gf_hal_common_get_screen_status()) {
                    gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                    gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                    gf_hal_notify_image_info(FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                    auth_context.fail_reason = FINGERPRINT_ACQUIRED_IMAGER_DIRTY;
                    gf_hal_notify_authentication_failed();
                } else {
                    gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                    gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                    gf_hal_notify_image_info(FINGERPRINT_ACQUIRED_INSUFFICIENT);
                    auth_context.fail_reason = FINGERPRINT_ACQUIRED_INSUFFICIENT;
                }
            }
            //gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_TOUCH_BY_MISTAKE);
        } else
        {
            if (OPERATION_AUTHENTICATE_FF == cmd->operation
                || OPERATION_AUTHENTICATE_IMAGE == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] no finger match", __func__);
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_acquired_info(GF_FINGERPRINT_ACQUIRED_GOOD);
                gf_hal_notify_authentication_failed();
            }
            else if (OPERATION_AUTHENTICATE_FIDO == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] authenticator fido, no finger match", __func__);
                gf_hal_notify_authentication_fido_failed();
            } else if (OPERATION_TEST_UNTRUSTED_AUTHENTICATE == cmd->operation)
            {
                LOG_D(LOG_TAG, "[%s] no finger match", __func__);
                gf_hal_notify_test_authentication_failed();
            }
        }

        gf_hal_dump_performance(__func__, cmd->operation, &cmd->dump_performance);

        auth_context.auth_result = 0;//0 means failed
        //auth_context.fail_reason = cmd->o_dismatch_reason;
        auth_context.quality_score = cmd->dump_performance.image_quality;
        auth_context.match_score = cmd->dump_performance.match_score;
        //auth_context.signal_value = cmd->o_sig_val;
        auth_context.img_area = cmd->dump_performance.valid_area;
        auth_context.retry_times = g_auth_retry_times;//context->retry;
        //auth_context.module_type = mContext->mSensor->mModuleType;
        //auth_context.lense_type = mContext->mSensor->mLenseType;
        //auth_context.dsp_availalbe = mDSPAvailable;
        gf_hal_notify_send_auth_dcsmsg(auth_context);

        if (g_hal_config.support_authenticate_ratio > 0)
        {
            gf_hal_save_auth_ratio_record(cmd);
        }

        if (g_hal_config.support_frr_analysis > 0)
        {
            gf_hal_handle_frr_database(err_code, cmd->dump_performance.image_quality,
                                       cmd->dump_performance.valid_area);
        }
    }  // do gf_hal_common_authenticate_not_match
    while (0);

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_common_irq_key
 * Description: Handle key interrupt .
 * Input: cmd
 * Output: None
 * Return: void
 */
void gf_hal_common_irq_key(gf_irq_t *cmd)
{
    gf_key_code_t key_code = GF_KEY_NONE;
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK))
        {
            if (OPERATION_HOME_KEY == cmd->operation)
            {
                key_code = GF_KEY_HOME;
            }
            else if (OPERATION_CAMERA_KEY == cmd->operation)
            {
                key_code = GF_KEY_CAMERA;
            }
            else if (OPERATION_POWER_KEY == cmd->operation)
            {
                key_code = GF_KEY_POWER;
            }

            if ((key_code != GF_KEY_NONE))
            {
                gf_hal_send_key_event(key_code, GF_KEY_STATUS_DOWN);
                g_key_down_flag = 1;
                g_long_press_keycode = key_code;
            }
        }  // cmd->irq_type & GF_IRQ_FINGER_DOWN_MASK

        if (0 != (cmd->irq_type & GF_IRQ_FINGER_UP_MASK))
        {
            if (g_key_down_flag > 0)
            {
                if (OPERATION_HOME_KEY == cmd->operation)
                {
                    gf_hal_send_key_event(GF_KEY_HOME, GF_KEY_STATUS_UP);
                    g_key_down_flag = 0;
                }
                else if (OPERATION_CAMERA_KEY == cmd->operation)
                {
                    gf_hal_send_key_event(GF_KEY_CAMERA, GF_KEY_STATUS_UP);
                    g_key_down_flag = 0;
                }
                else if (OPERATION_POWER_KEY == cmd->operation)
                {
                    gf_hal_send_key_event(GF_KEY_POWER, GF_KEY_STATUS_UP);
                    g_key_down_flag = 0;
                }
            }  // g_key_down_flag > 0
        }  // cmd->irq_type & GF_IRQ_FINGER_UP_MASK
    }  // do gf_hal_common_irq_key
    while (0);

    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_common_irq_finger_long_press
 * Description: Handle long press key.
 * Input: cmd
 * Output: None
 * Return: void
 */
void gf_hal_common_irq_finger_long_press(gf_irq_t *cmd, gf_error_t err_code)
{
    VOID_FUNC_ENTER();

    do
    {
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }

        if (0 != (cmd->irq_type & GF_IRQ_IMAGE_MASK))
        {
            if ((GF_ERROR_INVALID_FINGER_PRESS == err_code
                 || GF_ERROR_INVALID_BASE == err_code) && (g_key_down_flag > 0))
            {
                gf_hal_send_key_event(g_long_press_keycode, GF_KEY_STATUS_UP);
                g_key_down_flag = 0;
            }
        }
    }
    while (0);

    VOID_FUNC_EXIT();
}

int do_enumerate(void *dev, uint32_t group_id, uint32_t finger_id)
{
    UNUSED_VAR(dev);
    UNUSED_VAR(finger_id);
    fingerprint_msg_t message;
    if (g_fingerprint_device != NULL && g_fingerprint_device->notify != NULL) {
        uint32_t err = 0;
        int32_t indices_count = MAX_FINGERS_PER_USER;
        uint32_t indices[MAX_FINGERS_PER_USER] = {0};
        memset(&message, 0 , sizeof(fingerprint_msg_t));
        err = gf_hal_common_get_id_list(g_fingerprint_device, group_id, indices, &indices_count);
        if (err) {
            LOG_E(LOG_TAG, "[%s] gf_ha_common_get_id_list error~~~", __func__);
            return -1;
        }
        LOG_I(LOG_TAG, "[%s] ids count = %d", __func__, indices_count);

        message.data.enumerated.remaining_templates = indices_count;
        message.data.enumerated.gid = group_id;
        memset(message.data.enumerated.fingers, 0, MAX_ID_LIST_SIZE * sizeof(fingerprint_finger_id_t));
        for (uint32_t i = 0; i < indices_count; i++) {
            LOG_I(LOG_TAG, "[%s] ids[%d] = %u", __func__, i, indices[i]);
            message.data.enumerated.fingers[i].fid = indices[i];
            message.data.enumerated.fingers[i].gid = group_id;
        }
        message.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        g_fingerprint_device->notify(&message);
    }
    return 0;
}

/**
 * Function: gf_hal_common_get_qr_code
 * Description: Get QR code from otp
 * Input: qr_buf, buf_len
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_common_get_qr_code(uint8_t *qr_buf, uint8_t buf_len)
{
    gf_error_t err = GF_SUCCESS;
    gf_qr_code_t *cmd = NULL;
    uint32_t len =0;
    FUNC_ENTER();

    do {
        cmd = (gf_qr_code_t *) GF_MEM_MALLOC(sizeof(gf_qr_code_t));

        if (NULL == cmd) {
            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(cmd, 0, sizeof(gf_qr_code_t));
        err = gf_hal_invoke_command(GF_CMD_GET_QR_CODE, cmd, sizeof(gf_qr_code_t));
        if (err == GF_SUCCESS) {
            len = (buf_len > sizeof(cmd->qr_code)) ? sizeof(cmd->qr_code) : buf_len;
        LOG_E(LOG_TAG, "[%s] qr_code %s sizeof(cmd->qr_code) = %d, len = %d", __func__, cmd->qr_code, sizeof(cmd->qr_code), len);
            memcpy(qr_buf, cmd->qr_code, len);
        LOG_E(LOG_TAG, "[%s] qr_buf = %s, len = %d", __func__, qr_buf, len);
        }
    } while (0);

    if (cmd != NULL) {
        GF_MEM_FREE(cmd);
    }

    FUNC_EXIT(err);
    return err;
}
