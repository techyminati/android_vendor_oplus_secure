#define LOG_TAG "[ANC_TAC][DCS]"

#include "anc_tac_dcs_file.h"

#include <string.h>
#include <stdio.h>

#include "anc_data_collect.h"
#include "anc_file.h"
#include "string.h"
#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "anc_lib.h"
#include "anc_utils.h"
#include "anc_memory_wrapper.h"
#include "anc_tac_time.h"
#include "anc_type.h"
#include "sensor_command_param.h"
#include "dcs_type.h"


#define ANC_ALGO_SAVE_INFO_BUF_LEN 2048
#define ANC_ALGO_SAVE_IMAGE_MAX_NUM 10

#define INFO_ROOT ANC_DATA_ROOT "info"

const static char *gp_algo_info_root = INFO_ROOT;

const static char *gp_algo_base_info_name = "_fp_info.csv";

static char *gp_info_file_prefix[4] = {
    "init",
    "enroll",
    "enroll_finish",
    "verify"
};

#define INFO(a) p_info->a

static ANC_RETURN_TYPE AlgoInfoParseInitData(oplus_fingerprint_init_ta_info_t *p_info, char* p_info_buf, const char* p_info_dir , const char* p_info_name) {
    ANC_RETURN_TYPE ret = ANC_OK;
    const char *p_full_path = AsFullPath(p_info_dir, p_info_name);
    ret = AncTestDir(p_full_path);
    if (ANC_OK != ret) {
        ANC_LOGD("%s does not exists create one", p_full_path);
        AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN,
        "\
        sensor_id,\
        lens_type,\
        chip_id,\
        factory_type,\
        algo_version,\
        template_version,\
        init_finger_number,\
        all_template_num,\
        defect_total_num,\
        defect_cluster_num,\
        base_sum,\
        s_signal_min,\
        freqpeak,\
        line_pair_count,\
        \n");
        ret = AncAppendWriteFile(p_full_path, p_info_buf);
        AncMemset(p_info_buf, 0, sizeof(*p_info_buf) * ANC_ALGO_SAVE_INFO_BUF_LEN);
    }

/*
    if (sizeof(ANC_SENSOR_EFUSE_CHIP_PROTO_INFO) > sizeof(p_info->chip_type)) {
        ANC_LOGE("chipid buffer length not enough");
        return ANC_FAIL;
    }
*/

    ANC_SENSOR_EFUSE_CHIP_PROTO_INFO *p_chip_proto_info = (ANC_SENSOR_EFUSE_CHIP_PROTO_INFO*)p_info->chip_type;
    char chip_info[64] = {0};
    AncSnprintf(chip_info, sizeof(chip_info), "%c%c%02d%02d_%02d_%d_%d", p_chip_proto_info->chip_lot_id[0],p_chip_proto_info->chip_lot_id[1],
                 p_chip_proto_info->chip_lot_id[2], p_chip_proto_info->chip_lot_id[3], p_chip_proto_info->wafer_id,
               p_chip_proto_info->die_x, p_chip_proto_info->die_y);

    AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN, "%u,%u,%s,%s,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
        INFO(sensor_id),
        INFO(lens_type),
        chip_info,
        INFO(factory_type),
        INFO(algo_version),
        INFO(template_verison),
        INFO(init_finger_number),
        INFO(all_template_num),
        INFO(badpixel_num),
        INFO(badpixel_num_local),
        INFO(exposure_value),
        INFO(calabration_signal_value),
        INFO(flesh_touch_diff),
        INFO(scale)
    );
    ANC_LOGD("AlgoInfoParseInitData: %s", p_info_buf);
    ret = AncAppendWriteFile(p_full_path, p_info_buf);
    return ANC_OK;
}

