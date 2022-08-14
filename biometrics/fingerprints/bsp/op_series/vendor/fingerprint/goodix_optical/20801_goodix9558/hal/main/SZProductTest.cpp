/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][SZProductTest]"

#include <cutils/properties.h>
#include <pthread.h>
#include "CoreCreator.h"

#include "SZProductTest.h"
#include "HalContext.h"
#include "SZProductTestDefine.h"
#include "gf_sensor_types.h"
#include "gf_sz_config.h"
#include "SZSensor.h"
#include "TestUtils.hpp"
#include "GoodixFingerprint.h"
#include "Device.h"

#ifdef SUPPORT_DUMP
#include "SZHalDump.h"
#endif  // SUPPORT_DUMP

namespace goodix {
    sz_product_test_cmd_table_t szProductTestCmdTable[] = {
        TABLE(CMD_TEST_ENUMERATE),
        TABLE(CMD_TEST_GET_VERSION),
        TABLE(CMD_TEST_RESET_PIN),
        TABLE(CMD_TEST_INTERRUPT_PIN),
        TABLE(CMD_TEST_CANCEL),
        TABLE(CMD_TEST_GET_CONFIG),
        TABLE(CMD_TEST_SET_CONFIG),
        TABLE(CMD_TEST_MAX),
        TABLE(CMD_TEST_SZ_FINGER_DOWN),
        TABLE(CMD_TEST_SZ_FINGER_UP),
        TABLE(CMD_TEST_SZ_ENROLL),
        TABLE(CMD_TEST_SZ_FIND_SENSOR),
        TABLE(CMD_TEST_SZ_FUSION_PREVIEW),
        TABLE(CMD_TEST_SZ_UNTRUSTED_ENROLL),
        TABLE(CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE),
        TABLE(CMD_TEST_SZ_DELETE_UNTRUSTED_ENROLLED_FINGER),
        TABLE(CMD_TEST_SZ_RAWDATA_PREVIEW),
        TABLE(CMD_TEST_SZ_LDC_CALIBRATE),
        TABLE(CMD_TEST_SZ_ENROLL_TEMPLATE_COUNT),
        TABLE(CMD_TEST_SZ_UPDATE_CAPTURE_PARM),
        TABLE(CMD_TEST_SZ_CANCEL),
        TABLE(CMD_TEST_SZ_GET_CONFIG),
        TABLE(CMD_TEST_SZ_GET_VERSION),
        TABLE(CMD_TEST_SZ_K_B_CALIBRATION),
        TABLE(CMD_TEST_SZ_SET_GROUP_ID),
        TABLE(CMD_TEST_SZ_UPDATE_CFG),
        TABLE(CMD_TEST_SZ_UPDATE_FW),
        TABLE(CMD_TEST_SZ_UNTRUSTED_ENUMERATE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_DARK_BASE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_H_DARK),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_L_DARK),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_H_FLESH),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_L_FLESH),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHART),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHECKBOX),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_LOCATION_IMAGE),
        TABLE(CMD_TEST_SZ_FT_FACTORY_PERFORMANCE),
        TABLE(CMD_TEST_SZ_FT_EXPO_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_STOP_EXPO_AUTO_CALIBRATION),
        TABLE(CMD_TEST_SZ_FT_RESET),
        TABLE(CMD_TEST_SZ_FT_SPI_RST_INT),
        TABLE(CMD_TEST_SZ_FT_SPI),
        TABLE(CMD_TEST_SZ_FT_INIT),
        TABLE(CMD_TEST_SZ_FT_EXIT),
        TABLE(CMD_TEST_SZ_FT_CALIBRATE),
        TABLE(CMD_TEST_SZ_FT_MT_CHECK),
        TABLE(CMD_TEST_SZ_FT_KPI),
        TABLE(CMD_TEST_SZ_FT_FACTORY_CAPTURE_IMAGE_MANUAL),
        TABLE(CMD_TEST_SZ_DUMP_TEMPLATE),
        TABLE(CMD_TEST_SZ_SET_HBM_MODE),
        TABLE(CMD_TEST_SZ_CLOSE_HBM_MODE),
        TABLE(CMD_TEST_SZ_SET_HIGH_BRIGHTNESS),
        TABLE(CMD_TEST_SZ_SET_LOW_BRIGHTNESS),
        TABLE(CMD_TEST_SZ_SET_DUMP_ENABLE_FLAG),
        TABLE(CMD_TEST_SZ_FACTORY_TEST_GET_MT_INFO),
        TABLE(CMD_TEST_SZ_SET_MAX_BRIGHTNESS),
        TABLE(CMD_TEST_SZ_LOCAL_AREA_SAMPLE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHECKBOX_CAPTURE),
        TABLE(CMD_TEST_SZ_FT_CAPTURE_CHART_CAPTURE),
        TABLE(CMD_TEST_SZ_ENABLE_POWER),
        TABLE(CMD_TEST_SZ_DISABLE_POWER),
        TABLE(CMD_TEST_SZ_UI_READY),
        TABLE(CMD_TEST_SZ_UI_DISAPPER),
        TABLE(CMD_TEST_SZ_ENABLE_PAY_ENVIRONMENT_LEVEL),
        TABLE(CMD_TEST_SZ_DISABLE_PAY_ENVIRONMENT_LEVEL),
        TABLE(CMD_TEST_PRODUCT_CMD_MAX)
    };

    SZProductTest *SZProductTest::szTest = nullptr;
#ifdef SUPPORT_FRR_FAR
    SZFrrFarTest *SZProductTest::szFrrFarTest = nullptr;
