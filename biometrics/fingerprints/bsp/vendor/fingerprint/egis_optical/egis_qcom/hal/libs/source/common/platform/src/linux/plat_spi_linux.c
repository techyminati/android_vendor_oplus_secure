#ifndef __OTG_SENSOR__

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>

#include "egis_definition.h"
#include "plat_log.h"
#include "plat_heap.h"
#include "plat_spi.h"

#define DEVICE_PATH_NAME "/dev/esfp0"
#define SPEED_HZ 14000000
#define DELAY_USECS 0
#define BITS_PER_WORD 8
#define CS_CHANGE 1

HANDLE g_device_handle = 0;

#define LOG_TAG "RBS-EGISFP"

#if defined(__ET7XX__) && !defined(HIKEY960_EVB)
#define SPI_ADDR_REG_KEY (0xDF)
#define SPI_ADDR_PIX_ID (0xFC)

#define PIX_ID_ET713C (0x04)
#define PIX_ID_ET713D (0x05)

static int __power_control(int fd, int status);
static int __read_register(int fd, uint8_t addr, uint8_t *d);
static int __write_register(int fd, uint8_t addr, uint8_t d);
static int __get_image_buffer(int fd, uint8_t *buf, int size);
static int __set_max_clock(int fd, int status);
static int __get_pix_id(uint8_t* p_pix_id);
#endif

int io_dispatch_spi_test()
{
	int ret = 0;
	return ret;
}

BOOL g_is_device_open = FALSE;

int io_dispatch_connect(HANDLE *device_handle)
{
	struct stat st;
	if (g_is_device_open) return EGIS_OK;

	if (stat(DEVICE_PATH_NAME, &st) != 0) return EGIS_NO_DEVICE;

	if (g_device_handle > 0) {
		if (device_handle) *device_handle = g_device_handle;
		return EGIS_OK;
	}

	if ((g_device_handle = open(DEVICE_PATH_NAME, O_RDWR)) < 0) {
		g_device_handle = 0;
		if (device_handle) *device_handle = 0;
		return EGIS_NO_DEVICE;
	}
#if defined(__ET7XX__) && !defined(HIKEY960_EVB)
	__power_control(g_device_handle, 1);
	__set_max_clock(g_device_handle, 1);
#endif
	if (device_handle) *device_handle = g_device_handle;
	g_is_device_open = TRUE;
	return EGIS_OK;
}

int io_dispatch_disconnect(void)
{
	if (!g_is_device_open) return EGIS_OK;
	if (g_device_handle != 0) close(g_device_handle);
	g_device_handle = 0;
	g_is_device_open = FALSE;
	return EGIS_OK;
}

#ifdef DEVICE_DRIVER_HAS_SPI
int io_dispatch_call_driver(UINT ioctl_code, void *in_buffer,
			    UINT in_buffer_size, void *out_buffer,
			    UINT out_buffer_size, UINT *bytes_returned)
{
	static struct egis_ioc_transfer command = {
	    NULL, NULL, 0, SPEED_HZ, DELAY_USECS, BITS_PER_WORD, CS_CHANGE};
	command.tx_buf = (__u8 *)in_buffer;
	command.rx_buf = (__u8 *)out_buffer;
	command.len = (UINT)in_buffer_size;
	command.opcode = (__u8)ioctl_code;
	if (ioctl(g_device_handle, SPI_IOC_MESSAGE(1), &command) == -1) {
		egislog_e("%s fail", __func__);
		egislog_e("errno= %d", errno);
		return EGIS_COMMAND_FAIL;
	}
	return EGIS_OK;
}

int io_dispatch_read_register(BYTE address, BYTE *value)
{
	if (!value) return EGIS_COMMAND_FAIL;
	return io_dispatch_call_driver(FP_REGISTER_READ, (void *)&address, 1,
				       (void *)value, 1, NULL);
}

