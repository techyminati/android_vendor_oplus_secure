#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "plat_file.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "plat_time.h"
#include "response_def.h"
#include "save_image.h"
#include "save_image_v2.h"
#include "struct_def.h"
#include "type_definition.h"

#define LOG_TAG "RBS-SAVE-IMAGE"

#define IMG_SAVE_ROOT_JNI "/sdcard/temp/egis_image_db"
#define IMG_SAVE_ROOT "/data/vendor/optical_fingerprint/egis_image_db"
static char img_save_root[PATH_MAX];

#define mkdir_7(path) mkdir(path, 0777)

#define SAVE_IMG_MAX_COUNT 10000
#define SAVE_IMG_BUFF_MAX_SIZE 162 * 1024

#define MAX_PATH 256
#define MAX_FINGER 10
#define MAX_TRY_COUNT 16
#define MAX_TYPE 3

char g_userpath[MAX_PATH] = "Null";
char g_username[MAX_PATH] = "00000000";
char g_fingerid[MAX_PATH] = "fingerid";
char g_case[MAX_PATH] = "st";
int g_fingerid_num = 0;
int g_index_series = -1;
char g_test_case_id[PATH_MAX] = "P";
extern cache_info_t g_cache_info;
save_image_finger_count_t g_finger_count[MAX_FINGER] = {{0}};
extern fingerprint_enroll_info_t g_enroll_info;
extern fingerprint_verify_info_t g_save_debug_image_verify_info;
extern BOOL g_use_sysunlock_imagetool;

typedef enum {
    SPLICE_FINGER_ID,
    CHECK_ROOT_PATH,
    SPLICE_IMAGE_STATE,
    SPLICE_IMAGE_ANGLE,
    SPLICE_TRY_MATCH_FOLDER,
} splice_file_path_cmd_t;

int save_image(const char* path, void* image, size_t size) {
    int ret;
    const int k_element_total = 1;
    FILE* fp = NULL;

    if (path == NULL || image == NULL || size == 0) {
        egislog_e("path is NULL or image is NULL or size = 0");
        return -1;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        egislog_e("Can not open file");
        ret = FINGERPRINT_RES_OPEN_FILE_FAILED;
        goto exit;
    }

    ret = ((fwrite(image, size, k_element_total, fp)) == k_element_total)
              ? FINGERPRINT_RES_SUCCESS
              : FINGERPRINT_RES_OPEN_FILE_FAILED;
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);

exit:
    return ret;
}

static void mkdirs(const char* muldir) {
    int i = 0, len = 0;
    char str[PATH_MAX] = {0};
    strncpy(str, muldir, PATH_MAX);
    len = strlen(str);
    for (i = 1; i < len; i++) {
        if (str[i] == '/') {
            str[i] = '\0';
            if (access(str, 0) != 0 && mkdir_7(str) != 0) {
                egislog_e("Failed to create dir %s", str);
            }
            str[i] = '/';
        }
    }
    if (len > 0 && access(str, 0) != 0) {
        mkdir_7(str);
    }
    return;
}

inline static void getBitPath(rbs_obj_image_v1_0_t* obj_img, char* bitpath) {
    switch (obj_img->imgtype) {
        case 0:
            sprintf(bitpath, "image_bin");
            break;
        case 1:
            sprintf(bitpath, "image_raw");
            break;
        case 2:
            sprintf(bitpath, "image_bkg");
            break;
        case 3:
            sprintf(bitpath, "image_detect");
            break;
        default:
            sprintf(bitpath, "image_unknow");
            break;
    }
}

inline static void getBitName(rbs_obj_image_v1_0_t* obj_img, char* bitname) {
    switch (obj_img->imgtype) {
        case 0:
            sprintf(bitname, "bin_");
            break;
        case 1:
            sprintf(bitname, "raw_");
            break;
        case 2:
            sprintf(bitname, "bkg_");
            break;
        case 3:
            sprintf(bitname, "detect_");
            break;
        default:
            sprintf(bitname, "unknow_");
            break;
    }
}

inline static void TimeStamp(save_image_info_t image_info, char* time_info) {
    time_t timep;
    struct tm* p;
    int hh = 0;
    int mi = 0;
    int ss = 0;
    uint64_t sec;
    uint64_t msec;

    time(&timep);
    p = localtime(&timep);

    msec = image_info.rbs_obj_image->ta_time_ms;
    sec = msec / 1000;
    msec = msec % 1000;
    hh = sec / (60 * 60) % 24;
    mi = (sec / 60) % 60;
    ss = sec % 60;
    sprintf(time_info, "%02d%02d%02d%02d%02d%03lu", p->tm_mon + 1, p->tm_mday, hh, mi, ss, msec);
}

