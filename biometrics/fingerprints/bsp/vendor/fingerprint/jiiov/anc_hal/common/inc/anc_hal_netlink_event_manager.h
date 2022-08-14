#ifndef ANC_HAL_NETLINK_EVENT_MANAGER_H
#define ANC_HAL_NETLINK_EVENT_MANAGER_H

#include <pthread.h>

#include "anc_hal_manager.h"
#include "anc_error.h"
#include "anc_common_type.h"

typedef enum {
    ANC_NETLINK_EVENT_TEST = 0,
    ANC_NETLINK_EVENT_IRQ,
    ANC_NETLINK_EVENT_SCR_OFF,
    ANC_NETLINK_EVENT_SCR_ON,
    ANC_NETLINK_EVENT_TOUCH_DOWN,
    ANC_NETLINK_EVENT_TOUCH_UP,
    ANC_NETLINK_EVENT_UI_READY,
    ANC_NETLINK_EVENT_EXIT,
    ANC_NETLINK_EVENT_INVALID,
    ANC_NETLINK_EVENT_MAX
}ANC_NETLINK_EVENT_TYPE;

typedef enum {
    ANC_NETLINK_EVENT_STATUS_INVALID = 0,
    ANC_NETLINK_EVENT_STATUS_INIT,
    ANC_NETLINK_EVENT_STATUS_IDLE,
    ANC_NETLINK_EVENT_STATUS_RUNNING,
    ANC_NETLINK_EVENT_STATUS_EXIT
}ANC_NETLINK_EVENT_STATUS;


struct AncFingerprintManager;
struct sAncNetlinkEventManager {
    ANC_RETURN_TYPE (*Init)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*Deinit)(struct AncFingerprintManager *p_manager);

    pthread_t thread;
    pthread_mutex_t mutex;

    ANC_NETLINK_EVENT_STATUS status;
};



ANC_RETURN_TYPE InitNetlinkEventManager(struct AncFingerprintManager *p_manager);
ANC_RETURN_TYPE DeinitNetlinkEventManager(struct AncFingerprintManager *p_manager);


#endif