#define WRITE_REGISTER_COMMAND_SIZE 2
int io_dispatch_write_register(BYTE address, BYTE value)
{
	static BYTE in_buffer[WRITE_REGISTER_COMMAND_SIZE];
	in_buffer[0] = address;
	in_buffer[1] = value;
	return io_dispatch_call_driver(FP_REGISTER_WRITE, (void *)in_buffer,
				       WRITE_REGISTER_COMMAND_SIZE, 0, 0, NULL);
}
int io_dispatch_transfer_sync(void *in, void *out, unsigned int t_num)
{
	static struct egis_ioc_transfer command = {
	    NULL, NULL, 0, SPEED_HZ, DELAY_USECS, BITS_PER_WORD, CS_CHANGE};
	command.tx_buf = (__u8 *)in;
	command.rx_buf = (__u8 *)out;
	command.len = (UINT)t_num;
	command.opcode = FP_TRANSFER_SYNC;
	if (ioctl(g_device_handle, SPI_IOC_MESSAGE(1), &command) == -1) {
		egislog_e("%s fail", __func__);
		egislog_e("errno= %d", errno);
		return EGIS_COMMAND_FAIL;
	}
	return EGIS_OK;
}

__inline int io_dispatch_get_frame(BYTE *frame, UINT height, UINT width,
				   UINT number_of_frames)
{
#define GET_FRAME_COMMAND_SIZE 6
	static BYTE in_buffer[GET_FRAME_COMMAND_SIZE];
	int ret, get_frame_result;
	UINT size = height * width * number_of_frames;
	if (!frame || size < 1) return EGIS_COMMAND_FAIL;

	//
	//	Enable TGEN
	//
	ret = io_dispatch_set_tgen(TRUE);
	if (ret != EGIS_OK) return ret;

	in_buffer[0] = height;
	in_buffer[1] = width * number_of_frames;
	get_frame_result = io_dispatch_call_driver(
	    FP_GET_ONE_IMG, (void *)in_buffer, GET_FRAME_COMMAND_SIZE,
	    (void *)frame, size, NULL);
	//
	//	Diaable TGEN
	//
	ret = io_dispatch_set_tgen(FALSE);
	if (ret != EGIS_OK && get_frame_result == EGIS_OK) return ret;
	return get_frame_result;
}
#endif

int io_dispatch_call_driver_ex(UINT ioctl_code, void *in_buffer,
			       UINT in_buffer_size, void *out_buffer,
			       UINT out_buffer_size, UINT *bytes_returned,
			       void *pad)
{
	static struct egis_ioc_transfer command = {
	    NULL, NULL, 0, SPEED_HZ, DELAY_USECS, BITS_PER_WORD, CS_CHANGE};
	command.tx_buf = (__u8 *)in_buffer;
	command.rx_buf = (__u8 *)out_buffer;
	command.len = (UINT)in_buffer_size;
	command.opcode = (__u8)ioctl_code;
	command.pad[0] = ((__u8 *)pad)[0];
	command.pad[1] = ((__u8 *)pad)[1];
	command.pad[2] = ((__u8 *)pad)[2];
	if (ioctl(g_device_handle, SPI_IOC_MESSAGE(1), &command) == -1)
		return EGIS_COMMAND_FAIL;
	return EGIS_OK;
}

#define ET5XX_WRITE_OPCODE 0x24
#define ET5XX_READ_OPCODE 0x20
#define ET5XX_BURST_READ_OPCODE 0x22
#define ET5XX_BURST_WRITE_OPCODE 0x26
#define ET5XX_REVERSE_WRITE_OPCODE 0x27
#define ET5XX_GET_IMAGE_OPCODE 0x50

#define ET5_SERIES_WRITE_OP ET5XX_WRITE_OPCODE
#define ET5_SERIES_BURST_WRITE_OP ET5XX_BURST_WRITE_OPCODE
#define ET5_SERIES_REVERSE_WRITE_OP ET5XX_REVERSE_WRITE_OPCODE
#define ET5_SERIES_READ_OP ET5XX_READ_OPCODE
#define ET5_SERIES_BURST_READ_OP ET5XX_BURST_READ_OPCODE
#define ET5_SERIES_GET_IMAGE_OP ET5XX_GET_IMAGE_OPCODE