inline static void MatchCase(save_image_info_t image_info, char* match_case) {
    if (image_info.rbs_obj_image->matched_id == 0) {
        sprintf(match_case, "_F");
    } else {
        sprintf(match_case, "_P");
    }
}

void finger_count(save_image_info_t image_info) {
    if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2 && image_info.rbs_obj_image->imgtype == 0) {
        g_finger_count[g_fingerid_num].total_count++;
        if (image_info.rbs_obj_image->matched_id == 0) {
            g_finger_count[g_fingerid_num].not_match_count++;
        } else {
            g_finger_count[g_fingerid_num].match_count++;
        }
    }
}

inline static void EnrollVerifyPath(save_image_info_t image_info, char* enroll_verify_path) {
    if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
        sprintf(enroll_verify_path, "enroll");
    } else if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
        sprintf(enroll_verify_path, "verify");
    }
}

inline static void TotalExposureTime(save_image_info_t image_info, char* save_expo) {
    uint16_t total_expo;
    total_expo =
        image_info.rbs_obj_image->exposure_x10 * image_info.rbs_obj_image->hw_integrate_count;
    total_expo = total_expo / 10;
    sprintf(save_expo, "%d", total_expo);
}

inline static void getEnrollVerifyFileName(save_image_info_t image_info, char* enroll_verify_path,
                                           char* file_name) {
    char match_case[PATH_MAX] = {0};
    char expo[PATH_MAX] = {0};
    char time_info[PATH_MAX] = {0};
    char bit_mane[PATH_MAX] = {0};
    char* ppos = file_name;
    getBitName(image_info.rbs_obj_image, bit_mane);
    TimeStamp(image_info, time_info);
    MatchCase(image_info, match_case);

    sprintf(expo, "%d", image_info.rbs_obj_image->exposure_x10 / 10);

    if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
        sprintf(enroll_verify_path, "enroll/%u/%u", g_enroll_info.fingerprint_info.user_id,
                g_enroll_info.fingerprint_info.fingerprint_id);
        sprintf(ppos, "%s%s_%04d_%u_%010u_TRY_%d_EXP_%sx%d", bit_mane, time_info,
                image_info.rbs_obj_image->index_fingeron, g_enroll_info.fingerprint_info.user_id,
                g_enroll_info.fingerprint_info.fingerprint_id,
                image_info.rbs_obj_array->index_start, expo,
                image_info.rbs_obj_image->hw_integrate_count);
        ppos = file_name + strlen(file_name);
#ifndef ALGO_GEN_4
        sprintf(ppos, "_BTP_%d_POL_%d_QTY_%d_DQE_%d", image_info.rbs_obj_image->temperature,
                image_info.rbs_obj_image->bds_pool_add, image_info.rbs_obj_image->extract_qty,
                file_name, image_info.rbs_obj_image->is_dqe_enroll);
        ppos = file_name + strlen(file_name);
#else
        sprintf(ppos, "_DB_%d", image_info.rbs_obj_image->is_update_debase);
        ppos = file_name + strlen(file_name);
#endif
        sprintf(ppos, "_ENR_%d", image_info.rbs_obj_image->is_in_algo);
        ppos = file_name + strlen(file_name);

        if (g_enroll_info.is_redundant) {
            sprintf(ppos, "_RD");
            ppos = file_name + strlen(file_name);
        }

        if (image_info.rbs_obj_image->is_duplicate_finger) {
            sprintf(ppos, "_DC");
            ppos = file_name + strlen(file_name);
        }

        if (image_info.rbs_obj_image->is_reject_retry) {
            sprintf(ppos, "_REJECT-RETRY");
            ppos = file_name + strlen(file_name);
        }

    } else if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
        sprintf(enroll_verify_path, "verify/%u", g_save_debug_image_verify_info.user_id);
        sprintf(ppos, "%s%s_%04d_%u_%010u%s_TRY_%d_EXP_%sx%d", bit_mane, time_info,
                image_info.rbs_obj_image->index_fingeron, g_save_debug_image_verify_info.user_id,
                image_info.rbs_obj_image->matched_id, match_case,
                image_info.rbs_obj_array->index_start, expo,
                image_info.rbs_obj_image->hw_integrate_count);
        ppos = file_name + strlen(file_name);

#ifndef ALGO_GEN_4
        sprintf(ppos, "_BTP_%d_POL_%d_QTY_%d", image_info.rbs_obj_image->temperature,
                image_info.rbs_obj_image->bds_pool_add, image_info.rbs_obj_image->extract_qty);
