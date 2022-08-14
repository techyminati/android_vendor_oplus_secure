#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "plat_thread.h"
#include "plat_log.h"
#include "plat_mem.h"

int plat_thread_create(thread_handle_t* handle, void* routine)
{
	int retval;

	if (handle == NULL || routine == NULL) {
		ex_log(LOG_ERROR, "plat_thread_create invalid param");
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->hlinux != 0) {
		ex_log(LOG_INFO, "plat_thread_create (*handle) != NULL != 0");
		return THREAD_RES_OK;
	}

	retval = pthread_create((pthread_t*)&handle->hlinux, NULL, routine, NULL);
	if (retval != 0) {
		ex_log(LOG_ERROR, "pthread_create failed ,retval = %d, error = %d", retval, errno);
		return THREAD_ERR_CREATE_FAILED;
	}

	pthread_detach((pthread_t)handle->hlinux);

	return THREAD_RES_OK;
}

int plat_thread_release(thread_handle_t* handle)
{
	int retval;

	if (handle == NULL) {
		ex_log(LOG_ERROR, "plat_thread_release handle == NULL");
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->hlinux == 0) {
		ex_log(LOG_INFO, "plat_thread_release handle->hwin == NULL, thread has been closed");
		return THREAD_RES_OK;
	}

	retval = pthread_join((pthread_t)handle->hlinux, NULL);
	if (retval != 0) {  //show errno
		ex_log(LOG_ERROR, "pthread_join return failed, errno = %d", errno);
	}
	handle->hlinux = 0;
	return retval;
}

int plat_semaphore_create(semaphore_handle_t* handle, unsigned int initial_cnt, unsigned int max_cnt)
{
	int retval;
	if (handle == NULL || initial_cnt > max_cnt) {
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->sema != NULL) {
		ex_log(LOG_INFO, "plat_semaphore_create handle->sema != NULL, semaphore has already been created");
		return THREAD_RES_OK;
	}
	handle->sema = mem_alloc(sizeof(sem_t));
	retval = sem_init((sem_t*)handle->sema, 0, initial_cnt);
	if (retval != 0) {
		ex_log(LOG_ERROR, "plat_semaphore_create sem_init failed, errno = %d", errno);
		mem_free(handle->sema);
		handle->sema = NULL;
		return THREAD_ERR_CREATE_FAILED;
	}
	return THREAD_RES_OK;
}

int plat_semaphore_release(semaphore_handle_t* handle)
{
	int retval;
	if (handle == NULL) {
		ex_log(LOG_ERROR, "plat_semaphore_release handle == NULL");
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->sema == NULL) {
		ex_log(LOG_INFO, "plat_semaphore_release handle->sema == NULL, semaphore has already been closed");
		return THREAD_RES_OK;
	}

	retval = sem_destroy((sem_t*)handle->sema);
	mem_free(handle->sema);
	handle->sema = NULL;
	return retval;
}

int plat_semaphore_wait(semaphore_handle_t handle, int wait_time)
{
	int retval;
	struct timespec ts;
	long ms_to_sec, nanosec, overflow;

#define SEC_TO_MILLISEC (1000)
#define SEC_TO_NANOSEC (1000 * 1000 * 1000)
#define MILLISEC_TO_NANOSEC (1000 * 1000)

	if (handle.sema == NULL) {
		ex_log(LOG_ERROR, "one method was called before thread manager init");
		return THREAD_RES_OK;
	}

	if (wait_time <= -1) {
		retval = sem_wait((sem_t*)handle.sema);
	} else if (wait_time == 0) {
		retval = sem_trywait((sem_t*)handle.sema);
	} else {
		clock_gettime(CLOCK_REALTIME, &ts);
		ms_to_sec = wait_time / SEC_TO_MILLISEC;
		wait_time = wait_time % SEC_TO_MILLISEC;
		ts.tv_sec += ms_to_sec;

		nanosec = (wait_time * MILLISEC_TO_NANOSEC) + ts.tv_nsec;
		overflow = nanosec / SEC_TO_NANOSEC;
		ts.tv_sec += overflow;
		ts.tv_nsec = nanosec % SEC_TO_NANOSEC;
		retval = sem_timedwait((sem_t*)handle.sema, &ts);
	}

	if (retval == 0)
		return THREAD_RES_OK;

	switch (errno) {
		case ETIMEDOUT:
		case EAGAIN:
			retval = THREAD_RES_WAIT_TIMEOUT;
			break;
		case EINVAL:
			retval = THREAD_ERR_NOT_EXSIT;
			break;
		default:
			retval = THREAD_ERR_FAILED;
			break;
	}
	return retval;
}

int plat_semaphore_post(semaphore_handle_t handle)
{
	if (handle.sema == NULL) {
		ex_log(LOG_ERROR, "%s, semaphore is not created!", __func__);
		return THREAD_ERR_INVALID_PARAM;
	}

	return sem_post((sem_t*)handle.sema);
}
