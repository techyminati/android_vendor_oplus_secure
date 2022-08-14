/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _SZHALDUMP_H_
#define _SZHALDUMP_H_

#include "HalDump.h"
#include "gf_sz_types.h"
#include "gf_sz_product_test_types.h"

#define GF_DUMP_CALI_INFO_PATH "/gf_data/CaliParam/"
#define GF_DUMP_BMP_SMPL_DIR "/gf_data/Bmp/Samples"
#define GF_DUMP_BMP_TMPL_DIR "/gf_data/Bmp/Templates/"
#define GF_DUMP_DATA_SMPL_DIR "/gf_data/RawData/Samples"
#define GF_DUMP_DATA_TMPL_DIR "/gf_data/RawData/Templates/"
#define GF_DUMP_BASE_SMPL_DIR "/gf_data/Base/Samples"
#define GF_DUMP_BASE_TMPL_DIR "/gf_data/Base/Templates/"
#define GF_DEFAULT_FINGER_INFO "0000"

#define GF_DUMP_BMP_DIR "/gf_data/Bmp/"
#define GF_DUMP_EXTRA_DIR "/gf_data/Extra/"
#define GF_DUMP_DATA_DIR "/gf_data/Data/"
#define GF_DUMP_DSP_DIR "/gf_data/Dsp/"
#define GF_SZ_DUMP_PREVIEW_DIR "/gf_data/Preview/"

namespace goodix {
    class SZHalDump: public HalDump {
    public:
        enum {
            DUMP_L_DARK,
            DUMP_H_DARK,
            DUMP_DARK_BASE,
            DUMP_H_FLESH,
            DUMP_L_FLESH,
            DUMP_CALIBRATE_CHART,
            DUMP_CHART,
            DUMP_KBCali,
            DUMP_PERFORMANCECali,
            DUMP_AUTO_EXPO_START,
            DUMP_AUTO_EXPO_RESULT,
            DUMP_SIMPLE_FINGER,
            DUMP_SIMPLE_PERFORMANCE_DATA,
            DUMP_FUSION_PREVIEW_DATA,
            DUMP_RAWDATA_PREVIEW_DATA,
            DUMP_MAX
        };
        enum {
            DUMP_TYPE_BMP = 0,
            DUMP_TYPE_CSV = 1,
            DUMP_TYPE_TXT = 2,
        };
        explicit SZHalDump(HalContext *context);
        // '/' should not be followed in dumpDir tail. such as '/data' not '/data/'
        explicit SZHalDump(HalContext *context, const char* dumpDir);
        explicit SZHalDump(HalContext *context, const char* dumpDir, const char* dumpControlProperty);
        gf_error_t szDumpTemplates(void);
        gf_error_t szDumpFactory(uint8_t *in_buf, uint32_t in_size, uint32_t frame_type,
                                 char *timestamp,
                                 uint32_t timestamp_len, uint8_t *ft_time, gf_error_t result);
        gf_error_t szDumpAutoExposure(uint8_t *in_buf, uint32_t in_size,  // NOLINT(575)
                                      uint32_t frameType, char *timestamp, uint32_t timestamp_len,
                                      gf_error_t result, uint32_t value = 0);
        gf_error_t szDumpSettingPath(uint8_t *sensorUid);
        void setFarrDumpFixedStr(char *timeStamp, char *fingerId, uint32_t userEnv);
        gf_error_t szDumpPreview(uint8_t *in_buf, uint32_t in_size,
                                 uint32_t frame_type, char *timestamp,
                                 uint32_t timestamp_len, uint8_t *ft_time, gf_error_t result);
        gf_error_t szDumpScreenVendor(char *buf);
        uint8_t sz_ft_time[GF_SZ_FT_TIME] = {0};
        gf_sz_dump_hal_data_t szHalData;
        static char sz_user_env[GF_SZ_USER_ENV];
        static char sz_user_str[GF_SZ_USER_ENV];
        static char sz_env_str[GF_SZ_USER_ENV];
        static uint32_t sz_user_env_enable;
        static char KBCaliPath[GF_DUMP_FILE_PATH_MAX_LEN];
        static bool sz_dump_cali_flag;
        static bool sz_dump_update_cali_dir;
        char sz_screen_vendor[GF_SZ_SCREEN_VENDOR] = "unKnown";

