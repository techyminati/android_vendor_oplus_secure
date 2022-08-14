/************************************************************************************
 ** File: - HalContext.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HalContext for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,24/10/2018
 ** Author: oujinrong@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>               <desc>
 **  luhongyu         2018/10/24           create file
 **  Bangxiong.Wu     2019/05/23           Add for algo info property
 **  Qijia.Zhou       2019/06/04           Add for goodix dump data
 **  ziqing.guo       2019/08/17           add for solve the problem of entering recovery
 **  Ziqing.Guo       2019/08/21           move hypnus to fingerprint common module
 **  Ran.Chen         2019/10/22           modify max reinit times if TA dead to 5
 **  Zemin.Li         2020/08/31           remove reboot action when init fail
 ************************************************************************************/

#define LOG_TAG "[GF_HAL][HalContext]"
#include <cutils/properties.h>
#include "HalContext.h"
#include "CoreCreator.h"
#include "Sensor.h"
#include "Algo.h"
#include "SensorDetector.h"
#include "Device.h"
#include "HalLog.h"
#include "CaEntry.h"
#ifdef SUPPORT_TRACE
#include "Tracer.h"
#endif  // SUPPORT_TRACE
#ifdef SUPPORT_DUMP
#include "HalDump.h"
#endif  // SUPPORT_DUMP
#ifdef SUPPORT_AUTO_TEST
#include "HalMockDevice.h"
#endif  // SUPPORT_AUTO_TEST
#include "SZCustomizedProductTest.h"
#include "gf_sz_types.h"
#include "HalDsp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define MAX_REINIT_TIMES_IF_TA_DEAD         (5)
#define PROPERTY_FINGERPRINT_FACTORY "oplus.fingerprint.factory"

static gf_module_info_t module_info_list[] = {
    /* module, lens, module_info_string */
    {0, 0, "G_Quote"},
    {1, 0, "G_Ofilm"},
    {2, 0, "G_Truly"},
    {3, 0, "G_Primax"},
};

#ifdef SUPPORT_DSP
bool mDspSupport = true;
#endif

extern uint32_t gDumpLevelVal;

namespace goodix
{
        HalContext* HalContext::sInstance = nullptr;
    HalContext::HalContext() :
        mFingerprintCore(nullptr),
        mSensor(nullptr),
        mAlgo(nullptr),
        mDevice(nullptr),
        mCenter(nullptr),
        mCaEntry(nullptr),
        mConfig(nullptr),
        mDsp(nullptr)
#ifdef SUPPORT_DUMP
        , mHalDump(nullptr)
#endif   // SUPPORT_DUMP
        {
            mSensorInfo.chip_series = GF_SHENZHEN;
            mSensorInfo.chip_type = 0;
            mSensorInfo.col = 0;
            mSensorInfo.row = 0;
            sInstance = this;
        }

    HalContext::~HalContext()
    {
        destroy();
        HalContext::sInstance = nullptr;
    }

