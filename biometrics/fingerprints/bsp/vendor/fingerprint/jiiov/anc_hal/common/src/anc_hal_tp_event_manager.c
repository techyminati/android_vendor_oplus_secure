#define LOG_TAG "[ANC_HAL][TpManager]"

#include "anc_hal_tp_event_manager.h"

#include "anc_log.h"
#include <time.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include "anc_common_type.h"
#include "anc_memory_wrapper.h"
#include "anc_tac_time.h"
#include "anc_extension_command.h"

static ANC_TP_EVENT_TYPE g_wait_tp_event = TP_EVENT_INVALID_TYPE;
static ANC_TP_EVENT_TYPE g_tp_touch_status = TP_EVENT_INVALID_TYPE;
static ANC_TP_EVENT_TYPE g_screen_status = TP_EVENT_INVALID_TYPE;
static ANC_TP_EVENT_TYPE g_heart_rate_finger_status = TP_EVENT_INVALID_TYPE;


static ANC_RETURN_TYPE TemInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncTpEventManager *p_tp_event_manager = p_manager->p_tp_event_manager;
    pthread_condattr_t tp_pthread_condattr;

    pthread_mutex_init(&p_tp_event_manager->mutex, NULL);

    pthread_condattr_init(&tp_pthread_condattr);
    pthread_condattr_setclock(&tp_pthread_condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&p_tp_event_manager->cond, &tp_pthread_condattr);

    p_tp_event_manager->enable_touch_up_report = ANC_TRUE;

    return ret_val;
}

static ANC_RETURN_TYPE TemDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncTpEventManager *p_tp_event_manager = p_manager->p_tp_event_manager;

    pthread_cond_signal(&p_tp_event_manager->cond);

    pthread_mutex_destroy(&p_tp_event_manager->mutex);
    pthread_cond_destroy(&p_tp_event_manager->cond);

    return ret_val;
}

// time_out : ms
static ANC_RETURN_TYPE TemWaitTouchEvent(AncFingerprintManager *p_manager, ANC_TP_EVENT_TYPE wait_tp_event, uint32_t time_out) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int res = 0;
    AncTpEventManager *p_tp_event_manager = p_manager->p_tp_event_manager;
    struct timespec time_spec_now;
    struct timespec time_spec_wait;
    int second = time_out / MILLI_SECOND_PER_SECOND;
    long micro_second = (time_out % MILLI_SECOND_PER_SECOND) * MICRO_SECOND_PER_MILLI_SECOND;
    long nano_second = 0;

    pthread_mutex_lock(&p_tp_event_manager->mutex);

    g_wait_tp_event = wait_tp_event;

    clock_gettime(CLOCK_MONOTONIC, &time_spec_now);
    nano_second = time_spec_now.tv_nsec + micro_second * NANO_SECOND_PER_MICRO_SECOND;
    time_spec_wait.tv_sec = time_spec_now.tv_sec + second + nano_second / NANO_SECOND_PER_SECOND;
    time_spec_wait.tv_nsec = nano_second % NANO_SECOND_PER_SECOND;

    res = pthread_cond_timedwait(&p_tp_event_manager->cond, &p_tp_event_manager->mutex, &time_spec_wait);
    switch (res) {
        case 0 :
            ret_val = ANC_OK;
            break;
        case ETIMEDOUT :
            ret_val = ANC_FAIL_WAIT_TIME_OUT;
            break;
        case EINVAL :
            ANC_LOGE("the value specified by abstime is invalid or the value specified by cond or mutex is invalid");
            ret_val = ANC_FAIL;
            break;
        case EPERM :
            ANC_LOGE("The mutex was not owned by the current thread at the time of the cal");
            ret_val = ANC_FAIL;
            break;
        default :
            ret_val = ANC_FAIL;
            break;
    }

    g_wait_tp_event = TP_EVENT_INVALID_TYPE;

    pthread_mutex_unlock(&p_tp_event_manager->mutex);

    return ret_val;
}

#define FP_ENABLE_PATH "/proc/touchpanel/fp_enable"
ANC_RETURN_TYPE TemFpEnable(AncFingerprintManager *p_manager, bool on_off)
{
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_UNUSED(p_manager);
#ifdef ANC_LOCAL_REE_PLATFORM
    ANC_UNUSED(on_off);
#else
    FILE *fp;

    ANC_LOGD("set tp enable : %d", on_off);
    fp = fopen(FP_ENABLE_PATH, "wb");
    if (fp == NULL) {
        ANC_LOGE("%s, can't open file", FP_ENABLE_PATH);
        return ANC_FAIL;
    }
    if(0 == fwrite(on_off?"1":"0", 1, 1, fp)) {
        ret_val = ANC_FAIL;
    }

    fclose(fp);
#endif
    return ret_val;
}
// time_out : ms
static ANC_RETURN_TYPE TemWaitTouchDown(AncFingerprintManager *p_manager, uint32_t time_out) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (TP_EVENT_INVALID_TYPE != g_wait_tp_event) {
        ANC_LOGE("waiting for tp event : %d", g_wait_tp_event);
        return ANC_FAIL;
    }

    ret_val = TemWaitTouchEvent(p_manager, TP_EVENT_TOUCH_DOWN, time_out);

    return ret_val;
}

