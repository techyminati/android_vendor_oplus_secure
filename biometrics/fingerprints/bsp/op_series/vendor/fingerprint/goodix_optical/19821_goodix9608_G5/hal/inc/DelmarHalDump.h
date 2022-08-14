/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _DELMARHALDUMP_H_
#define _DELMARHALDUMP_H_
#include <cutils/properties.h>

#include "HalDump.h"
#include "gf_delmar_types.h"
#include "gf_delmar_product_test_types.h"
#ifdef SUPPORT_DEBUG_TOOLS
#include "gf_delmar_debug_tools_types.h"
#endif  // SUPPORT_DEBUG_TOOLS
#ifdef SUPPORT_FRR_FAR
#include "DelmarFrrFarToolsType.h"
#endif  // SUPPORT_FRR_FAR

#define GF_DUMP_LOCATION_CIRCLE "/gf_data/location_circle/"
#define GF_DUMP_LOCATION_CIRCLE_BMP "/gf_data/location_circle/circle_bmp/"
#define GF_DUMP_PERFORMANCE_PATH "/gf_data/production_test/"
#define GF_DUMP_KB_CALIBRATION_PATH "/gf_data/kb_calibration_test/"
#define GF_DUMP_NOISE_PATH "/gf_data/noise_test/"

namespace goodix {
    enum {
        DUMP_RAW_DATA_FOR_PERFORMANCE,
        DUMP_RAW_DATA_FOR_KB_CALI,
        DUMP_RAW_DATA_FOR_LOCATION_CIRCLE,
        DUMP_RAW_DATA_FOR_NOISE_TEST,
        DUMP_RAW_DATA_FOR_RGB_COLLECT,
        DUMP_ORIGIN_DATA_FOR_PERFORMANCE,
    };

    enum {
        KB_CALIBRATION_TYPE = 1,
        PRODUCT_TESTING_TYPE,
        CALCULATE_GAIN_TYPE,
        PERFORMANCE_TESTING_TYPE,
        GAIN_AND_CALIBRATION_TYPE,
    };

    class DelmarHalDump: public HalDump {
    public:
        explicit DelmarHalDump(HalContext *context);
        explicit DelmarHalDump(HalContext *context, const char *dumpDir, const char* dumpControlProperty = PROPERTY_DUMP_DATA);  // NOLINT(575)
        virtual gf_error_t dumpRawDataForPerformanceTest(uint16_t *data,
                                                         char *timeStamp,
                                                         uint32_t frameNum, uint32_t width, uint32_t height, uint32_t operationStep,
                                                         int32_t type, uint8_t sensorIdx, uint32_t feature_type);
        virtual gf_error_t dumpLocationCircleBmp(char *timeStamp, uint8_t *picData,
                                                 uint32_t picWidth,
                                                 uint32_t picHeight, uint32_t operationStep, uint8_t sensorIdx);
        virtual gf_error_t dumpPerformanceTestParameters(char *timeStamp, void *cmd,
                                                         uint32_t operationStep);
        virtual gf_error_t dumpPgaGainRecalculateResult(char *timeStamp, void *cmd);
        virtual gf_error_t dumpLbPgaGainRecalculateResult(char *timeStamp, void *cmd);
#ifdef SUPPORT_DUMP_DSP_GET_FEATURE
        void dumpDspGetFeatureBuf(uint8_t step, uint8_t* buf, uint32_t part, uint32_t retry);
#endif  // SUPPORT_DUMP_DSP_GET_FEATURE
#ifdef SUPPORT_DEBUG_TOOLS
        virtual gf_error_t dumpTestDataCollect(char *timeStamp,
                                               gf_test_collect_rawdata_pending_info_t *pendingInfo,
                                               gf_test_collect_rawdata_t *colCmd);
        virtual gf_error_t dumpTestRGBDataCollect(gf_delmar_bmp_data_t *bmpData, uint16_t *raw_data, char *timeStamp, uint32_t frameNum,
            uint32_t width, uint32_t height, uint32_t operationStep, int32_t type, uint8_t sensorIdx, char* saveDir, uint8_t* originData, uint32_t originCol, uint32_t originRow);
        virtual gf_error_t dumpSamplingData(gf_delmar_bmp_data_t *bmpData,
                                            uint16_t *data,
                                            uint16_t *cf_data,
                                            char *timeStamp,
                                            uint32_t frameNum,
                                            uint32_t width,
                                            uint32_t height,
                                            float expoTime,
                                            uint32_t gain,
                                            uint32_t brightness,
                                            uint32_t samplingObject,
                                            uint8_t sensorIdx,
                                            uint32_t step,
                                            char* saveDir,
                                            uint8_t* originData,
                                            uint32_t originCol,
                                            uint32_t originRow);
#endif  // SUPPORT_DEBUG_TOOLS

#ifdef SUPPORT_FRR_FAR
        virtual gf_error_t dumpFrrfarCaliInfo(uint32_t sensorIndex, uint8_t* pCaliInfo,
            uint32_t caliInfoLen, char* pDirName);
        virtual gf_error_t dumpFrrfarRawData(char *pTimeStamp, gf_frrfar_collect_pending_info_t* pInfo,
            gf_frrfar_cmd_collect_t* pColCmd);
        virtual gf_error_t dumpFrrfarFfdData(uint32_t collectCount, gf_frrfar_dump_data_t* p_dump_data,
            char* pDirName);
        virtual gf_error_t dumpFrrfarPreprocData(uint32_t preprocCount, uint8_t* pDumpData,
            uint32_t dumpLen, char* pDirName);
#endif  // SUPPORT_FRR_FAR
        gf_error_t dumpPermanceTestOriginData(int32_t type, uint8_t* data, uint32_t step,
            char *timeStamp, uint32_t frameNum, uint32_t cont_sampling_num, uint32_t col, uint32_t row);
        gf_error_t dumpScreenVendor(char *buf);
        gf_error_t doDumpCrcOriginData(uint16_t* data);
        gf_error_t dumpImageTestData(gf_error_t result, uint8_t *data, uint32_t data_len);
        void setDumpStamp(char * timeStamp) {
            strncpy((char*) mpStartTimeStr, timeStamp, sizeof(mpStartTimeStr));
        }