#ifdef DEVICE_DRIVER_HAS_SPI
int io_dispatch_set_tgen(BOOL enable) { return 0; }
#endif

int io_dispatch_read_eFuse(BYTE *buf, UINT len)
{
	return io_dispatch_call_driver(0x10, NULL, len, (void *)buf, len, NULL);  //  FP_EFUSE_READ == 0x10
}

#define OPCODE_5XX_SIZE 1

int io_5xx_dispatch_write_burst_register(BYTE start_addr, BYTE len,
					 BYTE *pvalue);
int io_5xx_dispatch_write_register(BYTE address, BYTE value)
{
	return io_5xx_dispatch_write_burst_register(address, 1, &value);
}

#define BURST_5XX_RD_WR_REG_SIZE 64
int io_5xx_dispatch_read_burst_register(BYTE start_addr, BYTE len, BYTE *pdata)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = WRITE_REGISTER_COMMAND_SIZE;
	int total_size = in_len + len;
	BYTE in_buffer[WRITE_REGISTER_COMMAND_SIZE] = {0};
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	in_buffer[0] = ET5_SERIES_BURST_READ_OP;
	in_buffer[1] = start_addr;

	tx.buf_addr = in_buffer;
	tx.buf_len = in_len;

	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	// egislog_d("%s, in_len = %d",__func__,in_len);
	// egislog_d("%s, len = %d",__func__,len);
	// egislog_d("%s, tx.buf_len = %d",__func__,tx.buf_len);
	// egislog_d("%s, rx.buf_len = %d",__func__,rx.buf_len);

	// int i =0;
	// for(i=0;i<tx.buf_len;i++)
	// egislog_d("%s, tx_buf[%d] = 0x%x",__func__, i,
	// *(BYTE*)(tx.buf_addr+i));

	// egislog_d("%s,>>> tx_buf= %p",__func__, tx.buf_addr);
	// egislog_d("%s,>>> rx_buf= %p",__func__, rx.buf_addr);
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
	// egislog_d("%s,<<< tx_buf= %p",__func__, tx.buf_addr);
	// egislog_d("%s,>>> rx_buf= %p",__func__, rx.buf_addr);
	// egislog_d("%s,>>> out_buf= %p",__func__, out_buffer);

	// for(i=0;i<rx.buf_len;i++)
	// egislog_d("%s, rx_buf[%d] = 0x%x",__func__, i,
	// *(BYTE*)(rx.buf_addr+i));
	memcpy(pdata, out_buffer + in_len, len);
	plat_free(out_buffer);
	return ret;
}

/*
 * Read Register workaround.
 */
int io_5xx_dispatch_read_register(BYTE address, BYTE *value)
{
	if (!value) return EGIS_COMMAND_FAIL;
	return io_5xx_dispatch_read_burst_register(address, 1, value);
}

int ET_5xx_dispatch_write_register(BYTE address, BYTE value)
{
	return io_5xx_dispatch_write_register(address, value);
}

BYTE ET_5xx_dispatch_read_register(BYTE address, const char *func)
{
	BYTE value = 0;
	int ret = io_5xx_dispatch_read_register(address, &value);
	if (ret != EGIS_OK) {
		egislog_e("%s, addr = 0x%x, ret = %d", func, address, ret);
	}
	return value;
}

int io_5xx_dispatch_write_reverse_register(BYTE start_addr, BYTE len,
					   BYTE *pvalue)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = WRITE_REGISTER_COMMAND_SIZE;
	int total_size = in_len + len;
	BYTE in_buffer[BURST_5XX_RD_WR_REG_SIZE] = {0};

	in_buffer[0] = ET5_SERIES_REVERSE_WRITE_OP;
	in_buffer[1] = start_addr;
	memcpy(&in_buffer[2], pvalue, len);

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;

	rx.buf_addr = in_buffer;
	rx.buf_len = total_size;
	return io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
}

