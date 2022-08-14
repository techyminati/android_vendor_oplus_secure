#ifndef FP_HAL_EXT_SERVICE_H_
#define FP_HAL_EXT_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <hardware/fingerprint.h>
#include "fp_eng_test.h"

typedef struct fingerprint_ext_device {
    fingerprint_device_t *dev;
    int (*update_status)(fingerprint_device_t *dev,uint32_t status);
    uint32_t (*get_status)(fingerprint_device_t *dev);
    int (*get_eng_test)(fingerprint_device_t *dev, fp_eng_tests_t *eng_test);
    int (*set_eng_notify)(fingerprint_device_t *dev, fingerprint_eng_notify_t eng_notify);
    int (*start_eng_test)(fingerprint_device_t *dev, uint32_t cmd);
    int (*stop_eng_test)(fingerprint_device_t *dev, uint32_t cmd);
    int (*send_command)(fingerprint_device_t *dev, uint32_t cmd);
    int (*set_param)(fingerprint_device_t *dev, uint32_t cmd);

} fingerprint_ext_device_t;

extern void add_fingerprint_ext_service(fingerprint_ext_device_t *dev);

#ifdef __cplusplus
}
#endif

#endif // FP_HAL_EXT_SERVICE_H_
