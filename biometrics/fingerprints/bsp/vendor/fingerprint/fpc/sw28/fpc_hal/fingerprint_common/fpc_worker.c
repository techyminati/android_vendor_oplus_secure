/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <pthread.h>

#include <fpc_log.h>

#include "fpc_worker.h"

typedef enum {
    WORKER_STATE_IDLE = 0,
    WORKER_STATE_RUNNING = 1,
} worker_state_t;

typedef struct {
    void (*func)(void*);
    void* arg;
} worker_task_t;

struct fpc_worker {
    pthread_mutex_t mutex;
    pthread_t thread;
    bool thread_started;
    pthread_cond_t cond;
    worker_task_t task;
    bool terminate;
    worker_state_t state;
};

static void* worker_function(void* arg)
{
    fpc_worker_t* worker = (fpc_worker_t*) arg;

    for (;;) {
        pthread_mutex_lock(&worker->mutex);
        worker->state = WORKER_STATE_IDLE;
        pthread_cond_signal(&worker->cond);

        for (;;) {
            if (worker->terminate) {
                pthread_mutex_unlock(&worker->mutex);
                return NULL;
            }
            if (worker->task.func) {
                break;
            }
            pthread_cond_wait(&worker->cond, &worker->mutex);
        }

        worker_task_t task = worker->task;
        worker->state = WORKER_STATE_RUNNING;
        worker->task.func = NULL;
        worker->task.arg = NULL;
        pthread_cond_signal(&worker->cond);
        pthread_mutex_unlock(&worker->mutex);
        task.func(task.arg);
    }
}

void fpc_worker_run_task(fpc_worker_t* worker, void (*func)(void*), void* arg)
{
    LOGD("%s", __func__);
    pthread_mutex_lock(&worker->mutex);
    worker->task.func = func;
    worker->task.arg = arg;
    pthread_cond_signal(&worker->cond);
    while (worker->task.func != NULL) {
        pthread_cond_wait(&worker->cond, &worker->mutex);
    }
    pthread_mutex_unlock(&worker->mutex);
}

void fpc_worker_join_task(fpc_worker_t* worker)
{
    LOGD("%s", __func__);
    pthread_mutex_lock(&worker->mutex);
    while (worker->state != WORKER_STATE_IDLE) {
        pthread_cond_wait(&worker->cond, &worker->mutex);
    }
    pthread_mutex_unlock(&worker->mutex);
}

fpc_worker_t* fpc_worker_new()
{
    fpc_worker_t* worker = (fpc_worker_t*) malloc(sizeof(fpc_worker_t));
    if (!worker) {
        return NULL;
    }

    memset(worker, 0, sizeof(fpc_worker_t));

    pthread_mutex_init(&worker->mutex, NULL);
    pthread_cond_init(&worker->cond, NULL);
    worker->state = WORKER_STATE_IDLE;

    int status = pthread_create(&worker->thread, NULL, worker_function, worker);

    if (status) {
        LOGE("%s pthread_create failed %i\n", __func__, status);
        goto err;
    }

    worker->thread_started = true;
    return worker;

err:
    fpc_worker_destroy(worker);
    return NULL;
}

void fpc_worker_destroy(fpc_worker_t* worker)
{
    if (!worker) {
        return;
    }

    pthread_mutex_lock(&worker->mutex);
    worker->terminate = true;
    pthread_mutex_unlock(&worker->mutex);

    pthread_cond_signal(&worker->cond);
    if (worker->thread_started) {
        pthread_join(worker->thread, NULL);
    }

    pthread_mutex_destroy(&worker->mutex);
    pthread_cond_destroy(&worker->cond);
    free(worker);
}
