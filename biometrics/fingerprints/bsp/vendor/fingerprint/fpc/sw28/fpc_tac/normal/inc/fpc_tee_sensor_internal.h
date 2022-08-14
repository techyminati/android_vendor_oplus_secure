
/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include "fpc_tee_sensor.h"
#include "fpc_tee.h"
#include "fpc_irq_device.h"
#include "fpc_reset_device.h"
#include "pthread.h"

struct fpc_tee_sensor {
    fpc_tee_t* tee;
    pthread_mutex_t mutex;
    int cancelled;
    fpc_irq_t* irq;
    fpc_reset_t* reset;
    int32_t cac_result;
    int alive_check;
};
