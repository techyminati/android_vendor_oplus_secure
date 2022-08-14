/*
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
*/

#ifndef FPC_HAL_EXT_ENGINEERING_H
#define FPC_HAL_EXT_ENGINEERING_H

#include <inttypes.h>
#include <stdbool.h>
#include "fpc_tee_hal.h"

#define COVERAGE_UNKNOWN -1
#define QUALITY_UNKNOWN -1
#define REMAINING_UNKNOWN -1

#define CAC_EARLY_STOP_DISABLE 0

typedef enum {
    RAW = 0,
    PREPROCESSED,
} fpc_hal_img_type_t;

typedef struct {
    fpc_hal_img_type_t image_type;
    uint8_t *buffer;
    uint32_t buffer_size;
} fpc_hal_img_data_t;

typedef enum {
    //needs to be in sync with FingerprintEngineering.java
    FPC_CAPTURE_MODE_CAPTURE = 0,
    FPC_CAPTURE_MODE_VERIFY = 1,
    FPC_CAPTURE_MODE_ENROLL = 2,
} fpc_capture_mode_t;

typedef struct {
    fpc_capture_mode_t mode;
    int32_t capture_result;
    int32_t template_update_result;
    int32_t identify_result;
    int32_t enroll_result;
    int32_t cac_result;
    uint32_t user_id;
    int32_t samples_remaining;
    int32_t coverage;
    int32_t quality;
    fpc_hal_img_data_t raw_image;
    fpc_hal_img_data_t enhanced_image;
} fpc_capture_data_t;

typedef void (*fpc_img_subscr_cb_t)(void *ctx, fpc_capture_data_t *capture_data);
typedef int (*fpc_img_inj_cb_t)(void *ctx, fpc_hal_img_data_t *img_data);
typedef void (*fpc_img_inj_cancel_cb_t)(void *ctx);
typedef void (*fpc_capture_cb_t)(void *ctx, fpc_capture_data_t *capture_data);

typedef struct fpc_engineering fpc_engineering_t;

struct fpc_engineering {
    void (*get_sensor_size)(fpc_engineering_t *self, uint8_t *width, uint8_t *height);
    int (*set_img_subscr_cb)(fpc_engineering_t *self, fpc_capture_cb_t callback, void *ctx);
    int (*handle_image_subscription)(fpc_engineering_t *self);
    int (*handle_image_subscription_enroll)(fpc_engineering_t *self, int capture_result,
                                            int enroll_result, int samples_remaining,
                                            uint32_t fid);
    int (*handle_image_subscription_auth)(fpc_engineering_t *self, int capture_result,
                                          int identify_result, int32_t coverage, int32_t quality,
                                          uint32_t fid);
    bool (*is_img_inj_enabled)(fpc_engineering_t *self);
    int (*set_img_inj_cb)(fpc_engineering_t *self,
                          fpc_img_inj_cb_t callback, fpc_img_inj_cancel_cb_t cancel, void *ctx);
    int (*handle_image_injection)(fpc_engineering_t *self);
    void (*cancel_image_injection)(fpc_engineering_t *self);
    int (*start_capture)(fpc_engineering_t *self, fpc_capture_cb_t callback,
                         fpc_capture_mode_t mode, void *ctx);
    int (*cancel_capture)(fpc_engineering_t *self);
    int (*set_enroll_token)(fpc_engineering_t *self, const uint8_t *token, ssize_t token_length);
    int (*get_enroll_challenge)(fpc_engineering_t *self, uint64_t *challenge);
    int (*get_build_info)(fpc_engineering_t *self, void (*callback)(const char *build_info, void *usr),
                          void *usr);
};

fpc_engineering_t *fpc_engineering_new(fpc_hal_common_t *hal);
int fpc_hal_ext_get_raw_image(fpc_engineering_t *self, fpc_hal_img_data_t *image_data);
int fpc_hal_ext_get_enhanced_image(fpc_engineering_t *self, fpc_hal_img_data_t *image_data);
void fpc_hal_ext_free_image(fpc_hal_img_data_t *image);

void fpc_engineering_destroy(fpc_engineering_t *self);

#endif // FPC_HAL_EXT_ENGINEERING_PRIVATE_H
