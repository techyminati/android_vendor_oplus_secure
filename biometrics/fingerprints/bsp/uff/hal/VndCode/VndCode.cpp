#define LOG_TAG "[FP_HAL][VND_CODE]"
#include "VndCode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HalContext.h"
#include "Device.h"
#include "fingerprint.h"
#include "FpType.h"
#include "Dump.h"
#include <string.h>

using android::HalContext;
void send_message_to_disaplay(vnd_code_t vnd_code)
{
    LOG_E(LOG_TAG, "disaplay:%s, %s", vnd_code.vnd_description, vnd_code.apk_msg);
    (void)HalContext::getInstance()->mFingerprintManager->notifyFingerprintCmd(0,
        FINGERPRINT_FACTORY_MESSAGE_DISPLAY, (const int8_t *) vnd_code.apk_msg, strlen(vnd_code.apk_msg));
}

fp_return_type_t do_reset(vnd_code_t *code)
{
    (void)code;
    return HalContext::getInstance()->mDevice->reset();
}

fp_return_type_t do_set_power_status(vnd_code_t *code)
{
    return code->code == E_CODE_POWER_ON ? \
        HalContext::getInstance()->mDevice->controlPower(DEVICE_ENABLE_POWER) : \
        HalContext::getInstance()->mDevice->controlPower(DEVICE_DISABLE_POWER);
}

fp_return_type_t do_set_hbm(vnd_code_t *code)
{
    fp_return_type_t ret = FP_SUCCESS;
    if (code->code == E_CODE_HBM_ON) {
        HalContext::getInstance()->mFacotryTest->setHbmMode(1);
    } else {
        HalContext::getInstance()->mFacotryTest->setHbmMode(0);
    }
    return ret;
}

fp_return_type_t do_setbrightness(vnd_code_t *code)
{
    fp_return_type_t ret = FP_SUCCESS;
    switch (code->code) {
    case E_CODE_SET_LOW_BRIGHTNESS:
        HalContext::getInstance()->mFacotryTest->setBrightness(0);
        break;
    case E_CODE_SET_MID_BRIGHTNESS:
        //TODO: get brightness from ta config dynamically
        HalContext::getInstance()->mFacotryTest->setBrightness(
            HalContext::getInstance()->mConfig->mTaConfig->mCaliMidBrightness);
        break;
    case E_CODE_SET_HIGH_BRIGHTNESS:
        //TODO: get brightness from ta config dynamically
        HalContext::getInstance()->mFacotryTest->setBrightness(0);
        break;
    default:
        break;
    }
    return ret;
}

fp_return_type_t do_save_calibrate_data(vnd_code_t *code)
{
    (void)code;
    return FP_SUCCESS;
}

fp_return_type_t do_save_capture_img_data(vnd_code_t *code)
{
    (void)code;
    LOG_I(LOG_TAG, "do_save_capture_img_data IN");
    android::Dump::dumpProcess(FP_MODE_APP_DATA, 0, 0, 0);
    LOG_I(LOG_TAG, "do_save_capture_img_data OUT");
    return FP_DUMP_OVER;
}

fp_return_type_t do_save_calibrate_dump(vnd_code_t *code)
{
    (void)code;
    android::Dump::dumpProcess(FP_MODE_FACTORY_TEST, 0, 0, 0);
    return FP_SUCCESS;
}

fp_return_type_t do_next_calibration(vnd_code_t *code)
{
    (void)code;
    return FP_SUCCESS;
}

f_func_t func_maps[] = {
    {E_CODE_RESET, do_reset},
    {E_CODE_POWER_ON, do_set_power_status},
    {E_CODE_POWER_OFF, do_set_power_status},
    {E_CODE_HBM_ON, do_set_hbm},
    {E_CODE_HBM_OFF, do_set_hbm},
    {E_CODE_SET_LOW_BRIGHTNESS, do_setbrightness},
    {E_CODE_SET_MID_BRIGHTNESS, do_setbrightness},
    {E_CODE_SET_HIGH_BRIGHTNESS, do_setbrightness},
    {E_CODE_SAVE_CALI_DATA, do_save_calibrate_data},
    {E_CODE_GET_CAPTURE_IMG, do_save_capture_img_data},
    {E_CODE_FACTORY_TEST_DUMP, do_save_calibrate_dump},
    {E_CODE_CALI_CONTINUE, do_next_calibration}
};

