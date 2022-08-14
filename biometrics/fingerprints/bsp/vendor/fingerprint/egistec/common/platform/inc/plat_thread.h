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
} thread_handle_t;
typedef struct semaphore_handle {
	void* sema;
} semaphore_handle_t;
/*typedef union semaphore_handle {
	long int hlinux;
	void* hwin;
} semaphore_handle_t;
*/
int plat_thread_create(thread_handle_t* handle, void* routine);
int plat_thread_release(thread_handle_t* handle);
int plat_semaphore_create(semaphore_handle_t* handle, unsigned int initial_cnt, unsigned int max_cnt);
int plat_semaphore_release(semaphore_handle_t* handle);
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

#endif