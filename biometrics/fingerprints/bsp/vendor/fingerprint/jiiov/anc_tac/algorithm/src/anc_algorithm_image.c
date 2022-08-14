#define LOG_TAG "[ANC_TAC][AlgoImage]"

#include "anc_algorithm_image.h"

#include <string.h>
#include <stdio.h>
#include <cutils/properties.h>

#include "anc_file.h"
#include "string.h"
#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "anc_lib.h"
#include "algorithm_image.h"
#include "anc_memory_wrapper.h"
#include "anc_tac_time.h"
#include "anc_type.h"
#include "anc_utils.h"


#define ANC_ALGO_SAVE_IMAGE_DIR_LEN 512
#define ANC_ALGO_SAVE_IMAGE_MAX_NUM 10

const static char *gp_algo_image_root = ANC_DATA_ROOT;

const static char *gp_image_folder_level_1_base = "base/";
const static char *gp_image_folder_level_2_enroll = "enroll/";
const static char *gp_image_folder_level_2_verify = "verify/";
const static char *gp_image_folder_level_3 = "0/";
const static char *gp_image_folder_level_4_enroll_tmp = "tmp/";

const static char *gp_image_folder_enroll_fail_suffix = "_unfinished";

const static char *gp_algo_base_image_name = "calibration.bin";
const static char *gp_algo_base_image_path = "/mnt/vendor/persist/fingerprint/jiiov/calibration_base.bin";

static char g_hb_folder_path[ANC_ALGO_SAVE_IMAGE_DIR_LEN] = "";

static char *gp_image_prefix[] = {
    "raw",
    "bin",
    "bkg",
    "tmp",
    "tmp",
    "pre",
    "base",
    "hb"
};

static char *gp_image_folder_level_1[] = {
    "image_raw/",
    "image_bin/",
    "image_bkg/",
    "image_tmp/",
    "image_tmp/",
    "image_pre/",
    "base/",
    "heart_beat/"
};

static ANC_RETURN_TYPE AlgoImageGetEnrollName(int image_type, AncAlgoEnrollImageInfo enroll_info, char *p_file_name, char* p_time_str, char* p_device_id_str) {
    char * p_type=gp_image_prefix[image_type-1];
    AncSnprintf(p_file_name, ANC_ALGO_SAVE_IMAGE_DIR_LEN, "%s_%s_%04d_%s_TRY_%d_E%d_%d_%d_%d.bin", p_type, p_time_str, enroll_info.index, p_device_id_str, enroll_info.try_count, enroll_info.expo_time/100, enroll_info.image_sum, \
            enroll_info.algo_status, enroll_info.sdk_status);
    ANC_LOGD("save algo image enroll file name =%s", p_file_name);

    return ANC_OK;
}

static ANC_RETURN_TYPE AlgoImageGetVerifyName(int image_type, AncAlgoVerifyImageInfo verify_info, char *p_file_name, char* p_time_str, char* p_device_id_str) {
    char * p_type = gp_image_prefix[image_type-1];
    char * p_result = (verify_info.cmp_result == 1 ? "PASS": "FAIL");
    if (image_type == ANC_ALGORITHM_IMAGE_TMI)
        p_result = (verify_info.cmp_result == 1 ? "PASS_island": "FAIL_island");
    if (image_type == ANC_ALGORITHM_IMAGE_TMC)
        p_result = (verify_info.cmp_result == 1 ? "PASS_cache": "FAIL_cache");
    AncSnprintf(p_file_name, ANC_ALGO_SAVE_IMAGE_DIR_LEN, "%s_%s_%04d_%s_%s_TRY_%d_E%d_%u_%d_%d_MS_%d_CS_%d_CMP_%d_%d.bin", p_type, p_time_str, verify_info.index, p_device_id_str, p_result, verify_info.try_count, verify_info.expo_time/100, \
            (uint32_t)verify_info.match_user_id, verify_info.match_finger_id, verify_info.matched_img_sum, verify_info.match_score, verify_info.match_cache_score, \
            verify_info.algo_status, verify_info.sdk_status);
    ANC_LOGD("save algo image verify file name =%s", p_file_name);

    return ANC_OK;
}

