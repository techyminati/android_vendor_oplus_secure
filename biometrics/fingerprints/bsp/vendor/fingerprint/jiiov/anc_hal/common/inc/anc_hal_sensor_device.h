#ifndef __ANC_HAL_SENSOR_DEVICE_H__
#define __ANC_HAL_SENSOR_DEVICE_H__

#include "anc_error.h"
#include "anc_type.h"
#include <stdint.h>


/* Kernel device node for IOCTL/Read/Write */
#ifndef ANC_DEVICE_PATH_NAME
#define ANC_DEVICE_PATH_NAME     "/dev/jiiov_fp"
#endif

/* Magic code for IOCTL-subsystem */
#ifndef ANC_IOC_MAGIC
#define ANC_IOC_MAGIC            'a'
#endif

/* Allocate/Release driver resource (GPIO/SPI etc.) */
#define ANC_IOC_INIT             _IO(ANC_IOC_MAGIC, 0)
#define ANC_IOC_DEINIT           _IO(ANC_IOC_MAGIC, 1)

/* HW reset the fingerprint module */
#define ANC_IOC_RESET            _IO(ANC_IOC_MAGIC, 2)

/* Low-level IRQ control */
#define ANC_IOC_ENABLE_IRQ       _IO(ANC_IOC_MAGIC, 3)
#define ANC_IOC_DISABLE_IRQ      _IO(ANC_IOC_MAGIC, 4)

/* SPI bus clock control, for power-saving purpose */
#define ANC_IOC_ENABLE_SPI_CLK   _IO(ANC_IOC_MAGIC, 5)
#define ANC_IOC_DISABLE_SPI_CLK  _IO(ANC_IOC_MAGIC, 6)

/* Fingerprint module power control */
#define ANC_IOC_ENABLE_POWER     _IO(ANC_IOC_MAGIC, 7)
#define ANC_IOC_DISABLE_POWER    _IO(ANC_IOC_MAGIC, 8)
#define ANC_IOC_CLEAR_FLAG       _IO(ANC_IOC_MAGIC, 20)

/* SPI speed related */
#define ANC_IOC_SPI_SPEED        _IOW(ANC_IOC_MAGIC, 9, uint32_t)


ANC_RETURN_TYPE OpenSensorDevice(void);
ANC_RETURN_TYPE CloseSensorDevice(void);
ANC_BOOL SensorDeviceIsOpened(void);
ANC_RETURN_TYPE SensorDeviceReset(void);
ANC_RETURN_TYPE SensorDevicePowerOn(void);
ANC_RETURN_TYPE SensorDevicePowerOff(void);
#ifdef ANC_SENSOR_SPI_MTK
ANC_RETURN_TYPE SensorDeviceOpenSpiClk(void);
ANC_RETURN_TYPE SensorDeviceCloseSpiClk(void);
#endif
ANC_RETURN_TYPE SensorClearTPFlag(void);
ANC_RETURN_TYPE SensorDeviceWrite(uint8_t *write_buffer, uint32_t num_bytes);
ANC_RETURN_TYPE SensorDeviceWriteRead(uint8_t *write_buffer, uint32_t write_bytes, uint8_t *read_buffer, uint32_t read_bytes, uint32_t read_buffer_length);
ANC_RETURN_TYPE SensorDeviceSetSpiSpeed(uint32_t spi_speed);

#endif
