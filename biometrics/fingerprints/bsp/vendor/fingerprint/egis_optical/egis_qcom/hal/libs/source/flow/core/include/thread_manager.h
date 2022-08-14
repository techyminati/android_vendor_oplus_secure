#ifndef __THREAD_MANAGER__
#define __THREAD_MANAGER__
#include "type_def.h"

typedef enum rbs_operation {
	RBS_MUTEX_TRANSFER,
} rbs_operation_t;

typedef enum task {
	TASK_EMPTY,
	TASK_IDLE,
	TASK_PROCESS,
	TASK_CANCEL,
	TASK_EXIT
} task_t;

typedef int (*do_operation_callback)();
typedef void (*rbsCancelFunc)(BOOL cancel_flag);

int trylock_operation(rbs_operation_t operation);

int unlock_operation(rbs_operation_t operation);

int thread_manager_init(void);

void thread_manager_uninitialize(void);

void thread_manager_run_task(do_operation_callback lpAddress, task_t task);

void thread_manager_cancel_task();

void thread_manager_set_cancel_func(rbsCancelFunc cancel_func);

void thread_manager_set_idle_task(do_operation_callback lpAddress);

int thread_try_lock_operation(void);

void thread_unlock_operation(void);

#endif