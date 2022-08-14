/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _SZPRODUCTTEST_H_
#define _SZPRODUCTTEST_H_

#include "GoodixFingerprint.h"
#include "ProductTest.h"
#include "SZFingerprintCore.h"
#include "gf_sz_product_test_types.h"
#include "gf_event.h"
#include "EventCenter.h"
#include "HalUtils.h"
#include "fp_eng_test.h"

#ifdef SUPPORT_FRR_FAR
#include "SZFrrFarTest.h"
#endif  // SUPPORT_FRR_FAR

namespace goodix {
    class SZProductTest : public ProductTest, public IEventHandler {
    public:
        char init_timestamp[TIME_STAMP_LEN];
        uint8_t ft_time[GF_SZ_FT_TIME] = {0};
        gf_sz_factory_config_t factory_config;
        explicit SZProductTest(HalContext *context);
        virtual ~SZProductTest();
        virtual gf_error_t onMessage(const MsgBus::Message &msg);
        virtual void notifySensorDisplayControl(int64_t devId, int32_t cmdId,
                                                const int8_t *sensorData, int32_t dataLen);
        virtual void notifyPreviewDisplayControl(int64_t devId, int32_t cmdId,
                                                 const gf_sz_bmp_data_t *previewData);
        void *gfCaptureBaseThread(gf_sz_k_b_calibrate_step_t kbStep);
        gf_error_t sleepSensor(void);
        virtual gf_error_t onEvent(gf_event_type_t e);
        virtual const char *toString(int32_t cmdID);
        virtual gf_error_t localAreaSample(void);
        virtual gf_error_t setInitTimeStamp(void);
        virtual gf_error_t setEngNotify(fingerprint_eng_notify_t notify);
    protected:
        static void szNotifyCallback(const gf_fingerprint_msg_t *msg);
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in,
                                          uint32_t inLen, int8_t **out, uint32_t *outLen);
        virtual gf_error_t cropKbcalidata(uint16_t *kbcalidata, uint32_t data_width,
                                          uint32_t data_height,
                                          gf_sz_image_crop_rect_t *cropRect, uint16_t *cropKbcalidata);
        virtual void notifyRawdataPreview(gf_error_t err,
                                          gf_sz_rawdata_preview_t *raw_data);
        virtual void notifyFindSensorPreviewBmp(gf_error_t err,
                                                gf_sz_find_sensor_image_t *sensor_image);
        virtual void notifyFusionPreview(gf_error_t err, gf_sz_fusion_preview_t *cmd);
        virtual void notifyCalibrationFinish(gf_error_t err);
        virtual gf_error_t findSensor(void);
        virtual gf_error_t setCaptureParam(const int8_t *cmd_buf, uint32_t cmd_len);
        virtual gf_error_t testKbCalibration(const int8_t *cmd_buf, uint8_t cmd_len);
        virtual gf_error_t setEnrollTemplateCount(const int8_t *cmd_buf,
                                                  uint32_t cmd_len);
        virtual gf_error_t testFusion(void);
        virtual gf_error_t testRawdataPreview(void);
        virtual gf_error_t getVersion(void);
        virtual gf_error_t getConfig(int8_t **cfgBuf, uint32_t *bufLen);
        void notifyEnumerate(int32_t cmdId, gf_fingerprint_finger_id_t *result,
                             uint32_t max_size);
        virtual void handleEnrollAuthEndMessage();
        gf_error_t retrieveImage(gf_sz_bmp_data_t &bmpData);
        gf_error_t factoryExit(void);
        gf_error_t factoryInit(const int8_t *cmd_buf, uint32_t cmd_len);
        virtual bool isNeedLock(int32_t cmdId);
        gf_error_t setActiveGroup(const int8_t *in, uint32_t inLen);
        WORK_STATE mWorkState;
        static SZProductTest *szTest;
#ifdef SUPPORT_FRR_FAR
        static SZFrrFarTest *szFrrFarTest;
#endif  // END SUPPORT_FRR_FAR
        FingerprintCore *szFingerprintCore;
        uint32_t mCurrentCmd;

    private:
        uint32_t kb_step;
        pthread_t handle_thread;
        uint32_t group_id;
    };

}  // namespace goodix


#endif /* _SHENZHENPRODUCTTEST_H_ */
