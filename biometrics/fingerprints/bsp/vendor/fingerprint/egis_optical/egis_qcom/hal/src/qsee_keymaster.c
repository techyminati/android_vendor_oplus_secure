#include "qsee_keymaster.h"
#include "plat_log.h"
#include <dlfcn.h>
#include <string.h>
//#include <fp_err.h>
#include "fp_client_lib.h"

#define KM_TA_NAME "keymaster64"
#define LOG_TAG "egis_fp_hal"
#define KEYMASTER_UTILS_CMD_ID 0x200UL

#define RESULT_OK 0
#define RESULT_FAILED -1

#define CLIENT_LIB "libets_teeclient_v3.so"

typedef enum
{
	KEYMASTER_NONE = 0,
	KEYMASTER_GET_AUTH_TOKEN_KEY = KEYMASTER_UTILS_CMD_ID + 5ULL,
	KEYMASTER_ANY = (int)UINT32_MAX,
}keymaster_cmd_t;

typedef enum {
	HW_AUTH_NONE = 0,
	HW_AUTH_PASSWORD = 1 << 0,
	HW_AUTH_FINGERPRINT = 1 << 1,
	HW_AUTH_ANY = UINT32_MAX,
} hw_authenticator_type_t;

typedef struct _km_get_auth_token_req_t
{
	keymaster_cmd_t cmd_id;
	hw_authenticator_type_t auth_type;
}__attribute__((packed)) KM_GET_AUTH_TOKEN_REQ_T;

typedef struct _km_get_auth_token_rsp_t
{
	int status;
	uint32_t auth_token_key_offset;
	uint32_t auth_token_key_len;
	uint8_t data[1024];
}__attribute__((packed)) KM_GET_AUTH_TOKEN_RSP_T;

int (*f_start_app) (EGIS_TLC_HANDLE_T **tlcComHandle, const char *app_name, int sb_size) = NULL;
int (*f_shutdown_app) (EGIS_TLC_HANDLE_T **tlcComHandle) = NULL;
int (*f_ets_keymaster_send_cmd) (EGIS_TLC_HANDLE_T *tlcComHandle,
					unsigned char *req_tci_data,
					int	req_tci_size,
					unsigned char *rsp_tci_data,
					int *rsp_tci_size) = NULL;

unsigned int get_secure_key(unsigned char* blob, unsigned int* blob_len)
{
	unsigned int retval = RESULT_OK;
	int result;
	void *dl_handle = NULL;
	EGIS_TLC_HANDLE_T *tlc_handle = NULL;
	void *msg_req;
	void *msg_rsp;
	int msg_req_len;
	int msg_rsp_len;

	KM_GET_AUTH_TOKEN_REQ_T km_auth_req;
	KM_GET_AUTH_TOKEN_RSP_T km_auth_rsp;

	egislog_i("get_secure_key enter\n");

	if(blob == NULL || blob_len == NULL)
	{
		egislog_e("invalid params");
		return -1;
	}

	dl_handle = dlopen(CLIENT_LIB, RTLD_LAZY);
	if(dl_handle == NULL)
	{
		egislog_e("load library failed %s", CLIENT_LIB);
		char* error_msg = dlerror();
		egislog_e("error_msg %s", error_msg);
		return -1;
	}

	f_start_app = dlsym(dl_handle, "qsc_start_app");
	f_shutdown_app = dlsym(dl_handle, "qsc_shutdown_app");
	f_ets_keymaster_send_cmd = dlsym(dl_handle, "ets_keymaster_issue_send_modified_cmd_req");

	if(f_start_app == NULL || f_shutdown_app == NULL || f_ets_keymaster_send_cmd == NULL)
	{
		egislog_e("load function failed");
		retval = -1;
		goto err1;
	}

	result =  f_start_app(&tlc_handle, KM_TA_NAME, 9216);
	if(result != 0 || tlc_handle == NULL)
	{
		egislog_e("start app failed");
		retval = -1;
		goto err1;
	}

	km_auth_req.cmd_id = KEYMASTER_GET_AUTH_TOKEN_KEY;
	km_auth_req.auth_type = HW_AUTH_FINGERPRINT;

	msg_req = &km_auth_req;
	msg_rsp = &km_auth_rsp;
	msg_req_len = sizeof(KM_GET_AUTH_TOKEN_REQ_T);
	msg_rsp_len = sizeof(KM_GET_AUTH_TOKEN_RSP_T);

	memset(msg_rsp, 0, msg_rsp_len);

	result = f_ets_keymaster_send_cmd(tlc_handle, msg_req, msg_req_len, msg_rsp, &msg_rsp_len);
	if(result != 0)
	{
		egislog_e("f_ets_keymaster_send_cmd failed, result = %d", result);
		retval = -1;
		goto err2;
	}

	if(km_auth_rsp.status != 0)
	{
		egislog_e("get master key failed , status = %d", km_auth_rsp.status);
		retval = -1;
		goto err2;
	}

	if(*blob_len < km_auth_rsp.auth_token_key_len)
	{
		egislog_e("output buffer is too short");
		retval = -1;
		goto err2;
	}

	*blob_len = km_auth_rsp.auth_token_key_len;
	memcpy(blob, km_auth_rsp.data, km_auth_rsp.auth_token_key_len);

err2:
	result = f_shutdown_app(&tlc_handle);
	if(result != 0)
	{
		egislog_e("shutdown keymaster app failed");
	}

err1:
	dlclose(dl_handle);
	f_start_app = NULL;
	f_shutdown_app = NULL;
	f_ets_keymaster_send_cmd = NULL;

	return retval;
}
