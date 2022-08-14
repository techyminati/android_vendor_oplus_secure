/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdio.h>
#include "fpc_tee_db_blob_test.h"
#include <stddef.h>
#include "fpc_log.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_ta_targets.h"
#include "fpc_tac.h"
#include "fpc_types.h"
#include "fpc_tee_internal.h"

/*
 * Creates and sends a open command to the TA
 *
 * @param tee   tee handle
 * @param mode  Open mode of the transfer. Valid values are FPC_TA_BIO_DB_RDONLY or FPC_TA_BIO_DB_WRONLY
 * @param size  Expected size (possibly fake) to be transfered between REE and TEE
  *
 * @return The TA response is returned. 0 indicates OK. Negative on error.
 *
 */
static int32_t send_open_command(fpc_tee_t* tee, uint32_t mode, uint32_t size)
{
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;
    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_OPEN_CMD;
    command->db_open.mode   = mode;
    command->db_open.size   = size;

    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

/*
 * Creates and sends a close command to the TA
 *
 * @param tee   tee handle
 *
 * @return The TA response is returned. 0 indicates OK. Negative on error.
 *
 */
static int32_t send_close_command(fpc_tee_t* tee)
{
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_CLOSE_CMD;

    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

/*
 * Creates and sends a write command to the TA
 *
 * @param tee               tee handle
 * @param payload_size      Payload size that is allocated in shared memory
 * @param populated_size    Payload size (possibly fake) that will be communicated in command to TA
 *                          A proper message would have payload_size == populated_size
 *
 * @return The TA response is returned. 0 indicates OK. Negative on error.
 *
 */
static int32_t send_write_command(fpc_tee_t* tee, uint32_t payload_size, uint32_t populated_size)
{
    int result = 0;
    const uint32_t message_size = (uint32_t)(payload_size + sizeof(fpc_ta_bio_command_t));
    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    ASSERT_TRUE(shared_buffer != NULL);
    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_WRITE_CMD;
    command->db_write.size        = populated_size;

    result = fpc_tac_transfer(tee->tac, shared_buffer);
    fpc_tac_free_shared(shared_buffer);
    return result;
}

/*
 * Creates and sends a read command to the TA
 *
 * @param tee               tee handle
 * @param payload_size      Payload size that is allocated in shared memory
 * @param populated_size    Payload size (possibly fake) we tell the TA we have allocated
 *                          A proper message would have payload_size == populated_size
 *
 * @return The TA response is returned. 0 indicates OK. Negative on error.
 *
 */
static int32_t send_read_command(fpc_tee_t* tee, uint32_t payload_size, uint32_t populated_size)
{
    int result = 0;
    const uint32_t message_size = (uint32_t)(payload_size + sizeof(fpc_ta_bio_command_t));
    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    ASSERT_TRUE(shared_buffer != NULL);
    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_READ_CMD;
    command->db_read.size         = populated_size;

    result = fpc_tac_transfer(tee->tac, shared_buffer);
    fpc_tac_free_shared(shared_buffer);
    return result;
}

/* Test for the open/close/read/write scenarios */
static int test_transfer_open(fpc_tee_t* tee)
{
    LOGD("%s", __func__);
    int result;

    result = send_open_command(tee, FPC_TA_BIO_DB_RDONLY, 4096);
    ASSERT_EQUAL(result, 0);

    // Open when transfer is already open, should fail
    result = send_open_command(tee, FPC_TA_BIO_DB_RDONLY, 4096);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_IO);

    // Close in order to progress testing
    result = send_close_command(tee);
    ASSERT_EQUAL(result, 0);

    // Try to open transfer size 0
    result = send_open_command(tee, FPC_TA_BIO_DB_RDONLY, 0);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Try to open transfer size exceeding limit.
    // TODO: Expose limit in order to include for test?
    result = send_open_command(tee, FPC_TA_BIO_DB_RDONLY, (3 << 20) + 1);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Open in Write mode
    result = send_open_command(tee, FPC_TA_BIO_DB_WRONLY, 4096);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), 0);

    // Open in read mode (already open in write mode)
    result = send_open_command(tee, FPC_TA_BIO_DB_WRONLY, 4096);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_IO);

    // Close in order to progress testing
    result = send_close_command(tee);
    ASSERT_EQUAL(result, 0);

    // Open/close multiple times within valid values
    for (uint32_t i=1; i <= 100; i++) {
        result = send_open_command(tee, FPC_TA_BIO_DB_WRONLY, 1000*i);
        ASSERT_EQUAL(result, 0);
        result = send_close_command(tee);
        ASSERT_EQUAL(result, 0);
    }

    // Mode other than FPC_TA_BIO_DB_RDONLY/FPC_TA_BIO_DB_WRONLY
    result = send_open_command(tee, 8888, 4096);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Consecutive close operations should be harmless
    for (int i=0; i < 5; i++) {
        result = send_close_command(tee);
        ASSERT_EQUAL(result, 0);
    }

    return 0;
}

