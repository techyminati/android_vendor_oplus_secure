#define LOG_TAG "[ANC_COMMON][Lib]"

#include "anc_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "anc_file.h"
#include "anc_log.h"
#include "anc_type.h"
#include "anc_memory_wrapper.h"
#include "anc_utils.h"

#define CRC_TABLE_SIZE 256
#define COLOR_TABLE_LEN (256 * 4)
#define BIT_2_VALID_CNT 12
#define BIT_4_VALID_CNT 16

typedef struct __attribute__((packed)) {
  uint16_t  bfType;
  uint32_t  bfSize;
  uint16_t  bfReserved1;
  uint16_t  bfReserved2;
  uint32_t bfOffBits;
} BitmapFileHeader;

typedef struct __attribute__((packed)) {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BitmapInfoHeader;

typedef struct __attribute__((packed)) {
  BitmapFileHeader bitmap_file_header;  // bitmap file header
  BitmapInfoHeader bitmap_info_header;  // bitmap info header
  char bmp_color_table[COLOR_TABLE_LEN];
} BitmapHeader;

static void GenerateTable(uint32_t table[CRC_TABLE_SIZE]) {
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < CRC_TABLE_SIZE; i++) {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++) {
            if (c & 1) {
                c = polynomial ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }
        table[i] = c;
    }
}

