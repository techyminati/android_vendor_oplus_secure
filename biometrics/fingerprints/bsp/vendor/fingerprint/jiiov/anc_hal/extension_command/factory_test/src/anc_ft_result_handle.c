#include "anc_ft_result_handle.h"
#include "anc_type.h"
#include "anc_log.h"
#include "anc_lib.h"
#include "anc_utils.h"
#include <stdio.h>
#include <sys/time.h>
#include "anc_memory_wrapper.h"


// #define IMAGE_STORE_DIR ANC_EXT_FT_IMAGE_DEFAULT_PATH
// // #define IMAGE_STORE_DIR "/data/local/tmp/anc_0302/ft"
// #define IMAGE_STORE_DIR "/sdcard/FactoryTestReport"

static char g_image_save_path[IMAGE_FILE_NAME_LEN * 2];

ANC_RETURN_TYPE ANCSetImageSavePath(const char* p_path) {
    if (p_path == NULL) {
        ANC_LOGE("parameter error");
        return ANC_FAIL;
    }

    AncMemcpy((void*)g_image_save_path, p_path, strlen(p_path));
    g_image_save_path[strlen(p_path)] = '\0';
    return ANC_OK;
}


ANC_RETURN_TYPE ANCSaveFPImage(const AncFPImageData *p_image_data, char* p_file_name, uint32_t file_name_len) {
    if ((p_image_data == NULL) || (p_file_name == NULL) || (file_name_len <= 0)) {
        ANC_LOGE("parameter error");
        return ANC_FAIL;
    }

    char output_file_name[IMAGE_FILE_NAME_LEN * 2] = {0};
    struct timeval tv;
    gettimeofday(&tv,NULL);
    long create_time = tv.tv_sec*1000 + tv.tv_usec/1000;
    if((p_image_data->p_buffer == NULL) || (p_image_data->image_size < p_image_data->width * p_image_data->height)) {
        ANC_LOGE("input parameter is null, img buf = %p, img size = %d\n",p_image_data->p_buffer,p_image_data->image_size);
        return ANC_FAIL;
    }

    int cnt = AncSnprintf(output_file_name, IMAGE_FILE_NAME_LEN, "%ld-C%d-exp_type_%d-exp_time_%d-%dx%d-%d.raw", create_time, p_image_data->cmd,p_image_data->exp_type, 
                    p_image_data->exp_time, p_image_data->width, p_image_data->height, p_image_data->image_size);
    if(cnt <= 0 || cnt > IMAGE_FILE_NAME_LEN){
        ANC_LOGE("img file name is truncated, (%d, %d)\n", cnt, IMAGE_FILE_NAME_LEN);
        return ANC_FAIL;
    }

    int write_data_size = AncWriteFile(g_image_save_path, output_file_name, p_image_data->p_buffer, p_image_data->image_size);
    if ((uint32_t)write_data_size != p_image_data->image_size) {
        ANC_LOGE("fail to save image : write size = %d\n", write_data_size);
        return ANC_FT_SAVE_IMAGE_FAIL;
    }

    size_t len = strlen(output_file_name);
    AncMemcpy(p_file_name, output_file_name, len);
    p_file_name[cnt] = '\0';

    return ANC_OK;
}