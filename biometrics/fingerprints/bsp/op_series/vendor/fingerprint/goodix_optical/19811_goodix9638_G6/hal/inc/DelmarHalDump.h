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

#define DUMP_UNENCRYPTED_SUB_DIR "gf_data"
#define DUMP_ENCRYPTED_BMP_DIR "/bmp/"
#define DUMP_ENCRYPTED_RAW_DIR "/rawdata/"
#define DUMP_ENCRYPTED_TEST_DIR "/test/"
#define DUMP_ENCRYPTED_CALI_DIR "/cali/"

// the property to control dump bmp+diffimage+blackset or bmp+rawfeature
#ifndef PROPERTY_DUMP_RAW_FEATURE_ENABLED
#define PROPERTY_DUMP_RAW_FEATURE_ENABLED   "gf.debug.dump_raw_feature"
#endif  // PROPERTY_DUMP_RAW_FEATURE_ENABLED

#define DEFAULT_DELMAR_DUMP_TIMESTAMP "19700101-000000-000"

#define GF_NEGATIVE_BREAK(ret) { if ((ret) < 0) {break;} }
#define PRINT_STRING_ARRAY_TO_BUFFER(buf, len, title, format, array, onlyOneItem) \
    do { \
        char *ptr = (buf); \
        int returnVal = 0; \
        uint32_t restLen = (uint32_t) (len); \
        returnVal = snprintf(ptr, restLen, "%s,", (title)); \
        GF_NEGATIVE_BREAK(returnVal); \
        ptr += returnVal; \
        restLen -= returnVal; \
        for (uint32_t x = 0; x < sensorNum; x++) { \
            returnVal = snprintf(ptr, restLen, format ",", (onlyOneItem) ? (array)[0] : (array)[x]); \
            GF_NEGATIVE_BREAK(returnVal); \
            ptr += returnVal; \
            restLen -= returnVal; \
        } \
        GF_NEGATIVE_BREAK(returnVal); \
        returnVal = snprintf(ptr, restLen, "%s", "\n"); \
    } while (0)

#define PRINT_NUMBER_ARRAY_TO_BUFFER(buf, len, title, format, array, onlyOneItem, denominator) \
    do { \
        char *ptr = (buf); \
        int returnVal = 0; \
        uint32_t restLen = (uint32_t) (len); \
        returnVal = snprintf(ptr, restLen, "%s,", (title)); \
        GF_NEGATIVE_BREAK(returnVal); \
        ptr += returnVal; \
        restLen -= returnVal; \
        for (uint32_t x = 0; x < sensorNum; x++) { \
            returnVal = snprintf(ptr, restLen, format ",", ((onlyOneItem) ? (array)[0] : (array)[x]) / (denominator)); \
            GF_NEGATIVE_BREAK(returnVal); \
            ptr += returnVal; \
            restLen -= returnVal; \
        } \
        GF_NEGATIVE_BREAK(returnVal); \
        returnVal = snprintf(ptr, restLen, "%s", "\n"); \
    } while (0)

#define PRINT_NUMBER_ARRAY_TO_BUFFER_THRESHOLD(buf, len, title, format1, array, format2, threshold, onlyOneItem, denominator) \
    do { \
        char *ptr = (buf); \
        int returnVal = 0; \
        uint32_t restLen = (uint32_t) (len); \
        returnVal = snprintf(ptr, restLen, "%s,", (title)); \
        GF_NEGATIVE_BREAK(returnVal); \
        ptr += returnVal; \
        restLen -= returnVal; \
        for (uint32_t x = 0; x < sensorNum; x++) { \
            returnVal = snprintf(ptr, restLen, format1 "(" format2 "),", \
                ((onlyOneItem) ? (array)[0] : (array)[x]) / (denominator), threshold); \
            GF_NEGATIVE_BREAK(returnVal); \
            ptr += returnVal; \
            restLen -= returnVal; \
        } \
        GF_NEGATIVE_BREAK(returnVal); \
        returnVal = snprintf(ptr, restLen, "%s", "\n"); \
    } while (0)

#define APPEND_LINE_TITLE(buf, title) ((buf).appendArray((uint8_t *) (title), strlen((char*) (title))))
#define APPEND_LINE_END(buf) ((buf).appendArray((uint8_t *) "\n", strlen("\n")))
#define APPEND_VALUE_WITH_FORMAT(buf, format, val, arrayCache) \
    do { \
        snprintf((arrayCache), sizeof(arrayCache), (format), (val)); \
        (buf).appendArray((uint8_t *) (arrayCache), strlen((arrayCache))); \
    } while (0)

#define APPEND_STRING_LINE(buf, title, str) \
    do { \
        APPEND_LINE_TITLE((buf), (title)); \
        (buf).appendArray((uint8_t *) (str), strlen((char *) (str))); \
        APPEND_LINE_END(buf); \
    } while (0)

