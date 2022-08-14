#ifndef __STRUCT_DEF_H__
#define __STRUCT_DEF_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char* key;
    char value[64];
} key_value_t;

typedef struct bd_fp_verify_info {
    uint8_t auth_result;
    uint8_t fail_reason;
    int quality_score;
    int match_score;
    int signal_value;
    int img_area;
    uint8_t retry_times;
    int algo_version;
    uint16_t chip_ic;
    uint8_t module_type;
    uint8_t lens_type;
    int dsp_available;
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
    bool is_redundant;
    int enroll_retry_count;
} fingerprint_enroll_info_t;

typedef struct fingerprint_verify_info {
    unsigned int user_id;
    fingerprint_ids_t fingerprints;
    unsigned int accuracy_level;
    unsigned long long challenge;
    unsigned int matched_fingerprint_id;
    int flow_try_match;
    int bad_qty_threshold;
    int is_passed;
    dcs_msg_t dcsmsg;
} fingerprint_verify_info_t;

typedef struct fingerprint_remove_info {
    fingerprint_info_t fingerprint_info;
} fingerprint_remove_info_t;

#endif