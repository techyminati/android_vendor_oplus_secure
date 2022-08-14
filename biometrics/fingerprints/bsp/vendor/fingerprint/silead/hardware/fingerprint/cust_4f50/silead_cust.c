/******************************************************************************
 * @file   silead_cust.c
 * @brief  Contains fingerprint operate functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * David Wang  2018/7/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_cust"
#include "log/logmsg.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

#include "silead_error.h"
#include "silead_cust.h"
#include "silead_screen.h"
#include "silead_impl.h"
#include "silead_worker.h"
#include "silead_ext_cb.h"
#include "silead_const.h"

#include "fingerprint_4f50.h"
#include "silead_finger_4f50.h"
#include "silead_cmd.h"

extern uint32_t m_gid;
extern int32_t silfp_common_cancel();
extern int32_t silfp_enroll_pause(int32_t pause);
extern fingerprint_notify_t silfp_notify_get_notify_callback();
extern int32_t silfp_common_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size);

static void silfp_send_engineering_image_quality_notice(int32_t successed, int32_t quality);
static void silfp_send_enumerated_notice();
static void silfp_send_touch_down_notice();
static void silfp_send_touch_up_notice();
static void silfp_send_hardware_notice();
static void silfp_bind_bigcore_bytid();
#ifdef FP_HYPNUSD_ENABLE
static void silfp_send_hypnus_notice(int32_t action_type, int32_t action_timeout);
#endif

static int32_t cust_chip_id = 0;
static char cust_algo_version[32];

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

typedef struct fp_ta_otp_bit_4f50 {
    uint32_t pn:24;
    uint32_t vendor_id:8;
    /* */
    uint32_t sn1:6;
    uint32_t track_id:6;
    uint32_t status:4;
    uint32_t day:6;
    uint32_t month:4;
    uint32_t year:6;
    /* */
    uint32_t reserved8:8;
    uint32_t sn5:6;
    uint32_t sn4:6;
    uint32_t sn3:6;
    uint32_t sn2:6;
} fp_ta_otp_bit_4f50_t;

typedef union fp_ta_otp_4f50 {
    fp_ta_otp_bit_4f50_t info;
    uint32_t raw[3];
} fp_ta_otp_4f50_t;

typedef struct fp_ta_otp_map_4f50 {
    char    idx;
    char    *str;
} fp_ta_otp_map_4f50_t;

typedef struct fp_ta_otp_map_common {
    char *str;
} fp_ta_otp_map_common_t;

static const fp_ta_otp_map_4f50_t s_otp_vendor[] = {
    {'F', "OFILM\0"},
    {'Q', "QTECH\0"},
    {'G', "G\0"},
    {'H', "HOLITECH\0"},
    {'S', "SABIO\0"},
    {'T', "TRULY\0"},
    {'K', "KIRE\0"},
    {'Z', "FINGERCHIP\0"},
    {'?', "Unknown\0"},
};

static const fp_ta_otp_map_common_t s_otp_pn_status[] = {
    {"0"}, {"X"}, {"G"}, {"P"}
};

static const fp_ta_otp_map_common_t s_otp_day[] = {
    {"0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"},
    {"A"}, {"B"}, {"C"}, {"D"}, {"E"}, {"F"}, {"G"}, {"H"}, {"J"}, {"K"}, 
    {"L"}, {"M"}, {"N"}, {"P"}, {"Q"}, {"R"}, {"S"}, {"T"}, {"U"}, {"V"}, 
    {"W"}, {"X"}, {"0"}
};

static const fp_ta_otp_map_common_t s_otp_common[] = {
    {"0"}, {"1"}, {"2"}, {"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"},
    {"A"}, {"B"}, {"C"}, {"D"}, {"E"}, {"F"}, {"G"}, {"H"}, {"I"}, {"J"},
    {"K"}, {"L"}, {"M"}, {"N"}, {"O"}, {"P"}, {"Q"}, {"R"}, {"S"}, {"T"},
    {"U"}, {"V"}, {"W"}, {"X"}, {"Y"}, {"Z"}, {"0"}
};

