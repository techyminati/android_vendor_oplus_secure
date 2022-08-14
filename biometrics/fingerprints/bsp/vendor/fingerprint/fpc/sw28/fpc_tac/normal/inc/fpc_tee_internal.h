/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_INTERNAL_H
#define FPC_TEE_INTERNAL_H

#include "fpc_tac.h"
#include "fpc_tee.h"

struct fpc_tee {
    fpc_tac_t* tac;
    fpc_tac_shared_mem_t* shared_buffer;
    uint32_t supported_features;
    uint32_t log_to_file_buffer_size;
    uint32_t nav_debug_file_buffer_size;
    uint32_t max_number_of_templates;
    char manifest_tag[64];
};

#endif // FPC_TEE_INTERNAL_H