int io_5xx_dispatch_write_burst_register(BYTE start_addr, BYTE len,
					 BYTE *pvalue)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = WRITE_REGISTER_COMMAND_SIZE;
	int total_size = in_len + len;
	BYTE in_buffer[BURST_5XX_RD_WR_REG_SIZE] = {0};

	in_buffer[0] = ET5_SERIES_BURST_WRITE_OP;
	in_buffer[1] = start_addr;
	memcpy(&in_buffer[2], pvalue, len);

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;

	rx.buf_addr = in_buffer;
	rx.buf_len = total_size;
	return io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
}

int io_5xx_dispatch_get_frame(BYTE *frame, UINT height, UINT width, UINT multi,
			      UINT number_of_frames)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = 1;
	BYTE in_buffer[1] = {ET5_SERIES_GET_IMAGE_OP};
	int total_size = sizeof(in_buffer) + height * width * number_of_frames;
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	in_buffer[0] = ET5_SERIES_GET_IMAGE_OP;

	tx.buf_addr = in_buffer;
	tx.buf_len = 1;

	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
	// memcpy(frame, out_buffer+in_len, height*width);
	memcpy(frame, out_buffer + in_len, height * width * number_of_frames);
	plat_free(out_buffer);
	return ret;
}

int io_5xx_dispatch_get_frame_16bits(unsigned short* frame, UINT width, UINT height, UINT multi,
			      UINT number_of_frames)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = 1;
	BYTE in_buffer[1] = {ET5_SERIES_GET_IMAGE_OP};
	int total_size =
	    sizeof(in_buffer) + height * width * 2 * number_of_frames;
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	in_buffer[0] = ET5_SERIES_GET_IMAGE_OP;

	tx.buf_addr = in_buffer;
	tx.buf_len = 1;

	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);

	memcpy(frame, out_buffer + in_len,
	       height * width * (sizeof(unsigned short)) * number_of_frames);
	plat_free(out_buffer);
	return ret;
}
int io_6xx_dispatch_get_frame_16(unsigned short *frame, UINT height, UINT width,
				 UINT number_of_frames)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = 1;
	BYTE in_buffer[1] = {ET5_SERIES_GET_IMAGE_OP};
	int total_size =
	    sizeof(in_buffer) + height * width * 2 * number_of_frames;
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	in_buffer[0] = ET5_SERIES_GET_IMAGE_OP;

	tx.buf_addr = in_buffer;
	tx.buf_len = 1;

	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);

	memcpy(frame, out_buffer + in_len,
	       height * width * (sizeof(unsigned short)) * number_of_frames);
	plat_free(out_buffer);
	return ret;
}


int polling_registry(BYTE addr, BYTE expect, BYTE mask)
{
	int i;
	BYTE value;
	// egislog_d("%s",__func__);
	for (i = 0; i < 3000; i++) {
		if (io_5xx_dispatch_read_register(addr, &value) != EGIS_OK)
			return EGIS_COMMAND_FAIL;
		if ((value & mask) == expect) return EGIS_OK;
	}
	egislog_e("%s, value&mask = 0x%x, expect = 0x%x", __func__,
		  value & mask, expect);

	return EGIS_COMMAND_FAIL;
}

int io_5xx_read_vdm(BYTE *frame, UINT height, UINT width)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = 1;
	BYTE in_buffer[1] = {0x60};
	int total_size = sizeof(in_buffer) + height * width;
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	tx.buf_addr = in_buffer;
	tx.buf_len = 1;

	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
	memcpy(frame, out_buffer + in_len, height * width);
	plat_free(out_buffer);
	return ret;
}

