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
