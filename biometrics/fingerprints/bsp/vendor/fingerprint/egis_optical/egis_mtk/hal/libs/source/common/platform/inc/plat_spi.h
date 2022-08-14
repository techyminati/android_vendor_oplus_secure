#pragma once

#include <stdint.h>
#include "type_definition.h"

#ifdef __cplusplus
extern "C"
{
#endif
/* For microchip sst */
/* microchip sst communication opcode */
#define MICROCHIP_SST_WRSR 0x01            /* Write-Status-Register */
#define MICROCHIP_SST_PAGE_PROGRAM 0x02    /* To program up to 256 Bytes */
#define MICROCHIP_SST_READ 0x03            /* Read Memory */
#define MICROCHIP_SST_WRDI 0x04            /* Write-Disable */
#define MICROCHIP_SST_RDSR 0x05            /* Read-Status-Register */
#define MICROCHIP_SST_WREN 0x06            /* Write-Enable */
#define MICROCHIP_SST_CHIP_ERASE 0x60      /* Erase Full Memory Array */
#define MICROCHIP_SST_SECTOR_ERASE 0xD7    /* Erase 4 KByle sector */
#define MICROCHIP_SST_BLOCK_ERASE 0xD8     /* Erase 64 KByle block */
#define MICROCHIP_SST_READ_HIGH_SPEED 0x0B /* HIGH_SPEED Read Memory */
#define MICROCHIP_SST_DPD 0xB9             /* Enter deep power down */
#define MICROCHIP_SST_RELEASE_DPD 0xAB     /* RELEASE from deep power down */

#define MICROCHIP_SST_PAGE_SIZE 256	/* MICROCHIP_SST_PAGE_SIZE 256 byte */
#define MICROCHIP_SST_SECTOR_SIZE_4K 4096  /* MICROCHIP_SST_SECTOR_SIZE 4096 4K*/
#define MICROCHIP_SST_BLOCK_SIZE_64K 65536 /* MICROCHIP_SST_BLOCK_SIZE_64K 65536 64K*/

#define MICROCHIP_SST_ADDRESS_SIZE 3
#define MICROCHIP_SST_RW_OFFSET 4 /* OP code 8 bits and address 32bits */
/* The programmed data must be between 1 to 256 Bytes and in whole byte
 * increments; sending less than a full byte will cause the partial byte to
 * be ignored.
 */
#define MICROCHIP_SST_PAGE_PROGRAM_LIMITATION 256
#define MICROCHIP_SST_HIGH_SPEED_READ_DUMMY_LEN 1
/* Wait to write/erase finish */
#define MICROCHIP_SST_STATUS_MAX_RETRY_COUNT 3000

int io_eeprom_rdsr(BYTE *eeprom_status);
int io_eeprom_wrsr(BYTE wrsr);
int io_eeprom_chip_erase();
int io_eeprom_write_enable(int enable);
int io_eeprom_sector_block_erase(BYTE *addr, BYTE is_sector);
int io_eeprom_write(BYTE *addr, int length, BYTE *value);
int io_eeprom_block_write(BYTE *addr, int length, BYTE *pvalue);
int io_eeprom_sector_write(BYTE *addr, int length, BYTE *pvalue);
int io_eeprom_sector_write_anysize(BYTE *addr, int length, BYTE *pvalue);
int io_eeprom_read(BYTE *addr, int length, BYTE *pvalue);
int io_eeprom_read_anysize(BYTE *addr, int length, BYTE *pvalue);
int io_eeprom_high_speed_read(BYTE *addr, int length, BYTE *pvalue);

int io_eeprom_read_id();
int io_release_dpd(BYTE is_rdpd);

int io_dispatch_spi_test(void);
int io_dispatch_connect(HANDLE *device_handle);
int io_dispatch_disconnect(void);
#ifdef _WINDOWS
int io_dispatch_reset();
BOOL set_spi_config(int mode, int speed);
#endif

int io_dispatch_read_register(BYTE address, BYTE *value);
int io_dispatch_write_register(BYTE address, BYTE value);
int io_dispatch_get_frame(BYTE *frame, UINT height, UINT width,
			  UINT number_of_frames);
int io_dispatch_set_tgen(BOOL status);
int io_dispatch_read_eFuse(BYTE *buf, UINT len);

int io_5xx_dispatch_write_register(BYTE address, BYTE value);
int io_5xx_dispatch_read_burst_register(BYTE start_addr, BYTE len, BYTE *pdata);
int io_5xx_dispatch_read_register(BYTE address, BYTE *value);
int ET_5xx_dispatch_write_register(BYTE address, BYTE value);
BYTE ET_5xx_dispatch_read_register(BYTE address, const char *func);
int io_5xx_dispatch_write_reverse_register(BYTE start_addr, BYTE len,
					   BYTE *pvalue);
int io_5xx_dispatch_write_burst_register(BYTE start_addr, BYTE len,
					 BYTE *pvalue);
int io_5xx_dispatch_get_frame(BYTE *frame, UINT height, UINT width, UINT multi,
			      UINT number_of_frames);
int io_5xx_dispatch_get_frame_16bits(unsigned short* frame, UINT width, UINT height, UINT multi,
	UINT number_of_frames);
int polling_registry(BYTE addr, BYTE expect, BYTE mask);
int io_5xx_read_vdm(BYTE *frame, UINT height, UINT width);
int io_5xx_write_vdm(BYTE *frame, UINT height, UINT width);
int io_5xx_read_nvm(BYTE *buffer);
int io_6xx_get_clb(BYTE *buffer, UINT length);
int io_6xx_set_clb(BYTE *buffer, UINT length);

int32_t io_7xx_read_cis_register(uint16_t addr, uint8_t *buf);
int32_t io_7xx_write_cis_register(uint16_t addr, uint8_t value);
int32_t io_7xx_dispatch_pre_capture();
int32_t __io_7xx_dispatch_get_cis_frame_sector(uint32_t sector, uint32_t size,
					       uint8_t *fb);
int32_t io_7xx_dispatch_get_cis_frame(uint32_t size, uint8_t *fb);

//
int32_t io_7xx_dispatch_read_register(uint16_t addr, uint8_t *value);
int32_t io_7xx_dispatch_write_register(uint16_t addr, uint8_t value);
int32_t io_7xx_dispatch_get_frame(uint8_t* buffer, uint32_t length, uint32_t frames);
int32_t io_7xx_dispatch_sensor_select(uint16_t sensor_sel);

#ifdef __cplusplus
}
#endif

