/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */
#ifndef FPC_TEE_DB_BLOB_TEST_H
#define FPC_TEE_DB_BLOB_TEST_H

#include "fpc_tee.h"

static int __attribute__((__unused__)) assert_passed = 0;
static int __attribute__((__unused__)) assert_failed = 0;

#define ASSERT_EQUAL(actual, expected)                                                                            \
do {                                                                                                              \
    if ((actual) != (expected)) {                                                                                 \
        LOGE("[FAIL] %s:%d - Expected %d. Got %d\n", __func__, __LINE__, (int32_t)(expected), (int32_t)(actual)); \
        assert_failed++;                                                                                          \
    } else {                                                                                                      \
        LOGE("[PASS] %s:%d\n", __func__, __LINE__);                                                               \
        assert_passed++;                                                                                          \
    }                                                                                                             \
} while (0);

#define ASSERT_TRUE(result)                         \
do {                                                \
    if (result) {                                   \
        LOGE("[PASS] %s:%d\n", __func__, __LINE__); \
        assert_passed++;                            \
    } else {                                        \
        LOGE("[FAIL] %s:%d\n", __func__, __LINE__); \
        assert_failed++;                            \
    }                                               \
} while (0);

int test_fpc_tee_db_blob(fpc_tee_t* tee);

#endif /* FPC_TEE_DB_BLOB_TEST_H */