static ANC_RETURN_TYPE AlgoInfoParseAuthData(oplus_fingerprint_auth_ta_info_t *p_info, const uint8_t* p_img_name, char* p_info_buf, const char* p_info_dir, const char* p_info_name) {
    ANC_RETURN_TYPE ret = ANC_OK;
    const char *p_full_path = AsFullPath(p_info_dir, p_info_name);
    ret = AncTestDir(p_full_path);
    uint32_t chip_id_buf_len = sizeof(p_info->auth_event_string1) + sizeof(p_info->auth_event_string2) + 1;
    char *p_chip_id = AncMalloc(chip_id_buf_len);
    if (p_chip_id == NULL) {
        ANC_LOGE("alloc memory fail");
    } else {
        AncMemset(p_chip_id, 0, chip_id_buf_len);
        AncMemcpy(p_chip_id, p_info->auth_event_string1, strlen(p_info->auth_event_string1));
        AncMemcpy(p_chip_id + strlen(p_info->auth_event_string1), p_info->auth_event_string2, strlen(p_info->auth_event_string2));
    }

    if (ANC_OK != ret) {
        ANC_LOGD("%s does not exists create one", p_full_path);
        AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN,
        "\
        image_name,\
        auth_result,\
        fail_reason,\
        retry_cnt,\
        finger_number,\
        matched_template_idx,\
        matched_image_sum,\
        study_status,\
        finger_final_score,\
        compare_cache_score,\
        quality_score,\
        live_score,\
        light_score,\
        finger_seg_score,\
        img_sum,\
        img_variance,\
        img_contrast,\
        finger_strange_score,\
        compress_image_pixel_1,\
        compress_image_pixel_2,\
        compress_image_pixel_3,\
        seg_feat_mean,\
        matched_user_id,\
        matched_finger_id,\
        mat_s,\
        mat_d,\
        mat_c,\
        mat_cs,\
        compress_cls_score,\
        compress_tnum_score,\
        compress_area_score,\
        compress_island_score,\
        algo_version,\
        verify_index,\
        fp_protector_score,\
        fp_temperature_score,\
        env_light,\
        fusion_cnt,\
        expo_time,\
        debase_quality_score,\
        fp_direction,\
        ghost_cache_behavior,\
        dynamic_liveness_th,\
        keyghost_score,\
        ctnghost_score,\
        small_cls_fast_reject_count+small_cls_fast_accept_count,\
        tnum_fast_accept_count+glb_fast_reject_count,\
        lf_fast_reject_count+total_cmp_cls_times,\
        total_cache_count,\
        compress_tsignal,\
        compress_tnoise,\
        compress_fov_hiFreq,\
        compress_expoTime_screenLeakRatio,\
        compress_magnification_shading,\
        chip_id,\
        screen_off,\
        finger_on_time,\
        capture_time,\
        extract_time,\
        verify_time,\
        study_time,\
        kpi_time_all\
        \n");
        ret = AncAppendWriteFile(p_full_path, p_info_buf);
        AncMemset(p_info_buf, 0, sizeof(*p_info_buf) * ANC_ALGO_SAVE_INFO_BUF_LEN);
    }
    AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN, "%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%s,%u,%u,%u,%u,%u,%u,%u\n",
    p_img_name,
    INFO(auth_result),
    INFO(fail_reason),
    INFO(ta_retry_times),
    INFO(finger_number),
    INFO(auth_ta_data[0]),  //matched_template_idx
    INFO(auth_ta_data[6]),  //matched_image_sum
    INFO(study_flag),       //study_status
    INFO(auth_ta_data[1]),  //compare_final_score
    INFO(match_score),      //compare_cache_score
    INFO(quality_score),    //finger_quality_score
    INFO(fake_result),      //live_score
    INFO(exposure_flag),    //light_score
    INFO(img_area),         //finger_seg_score
    INFO(auth_rawdata[p_info->ta_retry_times]),     //retry_cnt
    INFO(auth_ta_data[2]),  //img_variance
    INFO(auth_ta_data[3]),  //img_contrast
    INFO(auth_ta_data[4]),  //finger_strange_score
    INFO(auth_ta_data[17]), //compress_image_pixel_1
    INFO(auth_ta_data[18]), //compress_image_pixel_2
    INFO(auth_ta_data[19]), //compress_image_pixel_3
    INFO(signal_value),     //seg_feat_mean
    INFO(auth_ta_data[7]),  //matched_user_id
    INFO(auth_ta_data[8]),  //matched_finger_id
    INFO(auth_ta_data[9]),  //mat_s
    INFO(auth_ta_data[10]), //mat_d
    INFO(auth_ta_data[11]), //mat_c
    INFO(auth_ta_data[12]), //mat_cs
    INFO(auth_ta_data[13]), //compress_cls_score
    INFO(auth_ta_data[14]), //compress_tnum_score
    INFO(auth_ta_data[15]), //compress_area_score
    INFO(auth_ta_data[16]), //compress_island_score
    INFO(algo_version),
    INFO(auth_ta_data[20]), //verify_index
    INFO(screen_protector_type),    //fp_protector_score
    INFO(touch_diff),       //fp_temperature_score
    INFO(auth_ta_data[21]), //env_light
    INFO(auth_ta_data[22]), //fusion_cnt
    INFO(auth_ta_data[23]), //expo_time
    INFO(auth_ta_data[5]),  //debase_quality_score
    INFO(img_direction),    //fp_direction
    INFO(auth_ta_data[24]), //ghost_cache_behavior
    INFO(auth_ta_data[25]), //dynamic_liveness_th
    INFO(finger_type), //keyghost_score
    INFO(recog_round), //ctnghost_score
    INFO(fdt_base_flag), //small_cls_fast_reject_count+small_cls_fast_accept_count
    INFO(image_base_flag), //tnum_fast_accept_count+glb_fast_reject_count
    INFO(mp_touch_diff), //lf_fast_reject_count+total_cmp_cls_times
    INFO(memory_info), //total_cache_count
    INFO(auth_ta_data[26]), //compress_tsignal
    INFO(auth_ta_data[27]), //compress_tnoise
    INFO(auth_ta_data[28]), //compress_fov_hiFreq
    INFO(auth_ta_data[29]), //compress_expoTime_screenLeakRatio
    INFO(auth_ta_data[30]), //compress_magnification_shading
    p_chip_id,                //chip_id
    INFO(auth_event_state1),//screen_off
    INFO(errtouch_flag), // finger_on_time
    INFO(capture_time[p_info->ta_retry_times]),
    INFO(get_feature_time[p_info->ta_retry_times]),
    INFO(auth_time[p_info->ta_retry_times]),
    INFO(study_time),
    INFO(kpi_time_all[p_info->ta_retry_times])
    );
    ANC_LOGD("AlgoInfoParseAuthData: %s",p_info_buf);
    ret = AncAppendWriteFile(p_full_path, p_info_buf);

    if (p_chip_id != NULL) {
        AncFree(p_chip_id);
        p_chip_id = NULL;
    }

    return ret;
}

