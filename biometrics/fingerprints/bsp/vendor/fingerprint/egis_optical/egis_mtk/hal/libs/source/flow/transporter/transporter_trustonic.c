#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MobiCoreDriverApi.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "response_def.h"
#include "transporter.h"

#define TA_PATH "/odm/vendor/app/mcRegistry/08080000000000000000000000000000.tlbin"

typedef struct {
    unsigned char* in_data;
    unsigned int in_data_len;
    unsigned char* out_data;
    unsigned int out_data_len;
} EGIS_DATA_T;

typedef struct {
    unsigned int in_msg;
    unsigned int in_msg_len;
    unsigned int out_msg;
    unsigned int out_msg_len;
} EGIS_MESSAGE_T;

static const unsigned int DEVICE_ID = MC_DEVICE_ID_DEFAULT;
mcSessionHandle_t g_sessionHandle;
static void* g_buffer_data = NULL;
unsigned int g_is_connection_open = 0;

static int check_connection_open() {
    FILE* pStream;
    long filesize;
    unsigned char* content = NULL;
    int retval = FINGERPRINT_RES_SUCCESS;
    mcResult_t result;
    mcVersionInfo_t versionInfo;
    int buffer_size = sizeof(EGIS_MESSAGE_T);

    pStream = fopen(TA_PATH, "rb");
    if (NULL == pStream) {
        ex_log(LOG_ERROR, "Fail to open %s", TA_PATH);
        return FINGERPRINT_RES_FAILED;
    }

    if (fseek(pStream, 0L, SEEK_END) != 0) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    filesize = ftell(pStream);
    if (filesize <= 0) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    if (fseek(pStream, 0L, SEEK_SET) != 0) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    content = (unsigned char*)malloc(filesize);
    if (NULL == content) {
        retval = FINGERPRINT_RES_ALLOC_FAILED;
        goto EXIT;
    }

    if (fread(content, (unsigned int)filesize, 1, pStream) != 1) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    result = mcOpenDevice(DEVICE_ID);
    if (MC_DRV_OK != result) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    result = mcGetMobiCoreVersion(MC_DEVICE_ID_DEFAULT, &versionInfo);
    if (MC_DRV_OK != result) {
        mcCloseDevice(DEVICE_ID);
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    g_buffer_data = (void*)malloc(buffer_size);
    if (g_buffer_data == NULL) {
        mcCloseDevice(DEVICE_ID);
        retval = MC_DRV_ERR_NO_FREE_MEMORY;
        goto EXIT;
    }

    mem_set(g_buffer_data, 0x00, buffer_size);
    mem_set(&g_sessionHandle, 0, sizeof(mcSessionHandle_t));
    g_sessionHandle.deviceId = DEVICE_ID;

    result = mcOpenTrustlet(&g_sessionHandle, MC_SPID_TRUSTONIC_OTA, content, filesize,
                            (unsigned char*)g_buffer_data, buffer_size);

    if (MC_DRV_OK != result) {
        if (NULL != g_buffer_data) {
            free(g_buffer_data);
            g_buffer_data = NULL;
        }
        mcCloseDevice(DEVICE_ID);

        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    g_is_connection_open = 1;
    ex_log(LOG_INFO, "Open TA successfully %s", TA_PATH);

EXIT:
    fclose(pStream);

    if (NULL != content) {
        free(content);
        content = NULL;
    }

    return retval;
}

static int reset_connection() {
    mcResult_t result;
    int retval = FINGERPRINT_RES_SUCCESS;

    result = mcCloseSession(&g_sessionHandle);
    if (MC_DRV_OK != result) {
        ex_log(LOG_ERROR, "reset_connection mcCloseSession fail");
        retval = FINGERPRINT_RES_FAILED;
    }

    result = mcCloseDevice(DEVICE_ID);
    if (MC_DRV_OK != result) {
        ex_log(LOG_ERROR, "reset_connection mcCloseDevice fail");
        retval = FINGERPRINT_RES_FAILED;
    }

    if (NULL != g_buffer_data) {
        free(g_buffer_data);
        g_buffer_data = NULL;
    }

    return retval;
}

static int keep_connection_open() {
    if (g_is_connection_open > 0) {
        return FINGERPRINT_RES_SUCCESS;
    }

    // reset_connection();

    return check_connection_open();
}

int transporter(unsigned char* msg_data, unsigned int msg_data_len, unsigned char* out_data,
                unsigned int* out_data_len) {
    int retval = FINGERPRINT_RES_SUCCESS;
    mcResult_t result;
    unsigned int buffer_len = 0;
    unsigned char* buffer = NULL;
    unsigned char* tmp_data = NULL;
    EGIS_DATA_T egis_data;
    int egis_data_len;
    EGIS_MESSAGE_T egis_msg;
    int egis_msg_len = sizeof(egis_msg);
    void* map_info = NULL;

    if (NULL == msg_data || msg_data_len == 0) {
        ex_log(LOG_ERROR, "invalid input params\n");
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    retval = keep_connection_open();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "keep_connection_open failed\n");
        return FINGERPRINT_RES_FAILED;
    }

    mem_set(&egis_data, 0x00, sizeof(egis_data));
    if (NULL != out_data_len) {
        egis_data_len = *out_data_len + sizeof(unsigned int);
    } else {
        egis_data_len = sizeof(unsigned int);
    }

    tmp_data = (unsigned char*)malloc(egis_data_len);
    if (NULL == tmp_data) {
        ex_log(LOG_ERROR, "tmp_data malloc failed\n");
        return FINGERPRINT_RES_ALLOC_FAILED;
    }

    mem_set(tmp_data, 0x00, egis_data_len);

    egis_data.in_data = msg_data;
    egis_data.in_data_len = msg_data_len;
    egis_data.out_data = tmp_data;
    egis_data.out_data_len = egis_data_len;

    buffer_len = egis_data.in_data_len;
    egis_msg.out_msg_len = egis_data.out_data_len;
    if (egis_msg.out_msg_len > buffer_len) {
        buffer_len = egis_msg.out_msg_len;
    }

    buffer = (unsigned char*)malloc(buffer_len);
    if (NULL == buffer) {
        retval = FINGERPRINT_RES_ALLOC_FAILED;
        goto EXIT;
    }

    mem_move(buffer, egis_data.in_data, egis_data.in_data_len);

    map_info = (void*)malloc(sizeof(mcBulkMap_t));
    if (NULL == map_info) {
        retval = FINGERPRINT_RES_ALLOC_FAILED;
        ex_log(LOG_ERROR, "map_info malloc failed\n");
        goto EXIT;
    }

    result = mcMap(&g_sessionHandle, buffer, buffer_len, map_info);
    if (MC_DRV_OK != result) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    mem_move(&egis_msg.in_msg, map_info, sizeof(unsigned int));
    egis_msg.in_msg_len = buffer_len;
    egis_msg.out_msg = 0;

    mem_move(g_buffer_data, &egis_msg, egis_msg_len);
    result = mcNotify(&g_sessionHandle);
    if (MC_DRV_OK != result) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    result = mcWaitNotification(&g_sessionHandle, -1);
    if (MC_DRV_OK != result) {
        retval = FINGERPRINT_RES_FAILED;
        goto EXIT;
    }

    mem_move(&egis_msg, g_buffer_data, egis_msg_len);
    if (egis_msg.out_msg_len > 0) {
        mem_move(&retval, buffer, sizeof(unsigned int));
        if (out_data != NULL) {
            mem_move(out_data, buffer + sizeof(unsigned int),
                     egis_msg.out_msg_len - sizeof(unsigned int));
            *out_data_len = egis_msg.out_msg_len - sizeof(unsigned int);
        }
    }

EXIT:

    if (NULL != tmp_data) {
        free(tmp_data);
        tmp_data = NULL;
    }

    if (NULL != map_info) {
        (void)mcUnmap(&g_sessionHandle, buffer, map_info);
        free(map_info);
        map_info = NULL;
    }

    if (NULL != buffer) {
        free(buffer);
        buffer = NULL;
    }

    return retval;
}