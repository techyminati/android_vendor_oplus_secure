/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <pthread.h>
#include <string.h>

#include "gf_hal_timer.h"
#include "gf_hal_log.h"

#define LOG_TAG "[GF_HAL][gf_hal_timer]"

static pthread_mutex_t g_timer_mutex = PTHREAD_MUTEX_INITIALIZER;  // timer mutex

/**
 * Function: hal_destroy_timer
 * Description: Destroy timer.
 * Input: timer_id
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_destroy_timer(timer_t *timer_id)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    if (NULL != timer_id && 0 != *timer_id)
    {
        LOG_D(LOG_TAG, "[%s] timer_id=%p", __func__, (void *)timer_id);

        if (timer_delete(*timer_id) != 0)
        {
            LOG_E(LOG_TAG, "[%s] timer_delete failed", __func__);
            err = GF_ERROR_HAL_GENERAL_ERROR;
        }

        *timer_id = 0;
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_set_timer
 * Description: Set timer.
 * Input: timer_id, interval_second, value_second, value_nanosecond
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_set_timer(timer_t *timer_id,
                                time_t interval_second, time_t value_second,
                                int64_t value_nanosecond)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == timer_id || 0 == *timer_id)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameter.", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        LOG_D(LOG_TAG, "[%s] timer_id=%p", __func__, (void *)timer_id);
        struct itimerspec ts;
        memset(&ts, 0, sizeof(struct itimerspec));
        ts.it_interval.tv_sec = interval_second;
        ts.it_interval.tv_nsec = 0;
        ts.it_value.tv_sec = value_second;
        ts.it_value.tv_nsec = value_nanosecond;

        if (timer_settime(*timer_id, 0, &ts, NULL) != 0)
        {
            LOG_E(LOG_TAG, "[%s] timer_settime failed", __func__);
            hal_destroy_timer(timer_id);
            err = GF_ERROR_HAL_GENERAL_ERROR;
        }
    }
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_create_timer
 * Description: Create timer.
 * Input: timer_id, timer_thread
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t hal_create_timer(timer_t *timer_id,
                                   timer_thread_t timer_thread)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == timer_id || NULL == timer_thread)
        {
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (0 != *timer_id)
        {
            hal_destroy_timer(timer_id);
        }

        struct sigevent evp;

        memset(&evp, 0, sizeof(struct sigevent));

        evp.sigev_value.sival_int = 3;

        evp.sigev_value.sival_ptr = timer_id;

        evp.sigev_notify = SIGEV_THREAD;

        evp.sigev_notify_function = timer_thread;

        if (timer_create(CLOCK_REALTIME, &evp, timer_id) != 0)
        {
            LOG_E(LOG_TAG, "[%s] timer_create failed", __func__);
            *timer_id = 0;
            err = GF_ERROR_HAL_GENERAL_ERROR;
        }
        else
        {
            LOG_D(LOG_TAG, "[%s] timer_id=%p", __func__, (void *)timer_id);
        }
    }  // do hal_create_timer
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_create_timer
 * Description: Create timer in mutex lock.
 * Input: timer_id, timer_thread
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_create_timer(timer_t *timer_id, timer_thread_t timer_thread)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    if (NULL == timer_id)
    {
        LOG_E(LOG_TAG, "[%s] timer_id is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        pthread_mutex_lock(&g_timer_mutex);
        err = hal_create_timer(timer_id, timer_thread);
        pthread_mutex_unlock(&g_timer_mutex);
    }

    FUNC_EXIT(err);
    return err;
}


/**
 * Function: gf_hal_set_timer
 * Description: Set timer in mutex lock.
 * Input: timer_id, interval_second, value_second, value_nanosecond
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_set_timer(timer_t *timer_id,
                            time_t interval_second, time_t value_second,
                            int64_t value_nanosecond)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    if (NULL == timer_id)
    {
        LOG_E(LOG_TAG, "[%s] timer_id is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        pthread_mutex_lock(&g_timer_mutex);
        err = hal_set_timer(timer_id, interval_second, value_second, value_nanosecond);
        pthread_mutex_unlock(&g_timer_mutex);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_destroy_timer
 * Description: Destroy timer in mutex lock.
 * Input: timer_id
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_destroy_timer(timer_t *timer_id)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    if (NULL == timer_id)
    {
        LOG_E(LOG_TAG, "[%s] timer_id is null", __func__);
        err = GF_ERROR_BAD_PARAMS;
    }
    else
    {
        pthread_mutex_lock(&g_timer_mutex);
        err = hal_destroy_timer(timer_id);
        pthread_mutex_unlock(&g_timer_mutex);
    }

    FUNC_EXIT(err);
    return err;
}

