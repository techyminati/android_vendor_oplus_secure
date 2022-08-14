#include "opt_file.h"
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "plat_log.h"
#include "opt_file.h"
#include "op_manager.h"
#include "response_def.h"
#include "common_definition.h"
#include "type_definition.h"

#define MAX_READ_LEN 1024

#define MAX_CALIBRATION_DATA_SIZE 20 * 1024
#define MAX_TRANSPORTER_DATA_SIZE 512 * 1024
#define MAX_IMAGE_SIZE 10 * 1024
#define MAX_TEMPLATE_DATA_SIZE 300 * 1024
#define MAX_GROUP_INFO_SIZE 1024
#define CALIBRATION_NAME "cb"
#define TEMPLATE_NAME "tp"
#define USERINFO_NAME "gpinfo"
#define PATTERN_INFO_NAME "pattern"
#define ENROLL_MASK_NAME "emask"
#define TEMPLATE_HASH_VALUE 249997
#ifdef __TEMLATE_BACKUP__
#define TEMPLATE_NAME_BK "bk_tp"
#endif

fix_pattern_data g_fix_data;
extern char g_user_path[MAX_PATH_LEN];

static int save_file(const char* file_name, unsigned char* data, int data_len);
static int get_file(const char* file_name, unsigned char* data, int* data_len);
static int remove_file(const char* file_name);
static unsigned int get_hash(unsigned char *data, int data_len);
static void set_check_bit(unsigned char *data, int *data_len);
static BOOL check_template(unsigned char *data, int data_len);

static unsigned int get_hash(unsigned char *data, int data_len)
{
	unsigned int hash_value = 0;
	int index;

	for (index = 0; index < data_len; index++) {
		if ((index & 1) == 0) {   
			hash_value ^= ((hash_value << 7) ^ (*data++) ^ (hash_value >> 3));   
		} else {   
			hash_value ^= (~((hash_value << 11) ^ (*data++) ^ (hash_value >> 5)));
		}
	}

	return (hash_value % TEMPLATE_HASH_VALUE);
}

static void set_check_bit(unsigned char *data, int *data_len)
{
	unsigned int hash_value = get_hash(data, *data_len);
	ex_log(LOG_DEBUG, "set_check_bit hash_value:%d", hash_value);

	memcpy(data + *data_len, (unsigned char *)&hash_value, sizeof(unsigned int));
	*data_len += sizeof(unsigned int);
}

static BOOL check_template(unsigned char *data, int data_len)
{
	unsigned int old_hash_value;
	unsigned int hash_value = get_hash(data, (data_len - sizeof(unsigned int)));
	memcpy(&old_hash_value, data + (data_len - sizeof(unsigned int)), sizeof(unsigned int));

	ex_log(LOG_DEBUG, "check_bit old_hash_value:%d hash_value:%d", old_hash_value, hash_value);

	if (old_hash_value == hash_value) {
		return TRUE;
	}

	return FALSE;
}

static int save_file(const char* file_name, unsigned char* data, int data_len)
{
	int retval = FINGERPRINT_RES_SUCCESS;
	int write_len;
	FILE* file;

	if (NULL == data || data_len <= 0) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}

	file = fopen(file_name, "wb+");
	if (NULL == file) {
		retval = FINGERPRINT_RES_OPEN_FILE_FAILED;
		goto EXIT;
	}

	write_len = fwrite(data, 1, data_len, file);
	if (write_len < data_len) {
		retval = FINGERPRINT_RES_FAILED;
	}

	fclose(file);
EXIT:
	return retval;
}

