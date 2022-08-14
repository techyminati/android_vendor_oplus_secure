#ifndef  _GF_FPCORE_TYPES_H_
#define  _GF_FPCORE_TYPES_H_

#include "gf_base_types.h"


typedef enum {
    GF_CMD_AUTH_SETACTIVITY_GROUP,
    GF_CMD_AUTH_AUTHENTICATE,
    GF_CMD_AUTH_PRE_ENROLL,
    GF_CMD_AUTH_ENROLL,
    GF_CMD_AUTH_POST_ENROLL,
    GF_CMD_AUTH_GET_ID,
    GF_CMD_AUTH_REMOVE,
    GF_CMD_AUTH_ENUMERATE,
    GF_CMD_AUTH_CANCEL,
    GF_CMD_AUTH_SAVE_TEMPLATES,
    GF_CMD_AUTH_SET_HMAC_KEY,
    GF_CMD_AUTH_SCREEN_STATE,
    GF_CMD_AUTH_POST_AUTHENTICATE,
    GF_CMD_AUTH_INIT_FINISHED,
    GF_CMD_AUTH_GENERATE_CHALLENGE,
    GF_CMD_AUTH_REVOKE_CHALLENGE,
    GF_CMD_AUTH_INVALIDATE_AUTH_ID,
    GF_CMD_AUTH_AUTHENTICATE_END,
    GF_CMD_AUTH_RESET_LOCKOUT,
    GF_CMD_AUTH_MAX = 100,
} gf_auth_cmd_id_t;

typedef gf_cmd_header_t gf_post_enroll_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_group_id;
    uint32_t i_finger_id;
    uint32_t o_removing_templates;
    uint32_t o_deleted_fids[MAX_FINGERS_PER_USER];
    uint32_t o_deleted_gids[MAX_FINGERS_PER_USER];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_remove_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_group_id;
    uint8_t i_path[MAX_FILE_ROOT_PATH_LEN];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_set_active_group_t;

typedef struct {
    gf_cmd_header_t header;
    // response data
    uint64_t o_challenge;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_pre_enroll_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    // response data
    uint64_t o_challenge;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_gen_challenge_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint64_t i_challenge;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_revoke_challenge_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    // request data
    uint32_t i_group_id;
    uint8_t i_system_auth_token_version;
    gf_hw_auth_token_t i_auth_token;
    // response data
    uint32_t o_finger_id;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_enroll_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    // request data
    uint32_t i_group_id;
    uint32_t i_finger_id;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_save_finger_cmd_t;

typedef struct {
    gf_cmd_header_t header;

    // request data
    uint32_t i_group_id;
    uint64_t i_operation_id;
    uint64_t o_lockout_duration_millis;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_authenticate_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint64_t o_auth_id;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_get_auth_id_t;

typedef struct {
    gf_cmd_header_t header;
    uint64_t o_auth_id;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_invalidate_auth_id_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t o_size;
    uint32_t o_group_ids[MAX_FINGERS_PER_USER];
    uint32_t o_finger_ids[MAX_FINGERS_PER_USER];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_enumerate_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_gid;
    uint32_t i_finger_id;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_auth_save_templates_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_retry_count;
    uint32_t i_study_flag;
    uint32_t i_gid;
    uint32_t i_finger_id;
    uint8_t o_save_flag;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_auth_post_auth_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t i_hmac_key[QSEE_HMAC_KEY_MAX_LEN];
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_set_hmac_key_t;

typedef struct {
    gf_cmd_header_t header;
    uint32_t i_screen_state;  // 0: screen off, 1: screen on
    uint32_t i_authenticating;  // 0: yes, 1: no
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_screen_state_t;

typedef struct {
    gf_cmd_header_t header;
    gf_error_t i_result;
    uint32_t i_fid;
    int32_t i_retry;
    uint64_t o_lockout_duration_millis;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_auth_end_t;

typedef struct {
    gf_cmd_header_t header;
    gf_hw_auth_token_t i_auth_token;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_reset_lockout_cmd_t;

#endif /* _GF_FPCORE_TYPES_H_ */
