#include "plat_log.h"
#include "plat_thread.h"
#include "response_def.h"
#include "thread_manager.h"

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

static void *thread_manager(void);

#define THREADM_LOG_LEVEL LOG_VERBOSE

void queue_init()
{
	int result;
	ex_log(LOG_DEBUG, "*** queue_init enter");

	result = plat_semaphore_create(&g_tasks_sem[AVAILABLE_Q], 1, 1);
	ex_log(LOG_DEBUG, "plat_semaphore_create AVAILABLE_Q result = %d", result);
	result = plat_semaphore_create(&g_tasks_sem[TASK_Q], 0, 1);
	ex_log(LOG_DEBUG, "plat_semaphore_create TASK_Q result = %d", result);
}

void queue_deinit()
{
	ex_log(LOG_DEBUG, "*** queue_deinit enter");
	plat_semaphore_release(&g_tasks_sem[AVAILABLE_Q]);
	plat_semaphore_release(&g_tasks_sem[TASK_Q]);
}

int queue_pop(queue_task_type_t type, queue_wait_time_t wait_type)
{
	int result;
	int wait_time;
#define WAIT_IDLE_TIME 500  //ms

	ex_log(THREADM_LOG_LEVEL, "*** queue_pop(%d, %d) enter", type, wait_type);
	if (type != AVAILABLE_Q && type != TASK_Q) {
		ex_log(LOG_ERROR, "*** queue_pop(%d, %d) the type is wrong", type, wait_type);
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
			ex_log(LOG_ERROR, "*** queue_pop(%d, %d) the wait_type is wrong", type, wait_type);
			break;
	}

	result = plat_semaphore_wait(g_tasks_sem[type], wait_time);
	ex_log(THREADM_LOG_LEVEL, "*** queue_pop() result = %d", result);
	if (result == THREAD_RES_OK) {
		return POP_OK;
	} else if (result == THREAD_RES_WAIT_TIMEOUT) {
		return POP_TIMEOUT;
	} else {
		ex_log(LOG_ERROR, "failed to wait type %d, result = %u", type, result);
		return POP_ABANDONED;
	}
}

int queue_add(queue_task_type_t task_type, task_t task)
{
	ex_log(THREADM_LOG_LEVEL, "*** queue_add(%d, %d) enter", task_type, task);
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
	ex_log(THREADM_LOG_LEVEL, "*** queue_add(%d, %d) end", task_type, task);
	return 0;
}
/*
* @brief Set a cancel task, to kill the running tasks.
* @param cancel_func, a callback from captain.c
 * @return  - NULL
*/
void thread_manager_set_cancel_func(rbsCancelFunc cancel_func) { g_pfcancel_lpAddress = cancel_func; }

/*
* * @brief Set the IDLE task. The idle task will be started whenever main task is ended.
*   @param lpAddress, a callback from captain.c,
*          usually we set navigation/power_consumption as IDLE task.
 * @return  - NULL
* */
void thread_manager_set_idle_task(do_operation_callback lpAddress) { g_pfidle_task_lpAddress = lpAddress; }

