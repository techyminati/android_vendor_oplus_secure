/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define _HALCONTEXT_CPP_
#define LOG_TAG "[GF_HAL][HalContext]"

#include "HalContext.h"
#include "CoreCreator.h"
#include "Sensor.h"
#include "Algo.h"
#include "SensorDetector.h"
#include "Device.h"
#include "ProductTest.h"
#include "HalLog.h"
#include "CaEntry.h"
#ifdef SUPPORT_DSP_HAL
#include "HalDsp.h"
#endif  // SUPPORT_DSP_HAL
#ifdef SUPPORT_DUMP
#include "HalDump.h"
#endif  // SUPPORT_DUMP
#include "ExtModuleBase.h"
#include "ProductTest.h"
#include "Thread.h"
#include <cutils/properties.h>

#define MAX_REINIT_TIMES_IF_TA_DEAD         3
#define OPLUS_ENROLL_TIME_OUT (10 * 60 * 1000)
#define ENROLL_ERROR_MAX (10)
#define IMAGE_PASS_SCORE (20)

namespace goodix {

    gf_fingerprint_notify_t HalContext::factory_notify = nullptr;
    HalContext::HalContext() :
        mFingerprintCore(nullptr),
        mSensor(nullptr),
        mAlgo(nullptr),
        mDevice(nullptr),
        mCenter(nullptr),
        mCaEntry(nullptr),
        mConfig(nullptr),
#ifdef SUPPORT_DUMP
        mHalDump(nullptr),
#endif   // SUPPORT_DUMP
        mInitFinishThread(nullptr)
#ifdef SUPPORT_DSP_HAL
        , mDsp(nullptr)
#endif  // SUPPORT_DSP_HAL

    {  // NOLINT(660)
        HAL_CONTEXT_CREATE_INJ;
        mSensorInfo.chip_series = GF_SHENZHEN;
        mSensorInfo.chip_type = 0;
        mSensorInfo.col = 0;
        mSensorInfo.row = 0;
        mExtModuleList.clear();
        mInitFinishThread = new InitFinishThread(*this);
        mInited = false;
    }

    HalContext::~HalContext() {
        destroy();
        delete mInitFinishThread;
        HAL_CONTEXT_DELETE_INJ;
    }

