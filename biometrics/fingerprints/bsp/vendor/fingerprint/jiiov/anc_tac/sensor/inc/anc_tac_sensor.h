#ifndef __ANC_TAC_SENSOR_H__
#define __ANC_TAC_SENSOR_H__

#include "anc_error.h"
#include "sensor_command_param.h"

#define AUTO_EXPOSURE_TIME -1

ANC_RETURN_TYPE InitSensor();
ANC_RETURN_TYPE DeinitSensor();

ANC_RETURN_TYPE SensorSetParam(AncSensorCommandParam *p_param);
ANC_RETURN_TYPE SensorGetParam(AncSensorCommandParam *p_param);
ANC_RETURN_TYPE SensorSetExposureTime(int32_t exposure_time_us);    // exposure_time_us < 0: auto expo
ANC_RETURN_TYPE SensorSetRetryExposureTime(uint8_t type, int32_t retry0_exposure_time_us,
        int32_t retry1_exposure_time_us, int32_t retry2_exposure_time_us);  // exposure_time_us < 0: auto expo
ANC_RETURN_TYPE SensorGetExposureTime(int32_t *exposure_time_us);   // us
ANC_RETURN_TYPE SensorGetTotalExposureTime(int32_t *exposure_time_ms);  // ms
ANC_RETURN_TYPE SensorSetFrameFusionNum(uint8_t retry0, uint8_t retry1, uint8_t retry2);

ANC_RETURN_TYPE SensorCaptureImage(void);
ANC_RETURN_TYPE SensorCaptureImageWithMode(ANC_SENSOR_INNER_DATA_MODE mode, int32_t *p_exposure_time_us);

ANC_RETURN_TYPE SensorGetChipId(uint32_t *sensor_chip_id);
ANC_RETURN_TYPE SensorGetModuleId(char *p_module_id, uint32_t buffer_size);
ANC_RETURN_TYPE SensorSetPowerMode(ANC_SENSOR_POWER_MODE power_mode);
ANC_RETURN_TYPE SensorRestoreDefaultImageSize(void);
ANC_RETURN_TYPE SensorSelfTest(void);
ANC_RETURN_TYPE SensorAgingTest(void);

ANC_RETURN_TYPE SensorGetAbnormalExpoType(void);
ANC_BOOL SensorIsAbnormalExpo(void);

#endif
