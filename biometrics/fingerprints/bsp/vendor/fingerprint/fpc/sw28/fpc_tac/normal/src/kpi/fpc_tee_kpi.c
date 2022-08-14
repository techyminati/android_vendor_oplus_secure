/************************************************************************************
 ** File: - fpc/fpc_tac/normal/src/kpi/fpc_tee_kpi.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2018, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint SENSORTEST FOR FPC (SW28)
 **
 ** Version: 1.0
 ** Date created: 11:11:11,05/01/2019
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **     <author>        <data>                <desc>
 **    Yang.tan       2019/01/05        modify coverity error for sw28
 ************************************************************************************/
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <cutils/list.h>
#include <time.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_kpi_interface.h"
#include "fpc_tee_kpi.h"
#include "fpc_tee_internal.h"
#include "fpc_tee.h"
#include "fpc_log.h"

#define MAX_SUB_CAPTURES 10
#define MAX_FASTTAP_CAPTURE 3
#define KPI_RESULTS_SIZE 50

#ifdef FPC_STORE_IN_DATA_DIR
#define KPI_PATH "/data/diag_logs"
#else
#define KPI_PATH "/data/fpc/diag_logs"
#endif

#define MANIFEST_TAG_MAX 64

#ifndef FPC_CONFIG_RETRY_MATCH_TIMEOUT
#define FPC_CONFIG_RETRY_MATCH_TIMEOUT 0
#endif

/* This struct must be kept in sync with fpc_lib/src/fpc_kpi.c */
typedef struct {
    /* Duration of finger qualification in ms */
    int16_t fq;
    /* Duration of fallback image in ms */
    int16_t fallback;
    /* Duration of FSD in ms */
    int16_t fsd[MAX_SUB_CAPTURES];
    /* Duration of CAC in ms */
    int16_t cac[MAX_SUB_CAPTURES];
    /* Duration of capture in ms */
    int16_t capture;
    /* Duration of enroll/identify in ms */
    int16_t enroll_identify;
    /* Duration of identify @ enroll in ms */
    int16_t identify_at_enroll;
    /* Duration since finger detected in ms */
    int16_t total;

    char usecase;
    char selected_image; /* '?' - no clue, 'C' - CAC search image, 'F' - fallback image */
    int32_t progress;
    int32_t decision;
    int32_t coverage;
    int32_t quality;
    int32_t nbr_of_templates;
    int32_t template_id;
    int32_t fasttap_searches;
    int32_t fasttap_captures;
    uint32_t fasttap_start;
    uint32_t fasttap_search[MAX_FASTTAP_CAPTURE];
    uint32_t fasttap_capture[MAX_FASTTAP_CAPTURE];
} kpi_results_t;

typedef struct {
    uint16_t hwid;
} kpi_metadata_t;

/**
 * @brief List item for holding the KPI results
 *
 */
typedef struct {
    struct listnode list;    /**< Needed to be able to use list.h lib */
    uint16_t size;           /**< Number of results packages containt in results */
    kpi_results_t *results;  /**< The kpi results */
} kpi_node_t;

static int kpi_active = 0;
static char csv_file[PATH_MAX] = {0};
static kpi_metadata_t kpi_metadata;
static list_declare(kpi_data);

static int print_csv_metadata(FILE *csv)
{
    int len;
    int ret = 0;
    char value[PROP_VALUE_MAX];

    len = fprintf(csv, "0x%04x\t", kpi_metadata.hwid);
    if (len < 0) return 0;
    ret += len;

    property_get("ro.board.platform", value, "n/a");
    len = fprintf(csv, "%s\t", value);
    if (len < 0) return 0;
    ret += len;

    property_get("ro.boot.hardware", value, "n/a");
    len = fprintf(csv, "%s\t", value);
    if (len < 0) return 0;
    ret += len;

    property_get("ro.build.type", value, "n/a");
    len = fprintf(csv, "%s\t", value);
    if (len < 0) return 0;
    ret += len;


    return ret;
}

