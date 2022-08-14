/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _SZPRODUCTTEST_H_
#define _SZPRODUCTTEST_H_

#include "fingerprint_oplus.h"
#include "ProductTest.h"
#include "SZFingerprintCore.h"
#include "gf_sz_product_test_types.h"
#include "gf_event.h"
#include "EventCenter.h"
#include "HalUtils.h"

namespace goodix
{

    class SZProductTest : public ProductTest, public IEventHandler
    {
    public:
        char init_timestamp[TIME_STAMP_LEN];
        uint8_t ft_time[GF_SZ_FT_TIME] = {0};
        typedef enum
        {
            STATE_INIT,
            STATE_ENROLL,
            STATE_AUTHENTICATE,
            FUSION_PREVIEW,
            RAWDATA_PREVIEW,
            STATE_CANCEL,
            STATE_FARR_ENROLL,
            STATE_FARR_AUTHENTICATE,
            FACTORY_RESET_INT,
            STATE_KPI,
            STATE_CALI_BASE,
            STATE_CALI_AUTO_CALIBRATION,
            STATE_CALI_STOP_AUTO_CALIBRATION,
        } WORK_STATE;
        explicit SZProductTest(HalContext* context);
        virtual ~SZProductTest();
        virtual gf_error_t onMessage(const MsgBus::Message& msg);
        virtual void notifySensorDisplayControl(int64_t devId, int32_t cmdId,
                                            const int8_t *sensorData, int32_t dataLen);
        virtual void notifyPreviewDisplayControl(int64_t devId, int32_t cmdId,
                                            const gf_sz_bmp_data_t *previewData);
        void *gfCaptureBaseThread(gf_sz_k_b_calibrate_step_t kbStep);
        gf_error_t sleepSensor(void);
        virtual gf_error_t onEvent(gf_event_type_t e);

    protected:
        static void szNotifyCallback(const fingerprint_msg_t *msg);
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        virtual gf_error_t cropKbcalidata(uint16_t* kbcalidata, uint32_t data_width, uint32_t data_height,
                            gf_sz_image_crop_rect_t* cropRect, uint16_t* cropKbcalidata);
        virtual void notifyRawdataPreview(gf_error_t err, gf_sz_rawdata_preview_t* raw_data);
        virtual void notifyFindSensorPreviewBmp(gf_error_t err, gf_sz_find_sensor_image_t* sensor_image);
        virtual void notifyFusionPreview(gf_error_t err, gf_sz_fusion_preview_t* cmd);
        virtual void notifyCalibrationFinish(gf_error_t err);
        virtual gf_error_t findSensor(void);
        virtual gf_error_t setCaptureParam(const int8_t *cmd_buf, uint32_t cmd_len);
        virtual gf_error_t testKbCalibration(const int8_t  *cmd_buf, uint8_t cmd_len);
        virtual gf_error_t setEnrollTemplateCount(const int8_t *cmd_buf, uint32_t cmd_len);
        virtual gf_error_t testFusion(void);
        virtual gf_error_t testRawdataPreview(void);
        virtual gf_error_t getVersion(void);
        virtual gf_error_t getConfig(int8_t** cfgBuf, uint32_t* bufLen);
        virtual gf_error_t setFARRInit(const int8_t *cmd_buf, uint32_t cmd_len);
        virtual gf_error_t setFARRCancel(void);
        virtual gf_error_t getFARRCali(void);
        virtual void notifyFARRCali(gf_error_t err, gf_test_calibration_t* cmd);
        virtual gf_error_t setFARRPlayCali(const int8_t *cmd_buf, uint32_t cmd_len, int32_t cmdId);
        virtual void notifyFARRPlayCali(gf_error_t err, int32_t cmdId);
        virtual gf_error_t recordFARREnroll(void);
        virtual gf_error_t recordFARRAuth(void);
        virtual void notifyFARREnrollAuth(gf_error_t err, gf_test_frr_far_t* cmd, int32_t cmdId);
        virtual gf_error_t setFARRPlayEnrollAuth(const int8_t *cmd_buf, uint32_t cmd_len, int32_t cmdId);
        virtual void notifyFARRPlayEnroll(gf_error_t err, gf_test_frr_far_t* cmd);
        virtual void notifyFARRPlayAuth(gf_error_t err, gf_test_frr_far_t* cmd);
        virtual gf_error_t setFARRPlayCaliCancel(void);
        void notifyEnumerate(int32_t cmdId, fingerprint_finger_id_t* result, uint32_t max_size);

        virtual void handleEnrollAuthEndMessage();
        gf_error_t retrieveImage(gf_sz_bmp_data_t& bmpData);
        gf_error_t factoryExit(void);
        gf_error_t factoryInit(const int8_t *cmd_buf, uint32_t cmd_len);
        virtual bool isNeedLock(int32_t cmdId);
        gf_error_t setFARRPlayRawDataEnrollAuth(const int8_t *cmd_buf, uint32_t cmd_len, int32_t cmdId);
        void notifyRawdataAutnEnroll(gf_error_t err, gf_test_frr_far_raw_data_t* raw_data, int32_t cmdId);
        gf_error_t setActiveGroup(const int8_t *in, uint32_t inLen);

        WORK_STATE mWorkState;
        static SZProductTest* szTest;
        FingerprintCore* szFingerprintCore;
        uint32_t mCurrentCmd;

    private:
        uint32_t kb_step;
        pthread_t handle_thread;
        uint32_t group_id;
    };

}  // namespace goodix


#endif /* _SHENZHENPRODUCTTEST_H_ */
