/*
* Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TYPES
#define INCLUSION_GUARD_FPC_TYPES

#include "fpc_error.h"

/*
 * The following enum is used to determine the file where an error occurred.
 * The value is saved in the internal, module specific bits in a return value
 * when the "Module ID" bits are set to FPC_MODULE_ID_TA
 */
typedef enum {
    FPC_TA_INTERNAL_FILE_APP_MAIN             = 0,
    FPC_TA_INTERNAL_FILE_ROUTER,             // 1
    FPC_TA_INTERNAL_FILE_COMMON,             // 2
    FPC_TA_INTERNAL_FILE_TA_DB_BLOB,         // 3
    FPC_TA_INTERNAL_FILE_SENSOR,             // 4
    FPC_TA_INTERNAL_FILE_KPI,                // 5
    FPC_TA_INTERNAL_FILE_NAVIGATION,         // 6
    FPC_TA_INTERNAL_FILE_TEST,               // 7
    FPC_TA_INTERNAL_FILE_ENG,                // 8
    FPC_TA_INTERNAL_FILE_FIDO_AUTH,            // 9
    FPC_TA_INTERNAL_FILE_BIO,                // 10
    FPC_TA_INTERNAL_FILE_PN,                 // 11
    FPC_TA_INTERNAL_FILE_CRYPTO,             // 12
    FPC_TA_INTERNAL_FILE_HW_AUTH,            // 13
    FPC_TA_INTERNAL_FILE_HW_AUTH_PLATFORM,   // 14
    FPC_TA_INTERNAL_FILE_INTERNAL_TEST,      // 15

    FPC_TA_INTERNAL_FILE_MAX = 63,
} fpc_ta_internal_file_t;

#define FPC_TA_ERROR(external)                                                                \
                     (FPC_ERROR_SET_ERROR_GROUP(1)                                          | \
                      FPC_ERROR_SET_MODULE_GROUP(FPC_MODULE_ID_TA)                          | \
                      FPC_ERROR_SET_EXTERNAL_GROUP(external)                                | \
                      FPC_ERROR_SET_INTERNAL_GROUP(CURRENT_FILE_ID)                         | \
                      FPC_ERROR_SET_EXTRA_GROUP(__LINE__))

#endif // INCLUSION_GUARD_FPC_TYPES