static ANC_RETURN_TYPE AlgoImageGetHeartBeatPathFileName(int image_type, AncAlgoHeartBeatImageInfo hb_info, char *p_path_name, char* p_time_str, char *p_file_name) {
    if (hb_info.index == 0) {
        strcat(p_path_name, gp_image_folder_level_1[(int)image_type - 1]);
        strcat(p_path_name, p_time_str);
        AncMemcpy(g_hb_folder_path, p_path_name, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
        AncCreateMultiLevelDir(p_path_name);
    } else {
        AncMemcpy(p_path_name, g_hb_folder_path, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
    }

    AncSnprintf(p_file_name, ANC_ALGO_SAVE_IMAGE_DIR_LEN, "%s_%06d_%s_T%lld_E%d_F%d_B%d_R%d.bin", gp_image_prefix[image_type-1], hb_info.index, p_time_str, hb_info.timestamp, hb_info.expo_time, hb_info.feat, hb_info.bpm, hb_info.report_bpm);

    return ANC_OK;
}

static ANC_RETURN_TYPE AlgoImageMakeEnrollSubDir(char* p_image_dir, const char* p_sub_str) {
    ANC_RETURN_TYPE ret = ANC_OK;

    AncStrlcat(p_image_dir, p_sub_str, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
    ret = AncCreateDir(p_image_dir);

    return  ret;
}


static ANC_RETURN_TYPE AlgoImageMakeDirs(char *p_image_dir,char* p_image_type,const char *p_action_type_name) {
    ANC_RETURN_TYPE ret = ANC_OK;

    AncStrlcat(p_image_dir, p_image_type, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
    AncStrlcat(p_image_dir, p_action_type_name, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
    AncStrlcat(p_image_dir, "0/", ANC_ALGO_SAVE_IMAGE_DIR_LEN);
    ret = AncCreateMultiLevelDir(p_image_dir);

    return ret;
}

ANC_RETURN_TYPE  AlgoImageParse(uint8_t *p_share_buffer, ANC_COMMAND_EXTENSION_TYPE algo_command_type, uint8_t *p_image_name_buffer) {
    ANC_LOGD("save algo image");
    ANC_COMMAND_EXTENSION_TYPE command_type = algo_command_type;
    AncAlgorithmImageData *p_algo_image = NULL;
    ANC_RETURN_TYPE ret = ANC_OK;
    uint8_t* p_image_data = NULL;
    AncAlgorithmImageItemHeader *one_item = NULL;
    char a_image_dir[ANC_ALGO_SAVE_IMAGE_DIR_LEN] = "";
    char a_file_name[ANC_ALGO_SAVE_IMAGE_DIR_LEN] = "";
    int writed_data = 0;

    char prop_sn[PROPERTY_VALUE_MAX] = "0";
    if(property_get("ro.serialno", prop_sn, NULL) != 0) {
#ifdef ANC_DEBUG
        ANC_LOGD("sn number = %s", prop_sn);
#endif
    }
    p_algo_image = (AncAlgorithmImageData *)p_share_buffer;
    if (p_algo_image->image_count <= 0 || p_algo_image->image_count > ANC_ALGO_SAVE_IMAGE_MAX_NUM) {
        ANC_LOGE("save algo image count error!");
        return ANC_FAIL;
    }
    ANC_LOGD("save algo image count=%d", p_algo_image->image_count);

    one_item=(AncAlgorithmImageItemHeader *)(p_share_buffer+offsetof(AncAlgorithmImageData, p_image_item));
    p_image_data = (uint8_t *)(one_item+p_algo_image->image_count);

    const char *p_action_type_name = command_type == ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT ? gp_image_folder_level_2_enroll : gp_image_folder_level_2_verify;
    ANC_LOGD("save algo image sharebuffer=%p item=%p imagedata=%p",p_share_buffer, one_item, p_image_data);
    char time[50] = {0};
    AncStrfTime(time, sizeof(time));
    for (size_t i = 0; i < p_algo_image->image_count; i++) {
        AncMemset(a_file_name, 0, sizeof(a_file_name));
        strcpy(a_image_dir, gp_algo_image_root);

        ANC_LOGD("save algo image save index=%zu image type=%d", i, one_item->image_type);
        ANC_ALGORITHM_IMAGE_TYPE image_type = one_item->image_type;
        if((int)image_type > (int)(sizeof(gp_image_folder_level_1)/sizeof(gp_image_folder_level_1[0]))){
            ANC_LOGE("save algo image unknow type!");
            return ANC_FAIL;
        }

        if (image_type == ANC_ALGORITHM_IMAGE_BASE) {
            strcat(a_image_dir, gp_image_folder_level_1[(int)image_type - 1]);
            strcpy(a_file_name, gp_algo_base_image_name);
            ANC_LOGD("base image path:%s, file_name:%s", a_image_dir, a_file_name);
        } else if (image_type == ANC_ALGORITHM_IMAGE_HBR) {
            AncAlgoHeartBeatImageInfo hb_info = p_algo_image->image_info.hb;
            AlgoImageGetHeartBeatPathFileName((int)image_type, hb_info, a_image_dir, time, a_file_name);
            ANC_LOGD("hb image path:%s, file_name:%s", a_image_dir, a_file_name);
        } else {
            ret = AlgoImageMakeDirs(a_image_dir, gp_image_folder_level_1[(int)image_type - 1], p_action_type_name);
            if(ANC_OK != ret) {
                ANC_LOGE("save algo image mkdirs error!");
                return ret;
            }

            if (command_type == ANC_CMD_EXTENSION_SINGLE_ENROLL_COLLOCT) {
                ret = AlgoImageMakeEnrollSubDir(a_image_dir, gp_image_folder_level_4_enroll_tmp);
                if(ANC_OK != ret) {
                    ANC_LOGE("save algo image parent dir error!");
                    return ret;
                }
                ANC_LOGD("save algo image enroll folder = %s",a_image_dir);

                ANC_LOGD("save algo image make getenrollname");

                ret = AlgoImageGetEnrollName((int)image_type,p_algo_image->image_info.enroll, a_file_name, time, prop_sn);
                if(ANC_OK != ret) {
                    ANC_LOGE("save algo image enroll name error!");
                    return ret;
                }
            } else {
                ret = AlgoImageGetVerifyName((int)image_type,p_algo_image->image_info.verify, a_file_name, time, prop_sn);
                if(ANC_OK != ret) {
                    ANC_LOGE("save algo image enroll name error!");
                    return ret;
                }
            }
            ANC_LOGD("save algo image write %s : %s",a_image_dir, a_file_name);
        }

        if ((p_image_name_buffer != NULL) && (image_type == ANC_ALGORITHM_IMAGE_BKG)) {
            strcpy((char*)p_image_name_buffer, a_file_name);
        }

        char title[32] = {0};
        AncSnprintf(title, sizeof(title), "in ca save_image_%d", image_type);
        AncPrintBufferSumValue(title, (const char*)p_image_data, one_item->image_buffer_length);
        writed_data = AncWriteFile(a_image_dir, a_file_name, (uint8_t *)p_image_data, one_item->image_buffer_length);
        if ((uint32_t)writed_data != one_item->image_buffer_length) {
            ANC_LOGE("file to write file, writed size:%d, need to write size:%d",
                   writed_data, one_item->image_buffer_length);
            ret = ANC_FAIL;
        } else {
            ret = ANC_OK;
        }
        p_image_data += one_item->image_buffer_length;
        one_item++;
    }

    return ret;
}

ANC_RETURN_TYPE AlgoImageRenameEnrollFolder(uint32_t finger_id, ANC_BOOL is_finished) {
    ANC_RETURN_TYPE ret = ANC_OK;
    char time[50] = {0};
    AncStrfTime(time, sizeof(time));
    //rename enrolled finger folder by finger_id
    for(int i = 0; i < (int)(sizeof(gp_image_folder_level_1)/sizeof(gp_image_folder_level_1[0])); i++){
        char old_enroll_folder_path[ANC_ALGO_SAVE_IMAGE_DIR_LEN] = "";
        AncSnprintf(old_enroll_folder_path, ANC_ALGO_SAVE_IMAGE_DIR_LEN, "%s%s%s%s",gp_algo_image_root,gp_image_folder_level_1[i],gp_image_folder_level_2_enroll,gp_image_folder_level_3);
        char new_enroll_folder_path[ANC_ALGO_SAVE_IMAGE_DIR_LEN] = "";
        AncStrlcat(new_enroll_folder_path, old_enroll_folder_path, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
        AncStrlcat(old_enroll_folder_path, gp_image_folder_level_4_enroll_tmp, ANC_ALGO_SAVE_IMAGE_DIR_LEN);

        if(is_finished){
            char finger_id_buf[10]="";
            AncMemset((void *)finger_id_buf, 0, sizeof(finger_id_buf));
            AncItoa((int)finger_id, finger_id_buf, sizeof(finger_id_buf));
            AncStrlcat(new_enroll_folder_path, finger_id_buf, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
        } else {
            AncStrlcat(new_enroll_folder_path, time, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
            AncStrlcat(new_enroll_folder_path, gp_image_folder_enroll_fail_suffix, ANC_ALGO_SAVE_IMAGE_DIR_LEN);
        }
        AncStrlcat(new_enroll_folder_path, "/", ANC_ALGO_SAVE_IMAGE_DIR_LEN);
        ANC_LOGD("rename enroll folder, old = %s, new = %s",old_enroll_folder_path,new_enroll_folder_path);
        int ret_val = rename(old_enroll_folder_path,new_enroll_folder_path);
        if(ret_val != 0){
            ANC_LOGE("failed to rename enroll folder, path = %s, finger_id = %d, ret = %d",old_enroll_folder_path,finger_id,ret_val);
            ret = ANC_FAIL;
        }
    }

    // check base image
    char base_path[ANC_ALGO_SAVE_IMAGE_DIR_LEN] = "";
    AncSnprintf(base_path, ANC_ALGO_SAVE_IMAGE_DIR_LEN, "%s%s%s",gp_algo_image_root,gp_image_folder_level_1_base,gp_algo_base_image_name);
    if(ANC_OK != AncTestDir(base_path)){
        // not exist, copy one
        AncMemset(base_path,0,sizeof(base_path));
        AncSnprintf(base_path, ANC_ALGO_SAVE_IMAGE_DIR_LEN, "%s%s",gp_algo_image_root,gp_image_folder_level_1_base);
        ret = AncCopyFile(gp_algo_base_image_path,base_path,gp_algo_base_image_name);
        if(ANC_OK != ret){
            ANC_LOGE("failed to copy base image src = %s, dst = %s",gp_algo_base_image_path,base_path);
        }
    }

    return ret;
}
