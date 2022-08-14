#define LOG_TAG "[ANC_HAL][VirtualSensorDevice]"

#include "anc_hal_sensor_device.h"

#include "anc_error.h"
#include "anc_log.h"



ANC_RETURN_TYPE SensorDeviceReset(void) {
    return ANC_OK;
}

ANC_RETURN_TYPE SensorDevicePowerOn(void) {
    return ANC_OK;
}

#ifdef ANC_SENSOR_SPI_MTK
ANC_RETURN_TYPE SensorDeviceOpenSpiClk(void) {
    return ANC_OK;
}

ANC_RETURN_TYPE SensorDeviceCloseSpiClk(void) {
    return ANC_OK;
}
#endif

ANC_RETURN_TYPE SensorDevicePowerOff(void) {
    return ANC_OK;
}

ANC_RETURN_TYPE SensorClearTPFlag(void) {
    return ANC_OK;
}

ANC_RETURN_TYPE SensorDeviceSetSpiSpeed(uint32_t spi_speed) {
    ANC_UNUSED(spi_speed);

    return ANC_OK;
}

ANC_RETURN_TYPE OpenSensorDevice(void) {
    return ANC_OK;
}

ANC_RETURN_TYPE CloseSensorDevice(void) {
    return ANC_OK;
}

ANC_BOOL SensorDeviceIsOpened(void) {
    return ANC_OK;
}

ANC_RETURN_TYPE SensorDeviceWrite(uint8_t *write_buffer, uint32_t num_bytes) {
    ANC_UNUSED(write_buffer);
    ANC_UNUSED(num_bytes);

    return ANC_OK;
}

ANC_RETURN_TYPE SensorDeviceWriteRead(uint8_t *write_buffer, uint32_t write_bytes,
                         uint8_t *read_buffer, uint32_t read_bytes, uint32_t read_buffer_length) {
    ANC_UNUSED(write_buffer);
    ANC_UNUSED(write_bytes);
    ANC_UNUSED(read_buffer);
    ANC_UNUSED(read_bytes);
    ANC_UNUSED(read_buffer_length);

    return ANC_OK;
}
