#include "thread_manager.h"
#include "plat_log.h"
#include "plat_thread.h"
#include "response_def.h"

typedef enum queue_task_type {
    AVAILABLE_Q,
    TASK_Q,
} queue_task_type_t;

typedef enum queue_wait_time {
    WAIT_FOREVER,
    WAIT_IMMEDIATELY,
    WAIT_IDLE,
} queue_wait_time_t;

typedef enum queue_pop_result {
    POP_OK,
    POP_TIMEOUT,
    POP_ABANDONED,
} queue_pop_result_t;

static do_operation_callback g_pflpAddress = NULL;
static do_operation_callback g_pfidle_task_lpAddress = NULL;
static rbsCancelFunc g_pfcancel_lpAddress = NULL;

task_t g_task = TASK_EMPTY;
static semaphore_handle_t g_tasks_sem[2] = {{0}, {0}};
static thread_handle_t g_thread_handle = {0};
static semaphore_handle_t g_transfer_sem = {0};

static mutex_handle_t run_new_task_lock = {0};
static BOOL g_run_new_task_waiting = FALSE;
static BOOL g_run_new_task_abort = FALSE;
static void run_new_task_destroy() {
    if (run_new_task_lock.mutex != NULL) {
        plat_mutex_release(&run_new_task_lock);
        run_new_task_lock.mutex = NULL;
    }
}
static void run_new_task_create() {
    run_new_task_destroy();
    plat_mutex_create(&run_new_task_lock);
}
static void run_new_task_set_waiting() {
    if (run_new_task_lock.mutex == NULL) {
        ex_log(LOG_ERROR, "mutex is null");
        return;
    }
    plat_mutex_lock(run_new_task_lock);
    g_run_new_task_waiting = TRUE;
    g_run_new_task_abort = FALSE;
    plat_mutex_unlock(run_new_task_lock);
}
static void run_new_task_set_abort() {
    if (run_new_task_lock.mutex == NULL) {
        ex_log(LOG_ERROR, "mutex is null");
        return;
    }
    plat_mutex_lock(run_new_task_lock);
    g_run_new_task_abort = TRUE;
    plat_mutex_unlock(run_new_task_lock);
}
static BOOL run_new_task_is_abort() {
    if (run_new_task_lock.mutex == NULL) {
        ex_log(LOG_ERROR, "mutex is null");
        return FALSE;
    }
    BOOL has_abort = FALSE;
    plat_mutex_lock(run_new_task_lock);
    g_run_new_task_waiting = FALSE;
    if (g_run_new_task_abort) {
        has_abort = TRUE;
    }
    plat_mutex_unlock(run_new_task_lock);
    return has_abort;
}

static void* thread_manager(void);

#define THREADM_LOG_LEVEL LOG_VERBOSE

void queue_init() {
    int result;

    result = plat_semaphore_create(&g_tasks_sem[AVAILABLE_Q], 1, 1);

    result = plat_semaphore_create(&g_tasks_sem[TASK_Q], 0, 1);
}

void queue_deinit() {
    plat_semaphore_release(&g_tasks_sem[AVAILABLE_Q]);
    plat_semaphore_release(&g_tasks_sem[TASK_Q]);
}

int queue_pop(queue_task_type_t type, queue_wait_time_t wait_type) {
    int result;
    int wait_time;
#define WAIT_IDLE_TIME 500  // ms

    if (type != AVAILABLE_Q && type != TASK_Q) {
        return -1;
    }

    switch (wait_type) {
        case WAIT_FOREVER:
            wait_time = -1;
            break;
        case WAIT_IMMEDIATELY:
            wait_time = 0;
            break;
        case WAIT_IDLE:
            wait_time = WAIT_IDLE_TIME;
            break;
        default:
            break;
    }

    result = plat_semaphore_wait(g_tasks_sem[type], wait_time);
    if (result == THREAD_RES_OK) {
        return POP_OK;
    } else if (result == THREAD_RES_WAIT_TIMEOUT) {
        return POP_TIMEOUT;
    } else {
        return POP_ABANDONED;
    }
}

int queue_add(queue_task_type_t task_type, task_t task) {
    switch (task_type) {
        case TASK_Q:
            g_task = task;
            plat_semaphore_post(g_tasks_sem[task_type]);
            break;
        case AVAILABLE_Q:
            plat_semaphore_post(g_tasks_sem[task_type]);
            break;
        default:
            break;
    }

    return 0;
}
/*
 * @brief Set a cancel task, to kill the running tasks.
 * @param cancel_func, a callback from captain.c
 * @return  - NULL
 */
