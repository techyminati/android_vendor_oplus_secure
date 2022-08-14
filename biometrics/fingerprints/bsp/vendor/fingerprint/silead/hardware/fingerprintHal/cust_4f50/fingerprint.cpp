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
 **  Ziqing.Guo       2018/09/26      keep the same threshold for unlock && pay, may modify in the future
 ************************************************************************************/

#define LOG_TAG "fingerprint"
#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <errno.h>
#include <malloc.h>
#include <string.h>

#include "fingerprint_4f50.h"
#include "silead_finger_4f50.h"
#include "../silead_finger.h"

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



static int fingerprint_authenticatetype(struct fingerprint_device *dev,
                                    uint64_t __unused operation_id,
                                    uint32_t __unused gid, uint32_t authtype)
{
    FUN_ENTRY_DBG("fingerprint_authenticatetype");
    uint32_t far_level = 0;
    far_level = 101;
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

static int fingerprint_keymode_enable(struct fingerprint_device *dev, uint32_t keymode_enable)
{
    FUN_ENTRY_DBG("set keymode enable :%d", keymode_enable);
    silfp_finger_set_key_mode(keymode_enable);
    return 0;
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
/*
static int fingerprint_cleanup(struct fingerprint_device __unused *dev)
{
    FUN_ENTRY_DBG("fingerprint_cleanup");
    return silfp_cleanup();
}
*/
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

static int fingerprint_selftest(struct fingerprint_device *dev) {
    int ret = 0;
    FUN_ENTRY_DBG("huycadd 22  fingerprint_selftest");
    ret = silfp_get_engineering_info(3);
    FUN_ENTRY_DBG("huycadd 22 fingerprint_selftest ret=%d", ret);
    return ret;
}

static void fingerprint_getImageQuality(struct fingerprint_device *dev)
{
    int ret = 0;
    FUN_ENTRY_DBG("huycadd 22 fingerprint_getImageQuality");
    ret = silfp_get_engineering_info(1);
    FUN_ENTRY_DBG("huycadd fingerprint_getImageQuality ret=%d", ret);
}

static int32_t fingerprint_touchDown(struct fingerprint_device __unused *pDev)
{
    FUN_ENTRY_DBG("touchdown 2");
    //silfp_touch_down();
    return 0;
}

static int32_t fingerprint_touchUp(struct fingerprint_device __unused *pDev)
{
    FUN_ENTRY_DBG("touchup 2");
    // silfp_touch_up();
    return 0;
}

static int32_t fingerprint_setScreenState(struct fingerprint_device __unused *pDev, uint32_t screen_state)
{
    FUN_ENTRY_DBG("setFingerState screen_state:%d", screen_state);
    silfp_set_screen_state((uint32_t)screen_state);
    return 0;
}

//static int32_t fingerprint_sendFingerprintCmd(struct fingerprint_device __unused *pDev, int32_t cmd_id, int8_t* in_buf, uint32_t size)
static int32_t fingerprint_sendFingerprintCmd(struct fingerprint_device *pDev, int32_t cmd_id, int8_t* in_buf)
{
    FUN_ENTRY_DBG("hal sendFingerprintCmd cmdId=%d", cmd_id);
    switch (cmd_id) {
    case 1001: //CMD_FINGERPRINT_CAMERA
        ALOGD("keymode_enable(enable = %d)", in_buf[0]);
        silfp_finger_set_key_mode(in_buf[0]);

        break;
    case 31: //OTP
        ALOGD("Enter:cmd_id = %d", cmd_id);
        silfp_finger_opt_notify(cmd_id);
            break;
     default:
        break;
    }

    // silfp_send_fingerprint_cmd(cmd_id, in_buf, size);
    return 0; //gpHalContext->mAuthenticator->sendFingerprintCmd(cmd_id, in_buf, size);
}

static int32_t fingerprint_set_image_dump_flag(struct fingerprint_device __unused *pDev, uint32_t cmd, uint32_t data)
{
    FUN_ENTRY_DBG("hal fingerprint_set_image_dump_flag");
    // sl_fp_set_image_dump_flag(cmd, data);
    return 0;
}
static int32_t fingerprint_dump_calibration_image(struct fingerprint_device __unused *pDev)
{
    FUN_ENTRY_DBG("hal dump_calibration_image");
    //sl_dump_calibration_image();
    return 0; 
}
static int fingerprint_open(const hw_module_t* module, const char __unused *id, hw_device_t** device)
{
    FUN_ENTRY_DBG("fingerprint hal 4f50");

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
    dev->authenticateAsType = fingerprint_authenticatetype;
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->enumerate = fingerprint_enumerate;
    dev->set_notify = set_notify_callback;
    dev->notify = NULL;
    dev->selftest = fingerprint_selftest;

    dev->getImageQuality = fingerprint_getImageQuality;

//    dev->get_enrollment_total_times = fingerprint_get_enroll_total_times;
    dev->pause_enroll = fingerprint_pause_enroll;
    dev->continue_enroll = fingerprint_continue_enroll;
//    dev->pause_identify = fingerprint_pause_identify;
//    dev->continue_identify = fingerprint_continue_identify;
//    dev->getAlikeyStatus = fingerprint_get_alikey_status;
    //dev->cleanUp = fingerprint_cleanup;
    dev->wait_touch_down = fingerprint_set_touch_event_listener;
    //dev->setScreenState = fingerprint_set_screen_state;
    //dev->dynamicallyConfigLog = fingerprint_dynamically_config_log;
//    dev->getEngineeringInfo = fingerprint_get_engineering_info;
    dev->touchDown = fingerprint_touchDown;
    dev->touchUp = fingerprint_touchUp;
    dev->setScreenState = fingerprint_setScreenState;
    dev->sendFingerprintCmd = fingerprint_sendFingerprintCmd;
    dev->set_image_dump_flag = fingerprint_set_image_dump_flag;
    dev->dump_calibration_image = fingerprint_dump_calibration_image;
//  dev->setFingerKeymode = fingerprint_set_key_mode;

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
