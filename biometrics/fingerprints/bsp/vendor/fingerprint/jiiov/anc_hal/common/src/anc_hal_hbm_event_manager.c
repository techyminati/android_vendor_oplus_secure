#define LOG_TAG "[ANC_HAL][HbmManager]"

#include "anc_hal_hbm_event_manager.h"

#include "anc_log.h"
#include <time.h>

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include "anc_common_type.h"
#include "anc_tac_time.h"

static ANC_HBM_EVENT_TYPE g_wait_hbm_event = HBM_EVENT_INVALID_TYPE;

static ANC_RETURN_TYPE HemInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHbmEventManager *p_hbm_event_manager = p_manager->p_hbm_event_manager;
    pthread_condattr_t hbm_pthread_condattr;

    p_hbm_event_manager->hbm_event_type = HBM_EVENT_INVALID_TYPE;
    pthread_mutex_init(&p_hbm_event_manager->mutex, NULL);

    pthread_condattr_init(&hbm_pthread_condattr);
    pthread_condattr_setclock(&hbm_pthread_condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&p_hbm_event_manager->cond, &hbm_pthread_condattr);

    return ret_val;
}

static ANC_RETURN_TYPE HemDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncHbmEventManager *p_hbm_event_manager = p_manager->p_hbm_event_manager;

    pthread_cond_signal(&p_hbm_event_manager->cond);

    pthread_mutex_destroy(&p_hbm_event_manager->mutex);
    pthread_cond_destroy(&p_hbm_event_manager->cond);

    return ret_val;
}

ANC_RETURN_TYPE HemSetHbm(AncFingerprintManager *p_manager, bool on_off)
{
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_UNUSED(p_manager);
#ifdef ANC_LOCAL_REE_PLATFORM
    ANC_UNUSED(on_off);
#else
    FILE *fp;

    ANC_LOGD("set hbm event : %d", on_off);
    fp = fopen(ANC_HBM_PATH, "wb");
    if (fp == NULL) {
        ANC_LOGE("%s, can't open file", ANC_HBM_PATH);
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
static ANC_RETURN_TYPE HemWaitHBMReadyEvent(AncFingerprintManager *p_manager, ANC_HBM_EVENT_TYPE wait_hbm_event, uint32_t time_out) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int res = 0;
    AncHbmEventManager *p_hbm_event_manager = p_manager->p_hbm_event_manager;
    struct timespec time_spec_now;
    struct timespec time_spec_wait;
    int second = time_out / MILLI_SECOND_PER_SECOND;
    long micro_second = (time_out % MILLI_SECOND_PER_SECOND) * MICRO_SECOND_PER_MILLI_SECOND;
    long nano_second = 0;

    /* Check if HBM has been ready */
    if (p_hbm_event_manager->hbm_event_type == HBM_EVENT_HBM_READY) {
        return ANC_OK;
    }

    pthread_mutex_lock(&p_hbm_event_manager->mutex);

    g_wait_hbm_event = wait_hbm_event;

    clock_gettime(CLOCK_MONOTONIC, &time_spec_now);
    nano_second = time_spec_now.tv_nsec + micro_second * NANO_SECOND_PER_MICRO_SECOND;
    time_spec_wait.tv_sec = time_spec_now.tv_sec + second + nano_second / NANO_SECOND_PER_SECOND;
    time_spec_wait.tv_nsec = nano_second % NANO_SECOND_PER_SECOND;

    res = pthread_cond_timedwait(&p_hbm_event_manager->cond, &p_hbm_event_manager->mutex, &time_spec_wait);
    switch (res) {
        case 0 :
            ret_val = ANC_OK;
            ANC_LOGD("hbm event is coming");
            break;
        case ETIMEDOUT :
            ret_val = ANC_FAIL_WAIT_TIME_OUT;
            ANC_LOGD("wait hbm event is time out");
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

    g_wait_hbm_event = HBM_EVENT_INVALID_TYPE;

    pthread_mutex_unlock(&p_hbm_event_manager->mutex);

    return ret_val;
}

// time_out : ms
static ANC_RETURN_TYPE HemWaitHbmReady(struct AncFingerprintManager *p_manager, uint32_t time_out) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (TP_EVENT_INVALID_TYPE != g_wait_hbm_event) {
        ANC_LOGE("waiting for hbm event : %d", g_wait_hbm_event);
        return ANC_FAIL;
    }

    ret_val = HemWaitHBMReadyEvent(p_manager, HBM_EVENT_HBM_READY, time_out);

    return ret_val;
}



static ANC_RETURN_TYPE HemHBMReady(struct AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncHbmEventManager *p_hbm_event_manager = p_manager->p_hbm_event_manager;
    p_hbm_event_manager->hbm_ready_time = AncGetElapsedRealTimeMs();

    ANC_LOGD("HBM is Ready");

    usleep(ANC_CAPTURE_DELAY); // delay 17ms
    p_hbm_event_manager->hbm_event_type = HBM_EVENT_HBM_READY;

    if (HBM_EVENT_HBM_READY == g_wait_hbm_event) {
        pthread_cond_signal(&p_hbm_event_manager->cond);
    }

    return ret_val;
}

ANC_RETURN_TYPE HemSetHbmEventType(struct AncFingerprintManager *p_manager, ANC_HBM_EVENT_TYPE hbm_event_type) {
    AncHbmEventManager *p_hbm_event_manager = p_manager->p_hbm_event_manager;
    p_hbm_event_manager->hbm_event_type = hbm_event_type;

    return ANC_OK;
}

static void HemSendCondSignal(struct AncFingerprintManager *p_manager) {
    AncHbmEventManager *p_hbm_event_manager = p_manager->p_hbm_event_manager;
    pthread_cond_signal(&p_hbm_event_manager->cond);
}

static AncHbmEventManager g_hbm_event_manager = {
    .Init = HemInit,
    .Deinit = HemDeinit,
    .HBMReady = HemHBMReady,
    .WaitHBMReady = HemWaitHbmReady,
    .SetHbm = HemSetHbm,
    .SetHbmEventType = HemSetHbmEventType,
    .SendCondSignal = HemSendCondSignal,
};

ANC_RETURN_TYPE InitHbmEventManager(AncFingerprintManager *p_manager) {
    p_manager->p_hbm_event_manager = &g_hbm_event_manager;
    return g_hbm_event_manager.Init(p_manager);
}

ANC_RETURN_TYPE DeinitHbmEventManager(AncFingerprintManager *p_manager) {
    return g_hbm_event_manager.Deinit(p_manager);
}