#endif  // END SUPPORT_FRR_FAR

    SZProductTest::SZProductTest(HalContext *context) :
        ProductTest(context),
        group_id(SZ_UNTRUST_ENROLL_TEST_GID) {
        szFingerprintCore = createFingerprintCore(context);
        szFingerprintCore->setNotify(szNotifyCallback);
        szTest = this;
#ifdef SUPPORT_FRR_FAR
        szFrrFarTest = new SZFrrFarTest(context);
#endif  // END SUPPORT_FRR_FAR
        mCurrentCmd = 0;
        kb_step = 0;
        mWorkState = STATE_INIT;
        handle_thread = 0;
    }

    SZProductTest::~SZProductTest() {
        delete szFingerprintCore;
        szFingerprintCore = nullptr;
#ifdef SUPPORT_FRR_FAR
        delete szFrrFarTest;
        szFrrFarTest = nullptr;
#endif  // END SUPPORT_FRR_FAR
    }

    static int32_t rawdataToBmp(uint16_t *rawdata, uint32_t rawdata_len,
                                uint8_t *bmpdata,
                                uint32_t bmpdata_len, uint32_t width, uint32_t height) {
        gf_error_t err = GF_SUCCESS;
        uint32_t i = 0;
        uint32_t max = 0;
        uint32_t min = 10000;
        FUNC_ENTER();
        UNUSED_VAR(height);
        UNUSED_VAR(width);

        if (rawdata_len != bmpdata_len) {
            LOG_E(LOG_TAG, "invalid parm [%s]--------exit--------", __FUNCTION__);
            err = GF_ERROR_BAD_PARAMS;
            return err;
        }

        memset(bmpdata, 0, sizeof(uint8_t) * bmpdata_len);

        for (i = 0; i < rawdata_len; i++) {
            if (rawdata[i] > max) {
                max = rawdata[i];
            }

            if (rawdata[i] < min) {
                min = rawdata[i];
            }
        }

        if (max == min) {
            return err;
        }

        for (i = 0; i < rawdata_len; i++) {
            bmpdata[i] = (uint8_t)((rawdata[i] - min) * 255 / (max - min));
        }

        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::notifyFindSensorPreviewBmp(gf_error_t err,
                                                   gf_sz_find_sensor_image_t *sensor_image) {
        gf_sz_test_bmp_data_t bmp_data = { {0} };
        int8_t *test_result = NULL;
        uint32_t len = 0;
        fp_eng_tests_result eng_result;
        VOID_FUNC_ENTER();

        do {
            if (NULL == sensor_image) {
                return;
            }

            rawdataToBmp(sensor_image->raw_data.data, sensor_image->raw_data.data_len,
                         bmp_data.bmp_data, sensor_image->raw_data.data_len,
                         sensor_image->col, sensor_image->row);
            bmp_data.width = sensor_image->col;
            bmp_data.height = sensor_image->row;
            LOG_D(LOG_TAG, "[%s]  sensor_image->row=%d", __func__, sensor_image->row);
            LOG_D(LOG_TAG, "[%s]  sensor_image->col=%d", __func__, sensor_image->col);
            uint32_t array_len = bmp_data.width * bmp_data.height;
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // image quality
            len += HAL_TEST_SIZEOF_INT32;
            // valid area
            len += HAL_TEST_SIZEOF_INT32;
            // bmp_data
            len += HAL_TEST_SIZEOF_ARRAY(array_len);
            // bmp_data width
            len += HAL_TEST_SIZEOF_INT32;
            // bmp_data height
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE,
                                                     err);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_IMAGE_QUALITY,
                                                     bmp_data.image_quality);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_VALID_AREA,
                                                     bmp_data.valid_area);
                current = TestUtils::testEncodeArray(current, TEST_TOKEN_BMP_DATA,
                                                     (int8_t *) bmp_data.bmp_data, sizeof(uint8_t) * array_len);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_BMP_DATA_WIDTH,
                                                     bmp_data.width);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_BMP_DATA_HEIGHT,
                                                     bmp_data.height);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_SZ_FIND_SENSOR, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t SZProductTest::findSensor(void) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_find_sensor_image_t cmd = {{ 0 }};
        uint32_t size = sizeof(gf_sz_find_sensor_image_t);
        fp_eng_tests_result eng_result;
        FUNC_ENTER();

        do {
            memset(&cmd, 0, size);
            cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_FIND_SENSOR;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, size);

            if (GF_SUCCESS == err) {
                notifyFindSensorPreviewBmp(err, &cmd);
            } else {
                LOG_E(LOG_TAG, "[%s] gf_hal_find_sensor faild", __func__);
            }

            if(err == GF_SUCCESS){
                eng_result.result = 1;
                strcpy(eng_result.info, "PASS");
            }else{
                eng_result.result = -1;
                sprintf(eng_result.info, "%s", gf_strerror(err));
            }
            eng_result.cmdId = CMD_CAPTURE_AGING_TEST;

            notifyEngResult(&eng_result);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::localAreaSample(void) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_test_capture_image_t *cmd = NULL;
        uint32_t size = sizeof(gf_sz_test_capture_image_t);
        FUNC_ENTER();

        do {
            cmd = (gf_sz_test_capture_image_t *) malloc(size);

            if (NULL == cmd) {
                LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(cmd, 0, size);
            cmd->cmd_header.cmd_id = GF_SZ_CMD_FACTORY_TEST_LOCAL_AREA_SAMPLE;
            cmd->cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(cmd, size);

            if (GF_ERROR_SPI_RAW_DATA_CRC_FAILED == err) {
                err = GF_SZ_FT_ERROR_LOCAL_AREA_SAMPLE_FAIL;
            }
        } while (0);

        if (cmd != NULL) {
            free(cmd);
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::setCaptureParam(const int8_t *cmd_buf,
                                              uint32_t cmd_len) {
        gf_error_t err = GF_SUCCESS;
        const int8_t *in_buf = cmd_buf;
        gf_sz_capture_param_t capture_param = {{ 0 }};
        uint32_t token = 0;
        uint32_t Fusionratio = 0;
        uint32_t RetryCount = 0;
        FUNC_ENTER();

        do {
            if (NULL == in_buf) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            memset(&capture_param, 0, sizeof(gf_sz_capture_param_t));

            do {
                in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                switch (token) {
                    case TEST_TOKEN_SCALE_RATIO_VALUE: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.scale_ratio, in_buf);
                        break;
                    }

                    case TEST_TOKEN_LONG_FRAME_AVG_NUM: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.long_frame_avg_num, in_buf);
                        break;
                    }

                    case TEST_TOKEN_LONG_PAG_GAIN: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.long_pga_gain, in_buf);
                        break;
                    }

                    case TEST_TOKEN_SHORT_EXPOSURE_TIME: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.short_exposure_time,
                                                             in_buf);
                        break;
                    }

                    case TEST_TOKEN_SHORT_FRAME_AVG_NUM: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.short_frame_avg_num,
                                                             in_buf);
                        break;
                    }

                    case TEST_TOKEN_SHORT_PAG_GAIN: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.short_pga_gain, in_buf);
                        break;
                    }

                    case TEST_TOKEN_UNLOWERTHRESH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.unlowerthresh, in_buf);
                        break;
                    }

                    case TEST_TOKEN_UNHIGHTHRESH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.unhighthresh, in_buf);
                        break;
                    }

                    case TEST_TOKEN_FUSION_RATIO: {
                        in_buf = TestUtils::testDecodeUint32(&Fusionratio, in_buf);
                        break;
                    }

                    case TEST_TOKEN_PREPROCESS: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.preprocess, in_buf);
                        break;
                    }

                    case TEST_TOKEN_RECT_X: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rect_x, in_buf);
                        break;
                    }

                    case TEST_TOKEN_RECT_Y: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rect_y, in_buf);
                        break;
                    }

                    case TEST_TOKEN_RECT_WIDTH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rect_width, in_buf);
                        break;
                    }

                    case TEST_TOKEN_RECT_HEIGHT: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rect_height, in_buf);
                        break;
                    }

                    case TEST_TOKEN_BASE_CALIBRATE_SWITCH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.base_calibrate_switch,
                                                             in_buf);
                        break;
                    }

                    case TEST_TOKEN_LDC_SWITCH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.ldc_switch, in_buf);
                        break;
                    }

                    case TEST_TOKEN_TNR_RESET_THRESH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.tnr_thresh, in_buf);
                        break;
                    }

                    case TEST_TOKEN_TNR_RESET_FLAG: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.tnr_switch, in_buf);
                        break;
                    }

                    case TEST_TOKEN_LPF_SWITCH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.lpf_switch, in_buf);
                        break;
                    }

                    case TEST_TOKEN_LSC_SWITCH: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.lsc_switch, in_buf);
                        break;
                    }

                    case TEST_TOKEN_ENHANCE_LEVEL: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.enhance_level, in_buf);
                        break;
                    }

                    case TEST_TOKEN_RECT_BMP_COL: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rect_bmp_col, in_buf);
                        break;
                    }

                    case TEST_TOKEN_RECT_BMP_ROW: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rect_bmp_row, in_buf);
                        break;
                    }

                    case TEST_TOKEN_AUTHENTICATE_RETRYCOUNT: {
                        in_buf = TestUtils::testDecodeUint32(&RetryCount, in_buf);
                        break;
                    }

                    case TEST_TOKEN_SENSOR_PREVIEW_ENHANCED: {
                        in_buf = TestUtils::testDecodeUint32(&capture_param.rawdata_process_switch,
                                                             in_buf);
                        // g_rawdata_process_flag = capture_param->rawdata_process_switch;
                        break;
                    }

                    default:
                        break;
                }
            } while (in_buf < cmd_buf + cmd_len);

            capture_param.fusionratio = (float)(Fusionratio) / 1000;
            LOG_D(LOG_TAG, "[%s] scale_ratio %d", __func__, capture_param.scale_ratio);
            LOG_D(LOG_TAG, "[%s] long_frame_avg_num %d", __func__,
                  capture_param.long_frame_avg_num);
            LOG_D(LOG_TAG, "[%s] long_pga_gain %d", __func__, capture_param.long_pga_gain);
            LOG_D(LOG_TAG, "[%s] short_exposure_time %d", __func__,
                  capture_param.short_exposure_time);
            LOG_D(LOG_TAG, "[%s] short_frame_avg_num %d", __func__,
                  capture_param.short_frame_avg_num);
            LOG_D(LOG_TAG, "[%s] short_pga_gain %d", __func__,
                  capture_param.short_pga_gain);
            LOG_D(LOG_TAG, "[%s] unlowerthresh %d", __func__, capture_param.unlowerthresh);
            LOG_D(LOG_TAG, "[%s] unhighthresh %d", __func__, capture_param.unhighthresh);
            LOG_D(LOG_TAG, "[%s] Fusionratio %d", __func__, Fusionratio);
            LOG_D(LOG_TAG, "[%s] capture_param.fusionratio %f", __func__,
                  capture_param.fusionratio);
            LOG_D(LOG_TAG, "[%s] Preprocess 0x%x", __func__, capture_param.preprocess);
            LOG_D(LOG_TAG, "[%s] rect_x %d", __func__, capture_param.rect_x);
            LOG_D(LOG_TAG, "[%s] rect_y %d", __func__, capture_param.rect_y);
            LOG_D(LOG_TAG, "[%s] rect_width %d", __func__, capture_param.rect_width);
            LOG_D(LOG_TAG, "[%s] rect_height %d", __func__, capture_param.rect_height);
            LOG_D(LOG_TAG, "[%s] base_calibrate_switch %d", __func__,
                  capture_param.base_calibrate_switch);
            LOG_D(LOG_TAG, "[%s] ldc_switch %d", __func__, capture_param.ldc_switch);
            LOG_D(LOG_TAG, "[%s] tnr_thresh %d", __func__, capture_param.tnr_thresh);
            LOG_D(LOG_TAG, "[%s] tnr_switch %d", __func__, capture_param.tnr_switch);
            LOG_D(LOG_TAG, "[%s] lpf_switch %d", __func__, capture_param.lpf_switch);
            LOG_D(LOG_TAG, "[%s] lsc_switch %d", __func__, capture_param.lsc_switch);
            LOG_D(LOG_TAG, "[%s] enhance_level %d", __func__, capture_param.enhance_level);
            LOG_D(LOG_TAG, "[%s] rect_bmp_col %d", __func__, capture_param.rect_bmp_col);
            LOG_D(LOG_TAG, "[%s] rect_bmp_row %d", __func__, capture_param.rect_bmp_row);
            LOG_D(LOG_TAG, "[%s] retrycount %d", __func__, RetryCount);
            // g_authenticate_retrycount = mRetryCount;
            LOG_D(LOG_TAG, "[%s] rawdata_process_switch %d", __func__,
                  capture_param.rawdata_process_switch);
            capture_param.cmd_header.cmd_id = GF_SZ_CMD_TEST_SEND_CAPTURE_PARAM;
            capture_param.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand((gf_sz_capture_param_t *)&capture_param,
                                sizeof(gf_sz_capture_param_t));
            ((SZSensor*)mContext->mSensor)->setMulExpoLevel(SETTING_CUSTOMER_EXPO, 1);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::getVersion(void) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_test_get_version_t cmd = {{ 0 }};
        int8_t *test_result = NULL;
        uint32_t len = 0;
        uint32_t size = sizeof(gf_sz_test_get_version_t);
        FUNC_ENTER();

        do {
            memset(&cmd, 0, size);
            cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_GET_VERSION;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, size);

            if (err != GF_SUCCESS) {
                break;
            }

            LOG_D(LOG_TAG, "[%s]       algo_version=%s", __func__, cmd.algo_version);
            LOG_D(LOG_TAG, "[%s] preprocess_version=%s", __func__, cmd.preprocess_version);
            LOG_D(LOG_TAG, "[%s]         fw_version=%s", __func__, cmd.fw_version);
            LOG_D(LOG_TAG, "[%s]        tee_version=%s", __func__, cmd.tee_version);
            LOG_D(LOG_TAG, "[%s]         ta_version=%s", __func__, cmd.ta_version);
            LOG_D(LOG_TAG, "[%s]            chip_id=0x%02X%02X%02X%02X", __func__,
                  cmd.chip_id[0],
                  cmd.chip_id[1], cmd.chip_id[2], cmd.chip_id[3]);
            LOG_D(LOG_TAG, "[%s]          vendor_id=0x%02X%02X%02X%02X", __func__,
                  cmd.vendor_id[0],
                  cmd.vendor_id[1], cmd.vendor_id[2], cmd.vendor_id[3]);
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // int8_t algo_version[ALGO_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(ALGO_VERSION_INFO_LEN);
            // int8_t preprocess_version[ALGO_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(ALGO_VERSION_INFO_LEN);
            // int8_t fake_version[ALGO_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(ALGO_VERSION_INFO_LEN);
            // int8_t fw_version[FW_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(FW_VERSION_INFO_LEN);
            // int8_t tee_version[TEE_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(TEE_VERSION_INFO_LEN);
            // int8_t ta_version[TA_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(TA_VERSION_INFO_LEN);
            // int8_t package_version[TA_VERSION_INFO_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(TA_VERSION_INFO_LEN);
            // int8_t chip_id[GF_CHIP_ID_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(GF_CHIP_ID_LEN);
            // uint8_t vendor_id[GF_VENDOR_ID_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(GF_VENDOR_ID_LEN);
            // uint8_t sensor_id[GF_SENSOR_ID_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(GF_SENSOR_ID_LEN);
            // uint8_t production_date[PRODUCTION_DATE_LEN];
            len += HAL_TEST_SIZEOF_ARRAY(PRODUCTION_DATE_LEN);
            // uint8_t factory_algo_version[TEST_ALGO_VERSION_LEN]
            len += HAL_TEST_SIZEOF_ARRAY(TEST_ALGO_VERSION_LEN);
            // uint8_t ad_version[GF_AD_VERSION_LEN]
            len += HAL_TEST_SIZEOF_ARRAY(GF_AD_VERSION_LEN);
            // uint8_t lens_type[GF_LENS_TYPE_LEN]
            len += HAL_TEST_SIZEOF_ARRAY(GF_LENS_TYPE_LEN);
            test_result = (int8_t *) malloc(len);

            if (NULL == test_result) {
                len = 0;
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(test_result, 0, len);
            int8_t *current = test_result;
            current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_ALGO_VERSION,
                                                 (int8_t *) cmd.algo_version, ALGO_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_PREPROCESS_VERSION,
                                                 (int8_t *) cmd.preprocess_version, ALGO_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_FAKE_VERSION,
                                                 (int8_t *) cmd.fake_version, ALGO_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_FW_VERSION,
                                                 (int8_t *) cmd.fw_version, FW_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_TEE_VERSION,
                                                 (int8_t *) cmd.tee_version, TEE_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_TA_VERSION,
                                                 (int8_t *) cmd.ta_version, TA_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_PACKAGE_VERSION,
                                                 (int8_t *) cmd.package_version, TA_VERSION_INFO_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_CHIP_ID,
                                                 (int8_t *) cmd.chip_id,
                                                 GF_CHIP_ID_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_VENDOR_ID,
                                                 (int8_t *) cmd.vendor_id,
                                                 GF_VENDOR_ID_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_SENSOR_ID,
                                                 (int8_t *) cmd.sensor_id,
                                                 GF_SENSOR_ID_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_PRODUCTION_DATE,
                                                 (int8_t *) cmd.production_date,
                                                 PRODUCTION_DATE_LEN);
            current = TestUtils::testEncodeArray(current,
                                                 TEST_TOKEN_PRODUCTION_ALGO_VERSION,
                                                 (int8_t *) cmd.factory_algo_version,
                                                 TEST_ALGO_VERSION_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_AD_VERSION,
                                                 (int8_t *) cmd.ad_version,
                                                 GF_AD_VERSION_LEN);
            current = TestUtils::testEncodeArray(current, TEST_TOKEN_LENS_TYPE,
                                                 (int8_t *) cmd.lens_type,
                                                 GF_LENS_TYPE_LEN);
            TestUtils::testMemoryCheck(__func__, test_result, current, len);
            notifyTestCmd(0, CMD_TEST_SZ_GET_VERSION, test_result, len);
        } while (0);

        if (test_result != NULL) {
            free(test_result);
        }

        FUNC_EXIT(err);
        return err;
    }

    static gf_error_t weakrawdataToBmp(uint16_t *rawdata, uint32_t rawdata_len,
                                       uint8_t *bmpdata,
                                       uint32_t bmpdata_len, uint32_t width, uint32_t height) {
        gf_error_t err = GF_SUCCESS;
        uint32_t i = 0;
        uint32_t max = 0;
        uint32_t min = 0;
        FUNC_ENTER();
        UNUSED_VAR(width);
        UNUSED_VAR(height);

        if (rawdata_len != bmpdata_len) {
            LOG_E(LOG_TAG, "invalid param [%s]--------exit--------", __FUNCTION__);
            err = GF_ERROR_BAD_PARAMS;
            return err;
        }

        memset(bmpdata, 0, sizeof(uint8_t) * bmpdata_len);

        for (i = 1; i < rawdata_len; i++) {
            if (rawdata[i] > max) {
                max = rawdata[i];
            }

            if (rawdata[i] < min) {
                min = rawdata[i];
            }
        }

        if (max == min) {
            return err;
        }

        for (i = 0; i < rawdata_len; i++) {
            bmpdata[i] = (uint8_t)((rawdata[i] - min) * 255 / (max - min));
        }

        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZProductTest::cropKbcalidata(uint16_t *kbcalidata,
                                             uint32_t data_width, uint32_t data_height, gf_sz_image_crop_rect_t *cropRect,
                                             uint16_t *cropKbcalidata) {
        gf_error_t err = GF_SUCCESS;
        uint32_t row_index = 0;
        uint16_t *cropLine = NULL;
        FUNC_ENTER();
        UNUSED_VAR(data_height);

        do {
            if (kbcalidata == NULL || cropRect == NULL || cropKbcalidata == NULL) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            cropLine = cropKbcalidata;

            for (row_index = cropRect->y; row_index < cropRect->y + cropRect->height;
                 row_index++) {
                memcpy(cropLine, kbcalidata + row_index * data_width + cropRect->x,
                       cropRect->width * sizeof(uint16_t));
                cropLine += cropRect->width;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::notifyFusionPreview(gf_error_t err,
                                            gf_sz_fusion_preview_t *cmd) {
        int8_t *test_result = NULL;
        int8_t *long_exposure_bmp = NULL;
        int8_t *short_exposure_bmp = NULL;
        uint32_t len = 0;
        VOID_FUNC_ENTER();

        do {
            uint32_t fusion_array_len = cmd->crop_width * cmd->crop_height;
            uint32_t rawdata_array_len = cmd->col * cmd->row;
            uint32_t weak_data_len = cmd->weak_data_col * cmd->weak_data_row;
            long_exposure_bmp = new int8_t[rawdata_array_len] { 0 };

            if (NULL == long_exposure_bmp) {
                LOG_E(LOG_TAG, "[%s] Out of memory", __func__);
                break;
            }

            rawdataToBmp(cmd->rawdata.data, rawdata_array_len,
                         (uint8_t *)long_exposure_bmp, rawdata_array_len,
                         cmd->col, cmd->row);
            short_exposure_bmp = new int8_t[rawdata_array_len] { 0 };

            if (NULL == short_exposure_bmp) {
                LOG_E(LOG_TAG, "[%s] Out of memory", __func__);
                break;
            }

            rawdataToBmp(cmd->rawdata.data, rawdata_array_len,
                         (uint8_t *)short_exposure_bmp, rawdata_array_len,
                         cmd->col, cmd->row);
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // fusion_bmp_data
            len += HAL_TEST_SIZEOF_ARRAY(fusion_array_len);
            // long exposure rawdata for weak image
            len += HAL_TEST_SIZEOF_ARRAY(weak_data_len);
            // short exposure raw data
            len += HAL_TEST_SIZEOF_ARRAY(rawdata_array_len);
            // fusion_bmp_data width
            len += HAL_TEST_SIZEOF_INT32;
            // fusion_bmp_data height
            len += HAL_TEST_SIZEOF_INT32;
            // long exposure rawdata width
            len += HAL_TEST_SIZEOF_INT32;
            // long exposure rawdata height
            len += HAL_TEST_SIZEOF_INT32;
            // short exposure rawdata width
            len += HAL_TEST_SIZEOF_INT32;
            // short exposure rawdata height
            len += HAL_TEST_SIZEOF_INT32;
            // weak data col
            len += HAL_TEST_SIZEOF_INT32;
            // weak data row
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (NULL != test_result) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE,
                                                     err);
                current = TestUtils::testEncodeArray(current, TEST_TOKEN_FUSION_DATA,
                                                     (int8_t *) cmd->fusion_data, sizeof(uint8_t) * fusion_array_len);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FUSION_WIDTH,
                                                     cmd->crop_width);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FUSION_HEIGHT,
                                                     cmd->crop_height);
                current = TestUtils::testEncodeArray(current, TEST_TOKEN_LONG_EXPOSURE_DATA,
                                                     (int8_t *)cmd->weak_process_data, sizeof(uint8_t) * weak_data_len);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_LONG_EXPOSURE_WIDTH,
                                                     cmd->crop_width);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_LONG_EXPOSURE_HEIGHT,
                                                     cmd->crop_height);
                current = TestUtils::testEncodeArray(current, TEST_TOKEN_SHORT_EXPOSURE_DATA,
                                                     (int8_t *)short_exposure_bmp, sizeof(uint8_t) * rawdata_array_len);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SHORT_EXPOSURE_WIDTH,
                                                     cmd->col);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SHORT_EXPOSURE_HEIGHT,
                                                     cmd->row);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_WEAK_DATA_WIDTH,
                                                     cmd->weak_data_col);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_WEAK_DATA_HEIGHT,
                                                     cmd->weak_data_row);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_SZ_FUSION_PREVIEW, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        if (short_exposure_bmp != NULL) {
            delete []short_exposure_bmp;
        }

        if (long_exposure_bmp != NULL) {
            delete []long_exposure_bmp;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t SZProductTest::testFusion(void) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_fusion_preview_t cmd = {{ 0 }};
        gf_sz_image_crop_rect_t cropRect;
        uint16_t croped_kbcalidata[IMAGE_BUFFER_LEN];
        gf_sz_test_bmp_data_t test_bmp_data;
        uint32_t crop_len;
        FUNC_ENTER();

        do {
            memset(&cmd, 0, sizeof(gf_sz_fusion_preview_t));
            cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_FUSION;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, sizeof(gf_sz_fusion_preview_t));

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] invoke command failed ", __func__);
                break;
            }

            LOG_D(LOG_TAG, "[%s] crop rect: x = %d y = %d width = %d height = %d", __func__,
                  cmd.image_crop_rect.x, cmd.image_crop_rect.y,
                  cmd.image_crop_rect.width, cmd.image_crop_rect.height);
            LOG_D(LOG_TAG, "[%s] cmd->crop_width = %d cmd->crop_height = %d", __func__,
                  cmd.crop_width, cmd.crop_height);
            LOG_D(LOG_TAG, "[%s] cmd->weak_data_col = %d cmd->weak_data_row = %d", __func__,
                  cmd.weak_data_col, cmd.weak_data_row);
            cropRect = cmd.image_crop_rect;
            err = cropKbcalidata(cmd.KBcalidate, cmd.weak_data_col, cmd.weak_data_row,
                                 &cropRect, croped_kbcalidata);

            if (GF_SUCCESS != err) {
                err = GF_ERROR_PREPROCESS_FAILED;
                break;
            }

            crop_len = cropRect.width * cropRect.height;
            err = weakrawdataToBmp(croped_kbcalidata, crop_len, test_bmp_data.bmp_data,
                                   crop_len,
                                   cropRect.width, cropRect.height);

            if (GF_SUCCESS != err) {
                err = GF_ERROR_PREPROCESS_FAILED;
                break;
            }

            if (GF_SUCCESS == err) {
                cmd.weak_data_col = cropRect.width;
                cmd.weak_data_row = cropRect.height;
                memcpy(cmd.weak_process_data, test_bmp_data.bmp_data,
                       sizeof(test_bmp_data.bmp_data));
                notifyFusionPreview(err, &cmd);
            }
