#define LOG_TAG "[ANC_HAL][FPManager]"

#include "anc_hal_manager.h"

#include <string.h>
#include <cutils/trace.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <signal.h>

#include "anc_common_type.h"
#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "anc_algorithm.h"
#include "anc_tac_sensor.h"
#include "sensor_command_param.h"
#include "anc_token.h"
#include "anc_extension.h"
#include "anc_hal_extension_command.h"
#include "anc_hal_sensor_device.h"
#include "anc_tac_time.h"
#include "anc_memory_wrapper.h"
#include "anc_auxiliary_command.h"
#include "anc_extension_command.h"
#include "anc_log_string.h"
#include "anc_lib.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <errno.h>

#ifdef ANC_SAVE_ALGO_FILE
#include "anc_background_worker.h"
#endif

#define  ANC_FINGERPRINT_PROCESS_PRIORITY_DEFAULT -10  // 120-10=110

#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_ALWAYS

#define AUTHENTICATE_RETRY_MAX 5
#define ONE_TOUCH_EVENT_RETRY_COUNT 3
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"
#define PROPERTY_FINGERPRINT_QRCODE "oplus.fingerprint.qrcode.support"
#define PROPERTY_FINGERPRINT_QRCODE_VALUE "oplus.fingerprint.qrcode.value"

volatile bool g_cancel = ANC_FALSE;
volatile bool g_cancel_heart_rate_detect = ANC_TRUE;

long long pre_enroll_start_time;

AncFTAlgoTimeInfo g_time_info;
long long g_finger_up_down_time;
ANC_BOOL g_is_screen_off;
#ifdef ANC_SAVE_ALGO_FILE
extern AncBGWorker g_background_worker;
#endif

static int FpmInnerCancel(AncFingerprintManager *p_manager);

static ANC_RETURN_TYPE FpmInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("[FPM] Init");

    pthread_mutex_init(&p_manager->p_producer->mutex, NULL);
    pthread_mutex_init(&p_manager->p_producer->reportup_mutex, NULL);
    g_cancel = ANC_FALSE;
    return ret_val;
}

static ANC_RETURN_TYPE FpmDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("[FPM] Deinit");

    pthread_mutex_lock(&p_manager->p_producer->mutex);
    // do somethine here

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    pthread_mutex_destroy(&p_manager->p_producer->mutex);
    pthread_mutex_destroy(&p_manager->p_producer->reportup_mutex);
    return ret_val;
}

static void FpmPushTaskToConsumer(AncFingerprintManager *p_manager,
                              void (*Dowork)(void *p_arg), void *p_arg,
                              uint8_t *p_name) {
    AncWorkTask work_task;

    ANC_LOGD("[FPM] PushTaskToConsumer, task name:%s", p_name);

    work_task.Dowork = Dowork;
    work_task.p_arg = (void *)p_arg;
    work_task.p_name = p_name;
    p_manager->p_consumer->PushTask(p_manager, &work_task);
}



static void DoPreEnroll(void *p_arg) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    uint64_t challenge = 0;
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ret_val = GetEnrollChallenge(&challenge);
    if (ANC_OK == ret_val) {
        p_manager->p_producer->challenge = challenge;
    } else {
        p_manager->p_producer->challenge = 0;
    }
}

static uint64_t FpmPreEnroll(AncFingerprintManager *p_manager) {
    ANC_LOGD("[FPM] PreEnroll");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    pre_enroll_start_time = AncGetElapsedRealTimeMs();
    ANC_LOGD("PreEnroll start time = %lld", pre_enroll_start_time);

    DoPreEnroll((void *)p_manager);

    pthread_mutex_unlock(&p_manager->p_producer->mutex);

    return p_manager->p_producer->challenge;
}

#ifdef ANC_QUICKLY_PICK_UP_FINGER
#define FINGERPRINT_COUNT_THRESHOLD  2
#ifdef ANC_QUICKLY_PICK_UP_FINGER_VERSION_V2
#define GET_EXPO_RATIO_TIME(x)  ((x) / 3)
#endif
long long g_capture_last_expo_start_time;
long long g_capture_retry0_total_expo_time;
long long g_previous_finger_up_time;

static ANC_BOOL IsFingerUp(struct AncFingerprintManager *p_manager) {
    if ((p_manager->p_tp_event_manager->IsTpTouchUp())
            || (g_previous_finger_up_time != p_manager->p_tp_event_manager->finger_up_time)) {
        return ANC_TRUE;
    }
    return ANC_FALSE;
}

static ANC_RETURN_TYPE CheckQuicklyPickupFinger(struct AncFingerprintManager *p_manager, uint32_t retry_count) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    long long expo_finger_on_time = 0;
    int32_t total_expo_time = 100;  // 100ms

    if (retry_count == 0) {
        g_previous_finger_up_time = p_manager->p_tp_event_manager->finger_up_time;
        p_manager->p_tp_event_manager->enable_touch_up_report = ANC_TRUE;
    }
    if (IsFingerUp(p_manager)) {
        expo_finger_on_time = p_manager->p_tp_event_manager->finger_up_time - g_capture_last_expo_start_time;
        long long finger_up_down_time = p_manager->p_tp_event_manager->finger_up_time -
                                        p_manager->p_tp_event_manager->finger_down_time;
        g_finger_up_down_time = finger_up_down_time;
#ifdef ANC_QUICKLY_PICK_UP_FINGER_VERSION_V2
        int32_t threshold_expo_time = 100;
        SensorGetTotalExposureTime(&total_expo_time);
        if (retry_count == 0) {
            threshold_expo_time = (int32_t)g_capture_retry0_total_expo_time - GET_EXPO_RATIO_TIME(total_expo_time);
        } else {
            threshold_expo_time = total_expo_time;
        }
        ANC_LOGD("retry_count = %d, expo_finger_on_time = %lld, total_expo_time = %d,  threshold_expo_time = %d, finger_up_down_time = %lld", retry_count, expo_finger_on_time, total_expo_time, threshold_expo_time, finger_up_down_time);
        if (expo_finger_on_time < threshold_expo_time) {
#else
        if (retry_count == 0) {
            total_expo_time = (int32_t)g_capture_retry0_total_expo_time;
        } else {
            SensorGetTotalExposureTime(&total_expo_time);
        }
        ANC_LOGD("retry_count = %d, expo_finger_on_time = %lld, total_expo_time = %d, finger_up_down_time = %lld", retry_count, expo_finger_on_time, total_expo_time, finger_up_down_time);
        if (expo_finger_on_time < total_expo_time) {
#endif
            if (retry_count == 0) {
#ifdef ANC_FINGER_ON_TIME
                if (finger_up_down_time >= ANC_FINGER_ON_LIMIT_TIME)
                {
                    ret_val = ANC_CAPTURE_FINGER_UP;
                    ANC_LOGE("finger up after capture image, ret value:%d", ret_val);
                }
                else
#endif
                {
                    ret_val = ANC_CAPTURE_FINGER_MOVE_TOO_FAST;
                    ANC_LOGE("finger up too fast before capture image, ret value:%d", ret_val);
                }
            } else {
                ret_val = ANC_ALGO_MATCH_FAIL;
                ANC_LOGE("finger up before capture image, ret value:%d", ret_val);
            }
        } else {
            if (GetAllFingerprintsCount() > FINGERPRINT_COUNT_THRESHOLD) {
                ret_val = ANC_ALGO_MATCH_FAIL;
            } else {
                ret_val = ANC_CAPTURE_FINGER_UP;
            }
            ANC_LOGE("finger up after capture image, ret value:%d", ret_val);
        }
        if (retry_count == 0) {
            ExtCommandCbOnTouchUp(p_manager);
        }
    } else {
        g_finger_up_down_time = 1000; // 1000ms as a default value for no finger up
    }

    return ret_val;
}
#endif

#define ANC_ENROLL_CONTINUE_FAIL_MAX 10
#define CHECK_ENROLL_FAIL_CNT(_cnt) \
  do { \
    _cnt ++; \
    ANC_LOGD("Enroll continue fail cnt = %d", _cnt);  \
    if (_cnt == ANC_ENROLL_CONTINUE_FAIL_MAX) { \
      ret_val = ANC_ENROLL_FAIL; \
      goto DO_FAIL; \
    } \
  } while (0)

