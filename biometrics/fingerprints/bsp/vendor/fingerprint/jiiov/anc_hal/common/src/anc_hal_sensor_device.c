#define LOG_TAG "[ANC_HAL][SensorDevice]"

#include "anc_hal_sensor_device.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#ifdef ANC_QCOM_SPI_BIT_SHIFT
#include <arm_neon.h>
#endif

#include "anc_error.h"
#include "anc_log.h"


static int g_device_handle = -1;

static ANC_RETURN_TYPE SensorDeviceIoctl(int ioctl_cmd, uint32_t ioctl_data) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    int retval = ioctl(g_device_handle, ioctl_cmd, ioctl_data);
    if (retval != 0) {
        ANC_LOGE("sensor device ioctl fail, cmd:%d, ret value:%d", ioctl_cmd, retval);
        ret_val = ANC_FAIL_HW_UNAVALABLE;
    }

    return ret_val;
}

ANC_RETURN_TYPE SensorDeviceReset(void) {
    return SensorDeviceIoctl(ANC_IOC_RESET, 0);
}

ANC_RETURN_TYPE SensorDevicePowerOn(void) {
    return SensorDeviceIoctl(ANC_IOC_ENABLE_POWER, 0);
}

#ifdef ANC_SENSOR_SPI_MTK
ANC_RETURN_TYPE SensorDeviceOpenSpiClk(void) {
    ANC_LOGD("SensorDeviceOpenSpiClk");
    return SensorDeviceIoctl(ANC_IOC_ENABLE_SPI_CLK, 0);
}

ANC_RETURN_TYPE SensorDeviceCloseSpiClk(void) {
    ANC_LOGD("SensorDeviceCloseSpiClk");
    return SensorDeviceIoctl(ANC_IOC_DISABLE_SPI_CLK, 0);
}
#endif

ANC_RETURN_TYPE SensorDevicePowerOff(void) {
    return SensorDeviceIoctl(ANC_IOC_DISABLE_POWER, 0);
}

ANC_RETURN_TYPE SensorClearTPFlag(void) {
    return SensorDeviceIoctl(ANC_IOC_CLEAR_FLAG, 0);
}

ANC_RETURN_TYPE SensorDeviceSetSpiSpeed(uint32_t spi_speed) {
    return SensorDeviceIoctl(ANC_IOC_SPI_SPEED, spi_speed);
}

ANC_RETURN_TYPE OpenSensorDevice(void) {
    struct stat st;

    if (g_device_handle >= 0) {
        ANC_LOGE("sensor device has been open, device handle:%d", g_device_handle);
        return ANC_OK;
    }

    if (stat(ANC_DEVICE_PATH_NAME, &st) != 0) {
        ANC_LOGE("sensor device open fail, no such device path");
        return ANC_FAIL_HW_UNAVALABLE;
    }

    if ((g_device_handle = open(ANC_DEVICE_PATH_NAME, O_RDWR)) < 0) {
        ANC_LOGE("sensor device open fail, device handle:%d", g_device_handle);
        return ANC_FAIL_HW_UNAVALABLE;
    }

    return ANC_OK;
}

ANC_RETURN_TYPE CloseSensorDevice(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if (g_device_handle >= 0) {
        int retval = close(g_device_handle);
        if (retval != 0) {
            ANC_LOGE("sensor device close fail, ret value:%d", retval);
            ret_val = ANC_FAIL;
        } else {
            g_device_handle = -1;
        }
    }

    return ret_val;
}

ANC_BOOL SensorDeviceIsOpened(void) {
    return g_device_handle >= 0 ? ANC_TRUE : ANC_FALSE;
}

ANC_RETURN_TYPE SensorDeviceWrite(uint8_t *write_buffer, uint32_t num_bytes) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ssize_t retval = write(g_device_handle, write_buffer, num_bytes);
    if (retval < 0) {
        ANC_LOGE("sensor device write fail, ret value:%zd", retval);
        ret_val = ANC_FAIL;
    }

    return ANC_OK;
}

#ifdef ANC_QCOM_SPI_BIT_SHIFT
static ANC_RETURN_TYPE shift_transfer(uint8_t *rxbuf, int len) {
    if (NULL == rxbuf) {
        return ANC_FAIL;
    }

    uint8_t * ptr = (uint8_t *)(rxbuf + len - 1);
    while (ptr > rxbuf) {
        ptr--;
        ptr[1] = (uint8_t)(((ptr[1] >> 1) & 0x7F) | ((ptr[0] & 0x01) << 7));
    }
    rxbuf[0] = rxbuf[0] >> 1;

    return ANC_OK;
}

static ANC_RETURN_TYPE shift_neon(uint8_t *rxbuf, int len) {
    if (NULL == rxbuf) {
        return ANC_FAIL;
    }

    if (len < 64) {
        return shift_transfer(rxbuf, len);
    }

    int left = len % 64;
    uint8_t * ptr = (uint8_t *)(rxbuf + len - 1);
    while (left > 0) {
            ptr--;
            left--;
            ptr[1] = (uint8_t)(((ptr[1] >> 1) & 0x7F) | ((ptr[0] & 0x01) << 7));
    }

    uint8x16_t buf = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t t = 0;
    for (int k = 0; k + 16*4 < len; k += 16*4) {
        uint8x16x4_t a0 = vld4q_u8(rxbuf+k);
        uint8x16x4_t b0;
        buf = vextq_u8(buf, a0.val[3], 15);
        buf = vsetq_lane_u8(t, buf, 0);

        b0.val[0] = vsriq_n_u8(vshlq_n_u8(buf, 7), a0.val[0], 1);
        b0.val[1] = vsriq_n_u8(vshlq_n_u8(a0.val[0], 7), a0.val[1], 1);
        b0.val[2] = vsriq_n_u8(vshlq_n_u8(a0.val[1], 7), a0.val[2], 1);
        b0.val[3] = vsriq_n_u8(vshlq_n_u8(a0.val[2], 7), a0.val[3], 1);

        t = rxbuf[k+63];
        vst4q_u8(rxbuf+k, b0);
    }

    return ANC_OK;
}
#endif

ANC_RETURN_TYPE SensorDeviceWriteRead(uint8_t *write_buffer, uint32_t write_bytes,
                         uint8_t *read_buffer, uint32_t read_bytes, uint32_t read_buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    size_t need_read_bytes = write_bytes + read_bytes;
    if (need_read_bytes > read_buffer_length) {
        ANC_LOGE("read buffer length is too small: %zu, need : %zu\n", read_buffer_length, need_read_bytes);
        return ANC_FAIL_BUFFER_TOO_SMALL;
    }

    memcpy(read_buffer, write_buffer, write_bytes);

    ssize_t read_size = read(g_device_handle, read_buffer, read_buffer_length);
    if (read_size != (ssize_t)read_buffer_length) {
        ANC_LOGE("sensor device read fail: read_size: %zu, read_buffer_length: %zu", read_size, read_buffer_length);
        ret_val = ANC_FAIL;
    }
#ifdef ANC_QCOM_SPI_BIT_SHIFT
    else {
        /* Work around for Qualcomm platform SPI new timming-cycle mechanism */
        shift_neon(read_buffer, (int)read_size);
    }
#endif

    return ret_val;
}