#ifdef SUPPORT_DUMP
            SZHalDump *szdump = static_cast<SZHalDump *>(mContext->mHalDump);
            err = szdump->szDumpPreview((uint8_t *) &cmd,
                                        sizeof(gf_sz_fusion_preview_t),
                                        SZHalDump::DUMP_FUSION_PREVIEW_DATA,
                                        init_timestamp, sizeof(init_timestamp), ft_time, err);
#endif  // SUPPORT_DUMP
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZProductTest::sleepSensor(void) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_test_sleep_t *cmd = NULL;
        uint32_t size = sizeof(gf_sz_test_sleep_t);
        FUNC_ENTER();

        do {
            cmd = (gf_sz_test_sleep_t *)malloc(size);

            if (NULL == cmd) {
                LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }

            memset(cmd, 0, size);
            cmd->cmd_header.cmd_id = GF_SZ_CMD_TEST_SLEEP;
            cmd->cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(cmd, sizeof(gf_sz_test_sleep_t));

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] sleep sensor invoke command failed ", __func__);
                break;
            }
        } while (0);

        if (cmd != NULL) {
            free(cmd);
        }

        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::notifyRawdataPreview(gf_error_t err,
                                             gf_sz_rawdata_preview_t *raw_data) {
        int8_t *test_result = NULL;
        int8_t *short_exposure_bmp = NULL;
        uint32_t rawdata_array_len = 0;
        uint32_t len = 0;
        VOID_FUNC_ENTER();

        do {
            if (NULL == raw_data) {
                LOG_E(LOG_TAG, "[%s] GF_ERROR_BAD_PARAMS", __func__);
                return;
            }

            rawdata_array_len = raw_data->col * raw_data->row;
            short_exposure_bmp = new int8_t[rawdata_array_len] { 0 };

            if (NULL == short_exposure_bmp) {
                LOG_E(LOG_TAG, "[%s] Out of memory", __func__);
                break;
            }

            rawdataToBmp(raw_data->rawdata_short.data, rawdata_array_len,
                         (uint8_t *)short_exposure_bmp, rawdata_array_len,
                         raw_data->col, raw_data->row);
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // short exposure raw data
            len += HAL_TEST_SIZEOF_ARRAY(rawdata_array_len);
            // short exposure rawdata width
            len += HAL_TEST_SIZEOF_INT32;
            // short exposure rawdata height
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (NULL != test_result) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE,
                                                     err);
                current = TestUtils::testEncodeArray(current, TEST_TOKEN_SHORT_EXPOSURE_DATA,
                                                     short_exposure_bmp, sizeof(uint8_t) * rawdata_array_len);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SHORT_EXPOSURE_WIDTH,
                                                     raw_data->col);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SHORT_EXPOSURE_HEIGHT,
                                                     raw_data->row);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_SZ_RAWDATA_PREVIEW, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        if (short_exposure_bmp != NULL) {
            delete []short_exposure_bmp;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t SZProductTest::testRawdataPreview(void) {
        gf_sz_rawdata_preview_t cmd = {{ 0 }};
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            memset(&cmd, 0, sizeof(gf_sz_rawdata_preview_t));
            cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_RAWDATA_PREVIEW;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, sizeof(gf_sz_rawdata_preview_t));

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] test Exposure invoke command failed ", __func__);
                break;
            }

            notifyRawdataPreview(err, &cmd);
