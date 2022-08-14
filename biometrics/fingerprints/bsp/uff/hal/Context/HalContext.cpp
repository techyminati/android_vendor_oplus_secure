/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/HalContext.cpp
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][HalContext]"

#include "HalContext.h"
#include <cutils/properties.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Device.h"
#include "HalLog.h"
#include "FpCommon.h"
#include "FingerprintConfig.h"
#include "FpType.h"
#ifdef FINGERPRINT_DEBUG_SOCKET
#include <thread>
#include "FingerprintSocket.h"
#endif

namespace android {
HalContext *HalContext::sInstance = nullptr;
HalContext::HalContext()
    : mFingerprintManager(nullptr),
      mFingerprintMessage(nullptr),
      mSensor(nullptr),
      mDevice(nullptr),
      mCaEntry(nullptr),
      mDcsInfo(nullptr),
      mDump(nullptr) {
    sInstance = this;
}

HalContext::~HalContext()
{
    HalContext::sInstance = nullptr;
}

void HalContext::preChipSelected(unsigned int mdelay)
{
    mDevice->controlPower(DEVICE_DISABLE_POWER);
    usleep(mdelay * 1000);
    mDevice->controlPower(DEVICE_ENABLE_POWER);
    usleep(mdelay * 1000);
    mDevice->reset();
    usleep(mdelay * 1000);
}

fp_return_type_t HalContext::doChipSelected(fp_ta_name_t *name)
{
    fp_return_type_t ret = FP_ERROR;
    FpCa *mChipCaEntry = nullptr;
    int index = 0;
    for (index = (int)FP_GOODIX_TA; index < FP_MAX_VENDOR_TA; index++) {
        mChipCaEntry = new FpCa(TEE_PLATFORM);
        if (!mChipCaEntry) {
            ret = FP_ERR_NULL_PTR;
            break;
        }
        preChipSelected(20);
        ret = mChipCaEntry->startTa(FP_SPI_TA);
        if (ret) {
            LOG_I(LOG_TAG, "[%s] load ta fail:%d, %d", __func__, ret, index);
            ret = FP_SELECT_LOAD_SPI_TA;
            break;
        }
        fp_chip_select_t chip;
        memset(&chip, 0, sizeof(fp_chip_select_t));
        chip.header.module_id = 0;
        chip.header.cmd_id = index;
        ret = mChipCaEntry->sendCommand(&chip, sizeof(fp_chip_select_t));
        mChipCaEntry->closeTa();
        if (mChipCaEntry) {
            delete mChipCaEntry;
            mChipCaEntry = nullptr;
        }
        if (ret == 0) {
            ret = FP_SUCCESS;
            LOG_I(LOG_TAG, "[%s] match sensor success, sensor, 0x%04x", __func__, chip.info.chip_id);
            break;
        }
    }
    if (index == FP_MAX_VENDOR_TA) {
        ret = FP_SELECT_CHIPID_ERROR;
        LOG_E(LOG_TAG, "[%s] can not match any sensor.", __func__);
    }
    if (mChipCaEntry) {
        delete mChipCaEntry;
        mChipCaEntry = nullptr;
    }
    *name = (fp_ta_name_t)index;
    return ret;
}

fp_return_type_t HalContext::init() {
    fp_return_type_t err    = FP_SUCCESS;
    fp_ta_name_t name = FP_SPI_UNKNOW;
    FUNC_ENTER();
    deinit();
#ifdef FINGERPRINT_DEBUG_SOCKET
    std::thread t1([&](){
        FingerprintSocket socket;
        socket.createSocket();
    });
    t1.detach();
#endif
    LOG_I(LOG_TAG, "[%s], init HAL.", __func__);
    //Config from system
    mConfig = new FingerprintConfig(this);
    err = mConfig->getBaseInfo();
    CHECK_RESULT_SUCCESS(err);
    //Device init
    mDevice = new Device(this);
    err = (fp_return_type_t)mDevice->open();
    CHECK_RESULT_SUCCESS(err);

    err = (fp_return_type_t)mDevice->controlPower(DEVICE_ENABLE_POWER);
    CHECK_RESULT_SUCCESS(err);

    err = (fp_return_type_t)mDevice->reset();
    CHECK_RESULT_SUCCESS(err);
    // Select chip
#ifndef TEE_REE
#ifdef FINGERPRINT_CHIPID_ENABLE
    err = doChipSelected(&name);
    CHECK_RESULT_SUCCESS(err);
#else
    name = FP_GOODIX_TA;
#endif
#endif

    mCaEntry = new FpCa(TEE_PLATFORM);
    err = mCaEntry->startTa(name);
    CHECK_RESULT_SUCCESS(err);

    //config from ta
    err = mConfig->init();
    CHECK_RESULT_SUCCESS(err);
    mConfig->setIconType();

    mDcsInfo = new DcsInfo(this);
    err = (fp_return_type_t)mDcsInfo->init();
    CHECK_RESULT_SUCCESS(err);

    mFacotryTest = FactoryTest::getInstance();
    err = (fp_return_type_t)mFacotryTest->init(this);
    CHECK_RESULT_SUCCESS(err);

    mPerf = Perf::getInstance();

    mFingerprintManager = new FingerprintManager(this);
    err = mFingerprintManager->init();
    CHECK_RESULT_SUCCESS(err);

    mFingerprintMessage = new FingerprintMessage(this);

    //create netlink thread
    err = (fp_return_type_t)mDevice->enable();
    CHECK_RESULT_SUCCESS(err);

    mAutoSmoking = new AutoSmoking(mFingerprintManager, FP_SENSOR_TYPE_G_G3);

fp_out:
    FUNC_EXIT(err);
    if (err != FP_SUCCESS) {
        deinit();
    }
    return err;
}

fp_return_type_t HalContext::reInit() {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t HalContext::deinit()
{
    if (mDevice != nullptr) {
        mDevice->controlPower(DEVICE_DISABLE_POWER);
        mDevice->close();
    }

    if (mDcsInfo != nullptr) {
        delete mDcsInfo;
        mDcsInfo = nullptr;
    }

    if (mDevice != nullptr) {
        delete mDevice;
        mDevice = nullptr;
    }

    if (mFingerprintManager != nullptr) {
        delete mFingerprintManager;
        mFingerprintManager = nullptr;
    }

    if (mCaEntry != nullptr) {
        mCaEntry->closeTa();
    }
    return FP_SUCCESS;
}

fp_return_type_t HalContext::invokeCommand(void *cmd, int32_t size) {
    UNUSED(cmd);
    UNUSED(size);
    return FP_SUCCESS;
}

}  // namespace android
