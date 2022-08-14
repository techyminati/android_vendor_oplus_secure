#include "anc_factory_common.h"
#include "anc_extension_factory_mode.h"
#include "anc_extension_command.h"
#include "anc_memory_wrapper.h"
#include "anc_lib.h"
#include "anc_log.h"


static ANC_RETURN_TYPE Get6311FactoryAllTestItem(CmdExecute *executes, uint32_t executes_cnt) {
    CmdExecute cmds[EXTENSION_COMMAND_FT_COUNT] = {
        {EXTENSION_COMMAND_FT_INIT, ExtCommandFTInit, "ft_init"},
        {EXTENSION_COMMAND_FT_MODULE_T, ExtCommandFTModuleTest, "module_test"},
        {EXTENSION_COMMAND_FT_WHITE_AUTO_EXP_C, ExtCommandFTWhiteAutoExpCalibration, "base_sum"},
        {EXTENSION_COMMAND_FT_WHITE_PREVENT_A_T, ExtCommandFTWhitePreventATest, "white_prevent"},
        {EXTENSION_COMMAND_FT_WHITE_AUTO_EXP_A_T, ExtCommandFTWhiteAutoExpATest, "white_auto_exp"},
        {EXTENSION_COMMAND_FT_WHITE_LENS_AND_DEFECT_TC, ExtCommandFTWhiteLensAndDefectTC, "white_len_defect"},
        {EXTENSION_COMMAND_FT_WHITE_INSTALL_OFFSET_T, ExtCommandFTWhiteInstallOffsetTest, "white_icon_install_offset"},
        {EXTENSION_COMMAND_FT_BLACK_PREVENT_B_AND_AUTO_EXP_T, ExtCommandFTBlackAllTC, "black_prevent"},
        {EXTENSION_COMMAND_FT_WHITE_STRIPE_T, ExtCommandFTWhiteStripeAllTC, "white_stripe"},
        {EXTENSION_COMMAND_FT_SAVE_CALIBRATION_DATA, ExtCommandFTSaveCalibrationData, "save_calibraion_data"},
        {EXTENSION_COMMAND_FT_CAPTURE_FINGER_IMAGE, ExtCommandFTCaptureFingerImage, "test_finger_capture_image"},
        {EXTENSION_COMMAND_FT_DEINIT, ExtCommandFTDeinit, "ft_deinit"},
    };
    AncMemcpy(executes, cmds, sizeof(CmdExecute) * executes_cnt);
    return ANC_OK;
}

static ANC_RETURN_TYPE Get5446FactoryAllTestItem(CmdExecute *executes, uint32_t executes_cnt) {
    CmdExecute cmds[EXTENSION_COMMAND_FT_COUNT] = {
        {EXTENSION_COMMAND_FT_INIT, ExtCommandFTInit, "ft_init"},
        {EXTENSION_COMMAND_FT_MODULE_T, ExtCommandFTModuleTestV2, "module_test"},
        {EXTENSION_COMMAND_FT_WHITE_AUTO_EXP_C, ExtCommandFTWhiteExpCalibration, "expo calibration"},
        {EXTENSION_COMMAND_FT_WHITE_PREVENT_A_T, ExtCommandFTWhitePreventATest, "white_prevent"},
        {EXTENSION_COMMAND_FT_WHITE_AUTO_EXP_A_T, ExtCommandFTWhiteAutoExpATest, "white_auto_exp"},
        {EXTENSION_COMMAND_FT_WHITE_LENS_AND_DEFECT_TC, ExtCommandFTWhiteLensAndDefectTC, "white_len_defect"},
        {EXTENSION_COMMAND_FT_WHITE_INSTALL_OFFSET_T, ExtCommandFTWhiteInstallOffsetTest, "white_icon_install_offset"},
        {EXTENSION_COMMAND_FT_BLACK_PREVENT_B_AND_AUTO_EXP_T, ExtCommandFTBlackAllTC, "black_prevent"},
        {EXTENSION_COMMAND_FT_WHITE_STRIPE_T, ExtCommandFTWhiteStripeAllTC, "white_stripe"},
        {EXTENSION_COMMAND_FT_SAVE_CALIBRATION_DATA, ExtCommandFTSaveCalibrationData, "save_calibraion_data"},
        {EXTENSION_COMMAND_FT_CAPTURE_FINGER_IMAGE, ExtCommandFTCaptureFingerImageV2, "test_finger_capture_image"},
        {EXTENSION_COMMAND_FT_DEINIT, ExtCommandFTDeinit, "ft_deinit"},
    };
    AncMemcpy(executes, cmds, sizeof(CmdExecute) * executes_cnt);
    return ANC_OK;
}

ANC_RETURN_TYPE GetFactoryImpl(uint32_t sensor_chipid, CmdExecute *executes, uint32_t executes_cnt) {
    if ((executes_cnt != EXTENSION_COMMAND_FT_COUNT) && (executes == NULL)) {
        return ANC_FAIL;
    }

    ANC_RETURN_TYPE ret = ANC_OK;
    switch (sensor_chipid) {
    case 0x6311: //0302
        ret = Get6311FactoryAllTestItem(executes, executes_cnt);
        break;
    case 0x5446: //0301
        ret = Get5446FactoryAllTestItem(executes, executes_cnt);
        break;
    default:
        ANC_LOGE("sensor type wrong");
        ret = ANC_FAIL;
        break;
    }
    return ret;
}