#define OTP_ARRAY_TO_STR(_array, _idx, _buf, _len) do { \
        LOG_MSG_VERBOSE("_idx = %d", _idx); \
        uint8_t _num = ARRAY_SIZE(_array) - 1; \
        char *_pstr = (_array)[_num].str; \
        if ((_idx >= 0) && (_idx < _num)) { \
            _pstr = (_array)[_idx].str; \
        } \
        memcpy(_buf, _pstr, _len); \
        _buf += _len; \
    } while(0)

/*const char *m_default_path = "/data/vendor/silead";
const char* silfp_cust_get_dump_path(void)
{
    return m_default_path;
}

const char* silfp_cust_get_cal_path(void)
{
    return m_default_path;
}

const char* silfp_cust_get_ta_name(void)
{
    return NULL;
}*/

int32_t silfp_cust_is_capture_disable(void)
{
    return 0;
}

int32_t silfp_cust_need_cancel_notice(void)
{
    return 1;
}

int32_t silfp_cust_is_screen_from_drv(void)
{
    return 0;
}

int32_t silfp_cust_get_finger_status_mode(void)
{
    return SIFP_FINGER_STATUS_IRQ;
}

void silfp_cust_finger_down_after_action(void)
{
    silfp_send_touch_down_notice(); // notice finger down
    silfp_bind_bigcore_bytid();
#ifdef FP_HYPNUSD_ENABLE
    silfp_send_hypnus_notice(ACTION_TYPE, ACTION_TIMEOUT_500);
#endif
    silfp_send_hardware_notice();   // notice capture image start
}

void silfp_cust_finger_up_after_action(void)
{
    silfp_send_touch_up_notice(); // notice finger up
}

int32_t silfp_cust_get_sys_charging_state(void)
{
    LOG_MSG_VERBOSE("cust get charging state");
    char *ac_online = "/sys/class/power_supply/ac/online";
    char *usb_online = "/sys/class/power_supply/usb/online";
    char value[2] = {'\0', '\0'};
    int ac_fd = -1;
    int usb_fd = -1;
    ac_fd = open(ac_online, O_RDONLY);
    if (ac_fd < 0) {
        LOG_MSG_ERROR("open the file node %s failed(%s)", ac_online, strerror(errno));
        return 0;
    }
    read(ac_fd, value, 1);
    close(ac_fd);
    if (!strcmp(value, "1")) {
        LOG_MSG_DEBUG("ac_online value = %s, is charging\n", value);
        return 1;
    } else {
        usb_fd = open(usb_online, O_RDONLY);
        if (usb_fd < 0) {
            LOG_MSG_ERROR("open the file node %s failed(%s)", usb_online, strerror(errno));
            return 0;
        }
        read(usb_fd, value, 1);
        close(usb_fd);
        if (!strcmp(value, "1")) {
            LOG_MSG_DEBUG("usb_online value = %s, is charging\n", value);
            return 1;
        }
    }
    LOG_MSG_DEBUG("value = %s, so, have no charing", value);
    return 0;
}

int32_t silfp_cust_tpl_change_action(void)
{
    silfp_send_enumerated_notice(); // notice all tpl
    return 0;
}

int32_t silfp_cust_auth_get_retry_times(void)
{
    return 2; // auth retry times
}

int32_t silfp_cust_auth_mistouch_ignor_screen_off(void)
{
    return 1; // mistouch when screen off, ignor
}

int32_t silfp_cust_enroll_timeout_sec(void)
{
    return 600; // 10 minutes, enroll timeout
}

int32_t silfp_cust_enroll_cont_err_times(void)
{
    return 40; // cont error times
}

int32_t silfp_cust_enroll_report_remain_when_error(void)
{
    return 1; // need report remain when error
}