static void DoEnroll(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint32_t tp_ret = ANC_OK;

    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;

    int32_t scaling_time = ANC_SCALE_CPU_FREQUENCY_TIMEOUT_2000;

    uint32_t finger_id = 0;
    uint32_t group_id = p_manager->p_producer->current_group_id;
    uint32_t remaining = 0;
    ANC_BOOL is_need_deinit_enroll = ANC_FALSE;

    uint32_t finger_id_array[5];
    uint32_t finger_id_count = 0;
    uint32_t sync_array[6];
    uint32_t groupId = p_manager->p_producer->current_group_id;
    uint8_t enroll_continuous_fail_cnt = 0;

    AncMemset(finger_id_array, 0, sizeof(finger_id_array));
    AncMemset(sync_array, 0, sizeof(sync_array));
    AncMemset(&g_time_info, 0, sizeof(AncFTAlgoTimeInfo));
    sync_array[0] = groupId;

    long long pre_enroll_end_time = AncGetElapsedRealTimeMs();
    uint32_t pre_enroll_second_difference = (uint32_t)((pre_enroll_end_time - pre_enroll_start_time) / 1000);
    ANC_LOGD("PreEnroll end time:%lld, second difference = %d ", pre_enroll_end_time, pre_enroll_second_difference);
    if (pre_enroll_second_difference > p_manager->p_producer->enroll_timeout_second) {
        ret_val = ANC_ENROLL_TIME_OUT;
        ANC_LOGD("PreEnroll timeout");
        goto DO_FAIL;
    }

    long long enroll_start, enroll_end;

    enroll_start = AncGetElapsedRealTimeMs();

#ifdef FP_SET_PRIORITY
    setpriority(PRIO_PROCESS, 0, ANC_FINGERPRINT_PROCESS_PRIORITY_DEFAULT);
#endif

    if (ANC_OK == p_manager->p_external_feature_manager->DoWork1(
                        p_manager->p_external_feature_manager->p_device,
                        ANC_EXTERNAL_BIND_CORE)) {
        ANC_LOGD("enroll, bind cpu core");
    }

#ifdef ANC_USE_HW_AUTH_TOKEN
    if (ANC_OK != (ret_val = AuthorizeEnroll((uint8_t *)&p_manager->p_producer->auth_token, sizeof(p_manager->p_producer->auth_token)))) {
        ret_val = ANC_ENROLL_TIME_OUT; // temporary release for luna fix bug : https://jira.jiiov.com/browse/REALMELUNA-17
        goto DO_FAIL;
    }
#endif

    if (ANC_OK != (ret_val = AlgoInitEnroll())) {
        goto DO_FAIL;
    }
    is_need_deinit_enroll = ANC_TRUE;

    /* HW reset and re-init sensor before enroll */
    ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_HW_RESET_INIT);
    if(ANC_OK != ret_val) {
        ANC_LOGE("enroll hw reset failed");
        goto DO_FAIL;
    }

    ANC_LOGD("enroll clear TP flag");
    if (ANC_OK != (ret_val = SensorClearTPFlag())) {
        ANC_LOGE("enroll fail to clear tp flag");
    }

    tp_ret = p_manager->p_tp_event_manager->SetFpEnable(p_manager,true);
    if(ANC_OK != tp_ret) {
        ANC_LOGE("enroll setFpEnable failed");
        goto DO_FAIL;
    }

    while (ANC_TRUE) {
        if(g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        enroll_end = AncGetElapsedRealTimeMs();
        uint32_t second_difference = (uint32_t)((enroll_end - enroll_start) / 1000);
        if (second_difference > p_manager->p_producer->enroll_timeout_second) {
            ret_val = ANC_ENROLL_TIME_OUT;
            goto DO_FAIL;
        }

        // ret_val = p_manager->p_hbm_event_manager->SetHbm(p_manager, true);
        // if(ANC_OK != ret_val) {
        //     ANC_LOGE("enroll set hbm  failed");
        //     goto DO_FAIL;
        // }

        // if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
        //     ExtCommandCbOnTouchUp(p_manager);
        // }

        p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_DETECTED);

#ifdef USE_SLEEP_REPLACE_WAIT_TP_EVENT
        sleep(1); // waiting for 1 second
#else
        if(g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }
        ret_val = p_manager->p_tp_event_manager->WaitTouchDown(p_manager, WAIT_TOUCH_DOWN_TIME_OUT);
        switch (ret_val) {
            case ANC_OK :
                break;
            case ANC_FAIL_WAIT_TIME_OUT :
                continue;
            default :
                goto DO_FAIL;
        }
#endif

#ifdef ANC_QUICKLY_PICK_UP_FINGER
        g_previous_finger_up_time = p_manager->p_tp_event_manager->finger_up_time;
#endif

        if (g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        if (ANC_OK == p_manager->p_external_feature_manager->DoWork3(
                            p_manager->p_external_feature_manager->p_device,
                            ANC_EXTERNAL_SCALE_CPU_FREQUENCY,
                            (uint8_t *)(&scaling_time), sizeof(scaling_time))) {
            ANC_LOGD("scaling time : %d ms", scaling_time);
        }

        p_manager->p_hbm_event_manager->SetHbmEventType(p_manager, HBM_EVENT_INVALID_TYPE);
        ExtCommandCbOnTouchDown(p_manager);

        if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP_RESET))) {
            goto DO_FAIL;
        }

#ifdef ANC_WAIT_HBM_READY
WAIT_HBM:
        if (g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }
        if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
            ExtCommandCbOnTouchUp(p_manager);
            p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_FAST);
            p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
            continue;
        }
        //wait HBM ready signal, 600ms
        ret_val = p_manager->p_hbm_event_manager->WaitHBMReady(p_manager, WAIT_HBM_TIME_OUT);
        switch (ret_val) {
            case ANC_OK :
                break;
            case ANC_FAIL_WAIT_TIME_OUT :
                ANC_LOGE("enroll wait hbm ready time out, ret value: %d", ret_val);
                // p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_HBM_TIMEOUT);
                goto WAIT_HBM;
            default :
                goto DO_FAIL;
        }
        if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
            ExtCommandCbOnTouchUp(p_manager);
            p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_FAST);
            p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
            continue;
        }
#endif

#ifdef ANC_QUICKLY_PICK_UP_FINGER
        g_previous_finger_up_time = p_manager->p_tp_event_manager->finger_up_time;
#endif

        if(g_cancel == ANC_TRUE) {
            ExtCommandCbOnTouchUp(p_manager);
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        ANC_TIME_MEASURE_START(CaptureImage);
        ret_val = SensorCaptureImage();
        ANC_GET_TIME_MEASURE_END(CaptureImage, "enroll capture image",&g_time_info.capture_time[0]);
#ifdef ANC_QUICKLY_PICK_UP_FINGER
        if (IsFingerUp(p_manager)) {
            ExtCommandCbOnTouchUp(p_manager);
            p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_FAST);
            p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
            continue;
        }
#endif
        if (ANC_OK != ret_val) {
            ANC_LOGE("fail to capture image, ret value:%d", ret_val);
            if ((ANC_CAPTURE_LOW_AUTO_EXP == ret_val) || (ANC_CAPTURE_LOW_ENV_LIGHT == ret_val)
                || (ANC_CAPTURE_HIGH_ENV_LIGHT == ret_val) || (ANC_CAPTURE_IMAGE_CHECK_FAIL == ret_val)) {
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_PARTIAL);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                continue;
            } else if ((ANC_CAPTURE_WAIT_IMAGE_TIMEOUT == ret_val) || (ANC_CAPTURE_IMAGE_CRC_ERROR == ret_val)
                || (ANC_CAPTURE_RD_UNDERFLOW == ret_val)) {
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                /* It should be sensor issue, try to hw reset and re-init sensor */
                if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_HW_RESET_INIT)) {
                    goto DO_FAIL;
                }
                continue;
            }
            goto DO_FAIL;
        }

        if(g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        ANC_TIME_MEASURE_START(AlgoEnroll);
        do {
            ret_val = AlgoEnrollExtractFeature();
            if (ret_val != ANC_ALGO_EXTRACT_OK) {
                ANC_LOGE("extract feature failed during enroll. ret:%d", ret_val);
                break;
            }
            ret_val = AlgoEnrollFeature(&remaining, &finger_id);
        } while (0);
        ANC_GET_TIME_MEASURE_END(AlgoEnroll, "enroll algo", &g_time_info.enroll_time);
        ANC_LOGD("enroll, return type:%s, %d", AncConvertReturnTypeToString(ret_val), ret_val);
        switch (ret_val) {
            case ANC_ALGO_ENROLL_OK_CONTINUE:
                enroll_continuous_fail_cnt = 0;
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_GOOD);
                finger_id = 0;
                p_manager->p_producer->OnEnrollResult(p_device, finger_id, group_id, remaining);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                continue;
            case ANC_ALGO_ENROLL_FINISH:
                enroll_continuous_fail_cnt = 0;
                is_need_deinit_enroll = ANC_FALSE;
                ANC_LOGD("enroll finish finger id is:%d", finger_id);
                p_manager->p_producer->OnEnrollResult(p_device, finger_id, group_id, 0);
                if (ANC_OK != (ret_val = AlgoDeinitEnroll(&finger_id,ANC_TRUE))) {
                    goto DO_FAIL;
                }
                goto DO_OUT;
            case ANC_ALGO_ENROLL_SAME_FINGER:
                enroll_continuous_fail_cnt = 0;
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_ALREADY_ENROLLED);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                continue;
            case ANC_ALGO_ENROLL_DUPLICATE:
                CHECK_ENROLL_FAIL_CNT(enroll_continuous_fail_cnt);
                // p_manager->p_producer->OnEnrollResult(p_device, 0, group_id, remaining);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_SIMILAR);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                continue;
            case ANC_ALGO_BAD_IMG:
            case ANC_ALGO_LOW_QTY:
            case ANC_ALGO_ENROLL_LOW_QTY:
            case ANC_ALGO_FAKE_FINGER:
            case ANC_ALGO_GHOST_FINGER:
            case ANC_ALGO_EXTRACT_FAIL:
            case ANC_ALGO_ENROLL_FAIL:
                CHECK_ENROLL_FAIL_CNT(enroll_continuous_fail_cnt);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                continue;
            case ANC_ALGO_PARITAL:
                CHECK_ENROLL_FAIL_CNT(enroll_continuous_fail_cnt);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_PARTIAL);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                continue;
            default:
                ANC_LOGD("manager, enroll, no support:%d", ret_val);
                goto DO_FAIL;
        }
    }

DO_OUT:
    tp_ret = p_manager->p_tp_event_manager->SetFpEnable(p_manager,false);
    if(ANC_OK != tp_ret) {
        ANC_LOGE("enroll setFp false failed");
        goto DO_FAIL;
    }

    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);

    ret_val = GetAllFingerprintsId(finger_id_array, sizeof(finger_id_array)/sizeof(finger_id_array[0]), &finger_id_count);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get all fingerprints id,  ret val:%d", ret_val);
        goto DO_FAIL;
    }
    ANC_LOGW("After Enroll finish finger count is :%d", finger_id_count);

    AncMemcpy(&sync_array[1], finger_id_array, sizeof(finger_id_array));
    for(uint32_t i = 0; i< finger_id_count; i++) {
        ANC_LOGW("After Enroll finger id is :%d", finger_id_array[i]);
    }

    pre_enroll_start_time = AncGetElapsedRealTimeMs();
    p_device->fpe_worker_callback.OnExcuteCommand(p_device,
                                    EXTENSION_COMMAND_CB_SYNC_TEMPLATES, 0, (const uint8_t *)sync_array, (1+finger_id_count)*sizeof(uint32_t));

    return;

DO_FAIL:
    tp_ret= p_manager->p_tp_event_manager->SetFpEnable(p_manager,false);
    if(ANC_OK != tp_ret) {
        ANC_LOGE("enroll setFpEnable DO_FAIL failed");
    }

    ANC_LOGE("fail to enroll, ret value:%d", ret_val);
    switch (ret_val) {
        case ANC_CAPTURE_READ_SIZE_TOO_LARGE:
        case ANC_CAPTURE_BUFFER_NOT_ENOUGH:
        case ANC_ENROLL_FAIL:
        case ANC_ALGO_ENROLL_GET_TEMPLATE_FAIL:
            ExtCommandCbOnTouchUp(p_manager);
            p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            break;
        case ANC_FAIL_CANCEL:
            ANC_LOGE("enroll to be canceled");
            break;
        case ANC_ENROLL_TIME_OUT:
            ANC_LOGE("enroll, time out:%d seconds", p_manager->p_producer->enroll_timeout_second);
            p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_TIMEOUT);
            break;
        case ANC_FAIL_TA_TRANSMIT:
            kill(getpid(), SIGKILL);
            break;
        default:
            p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_HW_UNAVAILABLE);
            break;
    }

    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);

    if (is_need_deinit_enroll) {
        if (ANC_OK != (ret_val = AlgoDeinitEnroll(&finger_id,ANC_FALSE))) {
            ANC_LOGE("fail to release enroll, ret value:%d", ret_val);
        }
    }
}

