/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CUSTOMIZEDDELMARHALDUMP_H_
#define _CUSTOMIZEDDELMARHALDUMP_H_

#include "DelmarHalDump.h"

#define GF_MAX_CONFIG_ITEM_LEN 64
#define GF_MAX_CONFIG_SIZE 64

namespace goodix {
    class CustomizedDelmarHalDump: public DelmarHalDump {
    public:
        explicit CustomizedDelmarHalDump(HalContext *context);
        // '/' should not be followed in dumpDir tail. such as '/data' not '/data/'
        explicit CustomizedDelmarHalDump(HalContext *context, const char* dumpDir, const char* dumpControlProperty);
        gf_error_t dumpImageTestData(gf_error_t result, uint8_t *data, uint32_t data_len);
        gf_error_t doDumpCrcOriginData(uint16_t* data);
        uint32_t dsp_get_feature_time = 0;
        uint32_t enhance_dsp_get_feature_time = 0;
        uint32_t lastest_press = 0;
        uint32_t mTotalTime = 0;
        uint64_t mUiReadyTime = 0;
        uint64_t mFingerDownMsgTime = 0;
        int8_t pmTouchDownTimeStamp[TIME_STAMP_LEN] = "2019-07-23-09-40-12-433947";
        gf_error_t mResult = GF_SUCCESS;
        uint32_t mRetry = 0;

    protected:
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        gf_error_t onDspGetFeatureTime(const MsgBus::Message &msg);
        gf_error_t onUiReady(const MsgBus::Message &msg);
        gf_error_t onFingerDown(const MsgBus::Message &msg);
        gf_error_t customDumpDeviceJsonInfo(char *timestamp, char *root_path, uint8_t *data,
            uint32_t data_len);
        gf_error_t customDumpEnrollJsonInfo(char *timestamp, char *root_path,
            uint8_t *data, uint32_t data_len, uint32_t type);
        gf_error_t customDumpAuthJsonInfo(char *timestamp, char *root_path,
            uint8_t *data, uint32_t data_len, uint32_t type);
        virtual gf_error_t dumpEnrollData(char *timestamp, gf_error_t result,
            uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpAuthData(char *timestamp, gf_error_t result,
            uint32_t retry, uint8_t *data, uint32_t data_len);
        gf_error_t dumpDeviceInfo(char *timestamp, uint8_t *data,
                                         uint32_t data_len);

        gf_delmar_algo_kpi_t *kpi = NULL;

    private:
        void dumpReadConfigFIle(void);
        void dumpCalRawDataValue(uint16_t *rawData, uint32_t rawData_len, uint32_t *maxValue, uint32_t *minValue, uint32_t *averValue);
        gf_error_t dumpGetAllTime(gf_delmar_algo_kpi_t *kpi, uint32_t auth_or_enroll, uint32_t *getFeatureTime, uint32_t *auth_enroll_time, uint32_t *totalKpiTime);
        /*get config owned by config file*/
        typedef struct CONFIG_DATA_UNIT {
            uint8_t key[GF_MAX_CONFIG_ITEM_LEN];
            uint8_t value[GF_MAX_CONFIG_ITEM_LEN];
        } configDataUnit;

        struct CONFIG_DATA {
            configDataUnit unitDara[GF_MAX_CONFIG_SIZE];
            uint32_t unitNum = 0;
        } configData;

        uint8_t anti_proofing_version[DELMAR_ALGO_VERSION_INFO_LEN];
        uint8_t algo_version[DELMAR_ALGO_VERSION_INFO_LEN];
    };
}  // namespace goodix

#endif /* _CUSTOMIZEDDELMARHALDUMP_H_ */