static int get_file(const char* file_name, unsigned char* buffer,
		    int* buffer_len)
{
	int retval = FINGERPRINT_RES_SUCCESS;
	FILE* file;
	int read_len;
	int read_total_len = 0;
	unsigned char* pbuf;

	if (buffer == NULL || buffer_len == NULL || *buffer_len <= 0) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}

	unsigned char* read_buffer = (unsigned char*)malloc(MAX_READ_LEN);
	if (NULL == read_buffer) {
		return FINGERPRINT_RES_ALLOC_FAILED;
	}

	file = fopen(file_name, "rb");
	if (NULL == file) {
		*buffer_len = 0;
		retval = FINGERPRINT_RES_OPEN_FILE_FAILED;
	}

	if (FINGERPRINT_RES_SUCCESS == retval) {
		while (1) {
			pbuf = buffer + read_total_len;

			read_len = fread(read_buffer, 1, MAX_READ_LEN, file);
			if (read_len > 0) {
				read_total_len += read_len;
				if (*buffer_len < read_total_len) {
					retval = FINGERPRINT_RES_FAILED;
					goto EXIT;
				}
				memcpy(pbuf, read_buffer, read_len);
			}
			if (read_len < MAX_READ_LEN) {
				retval = FINGERPRINT_RES_SUCCESS;
				break;
			}
		};

		*buffer_len = read_total_len;
	}

EXIT:
	if (NULL != file) {
		fclose(file);
	}

	if (NULL != read_buffer) {
		free(read_buffer);
		read_buffer = NULL;
	}

	return retval;
}

static int remove_file(const char* file_name)
{
	int retval = FINGERPRINT_RES_SUCCESS;

	if(NULL == file_name) {
		ex_log(LOG_ERROR, "filename invalid");
		retval = FINGERPRINT_RES_FAILED;
		return retval;
	}

	if(0 != remove(file_name)) {
		ex_log(LOG_ERROR, "remove file failed : %s", file_name);
		retval = FINGERPRINT_RES_FAILED;
	}

	return retval;
}

int trustonic_receive_data(int type, unsigned char* in_data, int in_data_len)
{
	unsigned char* data = NULL;
	unsigned int receive_type;
	unsigned int fid;
	unsigned int user_id;
	char file_name[MAX_PATH_LEN] = {0};
	char user_path[MAX_PATH_LEN] = {0};
	int data_len = 0;
	int retval;
	struct stat st;

	strncpy(user_path, g_user_path, MAX_PATH_LEN - 1);

	switch (type) {
		case TYPE_RECEIVE_CALIBRATION_DATA: {
			data_len = MAX_CALIBRATION_DATA_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (NULL == data) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = opm_get_data(TYPE_RECEIVE_CALIBRATION_DATA,
					      NULL, 0, data, &data_len);
			if (retval == FINGERPRINT_RES_SUCCESS &&
			    data_len != 0) {
				save_file(CALIBRATION_NAME, data, data_len);
			}
		} break;

		case TYPE_RECEIVE_TEMPLATE: {
			if (in_data == NULL ||
			    in_data_len != 2 * sizeof(int)) {
				retval = FINGERPRINT_RES_INVALID_PARAM;
				break;
			}

			receive_type = *((int*)in_data);
			fid = *((int*)(in_data + sizeof(unsigned int)));
			ex_log(LOG_DEBUG, "receive_type:%d fid:%d", receive_type, fid);
			data_len = MAX_TEMPLATE_DATA_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (data == NULL) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = opm_get_data(TYPE_RECEIVE_TEMPLATE,
					      (unsigned char*)&fid, sizeof(fid),
					      data, &data_len);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				retval = FINGERPRINT_RES_FAILED;
				break;
			}

			if (stat(user_path, &st) == -1) {
				if(0 != mkdir(user_path, 0700)) {
					ex_log(LOG_ERROR, "create directory failed : %s", user_path);
				}
			}

			set_check_bit(data, &data_len);

#ifdef __TEMLATE_BACKUP__
			if (receive_type == ENROLL_RECEIVE_TEMPLATE) {
				snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path,
				 	TEMPLATE_NAME_BK, fid);				 
				retval = save_file(file_name, data, data_len);
			}
#endif
			snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path,
				 TEMPLATE_NAME, fid);
			retval = save_file(file_name, data, data_len);
		} break;

		case TYPE_DELETE_TEMPLATE: {
			if (in_data == NULL ||
			    in_data_len != sizeof(int)) {
				retval = FINGERPRINT_RES_INVALID_PARAM;
				break;
			}

			fid = *((int*)in_data);
			if (fid == 0) {
				DIR* dir = opendir(user_path);
				struct dirent* ptr = NULL;
				int result;
				retval = FINGERPRINT_RES_SUCCESS;
				while ((ptr = readdir(dir)) != NULL) {
					//is a template file
					if (ptr->d_type != 8 ||
					    strncmp(ptr->d_name, "tp_", strlen("tp_")) != 0) {
						continue;
					}
					//remove file
#ifdef __TEMLATE_BACKUP__
					snprintf(file_name, MAX_PATH_LEN, "%s/bk_%s", user_path, ptr->d_name);
				 	result = remove_file(file_name);
#endif
					snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, ptr->d_name);
					result = remove_file(file_name);
					if (result)
						retval = result;
				}

				if(0 != closedir(dir)) {
					ex_log(LOG_ERROR, "close template directory failed");
				}
			} else {
#ifdef __TEMLATE_BACKUP__
				snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, TEMPLATE_NAME_BK, fid);
				retval = remove_file(file_name);