static uint32_t CalCrc32(uint32_t initial,
                           const void* buf, int len) {
    uint32_t c = initial ^ 0xFFFFFFFF;
    const uint8_t* u = (const uint8_t*)buf;
    uint32_t table[CRC_TABLE_SIZE];
    GenerateTable(table);
    for (int i = 0; i < len; ++i) {
        c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

const char *AsFullPath(const char *p_root_path, const char *p_format, ...) {
    static char path[ANC_PATH_LEN] = {0};
    char file[sizeof(path) / 2] = {0};

    va_list ap;
    va_start(ap, p_format);

    int n = AncVsnprintf(file, sizeof(file), p_format, ap);
    va_end(ap);

    if ((unsigned long)n < sizeof(file)) {
        n = AncSnprintf(path, sizeof(path), "%s/%s", p_root_path, file);
        if ((unsigned long)n < sizeof(path)) {
            path[n] = '\0';
            return path;
        }
    }
    return NULL;
}

ANC_RETURN_TYPE GenerateCRC32(const void *p_buf, int size, uint32_t *p_crc32) {
    *p_crc32 = CalCrc32(0, p_buf, size);
    return ANC_OK;
}

ANC_RETURN_TYPE CheckCRC32(uint32_t crc32, const void *p_buf, int size) {
    uint32_t val = 0;
    GenerateCRC32(p_buf, size, &val);
    return val == crc32 ? ANC_OK : ANC_FAIL;
}

ANC_RETURN_TYPE AncReadFile(const char *p_file_path, void *p_data_buffer,
                            size_t data_buffer_size, size_t *p_read_size) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_FILE_HANDLE fd = NULL;
    int64_t file_size = 0;
    int32_t read_file_size = 0;

    do {
        fd = AncFileOpen(p_file_path, ANC_O_RDONLY);
        if (NULL == fd) {
            ANC_LOGE("fail to open file: %s", p_file_path);
            ret_val = ANC_FAIL_OPEN_FILE;
            break;
        }
        ANC_LOGD("file fd : %p", fd);
        ret_val = AncFstat(fd, &file_size);
        if (ANC_OK == ret_val) {
            if ((0 < file_size) && (data_buffer_size < (size_t)file_size)) {
                ANC_LOGE(
                    "image buffer is too small! image buffer size : %zu, file "
                    "size : %lld",
                    data_buffer_size, file_size);
                ret_val = ANC_FAIL_BUFFER_TOO_SMALL;
                break;
            }
        } else {
            break;
        }

        read_file_size = AncFileRead(fd, p_data_buffer, (uint32_t)file_size);
        if (read_file_size < 0) {
            ANC_LOGE("fail to read file! errno = %zd", read_file_size);
            ret_val = ANC_FAIL_READ_FILE;
            break;
        }
        *p_read_size = (size_t)file_size;

        ret_val = AncFileClose(fd);
        if (ret_val != ANC_OK) {
            ANC_LOGE("fail to close file! returned = %d", ret_val);
            break;
        } else {
            fd = NULL;
        }
    } while (0);

    /* Clean up */
    if (fd != NULL) {
        if (AncFileClose(fd)) {
            ANC_LOGE("Clean Up FAILED: Heap Memory Leaked", ret_val);
        }
    }

    return ret_val;
}

int32_t AncWriteFullPathFile(const char *file_path, const void *p_data,
                                     uint32_t data_size) {
    ANC_RETURN_TYPE ret = ANC_OK;
    ANC_FILE_HANDLE fd = NULL;
    int32_t writed_data = 0;

    do {
        fd = AncFileOpen(file_path, ANC_O_RDWR | ANC_O_CREAT | ANC_O_TRUNC);
        if (fd == NULL) {
            ret = ANC_FAIL;
            ANC_LOGE("file system open() FAILED!");
            break;
        }
        // write a file
        writed_data = AncFileWrite(fd, (void *)p_data, data_size);
        if ((uint32_t)writed_data != data_size) {
            ret = ANC_FAIL;
            ANC_LOGE("file system write() FAILED! writed size:%d, need to write size:%d",
                      writed_data, data_size);
            break;
        }
        // fsync
        ret = AncFileSync(fd);
        if (ANC_OK != ret) {
            ret = ANC_FAIL;
            ANC_LOGE("file system fsync() FAILED! errno = %d", ret);
            break;
        }
        // close
        ret = AncFileClose(fd);
        if (ret != ANC_OK) {
            ret = ANC_FAIL;
            ANC_LOGE("file system close FAILED! returned = %d", ret);
            break;
        } else {
            fd = NULL;
        }
    } while (0);

    /* Clean up */
    if (fd != NULL) {
        if (AncFileClose(fd)) {
            ANC_LOGE("Clean Up FAILED: Heap Memory Leaked", ret);
        }
    }

    if (ret != ANC_OK) {
        writed_data = 0;
    }

    return writed_data;
}

int32_t AncWriteFile(const char *file_path, const char *file_name,
                             uint8_t *p_data, uint32_t data_size) {
    int32_t writed_data = 0;

    ANC_CHECK_COND(file_path != NULL && file_name != NULL, ANC_FAIL,
                   "pointer cannot be empty");

    ANC_RETURN_TYPE ret = AncTestDir(file_path);
    if (ANC_OK != ret) {
        ANC_LOGD("%s does not exists create one", file_path);
        // create a new directory
        ret = AncMkDir(file_path, 0755);
        if (ret != ANC_OK) {
            ANC_LOGD("file system mkdir() FAILED! errno = %d", ret);
            return writed_data;
        }
    }

    const char *full_path = AsFullPath(file_path, file_name);
    writed_data = AncWriteFullPathFile(full_path, p_data, data_size);

    return writed_data;
}

ANC_RETURN_TYPE AncCreateDir(char *p_dir_path) {
    ANC_RETURN_TYPE ret_val = AncTestDir(p_dir_path);
    if (ANC_OK != ret_val) {
        ANC_LOGD("%s does not exists create one", p_dir_path);
        //create a new directory
        ret_val = AncMkDir(p_dir_path, 0755);
    }
    return ret_val;

}

ANC_RETURN_TYPE AncCreateMultiLevelDir(char * p_dir_path) {
    char dir_name[512];
    size_t i, len;
    ANC_RETURN_TYPE ret = ANC_OK;

    size_t str_res_len = strlcpy(dir_name, p_dir_path,sizeof(dir_name));
    if (str_res_len >= sizeof(dir_name)) {
        ANC_LOGE("AncCreateMultiLevelDir size error : %s", p_dir_path);
    }
    len =  strlen(dir_name);
    if('/' != dir_name[len-1]) {
            strcat(dir_name, "/");
            len++;
    }
    for(i=1; i<len; i++) {
        if('/' == dir_name[i]) {
            dir_name[i] = '\0';
            ret = AncCreateDir(dir_name);
            if (ANC_OK != ret) {
                return ret;
            }
            dir_name[i] = '/';
        }
    }
    return ret;
}

char *AncItoa(int num, char *str, unsigned radix) {
    char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i = 0, j, k;

    if ((radix == 10) && (num < 0)) {
        unum = (unsigned)-num;
        str[i++] = '-';
    } else {
        unum = (unsigned)num;
    }

    do {
        str[i++] = index[unum % ((unsigned)radix)];
        unum /= radix;
    } while (unum);

    str[i] = '\0';

    if (str[0] == '-') {
        k = 1;
    } else {
        k = 0;
    }

    char temp;
    for (j = k; j <= (i - 1) / 2; j++) {
        temp = str[j];
        str[j] = str[i - 1 + k - j];
        str[i - 1 + k - j] = temp;
    }

    return str;
}



ANC_RETURN_TYPE AncSaveImage(uint8_t *p_image, uint32_t image_size, char *file_path, char *file_name) {
    ANC_RETURN_TYPE ret = ANC_OK;
    int32_t writed_data = 0;

    writed_data = AncWriteFile(file_path, file_name, p_image, image_size);
    if ((size_t)writed_data != image_size) {
        ANC_LOGE("file to save image, writed size:%d, need to write size:%d",
                   writed_data, image_size);
        ret = ANC_FAIL;
    }

    return ret;

}

ANC_RETURN_TYPE AncCopyFile(const char *p_src_path,const char *p_dst_folder_path,const char* p_dst_file_name){
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_FILE_HANDLE fd = NULL;
    int64_t file_size = 0;
    int32_t read_file_size = 0;
    int32_t write_file_size = 0;
    char* p_data_buffer = NULL;

    do {
        fd = AncFileOpen(p_src_path, ANC_O_RDONLY);
        if (fd == NULL) {
            ANC_LOGE("fail to open file!");
            ret_val = ANC_FAIL_OPEN_FILE;
            break;
        }
        ANC_LOGD("file fd : %p", fd);

        AncFstat(fd, &file_size);
        if(file_size <= 0){
            ANC_LOGE("wrong file size: %lld\n", file_size);
            break;
        }

        p_data_buffer = AncMalloc((size_t)file_size);
        if(NULL == p_data_buffer) {
            ANC_LOGE("fail to anc malloc for verifying\n");
            break;
        }

        read_file_size = AncFileRead(fd, p_data_buffer, (uint32_t)file_size);
        if (read_file_size < 0) {
            ANC_LOGE("fail to read file! errno = %zd", read_file_size);
            ret_val = ANC_FAIL_READ_FILE;
            break;
        }

        ANC_RETURN_TYPE ret = AncTestDir(p_dst_folder_path);
        if (ANC_OK != ret) {
            ANC_LOGD("%s does not exists create one", p_dst_folder_path);
            // create a new directory
            ret = AncMkDir(p_dst_folder_path, 0755);
            if (ret != ANC_OK) {
                ANC_LOGD("file system mkdir() FAILED! errno = %d", ret);
                break;
            }
        }

        const char *full_path = AsFullPath(p_dst_folder_path, p_dst_file_name);
        write_file_size = AncWriteFullPathFile(full_path, p_data_buffer, (uint32_t)file_size);
        if(write_file_size != file_size){
            ANC_LOGE("write wrong file size: %d\n", write_file_size);
        }
    } while (0);

    /* Clean up */
    if (fd != NULL) {
        ret_val = AncFileClose(fd);
        if (ret_val != ANC_OK) {
            ANC_LOGE("Clean Up FAILED: Heap Memory Leaked, ret_val = %d", ret_val);
        }
    }

    if (p_data_buffer != NULL) {
        AncFree(p_data_buffer);
    }

    return ret_val;
}

ANC_RETURN_TYPE AncAppendWriteFile(const char *p_file_path, void *p_data_buffer) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    ANC_FILE_HANDLE fd = NULL;

    do {

        ret_val = AncTestDir(p_file_path);
        if (ANC_OK != ret_val) {
            ANC_LOGD("%s does not exists create one", p_file_path);
            // create a new file
            fd = AncFileOpen(p_file_path, ANC_O_RDWR | ANC_O_CREAT);
        } else {
            fd = AncFileOpen(p_file_path, ANC_O_RDWR | ANC_O_CREAT| ANC_O_APPEND);
        }

        if (fd == NULL) {
            ANC_LOGE("fail to open file!");
            break;
        }
        ANC_LOGD("file fd : %p", fd);
        //ANC_LOG_D("ptr = %p, len = %d, content = %s", p_data_buffer, strlen(p_data_buffer), p_data_buffer);
        int32_t writed_data = AncFileWrite(fd, p_data_buffer, (uint32_t)strlen(p_data_buffer));
        if ((uint32_t)writed_data != strlen(p_data_buffer)) {
            ret_val = ANC_FAIL;
            ANC_LOGE("file system write() FAILED! writed size:%d, need to write size:%d",
                      writed_data, strlen(p_data_buffer));
            break;
        }
                // fsync
        ret_val = AncFileSync(fd);
        if (ANC_OK != ret_val) {
            ret_val = ANC_FAIL;
            ANC_LOGE("file system fsync() FAILED! errno = %d", ret_val);
            break;
        }

        ret_val = AncFileClose(fd);
        if (ret_val != ANC_OK) {
            ANC_LOGE("fail to close file! returned = %d", ret_val);
            break;
        } else {
            fd = NULL;
        }
    } while (0);

    /* Clean up */
    if (fd != NULL) {
        if (AncFileClose(fd)) {
            ANC_LOGE("Clean Up FAILED: Heap Memory Leaked", ret_val);
        }
    }

    return ret_val;
}

