#ifndef __ANC_FP_RESULT_HANDLE_H__
#define __ANC_FP_RESULT_HANDLE_H__

#include <stdint.h>
#include "anc_error.h"

#define IMAGE_FILE_NAME_LEN 72

typedef struct {
    int32_t cmd;
    uint32_t exp_type;  /// 曝光方式，0：自动曝光，1：固定曝光
    int32_t exp_time;  /// 曝光时间
    uint32_t width;
    uint32_t height;
    uint32_t image_size;
    uint8_t* p_buffer;
}AncFPImageData;


ANC_RETURN_TYPE ANCSetImageSavePath(const char* p_path);
/// 保存文件
ANC_RETURN_TYPE ANCSaveFPImage(const AncFPImageData *p_image_data, char* p_file_name, uint32_t file_name_len);



#endif 