void thread_manager_set_cancel_func(rbsCancelFunc cancel_func) {
    g_pfcancel_lpAddress = cancel_func;
}

/*
 * * @brief Set the IDLE task. The idle task will be started whenever main task is ended.
 *   @param lpAddress, a callback from captain.c,
 *          usually we set navigation/power_consumption as IDLE task.
 * @return  - NULL
 * */
void thread_manager_set_idle_task(do_operation_callback lpAddress) {
    g_pfidle_task_lpAddress = lpAddress;
}

/**
 * @brief Initialization of thread_manager, create a main thread to do the other tasks.
 *        main thread is always alive.
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 */
int thread_manager_init(void) {
    int retval;

    queue_init();
    run_new_task_create();

    retval = plat_thread_create(&g_thread_handle, thread_manager);
    if (retval != 0) {
        ex_log(LOG_ERROR, "plat_thread_create failed return = %d", retval);
        queue_deinit();
        return FINGERPRINT_RES_FAILED;
    }
    return FINGERPRINT_RES_SUCCESS;
}
/**
 * @brief uninitialize of thread_manager.
 * @return  - NULL
 */
void thread_manager_uninitialize(void) {
    int result;
    result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
    switch (result) {
        case POP_TIMEOUT:
            // start cancel
            if (!g_pfcancel_lpAddress) {
                ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
                break;
            }
            g_pfcancel_lpAddress(TRUE);
            queue_pop(AVAILABLE_Q, WAIT_FOREVER);
            g_pfcancel_lpAddress(FALSE);
        case POP_OK:
            // add task
            g_pflpAddress = NULL;
            queue_add(TASK_Q, TASK_EXIT);
            break;
        default:
            break;
    }

    plat_thread_release(&g_thread_handle);
    run_new_task_destroy();
    queue_deinit();

    plat_semaphore_release(&g_transfer_sem);
}

/**
 * @brief Request to run a new task.
 *        If there is already a task running, it will wait until current task is canceled or ended.
 *        then, add new task to thread manager.
 *
 * @param lpAddress, a operation_callback from from captain.c, it implemants new task.
 * @param TaskName It can be either ENROLL/VERIFY/...etc, see rbs_thread_operation_t.
 * @return  - NULL
 */

void thread_manager_run_task(do_operation_callback lpAddress, task_t task) {
    unsigned int result;
    result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
    switch (result) {
        case POP_TIMEOUT:
            // start cancel
            if (!g_pfcancel_lpAddress) {
                ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
                break;
            }
            g_pfcancel_lpAddress(TRUE);
            run_new_task_set_waiting();
            queue_pop(AVAILABLE_Q, WAIT_FOREVER);
            g_pfcancel_lpAddress(FALSE);
            if (run_new_task_is_abort()) {
                ex_log(LOG_ERROR, "abort/ignore waiting task.");
                g_pflpAddress = NULL;
                queue_add(TASK_Q, TASK_CANCEL);
                break;
            }
        case POP_OK:
            // add task
            g_pflpAddress = lpAddress;
            queue_add(TASK_Q, task);
            break;
        default:
            break;
    }
    return;
}

/*
 * * @brief cancel_func, Ues the cancel callback task, to kill the running tasks.
 *        If there is already a task running, it will wait until current task is ended.
 * @return  - NULL
 */

void thread_manager_cancel_task() {
    int result;
    result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
    switch (result) {
        case POP_TIMEOUT:
            // start cancel
            if (!g_pfcancel_lpAddress) {
                ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
                break;
            }
            g_pfcancel_lpAddress(TRUE);
            run_new_task_set_abort();
            queue_pop(AVAILABLE_Q, WAIT_FOREVER);
            g_pfcancel_lpAddress(FALSE);
        case POP_OK:
            // add task
            g_pflpAddress = NULL;
            queue_add(TASK_Q, TASK_CANCEL);
            break;
        default:
            return;
    }
}

static void clear_task() {
    queue_add(AVAILABLE_Q, 0);
}

