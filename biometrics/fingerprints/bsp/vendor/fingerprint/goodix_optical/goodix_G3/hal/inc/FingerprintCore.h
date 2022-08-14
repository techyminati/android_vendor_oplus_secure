/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _FINGERPRINTCORE_H_
#define _FINGERPRINTCORE_H_
#include "fingerprint_oplus.h"
#include "gf_error.h"
#include "HalBase.h"
#include "EventCenter.h"
#include "gf_algo_types.h"
#include "gf_event.h"
#include "DcsInfo.h"
#include<mutex>
namespace goodix
{

    class Timer;
    void syncFingerListBasedOnSettings(HalContext* context, uint32_t groupId);
    class FingerprintCore: public HalBase, public IEventHandler
    {
    public:
        typedef enum
        {
            STATE_IDLE,
            STATE_ENROLL,
            STATE_AUTHENTICATE,
            STATE_REMOVE,
            STATE_ENUMERATE,
            STATE_PAUSE_ENUMERATE
        } WORK_STATE;

        typedef enum
        {
            STATE_INIT,
            STATE_DOWN,
            STATE_UIREADY,
            STATE_UP,
        } CAPTURE_STATE;

        class AuthenticateContext
        {
        public:
            AuthenticateContext()
            {
                result = GF_SUCCESS;
                retry = 0;
                auth_cmd = nullptr;
                fingerDownTime = 0;
                extend = nullptr;
            }
            gf_error_t result;
            int32_t retry;
            int32_t up;
            gf_algo_auth_image_t* auth_cmd;
            int64_t fingerDownTime;
            void* extend;
        };

        class EnrollContext
        {
        public:
            EnrollContext()
            {
                result = GF_SUCCESS;
                enroll_cmd = nullptr;
                fingerDownTime = 0;
                extend = nullptr;
            }
            gf_error_t result;
            gf_algo_enroll_image_t* enroll_cmd;
            int64_t fingerDownTime;
            void* extend;
        };

        explicit FingerprintCore(HalContext* context);
        virtual ~FingerprintCore();
        virtual gf_error_t init();
        gf_error_t setActiveGroup(uint32_t gid, const char *path = NULL);  // NOLINT(575)
        uint64_t preEnroll();
        gf_error_t enroll(const void *hat, uint32_t gid, uint32_t timeoutSec);
        gf_error_t postEnroll();
        gf_error_t authenticate(uint64_t operationId, uint32_t gid);
        gf_error_t authenticateAsType(uint64_t operationId, uint32_t gid, uint32_t authtype);
        void setImageDumpFlag(int flag);
        bool isRegionSupported();
        gf_error_t authenticateFido(uint32_t groupId, uint8_t *aaid, uint32_t aaidLen, uint8_t *finalChallenge, uint32_t challengeLen);
        uint64_t getAuthenticatorId();
        gf_error_t remove(uint32_t gid, uint32_t fid);
        gf_error_t enumerate(void *result, uint32_t *maxSize);
        gf_error_t enumerate();
        gf_error_t screenState(uint32_t state);
        uint8_t isIdValid(uint32_t gid, uint32_t fid);
        gf_error_t getIdList(uint32_t gid, uint32_t *list, int32_t *count);
        gf_error_t cancelFido();
        gf_error_t cancel();
        gf_error_t setNotify(fingerprint_notify_t notify);
        gf_error_t setFidoNotify(fingerprint_notify_t notify);
        void setAuthType(uint32_t authType);
        // overide IEventHandler
        gf_error_t onEvent(gf_event_type_t e);
        gf_error_t bind_bigcore_bytid();
        gf_error_t setUxThread(uint8_t enable);
        gf_error_t removeTemplates(uint32_t gid, uint32_t fid);
        gf_error_t touchdown();
        gf_error_t touchup();
        gf_error_t setScreenState(uint32_t state);
        gf_error_t sendFingerprintCmd(int32_t cmd_id,  int8_t* in_buf, uint32_t size);
        gf_error_t notifyFingerprintCmd(int64_t devId, int32_t cmdId, const int8_t *result, int32_t resultLen);
        gf_error_t setHbmMode(uint32_t Mode);
        gf_error_t setBrightness(uint32_t Mode);
        gf_error_t sendDumpdataToTa(uint8_t* dump_data,uint32_t size, uint8_t* enc_buf, uint32_t *encrypt_len);
        void syncFingerList(uint32_t groupId);
        gf_error_t pauseEnroll();
        gf_error_t continueEnroll();
        gf_error_t checkEnrollResult(gf_error_t result);
        gf_error_t get_total_enroll_times(uint32_t *enroll_times);
        gf_error_t notifyTouch(fingerprint_msg_type_t type);
        void init_report_data(void);
        void getDumpPictureLevel (void);
#ifdef FP_HYPNUSD_ENABLE
        gf_error_t set_hypnus(int32_t action_type, int32_t action_timeout);
#endif
    protected:
        virtual gf_error_t prepareEnrollRequest();
        virtual gf_error_t onEnrollRequested(const void *hat, uint32_t gid, uint32_t timeoutSec);
        gf_error_t onEnrollReceivedEvt(gf_event_type_t event);
        gf_error_t onEnrollDownEvt();
        virtual gf_error_t onEnrollUpEvt();
        virtual gf_error_t onEnrollStart(EnrollContext* context);
        virtual gf_error_t onBeforeEnrollCapture(EnrollContext* context);
        virtual gf_error_t onAfterEnrollCapture(EnrollContext* context);
        virtual gf_error_t onBeforeEnrollAlgo(EnrollContext* context);
        virtual gf_error_t onAfterEnrollAlgo(EnrollContext* context);
        virtual gf_error_t onEnrollStop(EnrollContext* context);
        virtual void onEnrollError(EnrollContext* context);
        virtual gf_error_t notifyEnrollProgress(EnrollContext* context);

