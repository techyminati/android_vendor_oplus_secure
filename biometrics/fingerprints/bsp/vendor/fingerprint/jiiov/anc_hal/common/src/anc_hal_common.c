#define LOG_TAG "[ANC_HAL][Common]"

#include "anc_hal_common.h"

#include "anc_log.h"
#include <stdio.h>


#include "anc_hal.h"
#include "anc_hal_manager.h"
#include "anc_common_type.h"


// --------   AncFingerprintWorker   --------------

static uint64_t FpPreEnroll(struct AncFingerprintDevice *p_device) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->PreEnroll != NULL) {
            return p_manager->p_producer->PreEnroll(p_manager);
        }
    }

    return 0;
}

static int FpEnroll(struct AncFingerprintDevice *p_device, const AncHwAuthToken *p_hat,
                    uint32_t gid, uint32_t timeout_sec) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->Enroll != NULL) {
            return p_manager->p_producer->Enroll(p_manager, p_hat, gid, timeout_sec);
        }
    }


    return -1;
}

static int FpPostEnroll(struct AncFingerprintDevice *p_device) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->PostEnroll != NULL) {
            return p_manager->p_producer->PostEnroll(p_manager);
        }
    }

    return -1;
}

static uint64_t FpGetAuthenticatorId(struct AncFingerprintDevice *p_device) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->GetAuthenticatorId != NULL) {
            return p_manager->p_producer->GetAuthenticatorId(p_manager);
        }
    }

    return 0;
}

static int FpCancel(struct AncFingerprintDevice *p_device) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->Cancel != NULL) {
            return p_manager->p_producer->Cancel(p_manager);
        }
    }

    return -1;
}

static int FpEnumerate(struct AncFingerprintDevice *p_device) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->Enumerate != NULL) {
            return p_manager->p_producer->Enumerate(p_manager);
        } else {
            ANC_LOGE("Enumerate, manager is null");
        }
    } else {
        ANC_LOGE("Enumerate, manager is null");
    }

    return -1;
}

static int FpRemove(struct AncFingerprintDevice *p_device, uint32_t gid, uint32_t fid) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->Remove != NULL) {
            return p_manager->p_producer->Remove(p_manager, gid, fid);
        }
    }

    return -1;
}

static int FpSetActiveGroup(struct AncFingerprintDevice *p_device, uint32_t gid,
                            const char *p_store_path) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->SetActiveGroup != NULL) {
            return p_manager->p_producer->SetActiveGroup(p_manager, gid, p_store_path);
        }
    }

    return -1;
}

static int FpAuthenticate(struct AncFingerprintDevice *p_device, uint64_t operation_id,
                       uint32_t gid) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->Authenticate != NULL) {
            return p_manager->p_producer->Authenticate(p_manager, operation_id, gid);
        }
    }

    return -1;
}


// --------   AncFingerprintExtensionWorker   --------------
static int FpeExcuteCommand(AncFingerprintDevice *p_device, int32_t command_id,
                       const uint8_t *p_in_param , uint32_t in_param_length,
                       uint8_t **p_output_buffer , uint32_t *p_output_buffer_length) {
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_device->p_data;

    if (p_manager != NULL) {
        if (p_manager->p_producer->ExcuteCommand != NULL) {
            return p_manager->p_producer->ExcuteCommand(p_manager, command_id,
                         p_in_param , in_param_length,
                         p_output_buffer, p_output_buffer_length);
        }
    }

    return -1;
}




static AncFingerprintDevice g_fp_device = {
    .fp_worker.PreEnroll = FpPreEnroll,
    .fp_worker.Enroll = FpEnroll,
    .fp_worker.PostEnroll = FpPostEnroll,
    .fp_worker.GetAuthenticatorId = FpGetAuthenticatorId,
    .fp_worker.Cancel = FpCancel,
    .fp_worker.Enumerate = FpEnumerate,
    .fp_worker.Remove = FpRemove,
    .fp_worker.SetActiveGroup = FpSetActiveGroup,
    .fp_worker.Authenticate = FpAuthenticate,

    .fpe_worker.ExcuteCommand = FpeExcuteCommand,
};

