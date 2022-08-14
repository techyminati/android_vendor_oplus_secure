#define LOG_TAG "[ANC_HAL][WorkConsumer]"

#include "anc_hal_work_consumer.h"

#include "anc_log.h"
#include <errno.h>

#include "anc_common_type.h"
#include "anc_memory_wrapper.h"

static ANC_BOOL gb_have_bound_core = ANC_FALSE;

// match with ANC_CONSUMER_STATUS
static char* gp_consumer_status_string[] = {
    "invalid",
    "init",
    "idle",
    "running",
    "exit",
};

static char *ConverConsumerStatusToString(ANC_CONSUMER_STATUS consumer_status) {
    char *p_string = "NULL";
    int consumer_status_string_array_size = sizeof(gp_consumer_status_string)/sizeof(gp_consumer_status_string[0]);
    int consumer_status_count = CONSUMER_STATUS_EXIT + 1;

    if ((consumer_status <= CONSUMER_STATUS_EXIT)
       && (consumer_status_count == consumer_status_string_array_size)) {
        p_string = gp_consumer_status_string[consumer_status];
    } else {
        ANC_LOGE("consumer status string array size:%d, consumer status count:%d, consumer status:%d",
                  consumer_status_string_array_size, consumer_status_count, consumer_status);
    }

    return p_string;
}

static void *DoTask(void *p_arg) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintWorkConsumer *p_consumer = p_manager->p_consumer;

    ANC_LOGD("start consumer thread");
    while (ANC_TRUE) {
        pthread_mutex_lock(&p_consumer->mutex);
        p_consumer->status = CONSUMER_STATUS_IDLE;
        ANC_LOGD("consumer can get work, consumer status:idle, %d",
                p_consumer->status);
        pthread_cond_signal(&p_consumer->cond);

        while (ANC_TRUE) {
            if (p_consumer->status == CONSUMER_STATUS_EXIT) {
                pthread_mutex_unlock(&p_consumer->mutex);
                ANC_LOGE("exit consumer, consumer status:exit, %d",
                        p_consumer->status);
                return NULL;
            }
            if (p_consumer->waiting_task.Dowork != NULL) {
                ANC_LOGW("consumer will work");
                break;
            }
            pthread_cond_wait(&p_consumer->cond, &p_consumer->mutex);
        }

        p_consumer->running_task = p_consumer->waiting_task;
        p_consumer->status = CONSUMER_STATUS_RUNNING;
        AncMemset(&p_consumer->waiting_task, 0, sizeof(p_consumer->waiting_task));
        pthread_cond_signal(&p_consumer->cond);
        pthread_mutex_unlock(&p_consumer->mutex);

        if (!gb_have_bound_core) {
            if (ANC_OK == p_manager->p_external_feature_manager->DoWork1(
                            p_manager->p_external_feature_manager->p_device,
                            ANC_EXTERNAL_BIND_CORE)) {
                gb_have_bound_core = ANC_TRUE;
                ANC_LOGD("consumer thread, bind cpu core");
            }
        }

        ANC_LOGW("consumer start to work, %s", p_consumer->running_task.p_name);
        p_consumer->running_task.Dowork(p_consumer->running_task.p_arg);
        ANC_LOGW("consumer finish work, %s", p_consumer->running_task.p_name);
        AncMemset(&p_consumer->running_task, 0, sizeof(p_consumer->running_task));
    }


    return NULL;
}

static ANC_RETURN_TYPE FpwcInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintWorkConsumer *p_consumer = p_manager->p_consumer;
    pthread_condattr_t consumer_pthread_condattr;

    pthread_mutex_init(&p_consumer->mutex, NULL);

    pthread_condattr_init(&consumer_pthread_condattr);
    pthread_condattr_setclock(&consumer_pthread_condattr, CLOCK_MONOTONIC);
    pthread_cond_init(&p_consumer->cond, &consumer_pthread_condattr);

    pthread_mutex_lock(&p_consumer->mutex);
    p_consumer->status = CONSUMER_STATUS_INVALID;
    pthread_mutex_unlock(&p_consumer->mutex);

    int status = pthread_create(&p_consumer->thread, NULL, DoTask, p_manager);
    if (0 != status) {
        ANC_LOGE("fail to create consumer pthread, status:%d", status);
        ret_val = ANC_FAIL;
    } else {
        pthread_mutex_lock(&p_consumer->mutex);
        p_consumer->status = CONSUMER_STATUS_INIT;
        pthread_mutex_unlock(&p_consumer->mutex);
    }

    return ret_val;
}

