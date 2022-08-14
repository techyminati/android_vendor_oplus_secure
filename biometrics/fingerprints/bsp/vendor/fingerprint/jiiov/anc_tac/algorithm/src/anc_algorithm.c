#define LOG_TAG "[ANC_TAC][Algo]"

#include "anc_algorithm.h"

#include <string.h>


#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "algorithm_command.h"
#include "anc_algorithm_image.h"
#include "anc_memory_wrapper.h"

#include "anc_tac_dcs.h"
#include "anc_tac_dcs_file.h"
#include "anc_data_collect.h"
#include "anc_tac_time.h"
#include "anc_log_string.h"

#include <cutils/properties.h>
#ifdef ANC_SAVE_ALGO_FILE
#include "anc_background_worker.h"
#endif
// #define ANC_SAVE_ALGO_FILE

#ifdef ANC_SAVE_ALGO_FILE
extern AncBGWorker g_background_worker;
#endif

#ifdef ANC_SAVE_ALGO_FILE
typedef struct {
    uint32_t cmd_type;
    uint32_t img_buf_size;
    uint32_t dcs_buf_size;
    uint8_t  data;
} ImageInfo;
#endif

uint8_t g_algo_version_string[100] = "";
#ifndef ANC_SAVE_AUTH_DEFAULT
#define ANC_SAVE_AUTH_DEFAULT 1
#endif

#ifndef ANC_NEED_ENCRYTER_DEFAULT
#define ANC_NEED_ENCRYTER_DEFAULT 0
#endif
static uint8_t g_app_save_auth_switch = ANC_SAVE_AUTH_DEFAULT;
static uint8_t g_need_encryted_save_file = ANC_NEED_ENCRYTER_DEFAULT;

static ANC_RETURN_TYPE AlgoTransmitCommon(AncAlgorithmCommand *p_algorithm, AncAlgorithmCommandRespond *p_algorithm_respond,
                                        ANC_BOOL is_used_share_buffer) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncSendCommand anc_send_command;
    AncSendCommandRespond anc_send_respond;

    AncMemset(&anc_send_respond, 0, sizeof(anc_send_respond));
    AncMemset(&anc_send_command, 0, sizeof(anc_send_command));
    anc_send_command.size = sizeof(AncSendCommand);
    anc_send_command.id = ANC_CMD_ALGORITHM;
    AncMemcpy(&(anc_send_command.data.algo), p_algorithm, sizeof(AncAlgorithmCommand));

    ANC_LOGD("TRANSMIT >>>> command id = %d (%s)", p_algorithm->command, AncConvertCommandIdToString(ANC_CMD_ALGORITHM, p_algorithm->command));
    long long time_start = AncGetElapsedRealTimeMs();

    if (is_used_share_buffer) {
        ret_val = AncCaTransmitModified(&anc_send_command, &anc_send_respond);
    } else {
        ret_val = AncCaTransmit(&anc_send_command, &anc_send_respond);
    }
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send algo command, ret value:%d, command id:%d",
                  ret_val, anc_send_command.data.algo.command);
    } else {
        ret_val = (ANC_RETURN_TYPE)anc_send_respond.status;
        AncMemcpy(p_algorithm_respond, &(anc_send_respond.respond.algo), sizeof(AncAlgorithmCommandRespond));
    }

    long long time_end = AncGetElapsedRealTimeMs();
    ANC_LOGD("TRANSMIT <<<< command id = %d (%s), spent time = %lld ms, ret_val = %d (%s)", p_algorithm->command,
        AncConvertCommandIdToString(ANC_CMD_ALGORITHM, p_algorithm->command), (time_end - time_start), ret_val, AncConvertReturnTypeToString(ret_val));

    return ret_val;
}

static ANC_RETURN_TYPE AlgoTransmitNoLockSharedBuffer(AncAlgorithmCommand *p_algorithm, AncAlgorithmCommandRespond *p_algorithm_respond) {
    return AlgoTransmitCommon(p_algorithm, p_algorithm_respond, ANC_TRUE);
}

