#include "device_int.h"
#include "fps_normal.h"
#include "plat_log.h"
#include "plat_time.h"
#include "response_def.h"
#include "plat_thread.h"
#include "common_definition.h"
#include "opt_file.h"
#ifndef _WINDOWS
#include <pthread.h>
pthread_t g_thread_on_off, g_thread_ui;
#endif
#include "oplus_event.h"

#if defined(SIMULATE_NO_SENSOR) || defined(RBS_EVTOOL)
#define POLL_TIME 1
#else
#define POLL_TIME 10
#endif

extern int g_hdev;
extern mutex_handle_t g_thread_power_on_lock;
extern int g_oparate_type;

struct HostTouchEvent {
    BOOL is_enable;
    BOOL is_finger_on;
    uint64_t timestamp_finger_on;
    BOOL is_finger_off;
    BOOL is_ui_ready;
    uint64_t timestamp_finger_off;
    BOOL is_low_coverage;
};

struct HostTouchEvent g_host_touch = {
#ifdef HOST_TOUCH_CONTROL
    .is_enable = TRUE,
#else
    .is_enable = FALSE,
#endif
    .is_finger_on = FALSE,
    .is_finger_off = FALSE,
    .is_low_coverage = FALSE};
void host_touch_set_enable(BOOL enable) {
    ex_log(LOG_DEBUG, "%s, %d", __func__, enable);
    g_host_touch.is_enable = enable;
    g_host_touch.is_finger_on = FALSE;
    g_host_touch.is_finger_off = FALSE;
    g_host_touch.is_low_coverage = FALSE;
    g_host_touch.is_ui_ready = FALSE;
}

void host_touch_set_finger_reset() {
    ex_log(LOG_DEBUG, "%s", __func__);
    g_host_touch.is_finger_on = FALSE;
    g_host_touch.is_finger_off = FALSE;
    g_host_touch.timestamp_finger_on = 0;
    g_host_touch.timestamp_finger_off = 0;
}

void host_touch_set_finger_on(BOOL on) {
    g_host_touch.is_finger_on = on;
    if (on) {
        g_host_touch.timestamp_finger_on = plat_get_time();
        ex_log(LOG_DEBUG, "%s, %d", __func__, on);
    } else {
        ex_log(LOG_DEBUG, "%s, reset.", __func__);
    }
    // ex_log(LOG_DEBUG, "timestamp finger on %lld", g_host_touch.timestamp_finger_on);
}

void host_touch_set_low_coverage(BOOL flag) {
    ex_log(LOG_DEBUG, "%s, %d", __func__, flag);
    g_host_touch.is_low_coverage = flag;
}

void host_touch_set_finger_off(BOOL off) {
    g_host_touch.is_finger_off = off;
    if (off) {
        g_host_touch.timestamp_finger_off = plat_get_time();
        ex_log(LOG_DEBUG, "%s, %d", __func__, off);
    } else {
        ex_log(LOG_DEBUG, "%s, reset.", __func__);
    }
    // ex_log(LOG_DEBUG, "timestamp finger off %lld", g_host_touch.timestamp_finger_off);
}

func_notify_t g_fn_callback = NULL;
static uint64_t g_get_image_start_time = 0;

void host_touch_set_ui_ready(BOOL ready) {
    ex_log(LOG_DEBUG, "%s, %d", __func__, ready);
    g_host_touch.is_ui_ready = ready;
}

BOOL host_touch_is_ui_ready() {
    return g_host_touch.is_ui_ready;
}

BOOL host_touch_is_using_oplus_flow() {
    extern int g_app_type;
    return (g_app_type == 2) ? TRUE : FALSE;
}

void get_image_check_start() {
    g_get_image_start_time = plat_get_time();
}

int get_image_stable_expo(unsigned long long start_wait_expo) {
    return g_get_image_start_time - start_wait_expo;
}

BOOL get_image_is_too_fast(uint32_t duration) {
    int32_t diff_time = g_host_touch.timestamp_finger_off - g_get_image_start_time;

    if (((diff_time < (int32_t)duration) && (diff_time > 0)) || (diff_time < 0)) {
        ex_log(LOG_DEBUG, "finger off early, delta = %d", diff_time);
        return TRUE;
    } else {
        ex_log(LOG_DEBUG, "finger off not early, delta = %d", diff_time);
        return FALSE;
    }
}

