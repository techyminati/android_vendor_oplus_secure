/************************************************************************************
 ** File: - goodix\fingerprint\fingerprint_hwbinder\OpticalFingerprint.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix(android Q)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,15/08/2019
 ** Author: Ziqing.Guo@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ziqing.Guo       2019/08/15       create file for goodix optical android Q (Euclid)
 ************************************************************************************/
#define LOG_TAG "FingerprintHal"
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <log/log.h>
#include <hardware/hardware.h>

#include "HalContext.h"
#include "HalContextExt.h"
#include "FingerprintCore.h"
#include "RegisterService.h"

#include "../../hwbinder/fingerprint.h"
using namespace goodix;

using goodix::HalContext;
using goodix::HalContextExt;
using goodix::FingerprintCore;

static HalContext* gpHalContext = nullptr;  // hal context

/**
 * Function: close
 * Description: close device
 * Input: pDev
 * Output: none
 * Return:
 * 0                  if succeed
 * FINGERPRINT_ERROR  if fail
 */
static int32_t close(hw_device_t __unused *pDev)
{
    ALOGV("fingerprint_close");
    if (gpHalContext == nullptr || pDev == nullptr)
    {
        ALOGE("no device");
        return FINGERPRINT_ERROR;
    }
    gpHalContext->deinit();
    free(pDev);
    delete gpHalContext;
    return 0;
}

/**
 * Function: preEnroll
 * Description: preEnroll
 * Input: pDev
 * Output: none
 * Return: authenticated token challenge, 64 bit random number
 */
static uint64_t preEnroll(struct fingerprint_device __unused *pDev)
{
    ALOGV("fingerprint_pre_enroll");
    assert(gpHalContext != nullptr);
    return gpHalContext->mFingerprintCore->preEnroll();
}

/**
 * Function: enroll
 * Description: enroll fingerprints
 * Input: pDev, pHat, gid, timeoutSec
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t enroll(struct fingerprint_device __unused *pDev, const hw_auth_token_t *pHat,
        uint32_t gid, uint32_t timeoutSec)
{
    ALOGV("fingerprint_enroll");
    assert(gpHalContext != nullptr);
    return gpHalContext->mFingerprintCore->enroll(pHat, gid, timeoutSec);
}

/**
 * Function: postEnroll
 * Description: postEnroll
 * Input: pDev
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t postEnroll(struct fingerprint_device __unused *pDev)
{
    ALOGV("fingerprint_post_enroll");
    assert(gpHalContext != nullptr);
    return gpHalContext->mFingerprintCore->postEnroll();
}

/**
 * Function: getAuthId
 * Description: get authenticator id
 * Input: pDev
 * Output: none
 * Return: authenticator id, 64 bit number
 */
static uint64_t getAuthId(struct fingerprint_device __unused *pDev)
{
    ALOGV("fingerprint_get_auth_id");
    assert(gpHalContext != nullptr);
    return gpHalContext->mFingerprintCore->getAuthenticatorId();
}

/**
 * Function: cancel
 * Description: cancel operation
 * Input: pDev
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t cancel(struct fingerprint_device __unused *pDev)
{
    assert(gpHalContext != nullptr);
    ALOGV("fingerprint_cancel");
    return gpHalContext->mFingerprintCore->cancel();
}

/**
 * Function: remove
 * Description: remove the fingerprint that group id is gid and fingerprint id is fid
 * Input: pDev, gid, fid
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t remove(struct fingerprint_device __unused *pDev, uint32_t gid, uint32_t fid)
{
    assert(gpHalContext != nullptr);
    ALOGV("fingerprint_remove");
    return gpHalContext->mFingerprintCore->remove(gid, fid);
}

/**
 * Function: setActiveGroup
 * Description: set active group to be gid
 * Input: pDev, gid, pStorePath
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t setActiveGroup(struct fingerprint_device __unused *pDev, uint32_t gid, const char *pStorePath)
{
    assert(gpHalContext != nullptr);
    ALOGV("fingerprint_set_active_group");
    return gpHalContext->mFingerprintCore->setActiveGroup(gid, pStorePath);
}

/**
 * Function: authenticate
 * Description: authenticate
 * Input: pDev, operationId, gid
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t authenticate(struct fingerprint_device __unused *pDev, uint64_t operationId,
        uint32_t gid)
{
    assert(gpHalContext != nullptr);
    ALOGV("authenticate");
    return gpHalContext->mFingerprintCore->authenticate(operationId, gid);
}

#if defined(FINGERPRINT_MODULE_API_VERSION_2_1) || defined(FINGERPRINT_MODULE_API_VERSION_3_0 )
/**
 * Function: enumerate
 * Description: enumerate fingerprints
 * Input: pDev
 * Output: none
 * Return:
 * 0       if succeed
 * others  if fail
 */