static ANC_RETURN_TYPE AlgoTransmit(AncAlgorithmCommand *p_algorithm, AncAlgorithmCommandRespond *p_algorithm_respond) {
    return AlgoTransmitCommon(p_algorithm, p_algorithm_respond, ANC_FALSE);
}

#ifdef ANC_SAVE_ALGO_FILE
static ANC_RETURN_TYPE PrintInitData(const oplus_fingerprint_init_ta_info_t *p_info) {
#ifdef ANC_DEBUG
#define P_INT(_a) ANC_LOGD("%s:%d", #_a, p_info->_a)
#define P_STRING(_a) ANC_LOGD("%s:%s", #_a, p_info->_a)
    ANC_LOGD("********** print big data init data **********");
    P_INT(sensor_id);
    P_INT(lens_type);
    P_STRING(chip_type);
    P_STRING(factory_type);

    P_STRING(algo_version);
    P_INT(init_finger_number);
    P_INT(all_template_num);
    P_INT(template_verison);

    P_INT(badpixel_num);
    P_INT(badpixel_num_local);
    P_INT(exposure_value);
    P_INT(calabration_signal_value);
    P_INT(flesh_touch_diff);
    P_INT(scale);
    /*
    P_INT(calabration_rawdata);
    P_INT(base_sum);
    P_INT(white_mean);
    P_INT(black_mean);
    P_INT(x);
    P_INT(y);
    P_INT(freqpeak);
    P_INT(shading_highest);
    P_INT(shading_lowest);
    P_INT(fov_size);
    P_INT(offset_x);
    P_INT(offset_y);
    P_INT(s_signal_max);
    P_INT(s_signal_min);
    P_INT(s_nosie_max);
    P_INT(s_nosie_min);
    P_INT(t_signal_max);
    P_INT(t_signal_min);
    P_INT(t_nosie_max);
    P_INT(t_nosie_min);
    P_INT(line_pair_count);
    P_INT(rotation_degree);
    P_INT(frequency_difference);

    P_INT(defect_info[0].total);
    P_INT(defect_info[0].lines);
    P_INT(defect_info[0].clusters);
    P_INT(defect_info[0].max_cluster_size);

    P_INT(defect_info[1].total);
    P_INT(defect_info[1].lines);
    P_INT(defect_info[1].clusters);
    P_INT(defect_info[1].max_cluster_size);
    */

#undef P_INT
#undef P_STRING
#else
    ANC_UNUSED(p_info);
#endif 
    return ANC_OK;
}

static void SaveImageTask(void *p_buffer) {
    ImageInfo *p_img_info = (ImageInfo *) p_buffer;
    uint8_t image_name_buf[ANC_ALGO_SAVE_INFO_PATH_LEN] = "";
    if (p_img_info->img_buf_size > 0) {
        AlgoImageParse(&(p_img_info->data), p_img_info->cmd_type, image_name_buf);
    }
    if (p_img_info->dcs_buf_size > 0) {
        AlgoInfoParse(&(p_img_info->data) + p_img_info->img_buf_size, p_img_info->cmd_type, image_name_buf);
    }
    return;
}

static uint8_t* MallocImageBuffer(uint32_t cmd_type, uint8_t *img_buf, uint32_t img_size, void *dcs_buf, uint32_t dcs_size) {
    uint8_t *buf_ptr = AncMalloc(3 * sizeof(uint32_t) + img_size + dcs_size);
    ImageInfo *p_img_info = (ImageInfo *) buf_ptr;
    p_img_info->cmd_type = cmd_type;
    p_img_info->img_buf_size = img_size;
    p_img_info->dcs_buf_size = dcs_size;
    if ((img_buf != NULL) && (img_size > 0)) {
        AncMemcpy(&(p_img_info->data), img_buf, img_size);
    }
    if ((dcs_buf != NULL) && (dcs_size > 0)) {
        AncMemcpy(&(p_img_info->data) + img_size, dcs_buf, dcs_size);
    }
    return buf_ptr;
}

