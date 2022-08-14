/******************************************************************************
 * @file   fingerprint.cpp
 * @brief  Contains fingerprint HAL main module.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *****************************************************************************/

#define LOG_TAG "fingerprint"
#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <hardware/hardware.h>

#if 1//def 1//SL_FP_FEATURE_OPLUS_CUSTOMIZE
    #include "fingerprint_oplus.h"
#else
    #include <hardware/fingerprint.h>
#endif

#include "silead_finger.h"
#include "silead_cust.h"
#include "silead_cmd_cust.h"

// must be the same with:
//  \system\core\fingerprintd\FingerprintDaemonProxy.cpp kVersion  or
//  \hardware\interfaces\biometrics\fingerprint\2.1\default\BiometricsFingerprint.cpp kVersion
#define FINGERPRINT_MODULE_ID_DEFAULT FINGERPRINT_MODULE_API_VERSION_2_1

#ifdef SILEAD_FP_HAL_MODULE_ID
#define SILEAD_FINGERPRINT_HARDWARE_MODULE_ID SILEAD_FP_HAL_MODULE_ID
#else
#define SILEAD_FINGERPRINT_HARDWARE_MODULE_ID "fingerprint.silead"
#endif

#define FUN_ENTRY_DBG(...) ALOGE(__VA_ARGS__)
//#define FUN_ENTRY_DBG(...) ((void)0)

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

static uint64_t fingerprint_pre_enroll(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_pre_enroll");
    return silfp_finger_pre_enroll();
}

static int fingerprint_enroll(struct fingerprint_device __unused *dev,
                              const hw_auth_token_t __unused *hat,
                              uint32_t __unused gid,
                              uint32_t __unused timeout_sec)
{
    FUN_ENTRY_DBG("fingerprint_enroll");
    return silfp_finger_enroll(hat, gid, timeout_sec);
}

static int fingetprint_post_enroll(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingetprint_post_enroll");
    return silfp_finger_post_enroll();
}

static int fingerprint_authenticate(struct fingerprint_device __unused *dev,
                                    uint64_t __unused operation_id,
                                    uint32_t __unused gid)
{
    FUN_ENTRY_DBG("fingerprint_authenticate");
    return silfp_finger_authenticate(operation_id, gid);
}

static int fingerprint_authenticateAsType(struct fingerprint_device __unused *dev,
                                    uint64_t __unused operation_id,
                                    uint32_t __unused gid, uint32_t authtype)
{
    FUN_ENTRY_DBG("fingerprint_authenticateAsType");
    uint32_t far_level = 0;
    if (authtype == AUTH_TYPE_PAY) {
        far_level = ORIGIN_LEVEL_FAR_THRESHOLD;
    } else {
        far_level = ORIGIN_LEVEL_FAR_THRESHOLD;
    }
    return silfp_finger_authenticate_ext(operation_id, gid, far_level);
}

static uint64_t fingerprint_get_auth_id(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_get_auth_id");
    return silfp_finger_get_auth_id();
}

static int fingerprint_cancel(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_cancel");
    return silfp_finger_cancel();
}

static int fingerprint_remove(struct fingerprint_device __unused *dev,
                              uint32_t __unused gid,
                              uint32_t __unused fid)
{
    FUN_ENTRY_DBG("fingerprint_remove");
    return silfp_finger_remove(gid, fid);
}

#ifdef ANDROID6
static int fingerprint_enumerate(struct fingerprint_device __unused *dev,
                                 fingerprint_finger_id_t __unused *results,
                                 uint32_t __unused *max_size)
{
    FUN_ENTRY_DBG("fingerprint_enumerate6");
    return silfp_finger_enumerate6(results, max_size);
}
#else
static int fingerprint_enumerate(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_enumerate");
    return silfp_finger_enumerate();
}
#endif

static int fingerprint_set_active_group(struct fingerprint_device __unused *dev,
                                        uint32_t __unused gid,
                                        const char __unused *store_path)
{
    FUN_ENTRY_DBG("fingerprint_set_active_group");
    char path_name[PATH_MAX];
    int pathlen = strlen(store_path);
    memcpy(path_name, store_path, pathlen);
    path_name[pathlen] = '\0';
    return silfp_finger_set_active_group(gid, path_name);
}

static int set_notify_callback(struct fingerprint_device *dev,
                               fingerprint_notify_t notify)
{
    FUN_ENTRY_DBG("set_notify_callback");
    dev->notify = notify;
    return silfp_finger_set_notify_callback(notify);
}

static int fingerprint_close(hw_device_t *dev)
{
    FUN_ENTRY_DBG("fingerprint_close");
    int err = -1;
    if (dev) {
        free(dev);
        err = 0;
    }

    silfp_finger_deinit();
    return err;
}

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
static int fingerprint_get_enroll_total_times(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_get_enroll_total_times");
    return silfp_get_enroll_total_times();
}

