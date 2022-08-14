#ifndef ANC_HAL_H
#define ANC_HAL_H

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*EnrollCallBackFunction)(void *p_device, uint32_t finger_id, uint32_t group_id, uint32_t remaining);
typedef void (*AcquireCallBackFunction)(void *p_device, int32_t vendor_code);
typedef void (*AuthenticateCallBackFunction)(void *p_device, uint32_t finger_id, uint32_t group_id,
                             const uint8_t* token, uint32_t token_length);
typedef void (*ErrorCallBackFunction)(void *p_device, int vendor_code);
typedef void (*RemoveCallBackFunction)(void *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
typedef void (*EnumerateCallBackFunction)(void *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
typedef void (*ExcuteCommandCallBackFunction)(void *p_device, int32_t command_id, int32_t argument, const uint8_t *out, uint32_t out_length);

typedef void (*ExternalWorkerFunction)(void *p_device, int32_t type, const uint8_t *p_buffer, uint32_t buffer_length);


// return value : 0 - on success, negative - on failure
int InitFingerprintDevice(void **p_device);
// return value : 0 - on success, negative - on failure
int DeinitFingerprintDevice(void *p_device);

uint64_t AncPreEnroll(void *p_device);
int AncEnroll(void *p_device, const uint8_t *p_hat,
                    uint32_t gid, uint32_t timeout_sec);
int AncPostEnroll(void *p_device);
uint64_t AncGetAuthenticatorId(void *p_device);
int AncCancel(void *p_device);
int AncEnumerate(void *p_device);
int AncRemove(void *p_device, uint32_t gid, uint32_t fid);
int AncSetActiveGroup(void *p_device, uint32_t gid,
                            const char *p_store_path);
int AncAuthenticate(void *p_device, uint64_t operation_id, uint32_t gid);
int AncExcuteCommand(void *p_device, int32_t command_id,
                       const uint8_t *p_in_param, uint32_t in_param_length,
                       uint8_t **p_output_buffer, uint32_t *p_output_buffer_length);

void SetEnrollCallBackFunction(void *p_device, EnrollCallBackFunction OnEnrollResult);
void SetAcquireCallBackFunction(void *p_device, AcquireCallBackFunction OnAcquired);
void SetAuthenticateCallBackFunction(void *p_device, AuthenticateCallBackFunction OnAuthenticated);
void SetErrorCallBackFunction(void *p_device, ErrorCallBackFunction OnError);
void SetRemoveCallBackFunction(void *p_device, RemoveCallBackFunction OnRemoved);
void SetEnumerateCallBackFunction(void *p_device, EnumerateCallBackFunction OnEnumerate);
void SetExcuteCommandCallBackFunction(void *p_device, ExcuteCommandCallBackFunction OnExcuteCommand);

void SetExternalWorkerFunction(void *p_device, ExternalWorkerFunction DoWork);

#ifdef __cplusplus
}
#endif

#endif
