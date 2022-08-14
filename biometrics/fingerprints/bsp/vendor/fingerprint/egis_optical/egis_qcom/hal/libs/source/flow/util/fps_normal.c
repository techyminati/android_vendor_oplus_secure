#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fps_normal.h"
#include "plat_log.h"
#include "response_def.h"

struct ioctl_cmd {
    int int_mode;
    int detect_period;
    int detect_threshold;
};

#ifndef DEVICE_DRIVER_HAS_SPI
struct egis_ioc_transfer {
    __u32 int_mode;
    __u32 detect_period;
    __u32 detect_threshold;
#ifdef POWER_CONTRL
    __u32 power_on;
#endif
};
#else
struct egis_ioc_transfer {
    __u8* tx_buf;
    __u8* rx_buf;
    __u32 len;
    __u32 speed_hz;
    __u16 delay_usecs;
    __u8 bits_per_word;
    __u8 cs_change;
    __u8 opcode;
#ifdef PROJECT_AOP_DRIVER
    __u8 sector;
    __u32 umax;
    __u32 umin;
    __u8 flag;
#endif
    __u8 pad[3];
};

#define FP_SPI_SPEED_HZ 9700000
#define DELAY_USECS 0
#define BITS_PER_WORD 8
#define CS_CHANGE 1
#define SPI_IOC_MAGIC 'k'
#define SPI_MSGSIZE(N)                                                   \
    ((((N) * (sizeof(struct egis_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
         ? ((N) * (sizeof(struct egis_ioc_transfer)))                    \
         : 0)
#define SPI_IOC_MESSAGE(N) _IOW(SPI_IOC_MAGIC, 0, char[SPI_MSGSIZE(N)])
#endif
#define DEVICE_PATH_NAME "/dev/esfp0"

#define FP_SENSOR_RESET 0x04
#define FP_POWER_CONTROL 0x05
#define FP_SENSOR_RESET_SET 0x07

#ifdef __OPLUS__
#define WAIT_TP_TOUCH_DOWN 0x16
#define WAIT_TP_TOUCH_UP   0x17
#define WAIT_UI_READY      0x18
#endif

#define INT_TRIGGER_INIT 0xa4   // Trigger signal initial routine
#define INT_TRIGGER_CLOSE 0xa5  // Trigger signal close routine
#define INT_TRIGGER_ABORT 0xa8

// INT TRIGGER TYPE
#define EDGE_TRIGGER_FALLING 0x0
#define EDGE_TRIGGER_RAISING 0x1
#define LEVEL_TRIGGER_LOW 0x2
#define LEVEL_TRIGGER_HIGH 0x3

#define FP_SPICLK_ENABLE 0x81
#define FP_SPICLK_DISABLE 0x82

unsigned char pad_edge_f[3] = {EDGE_TRIGGER_FALLING, 1, 1};
unsigned char pad_edge_r[3] = {EDGE_TRIGGER_RAISING, 10, 6};
unsigned char pad_level_l[3] = {LEVEL_TRIGGER_LOW, 0, 0};

#ifdef MTK_EVB
unsigned char* const default_pad = pad_edge_f;
#elif defined(__EDGE_TRIGGER__)
unsigned char* const default_pad = pad_edge_r;
#else
unsigned char* const default_pad = pad_level_l;
#endif

const unsigned char* g_pad = default_pad;

static unsigned int egis_call_driver(int device_handle, unsigned int ioctl_code, void* in_buffer,
		__unused unsigned int in_buffer_size, __unused void* out_buffer, void* pad,
		__unused unsigned int speed_hz) {
#ifndef DEVICE_DRIVER_HAS_SPI
    static struct egis_ioc_transfer xfer;

#ifdef POWER_CONTRL
    if (FP_POWER_CONTROL == ioctl_code) {
        xfer.power_on = ((__u8*)in_buffer)[0];
    }
#endif

    if (NULL != pad) {
        xfer.int_mode = ((__u8*)pad)[0];
        xfer.detect_period = ((__u8*)pad)[1];
        xfer.detect_threshold = ((__u8*)pad)[2];
    }

    if (-1 == ioctl(device_handle, ioctl_code, &xfer)) {
        return FINGERPRINT_RES_FAILED;
    }
#else
    static struct egis_ioc_transfer xfer = {NULL,        NULL,          0,        FP_SPI_SPEED_HZ,
                                            DELAY_USECS, BITS_PER_WORD, CS_CHANGE};
    xfer.tx_buf = (__u8*)in_buffer;
    xfer.rx_buf = (__u8*)out_buffer;
    xfer.len = (unsigned int)in_buffer_size;
    xfer.opcode = ioctl_code;
    if (pad) {
        xfer.pad[0] = ((__u8*)pad)[0];
        xfer.pad[1] = ((__u8*)pad)[1];
        xfer.pad[2] = ((__u8*)pad)[2];
    }
    if (speed_hz > 0) xfer.speed_hz = (__u32)speed_hz;
    if (ioctl(device_handle, SPI_IOC_MESSAGE(1), &xfer) == -1) return FINGERPRINT_RES_FAILED;
#endif
    return FINGERPRINT_RES_SUCCESS;
}

unsigned int egis_call_driver_ZQ(int device_handle, unsigned int ioctl_code, void* in_buffer,
                                 unsigned int in_buffer_size, void* out_buffer, void* pad,
                                 unsigned int speed_hz) {
    struct ioctl_cmd data;

    (void)in_buffer_size;
    (void)out_buffer;
    (void)pad;
    (void)speed_hz;

    ex_log(LOG_DEBUG, "Beck: (int)(((uint8_t* )in_buffer)[0] = %d",
           (int)(((uint8_t*)in_buffer)[0]));
    data.int_mode = (int)(((uint8_t*)in_buffer)[0]);

    if (-1 == ioctl(device_handle, ioctl_code, &data)) {
        return FINGERPRINT_RES_FAILED;
    }

    return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_open(int* device_handle) {
#ifdef SIMULATE_NO_SENSOR
    return FINGERPRINT_RES_SUCCESS;
#endif
    struct stat st;

    if (NULL == device_handle) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    if (stat(DEVICE_PATH_NAME, &st) != 0) {
        return FINGERPRINT_RES_HW_UNAVALABLE;
    }

    if ((*device_handle = open(DEVICE_PATH_NAME, O_RDWR)) < 0) {
        *device_handle = 0;
        return FINGERPRINT_RES_HW_UNAVALABLE;
    }

    return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_close(int device_handle) {
#ifdef SIMULATE_NO_SENSOR
    return FINGERPRINT_RES_SUCCESS;
#endif

    if (device_handle != 0) {
        int retval = close(device_handle);
        return retval == 0 ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_FAILED;
    }
    return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_reset_set(int device_handle, int high_low) {
#if defined(POWER_CONTRL)
    unsigned char pin_level[1] = {0};
    if (high_low == TRUE) {
        pin_level[0] = 1;
    }
    return egis_call_driver_ZQ((int)device_handle, FP_SENSOR_RESET_SET,
			(void*)pin_level, 1, NULL, NULL, 0);
#else
    return FINGERPRINT_RES_SUCCESS;
#endif
}

unsigned int fp_device_reset(int device_handle) {
#if (SIMULATE_NO_SENSOR || SENSOR_NO_RESET)
    return FINGERPRINT_RES_SUCCESS;
#endif
    if (0 == device_handle) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }
    return egis_call_driver((int)device_handle, FP_SENSOR_RESET, NULL, 0, NULL, NULL, 0);
}
unsigned int fp_device_clock_enable(__unused int device_handle, __unused BOOL enable) {
#ifdef SW_INTERRUPT
    return FINGERPRINT_RES_SUCCESS;
#endif

#ifdef DEVICE_DRIVER_NEED_SPI_ENALBE_DISABLE
    if (enable == TRUE) {
        return egis_call_driver((int)device_handle, FP_SPICLK_ENABLE, NULL, 0, NULL, NULL, 0);
    } else {
        return egis_call_driver((int)device_handle, FP_SPICLK_DISABLE, NULL, 0, NULL, NULL, 0);
    }
#else
    return FINGERPRINT_RES_SUCCESS;
#endif
}

void fp_set_interrupt_trigger_type(int trigger_type) {
#ifdef SW_INTERRUPT
    return;
#endif

    if (trigger_type == TRIGGER_RESET) {
        g_pad = default_pad;
        return;
    }

    if (trigger_type == EDGE_FALLING) {
        g_pad = pad_edge_f;
    } else if (trigger_type == EDGE_RAISING) {
        g_pad = pad_edge_r;
    } else if (trigger_type == LEVEL_LOW) {
        g_pad = pad_level_l;
    } else
        g_pad = default_pad;
}

unsigned int fp_device_interrupt_enable(int device_handle, int cmd) {
    unsigned int ret = 0;
    const unsigned char in_buffer[1] = {0x0}, out_buffer[1] = {0x0};
    ex_log(LOG_DEBUG, "fp_device_interrupt_enable cmd:%d", cmd);

#if (SIMULATE_NO_SENSOR || SW_INTERRUPT)
    return FINGERPRINT_RES_SUCCESS;
#endif

    if (cmd == FLAG_INT_INIT) {
        ret = egis_call_driver((int)device_handle, INT_TRIGGER_INIT, (void*)in_buffer, 1,
                               (void*)out_buffer, (void*)g_pad, 0);
    } else if (cmd == FLAG_INT_ABORT) {
        ret = egis_call_driver((int)device_handle, INT_TRIGGER_ABORT, (void*)in_buffer, 1,
                               (void*)out_buffer, NULL, 0);
    } else if (cmd == FLAG_INT_CLOSE) {
        ret = egis_call_driver((int)device_handle, INT_TRIGGER_CLOSE, (void*)in_buffer, 1,
                               (void*)out_buffer, NULL, 0);
    }

    return ret;
}

unsigned int fp_device_interrupt_wait(int device_handle, unsigned int timeout) {
    struct pollfd pollfds[2];
    int ret;

#if (SIMULATE_NO_SENSOR || SW_INTERRUPT)
    usleep(1000 * 1000);
    return FINGERPRINT_RES_SUCCESS;
#endif
    pollfds[0].fd = (int)device_handle;
    pollfds[0].events = POLLIN;

    ret = poll(pollfds, 1, (int)timeout);
    switch (ret) {
        case 0: {
            return FINGERPRINT_RES_TIMEOUT;
        }
        case -1: {
            return FINGERPRINT_RES_FAILED;
        }
        default: {
            if (pollfds[0].revents & POLLIN) {
                return FINGERPRINT_RES_SUCCESS;
            }
        }
    }

    return FINGERPRINT_RES_TIMEOUT;
}

unsigned int egisFpDeviceFingerOnZero(__unused int handle) {
    return FINGERPRINT_RES_SUCCESS;
}

unsigned int fp_device_power_control(__unused int device_handle, __unused BOOL enable) {
#if defined(PROJECT_AOP_DRIVER) || defined(POWER_CONTRL)
    unsigned char power_mode[1] = {0};
    if (enable == TRUE) {
        power_mode[0] = 1;
    }
    return egis_call_driver_ZQ((int)device_handle, FP_POWER_CONTROL, (void*)power_mode, 1, NULL,
                               NULL, 0);
#else
    return FINGERPRINT_RES_SUCCESS;
#endif
}

unsigned int fp_device_touch_wait(int device_handle) {
    int ret = FINGERPRINT_RES_SUCCESS;
#ifdef __OPLUS__
    const unsigned char in_buffer[1] = {0x0}, out_buffer[1] = {0x0};

    ret = egis_call_driver((int)device_handle, WAIT_TP_TOUCH_DOWN, (void*)in_buffer, 1,
                            (void*)out_buffer, NULL, 0);
#endif
    return ret;
}

unsigned int fp_device_no_touch_wait(int device_handle) {
    int ret = FINGERPRINT_RES_SUCCESS;
#ifdef __OPLUS__
    const unsigned char in_buffer[1] = {0x0}, out_buffer[1] = {0x0};

    ret = egis_call_driver((int)device_handle, WAIT_TP_TOUCH_UP, (void*)in_buffer, 1,
                            (void*)out_buffer, NULL, 0);
#endif
    return ret;
}

unsigned int fp_device_ui_wait(int device_handle) {
    int ret = FINGERPRINT_RES_SUCCESS;
#ifdef __OPLUS__
    const unsigned char in_buffer[1] = {0x0}, out_buffer[1] = {0x0};

    ret = egis_call_driver((int)device_handle, WAIT_UI_READY, (void*)in_buffer, 1,
                            (void*)out_buffer, NULL, 0);
#endif
    return ret;
}

int set_brightness(int level)
{
	if (level == 0)
		return 0;
	int fd, ret, bri_level;
	char bri_write[10] = {0}, bri_val[10] = {0};
	const char *dev_backlight_a50 =
	    "/sys/class/lcd/panel/device/backlight/panel/brightness";
	//a70 a90 use same path
	const char *dev_backlight_a70 =
	    "/sys/class/backlight/panel0-backlight/brightness";
	const char *dev_backlight_astar =
	    "/sys/class/leds/lcd-backlight/brightness";

	fd = open(dev_backlight_a50, O_RDWR);

	if (fd < 0) {
		fd = open(dev_backlight_a70, O_RDWR);
	}

	if (fd < 0) {
		fd = open(dev_backlight_astar, O_RDWR);
	}

	if (fd < 0) {
		ex_log(LOG_ERROR, "%s open fail, fd = %d", __func__, fd);
		return fd;
	}

	ret = read(fd, bri_val, 10);
	if (ret < 0) {
		ex_log(LOG_ERROR, "%s read fail, ret = %d", __func__, ret);
		goto out;
	}

	bri_level = atoi(bri_val);
	ex_log(LOG_DEBUG, "%s level %d -> %d", __func__, bri_level, level);

	sprintf(bri_write, "%d", level);
	ret = write(fd, bri_write, strlen(bri_write));
	if (ret < 0) {
		ex_log(LOG_ERROR, "%s write fail, ret = %d", __func__, ret);
		goto out;
	}

	memset(bri_val, 0x0, 10);
	ret = read(fd, bri_val, 10);
	if (ret < 0) {
		ex_log(LOG_ERROR, "%s read fail, ret = %d", __func__, ret);
		goto out;
	}

	ret = 0;
	bri_level = atoi(bri_val);
	ex_log(LOG_DEBUG, "%s check level %d, %s", __func__, bri_level, bri_val);

out:
	close(fd);
	return ret;
}
