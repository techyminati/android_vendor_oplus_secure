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

#ifndef ENABLE_DSP_ENROLL
#define ENABLE_DSP_ENROLL                   0
#endif  // ENABLE_DSP_ENROLL
#define MAX_REINIT_TIMES_IF_TA_DEAD         3

namespace goodix {

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

    gf_error_t HalContext::init() {
        gf_error_t err = GF_SUCCESS;
        bool wakeup = false;
#ifdef SUPPORT_DSP_HAL
        bool dspInited = false;
#endif   // SUPPORT_DSP_HAL
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
            {  // NOLINT(660)
                // it's neccessary to enable spi clock to access spi for beanpod
                Device::AutoSpiClock _m(mDevice);
                err = mCaEntry->startTap(isDSPEnabled());
                GF_ERROR_BREAK(err);
            }
            HAL_CONTEXT_INJ_DATA;
            Sensor::resetWakeupFlag();
            err = Sensor::doWakeup(this);
            if (err != GF_SUCCESS) {
                break;
            }
            wakeup = true;
            SensorDetector detector(this);
            err = detector.init();
            GF_ERROR_BREAK(err);
            err = detector.detectSensor(&mSensorInfo);
            GF_ERROR_BREAK(err);
#ifdef SUPPORT_DSP_HAL
            // init dsp
            if (isDSPEnabled()) {
                createDsp();
                mDsp->init(mSensorInfo);
                dspInited = true;
            }
#endif   // SUPPORT_DSP_HAL
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
#ifdef SUPPORT_DSP_HAL
        // In the case of enabling DSP, if the previous process is not initialized to DSP,
        // it is necessary to initialize the DSP, because it is found on some terminals that
        // electric leakage may occur if the DSP is not initialized.
        if (isDSPEnabled() && !dspInited) {
            // init dsp
            createDsp();
            mDsp->init(mSensorInfo);
        }
#endif   // SUPPORT_DSP_HAL
        if (err != GF_SUCCESS) {
            deinit();
        } else {
            mInited = true;
        }

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
            {  // NOLINT(660)
                Device::AutoSpiClock _m(mDevice);
#ifdef SUPPORT_DSP_HAL
                err = mCaEntry->startTap(isDSPEnabled());
#else  // SUPPORT_DSP_HAL
                err = mCaEntry->startTap(false);
#endif   // SUPPORT_DSP_HAL
                GF_ERROR_BREAK(err);
            }
            Sensor::resetWakeupFlag();
            err = Sensor::doWakeup(this);
            if (err != GF_SUCCESS) {
                break;
            }
            wakeup = true;
#ifdef SUPPORT_DSP_HAL
            if (isDSPEnabled()) {
                if (mDsp == nullptr) {
                    createDsp();
                    mDsp->init(mSensorInfo);
                } else {
                    mDsp->reinit();
                }
            }
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
        if (mDsp != nullptr) {
            delete mDsp;
            mDsp = nullptr;
        }
#endif   // SUPPORT_DSP_HAL

        HAL_CONTEXT_STOP_TR;
    }

    void HalContext::createExtModules(void* callback) {
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
        HAL_CONTEXT_HEARTBEATRATE_CREATE

        module = createProductTest(this);
        if (module != nullptr) {
            LOG_D(LOG_TAG, "[%s] add ProductTest into list", __func__);
            // keep ProductTest at end of the list.
            // !!!!Do not use push_back for any other modules!!!!
            mExtModuleList.push_back(module);
            module->setNotify((ext_module_callback)callback);
            module->onInitFinished();
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
                } else {
                    LOG_D(LOG_TAG, "[%s] reinit times large thant: %d, do not reinit again.",
                            __func__, MAX_REINIT_TIMES_IF_TA_DEAD);
                }
                break;
            } else {
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

    bool HalContext::isDSPEnabled() {
#ifdef SUPPORT_DSP_HAL
        return true;
#else  // SUPPORT_DSP_HAL
        return false;
#endif  // SUPPORT_DSP_HAL
    }

    bool HalContext::isDSPValid() {
#ifdef SUPPORT_DSP_HAL
        if (nullptr != mDsp) {
            if (DSP_AVAILABLE == mDsp->checkDspValid()) {
                return true;
            }
        }
        return false;
#else  // SUPPORT_DSP_HAL
        return false;
#endif  // SUPPORT_DSP_HAL
    }

    // 1: enroll process use dsp
    // 0: enroll process do not use dsp
    uint8_t HalContext::isDSPEnroll() {
        return ENABLE_DSP_ENROLL;
    }

#ifdef SUPPORT_DSP_HAL
    HalDsp* HalContext::getDsp() {
        if (isDSPEnabled()) {
            if (mDsp == nullptr) {
                LOG_E(LOG_TAG, "[%s] mDsp is nullptr!", __func__);
            }
            LOG_V(LOG_TAG, "[%s] mDSP addr is %p", __func__, mDsp);
            return mDsp;
        }
        return nullptr;
    }

    void HalContext::createDsp() {
        if (mDsp == nullptr) {
            mDsp = new HalDsp(mCaEntry);
            LOG_V(LOG_TAG, "[%s] mDSP init: %p.", __func__, mDsp);
        }
    }
#endif  // SUPPORT_DSP_HAL
}  // namespace goodix
