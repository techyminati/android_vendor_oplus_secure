/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TAC2_H
#define FPC_TAC2_H

#include <stdint.h>
#include "fpc_types.h"

typedef struct fpc_tac fpc_tac_t;

/**
 * @brief fpc_tac_open open a connection to the ta.
 * @return the tac or NULL on failure.
 */
fpc_tac_t* fpc_tac_open(void);

/**
 * @brief fpc_tac_release close the connection to the ta.
 */
void fpc_tac_release(fpc_tac_t* tac);


typedef struct fpc_tac_shared_mem {
    void* addr;
} fpc_tac_shared_mem_t;

fpc_tac_shared_mem_t* fpc_tac_alloc_shared(fpc_tac_t* tac, uint32_t size);

void fpc_tac_free_shared(fpc_tac_shared_mem_t* shared_buffer);

/**
 * transfer control to the ta, the entire content of the shared buffer shall
 * be treated as both input/output.
 */
int fpc_tac_transfer(fpc_tac_t* tac, fpc_tac_shared_mem_t* shared_buffer);


#endif // FPC_TAC2_H
