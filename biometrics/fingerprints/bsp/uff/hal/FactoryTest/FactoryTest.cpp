/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/FactoryTest.cpp
 **
 ** Description:
 **      FactoryTest for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][FactoryTest]"

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include "FactoryTest.h"
#include "HalLog.h"
#include "fingerprint.h"
#include "HalContext.h"
#include "Dump.h"
#include "Utils.h"
#include <string.h>

#define ROOT_PATH "/mnt/vendor/persist/"
#define CALI_FILE_NAME "fingerprint_cali.dat"

namespace android {

FactoryTest* FactoryTest::sInstance = NULL;

FactoryTest::FactoryTest()
{
    LOG_D(LOG_TAG, "FactoryTest create");
}

int FactoryTest::init(HalContext *context)
{
    this->mContext = context;
    return 0;
}

void FactoryTest::storeLcdBrightness() {
    getBrightnessValue();
}

void FactoryTest::recoveryLcdState() {
    setBrightness(atoi(mBrightValue));
    setHbmMode(0);
}

void FactoryTest::getBrightnessValue() {
    int32_t length = 0;
    int index = 0;
    memset(mBrightValue, 0, sizeof(mBrightValue));

    for (index = 0; index < sizeof(gBrightnessPath)/sizeof(gBrightnessPath[0]); index ++) {
        if (access(gBrightnessPath[index].c_str(), 0) == 0) {
            LOG_E(LOG_TAG, "Brightness path index %d, path:%s", index, gBrightnessPath[index].c_str());
            break;
        }
    }
    if (index == sizeof(gBrightnessPath)/sizeof(gBrightnessPath[0])) {
        LOG_E(LOG_TAG, " no brightness path available");
        return;
    }
    int fd = open(gBrightnessPath[index].c_str(), O_RDONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, " setBrightness err:%d, errno =%d", fd, errno);
        return;
    }
    length = read(fd, mBrightValue, sizeof(mBrightValue));
    if (length > 0) {
        LOG_E(LOG_TAG, "get mBrightValue = %s  length = %d ", mBrightValue, length);
    } else {
        LOG_E(LOG_TAG, "read brightness value fail");
    }
    close(fd);
}

void FactoryTest::setHbmMode(uint32_t val)
{
    char buf[50] = {'\0'};
    int index = 0;
    int len = 0;

    for (index = 0; index < sizeof(gHbmPath)/sizeof(gHbmPath[0]); index ++) {
        if (access(gHbmPath[index].c_str(), 0) == 0) {
            LOG_E(LOG_TAG, "hbm path index %d, path:%s", index, gHbmPath[index].c_str());
            break;
        }
    }
    if (index == sizeof(gHbmPath)/sizeof(gHbmPath[0])) {
        LOG_E(LOG_TAG, "no hbm path available");
        return;
    }

    int fd = open(gHbmPath[index].c_str(), O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "open err :%d, errno =%d", fd, errno);
        return;
    }
    snprintf(buf, sizeof(buf), "%d", val);
    len = write(fd, buf, 50);
    LOG_I(LOG_TAG, "set hbm status len end:%d, val:%d", len, val);
    close(fd);
    usleep(51*1000);
#ifdef FP_HBM_BRIGHTNESS_DELAY
    usleep(200*1000);
#endif
    LOG_I(LOG_TAG, "set hbm delay end");
}

void FactoryTest::setBrightness(uint32_t val)
{
    char buf[50] = {'\0'};
    int32_t len = 0;

    int index = 0;
    for (index = 0; index < sizeof(gBrightnessPath)/sizeof(gBrightnessPath[0]); index ++) {
        if (access(gBrightnessPath[index].c_str(), 0) == 0) {
            LOG_E(LOG_TAG, "Brightness path index %d, path:%s", index, gBrightnessPath[index].c_str());
            break;
        }
    }
    if (index == sizeof(gBrightnessPath)/sizeof(gBrightnessPath[0])) {
        LOG_E(LOG_TAG, "no brightness path available");
        return;
    }

    int fd = open(gBrightnessPath[index].c_str(), O_WRONLY);
    if (fd < 0) {
        LOG_E(LOG_TAG, "setBrightness err:%d, errno =%d", fd, errno);
        return;
    }
    snprintf(buf, sizeof(buf), "%d", val);
    len = write(fd, buf, 50);
    LOG_I(LOG_TAG, "write len %d, val:%d", len, val);
    close(fd);
    usleep(51*1000);
#ifdef FP_HBM_BRIGHTNESS_DELAY
    usleep(200*1000);
#endif
    LOG_I(LOG_TAG, "setBrightness delay end");
}