int32_t silfp_cust_trans_notice_code(int32_t code)
{
    switch (code) {
    case -SL_ERROR_SAME_AREA:
        return FINGERPRINT_ACQUIRED_TOO_SIMILAR;
    case -SL_ERROR_EROLL_DUPLICATE:
        return FINGERPRINT_ACQUIRED_ALREADY_ENROLLED;
    default:
        if (code < 0) {
            code = -code;
        }
    }
    return code;
}

int32_t silfp_cust_send_quality_notice(int32_t __unused result, int32_t __unused quality)
{
    silfp_send_engineering_image_quality_notice(result, quality);
    return 0;
}

int32_t silfp_cust_is_optic(void)
{
    return 0;
}

// add more extend HAL interface
static void silfp_send_engineering_image_quality_notice(int32_t result, int32_t quality)
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_ENGINEERING_INFO;
    msg.data.engineering.type=FINGERPRINT_IMAGE_QUALITY;
    msg.data.engineering.quality.successed = (result == 0) ? 1 : 0;
    msg.data.engineering.quality.image_quality = quality;
    msg.data.engineering.quality.image_quality_pass = (quality >= 10) ? 1 : 0;
    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}

void silfp_cust_get_qua_cov(uint32_t *quality, uint32_t *coverage)
{
    int32_t ret = 0;
    uint32_t data[2] = {0};
    uint32_t len = sizeof(data);
    uint32_t result = 0;

    ret = silfp_cmd_send_cmd_with_buf_and_get(REQ_CMD_SUB_ID_GET_AUTH_INFO, data, &len, &result);
    if (ret >= 0) {
        *quality = data[0];
        *coverage = data[1];
    }
}

void silfp_cust_get_chip_id(int32_t chip_id)
{
    char str[16] = {0};
    //chip_id such as 6157a001 or 6159a083
    snprintf(str, sizeof(str), "%x", chip_id >> 16);
    cust_chip_id = atoi(str);
}

static void itoa (uint32_t n,char s[])
{
    int i,j,sign;
    char c = 0;
    if((sign = n) < 0)//��¼����
        n = -n;//ʹn��Ϊ����
    i = 0;
    do {
        s[i++] = n % 10 + '0';//ȡ��һ������
    } while ((n /= 10) > 0); //ɾ��������
    if(sign < 0)
        s[i++] = '-';
    s[i] = '\0';

    j = i - 1;
    for(i = 0; i<j; i++,j--) { //���ɵ�����������ģ�����Ҫ�������
        c = s[j];
        s[j] = s[i];
        s[i] = c;
    }
}

void silfp_cust_get_algo_version(int32_t algo_version, char *algo_version_char)
{
    snprintf(cust_algo_version, sizeof(cust_algo_version), "v%d.%d.%d", algo_version/100000, (algo_version/1000)%100, algo_version%1000);
    memcpy(algo_version_char, cust_algo_version, sizeof(cust_algo_version));
}

