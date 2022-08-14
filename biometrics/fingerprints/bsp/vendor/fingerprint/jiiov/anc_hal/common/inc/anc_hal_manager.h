#ifndef ANC_HAL_MANAGER_H
#define ANC_HAL_MANAGER_H

#include <pthread.h>

#include "anc_hal_common.h"
#include "anc_hal_work_consumer.h"
#include "anc_hal_extension_command_common.h"
#include "anc_hal_netlink_event_manager.h"
#include "anc_hal_tp_event_manager.h"
#include "anc_hal_hbm_event_manager.h"
#include "anc_hal_sensor_manager.h"
#include "anc_error.h"
#include "anc_tac_dcs.h"
#include "anc_hal_dcs_info.h"

#undef ANC_PATH_MAX
#define ANC_PATH_MAX 256
struct AncFingerprintManager;
typedef struct {
    ANC_RETURN_TYPE (*Init)(struct AncFingerprintManager *p_manager);
    ANC_RETURN_TYPE (*Deinit)(struct AncFingerprintManager *p_manager);
    void (*PushTaskToConsumer)(struct AncFingerprintManager *p_manager,
                              void (*Dowork)(void *p_arg), void *p_arg,
                              uint8_t *p_name);

    uint64_t (*PreEnroll)(struct AncFingerprintManager *p_manager);
    int (*Enroll)(struct AncFingerprintManager *p_manager,
                  const AncHwAuthToken *p_hat, uint32_t gid, uint32_t timeout_sec);
    int (*PostEnroll)(struct AncFingerprintManager *p_manager);
    uint64_t (*GetAuthenticatorId)(struct AncFingerprintManager *p_manager);
    int (*Cancel)(struct AncFingerprintManager *p_manager);
    int (*Enumerate)(struct AncFingerprintManager *p_manager);
    int (*Remove)(struct AncFingerprintManager *p_manager, uint32_t gid, uint32_t fid);
    int (*SetActiveGroup)(struct AncFingerprintManager *p_manager,
                  uint32_t gid, const char *p_store_path);
    int (*Authenticate)(struct AncFingerprintManager *p_manager,
                  uint64_t operation_id, uint32_t gid);
    int (*ExcuteCommand)(struct AncFingerprintManager *p_manager,
                  int32_t command_id, const uint8_t *p_in_param , uint32_t in_param_length,
                  uint8_t **p_output_buffer , uint32_t *p_output_buffer_length);

    void (*OnEnrollResult)(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id, uint32_t remaining);
    void (*OnAcquired)(AncFingerprintDevice *p_device, int32_t vendor_code);
    void (*OnAuthenticated)(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                             const uint8_t* token, uint32_t token_length);
    void (*OnError)(AncFingerprintDevice *p_device, int vendor_code);
    void (*OnRemoved)(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
    void (*OnEnumerate)(AncFingerprintDevice *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
    void (*OnExcuteCommand)(AncFingerprintDevice *p_device, int32_t command_id, int32_t argument, const uint8_t *out, uint32_t out_length);

    ANC_RETURN_TYPE (*StartHeartBeatRateDetect)(struct AncFingerprintManager *p_manager);

    AncFingerprintDevice *p_device;
    AncHwAuthToken auth_token;
    uint32_t current_group_id;
    uint32_t enroll_timeout_second;
    char current_database_path[ANC_PATH_MAX];
    uint64_t challenge;
    uint64_t authenticator_id;
    uint32_t remove_finger_id;
    oplus_fingerprint_dcs_auth_result_type_t dcs_auth_result_type;
    uint32_t retry_count;
    uint32_t finger_id;
    uint32_t AuthScreenState;
    AncHalExtensionCommand command;

    pthread_mutex_t mutex;
    pthread_mutex_t reportup_mutex;
}AncFingerprintWorkProducer;

typedef struct {

    ANC_RETURN_TYPE (*DoWork1)(struct AncFingerprintDevice *p_device, int32_t type);
    ANC_RETURN_TYPE (*DoWork2)(struct AncFingerprintDevice *p_device, const uint8_t *p_buffer, uint32_t buffer_length);
    ANC_RETURN_TYPE (*DoWork3)(struct AncFingerprintDevice *p_device, int32_t type, const uint8_t *p_buffer, uint32_t buffer_length);


    AncFingerprintDevice *p_device;

}AncExternalFeatureManager;


typedef struct sAncFingerprintWorkConsumer AncFingerprintWorkConsumer;
typedef struct sAncNetlinkEventManager AncNetlinkEventManager;
typedef struct sAncTpEventManager AncTpEventManager;
typedef struct sAncHbmEventManager AncHbmEventManager;
typedef struct sAncSensorManager AncSensorManager;
typedef struct AncFingerprintManager{

    AncFingerprintWorkProducer *p_producer;
    AncFingerprintWorkConsumer *p_consumer;
    AncNetlinkEventManager *p_netlink_event_manager;
    AncTpEventManager *p_tp_event_manager;
    AncHbmEventManager *p_hbm_event_manager;
    AncSensorManager *p_sensor_manager;

    AncExternalFeatureManager *p_external_feature_manager;

}AncFingerprintManager;


ANC_RETURN_TYPE InitFingerprintManager(AncFingerprintDevice *p_device);
ANC_RETURN_TYPE DeinitFingerprintManager(AncFingerprintDevice *p_device);

#endif
