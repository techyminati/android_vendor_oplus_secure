#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include "transporter.h"
#include "response_def.h"
#include "plat_log.h"

const char *appname = "egista";

typedef struct _rbs_tlc_handle_t {
    void *handle;
    unsigned int handle_size;
} RBS_TLC_HANDLE_T;

RBS_TLC_HANDLE_T *g_tlcComHandle = NULL;
typedef struct transfer_package {
    unsigned int process;
    unsigned int command;
    unsigned int uid;
    unsigned int fid;
    unsigned int in_data_len;
    unsigned char in_data[0];
} transfer_package_t;
#define CLIENT_CMD_ETS_MESSAGE                0xe0

void *g_filehandle = NULL;
static int (*g_start_app)(RBS_TLC_HANDLE_T **p_tlcComHandle,
              const char *pStrAppname, int nBufSize) = NULL;
static int (*g_shut_down_app)(RBS_TLC_HANDLE_T **p_tlcComHandle) = NULL;
static int (*g_rbs_send_cmd)(RBS_TLC_HANDLE_T *l_tlcComHandle,
                 unsigned char *req_tci_data, int req_tci_size,
                 unsigned char *rsp_tci_data,
                 int *rsp_tci_size) = NULL;
static int (*g_rbs_send_cmd_and_in_data)(
    RBS_TLC_HANDLE_T *l_tlcComHandle, unsigned char *req_tci_data,
    int req_tci_size, unsigned char *rsp_tci_data, int *rsp_tci_size,
    unsigned char *in_data, unsigned int in_data_len) = NULL;

static int (*g_rbs_qsee_com)(
    RBS_TLC_HANDLE_T *l_tlcComHandle, unsigned char *req_tci_data,
    int req_tci_size, unsigned char *rsp_tci_data, int *rsp_tci_size,
    unsigned char *in_data, unsigned int in_data_len) = NULL;

static int check_connection_open()
{
    int retval = FINGERPRINT_RES_SUCCESS;

    const char *default_path = "libets_teeclient_v2.so";
#ifndef __aarch64__
	const char *other_path = "/odm/lib/hw/libets_teeclient_v2.so";
#else
	const char *other_path = "/odm/lib64/hw/libets_teeclient_v2.so";
#endif

    if (NULL == g_tlcComHandle) {
        if (!g_filehandle) {
            ex_log(LOG_DEBUG, "first, open file default_path = %s", default_path);
            g_filehandle = dlopen(default_path, RTLD_LAZY);
            if (NULL == g_filehandle) {
                ex_log(LOG_DEBUG, "%s, qsee_init first Failed", default_path);
				ex_log(LOG_DEBUG, "second, try again open file other_path = %s", other_path);
                g_filehandle = dlopen(other_path, RTLD_LAZY);
            }
        }

        if (NULL == g_filehandle) {
            ex_log(LOG_ERROR, "%s, qsee_init second Failed", other_path);
            return FINGERPRINT_RES_FAILED;
        }

        if (!g_start_app) {
            g_start_app = dlsym(g_filehandle, "qsc_start_app");
        }

        if (!g_shut_down_app) {
            g_shut_down_app = dlsym(g_filehandle, "qsc_shutdown_app");
        }

        if (!g_rbs_send_cmd) {
            g_rbs_send_cmd = dlsym(g_filehandle, "ets_issue_send_modified_cmd_req");
        }

        if (!g_rbs_send_cmd_and_in_data) {
            g_rbs_send_cmd_and_in_data = dlsym(g_filehandle,  "ets_issue_send_modified_cmd_req_in_data");
        }

        if (!g_rbs_qsee_com) {
            g_rbs_qsee_com = dlsym(g_filehandle, "ets_qsee_cmd_req_in_data");
        }

        if (g_start_app) {
            retval = g_start_app(&g_tlcComHandle, appname, 9216);
        }

        if (retval) {
            ex_log(LOG_ERROR,
                   "Start RbsFpAPP app: fail, result = %d, "
                   "app_name = %s",
                   retval, appname);
            return FINGERPRINT_RES_FAILED;
        } else {
            retval = FINGERPRINT_RES_SUCCESS;
            ex_log(LOG_DEBUG, "Start RbsFpAPP app: pass");
        }
    }

    if (!g_tlcComHandle) {
        ex_log(LOG_ERROR, "g_tlcComHandle is NULL");
        retval = FINGERPRINT_RES_FAILED;
    }

    return retval;
}

static int reset_connection()
{
    int retval = FINGERPRINT_RES_SUCCESS;

    if (NULL == g_shut_down_app) {
        goto EXIT;
    }

    if (NULL != g_tlcComHandle) {
        retval = g_shut_down_app(&g_tlcComHandle);
        if (retval) {
            ex_log(LOG_ERROR, "Shutdown app failed with ret = %d", retval);
            retval = FINGERPRINT_RES_FAILED;
        }
    } else {
        retval = FINGERPRINT_RES_FAILED;
        ex_log(LOG_ERROR, "cannot shutdown as the handle is NULL");
    }

EXIT:
    if (NULL != g_filehandle) {
        dlclose(g_filehandle);
        g_filehandle = NULL;
    }

    g_tlcComHandle = NULL;
    g_start_app = NULL;
    g_shut_down_app = NULL;
    g_rbs_send_cmd = NULL;
    g_rbs_send_cmd_and_in_data = NULL;
    g_rbs_qsee_com = NULL;

    return retval;
}

static int keep_connection_open()
{
    int retval;

    if (NULL != g_tlcComHandle &&
        NULL != g_rbs_send_cmd &&
        NULL != g_shut_down_app) {
        return FINGERPRINT_RES_SUCCESS;
    }

    retval = reset_connection();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        return FINGERPRINT_RES_FAILED;
    }

    return check_connection_open();
}

int transporter(unsigned char *msg_data, unsigned int msg_data_len,
        unsigned char *out_data, unsigned int *out_data_len)
{
    int retval;

    if (NULL == msg_data ||msg_data_len == 0) {
        ex_log(LOG_ERROR, "invalid input params\n");
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    retval = keep_connection_open();
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "keep_connection_open failed\n");
        return FINGERPRINT_RES_FAILED;
    }
    ((transfer_package_t*)msg_data)->process += CLIENT_CMD_ETS_MESSAGE;
    ex_log(LOG_DEBUG, "-- msg_data->process = %d ", ((transfer_package_t*)msg_data)->process);
    ex_log(LOG_DEBUG, "-- msg_data->command = %d ", ((transfer_package_t *)msg_data)->command);

    retval = g_rbs_qsee_com(g_tlcComHandle,
        msg_data,
        msg_data_len,
        msg_data,
        NULL,
        out_data,
        *out_data_len);

    if ((out_data_len != NULL) && (out_data != NULL) && (*out_data_len > 0)) {
        ex_log(LOG_DEBUG, " out_data_len  = %d ", *out_data_len);
    }

    ex_log(LOG_DEBUG, "transporter end, retval = %d ", retval);
    return retval;
}
