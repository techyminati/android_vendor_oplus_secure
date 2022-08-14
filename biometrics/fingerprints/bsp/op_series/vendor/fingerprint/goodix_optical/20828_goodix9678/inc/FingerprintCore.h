/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _FINGERPRINTCORE_H_
#define _FINGERPRINTCORE_H_
#include "GoodixFingerprint.h"
#include "gf_error.h"
#include "HalBase.h"
#include "EventCenter.h"
#include "gf_algo_types.h"
#include "gf_event.h"

namespace goodix {

    class Timer;
	class ProductTest;
    void syncFingerListBasedOnSettings(HalContext *context, uint32_t groupId);
    class FingerprintCore: public HalBase, public IEventHandler {
    public:
        typedef enum {
            STATE_IDLE,
            STATE_ENROLL,
            STATE_AUTHENTICATE,
            STATE_REMOVE,
            STATE_ENUMERATE,
            STATE_KPI
        } WORK_STATE;

        class AuthenticateContext {
        public:
            AuthenticateContext() {
                result = GF_SUCCESS;
                retry = 0;
                auth_cmd = nullptr;
                fingerDownTime = 0;
                extend = nullptr;
            }
            gf_error_t result;
            int32_t retry;
            gf_algo_auth_image_t *auth_cmd;
            int64_t fingerDownTime;
            void *extend;
        };
        class EnrollContext {
        public:
            EnrollContext() {
                result = GF_SUCCESS;
                enroll_cmd = nullptr;
                fingerDownTime = 0;
                extend = nullptr;
            }
            gf_error_t result;
            gf_algo_enroll_image_t *enroll_cmd;
            int64_t fingerDownTime;
            void *extend;
        };
        explicit FingerprintCore(HalContext *context);
        virtual ~FingerprintCore();
        virtual gf_error_t init();
        gf_error_t reInit();
        gf_error_t setActiveGroup(uint32_t gid, const char *path = NULL);  // NOLINT(575)
        uint64_t preEnroll();
        gf_error_t enroll(const void *hat, uint32_t gid, uint32_t timeoutSec);
        gf_error_t postEnroll();
        gf_error_t authenticate(uint64_t operationId, uint32_t gid);
        gf_error_t authenticateFido(uint32_t groupId, uint8_t *aaid, uint32_t aaidLen,
                                    uint8_t *finalChallenge, uint32_t challengeLen);
        uint64_t getAuthenticatorId();
        gf_error_t remove(uint32_t gid, uint32_t fid);
        gf_error_t enumerate(void *result, uint32_t *maxSize);
        gf_error_t enumerate();
        gf_error_t screenState(uint32_t state);
        uint8_t isIdValid(uint32_t gid, uint32_t fid);
        gf_error_t getIdList(uint32_t gid, uint32_t *list, int32_t *count);
        gf_error_t cancelFido();
        virtual gf_error_t cancel(bool notifyCancelMsg = true);  // NOLINT(575)
        gf_error_t setNotify(gf_fingerprint_notify_t notify);
        gf_error_t setFidoNotify(gf_fingerprint_notify_t notify);
        void setAuthType(uint32_t authType);
        // overide IEventHandler
        gf_error_t onEvent(gf_event_type_t e);
        gf_error_t removeTemplates(uint32_t gid, uint32_t fid);
        
