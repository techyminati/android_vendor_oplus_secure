/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_HW_AUTH_H
#define FPC_TEE_HW_AUTH_H

#include <stdint.h>
#include <errno.h>
#include "fpc_tee.h"

int fpc_tee_init_hw_auth(fpc_tee_t* tee);
int fpc_tee_set_auth_challenge(fpc_tee_t* tee, uint64_t challenge);
int fpc_tee_get_enrol_challenge(fpc_tee_t* tee, uint64_t* challenge);
int fpc_tee_authorize_enrol(fpc_tee_t* tee, const uint8_t* token,
                            uint32_t size_token);
int fpc_tee_get_auth_result(fpc_tee_t* tee, uint8_t* token,
                            uint32_t size_token);
int fpc_tee_init_hw_auth(fpc_tee_t* tee);

/**
 * Enable/disable token validation
 *
 * This function makes it possible to disable token validation. Disabling
 * token validation is a security risk and is only available in engineering builds.
 *
 * @param[in] self    Handle to engineering instance
 * @param[in] enabled true to enabled token validation (default), false to disable it.
 */
int fpc_tee_set_token_validation_enable(fpc_tee_t *tee, bool enabled);

#endif // FPC_TEE_HW_AUTH_H

