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
            STATE_KPI,
            STATE_PAUSE_ENUMERATE
        } WORK_STATE;
        WORK_STATE mWorkState;
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
        gf_error_t authenticateAsType(uint64_t operationId, uint32_t gid, uint32_t authtype);
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
        WORK_STATE getWorkState();
        bool isAuthDownDetected();
        virtual gf_error_t enrollPause();
        virtual gf_error_t enrollResume();
        virtual gf_error_t authenticatePause();
        virtual gf_error_t authenticateResume();
        bool mAuthDownDetected;

    private:
        uint32_t mEnrollTimerSec;
        Timer *mEnrollTimer;
        gf_algo_auth_type_t mAuthType;
        uint32_t enrollErrorCount;
        uint32_t mScreenStatus;
        uint32_t mGid;
        char mFpDataPath[MAX_FILE_ROOT_PATH_LEN];
        bool mAuthResultNotified;
        int64_t mPrenrollTime;
    };
}  // namespace goodix

enum
{
        PRODUCT_TEST_TOKEN_ERROR_CODE = 6000,
        PRODUCT_TEST_TOKEN_CHIP_TYPE,
        PRODUCT_TEST_TOKEN_CHIP_SERIES,
        PRODUCT_TEST_TOKEN_MCU_CHIP_ID,
        PRODUCT_TEST_TOKEN_SENSOR_CHIP_ID,
        PRODUCT_TEST_TOKEN_FLASH_CHIP_ID,
        PRODUCT_TEST_TOKEN_RANDOM_NUM,
        PRODUCT_TEST_TOKEN_COLLECT_PHASE,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_X,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_Y,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_WIDTH,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_HEIGHT,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_IMAGE_SENSOR_OFFSET,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_SCREEN_WIDTH,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_SCREEN_HEIGHT,
        PRODUCT_TEST_TOKEN_PERFORMANCE_TEST_CMD_ID,
        PRODUCT_TEST_TOKEN_SENSOR_INDEX,
        PRODUCT_TEST_TOKEN_RESULT_PGA_GAIN,
        PRODUCT_TEST_TOKEN_RESULT_EXPOSURE_TIME,
        PRODUCT_TEST_TOKEN_RESULT_FRAME_NUMBER,
        PRODUCT_TEST_TOKEN_RESULT_ITO_PATTERN_CODE,
        PRODUCT_TEST_TOKEN_RESULT_RUBBER_NUM,
        PRODUCT_TEST_TOKEN_RESULT_HOT_PIXEL_NUM,
        PRODUCT_TEST_TOKEN_RESULT_VALID_AREA,
        PRODUCT_TEST_TOKEN_RESULT_LOW_CORR_PITCH_LPF,
        PRODUCT_TEST_TOKEN_RESULT_EDGE_PIXEL_LEVEL1,
        PRODUCT_TEST_TOKEN_RESULT_EDGE_PIXEL_LEVEL2,
        PRODUCT_TEST_TOKEN_RESULT_MAX_LOCAL_BAD_PIXEL,
        PRODUCT_TEST_TOKEN_RESULT_MAX_TOTAL_BAD_PIXEL,
        PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE,
        PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE_LPF,
        PRODUCT_TEST_TOKEN_RESULT_FLAT_SNOISE_LPF_MT,
        PRODUCT_TEST_TOKEN_RESULT_CHART_DIRECTION,
        PRODUCT_TEST_TOKEN_RESULT_SIGNAL,
        PRODUCT_TEST_TOKEN_RESULT_SSNR,
        PRODUCT_TEST_TOKEN_RESULT_NOISE,
        PRODUCT_TEST_TOKEN_RESULT_SHAPENESS,
        PRODUCT_TEST_TOKEN_RESULT_SIGNAL_LPF,
        PRODUCT_TEST_TOKEN_RESULT_SSNR_LPF,
        PRODUCT_TEST_TOKEN_RESULT_NOISE_LPF,
        PRODUCT_TEST_TOKEN_RESULT_SHAPENESS_LPF,
        PRODUCT_TEST_TOKEN_RESULT_DARK_TNOISE,
        PRODUCT_TEST_TOKEN_RESULT_DARK_SNOISE,
        PRODUCT_TEST_TOKEN_RESULT_LIGHT_TNOISE,
        PRODUCT_TEST_TOKEN_RESULT_LIGHT_SNOISE,
        PRODUCT_TEST_TOKEN_RESULT_LIGHT_HIGH_MEAN,
        PRODUCT_TEST_TOKEN_CENTER_X_TO_CHIP,
        PRODUCT_TEST_TOKEN_CENTER_Y_TO_CHIP,
        PRODUCT_TEST_TOKEN_ANGLE_TO_CHIP,
        PRODUCT_TEST_TOKEN_CENTER_X_TO_SCREEN,
        PRODUCT_TEST_TOKEN_CENTER_Y_TO_SCREEN,
        PRODUCT_TEST_TOKEN_ANGLE_TO_SCREEN,
        PRODUCT_TEST_TOKEN_STATUS_BAR_HEIGHT,
        PRODUCT_TEST_TOKEN_IS_FULL_SCREEN,
        PRODUCT_TEST_TOKEN_IS_WHOLE_CIRCLE_PIC,
        PRODUCT_TEST_TOKEN_FEATURE_TYPE,
        PRODUCT_TEST_TOKEN_GAIN_TARGET,
        PRODUCT_TEST_TOKEN_BRIGHTNESS_LEVEL,
        PRODUCT_TEST_TOKEN_DISABLE_SAVE_FLAG,
        PRODUCT_TEST_TOKEN_PERFORMNCE_TEST_RESULT,
        PRODUCT_TEST_TOKEN_AGING_TEST_INTERVAL,
        PRODUCT_TEST_TOKEN_AGING_TEST_COUNT,
        PRODUCT_TEST_TOKEN_RESULT_BAD_POINT_NUM,
        PRODUCT_TEST_TOKEN_RESULT_CLUSTER_NUM,
        PRODUCT_TEST_TOKEN_RESULT_LARGEST_BAD_CLUSTER,
        PRODUCT_TEST_TOKEN_RESULT_BPN_IN_CLUSTER,
        PRODUCT_TEST_TOKEN_RESULT_LIGHT_HBAD_LINE_NUM,
        PRODUCT_TEST_TOKEN_RESULT_LIGHT_VBAD_LINE_NUM,
        PRODUCT_TEST_TOKEN_RESULT_AA_DARK_DIFF,
        PRODUCT_TEST_TOKEN_RESULT_UNOR_SIGNAL_LPF,
        PRODUCT_TEST_TOKEN_RESULT_SHARPNESS_LPF,
        PRODUCT_TEST_TOKEN_RESULT_TSNR,
        PRODUCT_TEST_TOKEN_RESULT_MAX_T_NOISE,
        PRODUCT_TEST_TOKEN_RESULT_HAF_BAD_POINT_NUM,
        PRODUCT_TEST_TOKEN_RESULT_HAF_BAD_BLOCK_NUM,
        PRODUCT_TEST_TOKEN_RESULT_BWHITE_PIXEL_NUM,
        PRODUCT_TEST_TOKEN_RESULT_BBLACK_PIXEL_NUM,
        PRODUCT_TEST_TOKEN_RESULT_LIGHT_LEAK_RATIO,
        PRODUCT_TEST_TOKEN_RESULT_DP_BAD_POINT_NUM,
        PRODUCT_TEST_TOKEN_RESULT_DP_MAX_BPN_IN_ROW,
        PRODUCT_TEST_TOKEN_RESULT_DP_MEAN_DIFF,
        PRODUCT_TEST_TOKEN_RESULT_DP_SNOISE_DARK,
        PRODUCT_TEST_TOKEN_RESULT_DP_SNOISE_LIGHT,
        PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_FLESH_HM,
        PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_FLESH_ML,
        PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_BLACK_HM,
        PRODUCT_TEST_TOKEN_RESULT_MIN_DIFF_BLACK_ML,
        PRODUCT_TEST_TOKEN_RESULT_MAX_DIFF_OFFSET,
        PRODUCT_TEST_TOKEN_IMAGE_QUALITY,
        PRODUCT_TEST_TOKEN_OPTICAL_TYPE,
        PRODUCT_TEST_TOKEN_LOCATION_CIRCLE_CENTER_OFFSET,
        PRODUCT_TEST_TOKEN_ALGO_VERSION,
        PRODUCT_TEST_TOKEN_PREPROCESS_VERSION,
        PRODUCT_TEST_TOKEN_PRODUCTION_ALGO_VERSION,
        PRODUCT_TEST_TOKEN_FAKE_VERSION,
        PRODUCT_TEST_TOKEN_FW_VERSION,
        PRODUCT_TEST_TOKEN_TEE_VERSION,
        PRODUCT_TEST_TOKEN_TA_VERSION,
        PRODUCT_TEST_TOKEN_CHIP_ID,
        PRODUCT_TEST_TOKEN_VENDOR_ID,
        PRODUCT_TEST_TOKEN_SENSOR_ID,
        PRODUCT_TEST_TOKEN_PRODUCTION_DATE,
        PRODUCT_TEST_TOKEN_PGA_GAIN,
        PRODUCT_TEST_TOKEN_EXPOSURE_TIME,
        PRODUCT_TEST_TOKEN_SENSOR_NUM,
        PRODUCT_TEST_TOKEN_MORPHOTYPE,
        PRODUCT_TEST_TOKEN_CF_MARK,
        PRODUCT_TEST_TOKEN_CF_MASK_TYPE,
        PRODUCT_TEST_TOKEN_RESULT_OTP_QRCODE,
        PRODUCT_TEST_TOKEN_SENSOR_BASE = 310000,
        PRODUCT_TEST_TOKEN_SENSOR_IDS,
        PRODUCT_TEST_TOKEN_SENSOR_X,
        PRODUCT_TEST_TOKEN_SENSOR_Y,
        PRODUCT_TEST_TOKEN_TOUCH_PRESS_X = PRODUCT_TEST_TOKEN_SENSOR_Y + 32,  //reserve 32 token for sensor coordinates
        PRODUCT_TEST_TOKEN_TOUCH_PRESS_Y,
        PRODUCT_TEST_TOKEN_LIGHT_RADIUS,
        PRODUCT_TEST_TOKEN_LIGHT_RADIUS_MM,
        PRODUCT_TEST_TOKEN_DISPLAY_CONTROL_STATE,
        PRODUCT_TEST_TOKEN_TOUCH_MAJOR,
        PRODUCT_TEST_TOKEN_TOUCH_MINOR,
        PRODUCT_TEST_TOKEN_TOUCH_ORIENTATION,
        PRODUCT_TEST_TOKEN_SENSOR_ROTATION
};

