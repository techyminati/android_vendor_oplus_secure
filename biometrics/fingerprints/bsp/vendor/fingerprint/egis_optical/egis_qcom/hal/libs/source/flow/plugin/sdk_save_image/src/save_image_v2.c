#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#include "plat_log.h"
#include "plat_mem.h"
#include "plat_time.h"
#include "plat_file.h"
#include "response_def.h"
#include "type_definition.h"
#include "struct_def.h"
#include "save_image_v2.h"

#ifndef _WINDOWS
#include <dirent.h>
#include <unistd.h>
#else
#include <direct.h>
#include <io.h>
#define access _access
#endif

#if defined(WIN32) && !defined(__cplusplus)

#define inline __inline

#endif

#define LOG_TAG "RBS-SAVE-IMAGE"

#define IMG_SAVE_PATH "/data/vendor/optical_fingerprint/egis_image_db"

#define mkdir_7(path) mkdir(path, 0777)

#define SAVE_IMG_MAX_COUNT 10000
#define SAVE_IMG_BUFF_MAX_SIZE 162 * 1024

#define MAX_PATH 256
#define MAX_FINGER 10
#define MAX_TRY_COUNT 16
#define MAX_TYPE 3

char g_userpath[MAX_PATH] = "Null";
char g_username[MAX_PATH] = "00000000";
char g_fingerid[MAX_PATH] = "0";
int g_fingerid_num = 0;
char g_case[MAX_PATH] = "st";
char g_test_case_id[PATH_MAX] = "P";
int g_index_series = -1;
unsigned long long g_timestamp = 0;

extern cache_info_t g_cache_info;

save_image_finger_count_t g_finger_count[MAX_FINGER] = {{0}};
char g_try_match_path[MAX_TYPE][MAX_TRY_COUNT][MAX_PATH] = {{{0}}};
char g_try_match_filename[MAX_TRY_COUNT][MAX_PATH] = {{0}};

typedef enum {
	SPLICE_FINGER_ID,
	CHECK_ROOT_PATH,
	SPLICE_IMAGE_STATE,
	SPLICE_IMAGE_ANGLE,
	SPLICE_TRY_MATCH_FOLDER,
} splice_file_path_cmd_t;

static void TimeStamp(save_image_info_t image_info);

int save_image(const char *path, void *image, size_t size)
{
	int ret;
	const int k_element_total = 1;
	FILE *fp = NULL;

	egislog_d("save_image : %s", path);

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
#ifndef _WINDOWS
	fflush(fp);
	fsync(fileno(fp));
#endif
	fclose(fp);
	
exit:
	return ret;
}

static void mkdirs(const char *muldir)
{
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
#ifndef _WINDOWS
		mkdir_7(str);
#endif
	}
	return;
}

inline static void getBitPath(save_image_info_t image_info, char* bitpath) {
	switch(image_info.rbs_obj_image->imgtype) {
		case IMGTYPE_8BIT:
			TimeStamp(image_info);
			sprintf(bitpath, "image_bin");
			break;
		case IMGTYPE_RAW:
			sprintf(bitpath, "image_raw");
			break;
		case IMGTYPE_BKG:
			sprintf(bitpath, "image_bkg");
			break;
		default :
			sprintf(bitpath, "image_unknow");
			break;
	}
}

inline static void EnrollVerifyPath(save_image_info_t image_info, char* enroll_verify_path) {
	if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
		sprintf(enroll_verify_path, "enroll");
	} else if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
		sprintf(enroll_verify_path, "verify");
	}
}

#define SAVEIMAGE_MERGE_ENROLL_FEATURE_LOW -2
#define SAVEIMAGE_MERGE_ENROLL_REDUNDANT_INPUT -8
#define SAVEIMAGE_MERGE_ENROLL_LOW_QTY -10
#define SAVEIMAGE_DUPLICATE -1004