static int FpmEnroll(AncFingerprintManager *p_manager,
                     const AncHwAuthToken *p_hat, uint32_t gid, uint32_t timeout_sec) {
    int ret_val = ANC_OK;

    ANC_LOGD("[FPM] Enroll");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    if (gid == p_manager->p_producer->current_group_id) {
        AncMemcpy((void *)&p_manager->p_producer->auth_token, (void *)p_hat,
               sizeof(p_manager->p_producer->auth_token));
#ifdef ANC_ENROLL_TIMEOUT_SECOND
        ANC_UNUSED(timeout_sec);
        p_manager->p_producer->enroll_timeout_second = ANC_ENROLL_TIMEOUT_SECOND; // 10 minutes
        ANC_LOGD("enroll, actual use timeout:%d seconds", p_manager->p_producer->enroll_timeout_second);
#else
        p_manager->p_producer->enroll_timeout_second = timeout_sec;
#endif
        FpmInnerCancel(p_manager);
        p_manager->p_producer->PushTaskToConsumer(p_manager, DoEnroll, (void *)p_manager,
                                                    (uint8_t *)"DoEnroll");
    } else {
        ANC_LOGE("gid:%d, current group id:%d", gid, p_manager->p_producer->current_group_id);
    }
    pthread_mutex_unlock(&p_manager->p_producer->mutex);

    return ret_val;
}

static void DoPostEnroll(void *p_arg) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    uint64_t challenge = 0;
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ret_val = GetEnrollChallenge(&challenge);
    if (ANC_OK == ret_val) {
        p_manager->p_producer->challenge = challenge;
    } else {
        p_manager->p_producer->challenge = 0;
    }
    ANC_LOGD("manager, get challenge : %llu", (unsigned long long)p_manager->p_producer->challenge);
}

static int FpmPostEnroll(AncFingerprintManager *p_manager) {
    int ret_val = ANC_OK;

    ANC_LOGD("[FPM] PostEnroll");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    DoPostEnroll((void *)p_manager);

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    ANC_LOGD("manager, post enroll will return challenge : %llu", (unsigned long long)p_manager->p_producer->challenge);
    return ret_val;
}

static uint64_t FpmGetAuthenticatorId(AncFingerprintManager *p_manager) {
    ANC_LOGD("[FPM] GetAuthenticatorId");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    uint64_t authenticator_id = 0;

    if (ANC_OK == GetAuthenticatorId(&authenticator_id)) {
        p_manager->p_producer->authenticator_id = authenticator_id;
    }

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    ANC_LOGD("manager, get authenticator id : %llu", (unsigned long long)authenticator_id);

    return authenticator_id;
}

static int FpmInnerCancel(AncFingerprintManager *p_manager) {
    int ret_val = ANC_OK;

    ANC_LOGD("[FPM] Inner Cancel");
    g_cancel = ANC_TRUE;
    g_cancel_heart_rate_detect = ANC_TRUE;
    p_manager->p_tp_event_manager->SendCondSignal(p_manager);
    p_manager->p_hbm_event_manager->SendCondSignal(p_manager);
    p_manager->p_consumer->GotoIdle(p_manager);
    g_cancel = ANC_FALSE;

    return ret_val;
}

static int FpmCancel(AncFingerprintManager *p_manager) {
    int ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;

    ANC_LOGD("[FPM] Cancel");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    ret_val = FpmInnerCancel(p_manager);

    p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_CANCELED);

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    return ret_val;
}
#if 0
static void DoSyncTemplates(void *p_arg,uint32_t *finger_id_array ,uint32_t finger_id_count ) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    uint32_t sync_array[6];
    uint32_t groupId = p_manager->p_producer->current_group_id;

    AncMemset(sync_array, 0, sizeof(sync_array));
    sync_array[0] = groupId;

    ANC_LOGW("SyncTemplates groupId is :%d", sync_array[0]);
    ANC_LOGW("SyncTemplates finger count is :%d", finger_id_count);

    for(uint32_t i = 0; i< finger_id_count; i++) {
        sync_array[i+1] = finger_id_array[i];
        ANC_LOGW("After SyncTemplates finger id is :%d", finger_id_array[i]);
        ANC_LOGW("After SyncTemplates sync_array is :%d", sync_array[i+1]);
    }

    p_device->fpe_worker_callback.OnExcuteCommand(p_device,
                                        EXTENSION_COMMAND_CB_SYNC_TEMPLATES, 0, (const uint8_t *)sync_array, (1+finger_id_count)*sizeof(uint32_t));
}
#endif
static void DoEnumerate(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    uint32_t id_array[5];
    uint32_t id_count = 0;

    uint32_t finger_id = 0;
    uint32_t group_id = p_manager->p_producer->current_group_id;
    uint32_t remaining = 0;

    AncMemset(id_array, 0, sizeof(id_array));
    ret_val = GetAllFingerprintsId(id_array, sizeof(id_array)/sizeof(id_array[0]), &id_count);
    if (ANC_OK == ret_val) {
        if (0 == id_count) {
            finger_id = 0;
            remaining = 0;
            p_manager->p_producer->OnEnumerate(p_device,
                                         finger_id, group_id, remaining);
        } else {
            for (uint32_t i=0; i<id_count; i++) {
                finger_id = id_array[i];
#ifdef ANC_REPORT_REMAIN_NUMBER
                remaining = id_count-i-1;
#else
                remaining = id_count;
#endif
                p_manager->p_producer->OnEnumerate(p_device,
                                         finger_id, group_id, remaining);
            }
        }
        //DoSyncTemplates(p_arg,id_array,id_count);
    } else {
        ANC_LOGE("fail to get all fingerprints id,  ret val:%d", ret_val);
        p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
    }
}

static int FpmEnumerate(AncFingerprintManager *p_manager) {
    int ret_val = ANC_OK;

    ANC_LOGD("[FPM] Enumerate");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    DoEnumerate((void *)p_manager);

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    return ret_val;
}

static ANC_RETURN_TYPE RemoveFinger(AncFingerprintManager *p_manager,
                        uint32_t finger_id, uint32_t remaining) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;

    ret_val = DeleteFingerprint(finger_id);
    if (ANC_OK == ret_val) {
        p_manager->p_producer->OnRemoved(p_device, finger_id,
                                   p_manager->p_producer->current_group_id, remaining);
    } else {
        ANC_LOGE("fail to delete fingerprint, finger id:%d, ret val:%d", finger_id, ret_val);
    }

    return ret_val;
}

static void DoRemove(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    uint32_t finger_id_array[5];
    uint32_t finger_id_count = 0;
    uint32_t sync_array[6];
    uint32_t groupId = p_manager->p_producer->current_group_id;
    int32_t scaling_time = ANC_SCALE_CPU_FREQUENCY_TIMEOUT_1000;

    AncMemset(finger_id_array, 0, sizeof(finger_id_array));
    AncMemset(sync_array, 0, sizeof(sync_array));
    sync_array[0] = groupId;

    if (ANC_OK == p_manager->p_external_feature_manager->DoWork1(
                        p_manager->p_external_feature_manager->p_device,
                        ANC_EXTERNAL_BIND_CORE)) {
        ANC_LOGD("remove, bind cpu core");
    }

    ret_val = GetAllFingerprintsId(finger_id_array, sizeof(finger_id_array)/sizeof(finger_id_array[0]), &finger_id_count);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get all fingerprints id,  ret val:%d", ret_val);
        goto DO_FAIL;
    }

    ANC_LOGD("Before remove finger finger count is :%d", finger_id_count);
    for(uint32_t i = 0; i< finger_id_count; i++) {
        ANC_LOGD("Before remove finger id is :%d", finger_id_array[i]);
    }
    ANC_LOGD("need remove finger id is :%d", p_manager->p_producer->remove_finger_id);
    if (0 == finger_id_count) {
        p_manager->p_producer->OnRemoved(p_device, p_manager->p_producer->remove_finger_id, groupId, 0);
    } else {
        if (ANC_OK == p_manager->p_external_feature_manager->DoWork3(
                            p_manager->p_external_feature_manager->p_device,
                            ANC_EXTERNAL_SCALE_CPU_FREQUENCY,
                            (uint8_t *)(&scaling_time), sizeof(scaling_time))) {
            ANC_LOGD("scaling time : %d ms", scaling_time);
        }
        if (0 == p_manager->p_producer->remove_finger_id) {
            for (uint32_t i = 0; i < finger_id_count; i++) {
                ret_val = RemoveFinger(p_manager, finger_id_array[i], (finger_id_count - 1 - i));
                if (ANC_OK != ret_val) {
                    ANC_LOGE("fail to remove fingerprint, ret val:%d", ret_val);
                    goto DO_FAIL;
                }
            }
        } else {
            ret_val = RemoveFinger(p_manager, p_manager->p_producer->remove_finger_id, 0);
            if (ANC_OK != ret_val) {
                ANC_LOGE("fail to remove fingerprint, ret val:%d", ret_val);
                goto DO_FAIL;
            }
        }
    }

    ret_val = GetAllFingerprintsId(finger_id_array, sizeof(finger_id_array)/sizeof(finger_id_array[0]), &finger_id_count);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get all fingerprints id,  ret val:%d", ret_val);
        goto DO_FAIL;
    }
    ANC_LOGW("After remove finger count is :%d", finger_id_count);

    AncMemcpy(&sync_array[1], finger_id_array, sizeof(finger_id_array));
    for(uint32_t i = 0; i< finger_id_count; i++) {
        ANC_LOGW("After remove finger id is :%d", finger_id_array[i]);
    }

    p_device->fpe_worker_callback.OnExcuteCommand(p_device,
                                    EXTENSION_COMMAND_CB_SYNC_TEMPLATES, 0, (const uint8_t *)sync_array, (1+finger_id_count)*sizeof(uint32_t));

    return ;


DO_FAIL :
    p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_UNABLE_TO_REMOVE);

    // ret_val = TemplateLoadDatabase();
    // if (ANC_OK != ret_val) {
    //     ANC_LOGE("manager, load data base error %d", ret_val);
    // }
}

static int FpmRemove(AncFingerprintManager *p_manager,
                     uint32_t gid, uint32_t fid) {
    int ret_val = ANC_OK;

    ANC_UNUSED(gid);

    ANC_LOGD("[FPM] Remove");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    if (gid == p_manager->p_producer->current_group_id) {
        p_manager->p_producer->remove_finger_id = fid;
        DoRemove((void *)p_manager);
    } else {
        ANC_LOGE("gid:%d, current group id:%d", gid, p_manager->p_producer->current_group_id);
    }

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    return ret_val;
}

#ifdef FP_JIIOV_TEMPLATE_UPDATE_ENABLE
static ANC_RETURN_TYPE UpdateTaFile(AncFingerprintManager *p_manager, uint32_t gid, char *path) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncExtensionUpdateFileInfo update_info;

    AncMemset(&update_info, 0, sizeof(update_info));
    update_info.gid = gid;
    size_t store_path_len = strlen(p_manager->p_producer->current_database_path);

    if (store_path_len >= sizeof(update_info.path)) {
        ANC_LOGE("manager, fail to update ta file, src path size:%d, dest path size:%d",
                (uint32_t)store_path_len, (uint32_t)sizeof(update_info.path));
        return ANC_FAIL_MEMORY;
    }
    update_info.path_size = (uint32_t)store_path_len;
    AncMemcpy(update_info.path, path, store_path_len);
    ret_val = ExtensionUpdateFile(&update_info);
    if (ANC_OK != ret_val) {
        ANC_LOGE("manager, fail to update ta file, error:%d", ret_val);
    }

    return ret_val;
}