#endif
				snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, TEMPLATE_NAME, fid);
				retval = remove_file(file_name);
			}
		} break;

		case TYPE_RECEIVE_USER_INFO: {
			ex_log(LOG_DEBUG, "TYPE_RECEIVE_USER_INFO enter");

			data_len = MAX_GROUP_INFO_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (NULL == data) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = opm_get_data(TYPE_RECEIVE_USER_INFO, NULL, 0, data, &data_len);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				retval = FINGERPRINT_RES_FAILED;
				ex_log(LOG_DEBUG, "opm_get_data failed %d", retval);
				break;
			}

			user_id = *((unsigned int*)in_data);
			snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, USERINFO_NAME, user_id);
			retval = save_file(file_name, data, data_len);

			ex_log(LOG_DEBUG, "TYPE_RECEIVE_USER_INFO leave %d", retval);
		} break;

		case TYPE_RECEIVE_PATTERN_INFO: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, PATTERN_INFO_NAME);
			ex_log(LOG_DEBUG, "TYPE_RECEIVE_PATTERN_INFO file_name : %s", file_name);

			data_len = MAX_FIX_PATTERN_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (data == NULL) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = opm_get_data(TYPE_RECEIVE_PATTERN_INFO,NULL, 0, data, &data_len);
			ex_log(LOG_DEBUG, "opm_get_data retval : %d", retval);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				retval = FINGERPRINT_RES_FAILED;
				break;
			}

			memcpy(g_fix_data.fix_data, data, data_len);
			g_fix_data.fix_data_size = data_len;

			retval = save_file(file_name, data, data_len);
			ex_log(LOG_DEBUG, "save_file retval : %d", retval);
		} break;

		case TYPE_DELETE_PATTERN_INFO: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, PATTERN_INFO_NAME);
			ex_log(LOG_DEBUG, "TYPE_DELETE_PATTERN_INFO file_name : %s", file_name);

			retval = opm_get_data(TYPE_DELETE_PATTERN_INFO, NULL, 0, NULL, NULL);
			ex_log(LOG_DEBUG, "opm_get_data retval : %d", retval);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				retval = FINGERPRINT_RES_FAILED;
				break;
			}

			retval = remove(file_name);
			ex_log(LOG_DEBUG, "remove retval : %d", retval);
		} break;

		case TYPE_RECEIVE_ENROLL_MASK: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, ENROLL_MASK_NAME);
			ex_log(LOG_DEBUG, "TYPE_RECEIVE_ENROLL_MASK file_name : %s", file_name);

			data_len = MAX_FIX_PATTERN_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (data == NULL) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = opm_get_data(TYPE_RECEIVE_ENROLL_MASK,NULL, 0, data, &data_len);
			ex_log(LOG_DEBUG, "opm_get_data retval : %d", retval);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				retval = FINGERPRINT_RES_FAILED;
				break;
			}

			memcpy(g_fix_data.enroll_mask, data, data_len);
			g_fix_data.enroll_mask_size = data_len;

			retval = save_file(file_name, data, data_len);
			ex_log(LOG_DEBUG, "save_file retval : %d", retval);
		} break;

		case TYPE_DELETE_ENROLL_MASK: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, PATTERN_INFO_NAME);
			ex_log(LOG_DEBUG, "TYPE_DELETE_ENROLL_MASK file_name : %s", file_name);

			retval = opm_get_data(TYPE_DELETE_ENROLL_MASK, NULL, 0, NULL, NULL);
			ex_log(LOG_DEBUG, "opm_get_data retval : %d", retval);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				retval = FINGERPRINT_RES_FAILED;
				break;
			}

			retval = remove(file_name);
			ex_log(LOG_DEBUG, "remove retval : %d", retval);
		} break;

		default:
			retval = FINGERPRINT_RES_INVALID_PARAM;
			break;
	}

	if (NULL != data) {
		free(data);
		data = NULL;
	}
	return retval;
}