#else
        sprintf(ppos, "_DB_%d", image_info.rbs_obj_image->is_update_debase);
#endif
        ppos = file_name + strlen(file_name);
        if (image_info.rbs_obj_image->matched_id != 0) {
            sprintf(ppos, "_LN_%d", image_info.rbs_obj_image->is_learning);
            ppos = file_name + strlen(file_name);
        }

        sprintf(ppos, "_AUTH_%d", image_info.rbs_obj_image->is_in_algo);
        ppos = file_name + strlen(file_name);

        if (image_info.rbs_obj_image->is_in_algo) {
            if (image_info.rbs_obj_image->matched_id == 0) {
                sprintf(ppos, "_NOTMATCH");
                ppos = file_name + strlen(file_name);
            }
            sprintf(ppos, "_MS_%d", image_info.rbs_obj_image->match_score);
            ppos = file_name + strlen(file_name);
#ifndef ALGO_GEN_4
            if (image_info.rbs_obj_image->extract_qty <
                g_save_debug_image_verify_info.bad_qty_threshold) {
                sprintf(ppos, "_QL");
                ppos = file_name + strlen(file_name);
            }
#endif
        }
        if (image_info.rbs_obj_image->is_last_match_failed) {
            sprintf(ppos, "_MFM");
            ppos = file_name + strlen(file_name);
        }
    }
    // egislog_d("%s black_edge = %d", __func__, image_info.rbs_obj_image->black_edge);

    if (image_info.rbs_obj_image->black_edge > 0) {
        sprintf(ppos, "_BLACKEDGE");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.rbs_obj_image->is_residual) {
        sprintf(ppos, "_RESIDUAL");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_finger_off_0) {
        sprintf(ppos, "_FINGEROFF_0");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_finger_off_1) {
        sprintf(file_name, "%s_FINGEROFF_1", file_name);
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_called_cancel) {
        sprintf(ppos, "_CANCEL");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.rbs_obj_image->is_get_image_vibration) {
        sprintf(ppos, "_VIBRATION");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.rbs_obj_image->is_scratch) {
        sprintf(ppos, "_SCRATCH");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_bad_image_fingeroff1) {
        sprintf(file_name, "%s_BADIMAGE_FINGEROFF_1", file_name);
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_partial_fingeroff1) {
        sprintf(file_name, "%s_PARTIAL_FINGEROFF_1", file_name);
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_bad_image) {
        sprintf(file_name, "%s_BADIMAGE", file_name);
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_partial) {
        sprintf(file_name, "%s_PARTIAL", file_name);
        ppos = file_name + strlen(file_name);
    }

    if (image_info.is_image_too_fast) {
        sprintf(file_name, "%s_TOO_FAST", file_name);
        ppos = file_name + strlen(file_name);
    }

    egislog_e("image_info.rbs_obj_image->fake_score = %d", image_info.rbs_obj_image->fake_score);
    if (image_info.rbs_obj_image->fake_score >= 50) {
        sprintf(ppos, "_FAKE_1");
        ppos = file_name + strlen(file_name);
    } else {
        sprintf(ppos, "_FAKE_0");
        ppos = file_name + strlen(file_name);
    }

    if (image_info.rbs_obj_image->qty >= 50) {
        sprintf(ppos, "_QTY_1");
        ppos = file_name + strlen(file_name);
    } else {
        sprintf(ppos, "_QTY_0");
        ppos = file_name + strlen(file_name);
    }

    sprintf(ppos,"_CLS_%d",image_info.rbs_obj_image->cls_score);
    ppos = file_name + strlen(file_name);
    /*if (image_info.rbs_obj_image->seg >= 50) {
        sprintf(ppos, "_SEG_1");
        ppos = file_name + strlen(file_name);
    } else {
        sprintf(ppos, "_SEG_0");
        ppos = file_name + strlen(file_name);
    }*/

    //TODO:
    sprintf(ppos, "_QID_%d",image_info.rbs_obj_image->status_score);
    ppos = file_name + strlen(file_name);

    sprintf(ppos, "_TID_%d",image_info.rbs_obj_image->strange_score);
    ppos = file_name + strlen(file_name);

    if (image_info.rbs_obj_image->is_last_try) {
        sprintf(ppos, "_LAST");
        ppos = file_name + strlen(file_name);
    }
}