static void FreeImageBuffer(void *buf) {
    AncFree(buf);
}

static ANC_RETURN_TYPE AlgoSaveImageInfo(uint8_t *p_share_buffer, uint32_t share_buffer_size, ANC_COMMAND_EXTENSION_TYPE cmd_type){
    uint8_t *task_buf = NULL;
    uint8_t *tmp_share_buf = p_share_buffer;

    if (cmd_type == ANC_CMD_EXTENSION_INIT_DATA_COLLOCT) {
        oplus_fingerprint_init_ta_info_t algo_info;
        ExtensionDCSInitDataCollect(&algo_info);
        PrintInitData(&algo_info);
        task_buf = MallocImageBuffer(cmd_type, tmp_share_buf, share_buffer_size, &algo_info, sizeof(oplus_fingerprint_init_ta_info_t));
    }
    if (cmd_type == ANC_CMD_EXTENSION_AUTH_RESULT_COLLOCT) {
        oplus_fingerprint_auth_ta_info_t algo_info;
        ExtensionDCSAuthResultCollect(&algo_info);
        task_buf = MallocImageBuffer(cmd_type, tmp_share_buf, share_buffer_size, &algo_info, sizeof(oplus_fingerprint_auth_ta_info_t));

    }
    if (cmd_type == ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT) {
        oplus_fingerprint_singleenroll_ta_info_t algo_info;
        ExtensionDCSSingleEnrollCollect(&algo_info);
        task_buf = MallocImageBuffer(cmd_type, tmp_share_buf, share_buffer_size, &algo_info, sizeof(oplus_fingerprint_singleenroll_ta_info_t));
    }
    if (cmd_type == ANC_CMD_EXTENSION_ENROLL_END_COLLOCT) {
        oplus_fingerprint_enroll_ta_info_t algo_info;
        ExtensionDCSEnrollEndCollect(&algo_info);
        task_buf = MallocImageBuffer(cmd_type, tmp_share_buf, share_buffer_size, &algo_info, sizeof(oplus_fingerprint_enroll_ta_info_t));
    }
    if (cmd_type == ANC_CMD_EXTENSION_HEART_BEAT_COLLOCT) {
        task_buf = MallocImageBuffer(cmd_type, tmp_share_buf, share_buffer_size, NULL, 0);
    }

    AncBGTask task = {task_buf, SaveImageTask, FreeImageBuffer};
    g_background_worker.PushTask(task);
    return ANC_OK;
}
#endif

static ANC_RETURN_TYPE isRegionSupported()
{
    ANC_RETURN_TYPE ret_val = ANC_FAIL;
    char area[PROPERTY_VALUE_MAX] = {'\0'};
    char *area_prop = "persist.sys.oplus.region";
    property_get(area_prop, area, "0");
    if (strcmp(area, "CN") == 0) {
        ret_val = ANC_OK;
    }
    return ret_val;
}

void AppSetSaveFileConfig(uint8_t on_off) {
    if(g_need_encryted_save_file == 1) {
        if (isRegionSupported() == ANC_FAIL) {
            g_app_save_auth_switch = 0;
            return;
        }
        g_app_save_auth_switch = on_off;
    }
}

#define SHARED_BUFFER_LENGTH (1 * 1024 * 1024)
static ANC_RETURN_TYPE ExtractFeature(uint32_t type, uint32_t retry_count, int32_t abnormal_exp, uint32_t *p_exp_ratio) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_EXTRACT_FEATURE;
    anc_algorithm_command.retry_count = retry_count;
    anc_algorithm_command.data = abnormal_exp;
    anc_algorithm_command.extract_type = type;
    // only do work, don't transmit file
    anc_algorithm_command.save_image = ANC_ALGORITHM_UNSAVE;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (p_exp_ratio != NULL) {
        *p_exp_ratio = anc_algorithm_respond.data;
    }

    ANC_LOGW("extract feature, return value:%d", ret_val);

    // transmit file