    protected:
        virtual gf_error_t init();
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        virtual gf_error_t doWork(AsyncMessage *message);
        virtual DumpConfig *getDumpConfig();
        virtual gf_error_t onDeviceInitEnd(const MsgBus::Message &msg);
        virtual gf_error_t dumpDeviceInfo(char *timestamp, uint8_t *data,
                                          uint32_t data_len);
        virtual gf_error_t dumpEnrollData(char *timestamp, gf_error_t result,
                                          uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpAuthData(char *timestamp, gf_error_t result,
                                        uint32_t retry, uint8_t *data, uint32_t data_len);
        virtual gf_error_t get_user_env_str();
        gf_error_t genResultStrForFileName(char *resultStr, uint32_t dumpOp,
                                           gf_error_t result, uint32_t retry);
        gf_error_t onHardwareReset(const MsgBus::Message &msg);
        gf_error_t onCaptureImageEnd(const MsgBus::Message &msg);
        gf_error_t onEnrollStart(const MsgBus::Message &msg);
        gf_error_t onAuthenticateStart(const MsgBus::Message &msg);
        uint32_t pow(uint32_t x, uint32_t n);
        gf_error_t dumpCalibrationInfo(char *timestamp, char *root_path, uint8_t *data,
                                       uint32_t data_len, uint32_t full, gf_dump_op_t op, char *str);
        gf_error_t dumpSensorInfo(char *timestamp, char *root_path, uint8_t *data,
                                  uint32_t data_len);
        gf_error_t dumpConfigInfo(char *timestamp, char *root_path, uint8_t *data,
                                  uint32_t data_len);
        gf_error_t dumpFirmwareInfo(char *timestamp, char *root_path, uint8_t *data,
                                    uint32_t data_len);
        gf_error_t dumpAlgoInfo(char *timestamp, char *root_path, uint8_t *data,
                                uint32_t data_len);

    private:
        // TODO add functions
        // virtual gf_error_t dumpDevice(char* timestamp, uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpBase(char *timestamp, uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpTemplate(char *timestamp, int32_t operation,
                                        uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpRawData(char *timestamp, int32_t operation,
                                       uint8_t *data, uint32_t data_len);
        virtual gf_error_t dumpBmpData(char *timestamp, int32_t operation,
                                       uint8_t *data, uint32_t data_len);
        gf_error_t dumpCalibration(char *timestamp, char *root_path, uint8_t *data,
                                   uint32_t data_len, uint32_t full, uint32_t op);
        gf_error_t dumpDeviceJsonInfo(char *timestamp, char *root_path, uint8_t *data,
                                      uint32_t data_len);
        gf_error_t dumpEnrollAuthJsonInfo(char *timestamp, char *root_path,
                                          uint8_t *data, uint32_t data_len, uint32_t type);
        gf_error_t doDumpTemplates(char *timestamp, uint8_t *data, uint32_t data_len);
        gf_error_t doDumpFactory(char *timestamp, uint8_t *data, uint32_t data_len,
                                 uint32_t frame_type, int32_t result);
        gf_error_t doDumpAutoExposure(char *timestamp, uint8_t *data, int32_t result,
                                      uint32_t frameType, uint32_t value);
        gf_error_t dump_base_cali_param(char *file, uint8_t *data);
        gf_error_t dump_authenroll_base_param(char *file, uint8_t *data);
        gf_error_t dumpAutoExpoResult(char *file, uint8_t *data);
        gf_error_t doDumpFactoryData(char *filepath, uint8_t *data, uint32_t data_len,
                                     uint8_t byte_size,
                                     uint8_t type, uint32_t width, uint32_t height);
        virtual gf_error_t dumpCalibrateResult(char *file, uint8_t *data);
        virtual gf_error_t dumpPerformanceResult(char *file, uint8_t *data);
        gf_error_t dumpFactoryLog(uint8_t *data, char *file);
        void dumpInitVar();
        gf_error_t doDumpPreview(char *timestamp, uint8_t *data,
                                 uint32_t data_len, uint32_t frame_type, int32_t result);
        char dumpFarrFixedTimeStamp[GF_DUMP_FILE_PATH_MAX_LEN];
        char dumpFarrFixedFingerId[GF_DUMP_FILE_PATH_MAX_LEN];
    };
}  // namespace goodix

#endif /* _SZHALDUMP_H_ */