static ANC_RETURN_TYPE AlgoInfoParseSingleEnrollData(oplus_fingerprint_singleenroll_ta_info_t *p_info,
                                                     const uint8_t *p_img_name, char *p_info_buf, const char *p_info_path,
                                                     const char *p_info_name) {
    ANC_RETURN_TYPE ret = ANC_OK;
    const char *p_full_path = AsFullPath(p_info_path, p_info_name);
    ret = AncTestDir(p_full_path);
    if (ANC_OK != ret) {
        ANC_LOGD("%s does not exists create one", p_full_path);
        AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN,
        "\
        image_name,\
        enroll_result,\
        quality_score,\
        live_score,\
        light_score,\
        finger_seg_score,\
        compress_image_pixel_3,\
        seg_feat_mean,\
        capture_time,\
        extract_time,\
        enroll_time,\
        kpi_time_all\
        \n");
        ret = AncAppendWriteFile(p_full_path, p_info_buf);
        AncMemset(p_info_buf, 0, sizeof(*p_info_buf) * ANC_ALGO_SAVE_INFO_BUF_LEN);
    }
    AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN, "%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
    p_img_name,
    INFO(singleenroll_result),
    INFO(quality_score),
    INFO(anomaly_flag),
    INFO(exposure_flag),
    INFO(img_area),
    INFO(enroll_rawdata),
    INFO(signal_value),
    INFO(capture_time),
    INFO(get_feature_time),
    INFO(enroll_time),
    INFO(kpi_time_all)
    );
    ANC_LOGD("AlgoInfoParseSingleEnrollData: %s",p_info_buf);
    ret = AncAppendWriteFile(p_full_path, p_info_buf);
    return ANC_OK;
}

static ANC_RETURN_TYPE AlgoInfoParseEnrollFinishData(oplus_fingerprint_enroll_ta_info_t *p_info, char* p_info_buf, const char* p_info_path, const char *p_info_name) {
    ANC_RETURN_TYPE ret = ANC_OK;
    const char *p_full_path = AsFullPath(p_info_path, p_info_name);
    ret = AncTestDir(p_full_path);
    if (ANC_OK != ret) {
        ANC_LOGD("%s does not exists create one", p_full_path);
        AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN,
        "\
        enroll_result,\
        finger_number,\
        duplicate_enroll_cnt,\
        total_enroll_cnt,\
        algo_version,\
        template_version\
        \n");
        ret = AncAppendWriteFile(p_full_path, p_info_buf);
        AncMemset(p_info_buf, 0, sizeof(*p_info_buf) * ANC_ALGO_SAVE_INFO_BUF_LEN);
    }
    AncSnprintf(p_info_buf, ANC_ALGO_SAVE_INFO_BUF_LEN, "%u,%u,%u,%u,%s,%u\n",
    INFO(enroll_result),
    INFO(finger_number),
    INFO(repetition_enroll_number),
    INFO(total_enroll_times),
    INFO(algo_version),
    INFO(template_version)
    );
    ANC_LOGD("AlgoInfoParseEnrollFinishData: %s", p_info_buf);
    ret = AncAppendWriteFile(p_full_path, p_info_buf);
    return ANC_OK;
}


