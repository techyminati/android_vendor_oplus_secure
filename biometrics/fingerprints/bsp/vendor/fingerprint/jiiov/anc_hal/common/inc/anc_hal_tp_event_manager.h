#ifndef ANC_HAL_TP_EVENT_MANAGER_H
#define ANC_HAL_TP_EVENT_MANAGER_H

#include <pthread.h>

#include "anc_hal_manager.h"
#include "anc_error.h"
#include "anc_common_type.h"

typedef enum {
    TP_EVENT_INVALID_TYPE = 0,
    TP_EVENT_TOUCH_DOWN,
    TP_EVENT_TOUCH_UP,
    TP_EVENT_SCREEN_ON,
    TP_EVENT_SCREEN_OFF,
    TP_EVENT_EXIT,
    TP_EVENT_HEART_RATE_FINGER_DOWN,
    TP_EVENT_HEART_RATE_FINGER_UP,
    TP_EVENT_MAX
}ANC_TP_EVENT_TYPE;

struct AncFingerprintManager;
struct sAncTpEventManager {
    ANC_RETURN_TYPE (*Init)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*Deinit)(struct AncFingerprintManager *p_manager);

    ANC_RETURN_TYPE (*TouchDown)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*TouchUp)(struct AncFingerprintManager *p_manager);

    ANC_RETURN_TYPE (*WaitTouchDown)(struct AncFingerprintManager *p_manager, uint32_t time_out);
    ANC_RETURN_TYPE (*WaitTouchUp)(struct AncFingerprintManager *p_manager, uint32_t time_out);

    ANC_RETURN_TYPE (*ScreenOn)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*ScreenOff)(struct AncFingerprintManager *p_manager);

    ANC_RETURN_TYPE (*HeartRateFingerDown)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*HeartRateFingerUp)(struct AncFingerprintManager *p_manager);

    ANC_BOOL (*IsHeartRateFingerDown)();
    ANC_BOOL (*IsHeartRateFingerUp)();

    ANC_BOOL (*IsTpTouchDown)();
    ANC_BOOL (*IsTpTouchUp)();
    ANC_BOOL (*IsScreenOn)();
    ANC_BOOL (*IsScreenOff)();
    ANC_RETURN_TYPE (*SetFpEnable)(struct AncFingerprintManager *p_manager, bool on_off);

    void (*SendCondSignal)(struct AncFingerprintManager *p_manager);

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    long long finger_down_time;
    long long finger_up_time;
    ANC_BOOL enable_touch_up_report;
};



ANC_RETURN_TYPE InitTpEventManager(struct AncFingerprintManager *p_manager);
ANC_RETURN_TYPE DeinitTpEventManager(struct AncFingerprintManager *p_manager);


#endif
