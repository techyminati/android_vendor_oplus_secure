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
 **  <author>        <data>                   <desc>
 **  luhongyu      2018/10/24              create file
 **  ziqing.guo    2019/08/17              add for solve the problem of entering recovery
 **  Ziqing.Guo    2019/08/21              move hypnus to fingerprint common module
 **  Zemin.Li      2020/08/31              remove reboot action when init fail
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

#define PROPERTY_FINGERPRINT_FACTORY "oplus.fingerprint.factory"
#define PROPERTY_FINGERPRINT_FACTORY_ALGO_VERSION "oplus.fingerprint.gf.package.version"

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


namespace goodix
{
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
        }

    HalContext::~HalContext()
    {
        destroy();
    }

    gf_error_t HalContext::init()
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();
        deinit();
        LOG_E(LOG_TAG, "[%s], init with G2 HAL.", __func__);
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

            mCaEntry = new CaEntry();
            err = mCaEntry->startTap();
            GF_ERROR_BREAK(err);

#ifdef SUPPORT_DSP
            /*init dsp*/
            if (mDspSupport) {
                mDsp = new HalDsp(mCaEntry);
                err = mDsp->init();
                GF_ERROR_BREAK(err);
            }
#endif   // SUPPORT_DSP

            err = mDevice->open();
            GF_ERROR_BREAK(err);

            err = mDevice->enablePower();
            GF_ERROR_BREAK(err);

            err = mDevice->reset();
            GF_ERROR_BREAK(err);

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

            mSensor->wakeupSensor();

            setModuleInfo();
        }
        while (0);

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
        do
        {
            err = mDevice->reset();
            GF_ERROR_BREAK(err);
            err = mCaEntry->shutdownTap();
            GF_ERROR_BREAK(err);
            err = mCaEntry->startTap();
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
#ifdef SUPPORT_DSP
        //if (mDsp != nullptr) {
        //    mDsp->deinit();
        //}
#endif   // SUPPORT_DSP
        destroy();
        return GF_SUCCESS;
    }

    void HalContext::destroy()
    {
        if (mFingerprintCore != nullptr)
        {
            delete mFingerprintCore;
            mFingerprintCore = nullptr;
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
        if (mDspSupport && mDsp != nullptr) {
            delete mDsp;
            mDsp = nullptr;
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