int io_5xx_write_vdm(BYTE *frame, UINT height, UINT width)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int ret = 0;
	int total_size = 1 + height * width;
	BYTE *in_buffer = (BYTE *)plat_alloc(total_size);

	if (in_buffer == NULL) {
		egislog_e("memory allocation fail");
		return EGIS_OUT_OF_MEMORY;
	}

	memcpy(in_buffer + 1, frame, height * width);

	in_buffer[0] = 0x62;

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;

	rx.buf_addr = in_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
	plat_free(in_buffer);
	return ret;
}
int io_6xx_get_clb(BYTE *buffer, UINT length)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = 1;
	BYTE in_buffer[1] = {0x64};
	int total_size = sizeof(in_buffer) + length;
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	tx.buf_addr = in_buffer;
	tx.buf_len = 1;

	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
	memcpy(buffer, out_buffer + in_len, length);
	plat_free(out_buffer);
	return ret;
}

int io_6xx_set_clb(BYTE *buffer, UINT length)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int ret = 0;
	int total_size = 1 + length;
	BYTE *in_buffer = (BYTE *)plat_alloc(total_size);

	if (in_buffer == NULL) {
		egislog_e("memory allocation fail");
		return EGIS_OUT_OF_MEMORY;
	}

	memcpy(in_buffer + 1, buffer, length);

	in_buffer[0] = 0x66;

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;

	rx.buf_addr = in_buffer;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);
	plat_free(in_buffer);
	return ret;
}

#define CMD_NV_EN 0x44
#define CMD_NV_RD1 0x40
#define CMD_NV_RD2 0x00
#define CMD_NV_DIS 0x48
#define CMD_DUMMY 0x00
#define NVM_LEN 64
#define NVM_COMMAND_SIZE 3

int io_5xx_read_nvm(BYTE *buffer)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int in_len = NVM_COMMAND_SIZE;
	int total_size = in_len + NVM_LEN;
	BYTE in_buffer[NVM_COMMAND_SIZE] = {0};
	BYTE *out_buffer = (BYTE *)plat_alloc(total_size);
	int ret = 0;

	in_buffer[0] = CMD_NV_EN;
	in_buffer[1] = CMD_DUMMY;
	in_buffer[2] = CMD_DUMMY;
	tx.buf_addr = in_buffer;
	rx.buf_addr = out_buffer;
	tx.buf_len = 2;
	rx.buf_len = 2;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);

	in_buffer[0] = CMD_NV_RD1;
	in_buffer[1] = CMD_NV_RD2;
	in_buffer[2] = CMD_DUMMY;
	tx.buf_len = in_len;
	rx.buf_len = total_size;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);

	in_buffer[0] = CMD_NV_DIS;
	in_buffer[1] = CMD_DUMMY;
	in_buffer[2] = CMD_DUMMY;
	tx.buf_len = 2;
	rx.buf_len = 2;
	ret = io_dispatch_transfer_sync((void *)&tx, (void *)&rx, 1);

	memcpy(buffer + in_len, out_buffer + in_len, NVM_LEN);

	plat_free(out_buffer);
	return ret;
}
#ifdef __ET7XX__
#ifdef HIKEY960_EVB

#define DUMMY (0xff)
#define ET7xx_SPEED_HZ_3M (3 * 1000 * 1000)
#define ET7xx_SPEED_HZ_5M (5 * 1000 * 1000)
#define ET7xx_SPEED_HZ_6M (6 * 1000 * 1000)
#define ET7xx_SPEED_HZ_12M (12 * 1000 * 1000)
#define ET7xx_SPEED ET7xx_SPEED_HZ_12M

#define ET7xx_OP_RDREG (0x20)
#define ET7xx_OP_WRREG (0x24)
#define ET7xx_OP_GET_FRAME (0x50)
#define ET7xx_OP_SENSOR_SELECT (0xFD)

