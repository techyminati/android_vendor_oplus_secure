#ifndef __ANC_LIB_H__
#define __ANC_LIB_H__

#include "anc_type.h"
#include "anc_error.h"

#define ANC_PATH_LEN 512

#define ANC_CHECK_ERR(_expr)                           \
    do {                                               \
        ANC_RETURN_TYPE _s = (_expr);                  \
        if (_s != ANC_OK) {                            \
            ANC_LOGE("error: return value = %d", _s); \
        }                                              \
    } while (0)

#define ANC_CHECK_RET(_expr)          \
    do {                              \
        ANC_RETURN_TYPE _s = (_expr); \
        if (_s != ANC_OK) {           \
            return _s;                \
        }                             \
    } while (0)

#define ANC_CHECK_BREAK(_expr)                                      \
    {                                                               \
        ANC_RETURN_TYPE _s = (_expr);                               \
        if (_s != ANC_OK) {                                         \
            ANC_LOGE("break: return value = %d, %s", _s, #_expr);  \
            break;                                                  \
        }                                                           \
    }

#define ANC_CHECK_COND(_cond, _fail, _msg...) \
    do {                                      \
        if (!(_cond)) {                       \
            ANC_LOGE(_msg);                  \
            return _fail;                     \
        }                                     \
    } while (0)

#define ANC_CHECK_COND_GOTO(_cond, _goto_name, _msg...) \
    do {                                                \
        if (!(_cond)) {                                 \
            ANC_LOGE(_msg);                            \
            goto _goto_name;                            \
        }                                               \
    } while (0)

#define ANC_ARRAY_LEN(_array) sizeof(_array) / sizeof(_array[0])

typedef struct {
    void *buf;
    uint32_t size;
} AncOutBlob;

typedef struct {
    const void *buf;
    uint32_t size;
} AncInBlob;

typedef struct {
    int32_t width;
    int32_t height;
    int32_t bit;        //图像像素的位数，例如2字节为16，4字节为32
    int32_t valid_bit;  //图像像素的有效位数，例如0302为10，0301为12
    uint8_t* p_data;
} ImageBin;

const char *AsFullPath(const char *p_root_path, const char *p_format, ...);
ANC_RETURN_TYPE CheckCRC32(uint32_t crc32, const void *p_buf, int size);
ANC_RETURN_TYPE GenerateCRC32(const void *p_buf, int size, uint32_t *p_crc32);

char *AncItoa(int num, char *str, unsigned radix);


ANC_RETURN_TYPE AncSaveImage(uint8_t *p_image, uint32_t image_size, char *file_path, char *file_name);

ANC_RETURN_TYPE AncReadFile(const char *p_file_path, void *p_data_buffer,
                            size_t data_buffer_size, size_t *p_read_size);
int32_t AncWriteFile(const char *file_path, const char *file_name,
                             uint8_t *p_data, uint32_t data_size);
int32_t AncWriteFullPathFile(const char *file_path, const void *p_data,
                                     uint32_t data_size);

ANC_RETURN_TYPE AncCreateDir(char *p_dir_path);
ANC_RETURN_TYPE AncCreateMultiLevelDir(char * p_dir_path);



ANC_RETURN_TYPE AncAppendWriteFile(const char *p_file_path, void *p_data_buffer);

ANC_RETURN_TYPE AncPrintBufferSumValue(const char *p_title, const char *p_buffer, uint32_t buffer_size);

ANC_RETURN_TYPE AncConvertBin2BMP(ImageBin *img_bin, uint8_t *p_bmp_buf, uint32_t bmp_buf_length, uint32_t *p_bmp_size);

#endif
