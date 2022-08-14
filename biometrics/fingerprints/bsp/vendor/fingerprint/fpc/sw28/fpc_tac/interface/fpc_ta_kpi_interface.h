/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef INCLUSION_GUARD_FPC_TA_KPI_INTERFACE
#define INCLUSION_GUARD_FPC_TA_KPI_INTERFACE

#include "fpc_ta_interface.h"

typedef union _fpc_ta_kpi_command_t
{
    fpc_ta_cmd_header_t header;
    fpc_ta_byte_array_msg_t kpi_ctrl;
} fpc_ta_kpi_command_t;

typedef enum {
    FPC_TA_KPI_STOP_CMD,
    FPC_TA_KPI_START_CMD,
} fpc_kpi_cmd_t;
#endif
