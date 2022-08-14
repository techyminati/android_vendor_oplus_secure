#define LOG_TAG "[ANC_HAL][SensorManager]"

#include "anc_hal_sensor_manager.h"

#include "anc_tac_sensor.h"
#include "anc_hal_sensor_device.h"
#include "anc_log.h"



// match with ANC_SENSOR_POWER_MODE
static char* gp_sensor_power_mode_string[] = {
    "normal",
    "wakeup",
    "sleep",
    "power on",
    "power on init",
    "power off",
    "hw reset",
    "init",
    "wakeup reset",
    "hw reset init",
    "wakeup scan",
};

static char *ConverSensorPowerModeToString(ANC_SENSOR_POWER_MODE power_mode) {
    char *p_string = "NULL";
    int power_mode_string_array_size = sizeof(gp_sensor_power_mode_string)/sizeof(gp_sensor_power_mode_string[0]);
    int power_mode_count = ANC_SENSOR_WAKEUP_SCAN + 1;

    if ((power_mode <= ANC_SENSOR_WAKEUP_SCAN)
       && (power_mode_count == power_mode_string_array_size)) {
        p_string = gp_sensor_power_mode_string[power_mode];
    } else {
        ANC_LOGE("power mode string array size:%d, power mode count:%d, power mode:%d",
                  power_mode_string_array_size, power_mode_count, power_mode);
    }

    return p_string;
}

static ANC_RETURN_TYPE SensorWakeup(ANC_SENSOR_POWER_MODE power_mode) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    do {
#ifdef ANC_SENSOR_SPI_MTK
        if (ANC_OK != (ret_val = SensorDeviceOpenSpiClk())) {
            ANC_LOGE("fail to open sensor spi clk");
            ret_val = ANC_FAIL_SPI_OPEN;
            break;
        }
#endif
        if (ANC_OK != (ret_val = SensorSetPowerMode(power_mode))) {
            ANC_LOGE("fail to wake sensor, return value = %d", ret_val);
        }
    } while (0);

    return ret_val;
}

static ANC_RETURN_TYPE SensorSleep(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    do {
#ifdef ANC_SENSOR_SPI_MTK
        if (ANC_OK != (ret_val = SensorDeviceOpenSpiClk())) {
            ANC_LOGE("fail to open sensor spi clk");
            ret_val = ANC_FAIL_SPI_OPEN;
            break;
        }
#endif
        if (ANC_OK != (ret_val = SensorSetPowerMode(ANC_SENSOR_SLEEP))) {
            ANC_LOGE("fail to sleep sensor, return value = %d", ret_val);
        }
#ifdef ANC_SENSOR_SPI_MTK
        SensorDeviceCloseSpiClk();
#endif
    } while (0);

    return ret_val;
}

static ANC_RETURN_TYPE SensorInit(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    do {
#ifdef ANC_SENSOR_SPI_MTK
        if (ANC_OK != (ret_val = SensorDeviceOpenSpiClk())) {
            ANC_LOGE("fail to open sensor spi clk");
            ret_val = ANC_FAIL_SPI_OPEN;
            break;
        }
#endif
        ANC_LOGD("init sensor");
        if (ANC_OK != (ret_val = InitSensor())) {
            ANC_LOGE("fail to init sensor, return value = %d", ret_val);
        }
        SensorSleep();
    } while (0);

    return ret_val;
}

static ANC_RETURN_TYPE SensorDeinit(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    do {
#ifdef ANC_SENSOR_SPI_MTK
        if (ANC_OK != (ret_val = SensorDeviceOpenSpiClk())) {
            ANC_LOGE("fail to open sensor spi clk");
            ret_val = ANC_FAIL_SPI_OPEN;
            break;
        }
#endif
        if (ANC_OK != (ret_val = DeinitSensor())) {
            ANC_LOGE("fail to deinit sensor, return value = %d", ret_val);
        }
#ifdef ANC_SENSOR_SPI_MTK
        SensorDeviceCloseSpiClk();
#endif
    } while (0);

    return ret_val;
}

