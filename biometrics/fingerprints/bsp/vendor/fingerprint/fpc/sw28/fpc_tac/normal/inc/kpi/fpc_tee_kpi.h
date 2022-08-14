/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_KPI_H
#define FPC_TEE_KPI_H
#include "fpc_tee.h"

void fpc_tee_kpi_start(fpc_tee_t *tee);
void fpc_tee_kpi_stop(fpc_tee_t *tee);

/**
 * @brief fpc_tee_kpi_start_sequence buffers kpi data in memory.
 *
 * For the first call, new_sequence should be set to TRUE. This will setup the new sequence.
 * To end the collection/sequence of kpi data and save the kpi data to file call
 * fpc_tee_kpi_stop_sequence(fpc_tee_t *tee, bool new_sequence)
 *
 * @param tee                Instance pointer to tee
 * @param new_sequence       When TRUE, setup the new sequence
 *                           when FALSE, continue to add kpi data on current sequence
 */
void fpc_tee_kpi_start_sequence(fpc_tee_t *tee, bool new_sequence);

/**
 * @brief fpc_tee_kpi_stop_sequence save kpi data to disk.
 *
 * Before calling fpc_tee_kpi_stop_sequence, fpc_tee_kpi_start_sequence should have been called
 * atleast once.
 * Will stop the sequence of kpi data and the kpi data buffered from calls to
 * fpc_tee_kpi_start_sequence will be saved to disk.
 *
 * @param tee                Instance pointer to tee
 */
void fpc_tee_kpi_stop_sequence(fpc_tee_t *tee);

#endif //FPC_TEE_KPI_H