// time_out : ms
static ANC_RETURN_TYPE TemWaitTouchUp(AncFingerprintManager *p_manager, uint32_t time_out) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (TP_EVENT_INVALID_TYPE != g_wait_tp_event) {
        ANC_LOGE("waiting for tp event : %d", g_wait_tp_event);
        return ANC_FAIL;
    }

    ret_val = TemWaitTouchEvent(p_manager,  TP_EVENT_TOUCH_UP, time_out);

    return ret_val;
}

static ANC_RETURN_TYPE TemTouchDown(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncTpEventManager *p_tp_event_manager = p_manager->p_tp_event_manager;
    p_tp_event_manager->finger_down_time = AncGetElapsedRealTimeMs();

    g_tp_touch_status = TP_EVENT_TOUCH_DOWN;
    if (TP_EVENT_TOUCH_DOWN == g_wait_tp_event) {
        pthread_cond_signal(&p_tp_event_manager->cond);
    }

    return ret_val;
}

static ANC_RETURN_TYPE TemTouchUp(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncTpEventManager *p_tp_event_manager = p_manager->p_tp_event_manager;
    p_tp_event_manager->finger_up_time = AncGetElapsedRealTimeMs();

    g_tp_touch_status = TP_EVENT_TOUCH_UP;

    if (p_tp_event_manager->enable_touch_up_report) {
        ExtCommandCbOnTouchUp(p_manager);
    }

    if (TP_EVENT_TOUCH_UP == g_wait_tp_event) {
        ANC_LOGW("finger up message");
        pthread_cond_signal(&p_tp_event_manager->cond);
    }

    return ret_val;
}

static ANC_RETURN_TYPE TemHeartRateFingerDown(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_UNUSED(p_manager);

    g_heart_rate_finger_status = TP_EVENT_HEART_RATE_FINGER_DOWN;

    return ret_val;
}

static ANC_RETURN_TYPE TemHeartRateFingerUp(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_UNUSED(p_manager);

    g_heart_rate_finger_status = TP_EVENT_HEART_RATE_FINGER_UP;

    return ret_val;
}

static ANC_BOOL IsTemHeartRateFingerDown() {
    return (TP_EVENT_HEART_RATE_FINGER_DOWN == g_heart_rate_finger_status) ? ANC_TRUE : ANC_FALSE;
}

static ANC_BOOL IsTemHeartRateFingerUp() {
    return (TP_EVENT_HEART_RATE_FINGER_UP == g_heart_rate_finger_status) ? ANC_TRUE : ANC_FALSE;
}

static ANC_RETURN_TYPE TemScreenOn(struct AncFingerprintManager *p_manager) {
    ANC_UNUSED(p_manager);
    g_screen_status = TP_EVENT_SCREEN_ON;
    return ANC_OK;
}

static ANC_RETURN_TYPE TemScreenOff(struct AncFingerprintManager *p_manager) {
    ANC_UNUSED(p_manager);
    g_screen_status = TP_EVENT_SCREEN_OFF;
    return ANC_OK;
}

static ANC_BOOL IsTemTpTouchDown() {
    return (TP_EVENT_TOUCH_DOWN == g_tp_touch_status) ? ANC_TRUE : ANC_FALSE;
}

static ANC_BOOL IsTemTpTouchUp() {
    return (TP_EVENT_TOUCH_UP == g_tp_touch_status) ? ANC_TRUE : ANC_FALSE;
}

static ANC_BOOL IsTemScreenOn() {
    return (TP_EVENT_SCREEN_ON == g_screen_status) ? ANC_TRUE : ANC_FALSE;
}

static ANC_BOOL IsTemScreenOff() {
    return (TP_EVENT_SCREEN_OFF == g_screen_status) ? ANC_TRUE : ANC_FALSE;
}

static void TemSendCondSignal(struct AncFingerprintManager *p_manager) {
    AncTpEventManager *p_tp_event_manager = p_manager->p_tp_event_manager;
    pthread_cond_signal(&p_tp_event_manager->cond);
}

static AncTpEventManager g_tp_event_manager = {
    .Init = TemInit,
    .Deinit = TemDeinit,
    .TouchDown = TemTouchDown,
    .TouchUp = TemTouchUp,
    .WaitTouchDown = TemWaitTouchDown,
    .WaitTouchUp = TemWaitTouchUp,
    .ScreenOn = TemScreenOn,
    .ScreenOff = TemScreenOff,
    .IsTpTouchDown = IsTemTpTouchDown,
    .IsTpTouchUp = IsTemTpTouchUp,
    .SetFpEnable = TemFpEnable,
    .IsScreenOn = IsTemScreenOn,
    .IsScreenOff = IsTemScreenOff,
    .SendCondSignal = TemSendCondSignal,
    .HeartRateFingerDown = TemHeartRateFingerDown,
    .HeartRateFingerUp = TemHeartRateFingerUp,
    .IsHeartRateFingerDown = IsTemHeartRateFingerDown,
    .IsHeartRateFingerUp = IsTemHeartRateFingerUp,
};

ANC_RETURN_TYPE InitTpEventManager(AncFingerprintManager *p_manager) {
    p_manager->p_tp_event_manager = &g_tp_event_manager;
    return g_tp_event_manager.Init(p_manager);
}

ANC_RETURN_TYPE DeinitTpEventManager(AncFingerprintManager *p_manager) {
    return g_tp_event_manager.Deinit(p_manager);
}