#ifdef ANC_SAVE_ALGO_FILE

    //app传入hal是否存储解锁加密数据，1为存图，0为不存
    if((g_app_save_auth_switch == 0)&&(type == 1)) { //type=1为解锁时候抽feature
        return ret_val;
    }

    if(g_need_encryted_save_file == 1) {
        if(isRegionSupported() == ANC_FAIL) {
            return ret_val;
        }
    }

    if (ANC_ALGO_EXTRACT_OK == ret_val) {
        return ret_val;
    } else {
        ANC_LOGE("fail to extract feature, the image will be saved");
    }

    ANC_RETURN_TYPE save_ret_val = ANC_OK;

    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.save_image = ANC_ALGORITHM_SAVE;

    uint8_t *p_tmp_shared_buffer = AncMalloc(SHARED_BUFFER_LENGTH);
    if (NULL == p_tmp_shared_buffer) {
        ANC_LOGE("fail to malloc shared buffer for algo file");
        return ANC_FAIL;
    }
    AncMemset(p_tmp_shared_buffer, 0, SHARED_BUFFER_LENGTH);


    ANC_CA_LOCK_SHARED_BUFFER();


    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;

    save_ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != save_ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        AncFree(p_tmp_shared_buffer);
        p_tmp_shared_buffer = NULL;
        return save_ret_val;
    }

    p_share_buffer[0] = g_need_encryted_save_file; // the flag wheather need encryted data
    ANC_LOGE("enroll need_encrypted_flag= %d",p_share_buffer[0]);

    save_ret_val = AlgoTransmitNoLockSharedBuffer(&anc_algorithm_command, &anc_algorithm_respond);

    uint32_t copy_share_buffer_size = (share_buffer_size <= SHARED_BUFFER_LENGTH) ? share_buffer_size : SHARED_BUFFER_LENGTH;
    AncMemcpy(p_tmp_shared_buffer, p_share_buffer, copy_share_buffer_size);

    ANC_CA_UNLOCK_SHARED_BUFFER();


    AlgoSaveImageInfo(p_tmp_shared_buffer, copy_share_buffer_size, type == 0 ? ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT : ANC_CMD_EXTENSION_AUTH_RESULT_COLLOCT);

    AncFree(p_tmp_shared_buffer);
    p_tmp_shared_buffer = NULL;

    ANC_LOGW("extract feature, save algo file return value:%d", save_ret_val);
#endif

    return ret_val;
}

ANC_RETURN_TYPE AlgorithmInit() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_INIT;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
#ifdef ANC_SAVE_ALGO_FILE
    AlgoSaveImageInfo(NULL, 0, ANC_CMD_EXTENSION_INIT_DATA_COLLOCT);
#endif
    return ret_val;
}

ANC_RETURN_TYPE GetAlgorithmVersion(uint8_t *p_version, uint32_t version_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    if ((NULL == p_version) || (0 == version_length)) {
        ANC_LOGE("parameters are error, version:%p, version length:%d",
                  p_version, version_length);
        return ANC_FAIL;
    }

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_GET_VERSION;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK == ret_val) {
        if (version_length > anc_algorithm_respond.version_size) {
            AncMemset(p_version, 0, version_length);
            AncMemcpy(p_version, anc_algorithm_respond.version,
                     anc_algorithm_respond.version_size);
            AncMemset(g_algo_version_string, 0, sizeof(g_algo_version_string));
            AncMemcpy(g_algo_version_string, anc_algorithm_respond.version,
                     anc_algorithm_respond.version_size);
        }
    } else {
        ANC_LOGE("fail to get algorithm version, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE AlgorithmDeinit() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_DEINIT;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);

    return ret_val;
}

ANC_RETURN_TYPE AlgoInitEnroll() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_INIT_ENROLL;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);

    return ret_val;
}

