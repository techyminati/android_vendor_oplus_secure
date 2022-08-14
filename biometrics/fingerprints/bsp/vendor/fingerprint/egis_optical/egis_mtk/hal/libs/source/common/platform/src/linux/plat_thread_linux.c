#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "plat_thread.h"
#include "plat_log.h"
#include "plat_mem.h"

#define LOG_TAG "RBS"

int plat_thread_create_ex(thread_handle_t* handle, void* routine, void* arg)
{
	int retval;

	if (handle == NULL || routine == NULL) {
		ex_log(LOG_ERROR, "plat_thread_create invalid param");
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->hlinux != 0) {
		//ex_log(LOG_INFO, "plat_thread_create (*handle) != NULL != 0");
		return THREAD_RES_OK;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	retval = pthread_create((pthread_t*)&handle->hlinux, NULL, routine, arg);
	if (retval != 0) {
		ex_log(LOG_ERROR, "pthread_create failed ,retval = %d, error = %d", retval, errno);
		pthread_attr_destroy(&attr);
		return THREAD_ERR_CREATE_FAILED;
	}

	pthread_attr_destroy(&attr);

	return THREAD_RES_OK;
}

int plat_thread_create(thread_handle_t* handle, void* routine)
{
	return plat_thread_create_ex(handle, routine, NULL);
}

int plat_thread_release(thread_handle_t* handle)
{
	int retval;

	if (handle == NULL) {
		ex_log(LOG_ERROR, "plat_thread_release handle == NULL");
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->hlinux == 0) {
		ex_log(LOG_INFO, "plat_thread_release handle->hlinux == 0, thread has been closed");
		return THREAD_RES_OK;
	}

	retval = pthread_join((pthread_t)handle->hlinux, NULL);
	if (retval != 0) {  //show errno
		ex_log(LOG_ERROR, "pthread_join return failed, errno = %d", errno);
	}
	handle->hlinux = 0;
	return retval;
}

int plat_mutex_create(mutex_handle_t* handle)
{
	int retval;
	if (handle == NULL) {
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->mutex != NULL) {
		egislog_d(
		    "plat_mutex_create handle->mutex != NULL, mutex has already "
		    "been created");
		return THREAD_RES_OK;
	}

	handle->mutex = plat_alloc(sizeof(pthread_mutex_t));
	retval = pthread_mutex_init((pthread_mutex_t*)handle->mutex, NULL);
	if (retval != 0) {
		egislog_e("plat_mutex_create pthread_mutex_init failed, errno = %d",
		          errno);
		plat_free(handle->mutex);
		handle->mutex = NULL;
		return THREAD_ERR_CREATE_FAILED;
	}

	return THREAD_RES_OK;
}

int plat_mutex_release(mutex_handle_t* handle)
{
	int retval;
	if (handle == NULL) {
		ex_log(LOG_ERROR, "plat_mutex_release handle == NULL");
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->mutex == NULL) {
		ex_log(LOG_INFO,
		       "plat_mutex_release handle->mutex == NULL, mutex has already "
		       "been closed");
		return THREAD_RES_OK;
	}

	retval = pthread_mutex_destroy((pthread_mutex_t*)handle->mutex);
	plat_free(handle->mutex);
	handle->mutex = NULL;
	return retval;
}

int plat_mutex_lock(mutex_handle_t handle)
{
	int retval;
	if (handle.mutex == NULL) {
		return THREAD_ERR_INVALID_PARAM;
	}

	retval = pthread_mutex_lock((pthread_mutex_t*)handle.mutex);
	if (retval == 0) return THREAD_RES_OK;

	switch (errno) {
		case EDEADLK:
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

int plat_mutex_trylock(mutex_handle_t handle)
{
	int retval;
	if (handle.mutex == NULL) {
		return THREAD_ERR_INVALID_PARAM;
	}

	retval = pthread_mutex_trylock((pthread_mutex_t*)handle.mutex);
	if (retval == 0) return THREAD_RES_OK;

	switch (errno) {
		case EBUSY:
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

int plat_mutex_unlock(mutex_handle_t handle)
{
	int retval;
	if (handle.mutex == NULL) {
		return THREAD_ERR_INVALID_PARAM;
	}

	retval = pthread_mutex_unlock((pthread_mutex_t*)handle.mutex);
	if (retval == 0) return THREAD_RES_OK;

	switch (errno) {
		case EPERM:
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

int plat_semaphore_create(semaphore_handle_t* handle, unsigned int initial_cnt, unsigned int max_cnt)
{
	int retval;
	if (handle == NULL || initial_cnt > max_cnt) {
		return THREAD_ERR_INVALID_PARAM;
	}

	if (handle->sema != NULL) {
		// ex_log(LOG_INFO, "plat_semaphore_create handle->sema != NULL, semaphore has already been created");
		return THREAD_RES_OK;
	}
	handle->sema = plat_alloc(sizeof(sem_t));
	retval = sem_init((sem_t*)handle->sema, 0, initial_cnt);
	if (retval != 0) {
		ex_log(LOG_ERROR, "plat_semaphore_create sem_init failed, errno = %d", errno);
		plat_free(handle->sema);
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
	plat_free(handle->sema);
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
