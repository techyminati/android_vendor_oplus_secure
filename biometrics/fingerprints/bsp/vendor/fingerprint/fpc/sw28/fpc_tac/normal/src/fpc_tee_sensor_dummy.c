/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include "fpc_types.h"
#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_tee_sensor.h"

fpc_tee_sensor_t* fpc_tee_sensor_init(fpc_tee_t* sensor)
{
    return (fpc_tee_sensor_t*) sensor;
}

void fpc_tee_sensor_release(fpc_tee_sensor_t* sensor)
{
    (void)sensor;
    LOGE("%s, dummy implementation!", __func__);
}

int fpc_tee_capture_image(fpc_tee_sensor_t* sensor, uint8_t wait_for_finger)
{
    (void)sensor;
    (void) wait_for_finger;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}

int fpc_tee_wait_finger_lost(fpc_tee_sensor_t* sensor)
{
    (void)sensor;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_set_cancel(fpc_tee_sensor_t* sensor)
{
    (void)sensor;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_clear_cancel(fpc_tee_sensor_t* sensor)
{
    (void)sensor;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_wait_irq(fpc_tee_sensor_t* sensor, int irq_value)
{
    (void)sensor;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_sensor_cancelled(fpc_tee_sensor_t* sensor)
{
    (void)sensor;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