ANC_RETURN_TYPE AlgoEnroll(uint32_t *p_remaining, uint32_t *p_fingerid) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ret_val = AlgoEnrollExtractFeature();
    if (ret_val != ANC_ALGO_EXTRACT_OK) {
        ANC_LOGE("extract feature failed during enroll. ret:%d", ret_val);
        return ANC_FAIL;
    }
    return AlgoEnrollFeature(p_remaining, p_fingerid);
}

ANC_RETURN_TYPE AlgoEnrollFeature(uint32_t *p_remaining, uint32_t *p_fingerid) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_ENROLL_FEATURE;
    // only do work, don't transmit file
    anc_algorithm_command.save_image = ANC_ALGORITHM_UNSAVE;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    *p_remaining = anc_algorithm_respond.data;
    *p_fingerid = anc_algorithm_respond.finger_id;

    ANC_LOGW("algo enroll feature, return value:%d", ret_val);

    // transmit file
#ifdef ANC_SAVE_ALGO_FILE
    if(g_need_encryted_save_file == 1) {
        if(isRegionSupported() == ANC_FAIL) {
            return ret_val;
        }
    }

    ANC_RETURN_TYPE save_ret_val = ANC_OK;

    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.save_image = ANC_ALGORITHM_SAVE;

    uint8_t *p_tmp_shared_buffer = AncMalloc(SHARED_BUFFER_LENGTH);
    if (NULL == p_tmp_shared_buffer) {
        ANC_LOGE("fail to malloc shared buffer for algo file");
        return ANC_FAIL;
    }
    AncMemset(p_tmp_shared_buffer, 0, SHARED_BUFFER_LENGTH);

    ANC_CA_LOCK_SHARED_BUFFER();

    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;
    save_ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != save_ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        AncFree(p_tmp_shared_buffer);
        p_tmp_shared_buffer = NULL;
        return save_ret_val;
    }

    p_share_buffer[0] = g_need_encryted_save_file; // the flag wheather need encryted data
    ANC_LOGE("enroll need_encrypted_flag= %d",p_share_buffer[0]);

    save_ret_val = AlgoTransmitNoLockSharedBuffer(&anc_algorithm_command, &anc_algorithm_respond);

    uint32_t copy_share_buffer_size = (share_buffer_size <= SHARED_BUFFER_LENGTH) ? share_buffer_size : SHARED_BUFFER_LENGTH;
    AncMemcpy(p_tmp_shared_buffer, p_share_buffer, copy_share_buffer_size);

    ANC_CA_UNLOCK_SHARED_BUFFER();

    AlgoSaveImageInfo(p_tmp_shared_buffer, copy_share_buffer_size, ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT);
    AncFree(p_tmp_shared_buffer);
    p_tmp_shared_buffer = NULL;

    ANC_LOGW("algo enroll feature, save algo file return value:%d", save_ret_val);
#endif
    return ret_val;
}

ANC_RETURN_TYPE AlgoDeinitEnroll(uint32_t *p_finger_id, ANC_BOOL is_finished) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_DEINIT_ENROLL;
    anc_algorithm_command.data = (is_finished == ANC_TRUE) ? 1 : 0;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    *p_finger_id = anc_algorithm_respond.finger_id;
#ifdef ANC_SAVE_ALGO_FILE
    AlgoImageRenameEnrollFolder(*p_finger_id,is_finished);
    AlgoSaveImageInfo(NULL, 0, ANC_CMD_EXTENSION_ENROLL_END_COLLOCT);
#else
    ANC_UNUSED(is_finished);
#endif
    return ret_val;
}

ANC_RETURN_TYPE AlgoEnrollExtractFeature() {
    return ExtractFeature(0, 0, 0, NULL);
}

