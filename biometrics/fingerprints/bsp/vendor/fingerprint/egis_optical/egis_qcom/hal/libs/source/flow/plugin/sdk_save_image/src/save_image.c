#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#include "egis_log.h"
#include "plat_heap.h"
#include "opt_file.h"
#include "response_def.h"
#include "type_definition.h"
#include "struct_def.h"
#include "save_image.h"

#define LOG_TAG "RBS-SAVE-IMAGE"

#define IMG_SAVE_PATH "/data/egis_image_db"

#define mkdir_7(path) mkdir(path, 0777)

#define SAVE_IMG_MAX_COUNT 10000
#define SAVE_IMG_BUFF_MAX_SIZE 162 * 1024

#define MAX_PATH 256

char g_userpath[MAX_PATH] = "Null";
char g_username[MAX_PATH] = "username";
char g_fingerid[MAX_PATH] = "fingerid";
char g_case[MAX_PATH] = "st";

extern cache_info_t g_cache_info;

typedef enum {
	SPLICE_FINGER_ID,
	CHECK_ROOT_PATH,
	SPLICE_IMAGE_STATE,
	SPLICE_IMAGE_ANGLE,
	SPLICE_TRY_MATCH_FOLDER,
} splice_file_path_cmd_t;

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
	fflush(fp);
	fsync(fileno(fp));
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
		mkdir_7(str);
	}
	return;
}

inline static void mk_check_create_dir(const char *path)
{
	struct stat st;
	egislog_v("%s", path);
	if (stat(path, &st) == -1) {
		if (mkdir_7(path) != 0) {
			egislog_e("Failed to create dir %s", path);
		}
	}
}

inline static void mk_root_path(char *root_path, BOOL is_raw_16bit)
{
	if (TRUE == is_raw_16bit) {
		sprintf(root_path, IMG_SAVE_PATH "/16bit/%s/%s", g_username, g_fingerid);
	} else {
		sprintf(root_path, IMG_SAVE_PATH "/8bit/%s/%s", g_username, g_fingerid);
	}

	mkdirs(root_path);
}

static BOOL is_image_info_valid(save_image_info_t image_info)
{
	if (image_info.img_type != TRANSFER_ENROLL_IMAGE && image_info.img_type != TRANSFER_VERIFY_IMAGE_V2) {
		egislog_e("img_type %d invalid", image_info.img_type);
		return FALSE;
	}

	if (image_info.img_data_size <= 0 || image_info.img_data_size > SAVE_IMG_BUFF_MAX_SIZE) {
		egislog_e("img_data_size %d invalid", image_info.img_data_size);
		return FALSE;
	}

	if (image_info.img_cnt <= 0 || image_info.img_data_size * image_info.img_cnt > SAVE_IMG_BUFF_MAX_SIZE) {
		egislog_e("img_cnt %d invalid", image_info.img_cnt);
		return FALSE;
	}

	if (image_info.select_index < 0 || image_info.select_index > image_info.img_cnt - 1) {
		egislog_e("select_index %d invalid", image_info.select_index);
		return FALSE;
	}

	if (image_info.img_buf == NULL) {
		egislog_e("img_buf NULL invalid");
		return FALSE;
	}

	return TRUE;
}

static BOOL is_raw16bit_data(save_image_info_t image_info)
{
	BOOL	retval = FALSE;

	if(image_info.img_data_size == (image_info.img_width * image_info.img_height * 2 * image_info.img_cnt))
		retval = TRUE;

	return retval;
}

