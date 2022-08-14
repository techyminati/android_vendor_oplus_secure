#ifndef __STRUCT_DEF_H__
#define __STRUCT_DEF_H__

#include <stdint.h>

typedef struct dcs_msg {
    uint32_t auth_result;
    uint32_t fail_reason;
    uint32_t quality_score;
    uint32_t match_score;
    uint32_t signal_value;
    uint32_t img_area;
    uint32_t retry_times;
    char algo_version[16];
    uint32_t chip_ic;
    uint32_t factory_id;
    uint32_t module_type;
    uint32_t lense_type;
    uint32_t dsp_availalbe;
} dcs_msg_t;

typedef struct fingerprint_info {
	unsigned int user_id;
	unsigned int fingerprint_id;
} fingerprint_info_t;

typedef struct fingerprint_ids {
	unsigned int fingerprint_ids[__SINGLE_UPPER_LIMITS__];
	unsigned int fingerprint_ids_count;
} fingerprint_ids_t;

typedef struct cache_info {
	unsigned int user_id;
	unsigned int fingerprint_ids[__SINGLE_UPPER_LIMITS__];
	unsigned int fingerprint_ids_count;
	uint64_t authenticator_id;  // Only useful when EX_CMD_GET_AUTH_ID is not supported.
} cache_info_t;

typedef struct fingerprint_enroll_info {
	fingerprint_info_t fingerprint_info;
} fingerprint_enroll_info_t;

typedef struct fingerprint_verify_info {
	unsigned int user_id;
	fingerprint_ids_t fingerprints;
	unsigned int accuracy_level;
	unsigned long long challenge;
	int is_passed;
	dcs_msg_t dcsmsg;
} fingerprint_verify_info_t;

typedef struct fingerprint_remove_info {
	fingerprint_info_t fingerprint_info;
} fingerprint_remove_info_t;

#endif