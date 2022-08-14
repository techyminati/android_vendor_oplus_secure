/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#include <inttypes.h>
#include <stdint.h>
#include "fpc_types.h"
#include "fpc_error_str.h"
#include <fpc_log.h>

const char *fpc_error_str(int err)
{
    int external_err = FPC_ERROR_GET_EXTERNAL_ERROR(err);

    switch (external_err) {
    case FPC_STATUS_WAIT_TIME:
        return "FPC_STATUS_WAIT_TIME";
    case FPC_STATUS_FINGER_PRESENT:
        return "FPC_STATUS_FINGER_PRESENT";
    case FPC_STATUS_FINGER_LOST:
        return "FPC_STATUS_FINGER_LOST";
    case FPC_STATUS_BAD_QUALITY:
        return "FPC_STATUS_BAD_QUALITY";
    case FPC_STATUS_FINGER_ALREADY_ENROLLED:
        return "FPC_STATUS_FINGER_ALREADY_ENROLLED";
    case FPC_STATUS_ENROLL_PROGRESS:
        return "FPC_STATUS_ENROLL_PROGRESS";
    case FPC_STATUS_ENROLL_LOW_COVERAGE:
        return "FPC_STATUS_ENROLL_LOW_COVERAGE";
    case FPC_STATUS_ENROLL_TOO_SIMILAR:
        return "FPC_STATUS_ENROLL_TOO_SIMILAR";
    case FPC_STATUS_ENROLL_LOW_QUALITY:
        return "FPC_STATUS_ENROLL_LOW_QUALITY";
    case FPC_STATUS_ENROLL_LOW_MOBILITY:
        return "FPC_STATUS_ENROLL_LOW_MOBILITY";
    case -FPC_ERROR_NOT_FOUND:
        return "FPC_ERROR_NOT_FOUND";
    case -FPC_ERROR_PAL:
        return "FPC_ERROR_PAL";
    case -FPC_ERROR_IO:
        return "FPC_ERROR_IO";
    case -FPC_ERROR_CANCELLED:
        return "FPC_ERROR_CANCELLED";
    case -FPC_ERROR_UNKNOWN:
        return "FPC_ERROR_UNKNOWN";
    case -FPC_ERROR_MEMORY:
        return "FPC_ERROR_MEMORY";
    case -FPC_ERROR_PARAMETER:
        return "FPC_ERROR_PARAMETER";
    case -FPC_ERROR_TEST_FAILED:
        return "FPC_ERROR_TEST_FAILED";
    case -FPC_ERROR_TIMEDOUT:
        return "FPC_ERROR_TIMEDOUT";
    case -FPC_ERROR_SENSOR:
        return "FPC_ERROR_SENSOR";
    case -FPC_ERROR_SPI:
        return "FPC_ERROR_SPI";
    case -FPC_ERROR_NOT_SUPPORTED:
        return "FPC_ERROR_NOT_SUPPORTED";
    case -FPC_ERROR_OTP:
        return "FPC_ERROR_OTP";
    case -FPC_ERROR_STATE:
        return "FPC_ERROR_STATE";
    case -FPC_ERROR_PN:
        return "FPC_ERROR_PN";
    case -FPC_ERROR_DEAD_PIXELS:
        return "FPC_ERROR_DEAD_PIXELS";
    case -FPC_ERROR_TEMPLATE_CORRUPTED:
        return "FPC_ERROR_TEMPLATE_CORRUPTED";
    case -FPC_ERROR_CRC:
        return "FPC_ERROR_CRC";
    case -FPC_ERROR_STORAGE:
        return "FPC_ERROR_STORAGE";
    case -FPC_ERROR_MAXIMUM_REACHED:
        return "FPC_ERROR_MAXIMUM_REACHED";
    case -FPC_ERROR_MINIMUM_NOT_REACHED:
        return "FPC_ERROR_MINIMUM_NOT_REACHED";
    case -FPC_ERROR_SENSOR_LOW_COVERAGE:
        return "FPC_ERROR_SENSOR_LOW_COVERAGE";
    case -FPC_ERROR_SENSOR_LOW_QUALITY:
        return "FPC_ERROR_SENSOR_LOW_QUALITY";
    case -FPC_ERROR_SENSOR_FINGER_NOT_STABLE:
        return "FPC_ERROR_SENSOR_FINGER_NOT_STABLE";
    case -FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS:
        return "FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS";
    case -FPC_ERROR_ALREADY_ENROLLED:
        return "FPC_ERROR_ALREADY_ENROLLED";
    default:
        return "UNKNOWN ERROR";
    }
}
