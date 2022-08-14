#pragma once
#include "type_definition.h"

typedef struct _egis_sensor_type_t {
	union {
		BYTE dev_id0;
		BYTE series;
	};
	union {
		BYTE dev_id1;
		BYTE type;
	};
	BYTE rev_id;
	BYTE pad;
} egis_sensor_type;

#ifdef _AUTOEXT_
#define EXT
#else
#define EXT extern
#endif
EXT egis_sensor_type g_egis_sensortype;
#undef EXT

#define ET5XX_SERIES_ID 5
#define ET6XX_SERIES_ID 6
#define ET510 0x0A  // 580
#define ET511 0x0B
#define ET512 0x0C
#define ET516 0x10
#define ET518 0x12
#define ET520 0x14
#define ET522 0x16
#define ET523 0x17
#define ET601 0x01
#define ET602 0x02
#define ET605 0x05
#define ET613 0x03

#define SENSOR_ID_ET711 711
#define SENSOR_ID_ET729 729
#define SENSOR_ID_ET713 713

#define MAX_IMG_SIZE (255 * 255)
#define MAX_SENSOR_WIDTH (255)

//
//	egis return code
//
#define EGIS_OK 0
#define EGIS_INCORRECT_PARAMETER 1
#define EGIS_OUT_OF_MEMORY 2
#define EGIS_COMMAND_FAIL 3
#define EGIS_NO_DEVICE 4
#define EGIS_RELEASE_DEVICE_FAIL 5
#define EGIS_NOT_CALIBRATION 6
#define EGIS_CALIBRATION_DVR_FAIL 7
#define EGIS_CALIBRATION_DTDVR_FAIL 8
#define EGIS_CALIBRATION_FINGER_ON_FAIL 9
#define EGIS_NEED_TO_REPEAT 10
#define EGIS_INCORRECT_STATUS 11
#define EGIS_WAIT_INTERRUPT 12
#define EGIS_WAIT_INTERRUPT_TIMEOUT 13
#define EGIS_FINGER_NOT_TOUCH 14
#define EGIS_FINGER_TOUCH 15
#define EGIS_FINGER_ON_NOT_STABLE 16
#define EGIS_FINGER_NOT_REMOVED 17
#define EGIS_STEAM_ON_SENSOR 18
#define EGIS_WET_FINGER 19
#define EGIS_PARTIAL_FINGER 20
#define EGIS_INLINETOOL_SENSOR_RESET 21
#define EGIS_RECOVERY 22
#define EGIS_NULL_POINTER 23
#define EGIS_RESET_FAIL 24
#define EGIS_CONTINUE 25
#define EGIS_RETRY_CALIBRATION 26
#define EGIS_TEST_FAIL 27
#define EGIS_CANCEL 28
#define EGIS_VDM_FINGER_PRESENT 29
#define EGIS_NVRAM_CHECKSUM 30
#define EGIS_PERMISSION_DENIED 31
#define EGIS_SECURE_ID_NULL 32
#define EGIS_SECURE_ID_NOTMATCH 33

#define EGIS_ESD_NEED_RESET 99

#define EGIS_TEST_FAIL_DARKDOT (1 << 0)
#define EGIS_TEST_FAIL_WHITEDOT (1 << 1)
#define EGIS_TEST_FAIL_ABNORMAL_AVG_FRAME (1 << 2)
#define EGIS_TEST_FAIL_MANY_BAD_DOT (1 << 3)
#define EGIS_TEST_FAIL_ABNORMAL_FIXED_FRAME (1 << 4)