#define FP_UPDATE_FALG    "persist.vendor.fp.template_updateflag"

bool check_update_flag() {
    int ret = 0;

    ANC_LOGI("enter check_update_flag start");

    char update_property[PROPERTY_VALUE_MAX] = {0};
    ret = property_get(FP_UPDATE_FALG, update_property, "0");
    ANC_LOGI("property:%s", update_property);
    ANC_LOGI("check_update_flag 1009, property_get =%d, errno =%d \n", ret, errno);
    if(strncmp(update_property, "V1", sizeof("V1")) == 0){
        ANC_LOGI("NO Need fingerprint template file fts write update");
        return false;
    } else {
        ANC_LOGI("Need fingerprint template file fts write update");
        property_set(FP_UPDATE_FALG, "V1");
        return true;
    }
}
#endif

static ANC_RETURN_TYPE DoSetActiveGroup(void *p_arg) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;

    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint64_t authenticator_id = 0;

    if (ANC_OK == p_manager->p_external_feature_manager->DoWork1(
                        p_manager->p_external_feature_manager->p_device,
                        ANC_EXTERNAL_BIND_CORE)) {
        ANC_LOGD("SetActiveGroup, bind cpu core");
    }

    int32_t scaling_time = ANC_SCALE_CPU_FREQUENCY_TIMEOUT_2000;

    if (ANC_OK == p_manager->p_external_feature_manager->DoWork3(
                        p_manager->p_external_feature_manager->p_device,
                        ANC_EXTERNAL_SCALE_CPU_FREQUENCY,
                        (uint8_t *)(&scaling_time), sizeof(scaling_time))) {
        ANC_LOGD("scaling time : %d ms", scaling_time);
    }

    ANC_LOGD("manager, set active group, group_id = %d", p_manager->p_producer->current_group_id);
#ifdef FP_JIIOV_TEMPLATE_UPDATE_ENABLE
    if (check_update_flag() == true) {
        DIR *dir;
        if ((dir=opendir("/data/vendor_de/")) == 0) {
            ANC_LOGE("opendir error");
            return  ANC_FAIL;
        } else {
            ANC_LOGI("opendir success");
        }

        struct dirent *stdinfo;
        while(1) {
            if ((stdinfo = readdir(dir)) == 0) {
                ANC_LOGI("stdinfo is 0");
                break;
            }
            ANC_LOGI("input name:%s, type:%d", stdinfo->d_name, stdinfo->d_type);

            if ((stdinfo->d_type != 4) ||
                (strcmp(stdinfo->d_name, ".") == 0) ||
                (strcmp(stdinfo->d_name, "..") == 0) ||
                (strcmp(stdinfo->d_name, "rpmb_status") == 0)) {
                continue;
            }

            ANC_LOGI("filter name:%s, type:%d", stdinfo->d_name, stdinfo->d_type);
            ANC_LOGI("dir_num:%d, type:%d", atoi(stdinfo->d_name), stdinfo->d_type);
            char path[256] = "";
            strcat(path, "/data/vendor_de/");
            strcat(path, stdinfo->d_name);
            strcat(path, "/fpdata/");

            ANC_LOGI("path:%s, type:%d", path, stdinfo->d_type);
            ret_val = UpdateTaFile(p_manager, (uint32_t)atoi(stdinfo->d_name), path);
            if (ANC_OK != ret_val) {
                ANC_LOGE("UpdateTaFile fail! error %d", ret_val);
                property_set(FP_UPDATE_FALG, "0");
            }
        }

        closedir(dir);
        ANC_LOGI("closedir over");
    }
#endif

    ret_val = TemplateSetActiveGroup(p_manager->p_producer->current_group_id,
                                    p_manager->p_producer->current_database_path);
    if (ANC_OK != ret_val) {
        ANC_LOGE("manager, set active group error %d", ret_val);
        goto out;
    }

    ret_val = TemplateLoadDatabase();
    if (ANC_OK != ret_val) {
        ANC_LOGE("manager, load data base error %d", ret_val);
        goto out;
    }

    ret_val = GetAuthenticatorId(&authenticator_id);
    if (ANC_OK != ret_val) {
        ANC_LOGE("manager, get auth id error %d", ret_val);
        goto out;
    }

    p_manager->p_producer->authenticator_id = authenticator_id;

#ifdef ANC_ACTIVE_SYNC_TEMPLATE
    ANC_LOGD("active sync template");
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    uint32_t sync_array[6] = {0};
    uint32_t finger_id_count = 0;

    sync_array[0] = p_manager->p_producer->current_group_id;
    if (ANC_OK == GetAllFingerprintsId(&sync_array[1], (sizeof(sync_array) / sizeof(sync_array[0])) - 1, &finger_id_count)) {
        p_device->fpe_worker_callback.OnExcuteCommand(p_device,
                    18, 0, (const uint8_t *)sync_array, (1+finger_id_count)*sizeof(uint32_t));
    } else {
        ANC_LOGE("failed sync template");
    }
#endif

out:
    if(ret_val) {
        p_manager->p_producer->authenticator_id = 0;//no template
    }

    return ret_val;
}

static int FpmSetActiveGroup(AncFingerprintManager *p_manager,
                     uint32_t gid, const char *p_store_path) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("[FPM] SetActiveGroup");

    if (p_store_path == NULL) {
        ANC_LOGE("setactivegroup, store path is NULL");
        return ANC_FAIL;
    } else if (strcmp(p_store_path, "/") == 0) {  // for VTS can pass
        ANC_LOGE("setactivegroup, store path is an unwritable location.");
        return ANC_FAIL;
    }

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    p_manager->p_producer->current_group_id = gid;
    if((strlen(p_store_path) + 1) > ANC_PATH_MAX) {
        ANC_LOGE("manager, store path is valid %s", p_store_path);
    }
    AncMemset(p_manager->p_producer->current_database_path, 0, ANC_PATH_MAX);
    AncMemcpy(p_manager->p_producer->current_database_path, p_store_path, strlen(p_store_path) + 1);

    ret_val = DoSetActiveGroup(p_manager);
    if (ret_val != ANC_OK) {
        ANC_LOGE("manager, setactivegroup failed, ret_val:%d", ret_val);
    }

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    return (int)ret_val;
}

typedef struct fingerprint_setuxthread
{
    int32_t pid;
    int32_t tid;
    uint8_t enable;
} fingerprint_setuxthread_t;

#ifdef FP_SET_UXTREAD
static void setUxThread(AncFingerprintManager *p_manager, uint8_t enable) {
    fingerprint_setuxthread_t  ANC_UXTHREADDATA;
    ANC_UXTHREADDATA.pid = getpid();
    ANC_UXTHREADDATA.tid = gettid();
    ANC_UXTHREADDATA.enable = enable;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;
    p_device->fp_external_worker.DoWork(p_device, ANC_SETUXTHREAD, (uint8_t*)(&ANC_UXTHREADDATA), sizeof(ANC_UXTHREADDATA));
    ANC_LOGD("authenticate, set thread UIfirst");
}
#endif

#ifdef ANC_DEBUG
static void PrintAuthenticateSummaryResult(const char *p_result, uint32_t retry_count) {

    if(NULL == p_result) {
        ANC_LOGE("error:info ptr is null!");
        return;
    }

    ANC_LOGD("authenticate result:%s spent:%lldms retry0:%s %lld+%lld+%lld=%lldms retry1:%s %lld+%lld+%lld=%lldms retry2:%s %lld+%lld+%lld=%lldms",
            p_result,
            g_time_info.verify_time_all,
            (retry_count == 0) ? "yes" : "no",
            g_time_info.capture_time[0],g_time_info.extract_time[0],g_time_info.verify_time[0],g_time_info.capture_time[0]+g_time_info.extract_time[0]+g_time_info.verify_time[0],
            (retry_count >= 1) ? "yes" : "no",
            g_time_info.capture_time[1],g_time_info.extract_time[1],g_time_info.verify_time[1],g_time_info.capture_time[1]+g_time_info.extract_time[1]+g_time_info.verify_time[1],
            (retry_count >= 2) ? "yes" : "no",
            g_time_info.capture_time[2],g_time_info.extract_time[2],g_time_info.verify_time[2],g_time_info.capture_time[2]+g_time_info.extract_time[2]+g_time_info.verify_time[2]
            );
}
#endif

static void DoAuthenticate(void *p_arg) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;

    int32_t scaling_time = ANC_SCALE_CPU_FREQUENCY_TIMEOUT_2000;

    uint32_t finger_id = 0;
    uint32_t need_study = 0;
    uint32_t group_id = p_manager->p_producer->current_group_id;
    uint32_t ret_val = ANC_OK;
    uint32_t algo_status = ANC_OK;
    uint32_t tp_ret = ANC_OK;
    uint32_t retry_count = 0;
    oplus_fingerprint_auth_ta_info_t jiiov_algo_info;
#ifdef ANC_QUICKLY_PICK_UP_FINGER
    uint32_t retry_finger_up_check = ANC_OK;
#endif
    // uint32_t authenticate_retry = 0;
    AncHwAuthToken auth_token;
    ANC_BOOL is_need_deinit_verify = ANC_FALSE;
    int32_t retry0_auto_expo_time = 10000;  // 10ms
    uint32_t retry2_expo_ratio = 0;
    ANC_SENSOR_INNER_DATA_MODE sensor_capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT;
    ANC_BOOL is_screen_off = ANC_FALSE;
    AncMemset(&g_time_info, 0, sizeof(AncFTAlgoTimeInfo));

#ifdef FP_SET_PRIORITY
    setpriority(PRIO_PROCESS, 0, ANC_FINGERPRINT_PROCESS_PRIORITY_DEFAULT);
#endif

    if (ANC_OK == p_manager->p_external_feature_manager->DoWork1(
                        p_manager->p_external_feature_manager->p_device,
                        ANC_EXTERNAL_BIND_CORE)) {
        ANC_LOGD("authenticate, bind cpu core");
    }

#ifdef ANC_USE_HW_AUTH_TOKEN
    ret_val = SetAuthenticateChallenge(p_manager->p_producer->challenge);
    if (ANC_OK != ret_val) {
        goto DO_FAIL;
    }