    gf_error_t HalContext::notifyFingerprintCmd(int64_t devId, int32_t msgId, int32_t cmdId, const int8_t *result, int32_t resultLen) {
        UNUSED_VAR(devId);
        UNUSED_VAR(msgId);
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_E(LOG_TAG, "[%s] enter notifyFingerprintCmd, return GF_SUCCESS cmdId = %d", __func__, cmdId);

        if( (cmdId != PRODUCT_TEST_CMD_SPI) &&
            (cmdId != PRODUCT_TEST_CMD_RESET_INTERRUPT_PIN) &&
            (cmdId != PRODUCT_TEST_CMD_OTP_FLASH) &&
            (cmdId != PRODUCT_TEST_CMD_PERFORMANCE_TESTING) &&
            (cmdId != PRODUCT_TEST_CMD_AGE_START) &&
            (cmdId != PRODUCT_TEST_CMD_AGE_START) &&
            (cmdId != PRODUCT_TEST_CMD_IMAGE_QUALITY) &&
            (cmdId != PRODUCT_TEST_CMD_GET_VERSION) &&
            (cmdId != PRODUCT_TEST_CMD_GET_OTP_QRCODE)) {
            LOG_E(LOG_TAG, "[%s] cmdId = %d, cmdId is not PRODUCT_TEST_CMD, do nothing!!!", __func__, cmdId);
            return GF_SUCCESS;
        }

        if (factory_notify != nullptr) {
            gf_fingerprint_msg_t message;
            memset(&message, 0, sizeof(gf_fingerprint_msg_t));
            switch (cmdId) {
                case PRODUCT_TEST_CMD_IMAGE_QUALITY: {
                    //decode image quality test error code and image quality
                    uint32_t error_code = GF_SUCCESS;
                    uint32_t image_quality = 0;
                    memcpy(&error_code, result + 4, sizeof(uint32_t));
                    if (GF_SUCCESS == error_code) {
                        memcpy(&image_quality, result + 12, sizeof(uint32_t));
                    }
                    message.type = GF_FINGERPRINT_ENGINEERING_INFO;
                    message.data.engineering.type = GF_FINGERPRINT_IMAGE_QUALITY;
                    message.data.engineering.quality.successed = (error_code == GF_SUCCESS ? 1 : 0);
                    message.data.engineering.quality.image_quality = image_quality;
                    message.data.engineering.quality.quality_pass = (message.data.engineering.quality.image_quality > IMAGE_PASS_SCORE ? 1 : 0);

                    LOG_E(LOG_TAG, "[%s] GF_FINGERPRINT_ENGINEERING_INFO, cmdId = %d, error_code = %d, resultLen = %d, image_quality = %d",
                        __func__, cmdId, error_code, resultLen, image_quality);
                    break;
                }

                default: {
                message.type = GF_FINGERPRINT_OPTICAL_SENDCMD;
                message.data.test.cmd_id = cmdId;
                message.data.test.result = (int8_t *) result;
                message.data.test.result_len = resultLen;
                break;
                }
            }

            LOG_D(LOG_TAG, "[%s] factory_notify success, cmdId = %d, result = %d, result len = %d", __func__, cmdId, result, resultLen);
            factory_notify(&message);
        }
        else {
            LOG_E(LOG_TAG, "[%s]factory_notify is null", __func__);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContext::init() {
        gf_error_t err = GF_SUCCESS;
        bool wakeup = false;
        FUNC_ENTER();
        deinit();
        mInited = false;

        do {
            HAL_CONTEXT_START_TR;
            HAL_CONTEXT_CREATE_DEV;
            err = mDevice->open();
            GF_ERROR_BREAK(err);
            err = mDevice->enablePower();
            GF_ERROR_BREAK(err);
            err = mDevice->reset();
            GF_ERROR_BREAK(err);
            mCaEntry = new CaEntry();
            err = mCaEntry->startTap();
            GF_ERROR_BREAK(err);
            HAL_CONTEXT_INJ_DATA;
            Sensor::resetWakeupFlag();
            err = Sensor::doWakeup(this);
            if (err != GF_SUCCESS) {
                break;
            }
            wakeup = true;
#ifdef SUPPORT_DSP_HAL
            /*init dsp*/
            createDsp();
            mDsp->init();
            // GF_ERROR_BREAK(err);
#endif   // SUPPORT_DSP_HAL
            SensorDetector detector(this);
            err = detector.init();
            GF_ERROR_BREAK(err);
            err = detector.detectSensor(&mSensorInfo);
            GF_ERROR_BREAK(err);
            err = mDevice->enable();
            GF_ERROR_BREAK(err);
            mSensor = createSensor(this);
            err = mSensor->init();
            GF_ERROR_BREAK(err);
            mAlgo = createAlgo(this);
            err = mAlgo->init();
            GF_ERROR_BREAK(err);
            HAL_CONTEXT_INJ_MOCK_DATA;
            mFingerprintCore = createFingerprintCore(this);
            err = mFingerprintCore->init();
            GF_ERROR_BREAK(err);
#ifdef SUPPORT_DUMP
            property_set(PROPERTY_DUMP_DATA, "1");
            mHalDump = createHalDump(this);
            GF_ERROR_BREAK(err);
            err = mHalDump->init();
            GF_ERROR_BREAK(err);
#endif   // SUPPORT_DUMP
            mCenter = new EventCenter(this);
            err = mCenter->init();
            GF_ERROR_BREAK(err);
            mMsgBus.sendMessage(MsgBus::MSG_DEVICE_INIT_END);
            mInitFinishThread->run("InitFinish");
        } while (0);
        if (wakeup) {
            Sensor::doSleep(this);
        }
        if (err != GF_SUCCESS) {
            deinit();
        } else {
            mInited = true;
        }
		createExtModules();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContext::reInit() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        bool wakeup = false;
        do {
            if (!mInited) {
                err = GF_ERROR_MODULE_NOT_INITED;
                break;
            }
            err = mDevice->reset();
            GF_ERROR_BREAK(err);
            err = mCaEntry->shutdownTap();
            GF_ERROR_BREAK(err);
            err = mCaEntry->startTap();
            GF_ERROR_BREAK(err);
            Sensor::resetWakeupFlag();
            err = Sensor::doWakeup(this);
            if (err != GF_SUCCESS) {
                break;
            }
            wakeup = true;
#ifdef SUPPORT_DSP_HAL
            mDsp->reinit();
            // GF_ERROR_BREAK(err);
#endif   // SUPPORT_DSP_HAL
            SensorDetector detector(this);
            err = detector.init();
            GF_ERROR_BREAK(err);
            err = detector.detectSensor(&mSensorInfo);
            GF_ERROR_BREAK(err);
            err = mSensor->init();
            GF_ERROR_BREAK(err);
            err = mAlgo->init();
            GF_ERROR_BREAK(err);
            err = mFingerprintCore->reInit();
            GF_ERROR_BREAK(err);
#ifdef SUPPORT_DUMP
            err = mHalDump->reInit();
            GF_ERROR_BREAK(err);
#endif   // SUPPORT_DUMP
            mInitFinishThread->run("InitFinish");
        } while (0);
        // FIXME: if doSleep not in thread, read data will happen error in init finish event
        if (wakeup) {
            Sensor::doSleep(this);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContext::deinit() {
        if (mDevice != nullptr) {
            mDevice->remove();
            mDevice->close();
        }

        if (mCaEntry != nullptr) {
            mCaEntry->shutdownTap();
        }

#ifdef SUPPORT_DSP_HAL
        if (mDsp != nullptr) {
            mDsp->deinit();
        }
#endif   // SUPPORT_DSP_HAL

        destroy();
        return GF_SUCCESS;
    }

    void HalContext::destroy() {
        if (mExtModuleList.size() > 0) {
            Vector<ExtModuleBase*>::iterator it;
            for (it = mExtModuleList.begin(); it != mExtModuleList.end(); it++) {
                if (*it != nullptr) {
                    delete (ExtModuleBase*)*it;
                }
            }
            mExtModuleList.clear();
        }

        if (mFingerprintCore != nullptr) {
            delete mFingerprintCore;
            mFingerprintCore = nullptr;
        }

        if (mSensor != nullptr) {
            delete mSensor;
            mSensor = nullptr;
        }

        if (mDevice != nullptr) {
            delete mDevice;
            mDevice = nullptr;
        }

        if (mCenter != nullptr) {
            delete mCenter;
            mCenter = nullptr;
        }

        if (mCaEntry != nullptr) {
            delete mCaEntry;
            mCaEntry = nullptr;
        }

        if (mConfig != nullptr) {
            delete mConfig;
            mConfig = nullptr;
        }

#ifdef SUPPORT_DUMP

        if (mHalDump != nullptr) {
            delete mHalDump;
            mHalDump = nullptr;
        }

#endif   // SUPPORT_DUMP

#ifdef SUPPORT_DSP_HAL
        usleep(50*1000);
        if (mDsp != nullptr) {
            delete mDsp;
            mDsp = nullptr;
        }
#endif   // SUPPORT_DSP_HAL

        HAL_CONTEXT_STOP_TR;
    }

    void HalContext::createExtModules() {
        VOID_FUNC_ENTER();
        ExtModuleBase* module = nullptr;
        if (!mExtModuleList.isEmpty()) {
            LOG_E(LOG_TAG, "[%s] module list is not empty, size=%d",
                    __func__, (int32_t)mExtModuleList.size());
            return;
        }
        HAL_CONTEXT_EXT_TOOLS_CREATE
        HAL_CONTEXT_DEBUG_TOOLS_CREATE
        HAL_CONTEXT_FRRFAR_TOOLS_CREATE

        module = createProductTest(this);
        if (module != nullptr) {
            LOG_D(LOG_TAG, "[%s] add ProductTest into list", __func__);
            // keep ProductTest at end of the list.
            // !!!!Do not use push_back for any other modules!!!!
            mExtModuleList.push_back(module);
            module->setNotify((ext_module_callback)notifyFingerprintCmd);
        }
        LOG_D(LOG_TAG, "[%s] module list size=%d", __func__, (int32_t)mExtModuleList.size());
        VOID_FUNC_EXIT();
    }

    gf_error_t HalContext::onExtModuleCommand(int32_t cmdId, const int8_t *in, uint32_t inLen,
                          int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_ERROR_UNKNOWN_CMD;
        ExtModuleBase* module = NULL;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] module list size=%d", __func__, (int32_t)mExtModuleList.size());
        do {
            if (!mExtModuleList.isEmpty()) {
                Vector<ExtModuleBase*>::iterator it;
                for (it = mExtModuleList.begin(); it != mExtModuleList.end(); it++) {
                    module = *it;
                    err = module->onCommand(cmdId, in, inLen, out, outLen);
                    if (err != GF_ERROR_UNKNOWN_CMD) {
                        break;
                    }
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContext::invokeCommand(void *cmd, int32_t size) {
        static uint32_t reinitTimesIfTaDead = 0;  // invoke command
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mSensorLock);
        Device::AutoSpiClock _m(mDevice);

        do {
            gf_cmd_header_t *header = (gf_cmd_header_t *) cmd;
            header->reset_flag = 0;
            err = mCaEntry->sendCommand(cmd, size);

            if (err == GF_ERROR_TA_DEAD) {
                LOG_E(LOG_TAG, "[%s] TA DEAD", __func__);
                mMsgBus.sendMessage(MsgBus::MSG_TA_DEAD);
                reinitTimesIfTaDead++;
                if (reinitTimesIfTaDead <= MAX_REINIT_TIMES_IF_TA_DEAD) {
                    reInit();
                }else {
                    LOG_D(LOG_TAG, "[%s] reinit times large than %d, do not reinit again.", __func__, MAX_REINIT_TIMES_IF_TA_DEAD);
                }
                break;
            }else{
                reinitTimesIfTaDead = 0;
            }

            if (GF_SUCCESS == err) {
                err = header->result;
            }

            if (header->reset_flag > 0) {
                LOG_D(LOG_TAG, "[%s] reset_flag > 0, reset chip", __func__);
                mDevice->reset();
            }
        }
        while (0);
        LOG_D(LOG_TAG, "[%s] err = %d, errno = %s", __func__, err, gf_strerror(err));
        return err;
    }

    InitFinishThread::InitFinishThread(HalContext& context): mContext(context) {
    }

    InitFinishThread::~InitFinishThread() {
    }

    bool InitFinishThread::threadLoop() {
        gf_init_finish_cmd_t cmd = { 0 };
        cmd.target = GF_TARGET_BIO;
        cmd.cmd_id = GF_CMD_AUTH_INIT_FINISHED;
        mContext.invokeCommand(&cmd, sizeof(gf_init_finish_cmd_t));
        LOG_D(LOG_TAG, "[%s] Init finished thead exit.", __func__);
        return false;
    }

#ifdef SUPPORT_DSP_HAL
    HalDsp* HalContext::getDsp() {
        if (mDsp == nullptr) {
            LOG_E(LOG_TAG, "[%s] mDsp is nullptr!", __func__);
        }
        return mDsp;
    }

    void HalContext::createDsp() {
        if (mDsp == nullptr) {
            mDsp = new HalDsp(mCaEntry);
        }
    }
#else
    HalDsp* HalContext::getDsp() {
        LOG_E(LOG_TAG, "[%s] mDsp is nullptr!", __func__);
        return nullptr;
    }

    void HalContext::createDsp() {
        LOG_E(LOG_TAG, "[%s] mDsp is nullptr!", __func__);
    }

#endif  // SUPPORT_DSP_HAL
}  // namespace goodix
