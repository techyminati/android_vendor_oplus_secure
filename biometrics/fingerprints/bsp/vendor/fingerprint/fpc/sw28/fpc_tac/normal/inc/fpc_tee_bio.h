/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef FPC_TEE_BIO_H
#define FPC_TEE_BIO_H
#include "stdint.h"
#include "fpc_tee.h"
#include "fpc_ta_bio_interface.h"

typedef struct fpc_tee_bio fpc_tee_bio_t;

typedef enum finger_store_template_mode{
    ADD_TEMPLATE_MODE,
    REMOVE_TEMPLATE_MODE,
    UPDATE_TEMPLATE_MODE
} finger_store_template_mode_t;

typedef struct finger_store_template_config {
    const char* path;
    finger_store_template_mode_t template_mode;
} finger_store_template_config_t;

#define FP_TEMPLATE_DB_BACKUP_0 "bak0"
#define FP_TEMPLATE_DB_BACKUP_1 "bak1"

fpc_tee_bio_t* fpc_tee_bio_init(fpc_tee_t* tee);
void fpc_tee_bio_release(fpc_tee_bio_t* tee);

int fpc_tee_set_gid(fpc_tee_bio_t* tee, uint32_t gid);
int fpc_tee_begin_enrol(fpc_tee_bio_t* tee);
int fpc_tee_cancel_enrol(fpc_tee_bio_t* bio);
int fpc_tee_enrol(fpc_tee_bio_t* bio, uint32_t* remaining, fpc_algo_enroll_statistics_t* enrol_data);
int fpc_tee_end_enrol(fpc_tee_bio_t* tee, uint32_t* id);
int fpc_tee_identify(fpc_tee_bio_t* tee, uint32_t* id);
int fpc_tee_identify_and_update(fpc_tee_bio_t* bio, uint32_t* id, fpc_ta_bio_identify_statistics_t *identify_data, uint32_t* update);
int fpc_tee_qualify_image(fpc_tee_bio_t* tee);

int fpc_tee_update_template(fpc_tee_bio_t* tee, uint32_t* update);
int fpc_tee_get_finger_ids(fpc_tee_bio_t* tee, uint32_t* size, uint32_t* ids);
int fpc_tee_delete_template(fpc_tee_bio_t* tee, uint32_t id);
int fpc_tee_get_template_db_id(fpc_tee_bio_t* tee, uint64_t* id);

int fpc_tee_load_empty_db(fpc_tee_bio_t* tee);
int fpc_tee_store_template_db(fpc_tee_bio_t* tee, finger_store_template_config_t* template_config);
int fpc_tee_load_template_db(fpc_tee_bio_t* tee, const char* path);
int fpc_tee_load_backup_template_db(fpc_tee_bio_t* tee, const char* path, uint32_t* template_status);
int fpc_create_backup_db(char* path, const char* suffix);

int fpc_tee_enable_duplicate_finger_detect(fpc_tee_bio_t* bio, bool need_judge_duplicate_finger);

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_get_identify_statistics(fpc_tee_bio_t* tee, fpc_ta_bio_identify_statistics_t* stat);
#endif

#endif /* FPC_TEE_BIO_H */
