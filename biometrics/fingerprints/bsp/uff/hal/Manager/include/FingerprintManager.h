/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Manager/include/FingerprintManager.h
 **
 ** Description:
 **      FingerprintManager for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#ifndef _FINGERPRINTMANAGER_H_
#define _FINGERPRINTMANAGER_H_

#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>
#include "HalContext.h"
#include "Handler.h"
#include "Message.h"
#include "ActionType.h"
#include "fingerprint.h"
#include "FpType.h"
#include "AutoSmoking.h"
#include "FingerprintFunction.h"

namespace android {
class HalContext;
class FpMessage;

class FingerprintManager : public HandlerCallback, public FingerprintFunction {
   public:
    explicit FingerprintManager(HalContext* context);
    virtual ~FingerprintManager();

    //msg
    virtual void handleMessage(FpMessage msg);
    fp_return_type_t sendMessageToWorkThread(action_message_t action);
    fp_return_type_t setNetlinkMessageToWait(int32_t wait_msg);
    int32_t getNetlinkMessageToBeWait(void);
    fp_return_type_t handleNetlinkMessage(int32_t msg);

    //autotest
    fp_return_type_t simulatedEvent(SIMULATED_EVENT_T event);

   private:
    std::mutex  mLock;
    sp<Handler> mManageHandler = nullptr;
    //HalContext* mContext;
    int32_t mNetlinkMessageToWait;
};
}  // namespace android

#endif /* _FINGERPRINTMANAGER_H_ */