#define APPEND_JSON_ATTR(buf, key, value, format, arrayCache) \
    do { \
        snprintf((arrayCache), sizeof(arrayCache), "    \"%s\": \"" format "\",\n", (key), (value)); \
        (buf).appendArray((uint8_t *) (arrayCache), strlen((arrayCache))); \
    } while (0)

#define APPEND_LAST_JSON_ATTR(buf, key, value, format, arrayCache) \
    do { \
        snprintf((arrayCache), sizeof(arrayCache), "    \"%s\": \"" format "\"\n", (key), (value)); \
        (buf).appendArray((uint8_t *) (arrayCache), strlen((arrayCache))); \
    } while (0)

#define APPEND_JSON_BEGIN(buf) APPEND_LINE_TITLE((buf), "{\n")
#define APPEND_JSON_END(buf) APPEND_LINE_TITLE((buf), "}\n")

#ifdef SUPPORT_CDC_SOCKET  // for sam s8 cdc
#define GF_DELMAR_CDC_STORGE_PATH "/sdcard/goodix"
#endif  // SUPPORT_CDC_SOCKET

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
        SEPERATE_PERFORMANCE_TESTING_TYPE = 1000,
    };

    typedef enum {
        OP_TYPE_ENROLL,
        OP_TYPE_AUTH,
        OP_TYPE_CALI,
        OP_TYPE_PRODUCT,
    } GF_DUMP_OP_TYPE;

    typedef enum {
        CATEGORY_TYPE_BMP,
        CATEGORY_TYPE_RAW,
        CATEGORY_TYPE_CALI,
        CATEGORY_TYPE_TEST,
    } GF_DUMP_CATEGORY_TYPE;

    class DelmarHalDump: public HalDump {
    public:
        explicit DelmarHalDump(HalContext *context);
        explicit DelmarHalDump(HalContext *context, const char *dumpDir, const char* dumpControlProperty = PROPERTY_DUMP_DATA);  // NOLINT(575)
#ifdef SUPPORT_DUMP_DSP_GET_FEATURE
        void dumpDspGetFeatureBuf(uint8_t step, uint8_t* buf, uint32_t part, uint32_t retry);
#endif  // SUPPORT_DUMP_DSP_GET_FEATURE
#ifdef SUPPORT_DEBUG_TOOLS
        virtual gf_error_t dumpTestDataCollect(char *timeStamp,
                                               gf_test_collect_rawdata_pending_info_t *pendingInfo,
                                               gf_delmar_collect_rawdata_t *colCmd);
        virtual gf_error_t dumpTestRGBDataCollect(gf_delmar_data_sampling_piece_t *pieces, uint32_t pieceCount,
                char *timeStamp, uint32_t frameNum, uint32_t width, uint32_t height, uint32_t operationStep,
                int32_t type, uint8_t sensorIdx, char* saveDir);
        virtual gf_error_t dumpSamplingData(gf_delmar_data_sampling_piece_t *pieces, uint32_t pieceCount,
                    char *timeStamp, uint32_t frameNum, uint32_t width, uint32_t height, float expoTime,
                    uint32_t gain, uint32_t brightness, uint32_t samplingObject, uint8_t sensorIdx,
                    uint32_t step, char* saveDir);
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
        gf_error_t dumpScreenVendor(char *buf);

    protected:
        virtual gf_error_t init();
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        virtual gf_error_t onDeviceInitEnd(const MsgBus::Message &msg);
        virtual gf_error_t onEnrollEnd(const MsgBus::Message &msg);
        virtual gf_error_t onAuthenticateEnd(const MsgBus::Message &msg);
        virtual gf_error_t onProductTestCollectFinished(const MsgBus::Message &msg);
        virtual gf_error_t onProductTestCalculateFinished(const MsgBus::Message &msg);
        virtual gf_error_t onProductTestImageQualityFinished(const MsgBus::Message &msg);
        virtual gf_error_t onAlgoDspGetFeatureStepFinished(const MsgBus::Message &msg);
        virtual gf_error_t doDumpAlgoDspGetFeatureData(AsyncMessage *msg);
        virtual gf_error_t dumpDeviceInfo(char *timestamp, uint8_t *data,
                                          uint32_t data_len);
        virtual gf_error_t dumpEnrollData(char *timestamp, gf_error_t result,
                                          uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpAuthData(char *timestamp, gf_error_t result,
                                        uint32_t retry, uint8_t *data,
                                        uint32_t data_len);
        virtual gf_error_t doWork(AsyncMessage *message);
        virtual DumpConfig *getDumpConfig();
        virtual gf_error_t doDumpTestDataCollect(AsyncMessage *msg);
        virtual gf_error_t doDumpFrrfarRawData(AsyncMessage* pMsg);
        virtual gf_error_t doDumpFrrfarFfdData(AsyncMessage* pMsg);
        virtual gf_error_t doDumpFrrfarPreprocData(AsyncMessage* pMsg);
        virtual gf_error_t doDumpFrrfarCaliInfo(AsyncMessage* pMsg);
        virtual gf_error_t doDumpTestRGBDataCollect(AsyncMessage *msg);
        virtual gf_error_t doDumpCali(AsyncMessage* pMsg);
        virtual gf_error_t doDumpAutoCali(AsyncMessage* pMsg);
        virtual gf_error_t doDumpSamplingData(AsyncMessage *msg);
        virtual gf_error_t doDumpProductTestCollectOriginData(AsyncMessage *msg);
        virtual gf_error_t doDumpProductTestCollectRawData(AsyncMessage *msg);
        virtual gf_error_t doDumpProductTestCalculateData(AsyncMessage *msg);
        virtual gf_error_t doDumpProductTestCalculateResult(AsyncMessage *msg);
        virtual gf_error_t doDumpProductTestImageQualityData(AsyncMessage *msg);
        virtual const char* getSubDirByType(GF_DUMP_OP_TYPE operation_type, GF_DUMP_CATEGORY_TYPE category_type);
        virtual gf_error_t doDumpCaliExtraInfo(uint32_t sensor_index,
                char *timestamp, uint8_t* buf, uint32_t buf_len);
        gf_error_t getEnrollDumpDir(char dirPath[GF_DUMP_FILE_PATH_MAX_LEN], uint32_t groupId,
                uint32_t fingerId, char *userFingerNumber, bool isTriggeredBySystemApp);
        gf_error_t getAuthDumpDir(char dirPath[GF_DUMP_FILE_PATH_MAX_LEN], uint32_t groupId,
                uint32_t fingerId, char *userFingerNumber, bool isTriggeredBySystemApp);

    private:
        gf_error_t dumpRealTimeData(char *timestamp, uint8_t *data, uint32_t data_len);
        gf_error_t dumpCalibrationData(char *timestamp, void *data,
                                       uint32_t width, uint32_t height);
        gf_error_t dumpCalibrationAutoParams(uint32_t sensor_index, uint32_t sg_index, char *timestamp,
                                             uint8_t *autocali_param,
                                             uint32_t len);
        gf_error_t dumpBaseInfo(int32_t sensor_index, const char *filepath,
                                void *info);
        gf_error_t doDumpAlgoConfig(const char *filepath);
        gf_error_t doDumpAllConfig(const char *filepath);
        gf_error_t dumpDataToJson(const char *filepath, uint8_t *rawData, uint32_t len);
        gf_error_t dumpCaliData(char *timestamp);
        gf_error_t dumpAutoCaliData(char *timestamp);
        gf_error_t dumpPerformanceTestThreshold(gf_delmar_calculate_cmd_t *cmd, gf_delmar_product_test_config_t *threshold, int32_t cali_model);
        gf_error_t decodeFingerConfigForUntrusted(const MsgBus::Message &msg);
        gf_error_t doDumpProductTestRawData(void *bundle, char *timestamp);
        gf_error_t doDumpProductTestOriginData(void *bundle);
        gf_error_t doDumpPerformanceTestResult(void *bundle);
        gf_error_t doDumpPerformanceTestData(void *bundle);
        gf_error_t dumpNewPerformanceTestThreshold(gf_delmar_calculate_cmd_t *cmd,
            gf_delmar_new_product_test_threshold_t *threshold, void *data);
        gf_error_t doDumpNewPerformanceTestResult(void *bundle);
        gf_error_t doDumpNewPerformanceTestData(void *bundle);
        gf_error_t dumpEnrollSuccessFlag(void);

        bool mDumpCali;
        uint8_t mpStartTimeStr[GF_DUMP_FILE_PATH_MAX_LEN];
        bool mDumpTombStone;
        uint8_t mpUntrustEnrollAuthEnv[GF_DUMP_FILE_PATH_MAX_LEN];  // test scene: normal, high temperature, low temperature, etc
        uint8_t mpUntrustedEnrollAuthFingerName[GF_DUMP_FILE_PATH_MAX_LEN];
        bool mDeviceInfoDumped;
        uint8_t mAuthDumpCount;
        bool mFirstPressOfEnroll;
        char mScreenVendor[PROPERTY_VALUE_MAX];
        char mpAutoCaliDumpTimeForEnroll[TIME_STAMP_LEN];
        char mDumpCaliEncryptFileName[GF_DUMP_FILE_PATH_MAX_LEN];
        char mDumpDeviceEncryptFileName[GF_DUMP_FILE_PATH_MAX_LEN];
        char mCachedEnrollSubDir[GF_DUMP_FILE_PATH_MAX_LEN];
    };
}  // namespace goodix

#endif /* _DELMARHALDUMP_H_ */