ANC_RETURN_TYPE AlgoInitVerify() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_INIT_VERIFY;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);

    return ret_val;
}

ANC_RETURN_TYPE AlgoVerifyExtractFeature(uint32_t retry_count, int32_t abnormal_exp, uint32_t *p_exp_ratio) {
    return ExtractFeature(1, retry_count, abnormal_exp, p_exp_ratio);
}

ANC_RETURN_TYPE AlgoVerify(uint32_t *p_finger_id, uint32_t *p_need_study, uint32_t *p_algo_status, uint32_t retry_count) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ret_val = AlgoVerifyExtractFeature(retry_count, 1, 0);
    if (ret_val != ANC_ALGO_EXTRACT_OK) {
        ANC_LOGE("extract feature failed during verify. ret:%d", ret_val);
        return ANC_FAIL;
    }
    return AlgoCompareFeature(p_finger_id, p_need_study, p_algo_status);
}

ANC_RETURN_TYPE AlgoCompareFeature(uint32_t *p_finger_id, uint32_t *p_need_study, uint32_t *p_algo_status) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_COMPARE_FEATURE;
    // only do work, don't transmit file
    anc_algorithm_command.save_image = ANC_ALGORITHM_UNSAVE;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    *p_finger_id = anc_algorithm_respond.finger_id;
    *p_need_study = anc_algorithm_respond.need_study;
    *p_algo_status = anc_algorithm_respond.algo_status;

    ANC_LOGW("algo compare feature, return value:%d", ret_val);

    // transmit file
#ifdef ANC_SAVE_ALGO_FILE

    //app传入hal是否存储解锁加密数据，1为存图，0为不存
    if(g_app_save_auth_switch == 0) {
        return ret_val;
    }

    ANC_RETURN_TYPE save_ret_val = ANC_OK;

    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.save_image = ANC_ALGORITHM_SAVE;

    uint8_t *p_tmp_shared_buffer = AncMalloc(SHARED_BUFFER_LENGTH);
    if (NULL == p_tmp_shared_buffer) {
        ANC_LOGE("fail to malloc shared buffer for algo file");
        return ANC_FAIL;
    }
    AncMemset(p_tmp_shared_buffer, 0, SHARED_BUFFER_LENGTH);

    ANC_CA_LOCK_SHARED_BUFFER();

    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;

    save_ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != save_ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        AncFree(p_tmp_shared_buffer);
        p_tmp_shared_buffer = NULL;
        return save_ret_val;
    }

    p_share_buffer[0] = g_need_encryted_save_file; // the flag wheather need encryted data
    ANC_LOGE("auth need_encrypted_flag= %d",p_share_buffer[0]);

    save_ret_val = AlgoTransmitNoLockSharedBuffer(&anc_algorithm_command, &anc_algorithm_respond);

    uint32_t copy_share_buffer_size = (share_buffer_size <= SHARED_BUFFER_LENGTH) ? share_buffer_size : SHARED_BUFFER_LENGTH;
    AncMemcpy(p_tmp_shared_buffer, p_share_buffer, copy_share_buffer_size);

    ANC_CA_UNLOCK_SHARED_BUFFER();

    AlgoSaveImageInfo(p_tmp_shared_buffer, copy_share_buffer_size, ANC_CMD_EXTENSION_AUTH_RESULT_COLLOCT);
    AncFree(p_tmp_shared_buffer);
    p_tmp_shared_buffer = NULL;

    ANC_LOGW("algo compare feature, save algo file return value:%d", save_ret_val);
#endif
    return ret_val;
}

ANC_RETURN_TYPE AlgoFeatureStudy(uint32_t finger_id) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_FEATURE_STUDY;
    anc_algorithm_command.finger_id = finger_id;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    return ret_val;
}

ANC_RETURN_TYPE AlgoDeinitVerify() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_DEINIT_VERIFY;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);

    return ret_val;
}