static int fingerprint_pause_enroll(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_pause_enroll");
    return silfp_pause_enroll();
}

static int fingerprint_continue_enroll(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_continue_enroll");
    return silfp_continue_enroll();
}

static int fingerprint_pause_identify(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_pause_identify");
    return silfp_pause_identify();
}

static int fingerprint_continue_identify(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_continue_identify");
    return silfp_continue_identify();
}

static int fingerprint_get_alikey_status(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_get_alikey_status");
    return silfp_get_alikey_status();
}

static void fingerprint_set_touch_event_listener(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_set_touch_event_listener");
    silfp_set_touch_event_listener();
}

static int fingerprint_set_screen_state(struct fingerprint_device __unused *dev,
                                        uint32_t sreenstate)
{
    FUN_ENTRY_DBG("fingerprint_set_screen_state");
    return silfp_set_screen_state(sreenstate);
}

static int fingerprint_dynamically_config_log(struct fingerprint_device __unused *dev,
                                            uint32_t on)
{
    FUN_ENTRY_DBG("fingerprint_dynamically_config_log");
    return silfp_dynamically_config_log(on);
}

static int fingerprint_get_engineering_info(struct fingerprint_device __unused *dev,
                                            uint32_t type)
{
    FUN_ENTRY_DBG("fingerprint_get_engineering_info");
    return silfp_get_engineering_info(type);
}

#endif

static int fingerprint_selft_test(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_selft_test");
    return silfp_get_engineering_info(FINGERPRINT_SELF_TEST);
}

static void  fingerprint_image_quality(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("FINGERPRINT_GET_IMAGE_QUALITYS");
    silfp_get_engineering_info(FINGERPRINT_GET_IMAGE_QUALITYS);
}

//unused
static int32_t fingerprint_touchDown(struct fingerprint_device __unused *pDev)
{
    FUN_ENTRY_DBG("touchdown 2");
    silfp_touch_down();
    return 0;
}

//unused
static int32_t fingerprint_touchUp(struct fingerprint_device __unused *pDev)
{
    FUN_ENTRY_DBG("touchup 2");
    silfp_touch_up();
    return 0;
}

static int32_t fingerprint_setScreenState(struct fingerprint_device __unused *pDev, uint32_t screen_state)
{
    FUN_ENTRY_DBG("setFingerState");
    silfp_set_screen_state((uint32_t)screen_state);
    return 0;
}

static int32_t fingerprint_sendFingerprintCmd(struct fingerprint_device __unused *pDev, int32_t cmd_id, int8_t* in_buf, uint32_t size)
{
    FUN_ENTRY_DBG("hal sendFingerprintCmd");
    silfp_send_fingerprint_cmd(cmd_id, in_buf, size);
    return 0; //gpHalContext->mAuthenticator->sendFingerprintCmd(cmd_id, in_buf, size);
}

static int fingerprint_open(const hw_module_t* module, const char __unused *id, hw_device_t** device)
{
    FUN_ENTRY_DBG("fingerprint hal default");

    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    int ret = silfp_finger_init();
    if (ret < 0) {
        silfp_finger_deinit();
        ALOGE("Could not init silead device (%d)", ret);
        return -ENODEV;
    }

    fingerprint_device_t *dev = (fingerprint_device_t *)malloc(sizeof(fingerprint_device_t));
    memset(dev, 0, sizeof(fingerprint_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = FINGERPRINT_MODULE_ID_DEFAULT;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = fingerprint_close;

    dev->pre_enroll = fingerprint_pre_enroll;
    dev->enroll = fingerprint_enroll;
    dev->post_enroll = fingetprint_post_enroll;
    dev->authenticate = fingerprint_authenticate;
    dev->authenticateAsType = fingerprint_authenticateAsType;
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->enumerate = fingerprint_enumerate;
    dev->set_notify = set_notify_callback;
    dev->notify = NULL;

#ifdef SL_FP_FEATURE_OPLUS_CUSTOMIZE
    dev->pause_enroll = fingerprint_pause_enroll;
    dev->continue_enroll = fingerprint_continue_enroll;
    dev->wait_touch_down = fingerprint_set_touch_event_listener;

    dev->selftest = fingerprint_selft_test;
    dev->getImageQuality = fingerprint_image_quality;
    dev->setScreenState = fingerprint_setScreenState;
    dev->touchDown = fingerprint_touchDown;
    dev->touchUp = fingerprint_touchUp;
    dev->setScreenState = fingerprint_setScreenState;
    dev->sendFingerprintCmd = fingerprint_sendFingerprintCmd;
#endif

    *device = (hw_device_t*) dev;
    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_ID_DEFAULT,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = SILEAD_FINGERPRINT_HARDWARE_MODULE_ID,
        .name = "silead Fingerprint HAL",
        .author = "silead@cswang",
        .methods = &fingerprint_module_methods,
        .dso = NULL,
        .reserved = {0},
    },
};