#endif
    if (ANC_OK != (ret_val = AlgoInitVerify())) {
        goto DO_FAIL;
    }
    is_need_deinit_verify = ANC_TRUE;

    ANC_LOGD("auth clear TP flag");
    if (ANC_OK != (ret_val = SensorClearTPFlag())) {
        ANC_LOGE("auth fail to clear tp flag");
    }

    tp_ret = p_manager->p_tp_event_manager->SetFpEnable(p_manager,true);
    if(ANC_OK != tp_ret) {
        ANC_LOGE("auth setFpEnable failed");
        goto DO_FAIL;
    }

    while (ANC_TRUE) {
        if(g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        // ret_val = p_manager->p_hbm_event_manager->SetHbm(p_manager, true);
        // if(ANC_OK != ret_val) {
        //     ANC_LOGE("auth set hbm  failed");
        //     goto DO_FAIL;
        // }

        p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_DETECTED);

#ifdef USE_SLEEP_REPLACE_WAIT_TP_EVENT
        sleep(1); // waiting for 1 second
#else
        ATRACE_BEGIN("verify_wait_touch_down");
        if (g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }
        ret_val = p_manager->p_tp_event_manager->WaitTouchDown(p_manager, WAIT_TOUCH_DOWN_TIME_OUT);
        ATRACE_END();
        switch (ret_val) {
            case ANC_OK :
                p_manager->p_producer->AuthScreenState = p_manager->p_tp_event_manager->IsScreenOn();
#ifdef FP_SET_UXTREAD
                setUxThread(p_manager, 1);
#endif
                break;
            case ANC_FAIL_WAIT_TIME_OUT :
                continue;
            default :
                goto DO_FAIL;
        }

#endif

        ANC_TIME_MEASURE_START(authtotaltime);
        ANC_TIME_MEASURE_START(UIreadytime);

        if(g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        if (ANC_OK == p_manager->p_external_feature_manager->DoWork3(
                            p_manager->p_external_feature_manager->p_device,
                            ANC_EXTERNAL_SCALE_CPU_FREQUENCY,
                            (uint8_t *)(&scaling_time), sizeof(scaling_time))) {
            ANC_LOGD("scaling time : %d ms", scaling_time);
        }

        p_manager->p_hbm_event_manager->SetHbmEventType(p_manager, HBM_EVENT_INVALID_TYPE);
#ifdef ANC_QUICKLY_PICK_UP_FINGER
        p_manager->p_tp_event_manager->enable_touch_up_report = ANC_FALSE;
#endif
        ExtCommandCbOnTouchDown(p_manager);

        if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP_SCAN))) {
            goto DO_FAIL;
        }

        is_screen_off = g_is_screen_off = p_manager->p_tp_event_manager->IsScreenOff();
        ANC_LOGD("is_screen_off = %d", is_screen_off);
#ifdef ANC_WAIT_HBM_READY
WAIT_HBM:
        if (g_cancel == ANC_TRUE) {
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }
        if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
            ExtCommandCbOnTouchUp(p_manager);
            p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_TOO_FAST_NO_IMGINFO;
            p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_FAST);
            p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
            continue;
        }
        //wait HBM ready signal, 600ms
        ATRACE_BEGIN("verify_wait_hbm_ready");
        ret_val = p_manager->p_hbm_event_manager->WaitHBMReady(p_manager, WAIT_HBM_TIME_OUT);
        ATRACE_END();
        switch (ret_val) {
            case ANC_OK :
                break;
            case ANC_FAIL_WAIT_TIME_OUT :
                ANC_LOGE("authenticate wait hbm ready time out, ret value: %d", ret_val);
                // p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_HBM_TIMEOUT);
                goto WAIT_HBM;
            default :
                goto DO_FAIL;
        }
        if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
            ExtCommandCbOnTouchUp(p_manager);
            p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_FAST);
            p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
            continue;
        }
#endif
        ANC_GET_TIME_MEASURE_END(UIreadytime, "UIready", &g_time_info.UIready_time);

        if(g_cancel == ANC_TRUE) {
            ExtCommandCbOnTouchUp(p_manager);
            ret_val = ANC_FAIL_CANCEL;
            goto DO_FAIL;
        }

        ANC_TIME_MEASURE_START(verify_all);
        for (retry_count = 0; retry_count < ONE_TOUCH_EVENT_RETRY_COUNT; retry_count++) {
            p_manager->p_producer->retry_count = retry_count;
            ANC_LOGD("authenticate retry :%d", retry_count);
            if(g_cancel == ANC_TRUE) {
                ret_val = ANC_FAIL_CANCEL;
                goto DO_FAIL;
            }

            switch (retry_count) {
                case 0:
                    sensor_capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT_EXPOSURE | ANC_SENSOR_SKIP_SCAN_CONFIG;
                    break;
                case 1:
                    sensor_capture_mode = ANC_SENSOR_TRANSMIT_EXPOSURE;
                    break;
                case 2:
                    sensor_capture_mode = ANC_SENSOR_TRANSMIT;
                    break;
                default:
                    sensor_capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT;
                    break;
            }

            if ((retry_count == 1) && SensorIsAbnormalExpo()) {
                ANC_LOGD("retry1 has been captured!");
            } else {
                if (retry_count == 1) {
                    // set retry2 expo time
                    if (retry2_expo_ratio != 0) {
                        ret_val = SensorSetRetryExposureTime(0, AUTO_EXPOSURE_TIME, AUTO_EXPOSURE_TIME, (int32_t)(retry0_auto_expo_time * (int32_t)retry2_expo_ratio / 100));
                        if(ANC_OK != ret_val) {
                            goto DO_FAIL;
                        }
                    }
                }

                ATRACE_BEGIN("verify_capture_image");
                ANC_TIME_MEASURE_START(verify_capture);
                ret_val = SensorCaptureImageWithMode(sensor_capture_mode, (retry_count == 0 ? &retry0_auto_expo_time : NULL));
                ANC_GET_TIME_MEASURE_END(verify_capture, "verify capture image",&g_time_info.capture_time[retry_count]);
                ATRACE_END();
#ifdef ANC_QUICKLY_PICK_UP_FINGER
                retry_finger_up_check = CheckQuicklyPickupFinger(p_manager, retry_count);
                if ((ANC_CAPTURE_FINGER_MOVE_TOO_FAST == retry_finger_up_check)
                    || (ANC_ALGO_MATCH_FAIL == retry_finger_up_check)) {
                    ret_val = retry_finger_up_check;
                    break;
                }
#endif
                if (ANC_OK != ret_val) {
                    ANC_LOGE("fail to capture image, ret value:%d, line %d", ret_val, __LINE__);
                    break;
                }
            }

            if (g_cancel == ANC_TRUE) {
                ret_val = ANC_FAIL_CANCEL;
                goto DO_FAIL;
            }

            // extract feature
            ATRACE_BEGIN("verify_extract_feature");
            ANC_TIME_MEASURE_START(extract_feature);
            int32_t abnormal_exp_val = (SensorIsAbnormalExpo()) ? 1: 0;
            ret_val = AlgoVerifyExtractFeature(retry_count, abnormal_exp_val, &retry2_expo_ratio);
            ANC_GET_TIME_MEASURE_END(extract_feature, "extract feature",&g_time_info.extract_time[retry_count]);
            ATRACE_END();
            if (ANC_ALGO_BAD_IMG == ret_val) {
                ANC_LOGE("extract bad image, ret value:%d", ret_val);
                ret_val = ANC_ALGO_MATCH_FAIL;
                algo_status = ANC_ALGO_MATCH_ALGO_FAIL;
                continue;
            } else if (ANC_ALGO_EXTRACT_RETRY_FAST_RET == ret_val) {
                ANC_LOGE("extract fast ret, ret value:%d", ret_val);
                ret_val = ANC_ALGO_MATCH_FAIL_STOP_RETRY;
                algo_status = ANC_ALGO_MATCH_ALGO_FAIL;
                break;
            } else if (ANC_ALGO_EXTRACT_OK != ret_val) {
                ANC_LOGE("fail to extract feature, ret value:%d", ret_val);
                goto DO_FAIL;
            }

            if (
#ifdef ANC_QUICKLY_PICK_UP_FINGER
                (ANC_OK == retry_finger_up_check) &&
#endif
                (retry_count == 0) && SensorIsAbnormalExpo())
                {
                // retry1 capture image for HDR if abnormal exposure
                uint32_t retry1_capture_ret = ANC_OK;
                ATRACE_BEGIN("verify_capture_image");
                ANC_TIME_MEASURE_START(verify_capture);
                retry1_capture_ret = SensorCaptureImageWithMode(ANC_SENSOR_TRANSMIT_EXPOSURE, NULL);
                ANC_GET_TIME_MEASURE_END(verify_capture, "verify capture image",&g_time_info.capture_time[1]);
                ATRACE_END();
                if (ANC_OK != retry1_capture_ret) {
                    ret_val = retry1_capture_ret;
                    ANC_LOGE("fail to capture image, ret value:%d,line %d", ret_val, __LINE__);
                    break;
                }

#ifdef ANC_QUICKLY_PICK_UP_FINGER
                /* check finger up for retry1 capture, and cache the return value here */
                retry_finger_up_check = CheckQuicklyPickupFinger(p_manager, 1);
#endif
            }

            // compare feature
            if (ANC_ALGO_EXTRACT_OK == ret_val) {
                ATRACE_BEGIN("verify_compare_feature");
                ANC_TIME_MEASURE_START(compare_feature);
                ret_val = AlgoCompareFeature(&finger_id, &need_study, &algo_status);
#ifdef FP_SET_UXTREAD
                setUxThread(p_manager, 0);
#endif
                p_manager->p_producer->finger_id = finger_id;
                ANC_GET_TIME_MEASURE_END(compare_feature, "compare feature",&g_time_info.verify_time[retry_count]);
                ATRACE_END();
                ANC_LOGD("AlgoCompareFeature, ret = (%d,%d)", ret_val, algo_status);
                if ((ANC_ALGO_MATCH_OK == ret_val) || (ANC_ALGO_MATCH_FAIL_STOP_RETRY == ret_val)) {
                    break;
                }
            }

#ifdef ANC_QUICKLY_PICK_UP_FINGER
            if ((ANC_CAPTURE_FINGER_UP == retry_finger_up_check)
                || (ANC_ALGO_MATCH_FAIL == retry_finger_up_check)
                || IsFingerUp(p_manager)) {
                ret_val = ANC_ALGO_MATCH_FAIL;
                break;
            }
#endif
        }
        ANC_LOGD("verify, return type:%s, %d, algo_status: %d", AncConvertReturnTypeToString(ret_val), ret_val, algo_status);
        switch (ret_val) {
            case ANC_ALGO_MATCH_OK:
                ANC_LOGD("algo verify, match ok, need_study:%d", need_study);
                ANC_GET_TIME_MEASURE_END(verify_all, "match ok",&g_time_info.verify_time_all);
#ifdef ANC_DEBUG
                PrintAuthenticateSummaryResult("ok", retry_count);
#endif
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_GOOD);
#ifdef ANC_USE_HW_AUTH_TOKEN
                AncMemset(&auth_token, 0, sizeof(auth_token));
                if (ANC_OK == (ret_val = GetAuthenticateResult((uint8_t *)&auth_token, sizeof(auth_token)))) {
                    p_manager->p_producer->OnAuthenticated(p_device, finger_id, group_id, (const uint8_t *)&auth_token, sizeof(auth_token));
                    p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_SUCCESS;
                    ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                    jiiov_algo_info.fail_reason = ANC_ALGO_MATCH_OK;
                    ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                    ANC_LOGD("anc_hal_manager authtotaltime :%lld", g_time_info.authtotaltime);
                    ret_val = sendDcsAuthEventInfo(p_manager, &jiiov_algo_info);
                    goto DO_OUT;
                } else {
                    goto DO_FAIL;
                }