static task_t get_task(BOOL idle_excuted) {
    int result;
    result = queue_pop(TASK_Q, idle_excuted ? WAIT_FOREVER : WAIT_IDLE);

    switch (result) {
        case POP_TIMEOUT:
            /* get AVAILABLE_Q, setup idle task as a TASK_*/
            if (queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY) == POP_OK) {
                return TASK_IDLE;
            }
            /* can not get AVAILABLE_Q, someone hold it and preparing, keep waiting for task forever
             */
            queue_pop(TASK_Q, WAIT_FOREVER);
        case POP_OK:
            /* get TASK_Q */
            return g_task;
        default:
            return TASK_EMPTY;
    }
}
/**
 * @brief main thread, responsible for runing new tasks.
 *        If there is a new task , set thread lock and run task, untill all task is finished.
 *        When all the tasks are done, start idle task after a timeout(ex:300ms).
 *
 * @return  - NULL
 */

static void* thread_manager(void) {
    int result = 0;
    BOOL idle_excuted = FALSE;
    task_t task = TASK_EMPTY;

    while (1) {
        task = get_task(idle_excuted);
        /* make idle task excuted once in idle time*/
        if (task != TASK_IDLE) idle_excuted = FALSE;

        switch (task) {
            case TASK_EMPTY:
                break;

            case TASK_IDLE:
                g_pfidle_task_lpAddress();
                idle_excuted = TRUE;
                break;

            case TASK_PROCESS:
                if (g_pflpAddress == NULL) {
                    ex_log(LOG_ERROR, "process cannot be excuted, pflpAddress=NULL");
                    goto exit;
                }
                result = g_pflpAddress();
                break;

            case TASK_CANCEL:
                /* defines for logger, works as the same as TASK_EMPTY*/
                break;
            case TASK_EXIT:
                goto exit;
            default:
                break;
        }

        clear_task();
    }
exit:

    return NULL;
}

static int g_in_opertation = 0;
/**
 * @brief Try to lock the main thread, usually for cpt_extra_api/other_tasks.
 * 		  Retuen Fail, if the main thread is occupied in ENROLL/VERIFY.
 *        Kill tasks then lock the thread, if thread is occupied in other tasks.
 *
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 *         Fail    - FINGERPRINT_RES_FAILED
 */
int thread_try_lock_operation(void) {
    int result;
    int retval = FINGERPRINT_RES_FAILED;

    if (g_in_opertation == 1) ex_log(LOG_ERROR, "during a operation");
    result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
    switch (result) {
        case POP_TIMEOUT:
            if (!g_pfcancel_lpAddress) {
                ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
                break;
            }
            g_pfcancel_lpAddress(TRUE);
            queue_pop(AVAILABLE_Q, WAIT_FOREVER);
            g_pfcancel_lpAddress(FALSE);
        case POP_OK:
            retval = FINGERPRINT_RES_SUCCESS;
            g_in_opertation = 1;
            break;
        default:
            break;
    }

    return retval;
}

/**
 * @brief Unlock the main thread.
 *
 * @return  - NULL
 */
void thread_unlock_operation(void) {
    if (g_in_opertation == 0) ex_log(LOG_ERROR, "not in a operation");
    g_in_opertation = 0;
    queue_add(TASK_Q, TASK_EMPTY);
}

/**
 * @brief trylock a operation thread. for other thread.
 *
 * @param operation, operation thread name which needs to trylock.
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 *         Fail    - FINGERPRINT_RES_FAILED
 */
int trylock_operation(rbs_operation_t operation) {
    int retval = FINGERPRINT_RES_FAILED;

    plat_semaphore_create(&g_transfer_sem, 1, 1);

    switch (operation) {
        case RBS_MUTEX_TRANSFER:
            if (THREAD_RES_OK == plat_semaphore_wait(g_transfer_sem, -1)) {
                retval = FINGERPRINT_RES_SUCCESS;
            }
            break;
        default:
            break;
    }

    return retval;
}

/**
 * @brief unlock a operation thread. for other thread.
 *
 * @param operation, operation thread name which needs to unlock.
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 *         Fail    - FINGERPRINT_RES_FAILED
 */
int unlock_operation(rbs_operation_t operation) {
    int retval = FINGERPRINT_RES_SUCCESS;
    switch (operation) {
        case RBS_MUTEX_TRANSFER:
            if (0 == plat_semaphore_post(g_transfer_sem)) {
                retval = FINGERPRINT_RES_SUCCESS;
            }
            break;
        default:
            retval = FINGERPRINT_RES_FAILED;
            break;
    }

    return retval;
}
