/************************************************************************************
 ** File: - fpc\fpc_tac\normal\src\fpc_tee_sensor.c
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2017, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Fingerprint TEE SENSOR PART for FPC (sw23.2.2 android O)
 **
 ** Version: 1.0
 ** Date created: 18:03:11,06/11/2017
 ** Author: Ziqing.guo@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>      <data>            <desc>
 **  Ziqing.guo   2017/11/06        create the file, add the api fpc_tee_identify_and_update
 **  Ziqing.guo   2017/11/07        add the api cancel enroll and duplicate finger
 ************************************************************************************/

#include <stdlib.h>

#include "fpc_ta_targets.h"
#include "fpc_ta_interface.h"
#include "fpc_ta_bio_interface.h"
#include "fpc_tee.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_bio_internal.h"

#include "fpc_tee_internal.h"
#include "fpc_log.h"
#include "fpc_types.h"
#include "string.h"
#include "fpc_error_str.h"

static int bio_command(fpc_tee_bio_t* bio, int32_t command_id)
{
    fpc_tee_t* tee = &bio->tee;

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->header.command = command_id;
    command->header.target = TARGET_FPC_TA_BIO;

    return fpc_tac_transfer(tee->tac, tee->shared_buffer);
}

int fpc_tee_set_gid(fpc_tee_bio_t* bio, uint32_t gid)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->bio.answer = gid;
    return bio_command(bio, FPC_TA_BIO_SET_ACTIVE_FINGERPRINT_SET_CMD);
}

int fpc_tee_begin_enrol(fpc_tee_bio_t* bio)
{
    LOGD("%s", __func__);
    return bio_command(bio, FPC_TA_BIO_BEGIN_ENROL_CMD);
}

int fpc_tee_cancel_enrol(fpc_tee_bio_t* bio) {
        LOGD("%s", __func__);
        return bio_command(bio, FPC_TA_BIO_CANCEL_ENROL_CMD);
}

int fpc_tee_enable_duplicate_finger_detect(fpc_tee_bio_t* bio, bool need_judge_duplicate_finger) {
        LOGD("%s", __func__);

        fpc_ta_enable_duplicate_fp_command_t* command = (fpc_ta_enable_duplicate_fp_command_t*)
                bio->tee.shared_buffer->addr;

        command->need_judge_duplicate_finger = need_judge_duplicate_finger;

        int status = bio_command(bio, FPC_TA_TEE_ENABLE_DUPLICATE_FP_DETECT_CMD);
        if (status) {
                return status;
        }

        return status;
}

int fpc_tee_enrol(fpc_tee_bio_t* bio, uint32_t* remaining, fpc_algo_enroll_statistics_t* enrol_data) {
        LOGD("%s", __func__);
        fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
        int status = bio_command(bio, FPC_TA_BIO_ENROL_CMD);
        if (status < 0) {
                return status;
        }

        *remaining = command->bio.answer;
        LOGD("%s: %s remains:%d", __func__, fpc_error_str(status), *remaining);
        //ziqing.guo add for get enrol data
        memset(enrol_data, 0, sizeof(fpc_algo_enroll_statistics_t));
        memcpy(enrol_data, &(command->enroll.enrol_data), sizeof(fpc_algo_enroll_statistics_t));

        return status;
}

int fpc_tee_end_enrol(fpc_tee_bio_t* bio, uint32_t* id)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    int status = bio_command(bio, FPC_TA_BIO_END_ENROL_CMD);
    if (status) {
        return status;
    }

    *id = command->bio.answer;
    return status;
}

int fpc_tee_identify(fpc_tee_bio_t* bio, uint32_t* id)
{
    LOGD("%s", __func__);

    int status = bio_command(bio, FPC_TA_BIO_IDENTIFY_CMD);
    if (status) {
        return status;
    }

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    *id = command->bio.answer;

    LOGD("%s: coverage: %d, quality: %d",
         __func__, command->identify.statistics.coverage, command->identify.statistics.quality);

        return status;
}

