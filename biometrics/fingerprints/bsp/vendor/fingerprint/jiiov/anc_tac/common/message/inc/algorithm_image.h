#ifndef __ALGORITHM_IMAGE_H__
#define __ALGORITHM_IMAGE_H__

#include "anc_type.h"

typedef enum {
    ANC_ALGORITHM_IMAGE_NONE = 0,
    ANC_ALGORITHM_IMAGE_RAW,
    ANC_ALGORITHM_IMAGE_BIN,
    ANC_ALGORITHM_IMAGE_BKG,
    ANC_ALGORITHM_IMAGE_TMI,
    ANC_ALGORITHM_IMAGE_TMC,
    ANC_ALGORITHM_IMAGE_PRE,
    ANC_ALGORITHM_IMAGE_BASE,
    ANC_ALGORITHM_IMAGE_HBR,
    ANC_ALGORITHM_IMAGE_MAX
}ANC_ALGORITHM_IMAGE_TYPE;


typedef struct {
    int index;
    int try_count;
    uint32_t image_sum;
    int algo_status;
    int sdk_status;
    int expo_time;
}__attribute__((packed)) AncAlgoEnrollImageInfo;

typedef struct {
    int index;
    int cmp_result;
    int try_count;
    int match_user_id;
    int match_finger_id;
    uint32_t matched_img_sum;
    int match_score;
    int match_cache_score;
    int algo_status;
    int sdk_status;
    int expo_time;
}__attribute__((packed)) AncAlgoVerifyImageInfo;

typedef struct {
    uint32_t index;
    uint64_t timestamp;
    uint32_t expo_time;
    uint32_t feat;
    uint32_t bpm;
    uint32_t report_bpm;
    uint32_t reserved[10];
}__attribute__((packed)) AncAlgoHeartBeatImageInfo;

typedef union {
    AncAlgoEnrollImageInfo enroll;
    AncAlgoVerifyImageInfo verify;
    AncAlgoHeartBeatImageInfo hb;
}__attribute__((packed)) AncAlgoImageInfo;


typedef struct {
    ANC_ALGORITHM_IMAGE_TYPE image_type;
    uint32_t image_buffer_length;
} __attribute__((packed)) AncAlgorithmImageItemHeader;

typedef struct {
    uint32_t image_count;
    AncAlgoImageInfo image_info;
    AncAlgorithmImageItemHeader* __attribute__((unused))p_image_item; //not really ptr dont use it
    char * __attribute__((unused))p_image_data;   //not really ptr dont use it
} __attribute__((packed)) AncAlgorithmImageData;

#endif