#else
                p_manager->p_producer->OnAuthenticated(p_device, finger_id, group_id, (const uint8_t *)&auth_token, sizeof(auth_token));
                goto DO_OUT;
#endif
            case ANC_ALGO_MATCH_FAIL_STOP_RETRY:
                ANC_LOGE("verify stop retry");
            case ANC_ALGO_MATCH_FAIL:
                switch (algo_status) {
                    case ANC_ALGO_MATCH_ALGO_FAIL:
                    case ANC_ALGO_MATCH_ALGO_PASS_AND_LIVE_FAILED:
                    case ANC_ALGO_MATCH_ALGO_FAIL_AND_LIVE_FAILED:
                    case ANC_ALGO_MATCH_ALGO_PASS_AND_GHOST_FAILED:
                    case ANC_ALGO_MATCH_ALGO_FAIL_AND_GHOST_FAILED:
                        ANC_LOGE("verify fail: %s", (algo_status == ANC_ALGO_MATCH_ALGO_FAIL) ? "default" : "fake");
                        ANC_GET_TIME_MEASURE_END(verify_all, "match fail", &g_time_info.verify_time_all);
#ifdef ANC_DEBUG
                        PrintAuthenticateSummaryResult("fail", retry_count);
#endif
                        p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_GOOD);
                        p_manager->p_producer->OnAuthenticated(p_device, 0, group_id, NULL, 0);
                        p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_FAIL;
                        ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                        jiiov_algo_info.fail_reason = (int32_t)ret_val;
                        ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                        ret_val = sendDcsAuthEventInfo(p_manager, &jiiov_algo_info);
                        p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                        continue;
                    case ANC_ALGO_MATCH_ALGO_FAIL_STRANGE:
                    case ANC_ALGO_MATCH_ALGO_FAIL_LOW_EXPO:
                    case ANC_ALGO_MATCH_ALGO_FAIL_HIGH_EXPO:
                    case ANC_ALGO_MATCH_ALGO_FAIL_PATTERN:
                        ANC_LOGE("verify fail: bad quality");
                        ANC_GET_TIME_MEASURE_END(verify_all, "match image bad quality",&g_time_info.verify_time_all);
                        p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_INSUFFICIENT);
                        p_manager->p_producer->OnAuthenticated(p_device, 0, group_id, NULL, 0);
                        if (is_screen_off) {
                            ExtCommandCbOnTouchUp(p_manager);
                        }
                        p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                        p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_FAIL;
                        ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                        jiiov_algo_info.fail_reason = (int32_t)ret_val;
                        ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                        ret_val = sendDcsAuthEventInfo(p_manager, &jiiov_algo_info);
                        continue;
                    case ANC_ALGO_MATCH_ALGO_FAIL_PARTIAL:
                        ANC_LOGE("verify fail: partial");
                        ANC_GET_TIME_MEASURE_END(verify_all, "match partial",&g_time_info.verify_time_all);
                        p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_PARTIAL);
                        p_manager->p_producer->OnAuthenticated(p_device, 0, group_id, NULL, 0);
                        if (is_screen_off) {
                            ExtCommandCbOnTouchUp(p_manager);
                            p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_OTHER_FAIL_REASON;
                        } else {
                            p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_FAIL;
                        }
                        p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                        ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                        jiiov_algo_info.fail_reason = (int32_t)ret_val;
                        ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                        ret_val = sendDcsAuthEventInfo(p_manager, &jiiov_algo_info);
                        continue;
                    default:
                        ANC_LOGE("verify unknown algo status: %d. treat as match fail", algo_status);
                        ANC_GET_TIME_MEASURE_END(verify_all, "match fail",&g_time_info.verify_time_all);
#ifdef ANC_DEBUG
                        PrintAuthenticateSummaryResult("fail", retry_count);
#endif
                        p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_GOOD);
                        p_manager->p_producer->OnAuthenticated(p_device, 0, group_id, NULL, 0);
                        p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                        continue;
                }
            case ANC_CAPTURE_IMAGE_CHECK_FAIL:
            case ANC_ALGO_BAD_IMG:
            case ANC_ALGO_LOW_QTY:
                ANC_GET_TIME_MEASURE_END(verify_all, "match low qty",&g_time_info.verify_time_all);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                if (is_screen_off) {
                    ExtCommandCbOnTouchUp(p_manager);
                }
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);

                p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_OTHER_FAIL_REASON;
                continue;
            case ANC_ALGO_PARITAL:
                ANC_GET_TIME_MEASURE_END(verify_all, "match partial",&g_time_info.verify_time_all);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_PARTIAL);
                if (is_screen_off) {
                    ExtCommandCbOnTouchUp(p_manager);
                }
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_OTHER_FAIL_REASON;
                ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                jiiov_algo_info.fail_reason = (int32_t)ret_val;
                ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                ret_val = sendDcsAuthEventInfo(p_manager, (oplus_fingerprint_auth_ta_info_t*)(&jiiov_algo_info));
                continue;
            case ANC_ALGO_FAKE_FINGER:
            case ANC_ALGO_GHOST_FINGER:
            case ANC_ALGO_EXTRACT_FAIL:
                ANC_GET_TIME_MEASURE_END(verify_all, "match others",&g_time_info.verify_time_all);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_INSUFFICIENT);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_OTHER_FAIL_REASON;
                ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                jiiov_algo_info.fail_reason = (int32_t)ret_val;
                ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                ret_val = sendDcsAuthEventInfo(p_manager, (oplus_fingerprint_auth_ta_info_t*)(&jiiov_algo_info));
                continue;
            case ANC_CAPTURE_FINGER_MOVE_TOO_FAST:
                ANC_GET_TIME_MEASURE_END(verify_all, "match finger move too fast",&g_time_info.verify_time_all);
                // ExtCommandCbOnTouchUp(p_manager);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_TOO_FAST);
                p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
                p_manager->p_producer->dcs_auth_result_type = DCS_AUTH_TOO_FAST_GET_IMGINFO;
                ExtensionDCSAuthResultCollect(&jiiov_algo_info);
                jiiov_algo_info.fail_reason = (int32_t)ret_val;
                ANC_GET_TIME_MEASURE_END(authtotaltime, "authtotaltime", &g_time_info.authtotaltime);
                ret_val = sendDcsAuthEventInfo(p_manager, (oplus_fingerprint_auth_ta_info_t*)(&jiiov_algo_info));
                continue;
            case ANC_CAPTURE_WAIT_IMAGE_TIMEOUT:
            case ANC_CAPTURE_IMAGE_CRC_ERROR:
            case ANC_CAPTURE_RD_UNDERFLOW:
                ANC_GET_TIME_MEASURE_END(verify_all, "match capture fail",&g_time_info.verify_time_all);
                p_manager->p_producer->OnAcquired(p_device, ANC_FINGERPRINT_ACQUIRED_IMAGER_DIRTY);
                /* It should be sensor issue, try to hw reset and re-init sensor */
                if (ANC_OK != p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_HW_RESET_INIT)) {
                    goto DO_FAIL;
                }
                continue;
            default:
                ANC_LOGE("algo verify have some errors, ret_val:%d", ret_val);
                goto DO_FAIL;
        }
    }

DO_OUT:
#ifdef ANC_QUICKLY_PICK_UP_FINGER
    if (!p_manager->p_tp_event_manager->enable_touch_up_report) {
        p_manager->p_tp_event_manager->enable_touch_up_report = ANC_TRUE;
        if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
            ExtCommandCbOnTouchUp(p_manager);
        }
    }
#endif
    tp_ret = p_manager->p_tp_event_manager->SetFpEnable(p_manager,false);
    if(ANC_OK != tp_ret) {
        ANC_LOGE("auth setFpEnable false failed");
        goto DO_FAIL;
    }

    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);

    if (is_need_deinit_verify) {
#ifdef ANC_ALGORITHM_STUDY_SWITCH
        ANC_BOOL study_flag = ANC_TRUE;
        char prop_learning[PROPERTY_VALUE_MAX] = {0};
        if(property_get("vendor.anc.fingerprint.learning", prop_learning, "true") != 0) {
            ANC_LOGD("prop learning = %s", prop_learning);
        }
        if (0 == strcmp(prop_learning, "false")) {
            study_flag = ANC_FALSE;
        } else {
            study_flag = ANC_TRUE;
        }
        need_study = study_flag && need_study;
        ANC_LOGD("need study : %d", need_study);
#endif
        if (need_study) {
            ATRACE_BEGIN("verify_study");
            ANC_TIME_MEASURE_START(Study);
            AlgoFeatureStudy(finger_id);
            ANC_GET_TIME_MEASURE_END(Study, "verify study",&g_time_info.study_time);
            ATRACE_END();
        }
        AlgoDeinitVerify();
    }
    return;

DO_FAIL:
#ifdef ANC_QUICKLY_PICK_UP_FINGER
    if (!p_manager->p_tp_event_manager->enable_touch_up_report) {
        p_manager->p_tp_event_manager->enable_touch_up_report = ANC_TRUE;
        if (p_manager->p_tp_event_manager->IsTpTouchUp()) {
            ExtCommandCbOnTouchUp(p_manager);
        }
    }
#endif
    tp_ret = p_manager->p_tp_event_manager->SetFpEnable(p_manager,false);
    if(ANC_OK != tp_ret) {
        ANC_LOGE("auth setFpEnable DO_FAIL failed");
    }

    switch (ret_val) {
        case ANC_CAPTURE_READ_SIZE_TOO_LARGE:
            p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            break;
        case ANC_CAPTURE_BUFFER_NOT_ENOUGH:
            p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_UNABLE_TO_PROCESS);
            break;
        case ANC_FAIL_CANCEL:
            ANC_LOGE("authenticate to be canceled");
            break;
        case ANC_FAIL_TA_TRANSMIT:
            kill(getpid(), SIGKILL);
            break;
        default:
            p_manager->p_producer->OnError(p_device, ANC_FINGERPRINT_ERROR_HW_UNAVAILABLE);
            break;
    }

    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);

    if (is_need_deinit_verify) {
        AlgoDeinitVerify();
    }
}