        gf_error_t setScreenState(uint32_t state);
        gf_error_t sendFingerprintCmd(int32_t cmd_id,  int8_t* in_buf, uint32_t size);
        gf_error_t setHbmMode(uint32_t Mode);
        gf_error_t setBrightness(uint32_t Mode);
        void syncFingerList(uint32_t groupId);
        virtual bool needRetryIfCaptureFailed(AuthenticateContext* context);
        gf_error_t checkEnrollResult(gf_error_t result);
        gf_error_t set_hypnus(int32_t action_type, int32_t action_timeout);
        void hypnus_request_change(int32_t action_type, int32_t action_timeout);
        typedef struct hypnus_state_info {
            bool hypnusd_request;
            pthread_mutex_t hypnusd_lock;
            pthread_cond_t hypnusd_cond;
            int hypnusd_action_type;
            int hypnusd_action_timeout;
        } hypnus_state_t;
    protected:
        virtual gf_error_t prepareEnrollRequest();
        virtual gf_error_t onEnrollRequested(const void *hat, uint32_t gid,
                                             uint32_t timeoutSec);
        gf_error_t onEnrollReceivedEvt(gf_event_type_t event);
        gf_error_t onEnrollDownEvt();
        virtual gf_error_t onEnrollUpEvt();
        virtual gf_error_t onEnrollStart(EnrollContext *context);
        virtual gf_error_t onBeforeEnrollCapture(EnrollContext *context);
        virtual gf_error_t onAfterEnrollCapture(EnrollContext *context);
        virtual gf_error_t onBeforeEnrollAlgo(EnrollContext *context);
        virtual gf_error_t onAfterEnrollAlgo(EnrollContext *context);
        virtual gf_error_t onAfterEnrollNotify(EnrollContext *context);
        virtual gf_error_t onEnrollStop(EnrollContext *context);
        virtual void onEnrollError(EnrollContext *context);
        virtual gf_error_t notifyEnrollProgress(EnrollContext *context);
        virtual gf_error_t prepareAuthRequest();
        virtual gf_error_t onAuthRequested(uint64_t operationId, uint32_t gid);
        gf_error_t onAuthReceivedEvt(gf_event_type_t event);
        gf_error_t onAuthDownEvt();
        virtual gf_error_t onAuthUpEvt();
        virtual gf_error_t onAuthStart(AuthenticateContext *context);
        virtual gf_error_t onBeforeAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthCapture(AuthenticateContext *context);
        virtual gf_error_t onBeforeAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthAlgo(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthRetry(AuthenticateContext *context);
        virtual gf_error_t onAfterAuthSuccess(AuthenticateContext *context);
        virtual gf_error_t onAuthStop(AuthenticateContext *context);
        virtual bool needRetry(AuthenticateContext *context);
        virtual void onAuthError(AuthenticateContext *context);
        virtual gf_error_t notifyAuthSuccess(AuthenticateContext *context);
        virtual void notifyAuthNotMatched();
        virtual gf_error_t onResetEvent();
        // notify message
        virtual gf_error_t notifyAcquiredInfo(gf_fingerprint_acquired_info_t info);
        virtual gf_error_t notifyErrorInfo(gf_fingerprint_error_t err);
        virtual gf_error_t notifyRemove(uint32_t gid, uint32_t fid,
                                        uint32_t remainingTemplates);
        virtual gf_error_t notifyEnumerate(gf_enumerate_t *result);
        virtual void onError(gf_error_t err);
        virtual void sensorIsBroken();
        virtual void doCancel();
        virtual gf_error_t dumpKPI(const char *func_name);
        virtual bool checkFingerLeave();
		virtual gf_error_t send_auth_dcsmsg(AuthenticateContext* context, bool notified);
        gf_error_t setDspSpeed(uint8_t operationCmd, uint8_t SpeedMode);
        bool waitDspSetSpeed(uint8_t operationCmd);

    private:
        static void enrollTimerThread(union sigval v);
        void startEnrollTimer(uint32_t timeoutSec);
        void cancelEnrollTimer();
        void wakeEnrollTimer();
        gf_error_t saveTemplates(uint32_t gid, uint32_t fid);

    protected:
        gf_fingerprint_notify_t mNotify;
        gf_fingerprint_notify_t mFidoNotify;
        int64_t mTotalKpiTime;
        uint32_t mFailedAttempts;
        virtual gf_error_t enrollPause();
        virtual gf_error_t enrollResume();
        virtual gf_error_t authenticatePause() {
            return GF_SUCCESS;
        }
        virtual gf_error_t authenticateResume() {
            return GF_SUCCESS;
        }
        bool mAuthDownDetected;
        WORK_STATE mWorkState;
        WORK_STATE getWorkState();
        bool isAuthDownDetected();
        int64_t uiReadyTime;


    private:
        uint32_t mEnrollTimerSec;
        Timer *mEnrollTimer;
        gf_algo_auth_type_t mAuthType;
        uint32_t enrollErrorCount;
        uint32_t mScreenStatus;
        uint32_t mGid;
        char mFpDataPath[MAX_FILE_ROOT_PATH_LEN];
        ProductTest *mProductTest;
        bool mAuthResultNotified;
        int64_t mPreEnrollTime;
    };
}  // namespace goodix

#endif /* _FINGERPRINTCORE_H_ */