static ANC_RETURN_TYPE AncHalSensorPowerMode(AncFingerprintManager *p_manager, ANC_SENSOR_POWER_MODE power_mode) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorManager *p_sensor_manager = p_manager->p_sensor_manager;

    ANC_LOGD("current power mode:%s, %d; set sensor power mode:%s, %d",
            ConverSensorPowerModeToString(p_sensor_manager->sensor_power_status),
            p_sensor_manager->sensor_power_status,
            ConverSensorPowerModeToString(power_mode), power_mode);

    pthread_mutex_lock(&p_sensor_manager->mutex);

    switch (power_mode) {
        case ANC_SENSOR_POWER_ON:
            if (ANC_OK != (ret_val = SensorDevicePowerOn())) {
                ANC_LOGE("fail to power on sensor device");
            }
            break;
        case ANC_SENSOR_POWER_ON_INIT:
            ANC_LOGD("power on sensor device");
            if (ANC_OK != (ret_val = SensorDevicePowerOn())) {
                ANC_LOGE("fail to power on sensor device");
                break;
            }

            ANC_LOGD("reset sensor device");
            if (ANC_OK != (ret_val = SensorDeviceReset())) {
                ANC_LOGE("fail to reset sensor device");
                SensorDevicePowerOff();
                break;
            }

            if (ANC_OK != (ret_val = SensorInit())) {
                SensorDevicePowerOff();
            }
            break;
        case ANC_SENSOR_POWER_OFF:
            SensorSleep();
            if (ANC_OK != (ret_val = SensorDevicePowerOff())) {
                ANC_LOGE("fail to power off sensor device");
            }
            break;
        case ANC_SENSOR_WAKEUP:
        case ANC_SENSOR_WAKEUP_RESET:
        case ANC_SENSOR_WAKEUP_SCAN:
            ret_val = SensorWakeup(power_mode);
            break;
        case ANC_SENSOR_SLEEP:
            ret_val = SensorSleep();
            break;
        case ANC_SENSOR_HW_RESET:
            ret_val = SensorDeviceReset();
            break;
        case ANC_SENSOR_HW_RESET_INIT:
            if (ANC_OK != (ret_val = SensorDeviceReset())) {
                ANC_LOGE("fail to reset sensor device");
                SensorSleep();
                break;
            }

            ret_val = SensorInit();
            break;
        case ANC_SENSOR_INIT:
            ret_val = SensorInit();
            break;
        default:
            ANC_LOGE("don't support the power mode, %d", power_mode);
            ret_val = ANC_FAIL_INVALID_COMMAND;
            break;
    }
    if (ANC_OK == ret_val) {
        p_sensor_manager->sensor_power_status = power_mode;
    }
    ANC_LOGD("sensor current power mode:%s, %d",
            ConverSensorPowerModeToString(p_sensor_manager->sensor_power_status),
            p_sensor_manager->sensor_power_status);

    pthread_mutex_unlock(&p_sensor_manager->mutex);

    return ret_val;
}

static ANC_RETURN_TYPE AncHalSensorInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorManager *p_sensor_manager = p_manager->p_sensor_manager;

    pthread_mutex_init(&p_sensor_manager->mutex, NULL);
    p_sensor_manager->sensor_power_status = ANC_SENSOR_POWER_OFF;

    do {
        ANC_LOGD("open sensor device");
        if (ANC_OK != (ret_val = OpenSensorDevice())) {
            ANC_LOGE("fail to open sensor device");
            break;
        }

        ret_val = p_sensor_manager->SetPowerMode(p_manager, ANC_SENSOR_POWER_ON_INIT);
        if (ANC_OK != ret_val) {
            CloseSensorDevice();
        }
    } while (0);

    if (ANC_OK != ret_val) {
        pthread_mutex_destroy(&p_sensor_manager->mutex);
    }

    return ret_val;
}

static ANC_RETURN_TYPE AncHalSensorDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncSensorManager *p_sensor_manager = p_manager->p_sensor_manager;

    pthread_mutex_destroy(&p_sensor_manager->mutex);

    ANC_LOGD("deinit sensor\n");
    if (ANC_OK != (ret_val = SensorDeinit())) {
        ANC_LOGE("fail to deinit sensor");
    }

    ANC_LOGD("power off sensor device\n");
    if (ANC_OK != (ret_val = SensorDevicePowerOff())) {
        ANC_LOGE("fail to power off sensor device\n");
    }

    ANC_LOGD("close sensor device\n");
    if (ANC_OK != (ret_val = CloseSensorDevice())) {
        ANC_LOGE("fail to close sensor device\n");
    }
    p_sensor_manager->sensor_power_status = ANC_SENSOR_POWER_OFF;

    return ret_val;
}

static AncSensorManager g_sensor_manager = {
    .Init = AncHalSensorInit,
    .Deinit = AncHalSensorDeinit,
    .SetPowerMode = AncHalSensorPowerMode,
};

ANC_RETURN_TYPE InitSensorManager(AncFingerprintManager *p_manager) {
    p_manager->p_sensor_manager = &g_sensor_manager;
    return g_sensor_manager.Init(p_manager);
}

ANC_RETURN_TYPE DeinitSensorManager(AncFingerprintManager *p_manager) {
    return g_sensor_manager.Deinit(p_manager);
}