inline static void MatchCase(save_image_info_t image_info, char* match_case) {



	if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
		if (image_info.img_state == SAVE_BAD) {
			if (image_info.rbs_obj_image->try_match_result == VERIFY_TRY_MATCH_RESULT_MATCH) {
				sprintf(match_case, "_P");
			} else {
				sprintf(match_case, "_B");
			}
		} else {
			if (image_info.rbs_obj_image->try_match_result == VERIFY_TRY_MATCH_RESULT_MATCH) {
				sprintf(match_case, "_P");
			} else {
				sprintf(match_case, "_F");
			}
		}

	} else {
		if (image_info.rbs_obj_image->try_match_result == VERIFY_TRY_MATCH_RESULT_NOT_MATCH) {
			sprintf(match_case, "_TRY");
			return;
		}
		if (image_info.rbs_obj_image->is_bad == 1) {
			sprintf(match_case, "_B");
		} else {
			switch (image_info.rbs_obj_image->algo_flag) {
				case SAVEIMAGE_MERGE_ENROLL_REDUNDANT_INPUT:
					sprintf(match_case, "_RD");
					break;
				case SAVEIMAGE_MERGE_ENROLL_LOW_QTY:
					sprintf(match_case, "_LQ");
					break;
				case SAVEIMAGE_DUPLICATE:
					sprintf(match_case, "_DP");
					break;
				case SAVEIMAGE_MERGE_ENROLL_FEATURE_LOW:
					sprintf(match_case, "_FEATLOW");
					break;
				default:
					sprintf(match_case, "_OK");
					break;
			}
		}
	}
}

static void TimeStamp(save_image_info_t image_info) {
	if (image_info.rbs_obj_image->index_series != g_index_series)
		g_timestamp = plat_get_time();
	g_index_series = image_info.rbs_obj_image->index_series;
}

inline static void MatchType(save_image_info_t image_info, char* match_type) {
	
	switch (image_info.rbs_obj_image->match_type) {
		case VERIFY_TYPE_NORMAL_MATCH:
			sprintf(match_type, "NM");
		break;
		case VERIFY_TYPE_QUICK_MATCH:
			sprintf(match_type, "QM");
		break;
		case VERIFY_TYPE_LQM_MATCH:
			sprintf(match_type, "LQM");
		break;
		default:
			sprintf(match_type, "NA");
		break;
	}
}

inline static void TransFingerIDnum(char* fingerid) {
	g_fingerid_num = atoi(fingerid);
}

void finger_count(save_image_info_t image_info) {
	if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2 && image_info.rbs_obj_image->imgtype == 0) {
		g_finger_count[g_fingerid_num].total_count++;
		if (image_info.img_state == SAVE_GOOD_MATCH)
			g_finger_count[g_fingerid_num].match_count++;
		else if (image_info.img_state == SAVE_GOOD_NOT_MATCH)
			g_finger_count[g_fingerid_num].not_match_count++;
	}
}

void move_image_try(save_image_info_t image_info) {
	char image_try_path[PATH_MAX] = {0};
	char old_image_try_file_name[PATH_MAX] = {0};
	char new_image_try_file_name[PATH_MAX] = {0};
	char bit_path[PATH_MAX] = {0};
	if (image_info.rbs_obj_image->index_try_match > 0 && image_info.rbs_obj_image->imgtype != IMGTYPE_BKG) {
		getBitPath(image_info, bit_path);
		sprintf(image_try_path, "%s/image_try/%s", IMG_SAVE_PATH, bit_path);
		sprintf(image_try_path, "%s/%llu_%s", image_try_path, g_timestamp, g_username);
		mkdirs(image_try_path);
		for (int idx = 0; idx < image_info.rbs_obj_image->index_try_match; idx++) {
			sprintf(old_image_try_file_name, "%s/%s", g_try_match_path[image_info.rbs_obj_image->imgtype][idx], g_try_match_filename[idx]);
			sprintf(new_image_try_file_name, "%s/%s", image_try_path, g_try_match_filename[idx]);
			egislog_d("new_try_path %s", new_image_try_file_name);
			rename(old_image_try_file_name, new_image_try_file_name);
			remove(g_try_match_path[image_info.rbs_obj_image->imgtype][idx]);
		}
	}
}