void silfp_notify_send_dcsmsg(int32_t auth_result, int32_t fail_reason, int32_t retry_times, silfp_broken_info* binfo)
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;
    uint32_t quality = 0;
    uint32_t coverage = 0;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_AUTHENTICATED_DCSSTATUS;
    //silfp_cmd_test_image_capture(0x04, NULL, NULL, &quality, NULL, NULL, NULL, NULL);

    memcpy(msg.data.auth_dcsmsg.algo_version,  cust_algo_version,  sizeof(cust_algo_version));
    msg.data.auth_dcsmsg.auth_result = auth_result;
    msg.data.auth_dcsmsg.chip_ic = cust_chip_id;
    msg.data.auth_dcsmsg.retry_times = retry_times;
    msg.data.auth_dcsmsg.fail_reason = fail_reason;
    LOG_MSG_DEBUG("authenticate, auth_result = %d", msg.data.auth_dcsmsg.auth_result);
    LOG_MSG_DEBUG("authenticate, fail_reason = %d", msg.data.auth_dcsmsg.fail_reason);
    LOG_MSG_DEBUG("authenticate, retry_times = %d", msg.data.auth_dcsmsg.retry_times);
    LOG_MSG_DEBUG("authenticate, algo_version = %s", msg.data.auth_dcsmsg.algo_version);
    LOG_MSG_DEBUG("authenticate, chip_ic = %d", msg.data.auth_dcsmsg.chip_ic);

    if (auth_result != SILFP_CHECK_RESULT) {
        silfp_cust_get_qua_cov(&quality, &coverage);
        msg.data.auth_dcsmsg.img_area = coverage;
        msg.data.auth_dcsmsg.quality_score = quality;
        LOG_MSG_DEBUG("authenticate, quality_score = %d", msg.data.auth_dcsmsg.quality_score);
        LOG_MSG_DEBUG("authenticate, img_area = %d", msg.data.auth_dcsmsg.img_area);
    } else if (binfo != NULL) {
        msg.data.auth_dcsmsg.signal_value = binfo->grey1;
        msg.data.auth_dcsmsg.match_score = binfo->grey2;
        msg.data.auth_dcsmsg.module_type = binfo->grey_diff;
        msg.data.auth_dcsmsg.lense_type = binfo->deadpx;
        msg.data.auth_dcsmsg.dsp_availalbe = binfo->badline;
        LOG_MSG_DEBUG("authenticate check_broken, grey1 = %d", binfo->grey1);
        LOG_MSG_DEBUG("authenticate check_broken, grey2 = %d", binfo->grey2);
        LOG_MSG_DEBUG("authenticate check_broken, grey_diff = %d", binfo->grey_diff);
        LOG_MSG_DEBUG("authenticate check_broken, deadpx = %d", binfo->deadpx);
        LOG_MSG_DEBUG("authenticate check_broken, badline = %d", binfo->badline);
    }

    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}

static void silfp_send_enumerated_notice()
{
    int ret = 0;
    uint32_t count = MAX_ID_LIST_SIZE;

    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    ret = silfp_common_enumerate6(msg.data.enumerated.finger, &count);
    if(ret >= 0) {
        msg.type = FINGERPRINT_TEMPLATE_ENUMERATING;
        msg.data.enumerated.remaining_templates = count;
        msg.data.enumerated.gid = m_gid;
        notify = silfp_notify_get_notify_callback();
        if (notify != NULL) {
            notify(&msg);
        }
    }
}

static void silfp_send_touch_down_notice()
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_TOUCH_DOWN;

    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}

static void silfp_send_touch_up_notice()
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_TOUCH_UP;

    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}

static void silfp_send_hardware_notice()
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_HARDWARE;

    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}

static void silfp_bind_bigcore_bytid()
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_BINDCORE;
    msg.data.bindcore_setting.tid = gettid();

    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}

#ifdef FP_HYPNUSD_ENABLE
static void silfp_send_hypnus_notice(int32_t action_type, int32_t action_timeout)
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type = FINGERPRINT_HYPNUSDSETACION;
    msg.data.hypnusd_setting.action_type = action_type;
    msg.data.hypnusd_setting.action_timeout = action_timeout;

    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }
}
#endif

int silfp_get_enroll_total_times()
{
    uint32_t num = 0;
    if (silfp_impl_get_enroll_num(&num) < 0) {
        num = 0;
    }
    LOG_MSG_DEBUG("enroll count: %d", num);
    return num;
}

int silfp_pause_enroll()
{
    LOG_MSG_DEBUG("enroll pause");
    return silfp_enroll_pause(1);
}

int silfp_continue_enroll()
{
    LOG_MSG_DEBUG("enroll continue");
    return silfp_enroll_pause(0);
}

int silfp_pause_identify()
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int silfp_continue_identify()
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int silfp_get_alikey_status()
{
    LOG_MSG_DEBUG("pause identify not implement");
    return 0;
}

int silfp_cleanup()
{
    LOG_MSG_DEBUG("clean up");
    return silfp_common_cancel();
}

