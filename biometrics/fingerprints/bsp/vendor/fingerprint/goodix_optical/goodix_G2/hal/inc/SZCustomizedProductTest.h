/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _SZCUSTOMIZEDPRODUCTTEST_H_
#define _SZCUSTOMIZEDPRODUCTTEST_H_

#include "SZProductTest.h"

namespace goodix
{
    class SZCustomizedProductTest : public SZProductTest
    {
    public:
        explicit SZCustomizedProductTest(HalContext* context);
        virtual ~SZCustomizedProductTest();

        virtual gf_error_t onEvent(gf_event_type_t e);
        gf_error_t getSensorInfo(gf_sz_chip_info_t *sensor);
        gf_error_t findBrightestPoint(gf_sz_expo_auto_calibration_t *expo_auto_calibration);
        gf_error_t findBrightestPoint_3p(gf_sz_expo_auto_calibration_t *expo_auto_calibration);
        gf_error_t findTargetExposureTime(gf_sz_expo_auto_calibration_t *expo_auto_calibration, gf_sz_exposure_data_t *exposure_dump_data);

        gf_error_t checkAeParam(gf_sz_expo_auto_calibration_t *expo_auto_calibration, gf_sz_exposure_data_t *exposure_dump_data);
        gf_error_t setExposureParam(gf_sz_expo_auto_calibration_t *expo_auto_calibration, int32_t flag);
        void notifyExpoAutoCalibrationFinish(gf_sz_expo_auto_calibration_t *expo_auto_calibration, uint32_t flag);
        gf_error_t getBigDataSensorInfo(void **buf);
        uint32_t ae_stop_flag;
        gf_error_t getMTInfo(void **mt, uint32_t *mt_len);
        uint32_t simple_cali_ae_flag;

    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);
        gf_error_t factoryCaptureImage(gf_sz_test_rawdata_t *rawdata, uint16_t exposure_time, uint8_t frame_avg_num, uint16_t pga_gain);
        gf_error_t expoAutoCalibration(void);
        gf_error_t stopExpoAutoCalibration(void);
        gf_error_t spiTest(void);
        gf_error_t factoryResetHard(void);
        gf_error_t factoryMtCheck(void);
        gf_error_t rawDataDedark(gf_sz_test_rawdata_t *raw_data, gf_sz_chip_info_t *sensor, uint32_t *row_dark_average);
        gf_error_t setHbmMode(void);
        gf_error_t closeHbmMode(void);
        gf_error_t setHighBrightness(void);
        gf_error_t setLowBrightness(uint32_t value);

        void notifySpiTest(gf_error_t err, gf_sz_spi_test_t *result);
        void notifyCaptureImageFinish(gf_error_t err, uint32_t capture_cmd);
        void notifyFactoryPerformance(gf_error_t err, gf_sz_factory_performance_t *result);
        void notifyFactoryCalibrate(gf_error_t err, gf_sz_factory_calibrate_t *result, uint32_t capture_cmd);
        void notifyfactoryReset(gf_error_t err);
        void notifyfactoryMtCheck(gf_error_t err);
        void notifySetScreenBrightness(gf_error_t err, int32_t cmd_id);
        gf_error_t gfHalSampleCaliInit();
        gf_error_t gfHalSampleCaliExit();
        void gfHalFactoryNotifySampleCaliBase(SAMPLE_CALI_STATE state);
        gf_error_t gfHalSampleCaliBase();
        gf_error_t gfHalSampleCaliPerformance(const int8_t *in, uint32_t inLen);
        gf_error_t gfHalSampleSaveData();
        void gfHalFactoryNotifySampleCaliPerformance(SAMPLE_CALI_STATE state);
    private:
        void* factoryCaptureImageThread(uint32_t cmd);
        gf_error_t factoryCalibrate(uint32_t frame_type, uint8_t reliability_flag, uint32_t capture_cmd);
        void* expoAutoCalibrationThread(void* obj);
        uint16_t g_frame_data[GF_SZ_FACTORY_TEST_FRAME_DATA_SIZE];
    };
}  // namespace goodix

#endif /* _SZCUSTOMIZEDPRODUCTTEST_H_ */