static int FpmAuthenticate(AncFingerprintManager *p_manager,
                     uint64_t operation_id, uint32_t gid) {
    int ret_val = ANC_OK;

    ANC_LOGD("[FPM] Authenticate");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    if (gid == p_manager->p_producer->current_group_id) {
        p_manager->p_producer->challenge = operation_id;
        FpmInnerCancel(p_manager);
        p_manager->p_producer->PushTaskToConsumer(p_manager, DoAuthenticate, (void *)p_manager,
                                                    (uint8_t *)"DoAuthenticate");
    } else {
        ANC_LOGE("gid:%d, current group id:%d", gid, p_manager->p_producer->current_group_id);
    }

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    return ret_val;
}

static int FpmExcuteCommand(AncFingerprintManager *p_manager,
          int32_t command_id, const uint8_t *p_in_param, uint32_t in_param_length,
          uint8_t **p_output_buffer, uint32_t *p_output_buffer_length) {
    int ret_val = ANC_OK;

    ANC_LOGD("[FPM] ExcuteCommand");

    pthread_mutex_lock(&p_manager->p_producer->mutex);

    AncMemset(&p_manager->p_producer->command, 0, sizeof(p_manager->p_producer->command));
    p_manager->p_producer->command.command_id = command_id;
    p_manager->p_producer->command.command.p_buffer = (uint8_t *)p_in_param;
    p_manager->p_producer->command.command.buffer_length = in_param_length;

    ret_val = (int)ExtensionCommandWork(p_manager, p_output_buffer, p_output_buffer_length);

    pthread_mutex_unlock(&p_manager->p_producer->mutex);
    return ret_val;
}

// --------   Callback   --------------
static void FpmCbOnEnrollResult(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining) {
    if (p_device != NULL) {
        ANC_LOGW("manager, on enroll result, finger_id: %d, remaining: %d", finger_id, remaining);
        p_device->fp_worker_callback.OnEnrollResult((void *)p_device,
                                         finger_id, group_id, remaining);
    }

}

static void FpmCbOnAcquired(AncFingerprintDevice *p_device, int32_t vendor_code) {
    if (p_device != NULL) {
        ANC_LOGW("manager, on acquired vendor_code: %d", vendor_code);
        p_device->fp_worker_callback.OnAcquired((void *)p_device, vendor_code);
    }
}

static void FpmCbOnAuthenticated(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                             const uint8_t *p_token, uint32_t token_length) {
    if (p_device != NULL) {
        ANC_LOGW("manager, on authenticated, finger_id: %d", finger_id);
        p_device->fp_worker_callback.OnAuthenticated(p_device,
                                         finger_id, group_id,
                                         p_token, token_length);
    }
}

static void FpmCbOnError(AncFingerprintDevice *p_device, int vendor_code) {
    if (p_device != NULL) {
        ANC_LOGW("manager, on error, vendor_code: %d", vendor_code);
        p_device->fp_worker_callback.OnError((void *)p_device, vendor_code);
    }
}

static void FpmCbOnRemoved(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining) {
    if (p_device != NULL) {
        ANC_LOGW("manager, on removed, finger_id: %d", finger_id);
        p_device->fp_worker_callback.OnRemoved((void *)p_device,
                                         finger_id, group_id, remaining);
    }
}

static void FpmCbOnEnumerate(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining) {

    if (p_device != NULL) {
        ANC_LOGW("manager, on enumerate, finger_id: %d", finger_id);
        p_device->fp_worker_callback.OnEnumerate(p_device,
                                         finger_id, group_id, remaining);
    }
}

static void FpmCbOnExcuteCommand(AncFingerprintDevice *p_device, int32_t command_id,
                                 int32_t argument, const uint8_t *p_out, uint32_t out_length) {

    if (p_device != NULL) {
        ANC_LOGW("manager, on excute command id: %d", command_id);
        p_device->fpe_worker_callback.OnExcuteCommand(p_device,
                                     command_id, argument, p_out, out_length);
    }
}

static void DoHeartBeatRateDetect(void *p_arg) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncFingerprintDevice *p_device = p_manager->p_producer->p_device;

    uint32_t ret_val = ANC_OK;
    int32_t capture_expo_time;
    ANC_SENSOR_INNER_DATA_MODE sensor_capture_mode = ANC_SENSOR_EXPOSURE_TRANSMIT;
    uint32_t capture_count = 0;
    uint32_t heart_rate_result[12] = {0};

    heart_rate_result[0] = HEARTBEAT_RATE_TOKEN_ERROR_CODE;
    heart_rate_result[1] = (uint32_t)ret_val;
    heart_rate_result[2] = HEARTBEAT_RATE_TOKEN_HEART_RATE_RESULT;
    heart_rate_result[3] = 0;
    heart_rate_result[4] = HEARTBEAT_RATE_TOKEN_HEART_RATE_DATA_NUM;
    heart_rate_result[5] = capture_count;
    heart_rate_result[6] = HEARTBEAT_RATE_TOKEN_HEART_RATE_DATA_PPG;
    heart_rate_result[7] = 2;
    heart_rate_result[8] = HEARTBEAT_RATE_TOKEN_HEART_RATE_DATA_PPG_ORIGN;
    heart_rate_result[9] = 9;
    heart_rate_result[10] = HEARTBEAT_RATE_TOKEN_HEART_RATE_TIME_SPEND;
    heart_rate_result[11] = 0;

    usleep(500000);  // delay 500ms

    if (p_manager->p_tp_event_manager->IsHeartRateFingerUp()) {
        ret_val = ANC_CAPTURE_FINGER_UP;
    } else if (g_cancel_heart_rate_detect == ANC_TRUE) {
        ret_val = ANC_FAIL_CANCEL;
    }

    if (ANC_OK != ret_val) {
        heart_rate_result[1]= (uint32_t)ret_val;
        heart_rate_result[5]= capture_count;
        p_device->fpe_worker_callback.OnExcuteCommand(p_device, EXTENSION_COMMAND_PRE_HEART_RATE_DETECT,
            0, (const uint8_t *)heart_rate_result, sizeof(heart_rate_result));
        return;
    }

    do {
        // enable hbm
        // ret_val = p_manager->p_hbm_event_manager->SetHbm(p_manager, true);
        // if(ANC_OK != ret_val) {
        //     ANC_LOGE("HBR set hbm failed");
        //     break;
        // }

        if (ANC_OK != (ret_val = p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_WAKEUP))) {
            break;
        }

        // set fixed exposure time
        if (ANC_OK != (ret_val = SensorSetRetryExposureTime(0, HEART_BEAT_EXPOSURE_TIME, HEART_BEAT_EXPOSURE_TIME, HEART_BEAT_EXPOSURE_TIME))) {
            break;
        }

        // set frame fusion number to 2,2,2
        if (ANC_OK != (ret_val = SensorSetFrameFusionNum(2, 2, 2))) {
            break;
        }

        if (p_manager->p_tp_event_manager->IsHeartRateFingerUp()) {
            ret_val = ANC_CAPTURE_FINGER_UP;
            break;
        }

        while (ANC_TRUE) {
            if (g_cancel_heart_rate_detect == ANC_TRUE) {
                ret_val = ANC_FAIL_CANCEL;
                break;
            }

            sensor_capture_mode = (capture_count == 0 ? ANC_SENSOR_EXPOSURE_TRANSMIT_EXPOSURE : ANC_SENSOR_TRANSMIT_EXPOSURE);

            // capture image
            ANC_TIME_MEASURE_START(hbr_capture);
            ret_val = SensorCaptureImageWithMode(sensor_capture_mode, &capture_expo_time);
            ANC_GET_TIME_MEASURE_END(hbr_capture, "HBR capture image",&g_time_info.capture_time[0]);
            if (ANC_OK != ret_val) {
                ANC_LOGE("fail to capture image, ret value:%d, line %d", ret_val, __LINE__);
                break;
            }

            if (p_manager->p_tp_event_manager->IsHeartRateFingerUp()) {
                ret_val = ANC_CAPTURE_FINGER_UP;
                break;
            }

            if (g_cancel_heart_rate_detect == ANC_TRUE) {
                ret_val = ANC_FAIL_CANCEL;
                break;
            }

            uint32_t result[2] = {0};
            ANC_TIME_MEASURE_START(get_hb_result);
            ret_val = AlgoGetHBResult(capture_count, &result[0], &result[1]);
            ANC_LOGD("AlgoGetHBResult, ret = %d, result = (%d,%d)", ret_val, result[0], result[1]);
            ANC_GET_TIME_MEASURE_END(get_hb_result, "get hb result",&g_time_info.extract_time[0]);
            if (ANC_OK != ret_val) {
                ANC_LOGE("fail to get hb result, ret value:%d, line %d", ret_val, __LINE__);
                break;
            }

            capture_count++;
            ANC_LOGD("HBR capture count :%d", capture_count);

            heart_rate_result[1]= (uint32_t)ret_val;
            heart_rate_result[3]= result[1];
            heart_rate_result[5]= capture_count;
            p_device->fpe_worker_callback.OnExcuteCommand(p_device, EXTENSION_COMMAND_PRE_HEART_RATE_DETECT,
                0, (const uint8_t *)heart_rate_result, sizeof(heart_rate_result));
        }
    } while (0);

    if (ANC_OK != ret_val) {
        heart_rate_result[1]= (uint32_t)ret_val;
        heart_rate_result[5]= capture_count;
        p_device->fpe_worker_callback.OnExcuteCommand(p_device, EXTENSION_COMMAND_PRE_HEART_RATE_DETECT,
            0, (const uint8_t *)heart_rate_result, sizeof(heart_rate_result));
    }

    // disable hbm
    // p_manager->p_hbm_event_manager->SetHbm(p_manager, false);

    // restore auto exposure
    SensorSetRetryExposureTime(0, AUTO_EXPOSURE_TIME, AUTO_EXPOSURE_TIME, AUTO_EXPOSURE_TIME);

    // restore frame fusion number to 2,4,4
    SensorSetFrameFusionNum(2, 4, 4);

    p_manager->p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_SLEEP);
}

static ANC_RETURN_TYPE FpmStartHeartBeatRateDetect(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("[FPM] FpmStartHeartBeatRateDetect");

    if (CONSUMER_STATUS_IDLE == p_manager->p_consumer->status) {
        p_manager->p_producer->PushTaskToConsumer(p_manager, DoHeartBeatRateDetect, (void *)p_manager,
                                                    (uint8_t *)"DoHeartBeatRateDetect");
    }

    return ret_val;
}

static ANC_RETURN_TYPE FpExternalWork1(AncFingerprintDevice *p_device, int32_t type) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (NULL == p_device) {
        ANC_LOGE("%s, p_device is NULL", __func__);
        return ANC_FAIL;
    }

    if (NULL == p_device->fp_external_worker.DoWork) {
        ANC_LOGE("%s, external work1 is NULL", __func__);
        return ANC_FAIL;
    }

    p_device->fp_external_worker.DoWork(p_device, type, NULL, 0);

    return ret_val;
}