static int io_7xx_dispatch_transfer_sync(void *in, void *out, int speed_hz, unsigned int t_num)
{
	static struct egis_ioc_transfer command;

	command.tx_buf = (__u8 *)in;
	command.rx_buf = (__u8 *)out;
	command.len = (UINT)t_num;
	command.speed_hz = (__u32)speed_hz;
	command.delay_usecs = DELAY_USECS;
	command.bits_per_word = BITS_PER_WORD;
	command.cs_change = CS_CHANGE;
	command.opcode = FP_TRANSFER_SYNC;
	if (ioctl(g_device_handle, SPI_IOC_MESSAGE(1), &command) == -1) {
		printf("%s fail", __func__);
		printf("errno= %d", errno);
		return EGIS_COMMAND_FAIL;
	}
	return EGIS_OK;
}

int32_t io_7xx_dispatch_read_register(uint16_t addr, uint8_t *val)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int ret = 0, total_size = 3;
	uint8_t in_buffer[total_size];
	uint8_t out_buffer[total_size];

	in_buffer[0] = ET7xx_OP_RDREG;
	in_buffer[1] = (uint8_t)addr;
	in_buffer[2] = DUMMY;

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;
	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_7xx_dispatch_transfer_sync((void *)&tx, (void *)&rx, ET7xx_SPEED, 1);
	*val = ((uint8_t *)rx.buf_addr)[2];
	return ret;
}

int32_t io_7xx_dispatch_write_register(uint16_t addr, uint8_t val)
{
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	int ret = 0, total_size = 3;
	uint8_t in_buffer[total_size];
	uint8_t out_buffer[total_size];

	in_buffer[0] = ET7xx_OP_WRREG;
	in_buffer[1] = (uint8_t)addr;
	in_buffer[2] = val;

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;
	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_7xx_dispatch_transfer_sync((void *)&tx, (void *)&rx, ET7xx_SPEED, 1);
	return ret;
}

int32_t io_7xx_dispatch_get_frame(uint8_t *buffer, uint32_t length, uint32_t frames)
{
	int total_size = length + 1;
	int ret;
	bsp_tzspi_transfer_t tx;
	bsp_tzspi_transfer_t rx;
	uint8_t *in_buffer = (uint8_t *)plat_alloc(total_size);
	uint8_t *out_buffer = (uint8_t *)plat_alloc(total_size);

	memset(in_buffer, DUMMY, total_size);
	in_buffer[0] = ET7xx_OP_GET_FRAME;

	tx.buf_addr = in_buffer;
	tx.buf_len = total_size;
	rx.buf_addr = out_buffer;
	rx.buf_len = total_size;
	ret = io_7xx_dispatch_transfer_sync((void *)&tx, (void *)&rx, ET7xx_SPEED, 1);

	memcpy(buffer, &((uint8_t *)rx.buf_addr)[1], length);
	plat_free(in_buffer);
	plat_free(out_buffer);
	return ret;
}
#else

/** CS1 NON-TZ Driver Start **/

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define FP_REGISTER_READ 0x01
#define FP_REGISTER_WRITE 0x02
#define FP_SET_SPI_CLOCK 0x06
#define FP_GET_IMG 0x12
#define FP_GET_ZAVG 0x14
#define MAX_SPI_CLOCK 20*1000*1000 //20Mhz for a50,a8 device.
#define MAX_SPI_CLOCK_ET713CD 25*1000*1000 //25Mhz for ET713CD sensor

static int __get_pix_id(uint8_t* p_pix_id)
{
	__write_register(g_device_handle, SPI_ADDR_REG_KEY, 0x88);
	__read_register(g_device_handle, SPI_ADDR_PIX_ID, p_pix_id);
	__write_register(g_device_handle, SPI_ADDR_REG_KEY, 0x00);
	return 0;
}

static int __set_max_clock(int fd, int status)
{
	struct egis_ioc_transfer xfer = {
	    .len = status,
	    .opcode = FP_SET_SPI_CLOCK,
		.speed_hz = (__u32) MAX_SPI_CLOCK,
	};
	uint8_t pix_id = 0;
	__get_pix_id(&pix_id);
	if (pix_id == PIX_ID_ET713C || pix_id == PIX_ID_ET713D) {
		xfer.speed_hz = (__u32)MAX_SPI_CLOCK_ET713CD;
	}
	return ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
}

