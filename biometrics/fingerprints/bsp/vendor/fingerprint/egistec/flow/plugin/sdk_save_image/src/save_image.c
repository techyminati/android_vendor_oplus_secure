#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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

static unsigned int g_enroll_count = 0;
static unsigned int g_verify_count = 0;
static unsigned int g_enroll_finger_off_count = 0;
static unsigned int g_verify_finger_off_count = 0;
static unsigned int g_finger_catch = 0;

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
	char str[MAX_PATH] = {0};
	strncpy(str, muldir, 256);
	len = strlen(str);
	for (i = 0; i < len; i++) {
		if (str[i] == '/') {
			str[i] = '\0';
			if (access(str, 0) != 0) {
				egislog_e("no directory, create %s", str);
				mkdir_7(str);
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
	egislog_d("%s", path);
	if (stat(path, &st) == -1) {
		egislog_e("no directory, create %s", path);
		mkdir_7(path);
	}
}

inline static void mk_root_path(char *root_path)
{
	sprintf(root_path, IMG_SAVE_PATH "/%s/%s", g_username, g_fingerid);
	mkdirs(root_path);
}

static BOOL is_image_info_valid(save_image_info_t image_info)
{
	if (image_info.img_type != TRANSFER_ENROLL_IMAGE && image_info.img_type != TRANSFER_VERIFY_IMAGE_V2 && image_info.img_type != TRANSFER_ENROLL_FINGER_LOST_IMAGE
		&& image_info.img_type != TRANSFER_VERIFY_FINGER_LOST_IMAGE  && image_info.img_type != TRANSFER_LIVE_IMAGE) {
		egislog_e("img_type %d invalid", image_info.img_type);
		return FALSE;
	}

	if (image_info.single_img_size <= 0 || image_info.single_img_size > SAVE_IMG_BUFF_MAX_SIZE) {
		egislog_e("single_img_size %d invalid", image_info.single_img_size);
		return FALSE;
	}

	if (image_info.img_cnt <= 0 || image_info.single_img_size * image_info.img_cnt > SAVE_IMG_BUFF_MAX_SIZE) {
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
			mk_root_path(path);
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

	char save_path[MAX_PATH] = {0};
	char file_name[MAX_PATH] = {0};
	char save_try_match_path[MAX_PATH] = {0};
	char complete_file_name[MAX_PATH] = {0};
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

		save_image(complete_file_name, image_info.img_buf + i * image_info.single_img_size, image_info.single_img_size);
	}

	image_cnt++;
	return;
}

static void get_filename_from_local_times(char *filename, save_image_info_t image_info, char *match_info) 
{
	char local_time[MAX_PATH] = {0};
	time_t now;
	struct tm *s_tm;
	time(&now);
	s_tm = localtime(&now);
	egislog_d("%4d-%02d-%02d %02d:%02d:%02d", s_tm->tm_year + 1900, s_tm->tm_mon+1, s_tm->tm_mday, s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
	
	sprintf(local_time, "%4d%02d%02d_%02d%02d%02d", s_tm->tm_year + 1900, s_tm->tm_mon+1, s_tm->tm_mday, s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);

	if (image_info.img_type == TRANSFER_ENROLL_IMAGE) {
		sprintf(filename, "/data/images/egis_enroll_%s_%06d.bin", local_time, g_enroll_count);
		g_enroll_count++;
	} else if (image_info.img_type == TRANSFER_VERIFY_IMAGE_V2) {
		sprintf(filename, "/data/images/egis_verify_%s_%06d_%s.bin", local_time, g_verify_count, match_info);
		g_verify_count++;
	} else if (image_info.img_type == TRANSFER_ENROLL_FINGER_LOST_IMAGE) {
		sprintf(filename, "/data/images/finger_off/egis_enroll_finger_off_%s_%06d_%s.bin", local_time, g_enroll_finger_off_count, match_info);
		g_enroll_finger_off_count++;
	} else if (image_info.img_type == TRANSFER_VERIFY_FINGER_LOST_IMAGE) {
		sprintf(filename, "/data/images/finger_off/egis_verify_finger_off_%s_%06d_%s.bin", local_time, g_verify_finger_off_count, match_info);
		g_verify_finger_off_count++;
	} else if (image_info.img_type == TRANSFER_LIVE_IMAGE) {
		sprintf(filename, "/data/images/finger_catch/egis_%s_%06d_%s.bin", local_time,g_finger_catch, match_info);
		g_finger_catch++;
	}
}

void debug_save_image_oplus_k4(save_image_info_t image_info, char *match_info)
{
	int i = 0;
	char file_name[MAX_PATH] = {0};

	if (!is_image_info_valid(image_info)) {
		egislog_e("wrong image information, rejct");
		return;
	}

	if (image_info.img_type == TRANSFER_ENROLL_FINGER_LOST_IMAGE || image_info.img_type == TRANSFER_VERIFY_FINGER_LOST_IMAGE) {
		mk_check_create_dir("/data/images/finger_off");
	} else if (image_info.img_type == TRANSFER_LIVE_IMAGE) {
		mk_check_create_dir("/data/images/finger_catch");
	} else {
		mk_check_create_dir("/data/images");
	}

	egislog_d("image_info.img_state:%d, image_info.select_index:%d, img_cnt %d", image_info.img_state, image_info.select_index, image_info.img_cnt);

	for (i = 0; i < image_info.img_cnt; i++) {
		if ((image_info.img_type != TRANSFER_ENROLL_FINGER_LOST_IMAGE && image_info.img_type != TRANSFER_VERIFY_FINGER_LOST_IMAGE &&
			image_info.img_type != TRANSFER_LIVE_IMAGE) && i != image_info.select_index) {
			egislog_d("do not save img, continue! image_info.img_state:%d, image_info.select_index:%d, i:%d", image_info.img_state, image_info.select_index, i);
			continue;
		}

		if (image_info.img_type == TRANSFER_ENROLL_FINGER_LOST_IMAGE || image_info.img_type == TRANSFER_VERIFY_FINGER_LOST_IMAGE){
			if (i != image_info.img_cnt - 1)
				get_filename_from_local_times(file_name, image_info, "finger_touch");
			else
				get_filename_from_local_times(file_name, image_info, match_info);
		}
		else
			get_filename_from_local_times(file_name, image_info, match_info);

		//get_filename_from_local_times(file_name, image_info, match_info);

		egislog_d("EGIS_SAVE_IMAGE filename %d :%s",i, file_name);
		save_image(file_name, image_info.img_buf + i * image_info.single_img_size, image_info.single_img_size);
	}
}