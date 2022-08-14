/************************************************************************************
 ** File: - fpc\fpc_tac\normal\src\fpc_tee.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      fpc tee interface (sw23.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,10/26/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 **
 ** --------------------------- Revision History: --------------------------------
 ** 	<author>	     <data>			<desc>
 **    Ziqing.guo      2017/10/26       create file, add for load respective TA according to fp vendor type
 **    Ran.Chen        2017/12/20       add fpc1023 for sdm17081
 ************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <limits.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "fpc_types.h"
#include "fpc_tac.h"
#include "fpc_ta_targets.h"
#include "fpc_tee.h"
#include "fpc_tee_internal.h"
#include "fpc_ta_common_interface.h"
#include "fpc_log.h"
#include "fingerprint_type.h"

typedef struct fpc_tee fpc_tee_t;

fpc_tee_t* fpc_tee_init(void)
{
    fpc_tee_t* tee = calloc(1, sizeof(fpc_tee_t));

    if (!tee) {
        return NULL;
    }

    strcpy(tee->manifest_tag, "unknown");

    tee->tac = fpc_tac_open();
    if (!tee->tac) {
        goto err;
    }

    tee->shared_buffer = fpc_tac_alloc_shared(
        tee->tac,
        MAX_COMMAND_SIZE);

    if (!tee->shared_buffer) {
        goto err;
    }

    return (fpc_tee_t*) tee;

err:
    fpc_tee_release((fpc_tee_t*) tee);
    return NULL;
}

void fpc_tee_release(fpc_tee_t* tee)
{
    if (!tee) {
        return;
    }

    if (tee->shared_buffer) {
        fpc_tac_free_shared(tee->shared_buffer);
    }
    if (tee->tac) {
        fpc_tac_release(tee->tac);
    }

    free(tee);
}

#define FPC_CONFIG_LOG_TO_FILE_LOCATION "/data/fpc_tpl/"

void fpc_tee_write_ta_log(const char *scenario, fpc_tee_t *tee)
{
    int status = 0;
    char filename[128];
    FILE* fd = NULL;
    uint32_t log_buffer_size = tee->log_to_file_buffer_size;

    fpc_tac_shared_mem_t *shared_ipc_buffer =
        fpc_tac_alloc_shared(tee->tac, log_buffer_size + sizeof(fpc_ta_byte_array_msg_t));

    if(!shared_ipc_buffer) {
        status = -FPC_ERROR_MEMORY;
        LOGE("%s: failed to allocate error buffer ret: %d", __func__, status);
        goto out;
    }

    memset(shared_ipc_buffer->addr, 0, log_buffer_size + sizeof(fpc_ta_byte_array_msg_t));

    fpc_ta_common_command_t *command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_COMMON;
    command->header.command = FPC_TA_COMMON_GET_LOG_CMD;
    command->error_msg.size = log_buffer_size;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        LOGE("%s, fpc_tac_transfer failed %d.", __func__, status);
        goto out;
    }

    if (snprintf(filename, sizeof(filename), "%s%s_%ld", FPC_CONFIG_LOG_TO_FILE_LOCATION, scenario,
                                                         time(NULL)) < (int)sizeof(filename)) {
        fd = fopen(filename, "wb");
    }

    if (!fd) {
        status = -errno;
        LOGE("%s, failed to open log file at %s returns error %i",
                __func__,
                FPC_CONFIG_LOG_TO_FILE_LOCATION,
                status);

        goto out;
    }

    uint32_t written = 0;

    do {
        status = fwrite(command->error_msg.array + written, 1,
                        command->error_msg.size - written, fd);
        if (status < 0) {
            LOGE("%s, write to %s returns error %i, errno %d",
                    __func__,
                    filename,
                    status,
                    errno);

            goto out;
        }
        written += status;
    } while (written < command->error_msg.size);

    LOGI("TA log written to %s", filename);

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }

    if (fd) {
        fclose(fd);
    }
}

static int get_manifest_tag(const char *build_info, const char **manifest_tag,
                            size_t *manifest_tag_len)
{
    int result = FPC_ERROR_NONE;

    const char *prefix    = "\"manifest_tag\":\"";
    const char *tag_start = NULL;
    const char *tag_end   = NULL;

    if (build_info == NULL || manifest_tag == NULL || manifest_tag_len == NULL) {
        result = -FPC_ERROR_PARAMETER;
        goto out;
    }

    *manifest_tag     = NULL;
    *manifest_tag_len = 0;

    tag_start = strstr(build_info, prefix);
    if (tag_start == NULL) {
        result = -FPC_ERROR_PARAMETER;
        goto out;
    }

    tag_start += strlen(prefix);
    tag_end = strchr(tag_start, '"');
    if (tag_end == NULL) {
        result = -FPC_ERROR_PARAMETER;
        goto out;
    }

    *manifest_tag     = tag_start;
    *manifest_tag_len = tag_end - tag_start;

out:
    return result;
}

static int set_manifest_tag(fpc_tee_t *tee, const char *build_info)
{
    int result = FPC_ERROR_NONE;

    const char *manifest_tag = NULL;
    size_t manifest_tag_len = 0;
    const size_t max_size = sizeof(tee->manifest_tag);

    result = get_manifest_tag(build_info, &manifest_tag, &manifest_tag_len);
    if (result != FPC_ERROR_NONE) {
        goto out;
    }

    if (manifest_tag_len + 1 > max_size) {
        result = -FPC_ERROR_MEMORY;
        goto out;
    }

    memcpy(tee->manifest_tag, manifest_tag, manifest_tag_len);
    tee->manifest_tag[manifest_tag_len] = '\0';

out:
    LOGD("%s: %s", __func__, tee->manifest_tag);

    return result;
}

int fpc_tee_get_build_info(fpc_tee_t *tee,
                           void (*buildinfo_callback)(const fpc_ta_common_build_info_msg_t *build_info, void *usr), void *usr)
{
    LOGD("%s", __func__);
    int status = -1;

    uint32_t size = sizeof(fpc_ta_common_build_info_msg_t);
    fpc_tac_shared_mem_t* shared_ipc_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_ipc_buffer) {
        goto out;
    }

    fpc_ta_common_command_t* command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_COMMON;
    command->header.command = FPC_TA_COMMON_GET_BUILD_INFO_CMD;
    command->build_info.size = MAX_BUILDINFO_SIZE;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        LOGE("%s, fpc_tac_transfer failed %d.", __func__, status);
        goto out;
    }

    if (buildinfo_callback != NULL) {
        buildinfo_callback(&command->build_info, usr);
    }

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    return status;
}

int fpc_tee_get_lib_version(fpc_tee_t *tee, char *lib_version)
{
    LOGD("%s", __func__);
    int status = -1;

    uint32_t size = sizeof(fpc_ta_common_command_t);
    fpc_tac_shared_mem_t* shared_ipc_buffer = fpc_tac_alloc_shared(tee->tac, size);
    if (!shared_ipc_buffer) {
        goto out;
    }

    fpc_ta_common_command_t* command = shared_ipc_buffer->addr;
    command->header.target = TARGET_FPC_TA_COMMON;
    command->header.command = FPC_TA_COMMON_GET_LIB_VERSION_CMD;
    command->lib_version.size = 0;

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);
    if (status) {
        LOGE("%s, fpc_tac_transfer failed %d.", __func__, status);
        goto out;
    }
    memcpy(lib_version, command->lib_version.array, command->lib_version.size);

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    return status;
}

static void log_build_info_callback(const fpc_ta_common_build_info_msg_t *build_info, void *usr)
{
    fpc_tee_t *tee = usr;

    const char *p   = build_info->array;
    const char *end = p + strlen(p);

    // The buildinfo string can be somewhat long, so add a newline after each
    // comma when printing it
    while (p < end) {
        while (*p == ' ') {
            p++;
        }
        const char *q = strchr(p, ',');
        if (q == NULL) {
            q = end;
        } else {
            q = q + 1;
        }
        LOGD("%s : %.*s", __func__, (int)(q - p), p);
        p = q;
    }

    set_manifest_tag(tee, build_info->array);

    LOGD("%s supported_features %08x", __func__, build_info->enabled_features);
    tee->supported_features = build_info->enabled_features;
    tee->log_to_file_buffer_size = build_info->log_to_file_buffer_size;
    LOGD("log_to_file_buf_sz=%u", tee->log_to_file_buffer_size);
    tee->max_number_of_templates = build_info->max_number_of_templates;
    LOGD("max_number_of_templates=%u", tee->max_number_of_templates);
}

int fpc_tee_log_build_info(fpc_tee_t *tee)
{
    return fpc_tee_get_build_info(tee, log_build_info_callback, tee);
}

bool fpc_tee_engineering_enabled(const fpc_tee_t* tee)
{
    return ((tee->supported_features >> FPC_TA_COMMON_ENGINEERING_ENABLED) & 1) == 1;
}

bool fpc_tee_sensortest_enabled(const fpc_tee_t* tee)
{
    return ((tee->supported_features >> FPC_TA_COMMON_SENSORTEST_ENABLED) & 1) == 1;
}

bool fpc_tee_navigation_enabled(const fpc_tee_t* tee)
{
    return ((tee->supported_features >> FPC_TA_COMMON_NAVIGATION_ENABLED) & 1) == 1;
}

bool fpc_tee_navigation_debug_enabled(const fpc_tee_t *tee)
{
    return fpc_tee_navigation_enabled(tee)
           && fpc_tee_engineering_enabled(tee);
}

bool fpc_tee_log_to_file_enabled(const fpc_tee_t* tee)
{
    return ((tee->supported_features >> FPC_TA_COMMON_LOG_TO_FILE_ENABLED) & 1) == 1;
}

bool fpc_tee_force_sensor_enabled(const fpc_tee_t *tee)
{
    return ((tee->supported_features >> FPC_TA_COMMON_FORCE_SENSOR_ENABLED) & 1) == 1;
}

bool fpc_tee_navigation_force_sw_enabled(const fpc_tee_t *tee)
{
    return ((tee->supported_features >> FPC_TA_COMMON_NAVIGATION_FORCE_SW_ENABLED) & 1) == 1;
}

bool fpc_tee_sensetouch_enabled(const fpc_tee_t *tee)
{
    return fpc_tee_force_sensor_enabled(tee) ||
           fpc_tee_navigation_force_sw_enabled(tee);
}

uint32_t fpc_tee_get_max_number_of_templates(const fpc_tee_t *tee)
{
    return tee->max_number_of_templates;
}