static int __power_control(int fd, int status)
{
	struct egis_ioc_transfer xfer = {
	    .len = status,
	    .opcode = FP_POWER_CONTROL,
	};
	return ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
}

static int __read_register(int fd, uint8_t addr, uint8_t *d)
{
	int ret;
	*d = 0;
	struct egis_ioc_transfer xfer = {
	    .tx_buf = &addr,
	    .rx_buf = d,
	    .len = 1,
	    .opcode = FP_REGISTER_READ,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
	if (ret < 0) {
		ex_log(LOG_ERROR, "ERROR,  %s, reason: [ %s (%d) ]\n", __func__, strerror(errno), errno);
		return -errno;
	}
	return ret;
}

static int __write_register(int fd, uint8_t addr, uint8_t d)
{
	int ret;
	uint8_t tx[] = {addr, d};
	struct egis_ioc_transfer xfer = {
	    .tx_buf = tx,
	    .len = ARRAY_SIZE(tx),
	    .opcode = FP_REGISTER_WRITE,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
	if (ret < 0) ex_log(LOG_ERROR, "ERROR,  %s, reason: [ %s (%d) ]\n", __func__, strerror(errno), errno);
	return ret;
}

static int __get_image_buffer(int fd, uint8_t *buf, int size)
{
	int ret;
	struct egis_ioc_transfer xfer = {
	    .rx_buf = buf,
	    .len = size,
	    .opcode = FP_GET_IMG,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
	if (ret < 0) ex_log(LOG_ERROR, "ERROR,  %s, reason: [ %s (%d) ]\n", __func__, strerror(errno), errno);
	return ret;
}

static int __get_zone_average(int fd, uint8_t *buf){
	int ret;
	struct egis_ioc_transfer xfer = {
		.rx_buf = buf,
		.len = 72,
		.opcode = FP_GET_ZAVG,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
	if (ret < 0)
		printf("ERROR,  %s, reason: [ %s (%d) ]\n", __func__, strerror(errno), errno);
	return ret;
}

int32_t io_7xx_dispatch_read_register(uint16_t addr, uint8_t *val)
{
	__read_register(g_device_handle, (uint8_t)addr, val);
	return 0;
}

int32_t io_7xx_dispatch_write_register(uint16_t addr, uint8_t val)
{
	__write_register(g_device_handle, (uint8_t)addr, val);
	return 0;
}

int32_t io_7xx_dispatch_get_frame(uint8_t *buffer, uint32_t length, uint32_t frames)
{
	__get_image_buffer(g_device_handle, buffer, length);
	return 0;
}

int32_t io_dispatch_command_read(enum io_dispatch_cmd cmd, int param1, int param2, uint8_t* out_buf, int* out_buf_size)
{
	switch (cmd) {
		case IOCMD_READ_ZONE_AVERAGE:
			__get_zone_average(g_device_handle, out_buf);
			break;

		case IOCMD_READ_HISTOGRAM:
		case IOCMD_READ_EFUSE:
			egislog_e("%s [%d] not supported yet", __func__, cmd);
			break;

		default:
			egislog_e("%s [%d] not supported", __func__, cmd);
			break;
	}
	return EGIS_OK;
}

int32_t io_7xx_dispatch_sensor_select(uint16_t sensor_sel)
{
	egislog_e("%s not supported yet", __func__);
	return EGIS_OK;
}

int io_eeprom_sector_write_anysize(BYTE* addr, int length, BYTE* pvalue) {
	egislog_e("%s not supported flash", __func__);
    return 0;
}
int io_eeprom_read_anysize(BYTE* addr, int length, BYTE* pvalue) {
	egislog_e("%s not supported flash", __func__);
	//not support
    return 0;
}

/** CS1 NON-TZ Driver END **/
#endif
#endif
#endif  // __OTG_SENSOR__
