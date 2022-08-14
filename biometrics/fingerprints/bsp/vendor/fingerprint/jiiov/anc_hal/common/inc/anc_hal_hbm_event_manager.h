#ifndef ANC_HAL_HBM_EVENT_MANAGER_H
#define ANC_HAL_HBM_EVENT_MANAGER_H

#include <pthread.h>

#include "anc_hal_manager.h"
#include "anc_error.h"
#include "anc_common_type.h"

typedef enum {
    HBM_EVENT_INVALID_TYPE = 0,
    HBM_EVENT_HBM_READY,//high brightness mode
    HBM_EVENT_MAX
}ANC_HBM_EVENT_TYPE;


struct AncFingerprintManager;
struct sAncHbmEventManager {
    ANC_RETURN_TYPE (*Init)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*Deinit)(struct AncFingerprintManager *p_manager);

    ANC_RETURN_TYPE (*HBMReady)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*WaitHBMReady)(struct AncFingerprintManager *p_manager, uint32_t time_out);
    ANC_RETURN_TYPE (*SetHbm)(struct AncFingerprintManager *p_manager, bool on_off);
    ANC_RETURN_TYPE (*SetHbmEventType)(struct AncFingerprintManager *p_manager, ANC_HBM_EVENT_TYPE hbm_event_type);

    void (*SendCondSignal)(struct AncFingerprintManager *p_manager);

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    long long hbm_ready_time;
    ANC_HBM_EVENT_TYPE hbm_event_type;
};


ANC_RETURN_TYPE InitHbmEventManager(struct AncFingerprintManager *p_manager);
ANC_RETURN_TYPE DeinitHbmEventManager(struct AncFingerprintManager *p_manager);


#endif
