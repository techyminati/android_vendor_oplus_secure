/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <stdlib.h>
#include <pthread.h>
#include <sys/queue.h>
#include <string.h>

#include "gf_queue.h"
#include "gf_hal_log.h"
#include "gf_hal_mem.h"

#define LOG_TAG "[GF_HAL][gf_queue]"

typedef struct gf_event
{
    uint32_t data;
    TAILQ_ENTRY(gf_event) tailq_entry;
} gf_event_t;

TAILQ_HEAD(tailq_head, gf_event) head;
static pthread_mutex_t g_queue_mutex = PTHREAD_MUTEX_INITIALIZER;  // queue mutex

/**
 * Function: queue_clear_unused_irq
 * Description: Remove unused GF_NETLINK_IRQ data.
 * Input: None
 * Output: None
 * Return: None
 */
static void queue_clear_unused_irq()
{
    // LOG_D(LOG_TAG, "[%s]", __func__);
    if (0 != TAILQ_EMPTY(&head))
    {
        return;
    }

    gf_event_t *event = NULL;
    TAILQ_FOREACH(event, &head, tailq_entry)
    {
        if (event->data == GF_NETLINK_IRQ)
        {
            LOG_D(LOG_TAG, "[%s] invalid irq, just drop it", __func__);
            TAILQ_REMOVE(&head, event, tailq_entry);
            GF_MEM_FREE(event);
            break;
        }
    }
    // queue_dump();
}

/**
 * Function: gf_queue_init
 * Description: Initialize a queue.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_queue_init()
{
    LOG_D(LOG_TAG, "[%s]", __func__);
    TAILQ_INIT(&head);
    return GF_SUCCESS;
}

/**
 * Function: gf_queue_exit
 * Description: Remove data and free memory in pthread_mutex_lock.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_queue_exit()
{
    LOG_D(LOG_TAG, "[%s]", __func__);
    gf_event_t *event = NULL;
    pthread_mutex_lock(&g_queue_mutex);
    while (NULL != (event = TAILQ_FIRST(&head)))
    {
        TAILQ_REMOVE(&head, event, tailq_entry);
        GF_MEM_FREE(event);
        event = NULL;
    }
    pthread_mutex_unlock(&g_queue_mutex);
    return GF_SUCCESS;
}

/**
 * Function: gf_queue_clear_unused_irq
 * Description: Remove unused GF_NETLINK_IRQ data in pthread_mutex_lock.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_queue_clear_unused_irq()
{
    LOG_D(LOG_TAG, "[%s]", __func__);
    gf_error_t ret = GF_SUCCESS;
    pthread_mutex_lock(&g_queue_mutex);
    queue_clear_unused_irq();
    pthread_mutex_unlock(&g_queue_mutex);
    return ret;
}

/**
 * Function: gf_empty_queue
 * Description: Check empty queue in pthread_mutex_lock.
 * Input: None
 * Output: None
 * Return: uint8_t
 */
uint8_t gf_empty_queue()
{
    uint8_t empty = 0;
    pthread_mutex_lock(&g_queue_mutex);
    empty = TAILQ_EMPTY(&head);
    pthread_mutex_unlock(&g_queue_mutex);
    return empty;
}

/**
 * Function: gf_queue_contains_irq
 * Description: Judge GF_NETLINK_IRQ data in queue.
 * Input: None
 * Output: None
 * Return: uint8_t
 */
static uint8_t gf_queue_contains_irq()
{
    uint8_t contains_flag = 0;
    gf_event_t *event = NULL;

    do
    {
        if (0 != TAILQ_EMPTY(&head))
        {
            break;
        }

        TAILQ_FOREACH(event, &head, tailq_entry)
        {
            if (event->data == GF_NETLINK_IRQ)
            {
                contains_flag = 1;
                break;
            }
        }
    }
    while (0);

    return contains_flag;
}

/**
 * Function: gf_enqueue
 * Description: Enqueue, and drop old irq message.
 * Input: cmd_type
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_enqueue(uint8_t cmd_type)
{
    gf_error_t ret = GF_SUCCESS;
    gf_event_t *event = NULL;
    event = (gf_event_t *) GF_MEM_MALLOC(sizeof(gf_event_t));

    if (event == NULL)
    {
        LOG_D(LOG_TAG, "[%s] out of memory", __func__);
        return GF_ERROR_OUT_OF_MEMORY;
    }
    memset(event, 0, sizeof(gf_event_t));
    pthread_mutex_lock(&g_queue_mutex);
    event->data = cmd_type;

    if (cmd_type == GF_NETLINK_IRQ)
    {
        if (gf_queue_contains_irq() > 0)
        {
            LOG_D(LOG_TAG, "[%s] drop irq", __func__);
            GF_MEM_FREE(event);
            ret = GF_ERROR_HAL_GENERAL_ERROR;
        }
        else
        {
            TAILQ_INSERT_TAIL(&head, event, tailq_entry);
        }
    }
    else
    {
        TAILQ_INSERT_TAIL(&head, event, tailq_entry);
    }

    // queue_dump();
    pthread_mutex_unlock(&g_queue_mutex);
    return ret;
}

/**
 * Function: gf_dequeue
 * Description: Dequeue, and free memory.
 * Input: cmd_type
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_dequeue(uint8_t *cmd_type)
{
    gf_error_t ret = GF_SUCCESS;
    gf_event_t *event = NULL;

    if (cmd_type == NULL)
    {
        LOG_E(LOG_TAG, "[%s] invalid parameters.", __func__);
        return GF_ERROR_BAD_PARAMS;
    }

    pthread_mutex_lock(&g_queue_mutex);

    do
    {
        if (0 != TAILQ_EMPTY(&head))
        {
            ret = GF_ERROR_GENERIC;
            break;
        }

        event = TAILQ_FIRST(&head);

        if (event != NULL)
        {
            *cmd_type = event->data;
            TAILQ_REMOVE(&head, event, tailq_entry);
            GF_MEM_FREE(event);
        }

        // queue_dump();
    }
    while (0);

    pthread_mutex_unlock(&g_queue_mutex);
    return ret;
}