extern BOOL g_app_is_using_jni;
void debug_save_image(save_image_info_t image_info) {
    egislog_d("%s start", __func__);

    if (g_app_is_using_jni) {
        strcpy(img_save_root, IMG_SAVE_ROOT_JNI);
    } else {
        strcpy(img_save_root, IMG_SAVE_ROOT);
    }

    char save_path[PATH_MAX] = {0};
    char file_name[PATH_MAX] = {0};
    char complete_file_name[PATH_MAX] = {0};
    char bit_path[PATH_MAX] = {0};
    char enroll_verify_path[PATH_MAX] = {0};

    getBitPath(image_info.rbs_obj_image, bit_path);
    getEnrollVerifyFileName(image_info, enroll_verify_path, file_name);

    if (g_use_sysunlock_imagetool) {
        EnrollVerifyPath(image_info, enroll_verify_path);
        sprintf(save_path, "%s/%s/%s/%s/%s/%s/%s", img_save_root, bit_path, g_test_case_id,
                g_username, g_fingerid, enroll_verify_path, g_case);
        if (image_info.rbs_obj_array->has_more_object == 0) {
            finger_count(image_info);
        }
    } else {
        sprintf(save_path, "%s/%s/%s", img_save_root, bit_path, enroll_verify_path);
    }

    sprintf(complete_file_name, "%s/%s.bin", save_path, file_name);
    mkdirs(save_path);

    int bpp = image_info.rbs_obj_image->bpp;

    egislog_i("save_image : %s", complete_file_name);
    if (bpp == 16) {
        save_img16_BigEndian(complete_file_name,
                             (uint16_t*)RBSOBJ_get_payload_pointer(image_info.rbs_obj_image),
                             image_info.rbs_obj_image->width, image_info.rbs_obj_image->height);
    } else {
        save_image(complete_file_name, RBSOBJ_get_payload_pointer(image_info.rbs_obj_image),
                   RBSOBJ_get_payload_size(image_info.rbs_obj_image));
    }

    egislog_i("%s end", __func__);
    return;
}

static int egis_normalize(unsigned char* dst, int* img16, int width, int height) {
    int min, max;
    int i, j, c, diff;
    int w_ROI = width < 128 ? width : 128, h_ROI = height < 128 ? height : 128;
    int x0 = (width - w_ROI) / 2, y0 = (height - h_ROI) / 2;
    int* img = img16;
    unsigned short* hist;
    if (dst == NULL || img16 == NULL) return 0;
    hist = calloc(65536, sizeof(short));
    if (hist == NULL) return 0;
    img += y0 * width + x0;
    for (i = 0; i < h_ROI; i++, img += width) {
        for (j = 0; j < w_ROI; j++) {
            hist[img[j] & 0xFFFF]++;
        }
    }
    max = 0xFFFF;
    min = 0;
    for (i = 0, c = 0; i < 65536 && c >= 0; i++) c -= hist[i];
    min = i - 1;
    for (i = 0xFFFF, c = 0; i >= 0 && c >= 0; i--) c -= hist[i];
    max = i + 1;
    free((void*)hist);
    diff = max - min;
    if (diff > 0) {
        for (i = 0; i < width * height; i++) {
            dst[i] = (img16[i] < min ? 0 : img16[i] > max ? 255 : (img16[i] - min) * 255 / diff);
        }
    } else {
        return 0;
    }

    return 1;
}

void save_img8_normalize(char* path, uint16_t* raw_img16, int width, int height) {
    int i;
    int* img16 = NULL;
    uint8_t* output_img = NULL;

    img16 = (int*)malloc(width * height * sizeof(int));
    output_img = (uint8_t*)malloc(width * height * sizeof(uint8_t));

    for (i = 0; i < (width * height); i++) {
        img16[i] = raw_img16[i];
    }
    egis_normalize(output_img, img16, width, height);
    plat_save_file(path, output_img, width * height * sizeof(uint16_t));

    if (NULL != img16) {
        free(img16);
        img16 = NULL;
    }
    if (NULL != output_img) {
        free(output_img);
        output_img = NULL;
    }
}

inline static void TransFingerIDnum(char* fingerid) {
    g_fingerid_num = atoi(fingerid);
}

int get_total_count() {
    return g_finger_count[g_fingerid_num].total_count;
}

int get_match_count() {
    return g_finger_count[g_fingerid_num].match_count;
}

int get_not_match_count() {
    return g_finger_count[g_fingerid_num].not_match_count;
}

void reset_finger_count() {
    TransFingerIDnum(g_fingerid);
    g_finger_count[g_fingerid_num].total_count = 0;
    g_finger_count[g_fingerid_num].match_count = 0;
    g_finger_count[g_fingerid_num].not_match_count = 0;
}