    protected:
        virtual gf_error_t init();
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        virtual gf_error_t onDeviceInitEnd(const MsgBus::Message &msg);
        virtual gf_error_t onEnrollEnd(const MsgBus::Message &msg);
        virtual gf_error_t onAuthenticateEnd(const MsgBus::Message &msg);
        virtual gf_error_t dumpDeviceInfo(char *timestamp, uint8_t *data,
                                          uint32_t data_len);
        virtual gf_error_t dumpEnrollData(char *timestamp, gf_error_t result,
                                          uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpAuthData(char *timestamp, gf_error_t result,
                                        uint32_t retry, uint8_t *data,
                                        uint32_t data_len);
        virtual gf_error_t doWork(AsyncMessage *message);
        virtual DumpConfig *getDumpConfig();
        virtual gf_error_t doDumpLocationCircleBmp(AsyncMessage *message);
        virtual gf_error_t doDumpRawDataForPerformanceTest(AsyncMessage *message);
        virtual gf_error_t doDumpPerformanceTestParameters(AsyncMessage *message);
        virtual gf_error_t doDumpPgaGainRecalculateResult(AsyncMessage *message);
        virtual gf_error_t doDumpLbPgaGainRecalculateResult(AsyncMessage *message);
        virtual gf_error_t doDumpTestDataCollect(AsyncMessage *msg);
        virtual gf_error_t doDumpFrrfarRawData(AsyncMessage* pMsg);
        virtual gf_error_t doDumpFrrfarFfdData(AsyncMessage* pMsg);
        virtual gf_error_t doDumpFrrfarPreprocData(AsyncMessage* pMsg);
        virtual gf_error_t doDumpFrrfarCaliInfo(AsyncMessage* pMsg);
        virtual gf_error_t doDumpTestRGBDataCollect(AsyncMessage *msg);
        virtual gf_error_t doDumpCali(AsyncMessage* pMsg);
        virtual gf_error_t doDumpAutoCali(AsyncMessage* pMsg);
        virtual gf_error_t doDumpSamplingData(AsyncMessage *msg);
        gf_error_t getEnrollDumpDir(char dirPath[GF_DUMP_FILE_PATH_MAX_LEN], uint32_t groupId,
                uint32_t fignerId, char *userFingerNumber, bool isTriggeredBySystemApp);
        gf_error_t getAuthDumpDir(char dirPath[GF_DUMP_FILE_PATH_MAX_LEN], uint32_t groupId,
                uint32_t fignerId, char *userFingerNumber, bool isTriggeredBySystemApp);

    private:
        gf_error_t doDumpPermanceTestOriginData(AsyncMessage *msg);
        gf_error_t dumpRealTimeData(char *timestamp, uint8_t *data, uint32_t data_len);
        gf_error_t dumpCalibrationData(char *timestamp, gf_delmar_dump_cali_data *cali_data,
                                       uint32_t width, uint32_t height);
        gf_error_t dumpCalibrationAutoParams(uint32_t sensor_index, char *timestamp,
                                             uint8_t *autocali_param,
                                             uint32_t len);
        gf_error_t dumpBaseInfo(int32_t sensor_index, const char *filepath,
                                gf_delmar_dump_base_info_t *base_info);
        gf_error_t doDumpAlgoConfig(const char *filepath);
        gf_error_t dumpDataToJson(const char *filepath, uint8_t *rawData, uint32_t len);
        gf_error_t dumpCaliData(char *timestamp);
        gf_error_t dumpAutoCaliData(char *timestamp);
        gf_error_t dumpPerformanceTestThreshold(gf_delmar_calculate_cmd_t *cmd, gf_delmar_product_test_config_t *threshold);
        gf_error_t decodeFingerConfigForUntrusted(const MsgBus::Message &msg);
        void updateEnrollAuthOpGroupSeq(int32_t operation);

        bool mDumpCali;
        uint8_t mpStartTimeStr[GF_DUMP_FILE_PATH_MAX_LEN];
        bool mDumpTombStone;
        uint8_t mpUntrustEnrollAuthEnv[GF_DUMP_FILE_PATH_MAX_LEN];  // test scene: normal, high temperature, low temperature, etc
        uint8_t mpUntrustedEnrollAuthFingerName[GF_DUMP_FILE_PATH_MAX_LEN];
        bool mDeviceInfoDumped;
        uint8_t mAuthDumpCount;
        bool mFirstPressOfEnroll;
        int32_t mEnrollAuthOpGroupSeq;
        char mScreenVendor[PROPERTY_VALUE_MAX];
        char mpAutoCaliDumpTimeForEnroll[TIME_STAMP_LEN];
    };
}  // namespace goodix

#endif /* _DELMARHALDUMP_H_ */
