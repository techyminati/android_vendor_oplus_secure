#define LOG_TAG "[ANC_TAC][VirtualSensor]"

#include "anc_tac_virtual_sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sensor_command_param.h"
#include "anc_log.h"
#include "anc_memory_wrapper.h"
#include "anc_lib.h"
#include "anc_utils.h"

#define VS_IMAGE_DIR ANC_DATA_ROOT "vc_image_raw"


#define VS_IMAGE_FILE_PATH_MAX NAME_MAX
static char g_image_file_path[VS_IMAGE_FILE_PATH_MAX];

typedef char IMAGE_FILE_PATH_ARRAY[VS_IMAGE_FILE_PATH_MAX];

#define VS_IMAGE_FILE_COUNT 50
IMAGE_FILE_PATH_ARRAY g_raw_image_file_name[VS_IMAGE_FILE_COUNT];

typedef struct {
    ANC_SENSOR_IMAGE_PARAM param;
    uint8_t *p_image_data;
} __attribute__((packed)) ANC_VIRTUAL_SENSOR_IMAGE_DATA;

typedef struct {
    int image_index;
    int image_sum;
    char image_file_path[VS_IMAGE_FILE_PATH_MAX];
    IMAGE_FILE_PATH_ARRAY *p_file_name_array;
}VcSpecialParam;


static VcSpecialParam g_vs_special_param = {
    .image_index = 0,
    .image_sum = 0,
    .image_file_path = {0},
    .p_file_name_array = NULL,
};

static ANC_RETURN_TYPE VcResetImagePath(const char *p_dir) {
    struct dirent **p_entry_list = NULL;
    ANC_LOGD("read %s", p_dir);
    int count = scandir(p_dir, &p_entry_list, NULL, alphasort);
    if (count < 0) {
        ANC_LOGE("read %s failed, no files!", p_dir);
        return ANC_FAIL;
    }

    g_vs_special_param.image_index = 0;
    g_vs_special_param.image_sum = 0;
    AncMemset(g_raw_image_file_name, 0, sizeof(g_raw_image_file_name));
    for (int i = 0; i < count; i++) {
        struct dirent *p_entry = p_entry_list[i];
        if (!(strcmp(p_entry->d_name, ".") == 0) && !(strcmp(p_entry->d_name, "..") == 0)) {
            if (g_vs_special_param.image_sum >= VS_IMAGE_FILE_COUNT) {
                ANC_LOGW("read %s the number of files exceeds %d", p_dir, VS_IMAGE_FILE_COUNT);
            } else {
                ANC_LOGD("vc image:%s", p_entry->d_name);
                AncSnprintf(g_raw_image_file_name[g_vs_special_param.image_sum], VS_IMAGE_FILE_PATH_MAX, "%s/%s",
                         p_dir, p_entry->d_name);
                g_vs_special_param.image_sum += 1;
            }
        }
        free(p_entry);
    }
    ANC_LOGD("vc image sum:%d", g_vs_special_param.image_sum);
    g_vs_special_param.p_file_name_array = g_raw_image_file_name;
    free(p_entry_list);
    return ANC_OK;
}

ANC_RETURN_TYPE VcSetCurrentImagePath(uint8_t *p_buffer, uint32_t buffer_length) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    struct stat s_buf;

    if (access((const char *)p_buffer, F_OK) != 0) {
        ANC_LOGE("%s %s can not access!", __func__, p_buffer);
        return ANC_FAIL;
    }
    stat((const char *)p_buffer, &s_buf);
    if (S_ISDIR(s_buf.st_mode)) {
        if (ANC_OK != (ret_val = VcResetImagePath((const char *)p_buffer))) {
            ANC_LOGE("fail to reset image path, return value:%d", ret_val);
        }
    } else {
        g_vs_special_param.image_index = 0;
        g_vs_special_param.image_sum = (int)buffer_length / NAME_MAX;
        g_vs_special_param.p_file_name_array = (IMAGE_FILE_PATH_ARRAY *)p_buffer;
    }
    return ret_val;
}

static ANC_RETURN_TYPE VcGetCurrentImagePath() {
    ANC_RETURN_TYPE ret_val = ANC_FAIL;
    size_t str_res_len1 = 0;
    size_t str_res_len2 = 0;
    size_t image_file_path_len = sizeof(g_image_file_path);

    if ((0 == g_vs_special_param.image_sum)) {
        ANC_LOGD("index:%d, sum:%d ", g_vs_special_param.image_index, g_vs_special_param.image_sum);
        if (ANC_OK != (ret_val = VcResetImagePath(VS_IMAGE_DIR))) {
            ANC_LOGE("fail to reset image path, return value:%d", ret_val);
            return ret_val;
        }
    } else if (g_vs_special_param.image_index >= g_vs_special_param.image_sum) {
        g_vs_special_param.image_index = 0;
    }

    AncMemset((void *)g_image_file_path, 0, image_file_path_len);
    if (g_vs_special_param.image_index < g_vs_special_param.image_sum) {
        str_res_len1 = strlcpy(g_image_file_path, g_vs_special_param.image_file_path, image_file_path_len);
        IMAGE_FILE_PATH_ARRAY *p_file_name_array = g_vs_special_param.p_file_name_array;
        str_res_len2 = AncStrlcat(g_image_file_path, p_file_name_array[g_vs_special_param.image_index], image_file_path_len);
        if ((str_res_len1 >= image_file_path_len) || (str_res_len2 >= image_file_path_len)) {
            ANC_LOGD("String truncation error : %s", g_image_file_path);
            ret_val = ANC_FAIL;
        } else {
            ret_val = ANC_OK;
            g_vs_special_param.image_index++;
        }
    }
    ANC_LOGD("image file path : %s", g_image_file_path);

    return ret_val;
}

ANC_RETURN_TYPE AncGetVirtualSensorImage(uint8_t *p_input_buffer,
                                        uint32_t input_buffer_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    uint8_t *p_image_raw_data = NULL;

    if (NULL == p_input_buffer) {
        ANC_LOGE("p_input_buffer is NULL.");
        return ANC_FAIL;
    }

    ANC_VIRTUAL_SENSOR_IMAGE_DATA *image_data = (ANC_VIRTUAL_SENSOR_IMAGE_DATA *)p_input_buffer;
    p_image_raw_data = p_input_buffer + offsetof(ANC_VIRTUAL_SENSOR_IMAGE_DATA, p_image_data);

    if (ANC_OK != (ret_val = VcGetCurrentImagePath())) {
        ANC_LOGE("fail to get current image file path : return value = %d\n", ret_val);
        return ret_val;
    }

    size_t read_size = 0;
    ret_val = AncReadFile(g_image_file_path, p_image_raw_data, input_buffer_size, &read_size);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to read image file path : return value = %d\n", ret_val);
        return ret_val;
    }
    image_data->param.image_size = (uint32_t)read_size;
    return ret_val;
}
