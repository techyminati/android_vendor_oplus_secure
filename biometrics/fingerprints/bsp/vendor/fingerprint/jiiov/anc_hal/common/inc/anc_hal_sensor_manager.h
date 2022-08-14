#ifndef ANC_HAL_SENSOR_MANAGER_H
#define ANC_HAL_SENSOR_MANAGER_H

#include <pthread.h>

#include "anc_hal_manager.h"
#include "anc_error.h"
#include "sensor_command_param.h"


struct AncFingerprintManager;
struct sAncSensorManager {
    ANC_RETURN_TYPE (*Init)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*Deinit)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*SetPowerMode)(struct AncFingerprintManager *p_manager, ANC_SENSOR_POWER_MODE power_mode);

    pthread_mutex_t mutex;
    ANC_SENSOR_POWER_MODE sensor_power_status;
};



ANC_RETURN_TYPE InitSensorManager(struct AncFingerprintManager *p_manager);
ANC_RETURN_TYPE DeinitSensorManager(struct AncFingerprintManager *p_manager);


#endif
