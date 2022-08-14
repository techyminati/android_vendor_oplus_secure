/************************************************************************************
 ** File: - ingerprint\silead\Fingerprint.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for silead_optical(android O)
 **
 ** Version: 1.0
 ** Date created: 15:09:11,18/11/2017
 ** Author: Ran.chen@Bsp.Fingerprint.basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         2018/06/24      add oplus feature
 **  Ziqing.Guo       2018/09/17      add the api authenticate as Auth Type(keyguard or Pay)
 ************************************************************************************/

#define LOG_TAG "fingerprint"
#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <hardware/hardware.h>
#include <hardware/fingerprint.h>

#include "silead_finger.h"

// must be the same with:
//  \system\core\fingerprintd\FingerprintDaemonProxy.cpp kVersion  or
//  \hardware\interfaces\biometrics\fingerprint\2.1\default\BiometricsFingerprint.cpp kVersion
#define FINGERPRINT_MODULE_ID_DEFAULT FINGERPRINT_MODULE_API_VERSION_2_1


#define SILEAD_FINGERPRINT_HARDWARE_MODULE_ID "fingerprint.silead"


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
static int fingerprint_enumerate(struct fingerprint_device *dev,
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

static int fingerprint_open(const hw_module_t* module, const char __unused *id, hw_device_t** device)
{
    FUN_ENTRY_DBG("fingerprint hal default");

    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    silfp_finger_set_key_mode(0);

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
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->enumerate = fingerprint_enumerate;
    dev->set_notify = set_notify_callback;
    dev->notify = NULL;

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