ANC_RETURN_TYPE TemplateSetActiveGroup(uint32_t gid, const char *p_store_path) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;
    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;

    ANC_CA_LOCK_SHARED_BUFFER();

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_TEMPLATE_SET_ACTIVE_GROUP;
    anc_algorithm_command.group_id = gid;

    ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get shared buffer");
        goto DO_FAIL;
    }

    size_t store_path_len = strlen(p_store_path) + 1;
    if(share_buffer_size < store_path_len) {
        ANC_LOGE("shared buffer size is %d insufficient, store path len %zu", share_buffer_size, store_path_len);
        ret_val = ANC_FAIL_MEMORY;
        goto DO_FAIL;
    }

    AncMemcpy(p_share_buffer, p_store_path, store_path_len);
    ret_val = AlgoTransmitNoLockSharedBuffer(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to set active group, ret value:%d", ret_val);
    }

DO_FAIL:
    ANC_CA_UNLOCK_SHARED_BUFFER();

    return ret_val;
}

ANC_RETURN_TYPE GetAuthenticatorId(uint64_t *p_authenticator_id) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    if (NULL == p_authenticator_id) {
        ANC_LOGE("authenticator id is NULL");
        return ANC_FAIL;
    }

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_TEMPLATE_GET_AUTHENTICATOR_ID;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get auth id, ret value:%d", ret_val);
    }
    *p_authenticator_id = anc_algorithm_respond.authenticator_id;
    return ret_val;
}

ANC_RETURN_TYPE TemplateLoadDatabase() {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_TEMPLATE_LOAD_DATABASE;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to load template database, ret value:%d", ret_val);
    }

    return ret_val;
}

ANC_RETURN_TYPE DeleteFingerprint(uint32_t finger_id) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_TEMPLATE_DELETE_FINGERPRINT;
    anc_algorithm_command.finger_id = finger_id;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);

    return ret_val;
}

ANC_RETURN_TYPE GetAllFingerprintsId(uint32_t *p_id_array, uint32_t id_array_size, uint32_t *p_id_count) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;
    uint32_t valid_id_count = 0;

    if ((NULL == p_id_array) || (id_array_size == 0) || (NULL == p_id_count)) {
        ANC_LOGE("parameters are error, id array:%p, id array size:%d, id count:%p", p_id_array, id_array_size, p_id_count);
        return ANC_FAIL;
    }

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_TEMPLATE_GET_ALL_FINGERPRINTS_IDS;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (id_array_size <= anc_algorithm_respond.id_count) {
        valid_id_count = id_array_size;
    } else {
        valid_id_count = anc_algorithm_respond.id_count;
    }

    for (uint32_t i=0; i<valid_id_count; i++) {
        p_id_array[i] = anc_algorithm_respond.id[i];
    }
    *p_id_count = valid_id_count;

    return ret_val;
}

uint32_t GetAllFingerprintsCount(void) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;
    uint32_t valid_id_count = 0;

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_TEMPLATE_GET_ALL_FINGERPRINTS_IDS;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if(ANC_OK == ret_val) {
        valid_id_count = anc_algorithm_respond.id_count;
    }

    return valid_id_count;
}

ANC_RETURN_TYPE AlgoGetEnrollTotalTimes(int32_t *p_enroll_total_times) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    if (NULL == p_enroll_total_times) {
        ANC_LOGE("enroll total times is NULL");
        return ANC_FAIL;
    }

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_GET_ENROLL_TOTAL_TIMES;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get enroll total times, ret value:%d", ret_val);
    } else {
        *p_enroll_total_times = anc_algorithm_respond.enroll_total_times;
        ANC_LOGD("get enroll total times: %d", *p_enroll_total_times);
    }

    return ret_val;
}