BOOL is_verify_finger_on_stable(BOOL isretry, int stable_time) {
    BOOL is_finger_on_stable;
    unsigned long long finger_on_off_diff;
    int total_exp_time;
    int total_stable_time;
    int out_size = sizeof(int);
    int enable_pipeline = VENDOR_ENABLE_PIPLINE;
    ex_log(LOG_DEBUG, "is_verify_finger_on_stable retry %d time %d", isretry, stable_time);

    opt_receive_data(TYPE_RECEIVE_TOTAL_EXP_TIME, NULL, 0, (unsigned char*)&total_exp_time,
                     &out_size);
    finger_on_off_diff = g_host_touch.timestamp_finger_off - g_get_image_start_time;
    total_stable_time = (unsigned long long)(total_exp_time + VENDOR_ET7XX_FINGER_OFF_CHECK_DELAY_TIME);
    is_finger_on_stable = finger_on_off_diff >= (unsigned long long)total_stable_time;

    ex_log(LOG_DEBUG,"finger_on_time=%llu finger_off_time=%llu", g_get_image_start_time, g_host_touch.timestamp_finger_off);
    ex_log(LOG_DEBUG,"finger_on_off_diff=%llu, total_exp_time=%d, finger_on_stable=%d", finger_on_off_diff,
              total_exp_time, is_finger_on_stable);

    if (enable_pipeline){		//if pipeline enabled
        if (isretry){
            is_finger_on_stable = stable_time > total_stable_time;
            ex_log(LOG_DEBUG,"is_finger_on_stable %d ", is_finger_on_stable);
        }
    }

    return is_finger_on_stable;
}

oplus_trigger_case_t g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;

oplus_trigger_case_t ui_trigger_case = TRIGGER_WAIT_UI_READY;

void* wait_oplus_trigger(void* pdata) {
    ex_log(LOG_DEBUG, "pdata = %d", (int)pdata);
    unsigned long long time_start_waitfinger = plat_get_time();
    ex_log(LOG_DEBUG, "oplus trigger thread time start = %llu", time_start_waitfinger);

    egis_netlink_threadloop(pdata);
    return NULL;
}

void host_touch_set_callback(func_notify_t callback) {
    g_fn_callback = callback;
}

int init_oplus_event_listener(uint8_t fd) {
#ifndef _WINDOWS
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    pthread_create(&g_thread_ui, &attr, wait_oplus_trigger, (void*)(unsigned long)(fd));
#endif
    return 0;
}

void host_touch_set_hbm_evalute_always(void) {
    extern int g_app_type;
    if (g_app_type == 2) return;
}

void host_touch_set_hbm_system(__unused BOOL on_off) {
    // on_off++;
    /*	extern int g_app_type;
        if (g_app_type != 2)
            return;
        host_touch_set_hbm(on_off);
        plat_sleep_time(50);
    */
}

#define HBM_PATH "/sys/kernel/oplus_display/hbm"

void host_touch_set_hbm(BOOL on_off) {
    FILE* fp;
    int ret = FINGERPRINT_RES_SUCCESS;

    ex_log(LOG_DEBUG, "%s [%d] ", __func__, on_off);
    fp = fopen(HBM_PATH, "wb");
    if (fp == NULL) {
        ex_log(LOG_ERROR, "%s, can't open file", __func__);
        return;
    }
    ret = fwrite(on_off ? "1" : "0", 1, 1, fp);
    fclose(fp);
}

#define TOUCH_DOWN_SET_HBM(x)                                       \
    do {                                                            \
        if (x == FINGERPRINT_RES_SUCCESS) host_touch_set_hbm(TRUE); \
    } while (0)

BOOL host_touch_is_enable() {
    return g_host_touch.is_enable;
}

BOOL host_touch_is_finger_on() {
    return g_host_touch.is_finger_on;
}