// return value : 0 - on success, negative - on failure
int InitFingerprintDevice(void **p_device) {
    int ret_val = 0;

    ANC_LOGD("init fingerprint device");
    *p_device = (void *)(&g_fp_device);


    if (ANC_OK != (ret_val = (int)InitFingerprintManager(&g_fp_device))) {
        ANC_LOGE("fail to init anc manager");
        ret_val = -1;
    }

    return ret_val;
}

// return value : 0 - on success, negative - on failure
int DeinitFingerprintDevice(void *p_device) {
    int ret_val = 0;

    ANC_LOGD("deinit fingerprint device");
    if (ANC_OK != (ret_val = (int)DeinitFingerprintManager((AncFingerprintDevice *)p_device))) {
        ANC_LOGE("fail to deinit anc manager");
        ret_val = -1;
    }

    return ret_val;
}

uint64_t AncPreEnroll(void *p_device) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    return p_finger_print_device->fp_worker.PreEnroll(p_finger_print_device);
}

int AncEnroll(void *p_device, const uint8_t *p_hat,
                    uint32_t gid, uint32_t timeout_sec) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;
    const AncHwAuthToken *p_auth_token = (AncHwAuthToken *)p_hat;

    return p_finger_print_device->fp_worker.Enroll(p_finger_print_device, p_auth_token, gid, timeout_sec);
}

int AncPostEnroll(void *p_device) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    return p_finger_print_device->fp_worker.PostEnroll(p_finger_print_device);
}

uint64_t AncGetAuthenticatorId(void *p_device) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    return p_finger_print_device->fp_worker.GetAuthenticatorId(p_finger_print_device);
}

int AncCancel(void *p_device) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    return p_finger_print_device->fp_worker.Cancel(p_finger_print_device);
}

int AncEnumerate(void *p_device) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;
    ANC_LOGD("AncEnumerate");
    return p_finger_print_device->fp_worker.Enumerate(p_finger_print_device);
}

int AncRemove(void *p_device, uint32_t gid, uint32_t fid) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;
    ANC_LOGD("AncRemove");
    return p_finger_print_device->fp_worker.Remove(p_finger_print_device, gid, fid);
}

int AncSetActiveGroup(void *p_device, uint32_t gid,
                            const char *p_store_path) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;
    ANC_LOGD("AncSetActiveGroup");
    return p_finger_print_device->fp_worker.SetActiveGroup(p_finger_print_device, gid, p_store_path);
}

int AncAuthenticate(void *p_device, uint64_t operation_id, uint32_t gid) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    return p_finger_print_device->fp_worker.Authenticate(p_finger_print_device, operation_id, gid);
}

int AncExcuteCommand(void *p_device, int32_t command_id,
                       const uint8_t *p_in_param, uint32_t in_param_length,
                       uint8_t **p_output_buffer, uint32_t *p_output_buffer_length) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    return p_finger_print_device->fpe_worker.ExcuteCommand(p_finger_print_device, command_id,
                       p_in_param , in_param_length, p_output_buffer, p_output_buffer_length);
}


void SetEnrollCallBackFunction(void *p_device, EnrollCallBackFunction OnEnrollResult) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_worker_callback.OnEnrollResult = OnEnrollResult;
}

void SetAcquireCallBackFunction(void *p_device, AcquireCallBackFunction OnAcquired) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_worker_callback.OnAcquired = OnAcquired;
}

void SetAuthenticateCallBackFunction(void *p_device, AuthenticateCallBackFunction OnAuthenticated) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_worker_callback.OnAuthenticated = OnAuthenticated;
}

void SetErrorCallBackFunction(void *p_device, ErrorCallBackFunction OnError) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_worker_callback.OnError = OnError;
}

void SetRemoveCallBackFunction(void *p_device, RemoveCallBackFunction OnRemoved) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_worker_callback.OnRemoved = OnRemoved;
}

void SetEnumerateCallBackFunction(void *p_device, EnumerateCallBackFunction OnEnumerate) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_worker_callback.OnEnumerate = OnEnumerate;
}

void SetExcuteCommandCallBackFunction(void *p_device, ExcuteCommandCallBackFunction OnExcuteCommand) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fpe_worker_callback.OnExcuteCommand = OnExcuteCommand;
}

void SetExternalWorkerFunction(void *p_device, ExternalWorkerFunction DoWork) {
    AncFingerprintDevice *p_finger_print_device = (AncFingerprintDevice *)p_device;

    p_finger_print_device->fp_external_worker.DoWork = DoWork;
}
