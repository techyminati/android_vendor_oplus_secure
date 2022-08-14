#ifndef __ALGORITHM_COMMAND_H__
#define __ALGORITHM_COMMAND_H__

#include "anc_type.h"

typedef enum {
    ANC_CMD_ALGORITHM_NONE = 0,
    ANC_CMD_ALGORITHM_INIT,
    ANC_CMD_ALGORITHM_DEINIT,
    ANC_CMD_ALGORITHM_INIT_ENROLL,
    ANC_CMD_ALGORITHM_ENROLL,
    ANC_CMD_ALGORITHM_DEINIT_ENROLL,
    ANC_CMD_ALGORITHM_INIT_VERIFY,
    ANC_CMD_ALGORITHM_VERIFY,
    ANC_CMD_ALGORITHM_EXTRACT_FEATURE,
    ANC_CMD_ALGORITHM_ENROLL_FEATURE,   // 9
    ANC_CMD_ALGORITHM_COMPARE_FEATURE, // 10
    ANC_CMD_ALGORITHM_FEATURE_STUDY,   // 11
    ANC_CMD_ALGORITHM_DEINIT_VERIFY,
    ANC_CMD_TEMPLATE_LOAD_DATABASE,
    ANC_CMD_TEMPLATE_SET_ACTIVE_GROUP,
    ANC_CMD_TEMPLATE_GET_AUTHENTICATOR_ID,
    ANC_CMD_TEMPLATE_DELETE_FINGERPRINT,
    ANC_CMD_TEMPLATE_GET_ALL_FINGERPRINTS_IDS,
    ANC_CMD_ALGORITHM_GET_ENROLL_TOTAL_TIMES,
    ANC_CMD_ALGORITHM_GET_VERSION,
    ANC_CMD_ALGORITHM_GET_IMAGE_QUALITY_SCORE,
    ANC_CMD_ALGORITHM_GET_HEART_BEAT_RESULT,
    ANC_CMD_ALGORITHM_MAX
}ANC_COMMAND_ALGORITHM_TYPE;

typedef enum {
    ANC_ALGORITHM_UNSAVE = 0,
    ANC_ALGORITHM_SAVE,
}ANC_COMMAND_ALGORITHM_SAVE_IMAGE_TYPE;

typedef struct {
    uint32_t command;
    int32_t data;
    uint32_t group_id;
    uint32_t finger_id;
    uint32_t retry_count;
    uint32_t extract_type;  /// 0, enroll. 1, verity
    ANC_COMMAND_ALGORITHM_SAVE_IMAGE_TYPE save_image;
} __attribute__((packed)) AncAlgorithmCommand;

typedef struct {
    uint32_t data;
    uint64_t authenticator_id;
    uint32_t finger_id;
    uint32_t id[5];
    uint32_t id_count;
    uint32_t need_study;
    int32_t enroll_total_times;
    uint8_t version[50];
    uint32_t version_size;
    uint32_t image_quality_score;
    uint32_t algo_status;
} __attribute__((packed)) AncAlgorithmCommandRespond;

#endif