void debug_save_image(save_image_info_t image_info)
{
	egislog_d("%s start", __func__);

	char save_path[PATH_MAX] = {0};
	char file_name[PATH_MAX] = {0};
	char complete_file_name[PATH_MAX] = {0};
	char bit_path[PATH_MAX] = {0};
	char enroll_verify_path[PATH_MAX] = {0};
	char match_case[PATH_MAX] = {0};
	char match_type[PATH_MAX] = {0};
	double exposure = (double)image_info.rbs_obj_image->exposure_x10/10;

	getBitPath(image_info, bit_path);
	EnrollVerifyPath(image_info, enroll_verify_path);
	MatchCase(image_info, match_case);
	MatchType(image_info, match_type);

	sprintf(save_path, "%s/%s/%s/%s/%s/%s/%s", IMG_SAVE_PATH, bit_path, g_test_case_id, g_username, g_fingerid, enroll_verify_path, g_case);

	if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
		sprintf(file_name, "%llu_%s_%s_%04d%s_%02d_%d_PQ_%d_FAKE_%d_%gx%d_BTP_%d_MLP_%d_MLL_%d_BDS_%d_POL_%02d_eqty_%d_%s_LRN_%02d_%d_MLE_%d.bin", 
			g_timestamp,
			g_username, 
			g_fingerid, 
			image_info.rbs_obj_image->index_fingeron, 
			match_case,
			image_info.rbs_obj_image->index_try_match,
			image_info.rbs_obj_image->match_score,
			image_info.rbs_obj_image->g2_partial,
			image_info.rbs_obj_image->fake_score,
			exposure,
			image_info.rbs_obj_image->hw_integrate_count,
			image_info.rbs_obj_image->temperature,
			image_info.rbs_obj_image->partial,
			image_info.rbs_obj_image->is_light,
			image_info.rbs_obj_image->bds_debug_path,
			image_info.rbs_obj_image->bds_pool_add,
			image_info.rbs_obj_image->extract_qty,
			match_type,
			image_info.rbs_obj_image->is_learning,
			image_info.rbs_obj_image->sw_integrate_count,
			image_info.rbs_obj_image->black_edge);
		if ((image_info.rbs_obj_image->try_match_result != VERIFY_TRY_MATCH_RESULT_MATCH) && (image_info.rbs_obj_array->has_more_object == 1)) {
			sprintf(save_path, "%s/%s_%s_%04d", save_path, 
				g_username, 
				g_fingerid, 
				image_info.rbs_obj_image->index_fingeron);
			sprintf(g_try_match_path[image_info.rbs_obj_image->imgtype][image_info.rbs_obj_image->index_try_match], "%s", save_path);
			sprintf(g_try_match_filename[image_info.rbs_obj_image->index_try_match], "%s", file_name);
		}
		else {
			finger_count(image_info);
			move_image_try(image_info);
		}
	} 
	else {
		sprintf(file_name, "%llu_%s_%s_%04d%s_%02d_%gx%d_BTP_%d_BDS_%d_POL_%02d_%d_MLE_%d.bin", 
			g_timestamp,
			g_username, 
			g_fingerid, 
			image_info.rbs_obj_image->index_fingeron, 
			match_case,
			image_info.rbs_obj_image->index_try_match,
			exposure,
			image_info.rbs_obj_image->hw_integrate_count,
			image_info.rbs_obj_image->temperature,
			image_info.rbs_obj_image->bds_debug_path,
			image_info.rbs_obj_image->bds_pool_add,
			image_info.rbs_obj_image->sw_integrate_count,
			image_info.rbs_obj_image->black_edge);
		if (image_info.rbs_obj_image->try_match_result == VERIFY_TRY_MATCH_RESULT_NOT_MATCH) {
			sprintf(save_path, "%s/image_try/%s", IMG_SAVE_PATH, bit_path);
			sprintf(g_try_match_path[image_info.rbs_obj_image->imgtype][image_info.rbs_obj_image->index_try_match], "%s", save_path);
			sprintf(g_try_match_filename[image_info.rbs_obj_image->index_try_match], "%s", file_name);
		}
		else {
			move_image_try(image_info);
		}
	}
	if (image_info.rbs_obj_image->imgtype == IMGTYPE_BKG) {
		sprintf(save_path, "%s/%s", IMG_SAVE_PATH, bit_path);
	}
	sprintf(complete_file_name, "%s/%s", save_path, file_name);
	mkdirs(save_path);

	int bpp = image_info.rbs_obj_image->bpp;
	if (bpp == 16) {
		save_img16_BigEndian(complete_file_name, (uint16_t*) RBSOBJ_get_payload_pointer(image_info.rbs_obj_image),
			image_info.rbs_obj_image->width, image_info.rbs_obj_image->height);
	} else {
		save_image(complete_file_name, RBSOBJ_get_payload_pointer(image_info.rbs_obj_image), 
			RBSOBJ_get_payload_size(image_info.rbs_obj_image));
	}
	return;
}

void save_img16_BigEndian(char* path, uint16_t* raw_img16, int width, int height) {
    int i;
    uint8_t* output_img = NULL;

    output_img = (uint8_t*)plat_alloc(width * height * sizeof(uint16_t));

    for (i = 0; i < (width * height); i++) {
        output_img[2 * i] = (raw_img16[i] >> 8) & 0xFF;
        output_img[2 * i + 1] = (raw_img16[i]) & 0xFF;
    }
    plat_save_file(path, output_img, width * height * sizeof(uint16_t));
    plat_free(output_img);
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