    gf_error_t HalContext::init()
    {
        gf_error_t err = GF_SUCCESS;
        bool wakeup = false;
        uint32_t init_flag = 1;
        FUNC_ENTER();
        deinit();
        LOG_E(LOG_TAG, "[%s], init with G3 HAL.", __func__);
        do
        {
#ifdef DYNAMIC_SUPPORT_DSP
            err = checkDspSupport(&mDspSupport);
            //GF_ERROR_BREAK(err);
#endif   //DYNAMIC_SUPPORT_DSP
#ifdef SUPPORT_TRACE
            startTracer(this);
#endif  // SUPPORT_TRACE
#ifdef SUPPORT_AUTO_TEST
            mDevice = new HalMockDevice(this);
#else  // SUPPORT_AUTO_TEST
            mDevice = createDevice(this);
#endif  // SUPPORT_AUTO_TEST

#ifdef OPLUS_SUPPORT_STABILITYTEST
            mStabilityTest = new StabilityTest;
#endif // SUPPOR STABILITYTEST
            mCaEntry = new CaEntry();
            err = mCaEntry->startTap();
            GF_ERROR_BREAK(err);

#ifdef SUPPORT_DSP
            if (mDspSupport) {
                /*init dsp*/
                mDsp = new HalDsp(mCaEntry);
                err = mDsp->init();
                //GF_ERROR_BREAK(err);
            }
#endif   // SUPPORT_DSP

            err = mDevice->open();
            GF_ERROR_BREAK(err);

            err = mDevice->enablePower();
            GF_ERROR_BREAK(err);

            err = mDevice->reset();
            GF_ERROR_BREAK(err);

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

            // FIXME: follow reset ?
            err = mDevice->enable();
            GF_ERROR_BREAK(err);
            // new builder by series
            mSensor = createSensor(this);
            err = mSensor->init(&mSensorId);
            GF_ERROR_BREAK(err);

            mAlgo = createAlgo(this);
            err = mAlgo->init();
            GF_ERROR_BREAK(err);

            mFingerprintCore = createFingerprintCore(this);
            err = mFingerprintCore->init();
            mFingerprintCore->init_report_data();
            GF_ERROR_BREAK(err);

            mFingerprintCore->getDumpPictureLevel();

#ifdef SUPPORT_DUMP
            //property_set(PROPERTY_DUMP_DATA, "1");
            mHalDump = createHalDump(this);
            GF_ERROR_BREAK(err);
            err = mHalDump->init();
            GF_ERROR_BREAK(err);
            mConfig->support_performance_dump = 1;
#else
            mConfig->support_performance_dump = 0;
#endif   // SUPPORT_DUMP
            mCenter = new EventCenter(this);
            err = mCenter->init();
            GF_ERROR_BREAK(err);
            mMsgBus.sendMessage(MsgBus::MSG_DEVICE_INIT_END, init_flag);

            mDcsInfo = new DcsInfo(this);
            mDcsInfo->init();

            mSensor->wakeupSensor();
            setModuleInfo();

            if (gDumpLevelVal == 0) {
                mFingerprintCore->setImageDumpFlag(1);
            } else if (gDumpLevelVal == 1) {
                property_set(PROPERTY_DUMP_DATA, "1");
            }
        }
        while (0);

        if (wakeup) {
            Sensor::doSleep(this);
        }
        if (err != GF_SUCCESS) {
            deinit();
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContext::reInit()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        bool wakeup = false;
        do
        {
            err = mDevice->reset();
            GF_ERROR_BREAK(err);
            err = mCaEntry->shutdownTap();
            GF_ERROR_BREAK(err);
            err = mCaEntry->startTap();
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
            err = mSensor->init(&mSensorId);
            GF_ERROR_BREAK(err);
            err = mAlgo->init();
            GF_ERROR_BREAK(err);
            err = mFingerprintCore->init();
            GF_ERROR_BREAK(err);
#ifdef SUPPORT_DUMP
            err = mHalDump->init();
            GF_ERROR_BREAK(err);
#endif   // SUPPORT_DUMP
#ifdef SUPPORT_DSP
            if (mDspSupport) {
                err = mDsp->reinit();
                GF_ERROR_BREAK(err);
            }
#endif   // SUPPORT_DSP
        }
        while (0);

        if (wakeup) {
            Sensor::doSleep(this);
        }
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalContext::deinit()
    {
        if (mDevice != nullptr)
        {
            mDevice->remove();
            mDevice->close();
        }
        if (mCaEntry != nullptr)
        {
            mCaEntry->shutdownTap();
        }
        destroy();
        return GF_SUCCESS;
    }

    void HalContext::destroy()
    {
        if (mFingerprintCore != nullptr)
        {
            delete mFingerprintCore;
        }
        if (mSensor != nullptr)
        {
            delete mSensor;
            mSensor = nullptr;
        }
        if (mDevice != nullptr)
        {
            delete mDevice;
            mDevice = nullptr;
        }
        if (mCenter != nullptr)
        {
            delete mCenter;
            mCenter = nullptr;
        }
        if (mCaEntry != nullptr)
        {
            delete mCaEntry;
            mCaEntry = nullptr;
        }

        if (mConfig != nullptr)
        {
            delete mConfig;
            mConfig = nullptr;
        }
#ifdef SUPPORT_DUMP
        if (mHalDump != nullptr)
        {
            delete mHalDump;
            mHalDump = nullptr;
        }
#endif   // SUPPORT_DUMP

#ifdef SUPPORT_TRACE
        stopTracer(this);
#endif  // SUPPORT_TRACE

#ifdef SUPPORT_DSP
        if (mDspSupport) {
            if (mDsp != nullptr) {
                delete mDsp;
                mDsp = nullptr;
            }
        }
#endif   // SUPPORT_DSP
    }

int HalContext::setModuleInfo() {
    uint32_t i = 0;
    SZCustomizedProductTest *szTest = NULL;
    gf_sensor_mt_info_t *mtInfo = NULL;
    uint32_t mtInfoLen = 0;
    uint32_t moduleType = 0;
    uint32_t lensType = 0;
    uint32_t moduleInfoSize = 0;
    char lensTypeStr[2] = { 0 };
    char moduleInfoStr[100] = { 0 };

    do {
        szTest = new SZCustomizedProductTest(this);
        if (NULL == szTest) {
            LOG_E(LOG_TAG, "[%s] failed to get szTest", __func__);
            break;
        }
        szTest->getMTInfo((void **)&mtInfo, &mtInfoLen);
        mSensor->sleepSensor();

        if (NULL != mtInfo)
        {
            moduleType = mtInfo->flash_vendor_id >> 4;
            lensType = mtInfo->flash_vendor_id & 0x0f;
            LOG_E(LOG_TAG, "[%s] moduleType %d, lensType %d", __func__, moduleType, lensType);
        }

        moduleInfoSize = sizeof(module_info_list)/sizeof(module_info_list[0]);

        for (i = 0; i < moduleInfoSize; i++) {
            if (moduleType == module_info_list[i].module) {
                break;
            }
        }

        if (i == moduleInfoSize) {
            property_set(PROPERTY_FINGERPRINT_FACTORY, "Unknown");
            LOG_E("[%s] unkown module", __func__);
            break;
        }

        sprintf(lensTypeStr, "%d", lensType);
        strncpy(moduleInfoStr, module_info_list[i].module_info_string, strlen(module_info_list[i].module_info_string));
        strcat(moduleInfoStr, lensTypeStr);

        if (mSensor->mModuleType == 3) {
            strcat(moduleInfoStr, "_G2.4");
        } else if (mSensor->mModuleType == 5) {
            strcat(moduleInfoStr, "_G3.0");
        } else {
            strcat(moduleInfoStr, "_G2");
        }
        property_set(PROPERTY_FINGERPRINT_FACTORY, moduleInfoStr);

    } while(0);


    if (mtInfo != NULL) {
        free(mtInfo);
        mtInfo = NULL;
    }
    if (szTest != NULL) {
        delete szTest;
    }
    return GF_SUCCESS;
}
    gf_error_t HalContext::invokeCommand(void *cmd, int32_t size) {
        static uint32_t reinitTimesIfTaDead = 0;  // invoke command
        gf_error_t err = GF_SUCCESS;
        Mutex::Autolock _l(mSensorLock);
        Device::AutoSpiClock _m(mDevice);

        do {
            gf_cmd_header_t *header = (gf_cmd_header_t *) cmd;
            header->reset_flag = 0;
            mDevice->hold_wakelock(GF_HOLD_WAKE_LOCK);
            err = mCaEntry->sendCommand(cmd, size);
            mDevice->hold_wakelock(GF_RELEASE_WAKE_LOCK);
            if (err == GF_ERROR_TA_DEAD) {
                LOG_E(LOG_TAG, "[%s] TA DEAD", __func__);
                mMsgBus.sendMessage(MsgBus::MSG_TA_DEAD);
                reinitTimesIfTaDead++;
                if (reinitTimesIfTaDead <= MAX_REINIT_TIMES_IF_TA_DEAD) {
                reInit();
                } else {
                    LOG_E(LOG_TAG, "[%s] reinit times large thant: %d, do not reinit again, exit!",
                            __func__, MAX_REINIT_TIMES_IF_TA_DEAD);
                    exit(0);
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
#ifdef DYNAMIC_SUPPORT_DSP
    gf_error_t HalContext::checkDspSupport(bool *mDspSupport) {
        static const char* max_gpuclk_path = "/sys/class/kgsl/kgsl-3d0/max_gpuclk";
        char max_gpuclk_buff[16];
        int32_t max_gpuclk = 0, AA_max_gpuclk = 610000000, AB_max_gpuclk = 700000000;//AA not support DSP;AB support DSP.
        int32_t fd = 0, length = 0;

        /*read max_gpuclk*/
        memset(max_gpuclk_buff, 0, 16);
        fd = open(max_gpuclk_path, O_RDONLY);
        if (fd < 0) {
            LOG_E(LOG_TAG, "[%s] open max_gpuclk node failed: fd = %d, errno = %d!", __func__, fd, errno);
            return GF_ERROR_INVALID_DATA;
        }
        length = read(fd, max_gpuclk_buff, 9);
        if (length > 0) {
            max_gpuclk = atoi(max_gpuclk_buff);
            LOG_D(LOG_TAG, "[%s] max_gpuclk = %d.", __func__, max_gpuclk);
        } else {
            LOG_E(LOG_TAG, "[%s] read max_gpuclk node failed, length = %d!", __func__, length);
            goto err;
        }

        /*checkDspSupport by max_gpuclk*/
        if (max_gpuclk == AB_max_gpuclk) {
            *mDspSupport = true;
        } else if (max_gpuclk == AA_max_gpuclk) {
            *mDspSupport = false;
        } else {
            LOG_E(LOG_TAG, "[%s] max_gpuclk maybe not correct, max_gpuclk = %d!", __func__, max_gpuclk);
            *mDspSupport = true;
        }
        LOG_D(LOG_TAG, "[%s] mDspSupport = %d.", __func__, *mDspSupport);

        close(fd);
        return GF_SUCCESS;

err:
        close(fd);
        return GF_ERROR_INVALID_DATA;
    }
#endif   //DYNAMIC_SUPPORT_DSP
}   // namespace goodix