BOOL host_touch_is_finger_on_last() {
    if (g_host_touch.timestamp_finger_on > g_host_touch.timestamp_finger_off) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL host_touch_is_finger_off_last() {
    return FALSE;
}

BOOL host_touch_is_low_coverage() {
    return g_host_touch.is_low_coverage;
}

BOOL host_touch_is_finger_off() {
    return g_host_touch.is_finger_off;
}

BOOL wait_trigger(int retry_count, int timeout_for_one_try, int total_timeout) {
    BOOL bret = FALSE;
    BOOL bWait = TRUE;
    int ret_val = 999;
    int icount = retry_count;
    int retval = FINGERPRINT_RES_SUCCESS;
    unsigned long long time_start_waitfinger = plat_get_time();
    ex_log(LOG_DEBUG, "time start = %llu", time_start_waitfinger);

    if (host_touch_is_enable()) {
        while (bWait) {
            plat_sleep_time(POLL_TIME);
            if (total_timeout >= 0) {
                if ((int)(plat_get_diff_time(time_start_waitfinger) / 1000) > total_timeout) {
                    ex_log(LOG_INFO, "!!! enroll timeout !!! =%d", total_timeout);
                    bret = FINGERPRINT_RES_SECURE_ENROLL_TIMEOUT;
                    break;
                }
            }
            if (check_cancelable()) break;
            retval = host_touch_is_finger_on() ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_TIMEOUT;
            if (check_cancelable()) break;

            if (host_touch_is_using_oplus_flow()) {
                if (g_trigger_case == TRIGGER_WAIT_TOUCH_DOWN) {
                    if (check_cancelable()) break;
                    retval = (host_touch_is_ui_ready() && host_touch_is_finger_on())
                                 ? FINGERPRINT_RES_SUCCESS
                                 : FINGERPRINT_RES_TIMEOUT;
                    // TOUCH_DOWN_SET_HBM(retval);
                    if (check_cancelable()) break;
                } else if (g_trigger_case == TRIGGER_WAIT_TOUCH_UP) {
                    if (check_cancelable()) break;
                    retval = host_touch_is_finger_off() ? FINGERPRINT_RES_SUCCESS
                                                        : FINGERPRINT_RES_TIMEOUT;
                    if (check_cancelable()) break;
                }
            }

            if (check_cancelable()) break;

            if (retry_count != 0 && icount-- <= 0) bWait = FALSE;

            if (FINGERPRINT_RES_SUCCESS == retval) {
                if (host_touch_is_finger_off_last()) {
                    if (!host_touch_is_using_oplus_flow()) host_touch_set_finger_reset();
                    ex_log(LOG_DEBUG, "poll touch event skipped.");

                    bret = TRUE;

                    break;
                } else {
                    if (!host_touch_is_using_oplus_flow()) host_touch_set_finger_reset();
                    ex_log(LOG_DEBUG, "poll touch event trigger.");
                    bret = TRUE;
                    break;
                }
            }
        }
        if(g_oparate_type == DO_VERIFY){
            if(g_trigger_case == TRIGGER_WAIT_TOUCH_DOWN){
                ex_log(LOG_DEBUG, "g_thread_power_on_lock begin");
                ret_val = plat_mutex_lock(g_thread_power_on_lock);
                ex_log(LOG_DEBUG, "lock g_thread_power_on_lock ret_val = %d",ret_val);
                ret_val = plat_mutex_unlock(g_thread_power_on_lock);
                ex_log(LOG_DEBUG, "unlock TRIGGER_WAIT_TOUCH_DOWN g_thread_power_on_lock ret_val = %d",ret_val);
            }else{
                ret_val = plat_mutex_unlock(g_thread_power_on_lock);
                ex_log(LOG_DEBUG, "unlock TRIGGER_WAIT_TOUCH_UP g_thread_power_on_lock ret_val = %d",ret_val);
            }
        }

        return bret;
    }

#ifdef ENABLE_POLL
    ex_log(LOG_DEBUG, "poll trigger");
    return TRUE;
#endif
    retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
    retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_INIT);

    if (retval != FINGERPRINT_RES_SUCCESS) return FALSE;

    while (bWait) {
        if (total_timeout >= 0)
            if ((int)(plat_get_diff_time(time_start_waitfinger) / 1000) > total_timeout) break;

        if (check_cancelable()) goto exit;
        retval = fp_device_interrupt_wait(g_hdev, timeout_for_one_try);
        if (check_cancelable()) goto exit;

        if (retry_count != 0 && --icount <= 0) bWait = FALSE;

        if (FINGERPRINT_RES_SUCCESS == retval) {
            ex_log(LOG_DEBUG, "interrupt trigger");
            bret = TRUE;
#ifdef __DEVICE_DRIVER_MEIZU_1712__
            // MEIZU_1712 workaround, need abort
            fp_device_interrupt_enable(g_hdev, FLAG_INT_ABORT);
            return bret;
#else
            break;
#endif
        }
    }
exit:
#ifndef _WINDOWS
    fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
#endif
    return bret;
}

poll_revents event_poll_wait(event_check_fd* check_group, uint32_t fd_count, int timeout) {
    const int sleep_time = 30;
    int count;
    unsigned int i;
    event_check_fd* check;
    poll_revents got_event = REVENT_TIMEOUT;

    if (NULL == check_group) {
        ex_log(LOG_ERROR, "event_poll_wait NULL check_group");
        return -1;
    }

    count = timeout / sleep_time;
    do {
        for (i = 0; i < fd_count; i++) {
            check = check_group + i;
            if (NULL == check->checkerFunc) return -1;
            check->revent = check->checkerFunc();
            if (check->revent) {
                got_event = REVENT_EVENT_POLLIN;
                // ex_log(LOG_DEBUG, "check, %d, %d",i, check->events);
            }
        }
        ex_log(LOG_DEBUG, "spi_poll_wait [%d] ", count);
        if (got_event != REVENT_TIMEOUT) break;

        if (count > 0) {
            plat_sleep_time(sleep_time);
        }
    } while (count-- > 0);

    return got_event;
}