ANC_RETURN_TYPE  AlgoInfoParse(uint8_t *p_share_buffer, ANC_COMMAND_EXTENSION_TYPE extension_command_type, uint8_t * p_image_name_buffer) {
    ANC_LOGD("save algo info");
    ANC_RETURN_TYPE ret = ANC_OK;
    ret = AncTestDir(gp_algo_info_root);
    if (ANC_OK != ret) {
        ANC_LOGD("%s does not exists create one", gp_algo_info_root);
        // create a new directory
        ret = AncCreateMultiLevelDir((char*)gp_algo_info_root);
        if (ret != ANC_OK) {
            ANC_LOGD("file system mkdir() FAILED! errno = %d", ret);
            return ret;
        }
    }
    char* p_info_buf = AncMalloc(ANC_ALGO_SAVE_INFO_BUF_LEN);
    AncMemset(p_info_buf, 0, sizeof(char) * ANC_ALGO_SAVE_INFO_BUF_LEN);

    char* p_info_file_name = AncMalloc(ANC_ALGO_SAVE_INFO_PATH_LEN);
    AncMemset(p_info_file_name, 0, sizeof(char) * ANC_ALGO_SAVE_INFO_PATH_LEN);

    if(extension_command_type == ANC_CMD_EXTENSION_INIT_DATA_COLLOCT){
        AncSnprintf(p_info_file_name, ANC_ALGO_SAVE_INFO_PATH_LEN, "%s%s", gp_info_file_prefix[0], gp_algo_base_info_name);
        oplus_fingerprint_init_ta_info_t *p_init_algo_info  = (oplus_fingerprint_init_ta_info_t *)p_share_buffer;
        ret = AlgoInfoParseInitData(p_init_algo_info, p_info_buf, gp_algo_info_root, p_info_file_name);
    }
    if(extension_command_type == ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT){
        AncSnprintf(p_info_file_name, ANC_ALGO_SAVE_INFO_PATH_LEN, "%s%s", gp_info_file_prefix[1], gp_algo_base_info_name);
        oplus_fingerprint_singleenroll_ta_info_t *p_single_enroll_algo_info  = (oplus_fingerprint_singleenroll_ta_info_t *)p_share_buffer;
        ret = AlgoInfoParseSingleEnrollData(p_single_enroll_algo_info, p_image_name_buffer, p_info_buf, gp_algo_info_root, p_info_file_name);
    }
    if(extension_command_type == ANC_CMD_EXTENSION_ENROLL_END_COLLOCT){
        AncSnprintf(p_info_file_name, ANC_ALGO_SAVE_INFO_PATH_LEN, "%s%s", gp_info_file_prefix[2], gp_algo_base_info_name);
        oplus_fingerprint_enroll_ta_info_t *p_enroll_finish_algo_info  = (oplus_fingerprint_enroll_ta_info_t *)p_share_buffer;
        ret = AlgoInfoParseEnrollFinishData(p_enroll_finish_algo_info, p_info_buf, gp_algo_info_root, p_info_file_name);
    }
    if(extension_command_type == ANC_CMD_EXTENSION_AUTH_RESULT_COLLOCT){
        AncSnprintf(p_info_file_name, ANC_ALGO_SAVE_INFO_PATH_LEN, "%s%s", gp_info_file_prefix[3], gp_algo_base_info_name);
        oplus_fingerprint_auth_ta_info_t *p_auth_algo_info  = (oplus_fingerprint_auth_ta_info_t *)p_share_buffer;
        ret = AlgoInfoParseAuthData(p_auth_algo_info,p_image_name_buffer, p_info_buf, gp_algo_info_root, p_info_file_name);
    }

    AncFree(p_info_buf);
    AncFree(p_info_file_name);

    if (ret != ANC_OK) {
        ANC_LOGE("write algo info error! type = %d err = %d",extension_command_type,ret);
        return ret;
    }

    return ret;
}