int silfp_set_touch_event_listener()
{
    LOG_MSG_DEBUG("set touch event");
    silfp_worker_set_state(STATE_LOCK);
    return 0;
}

int silfp_set_screen_state(uint32_t sreenstate)
{
    LOG_MSG_DEBUG("set screen state %d", sreenstate);
    if (FINGERPRINT_SCREEN_OFF == (fingerprint_screen_state_t)sreenstate) {
        silfp_worker_screen_state_callback(0, NULL);
    } else {
        silfp_worker_screen_state_callback(1, NULL);
    }
    return 0;
}

int silfp_dynamically_config_log(uint32_t __unused on)
{
    LOG_MSG_DEBUG("set dynamically config log not implement (%d)", on);
    return 0;
}

int silfp_get_engineering_info(uint32_t type)
{
    int ret = 0;
    switch (type) {
    case FINGERPRINT_GET_IMAGE_QUALITYS: {
        ret = silfp_ext_cb_request(EXT_CMD_IMAGE_QUALITY_GET, 0, NULL);
        break;
    }
    case FINGERPRINT_SELF_TEST: {
        ret = silfp_ext_cb_request_sync(EXT_CMD_SELF_TEST, 2);
        break;
    }
    default: {
        LOG_MSG_DEBUG("test %d not implement", type);
        break;
    }
    }
    return ret;
}