        virtual gf_error_t prepareAuthRequest();
        virtual gf_error_t onAuthRequested(uint64_t operationId, uint32_t gid);
        gf_error_t onAuthReceivedEvt(gf_event_type_t event);
        gf_error_t onAuthDownEvt();
        gf_error_t onEnrollUIReady();
        gf_error_t onAuthUIReady();
        gf_error_t waitUIReady();
        
        virtual gf_error_t onAuthUpEvt();
        virtual gf_error_t onAuthStart(AuthenticateContext* context);
        virtual gf_error_t onBeforeAuthCapture(AuthenticateContext* context);
        virtual gf_error_t onAfterAuthCapture(AuthenticateContext* context);
        virtual gf_error_t onBeforeAuthAlgo(AuthenticateContext* context);
        virtual gf_error_t onAfterAuthAlgo(AuthenticateContext* context);
        virtual gf_error_t onAfterAuthRetry(AuthenticateContext* context);
        virtual gf_error_t onAfterAuthSuccess(AuthenticateContext* context);
        virtual gf_error_t onAuthStop(AuthenticateContext* context);
        virtual bool needRetry(AuthenticateContext* context);
        virtual void onAuthError(AuthenticateContext* context);
        virtual gf_error_t send_auth_dcsmsg(AuthenticateContext* context);
#ifdef FP_CONFIG_SETTINGS_ENABLE
        virtual gf_error_t getFpConfigData(void *para);
#endif
        virtual gf_error_t notifyAuthSuccess(AuthenticateContext* context);
        virtual void notifyAuthNotMatched();

        // notify message
        virtual gf_error_t notifyAcquiredInfo(fingerprint_acquired_info_t info);
        virtual gf_error_t notifyErrorInfo(fingerprint_error_t err);
        virtual gf_error_t notifyRemove(uint32_t gid, uint32_t fid, uint32_t remainingTemplates);
        virtual gf_error_t notifyEnumerate(gf_enumerate_t* result);

        virtual gf_error_t notifyAuthFail();
        void onError(gf_error_t err);
        virtual void sensorIsBroken();
        virtual void doCancel();
        virtual gf_error_t dumpKPI(const char *func_name);
        void setAuthUpNotified(bool mAuthUp);
        bool getAuthUpNotified();

    private:
        static void enrollTimerThread(union sigval v);
        void startEnrollTimer(uint32_t timeoutSec);
        void cancelEnrollTimer();
        void wakeEnrollTimer();
        gf_error_t saveTemplates(uint32_t gid, uint32_t fid);
        WORK_STATE getWorkState();

    protected:
        fingerprint_notify_t mFidoNotify;
        bool mSkipFlag;

    private:
        bool mAuthDownDetected;
        bool mAuthUpNotified;
        std::mutex mUpNotifiedMutex;
        int64_t mTotalKpiTime;
        WORK_STATE mWorkState;
        uint32_t mEnrollTimerSec;
        Timer* mEnrollTimer;
        CAPTURE_STATE mFingerStatus;
        int64_t mPrenrollTime;
        uint32_t enrollErrorCount;
    public:
        uint32_t mScreenStatus;
        uint32_t mScreenInAuthMode;
        uint32_t mFailedAttempts;
        gf_algo_auth_type_t mAuthType;
        fingerprint_notify_t mNotify;
        uint32_t mGid;
        oplus_report_data_t report_data;
        oplus_fingerprint_dcs_auth_result_type_t dcs_auth_result_type;
        uint32_t mDSPAvailable;
        AuthenticateContext authContext;
        EnrollContext enrollContext;
        bool handleUIReadyEvent;

        uint32_t authTotalTime;
        uint32_t uiReadyTime;
        uint32_t captureTime;
        uint32_t enrollTotalTime;
        int64_t fingerDownTime;
    };
}  // namespace goodix

#endif /* _FINGERPRINTCORE_H_ */
