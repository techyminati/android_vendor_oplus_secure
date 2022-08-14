/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _GF_DELMAR_HEARTBEAT_RATE_TYPES_H_
#define _GF_DELMAR_HEARTBEAT_RATE_TYPES_H_

#include <stdint.h>
#include "gf_heartbeat_rate_types.h"
#include "gf_base_types.h"

enum {
    GF_HEARTBEAT_RATE_CMD_START_HEART_BEAT_RATE_DETECT = GF_HEARTBEAT_RATE_CMD_MAX + 1,
    GF_HEARTBEAT_RATE_CMD_STOP_HEART_BEAT_RATE_DETECT,
    GF_DELMAR_HEARTBEAT_RATE_CMD_MAX,
};

typedef struct {
    gf_cmd_header_t cmd_header;
    uint32_t frame_index;
    int32_t heart_rate_result;
    uint8_t heart_rate_out[10];
    uint32_t heart_rate_time;
    uint8_t heart_rate_saq;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_heart_rate_preview_t;

#define COLLECT_FILE_PATH_MAX_LEN (256)
typedef struct {
    gf_cmd_header_t header;
    int32_t hb_rate;
    int32_t result_flag;
    int32_t ppg_data;
    uint8_t is_hard_press;
    uint32_t rawdata_count;
    uint8_t finger_names[COLLECT_FILE_PATH_MAX_LEN];
    uint8_t i_continue_flag;
    uint32_t origin_ppg_data;
    uint32_t interval_time;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_heart_beat_rate_cmd_t;

typedef struct {
    gf_cmd_header_t header;
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_delmar_clear_heart_beat_rate_data_cmd_t;
#endif /* _GF_DELMAR_HEARTBEAT_RATE_TYPES_H_ */