static BOOL splice_file_path(char *path, splice_file_path_cmd_t cmd, save_image_info_t image_info)
{
	BOOL retval = TRUE;

	if (path == NULL) {
		egislog_e("path is null");
		return FALSE;
	}

	switch (cmd) {
		case SPLICE_FINGER_ID:
			if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
				sprintf(g_fingerid, "fingerid_%d", g_cache_info.fingerprint_ids_count);
			}
			break;

		case CHECK_ROOT_PATH:
			mk_root_path(path, is_raw16bit_data(image_info));
			mk_check_create_dir(path);
			break;

		case SPLICE_IMAGE_STATE:
			if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
				strcat(path, "/enroll");
			} else if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
				strcat(path, "/verify");
			} else {
				egislog_e("wrong img_type : %d", image_info.img_type);
			}

			if (image_info.img_state == SAVE_BAD) {
				strcat(path, "_bad_img");
			}

			mk_check_create_dir(path);
			break;

		case SPLICE_IMAGE_ANGLE:
			sprintf(path, "%s/%s", path, g_case);
			mk_check_create_dir(path);
			break;

		case SPLICE_TRY_MATCH_FOLDER:
			mk_check_create_dir(path);
			break;

		default:
			egislog_e("error command");
			retval = FALSE;
			break;
	}

	egislog_d("file name : %s", path);
	return retval;
}

static BOOL splice_file_name(char *name, int cnt, transfer_image_type_t img_type, save_image_state_t img_state)
{
	char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	time_t timep;
	struct tm *p;

	if (name == NULL) {
		egislog_e("path is null");
		return FALSE;
	}

	time(&timep);
	p = gmtime(&timep);

	sprintf(name, "%04d", cnt);

	if (img_state == SAVE_BAD) {
		strcat(name, "_B");
	} else if (img_type == TRANSFER_VERIFY_IMAGE_V2 && img_state == SAVE_GOOD_MATCH) {
		strcat(name, "_P");
	} else if (img_type == TRANSFER_VERIFY_IMAGE_V2 && img_state == SAVE_GOOD_NOT_MATCH) {
		strcat(name, "_F");
	}

	sprintf(name, "%s_%s_%dhr_%dm_%ds", name, wday[p->tm_wday], p->tm_hour + 8, p->tm_min, p->tm_sec);
	egislog_d("file name : %s", name);

	return TRUE;
}

void debug_save_image(save_image_info_t image_info)
{
	egislog_d("%s start", __func__);

	char save_path[PATH_MAX] = {0};
	char file_name[PATH_MAX] = {0};
	char save_try_match_path[PATH_MAX] = {0};
	char complete_file_name[PATH_MAX] = {0};
	int i = 0;
	BOOL has_try_math = FALSE;
	static int image_cnt = 0;

	if (!is_image_info_valid(image_info)) {
		egislog_e("wrong image information, rejct");
		return;
	}

	splice_file_path(save_path, SPLICE_FINGER_ID, image_info);
	splice_file_path(save_path, CHECK_ROOT_PATH, image_info);
	splice_file_path(save_path, SPLICE_IMAGE_STATE, image_info);
	splice_file_path(save_path, SPLICE_IMAGE_ANGLE, image_info);

	splice_file_name(file_name, image_cnt, image_info.img_type, image_info.img_state);

	if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2 && image_info.img_cnt > 1 && image_info.img_state != SAVE_BAD) {
		sprintf(save_try_match_path, "%s/%s", save_path, file_name);
		splice_file_path(save_try_match_path, SPLICE_TRY_MATCH_FOLDER, image_info);
		has_try_math = TRUE;
	}

	for (i = 0; i < image_info.img_cnt; i++) {
		if (image_info.img_state == SAVE_BAD && i != image_info.select_index) {
			continue;
		}

		if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
			sprintf(complete_file_name, "%s/%s.bin", save_path, file_name);
		} else if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
			if ((has_try_math && i == image_info.select_index) || !has_try_math) {
				sprintf(complete_file_name, "%s/%s.bin", save_path, file_name);
			} else {
				sprintf(complete_file_name, "%s/%s_%02d.bin", save_try_match_path, file_name, i);
			}
		}

		int image_size = image_info.img_width * image_info.img_height;
		if(TRUE == is_raw16bit_data(image_info)) {
			image_size = image_size * 2;
		}

		save_image(complete_file_name, image_info.img_buf + i * image_size, image_size);
	}

	if(image_info.is_new_finger_on == TRUE) {
		image_cnt++;
	}

	return;
}