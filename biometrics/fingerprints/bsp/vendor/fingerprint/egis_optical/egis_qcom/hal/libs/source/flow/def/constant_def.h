#ifndef __CONST_DEF_H__
#define __CONST_DEF_H__

/* deprecated
#define FINGER_UNKNOW 0
#define FINGER_TOUCH 1
#define FINGER_LEAVE 2
*/

typedef enum verify_accuracy_level_t {
	LEVEL_0 = 0,
	LEVEL_1,
	LEVEL_2,
	LEVEL_3
} verify_accuracy_level_t;

/* deprecated
typedef enum calibration_step {
	CALIBRATION_STEP1,
	CALIBRATION_STEP2
}calibration_step;
*/

/* deprecated
typedef enum process_status_t {
	//image quality, sync with fp_lib_enroll_result_t in fp_algomodule.h
	STATUS_IMG_GOOD						= 0,  //
->FP_LIB_ENROLL_SUCCESS
	STATUS_IMG_WATER					= 2,  //
->FP_LIB_ENROLL_HELP_TOO_WET
	STATUS_IMG_BAD						= 7,  //
->FP_LIB_ENROLL_FAIL_LOW_QUALITY
	STATUS_IMG_PARTIAL					= 8,  //
->FP_LIB_ENROLL_FAIL_LOW_COVERAGE

	//enroll status, sync with fp_lib_enroll_result_t in fp_algomodule.h
	STATUS_ENROLL_OK 					= 0,  //
->FP_LIB_ENROLL_FAIL_LOW_COVERAGE
	STATUS_ENROLL_HIGHLY_SIMILAR		= 1,  //
->FP_LIB_ENROLL_HELP_SAME_AREA
	STATUS_ENROLL_DUPLICATE				= 3,  //
->FP_LIB_ENROLL_HELP_ALREADY_EXIST
	STATUS_ENROLL_TOO_MANY_ATTEMPTS 	= 4,  //
->FP_LIB_ENROLL_TOO_MANY_ATTEMPTS
	STATUS_ENROLL_TOO_MANY_FAILED_ATTEMPTS = 5,  //
->FP_LIB_ENROLL_TOO_MANY_FAILED_ATTEMPTS
	STATUS_ENROLL_FAIL_NONE				= 6,  //
->FP_LIB_ENROLL_FAIL_NONE

	//verify status
	STATUS_VERIFY_MATCH 				= 0,
	STATUS_VERIFY_NOT_MATCH				= -1006,
	STATUS_VERIFY_FEATURE_LOW			= 1,
	STATUS_VERIFY_BAD_QUALITY			= 2,
	STATUS_VERIFY_NOT_IMPLEMENT			= 3,
	STATUS_VERIFY_IMG_TOOSMALL 			= 4,
	STATUS_VERIFY_IMG_ENCRYPT_FAIL		= 5,
	STATUS_VERIFY_LEARNING_FAILED		= 405,
	STATUS_VERIFY_LEARNING_LIMIT		= 406,
	STATUS_VERIFY_ENCRYPT_FAIL 			= 407,
	STATUS_VERIFY_TEMPLATE_TOO_YOUNG	= 408,
	STATUS_VERIFY_NULL_FEATURE			= -1019,

	//finger status,sync with fp_lib_return_t in fp_algomodule.h
	STATUS_FINGER_NOT_REMOVED			= 5,
//FP_LIB_FINGER_PRESENT
	STATUS_FINGER_NOT_TOUCH				= 6,
//FP_LIB_FINGER_LOST
} process_status_t;
*/

#endif
