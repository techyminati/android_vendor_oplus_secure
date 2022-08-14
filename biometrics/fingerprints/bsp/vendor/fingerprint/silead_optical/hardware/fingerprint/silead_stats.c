/******************************************************************************
 * @file   silead_stats.c
 * @brief  Contains time statistics functions.
 *
 *
 * Copyright (c) 2016-2017 Silead Inc.
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * Silead Inc. You shall not disclose the present software and shall use it
 * only in accordance with the terms of the license agreement you entered
 * into with Silead Inc. This software may be subject to export or import
 * laws in certain countries.
 *
 *
 * ------------------- Revision History ------------------------------
 * <author>    <date>   <version>     <desc>
 * Jack Zhang  2018/4/2    0.1.0      Init version
 *
 *****************************************************************************/

#define FILE_TAG "silead_stats"
#include "log/logmsg.h"

#include <time.h>

#include "silead_stats.h"

static uint32_t m_stats_enabled = 0;

static uint64_t m_auth_match_total_time = 0;
static uint32_t m_auth_match_total_count = 0;
static uint32_t m_auth_match_current_time = 0;

static uint64_t m_auth_mismatch_total_time = 0;
static uint32_t m_auth_mismatch_total_count = 0;
static uint32_t m_auth_mismatch_current_time = 0;

static uint64_t m_match_total_time = 0;
static uint32_t m_match_total_count = 0;
static uint32_t m_match_current_time = 0;

static uint64_t m_mismatch_total_time = 0;
static uint32_t m_mismatch_total_count = 0;
static uint32_t m_mismatch_current_time = 0;

static uint64_t m_image_total_time = 0;
static uint32_t m_image_total_count = 0;
static uint32_t m_image_current_time = 0;

static uint64_t m_begin_time = 0;
static uint64_t m_auth_begin_time = 0;

static void _stats_auth_dump(int32_t match)
{
    if (match) {
        LOG_MSG_INFO("img[(%d:%d:%d)] match[(%d:%d:%d)] total[(%d:%d:%d)]",
                     m_image_current_time, m_image_total_count, (uint32_t)(m_image_total_time/m_image_total_count),
                     m_auth_match_current_time, m_auth_match_total_count, (uint32_t)(m_auth_match_total_time/m_auth_match_total_count),
                     m_match_current_time, m_match_total_count, (uint32_t)(m_match_total_time/m_match_total_count));
    } else {
        LOG_MSG_INFO("img[(%d:%d:%d)] mis[(%d:%d:%d)] total[(%d:%d:%d)]",
                     m_image_current_time, m_image_total_count, (uint32_t)(m_image_total_time/m_image_total_count),
                     m_auth_mismatch_current_time, m_auth_mismatch_total_count, (uint32_t)(m_auth_mismatch_total_time/m_auth_mismatch_total_count),
                     m_mismatch_current_time, m_mismatch_total_count, (uint32_t)(m_mismatch_total_time/m_mismatch_total_count));
    }
}

static uint64_t _stats_get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void silfp_stats_reset(void)
{
    if (m_stats_enabled) {
        m_auth_match_total_time = 0;
        m_auth_match_total_count = 0;
        m_auth_match_current_time = 0;

        m_auth_mismatch_total_time = 0;
        m_auth_mismatch_total_count = 0;
        m_auth_mismatch_current_time = 0;

        m_match_total_time = 0;
        m_match_total_count = 0;
        m_match_current_time = 0;

        m_mismatch_total_time = 0;
        m_mismatch_total_count = 0;
        m_mismatch_current_time = 0;

        m_image_total_time = 0;
        m_image_total_count = 0;
        m_image_current_time = 0;

        m_begin_time = 0;
        m_auth_begin_time = 0;
    }
}

void silfp_stats_start(void)
{
    if (m_stats_enabled) {
        m_begin_time = _stats_get_time();
    }
}

void silfp_stats_capture_image(void)
{
    if (m_stats_enabled) {
        uint64_t end = _stats_get_time();
        if (end >= m_begin_time) {
            m_image_current_time = (uint32_t)(end - m_begin_time);
            m_image_total_count++;
            if (m_image_total_count == 0) {
                m_image_total_count++;
                m_image_total_time = m_image_current_time;
            } else {
                m_image_total_time += m_image_current_time;
            }
        }

        m_auth_begin_time = _stats_get_time();
    }
}

void silfp_stats_auth_mismatch(void)
{
    if (m_stats_enabled) {
        uint64_t end = _stats_get_time();
        if (end >= m_begin_time) {
            m_mismatch_current_time = (uint32_t)(end - m_begin_time);
            m_mismatch_total_count++;
            if (m_mismatch_total_count == 0) {
                m_mismatch_total_count++;
                m_mismatch_total_time = m_mismatch_current_time;
            } else {
                m_mismatch_total_time += m_mismatch_current_time;
            }
        }

        if (end >= m_auth_begin_time) {
            m_auth_mismatch_current_time = (uint32_t)(end - m_auth_begin_time);
            m_auth_mismatch_total_count++;
            if (m_auth_mismatch_total_count == 0) {
                m_auth_mismatch_total_count++;
                m_auth_mismatch_total_time = m_auth_mismatch_current_time;
            } else {
                m_auth_mismatch_total_time += m_auth_mismatch_current_time;
            }
        }
        _stats_auth_dump(0);
    }
}

void silfp_stats_auth_match(void)
{
    if (m_stats_enabled) {
        uint64_t end = _stats_get_time();
        if (end >= m_begin_time) {
            m_match_current_time = (uint32_t)(end - m_begin_time);
            m_match_total_count++;
            if (m_match_total_count == 0) {
                m_match_total_count++;
                m_match_total_time = m_match_current_time;
            } else {
                m_match_total_time += m_match_current_time;
            }
        }
        if (end >= m_auth_begin_time) {
            m_auth_match_current_time = (uint32_t)(end - m_auth_begin_time);
            m_auth_match_total_count++;
            if (m_auth_match_total_count == 0) {
                m_auth_match_total_count++;
                m_auth_match_total_time = m_auth_match_current_time;
            } else {
                m_auth_match_total_time += m_auth_match_current_time;
            }
        }
        _stats_auth_dump(1);
    }
}

void silfp_stats_set_enabled(uint32_t enable)
{
    m_stats_enabled = (enable ? 1 : 0);
}
