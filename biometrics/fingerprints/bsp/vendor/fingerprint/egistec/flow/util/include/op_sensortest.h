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
	SENSORTEST_GET_IMAGE_SNR = 1111,
} sensor_test_type;

#define CHECKFINGER_FPIMAGE_LITE 2000
#define CHECKFINGER_QM_EXTRACT 2001

int sensor_test_opation(int cid,int idev, unsigned char *in_data, int in_data_size,
                             unsigned char *buffer, int *buffer_size);

int sensortest_send_cmd(int cmd, unsigned char* out_data, int* out_data_size);
#endif