#ifdef SUPPORT_DUMP
            SZHalDump *szdump = static_cast<SZHalDump *>(mContext->mHalDump);
            err = szdump->szDumpPreview((uint8_t *) &(cmd.rawdata_short),
                                        sizeof(gf_sz_test_rawdata_t),
                                        SZHalDump::DUMP_RAWDATA_PREVIEW_DATA,
                                        init_timestamp, sizeof(init_timestamp), ft_time, err);
#endif  // SUPPORT_DUMP
        } while (0);

        // sleepSensor();
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::setEnrollTemplateCount(const int8_t *cmd_buf,
                                                     uint32_t cmd_len) {
        gf_error_t err = GF_SUCCESS;
        uint32_t token = 0;
        gf_sz_test_enroll_template_count_t enroll_template_count = {{ 0 }};
        uint32_t len = 0;
        int8_t *test_result = NULL;
        FUNC_ENTER();
        UNUSED_VAR(cmd_len);

        do {
            const int8_t *in_buf = cmd_buf;

            if (NULL == in_buf) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            memset(&enroll_template_count, 0, sizeof(gf_sz_test_enroll_template_count_t));
            in_buf = TestUtils::testDecodeUint32(&token, in_buf);
            LOG_D(LOG_TAG, "[%s] token = %d", __func__, token);

            switch (token) {
                case TEST_TOKEN_ENROLL_TEMPLATE_COUNT: {
                    in_buf = TestUtils::testDecodeUint32(&enroll_template_count.template_count,
                                                         in_buf);
                    break;
                }

                default:
                    break;
            }

            LOG_D(LOG_TAG, "[%s] set template_count = %d", __func__,
                  enroll_template_count.template_count);
            enroll_template_count.cmd_header.cmd_id = GF_SZ_CMD_TEST_ENROLL_TEMPLATE_COUNT;
            enroll_template_count.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand((gf_sz_test_enroll_template_count_t *)
                                &enroll_template_count, sizeof(gf_sz_test_enroll_template_count_t));
            LOG_D(LOG_TAG, "[%s] get template_count = %d", __func__,
                  enroll_template_count.template_count);
            len += HAL_TEST_SIZEOF_INT32;
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ENROLL_TEMPLATE_COUNT,
                                                     enroll_template_count.template_count);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_SZ_ENROLL_TEMPLATE_COUNT, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::getConfig(int8_t **cfgBuf, uint32_t *bufLen) {
        gf_error_t err = GF_SUCCESS;
        int8_t *test_result = NULL;
        gf_sz_test_get_config_t cmd = {{ 0 }};
        uint32_t len = 0;
        FUNC_ENTER();

        do {
            if (NULL == cfgBuf || NULL == bufLen) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            memset(&cmd, 0, sizeof(gf_sz_test_get_config_t));
            cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_GET_CONFIG;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, sizeof(gf_sz_test_get_config_t));
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // gf_chip_type_t chip_type;
            len += HAL_TEST_SIZEOF_INT32;
            // gf_chip_series_t chip_series;
            len += HAL_TEST_SIZEOF_INT32;
            // error_code
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t max_fingers;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t max_fingers_per_user;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t forbidden_untrusted_enroll;
            len += HAL_TEST_SIZEOF_INT32;
            // uint8_t forbidden_enroll_duplicate_fingers;
            len += HAL_TEST_SIZEOF_INT32;
            // uint8_t support_performance_dump;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t enrolling_min_templates;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t valid_image_quality_threshold;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t valid_image_area_threshold;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t duplicate_finger_overlay_score;
            len += HAL_TEST_SIZEOF_INT32;
            // uint32_t increase_rate_between_stitch_info;
            len += HAL_TEST_SIZEOF_INT32;
            // gf_authenticate_order_t authenticate_order;
            len += HAL_TEST_SIZEOF_INT32;
            // bmp_col
            len += HAL_TEST_SIZEOF_INT32;
            // bmp_row
            len += HAL_TEST_SIZEOF_INT32;
            // scale_ratio
            len += HAL_TEST_SIZEOF_INT32;
            // sensor_col
            len += HAL_TEST_SIZEOF_INT32;
            // sensor_row
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_CHIP_TYPE,
                                                     mContext->mSensorInfo.chip_type);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_CHIP_SERIES,
                                                     mContext->mSensorInfo.chip_series);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MAX_FINGERS,
                                                     mContext->mConfig->max_fingers);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MAX_FINGERS_PER_USER,
                                                     mContext->mConfig->max_fingers_per_user);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL,
                                                     (int32_t) mContext->mConfig->forbidden_untrusted_enroll);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS,
                                                     (int32_t) mContext->mConfig->forbidden_enroll_duplicate_fingers);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP,
                                                     (int32_t) mContext->mConfig->support_performance_dump);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_ENROLLING_MIN_TEMPLATES,
                                                     mContext->mConfig->enrolling_min_templates);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD,
                                                     mContext->mConfig->valid_image_quality_threshold);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD,
                                                     mContext->mConfig->valid_image_area_threshold);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE,
                                                     mContext->mConfig->duplicate_finger_overlay_score);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO,
                                                     mContext->mConfig->increase_rate_between_stitch_info);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_AUTHENTICATE_ORDER,
                                                     mContext->mConfig->authenticate_order);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_RECT_BMP_COL,
                                                     cmd.bmp_col);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_RECT_BMP_ROW,
                                                     cmd.bmp_row);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SCALE_RATIO_VALUE,
                                                     cmd.scale_ratio);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SENSOR_COL,
                                                     cmd.sensor_col);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SENSOR_ROW,
                                                     cmd.sensor_row);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_SZ_GET_CONFIG, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::notifyCalibrationFinish(gf_error_t err) {
        uint32_t len = 0;
        int8_t *test_result = NULL;
        VOID_FUNC_ENTER();

        do {
            len += HAL_TEST_SIZEOF_INT32;
            len += HAL_TEST_SIZEOF_INT32;
            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_CALIBRATION_ALGO_FINISHED_FLAG,
                                                     1);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE,
                                                     err);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            notifyTestCmd(0, CMD_TEST_SZ_K_B_CALIBRATION, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void *SZProductTest::gfCaptureBaseThread(gf_sz_k_b_calibrate_step_t kbStep) {
        gf_sz_k_b_calibrate_t cmd = {{ 0 }};
        uint32_t size = sizeof(gf_sz_k_b_calibrate_t);
        gf_error_t err = GF_SUCCESS;
        VOID_FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] step=%d", __func__, kbStep);

        do {
            memset(&cmd, 0, size);
            cmd.k_b_step = kbStep;
            cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_K_B_CALIBRATION;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, size);
            notifyCalibrationFinish(err);
        } while (0);

        handle_thread = 0;
        VOID_FUNC_EXIT();
        return NULL;
    }

    gf_error_t SZProductTest::testKbCalibration(const int8_t *cmd_buf,
                                                uint8_t cmd_len) {
        gf_error_t err = GF_SUCCESS;
        const int8_t *in_buf = cmd_buf;
        uint32_t token = 0;
        FUNC_ENTER();

        do {
            if (NULL == in_buf) {
                LOG_E(LOG_TAG, "[%s] cmd buf is null", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            in_buf = TestUtils::testDecodeUint32(&token, in_buf);
            LOG_D(LOG_TAG, "token = %u", token);

            switch (token) {
                case TEST_TOKEN_CALIBRATION_STEP: {
                    in_buf = TestUtils::testDecodeUint32(&kb_step, in_buf);  // todo
                    break;
                }

                default:
                    break;
            }
        } while (in_buf < cmd_buf + cmd_len);

        LOG_D(LOG_TAG, "kb_step = %d", kb_step);
        gfCaptureBaseThread((gf_sz_k_b_calibrate_step_t)kb_step);
        /*    if (pthread_create(&handle_thread, NULL, gf_capture_base_thread, kb_step) != 0)
            {
                LOG_E(LOG_TAG, "[%s] pthread_create failed", __func__);
            }
            else
            {
                pthread_detach(handle_thread);
            }*/
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::setInitTimeStamp(void) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            memset(init_timestamp, 0, sizeof(init_timestamp));
            HalUtils::genTimestamp(init_timestamp, sizeof(init_timestamp));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::factoryInit(const int8_t *cmd_buf, uint32_t cmd_len) {
        gf_error_t err = GF_SUCCESS;
        const int8_t *in_buf = cmd_buf;
        uint32_t token = 0;
        uint32_t data_len = 0;
        gf_sz_factory_init cmd = {{ 0 }};
        int8_t *test_result = NULL;
        uint32_t len = 0;
        uint32_t display_vendor = 0;
        fp_eng_tests_result eng_result;

        FUNC_ENTER();

        do {
            if (NULL == in_buf) {
                LOG_E(LOG_TAG, "[%s] cmd buf is null", __func__);
                break;
            }

            in_buf = TestUtils::testDecodeUint32(&token, in_buf);
            LOG_D(LOG_TAG, "token = %u", token);

            switch (token) {
                case TEST_TOKEN_METADATA: {
                    memset(ft_time, 0, sizeof(ft_time));
                    // get path
                    in_buf = TestUtils::testDecodeUint32(&data_len, in_buf);
                    memcpy(ft_time, in_buf, data_len);
                    in_buf += data_len;
                    break;
                }

                case TEST_TOKEN_DISPLAY_TYPE: {
                    in_buf = TestUtils::testDecodeUint32(&display_vendor, in_buf);
                    break;
                }

                default:
                    break;
            }
        } while (in_buf < cmd_buf + cmd_len);

        do {
            /*    memset(init_timestamp, 0, sizeof(init_timestamp));
            HalUtils::genTimestamp(init_timestamp, sizeof(init_timestamp));*/
            cmd.cmd_header.cmd_id = GF_SZ_CMD_FACTORY_TEST_INIT;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            cmd.i_factory_config.i_display_vendor = display_vendor;
            err = invokeCommand(&cmd, sizeof(gf_sz_factory_init));
            LOG_D(LOG_TAG, "[%s] factory config->maxBadPointNum: %d", __func__,
                  cmd.o_factory_config.o_maxBadPointNum);
            LOG_D(LOG_TAG, "[%s] factory config->maxClusterNum: %d", __func__,
                  cmd.o_factory_config.o_maxClusterNum);
            LOG_D(LOG_TAG, "[%s] factory config->maxPixelOfLargestBadCluster: %d", __func__,
                  cmd.o_factory_config.o_maxPixelOfLargestBadCluster);
            LOG_D(LOG_TAG, "[%s] factory config->maxPixelOfLargestOrientBadCluster: %d", __func__,
                  cmd.o_factory_config.o_maxPixelOfLargestOrientBadCluster);
            LOG_D(LOG_TAG, "[%s] factory config->maxLightNoiseT: %f", __func__,
                  cmd.o_factory_config.o_maxLightNoiseT);
            LOG_D(LOG_TAG, "[%s] factory config->maxLightNoiseS: %f", __func__,
                  cmd.o_factory_config.o_maxLightNoiseS);
            LOG_D(LOG_TAG, "[%s] factory config->minFleshTouchDiff: %f", __func__,
                  cmd.o_factory_config.o_minFleshTouchDiff);
            LOG_D(LOG_TAG, "[%s] factory config->minFovArea: %d", __func__,
                  cmd.o_factory_config.o_minFovArea);
            LOG_D(LOG_TAG, "[%s] factory config->maxLightLeakRatio: %f", __func__,
                  cmd.o_factory_config.o_maxLightLeakRatio);
            LOG_D(LOG_TAG, "[%s] factory config->minRelativeIlluminance: %f", __func__,
                  cmd.o_factory_config.o_minRelativeIlluminance);
            LOG_D(LOG_TAG, "[%s] factory config->maxScaleRatio: %f", __func__,
                  cmd.o_factory_config.o_maxScaleRatio);
            LOG_D(LOG_TAG, "[%s] factory config->minScaleRatio: %f", __func__,
                  cmd.o_factory_config.o_minScaleRatio);
            LOG_D(LOG_TAG, "[%s] factory config->minMaskCropArea: %d", __func__,
                  cmd.o_factory_config.o_minMaskCropArea);
            LOG_D(LOG_TAG, "[%s] factory config->minSSNR: %f", __func__,
                  cmd.o_factory_config.o_minSSNR);
            LOG_D(LOG_TAG, "[%s] factory config->minShapeness: %f", __func__,
                  cmd.o_factory_config.o_minShapeness);
            LOG_D(LOG_TAG, "[%s] factory config->minP2P: %f", __func__,
                  cmd.o_factory_config.o_minP2P);
            LOG_D(LOG_TAG, "[%s] factory config->minChartConstrast: %f", __func__,
                  cmd.o_factory_config.o_minChartConstrast);
            LOG_D(LOG_TAG, "[%s] factory config->nEffRegRad: %d", __func__,
                  cmd.o_factory_config.o_nEffRegRad);
            LOG_D(LOG_TAG, "[%s] factory config->nEffRegRad2: %d", __func__,
                  cmd.o_factory_config.o_nEffRegRad2);
            LOG_D(LOG_TAG, "[%s] factory config->nSPMTBadPixThd: %d", __func__,
                  cmd.o_factory_config.o_nSPMTBadPixThd);
            LOG_D(LOG_TAG, "[%s] factory config->nSPMTBadPixThd2: %d", __func__,
                  cmd.o_factory_config.o_nSPMTBadPixThd2);
            LOG_D(LOG_TAG, "[%s] factory config->nSaturatPixHighThd: %d", __func__,
                  cmd.o_factory_config.o_nSaturatPixHighThd);
            LOG_D(LOG_TAG, "[%s] factory config->chartDirectionThd: %d", __func__,
                  cmd.o_factory_config.o_chartDirectionThd);
            LOG_D(LOG_TAG, "[%s] factory config->chartDirectionTarget: %d", __func__,
                  cmd.o_factory_config.o_chartDirectionTarget);
            LOG_D(LOG_TAG, "[%s] factory config->maxScreenStructRatio: %f", __func__,
                  cmd.o_factory_config.o_maxScreenStructRatio);
            LOG_D(LOG_TAG, "[%s] factory config->maxCenterOffset: %f", __func__,
                  cmd.o_factory_config.o_maxCenterOffset);
            LOG_D(LOG_TAG, "[%s] factory config->simpleCaliScaleOffset: %f", __func__,
                  cmd.o_factory_config.o_simpleCaliScaleOffset);
            // Error code
            len += HAL_TEST_SIZEOF_INT32;
            // maxBadPointNum
            len += HAL_TEST_SIZEOF_INT32;
            // maxClusterNum
            len += HAL_TEST_SIZEOF_INT32;
            // maxPixelOfLargestBadCluster
            len += HAL_TEST_SIZEOF_INT32;
            // maxLightNoiseT
            len += HAL_TEST_SIZEOF_FLOAT;
            // maxLightNoiseS
            len += HAL_TEST_SIZEOF_FLOAT;
            // minFleshTouchDiff
            len += HAL_TEST_SIZEOF_FLOAT;
            // minFovArea
            len += HAL_TEST_SIZEOF_INT32;
            // maxLightLeakRatio
            len += HAL_TEST_SIZEOF_FLOAT;
            // minRelativeIlluminance
            len += HAL_TEST_SIZEOF_FLOAT;
            // maxScaleRatio
            len += HAL_TEST_SIZEOF_FLOAT;
            // minScaleRatio
            len += HAL_TEST_SIZEOF_FLOAT;
            // minMaskCropArea
            len += HAL_TEST_SIZEOF_INT32;
            // minSSNR
            len += HAL_TEST_SIZEOF_FLOAT;
            // minShapeness
            len += HAL_TEST_SIZEOF_FLOAT;
            // minP2P
            len += HAL_TEST_SIZEOF_FLOAT;
            // minChartConstrast
            len += HAL_TEST_SIZEOF_FLOAT;
            // nEffRegRad
            len += HAL_TEST_SIZEOF_INT32;
            // nEffRegRad2
            len += HAL_TEST_SIZEOF_INT32;
            // nSPMTBadPixThd
            len += HAL_TEST_SIZEOF_INT32;
            // nSPMTBadPixThd2
            len += HAL_TEST_SIZEOF_INT32;
            // nSaturatPixHighThd
            len += HAL_TEST_SIZEOF_INT32;
            // chartDirectionThd
            len += HAL_TEST_SIZEOF_INT32;
            // chartDirectionTarget
            len += HAL_TEST_SIZEOF_INT32;
            // maxScreenStructRatio
            len += HAL_TEST_SIZEOF_FLOAT;
            // max_expo_time
            len += HAL_TEST_SIZEOF_INT32;
            // min_expo_time
            len += HAL_TEST_SIZEOF_INT32;
            // maxCenterOffset
            len += HAL_TEST_SIZEOF_FLOAT;
            // maxDarkNoiseS
            len += HAL_TEST_SIZEOF_FLOAT;
            // maxDarkNoiseT
            len += HAL_TEST_SIZEOF_FLOAT;
 
            test_result = new int8_t[len] { 0 };

            if (test_result != NULL) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE, err);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MAX_BAD_POINT_NUM,
                                                     cmd.o_factory_config.o_maxBadPointNum);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MAX_CLUSTER_NUM,
                                                     cmd.o_factory_config.o_maxClusterNum);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_MAX_PIXEL_OF_LARGEST_BAD_CLUSTER,
                                                     cmd.o_factory_config.o_maxPixelOfLargestBadCluster);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MAX_LIGHT_NOISET,
                                                     cmd.o_factory_config.o_maxLightNoiseT);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MAX_LIGHT_NOISES,
                                                     cmd.o_factory_config.o_maxLightNoiseS);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MIN_FLESH_TOUCH_DIFF,
                                                     cmd.o_factory_config.o_minFleshTouchDiff);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MIN_FOV_AREA,
                                                     cmd.o_factory_config.o_minFovArea);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MAX_LIGHT_LEAK_RATIO,
                                                     cmd.o_factory_config.o_maxLightLeakRatio);
                current = TestUtils::testEncodeFloat(current,
                                                     TEST_TOKEN_MIN_RELATIVE_ILLUMINANCE,
                                                     cmd.o_factory_config.o_minRelativeIlluminance);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MAX_SCALE_RATIO,
                                                     cmd.o_factory_config.o_maxScaleRatio);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MIN_SCALE_RATIO,
                                                     cmd.o_factory_config.o_minScaleRatio);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_MIN_MASK_CROP_AREA,
                                                     cmd.o_factory_config.o_minMaskCropArea);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MIN_SSNR,
                                                     cmd.o_factory_config.o_minSSNR);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MIN_SHAPENESS,
                                                     cmd.o_factory_config.o_minShapeness);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MIN_P2P,
                                                     cmd.o_factory_config.o_minP2P);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MIN_CHART_CONSTRAST,
                                                     cmd.o_factory_config.o_minChartConstrast);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_EFF_REG_RAD,
                                                     cmd.o_factory_config.o_nEffRegRad);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_EFF_REG_RAD2,
                                                     cmd.o_factory_config.o_nEffRegRad2);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SPMT_BAD_PIX_THD,
                                                     cmd.o_factory_config.o_nSPMTBadPixThd);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SPMT_BAD_PIX_THD2,
                                                     cmd.o_factory_config.o_nSPMTBadPixThd2);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_SATURAT_PIX_HIGH_THD,
                                                     cmd.o_factory_config.o_nSaturatPixHighThd);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_CHART_DIRECTION_THD,
                                                     cmd.o_factory_config.o_chartDirectionThd);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_CHART_DIRECTION_TARGET,
                                                     cmd.o_factory_config.o_chartDirectionTarget);
                current = TestUtils::testEncodeFloat(current,
                                                     TEST_TOKEN_MAX_SCREEN_STRUCT_RATIO,
                                                     cmd.o_factory_config.o_maxScreenStructRatio);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_MAX_EXPO_TIME,
                                                     cmd.o_factory_config.o_max_expo_time);
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_MIN_EXPO_TIME,
                                                     cmd.o_factory_config.o_min_expo_time);
                LOG_E(LOG_TAG, "[%s] cmd.o_factory_config.o_min_expo_time = %d", __func__, cmd.o_factory_config.o_min_expo_time);
                LOG_E(LOG_TAG, "[%s] cmd.o_factory_config.o_max_expo_time = %d", __func__, cmd.o_factory_config.o_max_expo_time);

                current = TestUtils::testEncodeFloat(current,
                                                     TEST_TOKEN_MAX_CENTER_OFFSET,
                                                     cmd.o_factory_config.o_maxCenterOffset);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MAX_DARK_SNOISE,
                                                     cmd.o_factory_config.o_maxDarkNoiseS);
                current = TestUtils::testEncodeFloat(current, TEST_TOKEN_MAX_DARK_TNOISE,
                                                     cmd.o_factory_config.o_maxDarkNoiseT);
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
            } else {
                len = 0;
            }

            mContext->mFingerprintCore->notifyFingerprintCmd(0, CMD_TEST_SZ_FT_INIT, test_result, len);
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::factoryExit(void) {
        gf_error_t err = GF_SUCCESS;
        gf_sz_factory_exit cmd = {{ 0 }};
        FUNC_ENTER();

        do {
            cmd.cmd_header.cmd_id = GF_SZ_CMD_FACTORY_TEST_EXIT;
            cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
            err = invokeCommand(&cmd, sizeof(gf_sz_factory_exit));
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::notifyEnumerate(int32_t cmdId,
                                        gf_fingerprint_finger_id_t *result, uint32_t max_size) {
        int8_t *test_result = NULL;
        uint32_t *fid = NULL;
        uint32_t len = 0;
        uint32_t i = 0;
        VOID_FUNC_ENTER();

        do {
            fid = new uint32_t[max_size] { 0 };
            // message type
            len += HAL_TEST_SIZEOF_INT32;
            // gid
            len += HAL_TEST_SIZEOF_INT32;
            // fid
            len += HAL_TEST_SIZEOF_ARRAY(max_size * sizeof(uint32_t));
            test_result = new int8_t[len] { 0 };

            if ((test_result != NULL) && (result != NULL)) {
                int8_t *current = test_result;
                current = TestUtils::testEncodeInt32(current,
                                                     TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_TEMPLATE_ENUMERATING);
                current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_GID,
                                                     result[0].gid);

                for (i = 0; i < max_size; i++) {
                    fid[i] = result[i].fid;
                }

                current = TestUtils::testEncodeArray(current,
                                                     TEST_TOKEN_FINGERPRINT_ENUMERATING_FID,
                                                     (int8_t *)fid, max_size * sizeof(uint32_t));
                TestUtils::testMemoryCheck(__func__, test_result, current, len);
                notifyTestCmd(0, cmdId, test_result, len);
            }
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        if (fid != NULL) {
            delete []fid;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t SZProductTest::setActiveGroup(const int8_t *in, uint32_t inLen) {
        gf_error_t err = GF_SUCCESS;
        const int8_t *in_buf = in;
        uint32_t tmp_group_id = SZ_UNTRUST_ENROLL_TEST_GID;
        uint32_t token = 0;
        FUNC_ENTER();

        do {
            if (NULL == in) {
                LOG_D(LOG_TAG, "[%s] in buffer is null ", __func__);
                break;
            }

            do {
                in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                switch (token) {
                    case TEST_TOKEN_SET_GROUP_ID: {
                        in_buf = TestUtils::testDecodeUint32(&tmp_group_id, in_buf);
                        LOG_D(LOG_TAG, "[%s] group id  %u", __func__, tmp_group_id);

                        if (tmp_group_id != group_id) {
                            group_id = tmp_group_id;
                            szFingerprintCore->setActiveGroup(group_id);
                        }

                        break;
                    }

                    default: {
                        group_id = SZ_UNTRUST_ENROLL_TEST_GID;
                        szFingerprintCore->setActiveGroup(group_id);
                        LOG_D(LOG_TAG, "[%s] use default group id  %u", __func__, group_id);
                        break;
                    }
                }
            } while (in_buf < in + inLen);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }
    gf_error_t SZProductTest::executeCommand(int32_t cmdId, const int8_t *in,
                                             uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        // ProductTest::executeCommand(cmdId, in, inLen, out, outLen);

        do {
            switch (cmdId) {
                case CMD_TEST_SZ_FINGER_DOWN: {
                    mContext->mCenter->postEvent(EVENT_FINGER_DOWN);
                    break;
                }

                case CMD_TEST_SZ_FINGER_UP: {
                    mContext->mCenter->postEvent(EVENT_FINGER_UP);
                    break;
                }

                case CMD_TEST_SZ_LOCAL_AREA_SAMPLE: {
                    int8_t *test_result = NULL;
                    uint32_t len = 0;

                    do {
                        err = localAreaSample();
                        len += HAL_TEST_SIZEOF_INT32;
                        test_result = new int8_t[len] { 0 };

                        if (test_result != NULL) {
                            int8_t *current = test_result;
                            current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE,
                                                                 GF_ERROR_WITHOUT_OFFSET(err, GF_ERROR_SZ_FT_BASE));
                        } else {
                            len = 0;
                            LOG_D(LOG_TAG, "[%s] malloc memory failed for local area sample", __func__);
                            break;
                        }

                        notifyTestCmd(0, CMD_TEST_SZ_LOCAL_AREA_SAMPLE, test_result, len);
                    } while (0);

                    if (test_result != NULL) {
                        delete []test_result;
                    }

                    break;
                }

                case CMD_TEST_SZ_FIND_SENSOR: {
                    err = findSensor();
                    break;
                }

                case CMD_TEST_SZ_SET_GROUP_ID: {
                    err = setActiveGroup(in, inLen);
                    break;
                }

                case CMD_TEST_SZ_UNTRUSTED_ENROLL: {
                    gf_sz_untrust_enroll_t cmd = {{ 0 }};
                    const int8_t *in_buf = in;
                    uint32_t token = 0;
                    uint32_t enroll_user_env_len = 0;
                    mCurrentCmd = CMD_TEST_SZ_UNTRUSTED_ENROLL;
                    cmd.cmd_header.cmd_id = GF_SZ_CMD_UNTRUST_ENROLL_ENABLE;
                    cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
                    cmd.i_enable = 1;
                    err = invokeCommand(&cmd, sizeof(gf_sz_untrust_enroll_t));
                    mWorkState = STATE_ENROLL;
#ifdef SUPPORT_DUMP
                    SZHalDump::sz_user_env_enable = 0;
                    memset(SZHalDump::sz_user_env, 0, sizeof(SZHalDump::sz_user_env));
#endif  // SUPPORT_DUMP

                    if (NULL != in) {
                        do {
                            in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                            switch (token) {
                                case TEST_TOKEN_METADATA: {
                                    in_buf = TestUtils::testDecodeUint32(&enroll_user_env_len, in_buf);

                                    if ((enroll_user_env_len > 0) && (enroll_user_env_len < GF_SZ_USER_ENV)) {
#ifdef SUPPORT_DUMP
                                        SZHalDump::sz_user_env_enable = 1;
                                        memcpy(SZHalDump::sz_user_env, in_buf, enroll_user_env_len);
#endif  // SUPPORT_DUMP
                                    }

                                    break;
                                }

                                default:
                                    break;
                            }
                        } while (in_buf < in + inLen);
                    }

                    szFingerprintCore->enroll(NULL, group_id, 600);
                    break;
                }

                case CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE: {
                    const int8_t *in_buf = in;
                    uint32_t token = 0;
                    uint32_t user_env_len = 0;
#ifdef SUPPORT_DUMP
                    SZHalDump::sz_user_env_enable = 0;
                    memset(SZHalDump::sz_user_env, 0, sizeof(SZHalDump::sz_user_env));
#endif  // SUPPORT_DUMP

                    if (NULL != in) {
                        do {
                            in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                            switch (token) {
                                case TEST_TOKEN_METADATA: {
                                    in_buf = TestUtils::testDecodeUint32(&user_env_len, in_buf);

                                    if ((user_env_len > 0) && (user_env_len < GF_SZ_USER_ENV)) {
#ifdef SUPPORT_DUMP
                                        SZHalDump::sz_user_env_enable = 1;
                                        memcpy(SZHalDump::sz_user_env, in_buf, user_env_len);
#endif  // SUPPORT_DUMP
                                    }

                                    break;
                                }

                                default:
                                    break;
                            }
                        } while (in_buf < in + inLen);
                    }

                    mWorkState = STATE_AUTHENTICATE;
                    mCurrentCmd = CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE;
                    szFingerprintCore->authenticate(0, group_id);
                    break;
                }

                case CMD_TEST_SZ_UNTRUSTED_ENUMERATE: {
                    // set max as 10 for now
                    uint32_t max_size = 10;
                    gf_fingerprint_finger_id_t *result = NULL;
                    err = szFingerprintCore->setActiveGroup(group_id);

                    if (GF_SUCCESS == err) {
                        result = new gf_fingerprint_finger_id_t[max_size] { { 0 } };
                        err = szFingerprintCore->enumerate((void *)result, &max_size);

                        if ((GF_SUCCESS == err) && (max_size > 0)) {
                            notifyEnumerate(CMD_TEST_SZ_UNTRUSTED_ENUMERATE, result, max_size);
                        }
                    }

                    if (result != NULL) {
                        delete []result;
                    }

                    break;
                }

                case CMD_TEST_SZ_DELETE_UNTRUSTED_ENROLLED_FINGER: {
                    const int8_t *in_buf = in;
                    uint32_t finger_id = 0;
                    uint32_t token = 0;

                    do {
                        in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                        switch (token) {
                            case TEST_TOKEN_FINGERPRINT_FID: {
                                in_buf = TestUtils::testDecodeUint32(&finger_id, in_buf);
                                break;
                            }

                            default:
                                break;
                        }
                    } while (in_buf < in + inLen);

                    LOG_D(LOG_TAG, "[%s] remove fid %d", __func__, finger_id);
                    szFingerprintCore->remove(group_id, finger_id);
                    break;
                }

                case CMD_TEST_SZ_UPDATE_CAPTURE_PARM: {
                    err = setCaptureParam(in, inLen);
                    break;
                }

                case CMD_TEST_SZ_K_B_CALIBRATION: {
                    err = testKbCalibration(in, inLen);
                    break;
                }

                case CMD_TEST_SZ_GET_VERSION: {
                    err = getVersion();
                    break;
                }

                case CMD_TEST_SZ_FUSION_PREVIEW: {
                    mWorkState = FUSION_PREVIEW;
                    mContext->mCenter->registerHandler(this);
                    break;
                }

                case CMD_TEST_SZ_RAWDATA_PREVIEW: {
                    mWorkState = RAWDATA_PREVIEW;
                    mContext->mCenter->registerHandler(this);
                    break;
                }

                case CMD_TEST_SZ_LDC_CALIBRATE: {
                    break;
                }

                case CMD_TEST_SZ_ENROLL_TEMPLATE_COUNT: {
                    err = setEnrollTemplateCount(in, inLen);
                    break;
                }

                case CMD_TEST_SZ_CANCEL: {
                    if (STATE_ENROLL == mWorkState) {
                        gf_sz_untrust_enroll_t cmd = {{ 0 }};
                        cmd.cmd_header.cmd_id = GF_SZ_CMD_UNTRUST_ENROLL_ENABLE;
                        cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
                        cmd.i_enable = 0;
                        err = invokeCommand(&cmd, sizeof(gf_sz_untrust_enroll_t));
                    }

#ifdef SUPPORT_DUMP
                    SZHalDump::sz_user_env_enable = 0;
#endif  // SUPPORT_DUMP
                    szFingerprintCore->cancel();
                    mWorkState = STATE_INIT;
                    break;
                }

                case CMD_TEST_SZ_GET_CONFIG: {
                    err = getConfig(out, outLen);
                    mContext->mSensor->sleepSensor();
                    break;
                }

                case CMD_TEST_SZ_FT_INIT: {
                    err = factoryInit(in, inLen);
                    break;
                }

                case CMD_TEST_SZ_FT_EXIT: {
                    err = factoryExit();
                    mContext->mSensor->sleepSensor();
                    break;
                }

                case CMD_TEST_SZ_DUMP_TEMPLATE: {
#ifdef SUPPORT_DUMP
                    SZHalDump *szdump = static_cast<SZHalDump *>(mContext->mHalDump);
                    err = szdump->szDumpTemplates();
#endif  // SUPPORT_DUMP
                    break;
                }

                case CMD_TEST_SZ_UPDATE_CFG: {
                    gf_sz_test_update_cfg_t *cmd = NULL;
                    uint32_t size = sizeof(gf_sz_test_update_cfg_t);
                    const int8_t *in_buf = in;
                    uint32_t token = 0;
                    uint32_t update_mode = 0;
                    uint32_t cfg_file_len = 0;

                    do {
                        cmd = (gf_sz_test_update_cfg_t *)malloc(size);

                        if (NULL == cmd) {
                            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                            err = GF_ERROR_OUT_OF_MEMORY;
                            break;
                        }

                        memset(cmd, 0, size);
                        cmd->cmd_header.cmd_id = GF_SZ_CMD_TEST_UPDATE_CFG;
                        cmd->cmd_header.target = GF_TARGET_PRODUCT_TEST;
                        // decode update_mode
                        in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                        if (TEST_TOKEN_CFG_UPDATE_MODE != token) {
                            break;
                        }

                        in_buf = TestUtils::testDecodeUint32(&update_mode, in_buf);
                        LOG_D(LOG_TAG, "[%s] update_mode = 0x%02x", __func__, update_mode);
                        cmd->update_mode = update_mode;
                        // decode cfg_file_len and cfg_file_data
                        in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                        if (TEST_TOKEN_CFG_DATA != token) {
                            break;
                        }

                        in_buf = TestUtils::testDecodeUint32(&cfg_file_len, in_buf);
                        LOG_D(LOG_TAG, "[%s] cfg_file_len = %d", __func__, cfg_file_len);
                        cmd->cfg_file_len = cfg_file_len;
                        memcpy(cmd->cfg_file_data, in_buf, cmd->cfg_file_len);
                        err = invokeCommand(cmd, sizeof(gf_sz_test_update_cfg_t));

                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "[%s] update cfg invoke command failed ", __func__);
                            break;
                        }
                    } while (0);

                    if (cmd != NULL) {
                        free(cmd);
                    }

                    break;
                }

                case CMD_TEST_SZ_UPDATE_FW: {
                    gf_sz_test_update_fw_t *cmd = NULL;
                    const int8_t *in_buf = in;
                    uint32_t token = 0;
                    uint32_t fw_file_len = 0;
                    uint32_t size = sizeof(gf_sz_test_update_fw_t);

                    do {
                        cmd = (gf_sz_test_update_fw_t *)malloc(size);

                        if (NULL == cmd) {
                            LOG_E(LOG_TAG, "[%s] out of memory, cmd", __func__);
                            err = GF_ERROR_OUT_OF_MEMORY;
                            break;
                        }

                        memset(cmd, 0, size);
                        cmd->cmd_header.cmd_id = GF_SZ_CMD_TEST_UPDATE_FW;
                        cmd->cmd_header.target = GF_TARGET_PRODUCT_TEST;
                        // decode fw_file_len and fw_file_data
                        in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                        if (TEST_TOKEN_FW_DATA != token) {
                            break;
                        }

                        in_buf = TestUtils::testDecodeUint32(&fw_file_len, in_buf);
                        LOG_D(LOG_TAG, "[%s] fw_file_len = %d", __func__, fw_file_len);
                        cmd->fw_file_len = fw_file_len;
                        memcpy(cmd->fw_file_data, in_buf, cmd->fw_file_len);
                        err = invokeCommand(cmd, sizeof(gf_sz_test_update_fw_t));

                        if (GF_SUCCESS != err) {
                            LOG_E(LOG_TAG, "[%s] update fw invoke command failed ", __func__);
                            break;
                        }
                    } while (0);

                    if (cmd != NULL) {
                        free(cmd);
                    }

                    break;
                }

                case CMD_TEST_SZ_SET_DUMP_ENABLE_FLAG: {
#ifdef SUPPORT_DUMP
                    const int8_t *in_buf = in;
                    uint32_t token = 0;
                    uint32_t enable = 0;

                    if (NULL != in) {
                        do {
                            in_buf = TestUtils::testDecodeUint32(&token, in_buf);

                            switch (token) {
                                case TEST_TOKEN_METADATA: {
                                    in_buf = TestUtils::testDecodeUint32(&enable, in_buf);
                                    LOG_D(LOG_TAG, "[%s] enable = %d", __func__, enable);

                                    if (enable > 0) {
                                        property_set(PROPERTY_DUMP_DATA, "1");
                                    } else {
                                        property_set(PROPERTY_DUMP_DATA, "0");
                                    }

                                    break;
                                }

                                default:
                                    break;
                            }
                        } while (in_buf < in + inLen);
                    }

#endif  // SUPPORT_DUMP
                    break;
                }

                default: {
#ifdef SUPPORT_FRR_FAR
                    szFrrFarTest->setNotify(mpNotify);
                    err = szFrrFarTest->executeCommand(cmdId, in, inLen, out, outLen);
#endif  // END SUPPORT_FRR_FAR
                    break;
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::onMessage(const MsgBus::Message &msg) {
        gf_error_t err = GF_SUCCESS;
        int8_t *test_result = NULL;
        uint32_t len = 0;
        gf_sz_config_t * config = (gf_sz_config_t *) mContext->mConfig;
        FUNC_ENTER();

        do {
            switch (msg.msg) {
                case MsgBus::MSG_ENROLL_END: {
                    gf_sz_bmp_data_t bmp_cmd = { {0} };

                    if (CMD_TEST_SZ_UNTRUSTED_ENROLL != szTest->mCurrentCmd &&
                                CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE != szTest->mCurrentCmd) {
                        break;
                    }

                    memset(&bmp_cmd, 0, sizeof(gf_sz_bmp_data_t));
                    bmp_cmd.cmd_header.cmd_id = GF_SZ_CMD_TEST_GET_BMD_DATA;
                    bmp_cmd.cmd_header.target = GF_TARGET_PRODUCT_TEST;
                    err = invokeCommand(&bmp_cmd, sizeof(gf_sz_bmp_data_t));

                    if (GF_SUCCESS != err) {
                        break;
                    }

                    uint32_t array_len = bmp_cmd.height * bmp_cmd.width;
                    // message type
                    len += HAL_TEST_SIZEOF_INT32;
                    // bmp_data
                    len += HAL_TEST_SIZEOF_ARRAY(array_len);
                    // bmp_data width
                    len += HAL_TEST_SIZEOF_INT32;
                    // bmp_data height
                    len += HAL_TEST_SIZEOF_INT32;
                    LOG_D(LOG_TAG, "[%s] bmp width %d", __func__, bmp_cmd.width);
                    LOG_D(LOG_TAG, "[%s] bmp height %d", __func__, bmp_cmd.height);
                    LOG_D(LOG_TAG,
                          "[%s] row %d, col %d, image quality %d, valid area %d, frame index %d",
                          __func__,
                          bmp_cmd.row, bmp_cmd.col, bmp_cmd.image_quality, bmp_cmd.valid_area,
                          bmp_cmd.frame_index);
                    LOG_D(LOG_TAG,
                          "[%s] weak row %d, weak col %d, crop height %d, crop width %d, crop x %d, crop y %d",
                          __func__,
                          bmp_cmd.weak_data_row, bmp_cmd.weak_data_col, bmp_cmd.image_crop_rect.height,
                          bmp_cmd.image_crop_rect.width,
                          bmp_cmd.image_crop_rect.x, bmp_cmd.image_crop_rect.y);
                    test_result = new int8_t[len] { 0 };

                    if (test_result != NULL) {
                        int8_t *current = test_result;

                        if (CMD_TEST_SZ_UNTRUSTED_ENROLL == szTest->mCurrentCmd) {
                            current = TestUtils::testEncodeInt32(current,
                                                                 TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_TEMPLATE_ENROLLING);
                        } else if (CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE == szTest->mCurrentCmd) {
                            current = TestUtils::testEncodeInt32(current,
                                                                 TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_AUTHENTICATED);
                        }
                        //  memset 0 bmp_data, test apk will not show bmp
                        if (!config->show_enroll_and_authenticate_bmp_in_apk) {
                            memset((int8_t *) bmp_cmd.bmp_data, 0, sizeof(int8_t) * array_len);
                        }
                        current = TestUtils::testEncodeArray(current, TEST_TOKEN_BMP_DATA,
                                                             (int8_t *) bmp_cmd.bmp_data, sizeof(int8_t) * array_len);
                        current = TestUtils::testEncodeInt32(current, TEST_TOKEN_BMP_DATA_WIDTH,
                                                             bmp_cmd.width);
                        current = TestUtils::testEncodeInt32(current, TEST_TOKEN_BMP_DATA_HEIGHT,
                                                             bmp_cmd.height);
                        TestUtils::testMemoryCheck(__func__, test_result, current, len);

                        if (CMD_TEST_SZ_UNTRUSTED_ENROLL == szTest->mCurrentCmd) {
                            szTest->mpNotify(0, GF_FINGERPRINT_TEMPLATE_ENROLLING, szTest->mCurrentCmd,
                                             test_result, len);
                        } else if (CMD_TEST_SZ_UNTRUSTED_AUTHENTICATE == szTest->mCurrentCmd) {
                            szTest->mpNotify(0, GF_FINGERPRINT_AUTHENTICATED, szTest->mCurrentCmd,
                                             test_result, len);
                        }
                    }

                    break;
                }

                case MsgBus::MSG_WAIT_FOR_FINGER_INPUT: {
                    break;
                }

                case MsgBus::MSG_CANCELED: {
                    break;
                }

                default: {
                    break;
                }
            }
        } while (0);

        if (test_result != NULL) {
            delete []test_result;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t SZProductTest::onEvent(gf_event_type_t e) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] sz test get event: %d", __func__, e);

        do {
            switch (e) {
                case EVENT_FINGER_DOWN: {
                    err = mContext->mSensor->captureImage(GF_OPERATION_OTHER, 0);

                    if (GF_SUCCESS == err) {
                        switch (mWorkState) {
                            case FUSION_PREVIEW: {
                                err = testFusion();
                                if (GF_SUCCESS != err) {
                                    LOG_E(LOG_TAG, "[%s] test Fusion Preview fail ", __func__);
                                }
                                break;
                            }

                            case RAWDATA_PREVIEW: {
                                err = testRawdataPreview();
                                if (GF_SUCCESS != err) {
                                    LOG_E(LOG_TAG, "[%s] test Rawdata Preview fail ", __func__);
                                }
                                break;
                            }

                            default:
                                break;
                        }
                    }

                    break;
                }

                default:
                    break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::szNotifyCallback(const gf_fingerprint_msg_t *msg) {
        int8_t *test_result = NULL;
        uint32_t len = 0;
        VOID_FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] msg type %d", __func__, msg->type);

        switch (msg->type) {
            case GF_FINGERPRINT_ERROR: {
                LOG_D(LOG_TAG, "[%s] onError(%d)", __func__, msg->data.error);
                // message type
                len += HAL_TEST_SIZEOF_INT32;
                // error_code
                len += HAL_TEST_SIZEOF_INT32;
                test_result = new int8_t[len] { 0 };

                if (test_result != NULL) {
                    int8_t *current = test_result;
                    current = TestUtils::testEncodeInt32(current,
                                                         TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_ERROR);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_ERROR_CODE,
                                                         msg->data.error);
                    TestUtils::testMemoryCheck(__func__, test_result, current, len);
                }

                szTest->mpNotify(0, GF_FINGERPRINT_ERROR, szTest->mCurrentCmd, test_result,
                                 len);
                break;
            }

            case GF_FINGERPRINT_ACQUIRED: {
                LOG_D(LOG_TAG, "[%s] onAcquired(%d)", __func__,
                      msg->data.acquired.acquired_info);
                // message type
                len += HAL_TEST_SIZEOF_INT32;
                // acquired info
                len += HAL_TEST_SIZEOF_INT32;
                test_result = new int8_t[len] { 0 };

                if (test_result != NULL) {
                    int8_t *current = test_result;
                    current = TestUtils::testEncodeInt32(current,
                                                         TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_ACQUIRED);
                    current = TestUtils::testEncodeInt32(current,
                                                         TEST_TOKEN_FINGERPRINT_ACQUIRED_INFO, msg->data.acquired.acquired_info);
                    TestUtils::testMemoryCheck(__func__, test_result, current, len);
                }

                szTest->mpNotify(0, GF_FINGERPRINT_ACQUIRED, szTest->mCurrentCmd, test_result,
                                 len);
                break;
            }

            case GF_FINGERPRINT_AUTHENTICATED: {
                LOG_D(LOG_TAG, "[%s] onAuthenticated(fid=%d, gid=%d)",
                      __func__,
                      msg->data.authenticated.finger.fid,
                      msg->data.authenticated.finger.gid);
                // message type
                len += HAL_TEST_SIZEOF_INT32;
                // fid
                len += HAL_TEST_SIZEOF_INT32;
                // gid
                len += HAL_TEST_SIZEOF_INT32;
                test_result = new int8_t[len] { 0 };

                if (test_result != NULL) {
                    int8_t *current = test_result;
                    current = TestUtils::testEncodeInt32(current,
                                                         TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_AUTHENTICATED);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_FID,
                                                         msg->data.authenticated.finger.fid);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_GID,
                                                         msg->data.authenticated.finger.gid);
                    TestUtils::testMemoryCheck(__func__, test_result, current, len);
                }

                szTest->mpNotify(0, GF_FINGERPRINT_AUTHENTICATED, szTest->mCurrentCmd,
                                 test_result, len);
                break;
            }

            case GF_FINGERPRINT_TEMPLATE_ENROLLING: {
                LOG_D(LOG_TAG, "[%s] onEnrollResult(fid=%d, gid=%d, rem=%d)",
                      __func__,
                      msg->data.enroll.finger.fid,
                      msg->data.enroll.finger.gid,
                      msg->data.enroll.samples_remaining);
                // message type
                len += HAL_TEST_SIZEOF_INT32;
                // fid
                len += HAL_TEST_SIZEOF_INT32;
                // gid
                len += HAL_TEST_SIZEOF_INT32;
                // progress
                len += HAL_TEST_SIZEOF_INT32;
                test_result = new int8_t[len] { 0 };

                if (test_result != NULL) {
                    int8_t *current = test_result;
                    current = TestUtils::testEncodeInt32(current,
                                                         TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_TEMPLATE_ENROLLING);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_FID,
                                                         msg->data.enroll.finger.fid);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_GID,
                                                         msg->data.enroll.finger.gid);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_PROGRESS,
                                                         msg->data.enroll.samples_remaining);
                    TestUtils::testMemoryCheck(__func__, test_result, current, len);
                }

                szTest->mpNotify(0, GF_FINGERPRINT_TEMPLATE_ENROLLING, szTest->mCurrentCmd,
                                 test_result, len);
                break;
            }

            case GF_FINGERPRINT_TEMPLATE_REMOVED: {
                LOG_D(LOG_TAG, "[%s] onRemove(fid=%d, gid=%d)",
                      __func__,
                      msg->data.removed.finger.fid,
                      msg->data.removed.finger.gid);
                // message type
                len += HAL_TEST_SIZEOF_INT32;
                // fid
                len += HAL_TEST_SIZEOF_INT32;
                // gid
                len += HAL_TEST_SIZEOF_INT32;
                test_result = new int8_t[len] { 0 };

                if (test_result != NULL) {
                    int8_t *current = test_result;
                    current = TestUtils::testEncodeInt32(current,
                                                         TEST_TOKEN_FINGERPRINT_MESSAGE_TYPE, GF_FINGERPRINT_TEMPLATE_REMOVED);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_FID,
                                                         msg->data.removed.finger.fid);
                    current = TestUtils::testEncodeInt32(current, TEST_TOKEN_FINGERPRINT_GID,
                                                         msg->data.removed.finger.gid);
                    TestUtils::testMemoryCheck(__func__, test_result, current, len);
                }

                szTest->mpNotify(0, GF_FINGERPRINT_TEMPLATE_REMOVED, szTest->mCurrentCmd,
                                 test_result, len);
                break;
            }

            default: {
                LOG_E(LOG_TAG, "[%s] invalid msg type: %d", __func__, msg->type);
                break;
            }
        }

        if (test_result != NULL) {
            delete []test_result;
        }

        VOID_FUNC_EXIT();
    }

    void SZProductTest::handleEnrollAuthEndMessage() {
        VOID_FUNC_ENTER();
        VOID_FUNC_EXIT();
        return;
    }

    gf_error_t SZProductTest::retrieveImage(gf_sz_bmp_data_t &bmpData) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(bmpData);
        FUNC_EXIT(err);
        return err;
    }

    void SZProductTest::notifySensorDisplayControl(int64_t devId, int32_t cmdId,
                                                   const int8_t *sensorData, int32_t dataLen) {
        UNUSED_VAR(devId);
        UNUSED_VAR(cmdId);
        UNUSED_VAR(sensorData);
        UNUSED_VAR(dataLen);
    }

    void SZProductTest::notifyPreviewDisplayControl(int64_t devId, int32_t cmdId,
                                                    const gf_sz_bmp_data_t *previewData) {
        VOID_FUNC_ENTER();
        UNUSED_VAR(devId);
        UNUSED_VAR(cmdId);
        UNUSED_VAR(previewData);
        VOID_FUNC_EXIT();
    }

    bool SZProductTest::isNeedLock(int32_t cmdId) {
        bool ret = true;

        switch (cmdId) {
            case CMD_TEST_SZ_FINGER_DOWN:
            case CMD_TEST_SZ_FINGER_UP: {
                ret = false;
                break;
            }

            default : {
                break;
            }
        }

        return ret;
    }

    const char *SZProductTest::toString(int32_t cmdID) {
        uint32_t idx = 0;
        uint32_t len = sizeof(szProductTestCmdTable) / sizeof(sz_product_test_cmd_table_t);

        for (idx = 0; idx < len; idx++) {
            if (cmdID == szProductTestCmdTable[idx].cmdID) {
                return szProductTestCmdTable[idx].cmdIDString;
            }
        }
#ifdef SUPPORT_FRR_FAR
        return szFrrFarTest->toString(cmdID);
#else  // SUPPORT
        return "NULL";
#endif  // END
    }

    gf_error_t SZProductTest::setEngNotify(fingerprint_eng_notify_t notify)
    {
        gf_error_t err = GF_SUCCESS;
        VOID_FUNC_ENTER();
        UNUSED_VAR(notify);
        VOID_FUNC_EXIT();
        return err;
    }

}  // namespace goodix