int fpc_tee_identify_and_update(fpc_tee_bio_t* bio, uint32_t* id, fpc_ta_bio_identify_statistics_t *identify_data, uint32_t* update) {
        LOGD("%s", __func__);

        //1. identify flow
        int status = bio_command(bio, FPC_TA_BIO_IDENTIFY_CMD);
        //if (status) {
        //        return status;
        //}

        fpc_ta_bio_command_t* command1 = _get_bio_cmd_struct(bio);
        *id = command1->bio.answer;

        LOGD("%s: coverage: %d, quality: %d",
                        __func__, command1->identify.statistics.coverage, command1->identify.statistics.quality);

        //ziqng add for get identify_data
        memset(identify_data, 0, sizeof(fpc_ta_bio_identify_statistics_t));
        memcpy(identify_data, &(command1->identify.statistics), sizeof(fpc_ta_bio_identify_statistics_t));

        //status = command1->bio.response;
        if (status < 0) {
                LOGE("%s identify failed with status = %d", __func__, status);
                return status;
        }

        //2. update template flow
        status = bio_command(bio, FPC_TA_BIO_UPDATE_TEMPLATE_CMD);
        //if (status) {
        //        return status;
        //}

        fpc_ta_bio_command_t* command2 = _get_bio_cmd_struct(bio);
        *update = command2->bio.answer;

        //status = command2->bio.response;
        if (status) {
                LOGE("%s update failed with status = %d", __func__, status);
        }

        return status;
}

#ifdef FPC_CONFIG_ENGINEERING
int fpc_tee_get_identify_statistics(fpc_tee_bio_t* bio, fpc_ta_bio_identify_statistics_t* stat) {
    LOGD("%s", __func__);
    int status = bio_command(bio, FPC_TA_BIO_GET_IDENTIFY_STATISTICS_CMD);
    if (status) {
        return status;
    }

    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);

    stat->coverage = command->identify.statistics.coverage;
    stat->quality = command->identify.statistics.quality;
    stat->covered_zones = command->identify.statistics.covered_zones;
    stat->score = command->identify.statistics.score;
    stat->result = command->identify.statistics.result;

    return status;
}
#endif

int fpc_tee_update_template(fpc_tee_bio_t* bio, uint32_t* update)
{
    LOGD("%s", __func__);
    int status = bio_command(bio, FPC_TA_BIO_UPDATE_TEMPLATE_CMD);

    if (status) {
        return status;
    }

    fpc_ta_bio_command_t *command = _get_bio_cmd_struct(bio);
    *update = command->bio.answer;

    return status;
}

int fpc_tee_get_finger_ids(fpc_tee_bio_t *bio, uint32_t *size, uint32_t *ids)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t *command = _get_bio_cmd_struct(bio);
    command->bio.answer = *size;

    if (*size > fpc_tee_get_max_number_of_templates(&bio->tee)) {
        return -FPC_ERROR_PARAMETER;
    }

    if (fpc_tee_get_max_number_of_templates(&bio->tee) > MAX_NR_TEMPLATES_API) {
        return -FPC_ERROR_PARAMETER;
    }

    int status = bio_command(bio, FPC_TA_BIO_GET_FINGER_IDS_CMD);
    if (status) {
        return status;
    }

    if (*size < command->bio.answer) {
        LOGE("%s Not enough room for templates %u of %u", __func__, *size, command->bio.answer);
        return -FPC_ERROR_PARAMETER;
    }

    *size = command->bio.answer;
    memcpy(ids, command->get_ids.ids, *size * sizeof(command->get_ids.ids[0]));

    return status;
}

int fpc_tee_delete_template(fpc_tee_bio_t* bio, uint32_t id)
{
    LOGD("%s", __func__);
    fpc_ta_bio_command_t* command = _get_bio_cmd_struct(bio);
    command->bio.answer = id;
    return bio_command(bio, FPC_TA_BIO_DELETE_TEMPLATE_CMD);
}

int fpc_tee_get_template_db_id(fpc_tee_bio_t* bio, uint64_t* id)
{
    LOGD("%s", __func__);

    int status = bio_command(bio, FPC_TA_BIO_GET_TEMPLATE_DB_ID_CMD);
    if (status) {
        return status;
    }
    fpc_ta_bio_get_db_command_t *command = (fpc_ta_bio_get_db_command_t *)
                                           bio->tee.shared_buffer->addr;
    *id = command->id;
    return status;
}

int fpc_tee_load_empty_db(fpc_tee_bio_t *bio)
{
    LOGD("%s", __func__);
    return bio_command(bio, FPC_TA_BIO_LOAD_EMPTY_DB_CMD);
}


fpc_tee_bio_t *fpc_tee_bio_init(fpc_tee_t *tee)
{
    LOGD("%s", __func__);
    fpc_tee_bio_t *bio = (fpc_tee_bio_t *) tee;
    int result = bio_command(bio, FPC_TA_BIO_INIT_CMD);
    if (FAILED(result)) {
        return NULL;
    }
    return bio;
}

void fpc_tee_bio_release(fpc_tee_bio_t *bio)
{
    (void) bio; // unused
    return;
}