static int kpi_print_results(kpi_results_t *results, FILE *csv, int seq_num)
{
    int ret;
    int32_t fsd_total = 0;
    int32_t cac_total = 0;

    print_csv_metadata(csv);

    ret = fprintf(csv, "%c\t%c\t%d\t%d\t%d\t%d\t%d\t%s\t",
            results->usecase,
            results->selected_image,
            results->coverage,
            results->quality,
            results->decision,
            results->nbr_of_templates,
            results->template_id,
            "img.id");

    uint32_t i = 0;
    while (i < MAX_SUB_CAPTURES && results->fsd[i] != -1)
    {
        fsd_total += results->fsd[i];
        fprintf(csv, "%s%d", (i) ? ";" : "", results->fsd[i]);
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-" );

    i = 0;
    while (i < MAX_SUB_CAPTURES && results->cac[i] != -1)
    {
        cac_total += results->cac[i];
        fprintf(csv, "%s%d", (i) ? ";" : "", results->cac[i]);
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-" );
    ret = fprintf(csv, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t",
            results->fallback,
            fsd_total,
            cac_total,
            results->capture,
            results->enroll_identify,
            results->identify_at_enroll,
            results->total,
            results->fq);

    i = 0;
    while (i < MAX_FASTTAP_CAPTURE && results->fasttap_search[i])
    {
        fprintf(csv, "%s%u", (i) ? ";" : "",
                (uint32_t)(results->fasttap_search[i] - (i == 0 ? results->fasttap_start :
                                                         results->fasttap_capture[i - 1])));
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-");

    i = 0;
    while (i < MAX_FASTTAP_CAPTURE && results->fasttap_capture[i])
    {
        fprintf(csv, "%s%u", (i) ? ";" : "",
                (uint32_t)(results->fasttap_capture[i] - results->fasttap_search[i]));
        i++;
    }
    fprintf(csv, "%s\t", (i) ? "" : "-");
    fprintf(csv, "%d\t", results->progress);
    fprintf(csv, "%d\n", seq_num);

    return ret;
}

static void kpi_print_all_results(kpi_results_t *results, int count, int seq_num)
{
    int need_header = 1;
    int ret = 0;
    int i;
    struct stat st;
    FILE *fp_csv = NULL;

    if (!count) {
        return;
    }
    if (count < 0) {
        LOGE("%s: count negative", __func__);
        return;
    }
    if (count > KPI_RESULTS_SIZE) {
        LOGE("%s: count too high ", __func__);
        return;
    }

    fp_csv = fopen(csv_file, "a");
    if (!fp_csv) {
        LOGE("%s: open error '%s'", __func__, csv_file);
        return;
    }

    if (!fstat(fileno(fp_csv), &st)) {
        if (st.st_size > 0) {
            need_header = 0;
        }
    }

    if (need_header) {
        fprintf(fp_csv,
                "hwid\t"
                "platform\t"
                "hardware\t"
                "buildtype\t"
                "usecase\t"
                "selected\t"
                "coverage\t"
                "quality\t"
                "decision\t"
                "templates\t"
                "template\t"
                "imgid\t"
                "cac-fsd\t"
                "cac-sub\t"
                "fallback\t"
                "fsd_total\t"
                "cac_total\t"
                "capture\t"
                "enrol_identify\t"
                "identify_at_enrol\t"
                "total\t"
                "fq\t"
                "fasttap_search\t"
                "fasttap_capture\t"
                "progress\t"
                "seq_num\n");
    }

    for (i = 0; i < count; i++) {
        ret = kpi_print_results(results + i, fp_csv, seq_num);
        if (ret < 0) {
            break;
        }
    }
    if (ret < 0) {
        LOGE("%s: write error", __func__);
    }
    (void)fclose(fp_csv);
    (void)chmod(csv_file, 0777);
}

static int fpc_tee_kpi_ctrl(
    fpc_tee_t* tee,
    fpc_kpi_cmd_t cmd,
    uint32_t *size,
    uint8_t *data)
{
    int status = -1;
    uint32_t ipc_size = size ? sizeof(fpc_ta_kpi_command_t) + *size : 0;

    if (ipc_size < sizeof(fpc_ta_kpi_command_t)) {
        ipc_size = sizeof(fpc_ta_kpi_command_t);
    }

    fpc_tac_shared_mem_t *shared_ipc_buffer =
        fpc_tac_alloc_shared(tee->tac, ipc_size);
    if (!shared_ipc_buffer) {
        LOGE("%s: could not allocate shared buffer", __func__);
        goto out;
    }

    fpc_ta_kpi_command_t *command = shared_ipc_buffer->addr;
    memset(command, 0, sizeof(*command));
    command->header.target = TARGET_FPC_TA_KPI;
    command->header.command = cmd;
    command->kpi_ctrl.size = size ? *size : 0;
    if (command->kpi_ctrl.size && data) {
        memcpy(command->kpi_ctrl.array, data, command->kpi_ctrl.size);
    }

    status = fpc_tac_transfer(tee->tac, shared_ipc_buffer);

    if (size && data) {
        *size = *size < command->kpi_ctrl.size ?
                *size : command->kpi_ctrl.size;

        memcpy(data, command->kpi_ctrl.array, *size);
    }

out:
    if (shared_ipc_buffer) {
        fpc_tac_free_shared(shared_ipc_buffer);
    }
    LOGD("%s ret=%d", __func__, status);
    return status;
}

static void fpc_tee_kpi_save_buffered_sequence(void)
{
    struct listnode *node;
    kpi_node_t *result_node;

    int seq_num = 0;
    list_for_each(node, &kpi_data) {
        result_node = node_to_item(node, kpi_node_t, list);

        kpi_print_all_results(result_node->results, result_node->size, seq_num);
        free(result_node->results);
        list_remove(node);
        seq_num++;
    }
}

static void fpc_tee_kpi_next_sequence(fpc_tee_t *tee)
{
    uint32_t size = 0;
    uint8_t *io_buffer;
    kpi_results_t *results = NULL;
    int status = 0;
    kpi_node_t *kpi_node = NULL;

    if (!kpi_active) {
        return;
    }

    /* alloc memory for both kpi results and kpi metadata */
    size = KPI_RESULTS_SIZE * sizeof(kpi_results_t) + sizeof(kpi_metadata);
    io_buffer = malloc(size);

    if (!io_buffer) {
        LOGE("%s: failed to allocate io_buffer", __func__);
        return;
    }

    /* Fetch the kpi data */
    status = fpc_tee_kpi_ctrl(tee, FPC_TA_KPI_STOP_CMD, &size, io_buffer);

    if (status) {
        LOGE("%s: fpc_tee_kpi_ctrl failed with error: %d", __func__, status);
        goto out;
    }

    /* Copy the metadata*/
    memcpy(&kpi_metadata, io_buffer, sizeof(kpi_metadata));
    size -= sizeof(kpi_metadata);

    if (size % sizeof(kpi_results_t)) {
        LOGE("%s: size mismatch, expected multiple of %zu, got %" PRIu32,
             __func__,
             sizeof(kpi_results_t),
             size);
        goto out;
    }

    /* alloc memory to store just the kpi results */
    results = malloc(size);
    if (!results) {
        LOGE("%s: failed to allocate kpi_results", __func__);
        goto out;
    }
    /* Copy the kpi results, excluding the kpi metadata */
    memcpy(results, (io_buffer + sizeof(kpi_metadata)), size);

    /* Allocate the node to save in the list */
    kpi_node = (kpi_node_t *)malloc(sizeof(kpi_node_t));
    if (!kpi_node) {
        LOGE("%s: failed to allocate kpi_node", __func__);
        free(results);
        goto out;
    }
    kpi_node->size = size / sizeof(kpi_results_t);
    kpi_node->results = results;
    list_add_tail(&kpi_data, &kpi_node->list);
out:
    free(io_buffer);
}
static int get_kpi_status(void)
{
    char value[PROP_VALUE_MAX];
    int ret = 0;

    property_get("vendor.fpc.kpi.enable", value, "0");
    ret = atoi(value);
    LOGD("%s: property value='%s' kpi_active=%d", __func__, value, ret);

    return ret;
}

static void _fpc_tee_kpi_start(fpc_tee_t *tee)
{
    const char *manifest_tag = tee->manifest_tag;
    char value[PROP_VALUE_MAX];

    time_t rawtime;
    time(&rawtime);
    struct tm *t = gmtime(&rawtime);

    if (!csv_file[0]) {
        char platform[PROP_VALUE_MAX];

        property_get("ro.boot.serialno", value, "NA");

        property_get("ro.board.platform", platform, "na");

        snprintf(csv_file,
                 PATH_MAX,
                 "%s/oddc_%s%s_%04d%02d%02d%02d%02d%02d_%s_#%s#.csv",
                 KPI_PATH,
#ifdef FPC_CONFIG_DEBUG
                 "DEBUG_",
#else
                 "",
#endif
                 value,              // serial
                 t->tm_year + 1900,
                 t->tm_mon + 1,
                 t->tm_mday,
                 t->tm_hour,
                 t->tm_min,
                 t->tm_sec,
                 platform,
                 manifest_tag);
        LOGD("%s csv_file='%s'", __func__, csv_file);
    }

    int status = fpc_tee_kpi_ctrl(tee, FPC_TA_KPI_START_CMD, NULL, NULL);
    if (status) {
        LOGE("%s: fpc_tee_kpi_ctrl failed with error: %d", __func__, status);
    }
}

void fpc_tee_kpi_start_sequence(fpc_tee_t *tee, bool new_sequence)
{

    kpi_active = get_kpi_status();
    if (!kpi_active) {
        // vendor.fpc.kpi.enable property was unset, invalidate file name
        csv_file[0] = 0;
        return;
    }

    if (new_sequence && !list_empty(&kpi_data)) {
        /* Save the content of the list before starting new sequence */
        fpc_tee_kpi_save_buffered_sequence();
    }

    if (!new_sequence || (FPC_CONFIG_RETRY_MATCH_TIMEOUT == 0)) {
        /* fpc_tee_kpi_next_sequence will do FPC_TA_KPI_STOP_CMD and when retry is not used */
        /* this will fail for the first loop since there is no data, but it is handled      */
        /* internally and no data will be added to the list                                 */
        fpc_tee_kpi_next_sequence(tee);
    }
    if (FPC_CONFIG_RETRY_MATCH_TIMEOUT == 0) {
        /* Only save if retry is not enabled */
        fpc_tee_kpi_save_buffered_sequence();
    }
    _fpc_tee_kpi_start(tee);
}

void fpc_tee_kpi_start(fpc_tee_t *tee)
{
    kpi_active = get_kpi_status();
    if (!kpi_active) {
        // vendor.fpc.kpi.enable property was unset, invalidate file name
        csv_file[0] = 0;
        return;
    }
    _fpc_tee_kpi_start(tee);
}

void fpc_tee_kpi_stop_sequence(fpc_tee_t *tee)
{
    if (!kpi_active) {
        return;
    }

    fpc_tee_kpi_next_sequence(tee);
    fpc_tee_kpi_save_buffered_sequence();
    kpi_active = 0;
}

void fpc_tee_kpi_stop(fpc_tee_t *tee)
{
    if (!kpi_active) {
        return;
    }

    uint32_t size = KPI_RESULTS_SIZE * sizeof(kpi_results_t) + sizeof(kpi_metadata);
    uint8_t *io_buffer = malloc(size);
    if (!io_buffer) {
        LOGE("%s: failed to allocate kpi_results", __func__);
        return;
    }

    int status = fpc_tee_kpi_ctrl(
                     tee,
                     FPC_TA_KPI_STOP_CMD,
                     &size,
                     io_buffer);

    if (status) {
        LOGE("%s: fpc_tee_kpi_ctrl failed with error: %d", __func__, status);
        goto out;
    }

    memcpy(&kpi_metadata, io_buffer, sizeof(kpi_metadata));
    size -= sizeof(kpi_metadata);
    kpi_results_t *results = (kpi_results_t *)(io_buffer + sizeof(kpi_metadata));

    if (size % sizeof(kpi_results_t))
    {
        LOGE("%s: size mismatch, expected multiple of %zu, got %" PRIu32,
                __func__,
                sizeof(kpi_results_t),
                size);
        goto out;
    }

    kpi_print_all_results(results, size / sizeof(kpi_results_t), 0);
    kpi_active = 0;

out:
    free(io_buffer);
}
