//
//    Copyright 2017 Egis Technology Inc.
// 
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Egis in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//

#ifndef __PLAT_THREAD_H_
#define __PLAT_THREAD_H_

typedef enum plat_thread_result {
	THREAD_RES_OK,
	THREAD_RES_WAIT_TIMEOUT,
	THREAD_ERR_FAILED,
	THREAD_ERR_INVALID_PARAM,
	THREAD_ERR_CREATE_FAILED,
	THREAD_ERR_NOT_EXSIT,
} plat_thread_result_t;

typedef union thread_handle {
	unsigned long int hlinux;
	void* hwin;
	void* htz;
	void* param;
} thread_handle_t;

typedef struct semaphore_handle {
	void* sema;
} semaphore_handle_t;

typedef struct mutex_handle {
	void* mutex;
} mutex_handle_t;

int plat_thread_create(thread_handle_t* handle, void* routine);
int plat_thread_create_ex(thread_handle_t* handle, void* routine, void* arg);
int plat_thread_release(thread_handle_t* handle);
int plat_semaphore_create(semaphore_handle_t* handle, unsigned int initial_cnt, unsigned int max_cnt);
int plat_semaphore_release(semaphore_handle_t* handle);
int plat_mutex_create(mutex_handle_t* handle);
int plat_mutex_release(mutex_handle_t* handle);
/**
 * plat_semaphore_wait
 * 
 * @param handle
 * 	semaphore_handle_t object
 * @param wait_time
 * the waiting shall be terminated when the specified timeout expires. 
 * 	[-1]	Infinite
 * 	[0 ]	Immediately
 * 	[>0]	wait for @wait_time MicroSeconds
 * @return
 * 	[THREAD_RES_OK]	
 * 	[THREAD_RES_WAIT_TIMEOUT]
 * 	[THREAD_ERR_FAILED] 
 */
int plat_semaphore_wait(semaphore_handle_t handle, int wait_time);
int plat_semaphore_post(semaphore_handle_t handle);

/**
 * plat_mutex_lock
 * 
 * @param handle
 * 	mutex_handle_t object
 * @return
 * 	[THREAD_RES_OK]	
 * 	[THREAD_RES_WAIT_TIMEOUT]
 * 	[THREAD_ERR_FAILED] 
 */
int plat_mutex_lock(mutex_handle_t handle);

/**
 * plat_mutex_trylock
 * 
 * @param handle
 * 	mutex_handle_t object
 * @return
 * 	[THREAD_RES_OK]	
 * 	[THREAD_RES_WAIT_TIMEOUT]
 * 	[THREAD_ERR_FAILED] 
 */
int plat_mutex_trylock(mutex_handle_t handle);

/**
 * plat_mutex_unlock
 * 
 * @param handle
 * 	mutex_handle_t object
 * @return
 * 	[THREAD_RES_OK]	
 * 	[THREAD_RES_WAIT_TIMEOUT]
 * 	[THREAD_ERR_FAILED] 
 */
int plat_mutex_unlock(mutex_handle_t handle);

#endif