#define LOG_TAG "[ANC_TAC][Image]"

#include "anc_ca_image.h"



#include "anc_log.h"
#include "anc_command.h"
#include "anc_ca.h"
#include "anc_algorithm.h"
#include "anc_tac_sensor.h"
#include "anc_token.h"
#include "anc_file.h"
#include "anc_lib.h"
#include "anc_memory_wrapper.h"
#include "extension_command.h"
#include "anc_utils.h"


#ifdef ANC_EXT_TA_IMAGE_DEFAULT_PATH
#define IMAGE_STORE_DIR ANC_EXT_TA_IMAGE_DEFAULT_PATH
#else
#define IMAGE_STORE_DIR "/data/vendor/fingerprint/image_apk"
#endif
static ANC_RETURN_TYPE SaveImage(uint8_t *p_image_data, uint32_t image_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    static int index = 0;
    char index_string[16] = {0};
    char output_file_name[20] = {0};
    size_t output_file_name_len = sizeof(output_file_name);
    size_t str_res_len1 = 0;
    size_t str_res_len2 = 0;
    size_t str_res_len3 = 0;
    int writed_data = 0;

    if((p_image_data == NULL) || (image_size == 0)) {
        ANC_LOGE("input parameter is null");
        return ANC_FAIL;
    }

    AncMemset((void *)index_string, 0, sizeof(index_string));
    AncItoa(index, index_string, 10);
    str_res_len1 = strlcpy(output_file_name, "ca_raw16_", output_file_name_len);
    str_res_len2 = AncStrlcat(output_file_name, index_string, output_file_name_len);
    str_res_len3 = AncStrlcat(output_file_name, ".raw", output_file_name_len);
    if ((str_res_len1 >= output_file_name_len) || (str_res_len2 >= output_file_name_len)
             || (str_res_len3 >= output_file_name_len)) {
        ANC_LOGE("String truncation error : %s", output_file_name);
        return ANC_FAIL;
    }

    writed_data = AncWriteFile(IMAGE_STORE_DIR, output_file_name, p_image_data, image_size);
    if ((uint32_t)writed_data != image_size) {
        ANC_LOGE("fail to save image : ret_val = %d\n", ret_val);
        ret_val = ANC_FAIL;
    } else {
        ret_val = ANC_OK;
    }

    index++;

    return ret_val;
}

ANC_RETURN_TYPE ExtensionSaveImage(uint8_t *p_buffer, uint32_t p_buffer_length, uint8_t **pp_image_data, uint32_t *pp_image_size, int need_save) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    if((pp_image_data == NULL) || (pp_image_size == NULL)) {
        ANC_LOGE("input parameter is null");
        return ANC_FAIL;
    }

    AncExtensionImage *p_extension_image = NULL;
    p_extension_image = (AncExtensionImage *)p_buffer;
    *pp_image_size = p_extension_image->image_size;

    if(p_buffer_length  < (sizeof(AncExtensionImage) + p_extension_image->image_size)) {
        ANC_LOGE("save image buffer size %d is not enough", p_buffer_length);
        return ANC_FAIL;
    }

    uint8_t *p_image_data = p_buffer + offsetof(AncExtensionImage, p_image_data);
    *pp_image_data = p_image_data;

    if(need_save != 0) {
        if (ANC_OK != (ret_val = SaveImage(p_image_data, p_extension_image->image_size))) {
            return ret_val;
        }
    }

    return ret_val;
}
