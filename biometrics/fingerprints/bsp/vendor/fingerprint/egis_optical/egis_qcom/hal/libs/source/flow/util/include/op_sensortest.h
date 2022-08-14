#ifndef __OP_SENSORTEST__
#define __OP_SENSORTEST__

typedef enum {
    SENSORTEST_DIRTYDOTS_TEST = 1100,
    SENSORTEST_READ_REV_TEST = 1101,
    SENSORTEST_REGISTER_RW_TEST = 1102,
    SENSORTEST_CHECK_FINGER_ON = 1103,
    SENSORTEST_GET_IMAGE = 1104,
    SENSORTEST_WAIT_INTERRUTP = 1105,
    SENSORTEST_SET_CROP_INFO = 1106,
    SENSORTEST_GET_NVM_UID = 1107,
    SENSORTEST_START_INTERRUTP = 1108,
    SENSORTEST_TEST_INTERRUTP = 1109,
    SENSORTEST_STOP_INTERRUTP = 1110,
	SENSORTEST_IMAGE_QTY = 1200,
	SENSORTEST_AGING_TEST = 1201
} sensor_test_type;

#define CHECKFINGER_FPIMAGE_LITE 2000
#define CHECKFINGER_QM_EXTRACT 2001

/**
 * @brief sensor_test_opation for ET5XX
 * 
 * @return FINGERPRINT_RES_SUCCESS
 *         FINGERPRINT_RES_FAILED
 */
int sensor_test_opation(int cid, int idev, unsigned char *in_data, int in_data_size,
			unsigned char *buffer, int *buffer_size);
int flow_inline_legacy_cmd(int cmd, int param1, int param2, int param3, unsigned char *out_buf, int *out_size);
int do_7XX_spi_test(int cmd);
int do_7XX_sensortest(int cmd, int param1, int param2, int param3, 
						unsigned char *out_buf, int *out_size);
#endif