static int32_t enumerate(struct fingerprint_device __unused *pDev)
{
    ALOGV("fingerprint_enumerate");
    assert(gpHalContext != nullptr);
    return gpHalContext->mFingerprintCore->enumerate();
}
#endif  // end ifdef FINGERPRINT_MODULE_API_VERSION_2_1

/**
 * Function: setNotifyCallBack
 * Description: setNotifyCallBack
 * Input: pDev, notify
 * Output: none
 * Return: 0
 */
static int32_t setNotifyCallBack(struct fingerprint_device *pDev, fingerprint_notify_t notify)
{
    ALOGV("set_notify_callback");
    pDev->notify = notify;
    assert(gpHalContext != nullptr);
    gpHalContext->mFingerprintCore->setNotify((fingerprint_notify_t) notify);
    return 0;
}

/***************oplus api*******************/
static int fingerprint_get_enroll_total_times(struct fingerprint_device __unused *dev)
{
    uint32_t total_enroll_times = 0;
    gpHalContext->mFingerprintCore->get_total_enroll_times(&total_enroll_times);
    ALOGV("fingerprint_get_enroll_total_times:%d",total_enroll_times);
    return total_enroll_times;
}

static int fingerprint_pause_enroll(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_pause_enroll");
    return gpHalContext->mFingerprintCore->pauseEnroll();
}

static int fingerprint_continue_enroll(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_pause_enroll");
    return gpHalContext->mFingerprintCore->continueEnroll();
}

static void fingerprint_set_touch_event_listener(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_set_touch_event_listener");
    return ;
}

static int fingerprint_set_screen_state(struct fingerprint_device __unused *dev,
        uint32_t screenstate)
{
    ALOGV("fingerprint_set_screen_state");
    return gpHalContext->mFingerprintCore->setScreenState(screenstate);
}

static int fingerprint_touchdown(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_touchdown");
    gpHalContext->mFingerprintCore->touchdown();
    return 0;
}

static int fingerprint_touchup(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_touchup");
    gpHalContext->mFingerprintCore->touchup();
    return 0;
}

static int fingerprint_selft_test(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_selft_test");
    return 0;
}

static void fingerprint_image_quality(struct fingerprint_device __unused *dev)
{
    ALOGV("fingerprint_image_quality");
    return ;
}

static int32_t sendFingerprintCmd(struct fingerprint_device __unused *pDev, int32_t cmd_id, int8_t* in_buf, uint32_t size)
{
    assert(gpHalContext != nullptr);
    ALOGE("hal sendFingerprintCmd");
    return gpHalContext->mFingerprintCore->sendFingerprintCmd(cmd_id, in_buf, size);
}

static int fingerprint_authenticateAsType(struct fingerprint_device *dev,
        uint64_t __unused operation_id,
        uint32_t __unused gid, uint32_t __unused authtype)
{
    ALOGE("fingerprint_authenticateAsType");
    return 0;
}

/**
 * Function: open
 * Description: open device
 * Input: pModule, pId, pDevice
 * Output: none
 * Return: 0
 */