static ANC_RETURN_TYPE FpwcDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintWorkConsumer *p_consumer = p_manager->p_consumer;

    pthread_mutex_lock(&p_consumer->mutex);
    p_consumer->status = CONSUMER_STATUS_EXIT;
    pthread_mutex_unlock(&p_consumer->mutex);

    pthread_cond_signal(&p_consumer->cond);

    if (CONSUMER_STATUS_INVALID != p_consumer->status) {
        pthread_join(p_consumer->thread, NULL);
    }

    pthread_mutex_destroy(&p_consumer->mutex);
    pthread_cond_destroy(&p_consumer->cond);

    return ret_val;
}

static ANC_RETURN_TYPE FpwcPushTask(AncFingerprintManager *p_manager, AncWorkTask *p_task) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintWorkConsumer *p_consumer = p_manager->p_consumer;

    ANC_LOGW("pushing task to consumer");
    pthread_mutex_lock(&p_consumer->mutex);
    // waiting for consumer DoTask to be done
    ANC_LOGW("consumer status:%s, %d",
            ConverConsumerStatusToString(p_consumer->status),
            p_consumer->status);
    while (p_consumer->status != CONSUMER_STATUS_IDLE) {
        ANC_LOGW("push task, waiting for consumer DoTask(%s) to be done", p_consumer->running_task.p_name);
        pthread_cond_wait(&p_consumer->cond, &p_consumer->mutex);
    }

    p_consumer->waiting_task = *p_task;
    // can start DoTask
    ANC_LOGW("consumer can start DoTask");
    pthread_cond_signal(&p_consumer->cond);
    // waiting for DoTask to finish
    while (p_consumer->waiting_task.Dowork != NULL) {
        ANC_LOGW("waiting for consumer DoTask(%s) to start", p_consumer->waiting_task.p_name);
        pthread_cond_wait(&p_consumer->cond, &p_consumer->mutex);
    }

    pthread_mutex_unlock(&p_consumer->mutex);
    ANC_LOGW("pushed task to consumer");
    return ret_val;
}

static ANC_RETURN_TYPE FpwcClearTask(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintWorkConsumer *p_consumer = p_manager->p_consumer;

    pthread_mutex_lock(&p_consumer->mutex);
    // clear waiting task
    AncMemset(&p_consumer->waiting_task, 0, sizeof(p_consumer->waiting_task));
    pthread_mutex_unlock(&p_consumer->mutex);

    return ret_val;
}

static ANC_RETURN_TYPE FpwcGotoIdle(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintWorkConsumer *p_consumer = p_manager->p_consumer;
    int res = 0;
    uint32_t time_out = 500; // 500ms
    struct timespec time_spec_now;
    struct timespec time_spec_wait;
    int second = time_out / MILLI_SECOND_PER_SECOND;
    long micro_second = (time_out % MILLI_SECOND_PER_SECOND) * MICRO_SECOND_PER_MILLI_SECOND;
    long nano_second = 0;

    pthread_mutex_lock(&p_consumer->mutex);
    // clear waiting task
    AncMemset(&p_consumer->waiting_task, 0, sizeof(p_consumer->waiting_task));

    // waiting for current task to be done
    while (p_consumer->status != CONSUMER_STATUS_IDLE) {
        ANC_LOGD("go to idle, waiting for consumer DoTask(%s) to be done, status:%s, %d",
            p_consumer->running_task.p_name,
            ConverConsumerStatusToString(p_consumer->status),
            p_consumer->status);

        clock_gettime(CLOCK_MONOTONIC, &time_spec_now);
        nano_second = time_spec_now.tv_nsec + micro_second * NANO_SECOND_PER_MICRO_SECOND;
        time_spec_wait.tv_sec = time_spec_now.tv_sec + second + nano_second / NANO_SECOND_PER_SECOND;
        time_spec_wait.tv_nsec = nano_second % NANO_SECOND_PER_SECOND;

        res = pthread_cond_timedwait(&p_consumer->cond, &p_consumer->mutex, &time_spec_wait);
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
                goto DO_FAIL;
            case EPERM :
                ANC_LOGE("The mutex was not owned by the current thread at the time of the cal");
                ret_val = ANC_FAIL;
                goto DO_FAIL;
            default :
                ANC_LOGE("unknown error, res:%d", res);
                ret_val = ANC_FAIL;
                goto DO_FAIL;
        }
    }

DO_FAIL:
    pthread_mutex_unlock(&p_consumer->mutex);

    return ret_val;
}


static AncFingerprintWorkConsumer g_fp_consumer = {
    .Init = FpwcInit,
    .Deinit = FpwcDeinit,
    .PushTask = FpwcPushTask,
    .ClearTask = FpwcClearTask,
    .GotoIdle = FpwcGotoIdle,
};

ANC_RETURN_TYPE InitFingerprintWorkConsumer(AncFingerprintManager *p_manager) {
    p_manager->p_consumer = &g_fp_consumer;
    return g_fp_consumer.Init(p_manager);
}

ANC_RETURN_TYPE DeinitFingerprintWorkConsumer(AncFingerprintManager *p_manager) {
    return g_fp_consumer.Deinit(p_manager);
}