static int test_transfer_read(fpc_tee_t* tee)
{
    LOGD("%s", __func__);
    int result;

    const uint32_t READ_SIZE = 4096;

    // Open in write mode
    result = send_open_command(tee, FPC_TA_BIO_DB_WRONLY, READ_SIZE);
    ASSERT_EQUAL(result, 0);

    // Cannot read in write mode
    result = send_read_command(tee, READ_SIZE, READ_SIZE);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Close in order to progress testing
    result = send_close_command(tee);
    ASSERT_EQUAL(result, 0);

    // Open in read mode
    result = send_open_command(tee, FPC_TA_BIO_DB_RDONLY, READ_SIZE);
    ASSERT_EQUAL(result, 0);

    // write one byte exceeding the allocated payload
    // REE side allocates for generic 'fpc_ta_bio_command_t'
    // TEE side is using explicit 'fpc_ta_byte_array_msg_t'
    // This means there are some extra bytes available on TEE side.
    const uint32_t ONE_BYTE_TO_MUCH =
            READ_SIZE + (sizeof(fpc_ta_bio_command_t) - sizeof(fpc_ta_byte_array_msg_t)) + 1;

    result = send_read_command(tee, READ_SIZE, ONE_BYTE_TO_MUCH);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Trigger uint32_t overflow (payload exclude message size)
    result = send_read_command(tee, READ_SIZE, 0xFFFFFFFF);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Close in order to progress testing
    result = send_close_command(tee);
    ASSERT_EQUAL(result, 0);

    return 0;
}

static int test_transfer_write(fpc_tee_t* tee)
{
    LOGD("%s", __func__);
    int result;

    const uint32_t WRITE_SIZE = 4096;

    // Open in read mode
    result = send_open_command(tee, FPC_TA_BIO_DB_RDONLY, WRITE_SIZE);
    ASSERT_EQUAL(result, 0);

    // Cannot write in read mode mode
    result = send_write_command(tee, WRITE_SIZE, WRITE_SIZE);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Close in order to progress testing
    result = send_close_command(tee);
    ASSERT_EQUAL(result, 0);

    // Open in write mode
    result = send_open_command(tee, FPC_TA_BIO_DB_WRONLY, WRITE_SIZE);
    ASSERT_EQUAL(result, 0);

    // write one byte exceeding the allocated payload
    // REE side allocates for generic 'fpc_ta_bio_command_t'
    // TEE side is using explicit 'fpc_ta_byte_array_msg_t'
    // This means there are some extra bytes available on TEE side.
    const uint32_t ONE_BYTE_TO_MUCH =
            WRITE_SIZE + (sizeof(fpc_ta_bio_command_t) - sizeof(fpc_ta_byte_array_msg_t)) + 1;

    result = send_write_command(tee, WRITE_SIZE, ONE_BYTE_TO_MUCH);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Trigger uint32_t overflow (payload exclude message size)
    result = send_write_command(tee, WRITE_SIZE, 0xFFFFFFFF);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    // Close in order to progress testing
    result = send_close_command(tee);
    ASSERT_EQUAL(result, 0);

    result = send_write_command(tee, 0, 0);
    ASSERT_EQUAL(FPC_ERROR_GET_EXTERNAL_ERROR(result), -FPC_ERROR_PARAMETER);

    return 0;
}

/*
 * db blob test entry
 */
int test_fpc_tee_db_blob(fpc_tee_t* tee)
{
    printf("Execute %s\n", __func__);

    if (!tee) {
        return -FPC_ERROR_PARAMETER;
    }

    // Ensure we are in a 'closed transfer state'
    send_close_command(tee);

    test_transfer_open(tee);

    test_transfer_read(tee);

    test_transfer_write(tee);

    printf("Executed %s. Assert PASS=%d, FAIL=%d\n", __func__, assert_passed, assert_failed);
    return 0;
}
