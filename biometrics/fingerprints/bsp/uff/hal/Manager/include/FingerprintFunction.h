/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Manager/include/FingerprintAuth.h
 **
 ** Description:
 **      FingerprintAuth for fingerprint
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
#ifndef _FINGERPRINTFUNCTION_H_
#define _FINGERPRINTFUNCTION_H_

#include <stddef.h>
#include <stdint.h>
#include <semaphore.h>
#include "ActionType.h"
#include "fingerprint.h"
#include "FpType.h"
#include "AutoSmoking.h"
#include "FingerprintNotify.h"
#include "FingerprintCommon.h"
#include <utils/Mutex.h>
namespace android {
class HalContext;
//class FingerprintCommon;
//class FingerprintNotify;

class FingerprintFunction : public FingerprintNotify, public FingerprintCommon, public IAutoSmokingInterface{
   public:
    explicit FingerprintFunction(HalContext* context);
    virtual ~FingerprintFunction();

    //init
    fp_return_type_t init();
    fp_return_type_t setNotify(fingerprint_notify_t notify);
    fp_return_type_t setActiveGroup(uint32_t gid, const char* path = NULL);
    fp_return_type_t enumerate();

    //enroll
    fp_return_type_t getTotalEnrollTimes(uint32_t* enroll_times);
    uint64_t         preEnroll();
    fp_return_type_t enroll(const void* hat, uint32_t gid, uint32_t timeoutSec);
    fp_return_type_t enrollDown(fp_enroll_type_t type);
    fp_return_type_t enrollImage(fp_enroll_image_result_t* enroll_result);
    fp_return_type_t enrollSaveTemplate();
    fp_return_type_t postEnroll();
    fp_return_type_t enrollFinish();
    void checkEnrollError(fp_return_type_t err);

    //auth
    uint64_t         getAuthenticatorId();
    fp_return_type_t authenticateAsType(uint64_t operationId, uint32_t gid, uint32_t authtype);
    fp_return_type_t authenticate(uint64_t operationId, uint32_t gid);
    fp_return_type_t authenticateStart();
    fp_return_type_t authenticateDown(fp_auth_type_t type);
    fp_return_type_t captureImg(fp_mode_t mode, uint32_t retry_index);
    fp_return_type_t getFeature(fp_mode_t mode, uint32_t retry_index);
    fp_return_type_t authenticateCompare(fp_auth_compare_result_t* compare_result, uint32_t retry_index);
    fp_return_type_t authenticateStudy(uint32_t finger_id);
    fp_return_type_t authenticateFinish();

    //remove
    fp_return_type_t removeSingleFinger(uint32_t gid, uint32_t fid, uint32_t remainCount);
    fp_return_type_t remove(uint32_t gid, uint32_t fid);

    //common
    fp_return_type_t screenState(uint32_t state);
    fp_return_type_t cancel();
    fp_return_type_t touchdown();
    fp_return_type_t touchup();
    fp_return_type_t sendAuthDcsmsg();
    void setWorkMode(fp_mode_t mode);
    fp_mode_t getWorkMode();
    fp_return_type_t doTouchDownEvent(fp_auth_type_t type);
    fp_return_type_t setFingerstatus(finger_status_type_t status);
    fp_return_type_t checkFingerUpTooFast(uint32_t retry_index);

    //sendcmd
    fp_return_type_t sendFingerprintCmd(int32_t cmd_id, int8_t* in_buf, uint32_t size);
    fp_return_type_t sendTaCommand(void *cmd, uint32_t size);

   public:
    uint32_t               mScreenStatus;
    uint32_t               current_gid;
    finger_status_type_t mFingerStatus;
    fp_auth_compare_result_t compare_result;
    fp_enroll_image_result_t enroll_result;
    int64_t mFingerDownTime;
    int64_t mUiReadyTime;
    int64_t mFunctionFinishTime;

   private:
    Mutex mWorkModeMutex;
    fp_mode_t   mWorkMode;
    std::mutex  mLock;
    sem_t mUireadySem;
    HalContext* mContext;
    int mSerialEnrErrCount;
};
}  // namespace android

#endif /* _FINGERPRINTFUNCTION_H_ */