fp_return_type_t FactoryTest::sendFingerprintCmdToTA(int32_t cmd_id, vnd_code_t *vnd_code_p)
{
    fp_return_type_t ret = FP_SUCCESS;
    fp_factory_t data;
    memset(&data, 0, sizeof(fp_factory_t));
    data.header.module_id = FP_MODULE_PRODUCT_TEST;
    data.header.cmd_id = cmd_id;
    memcpy(&data.vnd_code, vnd_code_p, sizeof(*vnd_code_p));
    data.vnd_code.cmd_id = (int32_t)cmd_id;
    ret = HalContext::getInstance()->mCaEntry->sendCommand(&data, sizeof(fp_factory_t));
    if (ret) {
        LOG_E(LOG_TAG, "sendFingerprintCmdToTA, err:%d", ret);
        return FP_ERR_SEND_CMD_TO_TA;
    }
    memcpy(vnd_code_p, &data.vnd_code, sizeof(data.vnd_code));
    return ret;
}

fp_return_type_t FactoryTest::notifyFactoryResult(int32_t cmd_id, int result, char *buffer)
{
    char message[128] = {0};
    LOG_E(LOG_TAG, "result:%d, description:%s", result, buffer);
    sprintf(message, "result:%d;description:%s", result, buffer);
    return HalContext::getInstance()->mFingerprintManager->notifyFingerprintCmd(0,
        cmd_id, (const int8_t *)message, strlen(message));
}

fp_return_type_t FactoryTest::sendFingerprintCmd(int32_t cmd_id, int8_t* in_buf, uint32_t size)
{
    (void)in_buf;
    (void)size;
    fp_return_type_t ret = FP_SUCCESS;
    vnd_code_t vnd_code;
    switch (cmd_id) {
    case FINGERPRINT_FACTORY_AUTO_TEST:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_AUTO_TEST, &vnd_code);
        break;
    case FINGERPRINT_FACTORY_QTY_TEST:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_QTY_TEST, &vnd_code);
        break;
    case FINGERPRINT_FACTORY_GET_ALGO_INFO:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_GET_ALGO_INFO, &vnd_code);
        break;
    case FINGERPRINT_FACTORY_GET_SENSOR_QRCODE:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_GET_SENSOR_QRCODE, &vnd_code);
        break;
    case FINGERPRINT_FACTORY_AGING_TEST:
        setHbmMode(1);
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_AGING_TEST, &vnd_code);
        setHbmMode(0);
        break;
    case FINGERPRINT_FACTORY_CALI_FLESH_BOX_TEST:
        storeLcdBrightness();
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_CALI_FLESH_BOX_TEST, &vnd_code);
        break;
    case FINGERPRINT_FACTORY_CALI_BLACK_BOX_TEST:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_CALI_BLACK_BOX_TEST, &vnd_code);
        break;
    case FINGERPRINT_FACTORY_CALI_CHART_TEST:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_FACTORY_CALI_CHART_TEST, &vnd_code);
        break;
    case FINGERPRINT_CAPTURE_TOOL_GET_IMG:
        ret = __decorate(sendFingerprintCmdToTA, vnd_code, E_CAPTURE_TOOL_GET_IMG, &vnd_code);
        break;
    default:

        break;
    }

    if (notifyFactoryResult(cmd_id, ret, vnd_code.vnd_description)) {
        LOG_E(LOG_TAG, "notify factory result not correct");
    }

    if (cmd_id == FINGERPRINT_FACTORY_CALI_CHART_TEST) {
        recoveryLcdState();
    }

    if (ret) {
        sendFingerprintCmdToTA(E_FACTORY_CALI_ABORT, &vnd_code);
    }
    return ret;
}

FactoryTest::~FactoryTest()
{
    if (sInstance) {
        delete sInstance;
    }
}

};  // namespace android
