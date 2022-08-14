#ifndef __STRUCT_DEF_H__
#define __STRUCT_DEF_H__

#include <stdint.h>

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
} fingerprint_verify_info_t;

typedef struct fingerprint_remove_info {
	fingerprint_info_t fingerprint_info;
} fingerprint_remove_info_t;

#endif