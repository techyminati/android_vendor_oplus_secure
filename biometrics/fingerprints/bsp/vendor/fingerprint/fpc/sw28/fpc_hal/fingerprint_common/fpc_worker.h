/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_WORKER_H
#define FPC_WORKER_H

typedef struct fpc_worker fpc_worker_t;

void fpc_worker_run_task(fpc_worker_t *worker, void (*func)(void *), void *arg);
void fpc_worker_join_task(fpc_worker_t *worker);
fpc_worker_t *fpc_worker_new(void);
void fpc_worker_destroy(fpc_worker_t *worker);

#endif // FPC_WORKER_H

