/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _DELMARHALUTILS_H_
#define _DELMARHALUTILS_H_

#include "gf_base_types.h"
#include "gf_delmar_types.h"
#include "FingerprintCore.h"

namespace goodix {
    class DelmarHalUtils {
    public:
        static uint64_t calculateCircleSensorPriority(
                const gf_delmar_coordinate_info_t *coordinateInfo, int32_t sensorNum, uint8_t morphotype);

        static uint64_t calculateEllipseSensorPriority(const gf_delmar_coordinate_info_t *coordinateInfo,
                const gf_delmar_press_area_info_t *pressArea, int32_t sensorNum, int32_t sensorWidth,
                int32_t sensorHeight, const gf_delmar_config_t *delmarConfig, uint8_t morphotype);

        static gf_error_t printKpiPerformance(FingerprintCore::WORK_STATE state, int64_t totalKpiTime,
                const gf_delmar_algo_performance_t *performance, const char *func_name);

        static bool isInCenterMode(uint32_t sensorIds, uint8_t morphotype);
        static gf_error_t genTimestamp(char *timestamp, uint32_t len);
    };
}  // namespace goodix

#endif  // _DELMARHALUTILS_H_