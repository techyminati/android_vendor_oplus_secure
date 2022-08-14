#define LOG_TAG "[ANC_TAC][BackgroundWorker]"

#include "anc_background_worker.h"
#include "anc_log.h"
#include <errno.h>

static pthread_t g_thread;
static pthread_mutex_t g_mutex;
static pthread_cond_t g_cond;
static ANC_BOOL g_is_running = ANC_FALSE;

static AncBGTaskNode_t g_task_head;
static AncBGTaskNode_t *gp_task_head = &g_task_head;

#define CHECK_RET(_expr)                    \
    if (0 != _expr) {                       \
        ANC_LOGE("background worker, failed %s", #_expr);         \
        return ANC_FAIL;                    \
    }

static void *DoTask() {
    ANC_LOGD("start background worker thread");

    pthread_mutex_lock(&g_mutex);
    g_is_running = ANC_TRUE;
    pthread_mutex_unlock(&g_mutex);

    while (ANC_TRUE) {
        pthread_mutex_lock(&g_mutex);

        while (gp_task_head->next == NULL && g_is_running == ANC_TRUE) {
            pthread_cond_wait(&g_cond, &g_mutex);
        }

        if (g_is_running == ANC_FALSE) {
            pthread_mutex_unlock(&g_mutex);
            ANC_LOGD("exit background worker thread");
            return NULL;
        }

        AncBGTaskNode_t *p_node = gp_task_head->next;
        AncBGTask task = p_node->task;
        gp_task_head->next = p_node->next;
        AncFree(p_node);

        pthread_mutex_unlock(&g_mutex);
        ANC_LOGD("background worker, begin to excute task");
        task.Excute(task.data);
        if (task.Free != NULL) {
            task.Free(task.data);
        }
        ANC_LOGD("background worker, task finished");
    }


    return NULL;
}

static ANC_RETURN_TYPE WorkerInit() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("init background worker");
    if (g_is_running) {
        return ANC_OK;
    }

    CHECK_RET(pthread_mutex_init(&g_mutex, NULL));
    CHECK_RET(pthread_cond_init(&g_cond, NULL));
    CHECK_RET(pthread_create(&g_thread, NULL, DoTask, NULL));

    return ret_val;
}

static ANC_RETURN_TYPE WorkerDeinit() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("deinit background worker");
    pthread_mutex_lock(&g_mutex);
    g_is_running = ANC_FALSE;
    pthread_mutex_unlock(&g_mutex);

    pthread_cond_signal(&g_cond);
    pthread_join(g_thread, NULL);

    while (gp_task_head->next != NULL) {
        AncBGTaskNode_t *p_node = gp_task_head->next;
        AncBGTask task = p_node->task;
        if (task.Free != NULL) {
            task.Free(task.data);
        }
        gp_task_head->next = p_node->next;
        AncFree(p_node);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond);
    ANC_LOGD("deinit background worker finished");
    return ret_val;
}

static ANC_RETURN_TYPE WorkerPushTask(AncBGTask task) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    ANC_LOGD("push task to background worker");
    if (task.Excute == NULL) {
        ANC_LOGE("background worker, no Excute func");
        return ANC_FAIL;
    }

    pthread_mutex_lock(&g_mutex);

    AncBGTaskNode_t *p_task_tail = gp_task_head;
    while (p_task_tail->next != NULL) {
        p_task_tail = p_task_tail->next;
    }
    AncBGTaskNode_t *p_node = (AncBGTaskNode_t *) AncMalloc(sizeof(AncBGTaskNode_t));
    AncMemset(p_node, 0, sizeof(AncBGTaskNode_t));
    p_node->task = task;
    p_task_tail->next = p_node;
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
    return ret_val;
}

AncBGWorker g_background_worker = {
    .Init = WorkerInit,
    .Deinit = WorkerDeinit,
    .PushTask = WorkerPushTask,
};

