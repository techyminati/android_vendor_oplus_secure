#ifndef ANC_HAL_WORK_CONSUMER_H
#define ANC_HAL_WORK_CONSUMER_H

#include <pthread.h>

#include "anc_hal_manager.h"
#include "anc_error.h"

typedef enum {
    CONSUMER_STATUS_INVALID = 0,
    CONSUMER_STATUS_INIT,
    CONSUMER_STATUS_IDLE,
    CONSUMER_STATUS_RUNNING,
    CONSUMER_STATUS_EXIT
}ANC_CONSUMER_STATUS;

typedef struct {
    void (*Dowork)(void *p_arg);
    void *p_arg;
    uint8_t *p_name;
}AncWorkTask;

struct AncFingerprintManager;
struct sAncFingerprintWorkConsumer{
    ANC_RETURN_TYPE (*Init)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*Deinit)(struct AncFingerprintManager *p_manager);

    ANC_RETURN_TYPE (*PushTask)(struct AncFingerprintManager *p_manager, AncWorkTask *p_task);
    ANC_RETURN_TYPE (*ClearTask)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*GotoIdle)(struct AncFingerprintManager *p_manager);

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    AncWorkTask waiting_task;
    AncWorkTask running_task;
    ANC_CONSUMER_STATUS status;
};



ANC_RETURN_TYPE InitFingerprintWorkConsumer(struct AncFingerprintManager *p_manager);
ANC_RETURN_TYPE DeinitFingerprintWorkConsumer(struct AncFingerprintManager *p_manager);


#endif
