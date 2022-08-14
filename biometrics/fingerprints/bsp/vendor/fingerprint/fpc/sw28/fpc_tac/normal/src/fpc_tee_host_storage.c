/************************************************************************************
 ** File: - fpc\fpc_tac\normal\src\fpc_tee_host_storage.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE SECURE STORAGE FILE for FPC (SW23.2.2)
 **
 ** Version: 1.0
 ** Date created: 11:11:11,14/12/2017
 ** Author: Bin.Li@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 ** 	<author>	  <data>			<desc>
 **     Bin.Li         2017/12/14        create the file, add backup template
 **     Long.Liu       2019/02/14        add FPC_ERROR_CRYPTO status, load_fpc_empty_db
 ************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fpc_ta_interface.h"
#include "fpc_tee.h"
#include "fpc_tac.h"
#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_ta_targets.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"
#include "fpc_tee_hal.h"

#define MIN(A, B) ((A) < (B) ? (A) : (B))

//ziqing add for backup fingerprint template
static int FileCopy(const char* src, char* des) {
        FILE *input = fopen(src, "r");
        if (input == NULL) {
                LOGE("'%s' open failed!", src);
                return -1;
        }
        FILE *output = fopen(des, "wb+");
        if (output == NULL) {
                fclose(input);
                LOGE("'%s' open failed!", des);
                return -2;
        }

        size_t st = 50000;
        char *buffer = (char *)malloc(st*sizeof(char));
        for (;;) {
                size_t readlen = 0;
                readlen = fread(buffer, 1, sizeof(buffer), input);
                if (readlen == 0) {
                        break;
                }
                size_t writelen = 0;
                while (writelen < readlen) {
                        writelen += fwrite(buffer + writelen, 1, (readlen - writelen), output);
                }
        }
        fclose(input);
        fclose(output);
        free(buffer);
        return 0;
}

//ziqing add for backup fingerprint template
int fpc_create_backup_db(char* path, const char* suffix) {
        char temp_path_bak[PATH_MAX];
        struct stat stat_info;
        if (snprintf(temp_path_bak, PATH_MAX, "%s.%s", path, suffix) >= PATH_MAX) {
                LOGE("%s input:path too long", __func__);
                return -FPC_ERROR_IO;
        }
        if (stat(temp_path_bak, &stat_info) == 0) {
                if (FileCopy(temp_path_bak, path)) {
                        LOGE("%s stat failed with error %s", __func__, strerror(errno));
                        return -FPC_ERROR_IO;
                }
        }
        return 0;
}

static int get_file_size(const char* path, size_t* size) {
    struct stat file_info;
    memset(&file_info, 0, sizeof(file_info));
    *size = 0;

    if (stat(path, &file_info)) {
        switch(errno) {
        case ENOENT:
            return 0;
        default:
            LOGE("%s stat failed with error %s", __func__, strerror(errno));
            return -FPC_ERROR_IO;
        }
    }

    *size = file_info.st_size;

    return 0;
}

static int fpc_read_db_blob_from_file(void *buf, int size, const char* path)
{
    int c = 0;
    int fd;

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        LOGE("%s failed to open file %s, %s", __func__, path, strerror(errno));
        return -FPC_ERROR_IO;
    }

    c = read(fd, buf, size);
    close(fd);

    if (size != c) {
        LOGE("%s failed to read full size of file %s ", __func__, path);
        return -FPC_ERROR_IO;
    } else {
        LOGD("%s Successfully read template database %s", __func__, path);
        return 0;
    }
}

static int fpc_write_db_blob_to_file(void *buf, int c, const char* path)
{
    mode_t mode = S_IRUSR | S_IWUSR;
    int r = 0;
    int fd;

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        LOGE("%s failed to open file %s, %s", __func__, path, strerror(errno));
        return -FPC_ERROR_IO;
    }

    r = write(fd, buf, c);
    fsync(fd);
    close(fd);

    if (r == -1) {
        LOGE("%s failed to write file %s, %s", __func__, path, strerror(errno));
        return -FPC_ERROR_IO;
    } else if (r != c) {
        LOGE("%s failed to write full size of file %s", __func__, path);
        return -FPC_ERROR_IO;
    }

    LOGD("%s Successfully wrote template database %s", __func__, path);

    return 0;
}

static int get_db_blob_size(fpc_tee_t* tee, size_t *blob_size)
{
    int status = 0;

    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;
    command->header.target    = TARGET_FPC_TA_DB_BLOB;
    command->header.command   = FPC_TA_BIO_GET_DB_SIZE_CMD;
    command->get_db_size.size = 0;

    status = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    *blob_size = command->get_db_size.size;

    return status;
}

static int db_open(fpc_tee_t *tee, uint32_t mode, uint32_t size)
{
    int result;
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;

    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_OPEN_CMD;
    command->db_open.mode   = mode;
    command->db_open.size   = size;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (FAILED(result)) {
        LOGE("%s Failed to complete command", __func__);
    }
    return result;
}

static int db_close(fpc_tee_t *tee)
{
    int result;
    fpc_ta_bio_command_t* command = tee->shared_buffer->addr;

    command->header.target  = TARGET_FPC_TA_DB_BLOB;
    command->header.command = FPC_TA_BIO_DB_CLOSE_CMD;

    result = fpc_tac_transfer(tee->tac, tee->shared_buffer);

    if (FAILED(result)) {
        LOGE("%s Failed to complete command", __func__);
    }
    return result;
}

static int send_db_read_commands(fpc_tee_t* tee, uint8_t *blob, size_t blob_size)
{
    int result = 0;
    const uint32_t payload_size = MIN(blob_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_bio_command_t);
    fpc_tac_shared_mem_t* shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_READ_CMD;
    command->db_read.size         = payload_size;

    uint32_t blob_offset    = 0;
    uint32_t remaining_size = blob_size;

    // Send read commands until complete payload is transfered
    while (remaining_size) {
        const uint32_t remaining_payload_size = MIN(MAX_CHUNK, remaining_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (FAILED(result)) {
            goto free_shared;
        }

        remaining_size -= remaining_payload_size;

        // TEE side returns the remaining number of bytes
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d). Abort transfer ", __func__, remaining_size, result);
            result = -FPC_ERROR_STATE;
            goto free_shared;
        }

        memcpy(&blob[blob_offset], command->db_read.array, remaining_payload_size);
        blob_offset += remaining_payload_size;
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

static int send_db_write_commands(fpc_tee_t* tee, uint8_t *file_buffer, size_t file_buffer_size)
{
    int result = 0;
    fpc_tac_shared_mem_t* shared_buffer = NULL;

    // Setup write message
    const uint32_t payload_size = MIN(file_buffer_size, MAX_CHUNK);
    const uint32_t message_size = payload_size + sizeof(fpc_ta_bio_command_t);
    shared_buffer = fpc_tac_alloc_shared(tee->tac, message_size);

    if (!shared_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    fpc_ta_bio_command_t* command = shared_buffer->addr;
    command->header.target        = TARGET_FPC_TA_DB_BLOB;
    command->header.command       = FPC_TA_BIO_DB_WRITE_CMD;
    command->db_write.size        = payload_size;

    uint32_t remaining_size = file_buffer_size;
    uint32_t payload_offset = 0;

    // Send write commands until complete payload is transfered
    while (remaining_size) {
        uint32_t remaining_payload_size = MIN(MAX_CHUNK, remaining_size);
        memcpy(command->db_write.array, &file_buffer[payload_offset], remaining_payload_size);

        result = fpc_tac_transfer(tee->tac, shared_buffer);
        if (FAILED(result)) {
            LOGE("%s - Failed to write %u bytes of data. Result %d", __func__, remaining_payload_size, result);
            goto free_shared;
        }

        remaining_size -= remaining_payload_size;
        payload_offset += remaining_payload_size;

        // TEE side returns the remaining number of bytes
        if (remaining_size != (uint32_t)result) {
            LOGE("%s - REE and TEE out of sync (%u vs %d). Abort transfer ", __func__, remaining_size, result);
            result = -1;
            goto free_shared;
        }
    }

free_shared:
    fpc_tac_free_shared(shared_buffer);
    return result;
}

int fpc_tee_store_template_db(fpc_tee_bio_t* bio, finger_store_template_config_t* template_config)
{
    LOGD("%s", __func__);
    fpc_tee_t* tee = &bio->tee;
    char temp_path[PATH_MAX];
    int result;
    size_t blob_size = 0;

    if (!template_config) {
            LOGE("template config doesn't exist");
            return -FPC_ERROR_PARAMETER;
    }
    char temp_path_bak[PATH_MAX];

    if (strlen(template_config->path) >= PATH_MAX) {
        LOGE("%s input path too long", __func__);
        return -FPC_ERROR_PARAMETER;
    }

    strcpy(temp_path, template_config->path);
    char *dir = dirname(temp_path);

    result = mkdir(dir, 0700);
    if (result && (errno != EEXIST)) {
        LOGE("%s - mkdir(%s) failed with error %s", __func__, dir, strerror(errno));
        return -FPC_ERROR_IO;
    }

    if (snprintf(temp_path, PATH_MAX, "%s.bak", template_config->path) >= PATH_MAX) {
        LOGE("%s input:path too long", __func__);
        return -FPC_ERROR_PARAMETER;
    }

    //ziqing add, backup template only when we add or remove fp template, this backup template is named "user.db.bak1"
    if (template_config->template_mode != UPDATE_TEMPLATE_MODE) {
            struct stat stat_info;
            if (stat(template_config->path, &stat_info) == 0) {
                    if (snprintf(temp_path_bak, PATH_MAX, "%s.%s", template_config->path, FP_TEMPLATE_DB_BACKUP_1) >= PATH_MAX) {
                            LOGE("%s input temp_path:bak1 too long", __func__);
                            return -FPC_ERROR_PARAMETER;
                    }
                    if (FileCopy(template_config->path, temp_path_bak)) {
                            LOGE("%s create temp_path:bak1 failed with error %s", __func__, strerror(errno));
                            return -FPC_ERROR_IO;
                    }
            }
    }

    // 2 store user.db
    result = get_db_blob_size(tee, &blob_size);
    if (result < 0) {
        return result;
    } else if (blob_size == 0) {
        return -FPC_ERROR_IO;
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_RDONLY, blob_size);
    if (result < 0) {
        LOGE("%s - transfer_open failed with %d", __func__, result);
        goto close;
    }

    uint8_t *blob = malloc(blob_size);
    if (!blob) {
        result = -FPC_ERROR_MEMORY;
        goto close;
    }

    // Transfer encrypted content from TA (chunk if necessary)
    result = send_db_read_commands(tee, blob, blob_size);
    if (result < 0) {
        goto free;
    }

    result = fpc_write_db_blob_to_file(blob, blob_size, temp_path);
    if (result < 0) {
        LOGE("%s - Failed (%d) to write template db to file system. Keeping the old database.", __func__, result);
        goto free;
    }

    if (rename(temp_path, template_config->path) < 0) {
        LOGE("%s - rename failed with error %s", __func__, strerror(errno));
        (void)unlink(temp_path);
        result = -FPC_ERROR_IO;
        goto free;
    }

//ziqing add, backup template only when we add or remove fp template, this backup template is named "user.db.bak1"
if (template_config->template_mode != UPDATE_TEMPLATE_MODE) {
        struct stat stat_info;
        if (stat(template_config->path, &stat_info) == 0) {
                if (snprintf(temp_path_bak, PATH_MAX, "%s.%s", template_config->path, FP_TEMPLATE_DB_BACKUP_0) >= PATH_MAX) {
                        LOGE("%s input temp_path:bak0 too long", __func__);
                        result = -FPC_ERROR_PARAMETER;
                        goto free;
                }
                if (FileCopy(template_config->path, temp_path_bak)) {
                        LOGE("%s create temp_path:bak0 failed with error %s", __func__, strerror(errno));
                        result = -FPC_ERROR_IO;
                        goto free;
                }
        }
}


free:
    free(blob);
close:
    db_close(tee);
    return result;
}


int fpc_tee_load_template_db(fpc_tee_bio_t* bio, const char* path)
{
    fpc_tee_t* tee = &bio->tee;

    size_t file_size = 0;

    int result = get_file_size(path, &file_size);
    if (result < 0) {
        return result;
    }

    if (file_size == 0) {
        return fpc_tee_load_empty_db(bio);
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_WRONLY, file_size);
    if (result < 0) {
        LOGE("Failed to open transfer in write mode with %zu bytes of payload", file_size);
        goto close;
    }

    uint8_t *file_buffer = malloc(file_size);
    if (!file_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    result = fpc_read_db_blob_from_file(file_buffer, file_size, path);
    if (result < 0) {
        goto free;
    }

    // Transfer encrypted content to TA (chunk if necessary)
    result = send_db_write_commands(tee, file_buffer, file_size);
    if (result < 0) {
        goto free;
    }

free:
    free(file_buffer);
close:
    db_close(tee);
    return result;
}


int fpc_tee_load_backup_template_db(fpc_tee_bio_t* bio, const char* path, uint32_t* template_status)
{
    fpc_tee_t* tee = &bio->tee;

    size_t file_size = 0;

    int result = get_file_size(path, &file_size);
    if (result < 0) {
        return result;
    }

    if (file_size == 0) {
        *template_status = HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_SUCC;
        return fpc_tee_load_empty_db(bio);
    }

    // If open fails ensure that we always close. REE/TEE could get out of sync
    // if the fingerprint daemon crashes.
    result = db_open(tee, FPC_TA_BIO_DB_WRONLY, file_size);
    if (result < 0) {
        LOGE("Failed to open transfer in write mode with %zu bytes of payload", file_size);
        goto close;
    }

    uint8_t *file_buffer = malloc(file_size);
    if (!file_buffer) {
        return -FPC_ERROR_MEMORY;
    }

    result = fpc_read_db_blob_from_file(file_buffer, file_size, path);
    if (result < 0) {
        goto free;
    }

    // Transfer encrypted content to TA (chunk if necessary)
    result = send_db_write_commands(tee, file_buffer, file_size);
    if (result < 0) {
        goto free;
    }

free:
    if ((-FPC_ERROR_IO == result)) {
        *template_status = HAL_LOAD_TEMPLATE_RESTORED_FAIL_EMPTYDB_SUCC;
        result = fpc_tee_load_empty_db(bio);
    }
    free(file_buffer);
close:
    db_close(tee);
    return result;
}