static ANC_RETURN_TYPE FpExternalWork2(AncFingerprintDevice *p_device, const uint8_t *p_buffer, uint32_t buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (NULL == p_device) {
        ANC_LOGE("%s, p_device is NULL", __func__);
        return ANC_FAIL;
    }

    if (NULL == p_device->fp_external_worker.DoWork) {
        ANC_LOGE("%s, external work2 is NULL", __func__);
        return ANC_FAIL;
    }

    p_device->fp_external_worker.DoWork(p_device, -1, p_buffer, buffer_length);

    return ret_val;
}

static ANC_RETURN_TYPE FpExternalWork3(AncFingerprintDevice *p_device, int32_t type,
                    const uint8_t *p_buffer, uint32_t buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (NULL == p_device) {
        ANC_LOGE("%s, p_device is NULL", __func__);
        return ANC_FAIL;
    }

    if (NULL == p_device->fp_external_worker.DoWork) {
        ANC_LOGE("%s, external work3 is NULL", __func__);
        return ANC_FAIL;
    }

    p_device->fp_external_worker.DoWork(p_device, type, p_buffer, buffer_length);

    return ret_val;
}



AncFingerprintWorkProducer g_work_producer = {
    .Init = FpmInit,
    .Deinit = FpmDeinit,
    .PushTaskToConsumer = FpmPushTaskToConsumer,

    .PreEnroll = FpmPreEnroll,
    .Enroll = FpmEnroll,
    .PostEnroll = FpmPostEnroll,
    .GetAuthenticatorId = FpmGetAuthenticatorId,
    .Cancel = FpmCancel,
    .Enumerate = FpmEnumerate,
    .Remove = FpmRemove,
    .SetActiveGroup = FpmSetActiveGroup,
    .Authenticate = FpmAuthenticate,
    .ExcuteCommand = FpmExcuteCommand,

    .OnEnrollResult = FpmCbOnEnrollResult,
    .OnAcquired = FpmCbOnAcquired,
    .OnAuthenticated = FpmCbOnAuthenticated,
    .OnError = FpmCbOnError,
    .OnRemoved = FpmCbOnRemoved,
    .OnEnumerate = FpmCbOnEnumerate,
    .OnExcuteCommand = FpmCbOnExcuteCommand,

    .StartHeartBeatRateDetect = FpmStartHeartBeatRateDetect,

    .p_device = NULL,
};

AncExternalFeatureManager g_external_feature_manager = {
    .DoWork1 = FpExternalWork1,
    .DoWork2 = FpExternalWork2,
    .DoWork3 = FpExternalWork3,

    .p_device = NULL,
};

AncFingerprintManager g_fp_manager = {
    .p_producer = &g_work_producer,
    .p_consumer = NULL,

    .p_external_feature_manager = &g_external_feature_manager,
};

ANC_RETURN_TYPE InitFingerprintManager(AncFingerprintDevice *p_device) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint8_t algo_version_string[100] = "";
    char module_id[32];

    g_fp_manager.p_external_feature_manager->p_device = p_device;

    g_fp_manager.p_producer->p_device = p_device;
    if (ANC_OK != (ret_val = g_fp_manager.p_producer->Init(&g_fp_manager))) {
        ANC_LOGE("fail to init anc producer");
        goto INIT_FP_MANAGER_FAIL;
    }

    if (ANC_OK != (ret_val = InitNetlinkEventManager(&g_fp_manager))) {
        ANC_LOGE("fail to init netlink event manager");
        goto INIT_NL_MANAGER_FAIL;
    }

    if (ANC_OK != (ret_val = InitTpEventManager(&g_fp_manager))) {
        ANC_LOGE("fail to init tp event manager");
        goto INIT_TP_MANAGER_FAIL;
    }

    if (ANC_OK != (ret_val = InitHbmEventManager(&g_fp_manager))) {
        ANC_LOGE("fail to init hbm event manager");
        goto INIT_HBM_MANAGER_FAIL;
    }

    if (ANC_OK != (ret_val = InitFingerprintWorkConsumer(&g_fp_manager))) {
        ANC_LOGE("fail to init anc consumer");
        goto INIT_FP_WC_FAIL;
    }

    p_device->p_data = (void *)&g_fp_manager;

    if (ANC_OK != (ret_val = InitAncCa())) {
        ANC_LOGE("fail to init anc ca");
        goto INIT_ANC_CA_FAIL;
    }

    ANC_LOGD("set ca time to ta");
    if(ANC_OK != (ret_val = ExtensionSetCurrentCATime())) {
        ANC_LOGE("fail to set time to ta");
        goto SET_CA_TIME_FAIL;
    }

    ANC_LOGD("check ta version");
    if(ANC_OK != (ret_val = AuxiliaryCheckTaVersion())) {
        ANC_LOGE("fail to check ta version");
        goto CHECK_TA_VERSION_FAIL;
    }

    ANC_LOGD("init sensor manager");
    if (ANC_OK != (ret_val = InitSensorManager(&g_fp_manager))) {
        ANC_LOGE("fail to init sensor manager");
        goto INIT_SENSOR_FAIL;
    }

#ifdef ANC_SAVE_ALGO_FILE
    if (ANC_OK != (ret_val = g_background_worker.Init())) {
        ANC_LOGE("fail to init background worker ret_val = %d", ret_val);
        goto INIT_BACKGROUND_WORKER_FAIL;
    }
#endif

    g_fp_manager.p_sensor_manager->SetPowerMode(&g_fp_manager, ANC_SENSOR_WAKEUP);

    ANC_LOGD("get module id");
    AncMemset(module_id, 0, sizeof(module_id));
    ret_val = SensorGetModuleId(module_id, sizeof(module_id));
    if(ANC_OK != ret_val) {
        ANC_LOGE("get module id failed");
    } else {
        property_set(PROPERTY_FINGERPRINT_QRCODE_VALUE, (const char *)module_id);
        ANC_LOGD("init get module id is %s", module_id);
    }

    ANC_LOGD("init algo");
    ret_val = AlgorithmInit();
    if (ANC_OK == ret_val) {
        ExtensionSetCaliProperty(ANC_TRUE);
    } else {
        if (ANC_FAIL_NO_CALIBRATION == ret_val) {
            ExtensionSetCaliProperty(ANC_FALSE);
        }
        ANC_LOGE("fail to init algo ret_val = %d", ret_val);
        goto INIT_ALGO_FAIL;
    }
    g_fp_manager.p_sensor_manager->SetPowerMode(&g_fp_manager, ANC_SENSOR_SLEEP);

    ANC_LOGD("get algo version");
    AncMemset(algo_version_string, 0, sizeof(algo_version_string));
    if (ANC_OK != (ret_val = GetAlgorithmVersion(algo_version_string,
             sizeof(algo_version_string)/sizeof(algo_version_string[0])))) {
        ANC_LOGE("fail to get algo version ret_val %d", ret_val);
        goto GET_ALGO_VERSION_FAIL;
    }
    ANC_LOGD("algo version:%s", algo_version_string);

    property_set(PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION, (const char *)algo_version_string);
    property_set(PROPERTY_FINGERPRINT_QRCODE, "1");

    ANC_LOGD("get hmac key");
    if (ANC_OK != (ret_val = GetHmacKey())) {
        ANC_LOGE("fail to get hmac key");
        goto GET_HMAC_KEY_FAIL;
    }

    if (ExtensionLoadCalibration() != ANC_OK) {
        ExtensionSetCaliProperty(ANC_FALSE);
        ANC_LOGE("failed load calibrain. should be no calibration");
    }
    return ret_val;

GET_HMAC_KEY_FAIL :
GET_ALGO_VERSION_FAIL :
    AlgorithmDeinit();
INIT_ALGO_FAIL :

#ifdef ANC_SAVE_ALGO_FILE
    g_background_worker.Deinit();
INIT_BACKGROUND_WORKER_FAIL:
#endif

    DeinitSensorManager(&g_fp_manager);
INIT_SENSOR_FAIL :
CHECK_TA_VERSION_FAIL :
SET_CA_TIME_FAIL :
    DeinitAncCa();
INIT_ANC_CA_FAIL :
    DeinitFingerprintWorkConsumer(&g_fp_manager);
INIT_FP_WC_FAIL :
    DeinitHbmEventManager(&g_fp_manager);
INIT_HBM_MANAGER_FAIL :
    DeinitTpEventManager(&g_fp_manager);
INIT_TP_MANAGER_FAIL :
    DeinitNetlinkEventManager(&g_fp_manager);
INIT_NL_MANAGER_FAIL :
    g_fp_manager.p_producer->Deinit(&g_fp_manager);
INIT_FP_MANAGER_FAIL :
    return ret_val;
}

ANC_RETURN_TYPE DeinitFingerprintManager(AncFingerprintDevice *p_device) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    g_fp_manager.p_producer->p_device = NULL;
    p_device->p_data = NULL;

    if (ANC_OK != (ret_val = DeinitFingerprintWorkConsumer(&g_fp_manager))) {
        ANC_LOGE("fail to deinit anc consumer");
    }

    if (ANC_OK != (ret_val = DeinitHbmEventManager(&g_fp_manager))) {
        ANC_LOGE("fail to deinit hbm event manager");
    }

    if (ANC_OK != (ret_val = DeinitTpEventManager(&g_fp_manager))) {
        ANC_LOGE("fail to deinit tp event manager");
    }

    if (ANC_OK != (ret_val = DeinitNetlinkEventManager(&g_fp_manager))) {
        ANC_LOGE("fail to deinit netlink event manager");
    }

    if (ANC_OK != (ret_val = g_fp_manager.p_producer->Deinit(&g_fp_manager))) {
        ANC_LOGE("fail to deinit anc producer");
    }

    ANC_LOGD("deinit algo");
    if (ANC_OK != (ret_val = AlgorithmDeinit())) {
        ANC_LOGE("fail to deinit algo");
    }

#ifdef ANC_SAVE_ALGO_FILE
    ANC_LOGD("deinit background worker");
    if (ANC_OK != (ret_val = g_background_worker.Deinit())) {
        ANC_LOGE("fail to deinit background worker");
    }
#endif

    ANC_LOGD("deinit sensor manager");
    if (ANC_OK != (ret_val = DeinitSensorManager(&g_fp_manager))) {
        ANC_LOGE("fail to deinit sensor manager");
    }

    if (ANC_OK != (ret_val = DeinitAncCa())) {
        ANC_LOGE("fail to deinit ca");
    }

    if (ANC_OK != (ret_val = AncMemoryWrapperCheck())) {
        ANC_LOGE("fail to check memory leak");
    }

    return ret_val;
}
