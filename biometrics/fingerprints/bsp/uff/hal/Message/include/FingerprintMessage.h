/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/include/FingerprintMessage.h
 **
 ** Description:
 **      FingerprintMessage for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _FPMESSAGE_H_
#define _FPMESSAGE_H_

#include <stddef.h>
#include <stdint.h>
#include "FpType.h"
#include "HalContext.h"
#include "Handler.h"
#include "Message.h"

/* netlink feature support */
#define NETLINKROUTE 25
#define MAX_NL_MSG_LEN 32

typedef enum netlink_status {
    netlink_creat_socket_and_start_loop  = 0,
    netlink_exit_loop_and_destory_socket = 1,
} netlink_status_type_t;

typedef enum netlink_cmd {
    NETLINK_EVENT_TEST         = 0,
    NETLINK_EVENT_IRQ          = 1,
    NETLINK_EVENT_SCREEN_OFF   = 2,
    NETLINK_EVENT_SCREEN_ON    = 3,
    NETLINK_EVENT_TP_TOUCHDOWN = 4,
    NETLINK_EVENT_TP_TOUCHUP   = 5,
    NETLINK_EVENT_UI_READY     = 6,
    NETLINK_EVENT_UI_DISAPPEAR = 7,
    NETLINK_EVENT_EXIT         = 8,
    NETLINK_EVENT_INVALID      = 9,
    NETLINK_EVENT_MAX
} netlink_cmd_t;

typedef enum fp_event_state {
    E_EVENT_STATE_NONE         = 0,
    E_EVENT_STATE_TP_TOUCHDOWN = 1,
    E_EVENT_STATE_TP_TOUCHUP   = 2,
    E_EVENT_STATE_UI_READY     = 3,
    E_EVENT_STATE_UI_DISAPPEAR = 4,
    E_EVENT_STATE__OTHERS      = 5,
} fp_event_state_t;

typedef struct fp_event_data {
    fp_event_state_t last_state;
} fp_event_data_t;

namespace android {
class HalContext;
class FingerprintMessage : public HandlerCallback {
   public:
    fp_event_data_t mEventState;

   public:
    explicit FingerprintMessage(HalContext* context);
    virtual ~FingerprintMessage();
    static FingerprintMessage* getInstance() { return sInstance; }
    fp_return_type_t           handleNetlinkMessage(int32_t msg);
    void setCurrentEventState(int msg);
    bool handleUireadyEvent();
    bool handleTouchUpEvent();
    bool handleTouchDownEvent();
    void interruptProcessFpEvent();
    fp_return_type_t netlinkThreadLoop();
    void             handleMessage(FpMessage msg);
    fp_return_type_t sendToMessageThread(netlink_status_type_t status);

   private:
    static FingerprintMessage* sInstance;
    sp<Handler>                mMessageHandler = nullptr;
    HalContext*                mHalContext;
    std::mutex                 mEventLock;
};
}  // namespace android

#endif /* _FPMESSAGE_H_ */