static int32_t open(const hw_module_t *pModule, const char __unused *pId, hw_device_t **ppDevice)
{
    int32_t err = GF_SUCCESS;
    ALOGV("fingerprint_open");

    if (ppDevice == NULL)
    {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    fingerprint_device_t *pDev = (fingerprint_device_t *) malloc(sizeof(fingerprint_device_t));

    if (pDev == NULL)
    {
        ALOGE("out of memory");
        return -EINVAL;
    }

    memset(pDev, 0, sizeof(fingerprint_device_t));
    pDev->common.tag = HARDWARE_DEVICE_TAG;
#if defined(FINGERPRINT_MODULE_API_VERSION_3_0)
    pDev->common.version = FINGERPRINT_MODULE_API_VERSION_2_1;
#else  // not FINGERPRINT_MODULE_API_VERSION_3_0
    pDev->common.version = FINGERPRINT_MODULE_API_VERSION_2_0;
#endif  // defined(FINGERPRINT_MODULE_API_VERSION_3_0)
    pDev->common.module = (struct hw_module_t *) pModule;
    pDev->common.close = close;
    pDev->pre_enroll = preEnroll;
    pDev->enroll = enroll;
    pDev->post_enroll = postEnroll;
    pDev->get_authenticator_id = getAuthId;
    pDev->cancel = cancel;
    pDev->remove = remove;
    pDev->set_active_group = setActiveGroup;
    pDev->authenticate = authenticate;

    pDev->set_notify = setNotifyCallBack;
    pDev->notify = NULL;
#if defined(FINGERPRINT_MODULE_API_VERSION_2_1) || defined(FINGERPRINT_MODULE_API_VERSION_3_0)
    pDev->enumerate = enumerate;
#endif  // FINGERPRINT_MODULE_API_VERSION_2_1

    /*oplus api start*/
    pDev->pause_enroll = fingerprint_pause_enroll;
    pDev->continue_enroll = fingerprint_continue_enroll;
    pDev->wait_touch_down = fingerprint_set_touch_event_listener;
    pDev->get_enrollment_total_times = fingerprint_get_enroll_total_times;
    pDev->selftest = fingerprint_selft_test;
    pDev->getImageQuality = fingerprint_image_quality;
    pDev->touchDown = fingerprint_touchdown;
    pDev->touchUp = fingerprint_touchup;
    pDev->setScreenState = fingerprint_set_screen_state;
    pDev->authenticateAsType = fingerprint_authenticateAsType;
    /*oplus api end*/
    pDev->sendFingerprintCmd = sendFingerprintCmd;

    if (gpHalContext != nullptr)
    {
        delete gpHalContext;
    }
    gpHalContext = static_cast<HalContext*>(new HalContextExt());
    if (gpHalContext == nullptr)
    {
        ALOGE("out of memory, gpHalContext is NULL");
        free(pDev);
        return -EINVAL;
    }
    err = gpHalContext->init();
    if (err != GF_SUCCESS)
    {
        free(pDev);
        delete gpHalContext;
        gpHalContext = nullptr;
        return err;
    }
    err = registerService(gpHalContext);
    *ppDevice = (hw_device_t *) pDev;
    return 0;
}

static struct hw_module_methods_t gFingerprintModuleMethods =  // hw module methods
{
    .open = open,
};

/**
 * Fingerprint Module
 *
 * Every hardware module must have a data structure named HAL_MODULE_INFO_SYM
 * and the fields of this data structure must begin with hw_module_t
 * followed by module specific information.
 */

fingerprint_module_t HAL_MODULE_INFO_SYM =
{
    .common =
    {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = "fingerprint.goodix_G2",
        .name = "Goodix Fingerprint HAL",
        .author = "Goodix",
        .methods = &gFingerprintModuleMethods,
        .dso = NULL,
        .reserved = {   0  }
    },
};