typedef enum
{
    PRODUCT_TEST_CMD_SPI = 8,  // 8
    PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN,
    PRODUCT_TEST_CMD_OTP_FLASH,
    PRODUCT_TEST_CMD_PERFORMANCE_TESTING,
    PRODUCT_TEST_CMD_PERFORMANCE_TESTING_RESULT,
    PRODUCT_TEST_CMD_CANCEL,
    PRODUCT_TEST_CMD_GET_OTP_INFO,
    PRODUCT_TEST_CMD_OPEN_HBM_MODE,
    PRODUCT_TEST_CMD_CLOSE_HBM_MODE,
    PRODUCT_TEST_CMD_SET_GAIN_TARGET,
    PRODUCT_TEST_CMD_GET_GAIN_TARGET,
    PRODUCT_TEST_CMD_SET_BRIGHTNESS_LEVEL,
    PRODUCT_TEST_CMD_AGE_START,  // 20
    PRODUCT_TEST_CMD_AGE_STOP,  // 21
    PRODUCT_TEST_CMD_IMAGE_QUALITY,
    PRODUCT_TEST_CMD_GET_VERSION,
    PRODUCT_TEST_CMD_SET_SENSOR_CONFIG,
    PRODUCT_TEST_CMD_GET_SENSOR_CONFIG,
    PRODUCT_TEST_CMD_CAPTURE_IMAGE,
    PRODUCT_TEST_CMD_MORPHOTYPE, // 27
    PRODUCT_TEST_CMD_SET_SAMPLING_CFG,
    PRODUCT_TEST_CMD_GET_SAMPLING_CFG,
    PRODUCT_TEST_CMD_GET_CALI_STATE,
    PRODUCT_TEST_CMD_GET_OTP_QRCODE,
    PRODUCT_TEST_CMD_DELMAR_MAX,
}
DELMAR_PRODUCT_TEST_CMD_ID;

#endif /* _FINGERPRINTCORE_H_ */
