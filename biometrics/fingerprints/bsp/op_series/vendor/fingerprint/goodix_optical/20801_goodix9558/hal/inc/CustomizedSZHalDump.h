/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _CUSTOMIZEDSZHALDUMP_H_
#define _CUSTOMIZEDSZHALDUMP_H_

#include "SZHalDump.h"

#define GF_MAX_CONFIG_ITEM_LEN 64
#define GF_MAX_CONFIG_SIZE 64

namespace goodix {
    class CustomizedSZHalDump: public SZHalDump {
    public:
        explicit CustomizedSZHalDump(HalContext *context);
        // '/' should not be followed in dumpDir tail. such as '/data' not '/data/'
        explicit CustomizedSZHalDump(HalContext *context, const char* dumpDir);
        explicit CustomizedSZHalDump(HalContext *context, const char* dumpDir, const char* dumpControlProperty);
        uint32_t dsp_get_feature_time = 0;
        uint32_t enhance_dsp_get_feature_time = 0;
        uint32_t lastest_press = 0;
        uint32_t mTotalTime = 0;
        uint64_t mUiReadyTime = 0;
        uint64_t mFingerDownMsgTime = 0;
        int8_t pmTouchDownTimeStamp[TIME_STAMP_LEN];

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

    private:
        void dumpReadConfigFIle(void);
        void dumpCalRawDataValue(uint16_t *rawData, uint16_t row , uint16_t col,
            uint32_t *maxValue, uint32_t *minValue, uint32_t *averValue);
        /*get config owned by config file*/
        typedef struct CONFIG_DATA_UNIT {
            uint8_t key[GF_MAX_CONFIG_ITEM_LEN];
            uint8_t value[GF_MAX_CONFIG_ITEM_LEN];
        } configDataUnit;

        struct CONFIG_DATA {
            configDataUnit unitDara[GF_MAX_CONFIG_SIZE];
            uint32_t unitNum = 0;
        } configData;
    };
}  // namespace goodix

#endif /* _CUSTOMIZEDSZHALDUMP_H_ */