int32_t silfp_cust_otp_parse(void __unused *buf, uint32_t __unused size, uint32_t __unused offset, uint32_t __unused *otp, uint32_t __unused count)
{
    uint16_t num = NUM_ELEMS(s_otp_vendor);
    uint32_t len;
    fp_ta_otp_4f50_t *p_otp = (fp_ta_otp_4f50_t *)otp;
    char *pstr = s_otp_vendor[num-1].str;
    char *p = (char *)buf + offset + 1;
    uint8_t i;

    if (!buf || !size || !otp || !count) {
        return -SL_ERROR_BAD_PARAMS;
    }

    if ((p_otp->info.month > 0) && (p_otp->info.month < 13) && (p_otp->info.day > 0)) {
        // parse factory id
        for ( i = 0; i < num; i ++) {
            if ((uint8_t)(p_otp->info.vendor_id) == (uint8_t)s_otp_vendor[i].idx) {
                pstr = s_otp_vendor[i].str;
                break;
            }
        }
        sprintf(p, "Verdor: %s\nPN: %d\nDate: %d-%02d-%02d", pstr, p_otp->info.pn, 2010+p_otp->info.year, p_otp->info.month, p_otp->info.day);
    } else {
        sprintf(p, "%s", "NA");
    }

    len = strlen((char *)p);
    offset += len+1;
    *(p-1) = len;
    //LOG_MSG_ERROR("len=%d offset=%d, %s", len, offset, p);

    return offset;
}
int32_t silfp_cust_otp_parse_ex(void __unused *buf, uint32_t __unused size, uint32_t __unused *otp)
{
    uint32_t len = 0;
    fp_ta_otp_4f50_t *p_otp = (fp_ta_otp_4f50_t *)otp;
    char *otp_str = (char *)buf;

    if (!buf || !size || !otp) {
		LOG_MSG_INFO("otp_parse xxxxxxxxxxxxxxxxxxxxxxxxx");
        return -SL_ERROR_BAD_PARAMS;
    }
	LOG_MSG_INFO("otp_parse1 xxxxxxxxxxxxxxxxxxxxxxxxx");
    if ((p_otp->info.month > 0) && (p_otp->info.month < 13) && (p_otp->info.day > 0)) {
	LOG_MSG_INFO("otp_parse3 xxxxxxxxxxxxxxxxxxxxxxxxx");
        LOG_MSG_DEBUG("size=%d", size);
        //factory id
        LOG_MSG_DEBUG("vendor_id=%c, pn=%d", p_otp->info.vendor_id, p_otp->info.pn);
        snprintf(otp_str, size, "%c%d", p_otp->info.vendor_id, p_otp->info.pn);
        len = strlen(otp_str);
        LOG_MSG_DEBUG("vendor_id+pn=%s, len=%d", otp_str, len);
        otp_str += len;
        //year\month\day
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.year, otp_str, 1);
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.month, otp_str, 1);
        OTP_ARRAY_TO_STR(s_otp_day, p_otp->info.day, otp_str, 1);
        //pn status
        OTP_ARRAY_TO_STR(s_otp_pn_status, p_otp->info.status, otp_str, 1);
        //track_id
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.track_id, otp_str, 1);
        //sn1
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.sn1, otp_str, 1);
        //sn2
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.sn2, otp_str, 1);
        //sn3
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.sn3, otp_str, 1);
        //sn4
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.sn4, otp_str, 1);
        //sn5
        OTP_ARRAY_TO_STR(s_otp_common, p_otp->info.sn5, otp_str, 1);
		// End
       *otp_str = '\0';
        //LOG_MSG_DEBUG("serial =%s, serial_len=%d", (char *)buf, strlen((char *)buf));
        
    } else {
	LOG_MSG_INFO("otp_parse2 xxxxxxxxxxxxxxxxxxxxxxxxx");
        sprintf(otp_str, "%s", "NA");
    }

    len = strlen((char *)buf);

    return len;
}
typedef struct OplusQrCodeInfo {
	uint32_t token1;
	uint32_t errcode;
	uint32_t token2;
	uint32_t qrcode_length;
	char qrcode[32];
}OplusQrCodeInfo_t;
int32_t silfp_get_otp_msg(void __unused *buffer, uint32_t __unused size)
{
	LOG_MSG_INFO("otp_msg_enter");
    uint32_t otp[3] = {0};
	int32_t ret = 0;
	  
    if (!buffer || !size) {
	LOG_MSG_INFO("otp_msg_buf_enter");
        return -SL_ERROR_BAD_PARAMS;
    }
	LOG_MSG_INFO("otp_msg_download");
    silfp_cmd_download_normal();
    ret = silfp_cmd_get_otp(&otp[0], &otp[1], &otp[2]);
    if (ret >= 0) {
        LOG_MSG_DEBUG("OTP 0x%08X 0x%08X 0x%08X", otp[0], otp[1], otp[2]);
        ret = silfp_cust_otp_parse_ex(buffer, size, otp);
    }
	if(ret >= 0 ) {
		size = ret;
		LOG_MSG_DEBUG("otp_info_for_oplus:%s,len: %d",buffer,ret);
	}
	else {
		LOG_MSG_DEBUG("parse otp failed");	
	}

    return ret;
	 LOG_MSG_INFO("otp_msg exit");
}
void silfp_finger_opt_notify(int32_t cmd_id)
{
    fingerprint_notify_t notify = NULL;
    fingerprint_msg_t msg;
	OplusQrCodeInfo_t qrcodeinfo;
	
    memset(&msg, 0, sizeof(msg));
	uint8_t qr_code_len = sizeof(qrcodeinfo);
	
	qrcodeinfo.token1 = 1;
	qrcodeinfo.errcode = 0;
	qrcodeinfo.token2 = qrcodeinfo.token1;
	silfp_get_otp_msg(qrcodeinfo.qrcode,qr_code_len);
	qrcodeinfo.qrcode_length = (uint32_t)qr_code_len;
	
    msg.type = FINGERPRINT_OPTICAL_SENDCMD;
	msg.data.test.cmd_id = cmd_id;
	msg.data.test.result = (int8_t *) &qrcodeinfo;
	msg.data.test.result_len = sizeof(qrcodeinfo);
	LOG_MSG_DEBUG("Notify ... otp_info_for_oplus:%s,len: %d, cmd_id = %d",qrcodeinfo.qrcode,qrcodeinfo.qrcode_length,cmd_id);
	
    notify = silfp_notify_get_notify_callback();
    if (notify != NULL) {
        notify(&msg);
    }	
}
