/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include "fpc_types.h"
#include "fpc_log.h"
#include "fpc_tee.h"
#include "fpc_tee_bio.h"

fpc_tee_bio_t* fpc_tee_bio_init(fpc_tee_t* tee)
{
    return (fpc_tee_bio_t*)tee;
}
void fpc_tee_bio_release(fpc_tee_bio_t* tee)
{
    (void)tee;
    LOGE("%s, dummy implementation!", __func__);
}

int fpc_tee_set_gid(fpc_tee_bio_t* tee, int32_t gid)
{
    (void)tee; (void)gid;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_begin_enrol(fpc_tee_bio_t* tee)
{
    (void)tee;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_enrol(fpc_tee_bio_t* tee, uint32_t* remaining)
{
    (void)tee; (void) remaining;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_end_enrol(fpc_tee_bio_t* tee, uint32_t* id)
{
    (void)tee; (void)id;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_identify(fpc_tee_bio_t* tee, uint32_t* id, uint32_t* update)
{
    (void)tee; (void)id; (void) update;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}

int fpc_tee_update_template(fpc_tee_bio_t* tee)
{
    (void)tee;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_get_finger_ids(fpc_tee_bio_t* tee, uint32_t* size, uint32_t* ids)
{
    (void)tee; (void)size; (void)ids;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_delete_template(fpc_tee_bio_t* tee, uint32_t id)
{
    (void)tee; (void)id;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_get_template_db_id(fpc_tee_bio_t* tee, uint64_t* id)
{
    (void)tee; (void)id;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}

int fpc_tee_load_empty_db(fpc_tee_bio_t* tee)
{
    (void)tee;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_store_template_db(fpc_tee_bio_t* tee, const char* path)
{
    (void)tee; (void)path;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}
int fpc_tee_load_template_db(fpc_tee_bio_t* tee, const char* path)
{
    (void)tee; (void)path;
    LOGE("%s, dummy implementation!", __func__);
    return 0;
}

