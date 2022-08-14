/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _DELMARHALUTILS_H_
#define _DELMARHALUTILS_H_

#include "gf_base_types.h"
#include "gf_delmar_types.h"
#include "FingerprintCore.h"
#include "HalContext.h"
#include "MsgBus.h"

enum {
    DELMAR_MSG_BEGIN = goodix::MsgBus::MSG_MAX,
    MSG_PRODUCT_TEST_COLLECT_FINISHED,
    MSG_PRODUCT_TEST_CALCULATE_FINISHED,
    MSG_PRODUCT_TEST_IMAGE_QUALITY_FINISHED,
    MSG_ALGO_DSP_GET_FEATURE_STEP_FINISHED,
    MSG_HB_RATE_CAPTURE_IMG_FINISHED,
    MSG_HB_RATE_COLLECT_FINISHED,
    MSG_HB_RATE_COLLECT_PPG_FINISHED,
    MSG_HB_RATE_DUMP_RESULT,
    MSG_AUTHENTICATE_BEFORE_ALGO,
    MSG_ENROLL_BEFORE_ALGO,
    MSG_AUTH_DSP_FINISHED,
    DELMAR_MSG_END,
};

namespace goodix {
    class HalContext;

    class DelmarHalUtils {
    public:
        static uint64_t calculateCircleSensorPriority(
                const gf_delmar_coordinate_info_t *coordinateInfo, int32_t sensorNum, uint8_t morphotype);

        static uint64_t calculateEllipseSensorPriority(const gf_delmar_coordinate_info_t *coordinateInfo,
                const gf_delmar_press_area_info_t *pressArea, int32_t sensorNum, int32_t sensorWidth,
                int32_t sensorHeight, const gf_delmar_config_t *delmarConfig, uint8_t morphotype);

        static gf_error_t printKpiPerformance(FingerprintCore::WORK_STATE state, int64_t totalKpiTime, const gf_delmar_algo_performance_t *performance, HalContext* ctx, const char *func_name, bool forcePrintKpi = false);  // NOLINT(575)

        static bool isInCenterMode(uint32_t sensorIds, uint8_t morphotype);
        static gf_error_t genTimestamp(char *timestamp, uint32_t len);
        static gf_error_t setSensorPrivData(uint8_t *data, uint32_t size);
        static uint32_t calcSensorIdCount(uint64_t sensorIds);
        static void registerModuleVersion(const char *module_name, const char* version);
        static gf_error_t checkModuleVerions(void);
        static bool hasThermometer(HalContext *context);
        static bool hasSecondGain(gf_delmar_sensor_type_t sensorType);
        static uint32_t countSensorIdNum(uint64_t sensorIds);
        static void setKpiTotalTime(uint32_t time);
        static bool independentFastAuthNeedRetry(HalContext *context, gf_error_t result, int32_t retry);
        static bool isOptical7X(uint8_t optical);

    private:
        static void checkAlgoTotalTime(uint32_t algoTotalTime, const char *func_name);
    };
}  // namespace goodix

#endif  // _DELMARHALUTILS_H_