enum io_dispatch_cmd {
	IOCMD_READ_ZONE_AVERAGE = 700,
	IOCMD_READ_HISTOGRAM,
	IOCMD_READ_EFUSE,
};

int32_t io_dispatch_command_read(enum io_dispatch_cmd cmd, int param1, int param2, uint8_t* out_buf, int* out_buf_size);

#ifdef NORMAL_WORLD
__inline int io_dispatch_call_driver(UINT ioctl_code, void *in_buffer,
				     UINT in_buffer_size, void *out_buffer,
				     UINT out_buffer_size,
				     UINT *bytes_returned);

__inline int io_dispatch_call_driver_ex(UINT ioctl_code, void *in_buffer,
					UINT in_buffer_size, void *out_buffer,
					UINT out_buffer_size,
					UINT *bytes_returned, void *pad);

#endif

// linux_driver.h
#ifndef _WINDOWS
#ifndef TZ_MODE
#include <asm/ioctl.h>
#endif
#endif
//
//	Driver IO control code
//
#define FP_REGISTER_READ 0x01
#define FP_REGISTER_WRITE 0x02
#define FP_GET_ONE_IMG 0x03
#define FP_SENSOR_RESET 0x04
#define FP_POWER_CONTROL 0x05
#define FP_SET_SPI_CLOCK 0x06
#define FP_RESET_SET 0x07

#define FP_DIABLE_SPI_CLOCK 0x10
#define FP_CPU_SPEEDUP 0x11

#define FP_505_REGISTER_READ 0x50
#define FP_505_REGISTER_WRITE 0x51
#define FP_505_REG_BURST_READ 0x52
#define FP_505_REG_BURST_WRITE 0x53
#define FP_505_GET_ONE_IMG 0x54

#define FP_TRANSFER_SYNC 0xAA

//
// Interrupt trigger routine
//
#define INT_TRIGGER_INIT 0xa4   // trigger signal initial routine
#define INT_TRIGGER_CLOSE 0xa5  // trigger signal close routine
#define INT_POLLING_ABORT 0xa8  // abort trigger

#ifndef _WINDOWS
#ifndef TZ_MODE
struct egis_ioc_transfer {
	__u8 *tx_buf;
	__u8 *rx_buf;
	__u32 len;
	__u32 speed_hz;
	__u16 delay_usecs;
	__u8 bits_per_word;
	__u8 cs_change;
	__u8 opcode;
	__u8 pad[3];
	//
	//  __u32		pad;
	//
	//
	/* If the contents of 'struct spi_ioc_transfer' ever change
	 * incompatibly, then the ioctl number (currently 0) must change;
	 * ioctls with constant size fields get a bit more in the way of
	 * error checking than ones (like this) where that field varies.
	 *
	 * NOTE: struct layout is the same in 64bit and 32bit userspace.
	 */
	//
};

#define SPI_IOC_MAGIC 'k'
#define SPI_MSGSIZE(N)                                                       \
	((((N) * (sizeof(struct egis_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
	     ? ((N) * (sizeof(struct egis_ioc_transfer)))                    \
	     : 0)
#define SPI_IOC_MESSAGE(N) _IOW(SPI_IOC_MAGIC, 0, char[SPI_MSGSIZE(N)])
#endif
#endif

typedef struct {
	void *buf_addr;
	/**<buff address for read or write data*/
	unsigned int buf_len;
	/**<size in bytes*/
	unsigned int total_len;
	/**<total bytes successfully transfered on SPI Bus*/
} sec_tzspi_transfer_t;

/**
 * Open the device, and perform some HW intialization.
 *
 *
 * @return 0 on success, negative on failure
 */
extern int sec_tzspi_open(void);

/**
 * Reads data from SPI bus.
 *
 * @param[in] p_read_info Read buffer information
 *
 * @return 0 on success, negative on failure
 */
extern int sec_tzspi_read(sec_tzspi_transfer_t *p_read_info);

/**
 * Writes data on SPI bus.
 *
 * @param[in] p_write_info Write buffer information
 *
 * @return 0 on success, negative on failure
 */
extern int sec_tzspi_write(sec_tzspi_transfer_t *p_write_info);

/**
 * Writes data on SPI bus.
 *
 * @param[in] p_write_info Write buffer information
 * @param[in] p_read_info Read buffer information
 *
 * @return 0 on success, negative on failure
 */
extern int sec_tzspi_full_duplex(sec_tzspi_transfer_t *p_write_info,
				 sec_tzspi_transfer_t *p_read_info);

/**
 * Close the client access to the spi device.
 *
 *
 *
 * @return 0 on success, negative on failure
 */
extern int sec_tzspi_close(void);

typedef struct {
	void *buf_addr;
	uint32_t buf_len;
	uint32_t total_len;
} bsp_tzspi_transfer_t;