ANC_RETURN_TYPE AlgoGetImageQualityScore(uint32_t *P_image_quality_score) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    if (NULL == P_image_quality_score) {
        ANC_LOGE("image quality score is NULL");
        return ANC_FAIL;
    }

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_GET_IMAGE_QUALITY_SCORE;

    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to get image quality score, ret value:%d", ret_val);
    } else {
        *P_image_quality_score = anc_algorithm_respond.image_quality_score;
        ANC_LOGD("get image quality score: %d", *P_image_quality_score);
    }

    return ret_val;
}

ANC_RETURN_TYPE AlgoGetHBResult(uint32_t retry_count, uint32_t *p_feat, uint32_t *p_bpm) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncAlgorithmCommand anc_algorithm_command;
    AncAlgorithmCommandRespond anc_algorithm_respond;

    if ((NULL == p_feat) || (NULL == p_bpm)) {
        ANC_LOGE("HB result param is NULL");
        return ANC_FAIL;
    }

    AncMemset(&anc_algorithm_command, 0, sizeof(anc_algorithm_command));
    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.command = ANC_CMD_ALGORITHM_GET_HEART_BEAT_RESULT;
    anc_algorithm_command.retry_count = retry_count;

    *p_feat = 0;
    *p_bpm = 0;

    anc_algorithm_command.save_image = ANC_ALGORITHM_UNSAVE;
    ret_val = AlgoTransmit(&anc_algorithm_command, &anc_algorithm_respond);
    if (ANC_OK == ret_val) {
        *p_feat = anc_algorithm_respond.image_quality_score;
        *p_bpm = anc_algorithm_respond.data;
    }

// transmit file
#ifdef ANC_SAVE_ALGO_FILE
    if(g_need_encryted_save_file == 1) {
        if(isRegionSupported() == ANC_FAIL) {
            return ret_val;
        }
    }

    ANC_RETURN_TYPE save_ret_val = ANC_OK;

    AncMemset(&anc_algorithm_respond, 0, sizeof(anc_algorithm_respond));
    anc_algorithm_command.save_image = ANC_ALGORITHM_SAVE;

    uint8_t *p_tmp_shared_buffer = AncMalloc(SHARED_BUFFER_LENGTH);
    if (NULL == p_tmp_shared_buffer) {
        ANC_LOGE("fail to malloc shared buffer for algo file");
        return ANC_FAIL;
    }
    AncMemset(p_tmp_shared_buffer, 0, SHARED_BUFFER_LENGTH);

    ANC_CA_LOCK_SHARED_BUFFER();

    uint8_t *p_share_buffer = NULL;
    uint32_t share_buffer_size = 0;

    save_ret_val = AncGetIonSharedBuffer(&p_share_buffer, &share_buffer_size);
    if (ANC_OK != save_ret_val) {
        ANC_LOGE("fail to get shared buffer");
        ANC_CA_UNLOCK_SHARED_BUFFER();
        AncFree(p_tmp_shared_buffer);
        p_tmp_shared_buffer = NULL;
        return save_ret_val;
    }

    p_share_buffer[0] = g_need_encryted_save_file; // the flag wheather need encryted data
    ANC_LOGE("auth need_encrypted_flag= %d",p_share_buffer[0]);

    save_ret_val = AlgoTransmitNoLockSharedBuffer(&anc_algorithm_command, &anc_algorithm_respond);

    uint32_t copy_share_buffer_size = (share_buffer_size <= SHARED_BUFFER_LENGTH) ? share_buffer_size : SHARED_BUFFER_LENGTH;
    AncMemcpy(p_tmp_shared_buffer, p_share_buffer, copy_share_buffer_size);

    ANC_CA_UNLOCK_SHARED_BUFFER();

    AlgoSaveImageInfo(p_tmp_shared_buffer, copy_share_buffer_size, ANC_CMD_EXTENSION_HEART_BEAT_COLLOCT);
    AncFree(p_tmp_shared_buffer);
    p_tmp_shared_buffer = NULL;

    ANC_LOGW("algo get hb result, save algo file return value:%d", save_ret_val);
#endif

    return ret_val;
}