ANC_RETURN_TYPE AncPrintBufferSumValue(const char *p_title, const char *p_buffer, uint32_t buffer_size) {
    uint64_t sum = 0;

    if (buffer_size % 2 == 0) {
        const uint16_t *p_data = (const uint16_t*)p_buffer;
        for (uint32_t i = 0; i < (buffer_size / 2); i++) {
            sum += p_data[i];
        }
    } else {
        for (uint32_t i = 0; i < buffer_size; i++) {
            sum += p_buffer[i];
        }
    }
    ANC_LOGD("%s -- sum = %lld", p_title, sum);
    return ANC_OK;
}

// convert bin to 8 bit bmp
ANC_RETURN_TYPE AncConvertBin2BMP(ImageBin *img_bin, uint8_t *p_bmp_buf, uint32_t bmp_buf_length, uint32_t *p_bmp_size) {
    BitmapHeader *p_header = (BitmapHeader *)p_bmp_buf;
    uint8_t *p_data = p_bmp_buf + sizeof(BitmapHeader);

    if ((img_bin == NULL) || (p_bmp_buf == NULL) || (bmp_buf_length == 0) || (p_bmp_size == NULL)) {
        ANC_LOGE("parameter error:"
                "img_bin: %p, p_bmp_buf: %p, bmp_buf_length: %d, p_bmp_size: %p",
                img_bin, p_bmp_buf, bmp_buf_length, p_bmp_size);
        return ANC_FAIL;
    }

    memset(p_bmp_buf, 0, bmp_buf_length);

    uint8_t* p_bin_buf = img_bin->p_data;
    uint32_t img_size = (uint32_t)(img_bin->width * img_bin->height);
    uint32_t img_byte = (uint32_t)(img_bin->bit / 8);
    uint32_t total_size = img_size * img_byte + sizeof(BitmapHeader);
    uint32_t valid_max_val = (1 << img_bin->valid_bit) - 1;

    if (bmp_buf_length < total_size) {
        ANC_LOGE("bmp buf size not enough: %d expect: %d", bmp_buf_length, total_size);
        return ANC_FAIL;
    }

    // define the bitmap file header
    p_header->bitmap_file_header.bfType = 0x4D42;
    p_header->bitmap_file_header.bfSize = total_size;
    p_header->bitmap_file_header.bfOffBits = sizeof(BitmapHeader);

    // define the bitmap information header
    p_header->bitmap_info_header.biSize = sizeof(BitmapInfoHeader);
    p_header->bitmap_info_header.biWidth = img_bin->width;
    p_header->bitmap_info_header.biHeight = -img_bin->height;    // image stored from the top to bottom
    p_header->bitmap_info_header.biPlanes = 1;
    p_header->bitmap_info_header.biBitCount = 8;

    // define color table
    for (int i = 0; i < 256; i ++) {
        memset(&(p_header->bmp_color_table[4 * i]), i, 4);
        p_header->bmp_color_table[i * 4 + 3] = 0;
    }

    // write the image data
    if (img_byte == 2) {
        if (valid_max_val == 0) {
            valid_max_val = (1 << BIT_2_VALID_CNT) - 1;
        }
        for (uint32_t i = 0; i < img_size; i++)
            p_data[i] = (uint8_t)(*((uint16_t *)p_bin_buf + i) * 255 / valid_max_val);
    } else if (img_byte == 4) {
        if (valid_max_val == 0) {
            valid_max_val = (1 << BIT_4_VALID_CNT) - 1;
        }
        for (uint32_t i = 0; i < img_size; i++)
            p_data[i] = (uint8_t)(*((uint32_t *)p_bin_buf + i) * 255 / valid_max_val);
    }

    *p_bmp_size = total_size;

    return ANC_OK;
}