/**
 * @brief Initialization of thread_manager, create a main thread to do the other tasks.
 *        main thread is always alive.
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 */
int thread_manager_init(void)
{
	int retval;

	queue_init();

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
void thread_manager_uninitialize(void)
{
	int result;
	ex_log(LOG_DEBUG, "thread_manager_uninitialize enter");
	result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
	switch (result) {
		case POP_TIMEOUT:
			//start cancel
			ex_log(LOG_DEBUG, "+++ POP_TIMEOUT");
			if (!g_pfcancel_lpAddress) {
				ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
				break;
			}
			g_pfcancel_lpAddress(TRUE);
			ex_log(LOG_DEBUG, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ");
			queue_pop(AVAILABLE_Q, WAIT_FOREVER);
			ex_log(LOG_DEBUG, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ok");
			g_pfcancel_lpAddress(FALSE);
		case POP_OK:
			//add task
			ex_log(LOG_DEBUG, "+++ POP_OK");
			g_pflpAddress = NULL;
			queue_add(TASK_Q, TASK_EXIT);
			break;
		default:
			ex_log(LOG_ERROR, "Wait available return %u", result);
			break;
	}

	plat_thread_release(&g_thread_handle);
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

void thread_manager_run_task(do_operation_callback lpAddress, task_t task)
{
	unsigned int result;
	ex_log(LOG_DEBUG, "+++ thread_manager_run_task enter, task = %d", task);
	ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY) ");
	result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
	switch (result) {
		case POP_TIMEOUT:
			//start cancel
			ex_log(THREADM_LOG_LEVEL, "+++ POP_TIMEOUT");
			if (!g_pfcancel_lpAddress) {
				ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
				break;
			}
			g_pfcancel_lpAddress(TRUE);
			ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ");
			queue_pop(AVAILABLE_Q, WAIT_FOREVER);
			ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ok");
			g_pfcancel_lpAddress(FALSE);
		case POP_OK:
			//add task
			ex_log(THREADM_LOG_LEVEL, "+++ POP_OK");
			g_pflpAddress = lpAddress;
			queue_add(TASK_Q, task);
			break;
		default:
			ex_log(LOG_ERROR, "Wait available return %u", result);
			break;
	}
	ex_log(LOG_DEBUG, "+++ thread_manager_run_task end");
	return;
}

/*
* * @brief cancel_func, Ues the cancel callback task, to kill the running tasks.
*        If there is already a task running, it will wait until current task is ended.
 * @return  - NULL
*/

void thread_manager_cancel_task()
{
	int result;
	result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
	switch (result) {
		case POP_TIMEOUT:
			//start cancel
			ex_log(THREADM_LOG_LEVEL, "+++ POP_TIMEOUT");
			if (!g_pfcancel_lpAddress) {
				ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
				break;
			}
			g_pfcancel_lpAddress(TRUE);
			ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ");
			queue_pop(AVAILABLE_Q, WAIT_FOREVER);
			ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ok");
			g_pfcancel_lpAddress(FALSE);
		case POP_OK:
			//add task
			ex_log(THREADM_LOG_LEVEL, "+++ POP_OK");
			g_pflpAddress = NULL;
			queue_add(TASK_Q, TASK_CANCEL);
			break;
		default:
			ex_log(LOG_ERROR, "Wait available return %u", result);
			return;
	}
}

static void clear_task()
{
	queue_add(AVAILABLE_Q, 0);
}

static task_t get_task(BOOL idle_excuted)
{
	int result;
	ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(TASK_Q, %s) ", idle_excuted ? "WAIT_FOREVER" : "WAIT_IDLE");
	result = queue_pop(TASK_Q, idle_excuted ? WAIT_FOREVER : WAIT_IDLE);

	switch (result) {
		case POP_TIMEOUT:
			/* get AVAILABLE_Q, setup idle task as a TASK_*/
			ex_log(THREADM_LOG_LEVEL, "+++ POP_TIMEOUT");
			if (queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY) == POP_OK) {
				return TASK_IDLE;
			}
			/* can not get AVAILABLE_Q, someone hold it and preparing, keep waiting for task forever */
			queue_pop(TASK_Q, WAIT_FOREVER);
		case POP_OK:
			/* get TASK_Q */
			return g_task;
		default:
			ex_log(LOG_ERROR, "Wait available return %u", result);
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

static void *thread_manager(void)
{
	int result = 0;
	BOOL idle_excuted = FALSE;
	task_t task = TASK_EMPTY;

	while (1) {
		ex_log(THREADM_LOG_LEVEL, "NEW LOOP");
		task = get_task(idle_excuted);
		ex_log(THREADM_LOG_LEVEL, "going to do task = %d", task);
		/* make idle task excuted once in idle time*/
		if (task != TASK_IDLE)
			idle_excuted = FALSE;

		switch (task) {
			case TASK_EMPTY:
				ex_log(THREADM_LOG_LEVEL, "=== TASK_EMPTY");
				break;

			case TASK_IDLE:
				ex_log(LOG_DEBUG, "=== TASK_IDLE start");

				g_pfidle_task_lpAddress();
				idle_excuted = TRUE;
				ex_log(THREADM_LOG_LEVEL, "=== TASK_IDLE end");
				break;

			case TASK_PROCESS:
				ex_log(THREADM_LOG_LEVEL, "=== TASK_PROCESS start");
				if (g_pflpAddress == NULL) {
					ex_log(LOG_ERROR, "process cannot be excuted, pflpAddress=NULL");
					goto exit;
				}
				result = g_pflpAddress();
				ex_log(THREADM_LOG_LEVEL, "=== TASK_PROCESS end, result=%d", result);
				break;

			case TASK_CANCEL:
				/* defines for logger, works as the same as TASK_EMPTY*/
				ex_log(THREADM_LOG_LEVEL, "=== TASK_CANCEL");
				break;
			case TASK_EXIT:
				ex_log(THREADM_LOG_LEVEL, "=== TASK_EXIT");
				goto exit;
			default:
				ex_log(LOG_ERROR, "none excutable");

				break;
		}
		ex_log(THREADM_LOG_LEVEL, "task excuted");
		clear_task();
		ex_log(THREADM_LOG_LEVEL, "task excuted , clear");
	}
exit:
	ex_log(LOG_DEBUG, "thread_manager end, retval = %d", result);

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
int thread_try_lock_operation(void)
{
	int result;
	int retval = FINGERPRINT_RES_FAILED;

	ex_log(THREADM_LOG_LEVEL, "thread_try_lock_operation enter");

	if (g_in_opertation == 1) ex_log(LOG_ERROR, "during a operation");
	result = queue_pop(AVAILABLE_Q, WAIT_IMMEDIATELY);
	switch (result) {
		case POP_TIMEOUT:
			ex_log(THREADM_LOG_LEVEL, "+++ POP_TIMEOUT");
			if (!g_pfcancel_lpAddress) {
				ex_log(LOG_ERROR, "g_pfcancel_lpAddress is NULL ,cannot be used to cancel, break");
				break;
			}
			g_pfcancel_lpAddress(TRUE);
			ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ");
			queue_pop(AVAILABLE_Q, WAIT_FOREVER);
			ex_log(THREADM_LOG_LEVEL, "+++ queue_pop(AVAILABLE_Q, WAIT_FOREVER) ok");
			g_pfcancel_lpAddress(FALSE);
		case POP_OK:
			ex_log(THREADM_LOG_LEVEL, "+++ POP_OK");
			retval = FINGERPRINT_RES_SUCCESS;
			g_in_opertation = 1;
			break;
		default:
			ex_log(LOG_ERROR, "Wait available return %u", result);
			break;
	}

	ex_log(THREADM_LOG_LEVEL, "thread_try_lock_operation end, retval = %d", retval);
	return retval;
}

/**
 * @brief Unlock the main thread.
 * 
 * @return  - NULL
 */
void thread_unlock_operation(void)
{
	if (g_in_opertation == 0)
		ex_log(LOG_ERROR, "not in a operation");
	g_in_opertation =0;
	ex_log(THREADM_LOG_LEVEL, "thread_unlock_operation enter");
	queue_add(TASK_Q, TASK_EMPTY);
	ex_log(THREADM_LOG_LEVEL, "thread_unlock_operation end");
}

/**
 * @brief trylock a operation thread. for other thread.
 * 
 * @param operation, operation thread name which needs to trylock.
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 *         Fail    - FINGERPRINT_RES_FAILED
 */
int trylock_operation(rbs_operation_t operation)
{
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

	ex_log(THREADM_LOG_LEVEL, "trylock_operation = %d, retval = %d", operation, retval);
	return retval;
}

/**
 * @brief unlock a operation thread. for other thread.
 * 
 * @param operation, operation thread name which needs to unlock.
 * @return SUCCESS - FINGERPRINT_RES_SUCCESS
 *         Fail    - FINGERPRINT_RES_FAILED
 */
int unlock_operation(rbs_operation_t operation)
{
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

	ex_log(THREADM_LOG_LEVEL, "unlock_operation = %d ,ret %d", operation, retval);
	return retval;
}
