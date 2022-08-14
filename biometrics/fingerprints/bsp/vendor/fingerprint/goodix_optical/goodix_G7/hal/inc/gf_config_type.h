/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#ifndef _GF_CONFIG_TYPE_H_
#define _GF_CONFIG_TYPE_H_

#include <stdint.h>
#include "gf_base_types.h"

typedef enum {
    GF_AUTHENTICATE_BY_USE_RECENTLY = 0,
    GF_AUTHENTICATE_BY_ENROLL_ORDER,
    GF_AUTHENTICATE_BY_REVERSE_ENROLL_ORDER,
} gf_authenticate_order_t;

typedef struct gf_config {
    uint32_t max_fingers;
    uint32_t max_fingers_per_user;
    /**
     * min_value <= enrolling_min_templates <= max_value
     *           CHIP_TYPE                 min_value   max_value
     *  GF316M/GF516M/GF816M                  10          40
     *  GF318M/GF3118M/GF518M/GF5118M         8           30
     *  GF3206/GF3208/GF3288/GF3268           8           30
     *  GF3266                                8           25
     *  GF3288                                8           20
     *  GF5206/GF5216                         10          40
     *  GF5208/GF5218                         8           40
     */
    uint32_t enrolling_min_templates;

    uint32_t valid_image_quality_threshold;
    uint32_t valid_image_area_threshold;
    uint32_t duplicate_finger_overlay_score;
    uint32_t increase_rate_between_stitch_info;

    uint8_t forbidden_untrusted_enroll;
    uint8_t forbidden_enroll_duplicate_fingers;

    /**
     * Reference Android M com.android.server.fingerprint.FingerprintService.java
     * private static final int MAX_FAILED_ATTEMPTS = 5;
     * authenticate failed too many attempts. Try again later.
     * value 0, don't check authenticate failed attempts.
     */
    uint32_t max_authenticate_failed_attempts;
    uint32_t max_authenticate_failed_lock_out;
    uint32_t max_failed_attempts_temporary_lockout;
    uint32_t max_failed_attempts_permanent_lockout;
    uint32_t lockout_timeout_sec;
    uint32_t relatively_recent_timeout_sec;
    uint8_t support_lockout_in_ta;
    gf_authenticate_order_t authenticate_order;
    /**
     * configure fingerprint template study update save threshold
     */
    uint32_t template_update_save_threshold;
    /* config whether proguard the finger id when transport to others ta application
           0:disable, this is the default value;
           1:enable.
     */
    uint8_t support_bio_finger_id_proguard;
    uint8_t bio_finger_id_proguard_factor;  // used by proguard function, the range is 0x00~0xFF
    uint8_t support_performance_dump;
    uint8_t support_ui_ready;
    uint32_t enroll_auth_token_timeout;  // the unit is in seconds
    uint8_t save_extra_info_with_template;
    uint32_t enroll_timeout_sec;
    uint64_t align_reserve;  // used for make struct align by 8bytes
    uint8_t reserve[MAX_RESERVE_SIZE];
} gf_config_t;

#endif /* _GF_CONFIG_TYPE_H_ */
