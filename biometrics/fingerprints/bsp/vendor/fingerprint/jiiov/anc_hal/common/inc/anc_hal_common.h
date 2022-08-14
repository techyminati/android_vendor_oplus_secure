#ifndef ANC_HAL_COMMON_H
#define ANC_HAL_COMMON_H


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Below from android/hardware/libhardware/include/hardware/fingerprint.h
 */
typedef enum {
    ANC_FINGERPRINT_ERROR_HW_UNAVAILABLE = 1, /* The hardware has an error that can't be resolved. */
    ANC_FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2, /* Bad data; operation can't continue */
    ANC_FINGERPRINT_ERROR_TIMEOUT = 3, /* The operation has timed out waiting for user input. */
    ANC_FINGERPRINT_ERROR_NO_SPACE = 4, /* No space available to store a template */
    ANC_FINGERPRINT_ERROR_CANCELED = 5, /* The current operation can't proceed. See above. */
    ANC_FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6, /* fingerprint with given id can't be removed */
    ANC_FINGERPRINT_ERROR_LOCKOUT = 7, /* the fingerprint hardware is in lockout due to too many attempts */
    ANC_FINGERPRINT_ERROR_HAL_INITED = 998, /*hal init done*/
    ANC_FINGERPRINT_ERROR_DO_RECOVER = 999, /*fingerprint recover. */
    ANC_FINGERPRINT_ERROR_VENDOR_BASE = 1000, /* vendor-specific error messages start here */
    ANC_FINGERPRINT_ERROR_HBM_TIMEOUT = ANC_FINGERPRINT_ERROR_VENDOR_BASE + 1,
}ANC_FINGERPRINT_ERROR_TYPE;

typedef enum {
    ANC_FINGERPRINT_ACQUIRED_GOOD = 0,
    ANC_FINGERPRINT_ACQUIRED_PARTIAL = 1, /* sensor needs more data, i.e. longer swipe. */
    ANC_FINGERPRINT_ACQUIRED_INSUFFICIENT = 2, /* image doesn't contain enough detail for recognition*/
    ANC_FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3, /* sensor needs to be cleaned */
    ANC_FINGERPRINT_ACQUIRED_TOO_SLOW = 4, /* mostly swipe-type sensors; not enough data collected */
    ANC_FINGERPRINT_ACQUIRED_TOO_FAST = 5, /* for swipe and area sensors; tell user to slow down*/
    ANC_FINGERPRINT_ACQUIRED_DETECTED = 6, /* when the finger is first detected. Used to optimize wakeup.
                                          Should be followed by one of the above messages */
    ANC_FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000, /* vendor-specific acquisition messages start here */
    ANC_FINGERPRINT_ACQUIRED_TOO_SIMILAR = ANC_FINGERPRINT_ACQUIRED_VENDOR_BASE + 1, /*for similar enrolled area*/
    ANC_FINGERPRINT_ACQUIRED_ALREADY_ENROLLED = ANC_FINGERPRINT_ACQUIRED_VENDOR_BASE + 2, /*for the same fingerprint as enrolled*/
}ANC_FINGERPRINT_ACQUIRED_TYPE;

typedef enum {
    ANC_EXTERNAL_BIND_CORE = 1,
    ANC_EXTERNAL_SCALE_CPU_FREQUENCY = 2,
    ANC_SEND_DCS_EVENT_INFO = 3,
    ANC_SETUXTHREAD = 4
}ANC_EXTERNAL_TYPE;

#define ANC_SCALE_CPU_FREQUENCY_TIMEOUT_500  500
#define ANC_SCALE_CPU_FREQUENCY_TIMEOUT_1000 1000
#define ANC_SCALE_CPU_FREQUENCY_TIMEOUT_2000 2000
#define ANC_SCALE_CPU_FREQUENCY_TIMEOUT_3000 3000


/**
 * Data format for an authentication record used to prove successful authentication.
 */
typedef struct __attribute__((__packed__)) {
    uint8_t version;  // Current version is 0
    uint64_t challenge;
    uint64_t user_id;             // secure user ID, not Android user ID
    uint64_t authenticator_id;    // secure authenticator ID
    uint32_t authenticator_type;  // hw_authenticator_type_t, in network order
    uint64_t timestamp;           // in network order
    uint8_t hmac[32];
} AncHwAuthToken;

struct AncFingerprintDevice;

typedef struct {

    uint64_t (*PreEnroll)(struct AncFingerprintDevice *dev);
    int (*Enroll)(struct AncFingerprintDevice *dev, const AncHwAuthToken *hat,
                    uint32_t gid, uint32_t timeout_sec);
    int (*PostEnroll)(struct AncFingerprintDevice *dev);
    uint64_t (*GetAuthenticatorId)(struct AncFingerprintDevice *dev);
    int (*Cancel)(struct AncFingerprintDevice *dev);
    int (*Enumerate)(struct AncFingerprintDevice *dev);
    int (*Remove)(struct AncFingerprintDevice *dev, uint32_t gid, uint32_t fid);
    int (*SetActiveGroup)(struct AncFingerprintDevice *dev, uint32_t gid,
                            const char *store_path);
    int (*Authenticate)(struct AncFingerprintDevice *dev, uint64_t operation_id, uint32_t gid);

}AncFingerprintWorker;

typedef struct {

    void (*OnEnrollResult)(void *p_device, uint32_t finger_id, uint32_t group_id, uint32_t remaining);
    void (*OnAcquired)(void *p_device, int32_t vendor_code);
    void (*OnAuthenticated)(void *p_device, uint32_t finger_id, uint32_t group_id,
                             const uint8_t* token, uint32_t token_length);
    void (*OnError)(void *p_device, int vendor_code);
    void (*OnRemoved)(void *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);
    void (*OnEnumerate)(void *p_device, uint32_t finger_id, uint32_t group_id,
                       uint32_t remaining);

}AncFingerprintWorkerCallback;

typedef struct {

    int (*ExcuteCommand)(struct AncFingerprintDevice *dev, int32_t command_id,
                       const uint8_t *p_in_param , uint32_t in_param_length,
                       uint8_t **p_output_buffer , uint32_t *p_output_buffer_length);

}AncFingerprintExtensionWorker;


typedef struct {

    void (*OnExcuteCommand)(void *p_device, int32_t command_id, int32_t argument, const uint8_t *out, uint32_t out_length);

}AncFingerprintExtensionWorkerCallback;

typedef struct {

    void (*DoWork)(void *p_device, int32_t type, const uint8_t *p_buffer, uint32_t buffer_length);

}AncFingerprintExternalWorker;

typedef struct AncFingerprintDevice {

    AncFingerprintWorker fp_worker;
    AncFingerprintWorkerCallback fp_worker_callback;
    AncFingerprintExtensionWorker fpe_worker;
    AncFingerprintExtensionWorkerCallback fpe_worker_callback;

    AncFingerprintExternalWorker fp_external_worker;

    void *p_data;

}AncFingerprintDevice;


#ifdef __cplusplus
}
#endif

#endif