int trustonic_send_data(int type, unsigned char* in_data, int in_data_len)
{
	int retval;
	unsigned char* data = NULL;
	char file_name[MAX_PATH_LEN] = {0};
#ifdef __TEMLATE_BACKUP__	
	char bk_file_name[MAX_PATH_LEN] = {0};
#endif	
	char user_path[MAX_PATH_LEN] = {0};
	int data_len;
	unsigned int user_id = 0;
	BOOL template_ready = FALSE;

	strncpy(user_path, g_user_path, MAX_PATH_LEN - 1);

	switch (type) {
		case TYPE_SEND_CALIBRATION_DATA: {
			data_len = MAX_CALIBRATION_DATA_SIZE;
			data = (unsigned char*)malloc(data_len);
			retval = get_file(CALIBRATION_NAME, data, &data_len);

			if (FINGERPRINT_RES_SUCCESS != retval) break;

			retval = opm_set_data(TYPE_SEND_CALIBRATION_DATA, data,
					      data_len);
		} break;

		case TYPE_SEND_TEMPLATE: {
			retval = opm_set_data(TYPE_DELETE_TEMPLATE, NULL, 0);
			if (retval != FINGERPRINT_RES_SUCCESS) {
				break;
			}

			data_len = MAX_TEMPLATE_DATA_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (NULL == data) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			DIR* dir = opendir(user_path);
			struct dirent* ptr = NULL;

			if (dir == NULL) {
				retval = FINGERPRINT_RES_FAILED;
				break;
			}
			ex_log(LOG_DEBUG, "user path = %s", user_path);
			while ((ptr = readdir(dir)) != NULL) {
				if (strcmp(ptr->d_name, ".") == 0 ||
				    strcmp(ptr->d_name, "..") == 0) {
					continue;
				}

				if (ptr->d_type != 8) {
					continue;
				}

				template_ready = FALSE;

				if (strncmp(ptr->d_name, "tp_", strlen("tp_")) == 0) {
					snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, ptr->d_name);
#ifdef __TEMLATE_BACKUP__				
					snprintf(bk_file_name, MAX_PATH_LEN, "%s/bk_%s", user_path, ptr->d_name);
#endif	
					retval = get_file(file_name, data, &data_len);
					if (retval == FINGERPRINT_RES_SUCCESS) {
						if (check_template(data, data_len)) {
							template_ready = TRUE;
#ifdef __TEMLATE_BACKUP__
							save_file(bk_file_name, data, data_len);
#endif							
							retval = opm_set_data(TYPE_SEND_TEMPLATE, data, data_len - sizeof(unsigned int));
						}
					}
#ifdef __TEMLATE_BACKUP__
					if (!template_ready) {
						retval = get_file(bk_file_name, data, &data_len);
						if (retval == FINGERPRINT_RES_SUCCESS) {
							if (check_template(data, data_len)) {
								template_ready = TRUE;
								save_file(file_name, data, data_len);
								retval = opm_set_data(TYPE_SEND_TEMPLATE, data, data_len - sizeof(unsigned int));
							}
						}
					}
#endif				
					if (!template_ready) {
						remove_file(file_name);
#ifdef __TEMLATE_BACKUP__						
						remove_file(bk_file_name);
#endif
					}	
				}
#ifdef __TEMLATE_BACKUP__
				else if (strncmp(ptr->d_name, "bk_tp_", strlen("bk_tp_")) == 0) {
					snprintf(bk_file_name, MAX_PATH_LEN, "%s/%s", user_path, ptr->d_name);
					snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, (ptr->d_name + strlen("bk_")));
					if (access(file_name, F_OK) != 0) {
						retval = get_file(bk_file_name, data, &data_len);
						if (retval == FINGERPRINT_RES_SUCCESS) {
							if (check_template(data, data_len)) {
								template_ready = TRUE;
								save_file(file_name, data, data_len);
								retval = opm_set_data(TYPE_SEND_TEMPLATE, data, data_len - sizeof(unsigned int));
							}
						}

						if (!template_ready) {
							remove_file(bk_file_name);
						}
					}
				}
#endif				
			}

			if(0 != closedir(dir)) {
				ex_log(LOG_ERROR, "close template directory failed");
			}
		} break;

		case TYPE_SEND_USER_INFO: {
			ex_log(LOG_DEBUG, "TYPE_SEND_USER_INFO enter");

			if(in_data == NULL || in_data_len < (int)sizeof(int)) {
				ex_log(LOG_ERROR, "invalid param");
				return FINGERPRINT_RES_INVALID_PARAM;
			}

			user_id = *((unsigned int*)in_data);
			snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, USERINFO_NAME, user_id);

			ex_log(LOG_DEBUG, "path : %s", file_name);
			data_len = MAX_GROUP_INFO_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (NULL == data) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = get_file(file_name, data, &data_len);
			if (retval == FINGERPRINT_RES_SUCCESS) {
				ex_log(LOG_DEBUG, "send user info to ta");
				retval = opm_set_data(TYPE_SEND_USER_INFO, data, data_len);
			}
			ex_log(LOG_DEBUG, "TYPE_SEND_USER_INFO leave %d", retval);
		} break;

		case TYPE_SEND_PATTERN_INFO: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, PATTERN_INFO_NAME);
			ex_log(LOG_DEBUG, "TYPE_SEND_PATTERN_INFO file_name : %s", file_name);
			data_len = MAX_FIX_PATTERN_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (NULL == data) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = get_file(file_name, data, &data_len);
			ex_log(LOG_DEBUG, "get_file return : %d", retval);
			if (retval == FINGERPRINT_RES_SUCCESS) {
				memcpy(g_fix_data.fix_data, data, data_len);
				g_fix_data.fix_data_size = data_len;

				retval = opm_set_data(TYPE_SEND_PATTERN_INFO, data, data_len);
				ex_log(LOG_DEBUG, "opm_set_data return : %d", retval);
			}
		} break;

		case TYPE_UPDATA_PATTERN_INFO_TO_USER: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, PATTERN_INFO_NAME);
			ex_log(LOG_DEBUG, "TYPE_UPDATA_PATTERN_INFO_TO_USER file_name : %s", file_name);

			if(g_fix_data.fix_data_size > 0) {
				retval = save_file(file_name, g_fix_data.fix_data, g_fix_data.fix_data_size);
				ex_log(LOG_DEBUG, "save_file data_size:%d, retval:%d", g_fix_data.fix_data_size, retval);
			} else {
				ex_log(LOG_INFO, "fix data empty");
				retval = FINGERPRINT_RES_SUCCESS;
			}
		} break;

		case TYPE_SEND_ENROLL_MASK: {
			snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, ENROLL_MASK_NAME);
			ex_log(LOG_DEBUG, "TYPE_SEND_ENROLL_MASK file_name : %s", file_name);
			data_len = MAX_FIX_PATTERN_SIZE;
			data = (unsigned char*)malloc(data_len);
			if (NULL == data) {
				retval = FINGERPRINT_RES_ALLOC_FAILED;
				break;
			}

			retval = get_file(file_name, data, &data_len);
			ex_log(LOG_DEBUG, "get_file return : %d", retval);
			if (retval == FINGERPRINT_RES_SUCCESS) {
				memcpy(g_fix_data.enroll_mask, data, data_len);
				g_fix_data.enroll_mask_size = data_len;

				retval = opm_set_data(TYPE_SEND_ENROLL_MASK, data, data_len);
				ex_log(LOG_DEBUG, "opm_set_data return : %d", retval);
			}
		} break;

		default:
			retval = FINGERPRINT_RES_INVALID_PARAM;
			break;
	}

	if (NULL != data) {
		free(data);
		data = NULL;
	}

	return retval;
}