#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "egis_definition.h"
#include "op_manager.h"
#include "opt_file.h"
#include "plat_file.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "response_def.h"

#define MAX_READ_LEN 1024
#define FRAME_WIDTH (200)
#define FRAME_HEIGHT (200)
#define SIZEOFSHORT sizeof(unsigned short)

#define MAX_CALIBRATION_DATA_SIZE 180000  //(FRAME_WIDTH * FRAME_HEIGHT * SIZEOFSHORT + 192)
#define MAX_TRANSPORTER_DATA_SIZE (512 * 1024)
#define MAX_IMAGE_SIZE (10 * 1024)
#define MAX_TEMPLATE_DATA_STRUCT_SIZE (3 * 1024 * 1024 + 100)
#define RECEIVE_SEND_DATA_SIZE_ONCE (800 * 1024)
#define MAX_SCRATCH_SIZE (FRAME_WIDTH * FRAME_HEIGHT * SIZEOFSHORT + 100)
#define MAX_ENCRY_IMAGE (FRAME_WIDTH * FRAME_HEIGHT * SIZEOFSHORT + 100)
#define MAX_DEBASE_SIZE (FRAME_WIDTH * FRAME_HEIGHT * SIZEOFSHORT * 10 + 164)
#define MAX_TEMPLATE_DATA_SIZE 300 * 1024
#define MAX_BDS_SIZE (2464260)
#define MAX_GROUP_INFO_SIZE 1024

#define BACKUP_CALIBRATION_NAME "/data/vendor/optical_fingerprint/calibration.bin"
#define CALIBRATION_NAME "/mnt/vendor/persist/fingerprint/cb"
#define SCRATCH_NAME "/data/vendor_de/0/fpdata/sc"
#define DEBASE_NAME "/mnt/vendor/persist/fingerprint/debase"
#define USERID_NAME "/mnt/vendor/persist/fingerprint/userinfo"
#define BDS_NAME "bds"
#define TEMPLATE_NAME "tp"
#define USERINFO_NAME "gpinfo"
#define CALI_DATA_COPY_PATH "/data/vendor/fingerprint/et713/cb"
#define TEMPLATE_NAME_BK "bk_tp"
#define TEMPLATE_HASH_VALUE 249997
extern cache_info_t g_cache_info;
extern int g_user_info_value;

extern char g_user_path[MAX_PATH_LEN];
static int save_file(const char* file_name, unsigned char* data, int data_len);
static int get_file(const char* file_name, unsigned char* data, int* data_len);
static int remove_file(const char* file_name);
static int remove_dir(const char* dir);

int save_file(const char* file_name, unsigned char* data, int data_len) {
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

#ifdef __TEMLATE_BACKUP__
static unsigned int get_hash(unsigned char* data, int data_len) {
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

static BOOL check_template(unsigned char* data, int data_len) {
    unsigned int old_hash_value;
    unsigned int hash_value = get_hash(data, (data_len - sizeof(unsigned int)));
    mem_move(&old_hash_value, data + (data_len - sizeof(unsigned int)), sizeof(unsigned int));

    ex_log(LOG_DEBUG, "check_bit old_hash_value:%d hash_value:%d", old_hash_value, hash_value);

    if (old_hash_value == hash_value) {
        return TRUE;
    }

    return FALSE;
}
#endif

int get_file(const char* file_name, unsigned char* buffer, int* buffer_len) {
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
                mem_move(pbuf, read_buffer, read_len);
            } else if (read_len <= 0) {
                retval = FINGERPRINT_RES_BLURRY_DATA;
                goto EXIT;
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

static int convict_fingerid(unsigned char* out_data, int* out_data_size) {
    int retval = FINGERPRINT_RES_SUCCESS;
    int i = 0;
    DIR* dir = opendir(g_user_path);
    struct dirent* ptr = NULL;
    unsigned int* fignerids = (unsigned int*)out_data;
    while ((ptr = readdir(dir)) != NULL) {
        // is a template file
        char fingerid[32];
        if (ptr->d_type != 8 || strncmp(ptr->d_name, "tp_", strlen("tp_")) != 0) {
            continue;
        }
        snprintf(fingerid, 32, "%s", ptr->d_name + 3);
        ex_log(LOG_DEBUG, "convict_fingerid %s,i=%d ", fingerid, i);

        if (i < 5) {
            fignerids[i] = atoi((const char*)fingerid);
            ex_log(LOG_ERROR, "g_cache_info.fingerprint_ids[%d] = %u", i, fignerids[i]);
        }

        i++;
    }

    *out_data_size = i;

    if (0 != closedir(dir)) {
        ex_log(LOG_ERROR, "close template directory failed");
    }

    return retval;
}

int trustonic_delete_data(int type) {
    int retval = FINGERPRINT_RES_SUCCESS;
    char* pname = NULL;
    char file_name[MAX_PATH_LEN] = {0};

    ex_log(LOG_DEBUG, "trustonic_delete_data type = %d", type);
    switch (type) {
        case TYPE_DELETE_SCRATCH_FILE:
            pname = SCRATCH_NAME;
            snprintf(file_name, MAX_PATH_LEN, "%s", pname);
            break;
        case TYPE_DELETE_BDS_FILE:
            pname = BDS_NAME;
            snprintf(file_name, MAX_PATH_LEN, "%s/%s", g_user_path, pname);
            break;
        case TYPE_DELETE_DEBASE:
            pname = DEBASE_NAME;
            snprintf(file_name, MAX_PATH_LEN, "%s", pname);
            break;
        default:
            pname = NULL;
            break;
    }

    if (pname) {
        retval = remove(file_name);
    }

    ex_log(LOG_DEBUG, "trustonic_delete_data retval = %d , file_name = %s", retval, file_name);

    return retval;
}

int trustonic_receive_fingerid(int type, unsigned char* out_data, int* out_data_size) {
    int retval = FINGERPRINT_RES_SUCCESS;
    ex_log(LOG_DEBUG, "trustonic_receive_fingerid type = %d", type);
    switch (type) {
        case TYPE_RECEIVE_CONVICT_ID: {
            retval = convict_fingerid(out_data, out_data_size);
        } break;
        default:
            retval = FINGERPRINT_RES_INVALID_PARAM;
            break;
    }

    ex_log(LOG_DEBUG, "trustonic_receive_fingerid retval = %d", retval);
    return retval;
}

int trustonic_receive_data(int type, unsigned char* in_data, int in_data_len,
                           unsigned char* out_data, unsigned int* out_data_len) {
    unsigned char* data = NULL;
    char file_name[MAX_PATH_LEN] = {0};
    char user_path[MAX_PATH_LEN] = {0};
    int data_len = 0;
    int retval = FINGERPRINT_RES_SUCCESS;
    struct stat st;

    strncpy(user_path, g_user_path, MAX_PATH_LEN-1);
    user_path[MAX_PATH_LEN-1] = '\0';
    ex_log(LOG_DEBUG, "trustonic_receive_data type = %d", type);

    switch (type) {
        case TYPE_RECEIVE_SCRATCH:
        case TYPE_RECEIVE_CALIBRATION_DATA:
        case TYPE_RECEIVE_BACKUP_CALIBRATION_DATA:
        case TYPE_RECEIVE_DEBASE: {
            char* pname = NULL;
            if (type == TYPE_RECEIVE_CALIBRATION_DATA) {
                data_len = MAX_CALIBRATION_DATA_SIZE;
                pname = CALIBRATION_NAME;
            } else if (type == TYPE_RECEIVE_BACKUP_CALIBRATION_DATA) {
                data_len = MAX_CALIBRATION_DATA_SIZE;
                pname = BACKUP_CALIBRATION_NAME;
            } else if (type == TYPE_RECEIVE_SCRATCH) {
                data_len = MAX_SCRATCH_SIZE;
                pname = SCRATCH_NAME;
            } else if (type == TYPE_RECEIVE_DEBASE) {
                data_len = MAX_DEBASE_SIZE;
                pname = DEBASE_NAME;
            } else {
                retval = FINGERPRINT_RES_INVALID_PARAM;
                break;
            }

            data = (unsigned char*)malloc(data_len);
            if (NULL == data) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            retval = opm_get_data(type, NULL, 0, data, &data_len);
            ex_log(LOG_DEBUG, "opm_get_data ret = %d ,datalen %d ", retval, data_len);

            if (retval == FINGERPRINT_RES_SUCCESS && data_len != 0) {
                int ret = save_file(pname, data, data_len);
                ex_log(LOG_DEBUG, "opm_get_data ret = %d ,datalen %d ,%s ", ret, data_len, pname);
            }
        } break;
        case TYPE_RECEIVE_BDS: {
            int receive_pos = 0;
            unsigned int in_array = 0;
            unsigned char* pBuffer = NULL;
            int receive_size;

            data_len = MAX_BDS_SIZE;
            data = (unsigned char*)malloc(data_len);
            if (data == NULL) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            mem_set(data, 0x00, MAX_BDS_SIZE);
            retval = opm_get_data(TYPE_RECEIVE_BDS_START, NULL, 0, NULL, 0);

            if (retval != FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_ERROR, "TYPE_RECEIVE_BDS TYPE_RECEIVE_BDS_START NO NEED UPDATE");
                break;
            }

            do {
                pBuffer = data + receive_pos;
                receive_size = (data_len - receive_pos) > RECEIVE_SEND_DATA_SIZE_ONCE
                                   ? RECEIVE_SEND_DATA_SIZE_ONCE
                                   : (data_len - receive_pos);
                in_array = receive_pos;
                retval = opm_get_data(TYPE_RECEIVE_BDS, (unsigned char*)&in_array,
                                      sizeof(unsigned int), pBuffer, &receive_size);
                ex_log(LOG_DEBUG, "opm_get_data ret = %d,pos %d ,receive_size = %d, data_len = %d",
                       retval, receive_pos, receive_size, data_len);

                receive_pos += receive_size;
                if (retval == FINGERPRINT_RES_SUCCESS) {
                    break;
                }

                if (retval != FINGERPRINT_RES_TEMPLATE_CONTINUE ||
                    receive_size > RECEIVE_SEND_DATA_SIZE_ONCE) {
                    retval = FINGERPRINT_RES_FAILED;
                    break;
                }

            } while (receive_pos < data_len);

            retval = opm_get_data(TYPE_RECEIVE_BDS_END, NULL, 0, NULL, 0);
            if (retval != FINGERPRINT_RES_SUCCESS) {
                break;
            }

            if (stat(user_path, &st) == -1) {
                mkdir(user_path, 0700);
            }

            data_len = receive_pos;

            snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, BDS_NAME);
            retval = save_file(file_name, data, data_len);
            ex_log(LOG_DEBUG, "opm_get_data ret = %d ,datalen %d, receive len %d file_name %s ",
                   retval, data_len, receive_pos, file_name);

        } break;
        case TYPE_RECEIVE_TEMPLATE: {
            int receive_pos = 0;
            unsigned int fid = 0;
            unsigned int in_array[2];
            unsigned char* pBuffer = NULL;
            int receive_size = 0;
            if (in_data == NULL || in_data_len != sizeof(unsigned int)) {
                retval = FINGERPRINT_RES_INVALID_PARAM;
                break;
            }
            fid = *((unsigned int*)in_data);
            in_array[0] = fid;
            data_len = MAX_TEMPLATE_DATA_STRUCT_SIZE;
            data = (unsigned char*)malloc(data_len);
            if (data == NULL) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            mem_set(data, 0x00, MAX_TEMPLATE_DATA_STRUCT_SIZE);
            retval = opm_get_data(TYPE_RECEIVE_TEMPLATE_START, (unsigned char*)in_array,
                                  sizeof(in_array), NULL, 0);
            ex_log(LOG_DEBUG, "TYPE_RECEIVE_TEMPLATE_START return %d", retval);

            if (retval != FINGERPRINT_RES_SUCCESS) break;
            do {
                pBuffer = data + receive_pos;
                receive_size = (data_len - receive_pos) > RECEIVE_SEND_DATA_SIZE_ONCE
                                   ? RECEIVE_SEND_DATA_SIZE_ONCE
                                   : (data_len - receive_pos);
                in_array[1] = receive_pos;
                ex_log(LOG_DEBUG, "TYPE_RECEIVE_TEMPLATE_START receive_pos = %d,receive_size=%d",
                       receive_pos, receive_size);
                retval = opm_get_data(TYPE_RECEIVE_TEMPLATE, (unsigned char*)in_array,
                                      sizeof(in_array), pBuffer, &receive_size);
                ex_log(LOG_DEBUG,
                       "opm_get_data ret = %d fid %d ,pos %d ,receive_size = %d, data_len = %d ",
                       retval, *((unsigned int*)in_data), receive_pos, receive_size, data_len);

                receive_pos += receive_size;
                if (retval == FINGERPRINT_RES_SUCCESS) {
                    break;
                }

                if (retval != FINGERPRINT_RES_TEMPLATE_CONTINUE ||
                    receive_size > RECEIVE_SEND_DATA_SIZE_ONCE) {
                    retval = FINGERPRINT_RES_FAILED;
                    break;
                }

            } while (receive_pos < data_len);

            opm_get_data(TYPE_RECEIVE_TEMPLATE_END, NULL, 0, NULL, 0);
            if (retval != FINGERPRINT_RES_SUCCESS) break;

            if (stat(user_path, &st) == -1) {
                if (mkdir(user_path, 0700) != 0) {
                    ex_log(LOG_ERROR, "mkdir %s fail", user_path);
                    break;
                }
            }

            data_len = receive_pos;
#ifdef __TEMLATE_BACKUP__

            snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, TEMPLATE_NAME_BK, fid);
            retval = save_file(file_name, data, data_len);
            ex_log(LOG_INFO, "TYPE_RECEIVE_TEMPLATE_file_:%s", file_name);

#endif
            snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, TEMPLATE_NAME, fid);
            retval = save_file(file_name, data, data_len);
            ex_log(LOG_DEBUG, "opm_get_data ret = %d ,datalen %d, receive len %d file_name %s ",
                   retval, data_len, receive_pos, file_name);

        } break;

        case TYPE_DELETE_TEMPLATE: {
            if (in_data == NULL || in_data_len != sizeof(unsigned int)) {
                retval = FINGERPRINT_RES_INVALID_PARAM;
                break;
            }

            int fid = *((int*)in_data);
            if (fid == 0) {
                DIR* dir = opendir(user_path);
                struct dirent* ptr = NULL;
                int result;
                retval = FINGERPRINT_RES_SUCCESS;
                while ((ptr = readdir(dir)) != NULL) {
                    // is a template file
                    unsigned char* fingerid = NULL;
                    if (ptr->d_type != 8 || strncmp(ptr->d_name, "tp_", strlen("tp_"))) {
                        continue;
                    }
                    // remove file
#ifdef __TEMLATE_BACKUP__
                    snprintf(file_name, MAX_PATH_LEN, "%s/bk_%s", user_path, ptr->d_name);
                    result = plat_remove_file(file_name);
                    ex_log(LOG_INFO, "TYPE_DELETE_TEMPLATE_1_:%s", file_name);
#endif

                    fingerid = (unsigned char*)ptr->d_name + 3;
                    snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, ptr->d_name);
                    result = remove(file_name);
                    if (result) {
                        ex_log(LOG_DEBUG, "remove template path %s ,%d", file_name, result);
                        retval = result;
                        break;
                    }

                    snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, fingerid);
                    result = remove_dir(file_name);
                    ex_log(LOG_DEBUG, "remove image file  path %s ,%d", file_name, result);
                }
                closedir(dir);
            } else {
#ifdef __TEMLATE_BACKUP__
                snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, TEMPLATE_NAME_BK, fid);
                retval = plat_remove_file(file_name);
                ex_log(LOG_INFO, "TYPE_DELETE_TEMPLATE_2_:%s", file_name);
#endif
                snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, TEMPLATE_NAME, fid);
                retval = remove(file_name);
                ex_log(LOG_DEBUG, "remove template path %s ", file_name);

                snprintf(file_name, MAX_PATH_LEN, "%s/%u", user_path, fid);
                retval = remove_dir(file_name);
                ex_log(LOG_DEBUG, "remove image file  path %s ,%d ", file_name, retval);
            }
        } break;
        case TYPE_DELETE_ENCRY_IMAGE: {
            int fid = *((int*)in_data);
            snprintf(file_name, MAX_PATH_LEN, "%s/%u", user_path, fid);
            retval = remove_dir(file_name);
            ex_log(LOG_DEBUG, "remove image file  path %s ,%d ", file_name, retval);
            break;
        }
        case TYPE_RECEIVE_USER_INFO: {
            ex_log(LOG_DEBUG, "TYPE_RECEIVE_USER_INFO enter");
            // char user_path[MAX_PATH_LEN] = {0};
            unsigned int user_id;
            data_len = MAX_GROUP_INFO_SIZE;
            data = (unsigned char*)malloc(data_len);
            if (NULL == data) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            retval = opm_get_data(TYPE_RECEIVE_USER_INFO, NULL, 0, data, &data_len);
            if (retval != FINGERPRINT_RES_SUCCESS) {
                retval = FINGERPRINT_RES_FAILED;
                ex_log(LOG_ERROR, "opm_get_data failed %d", retval);
                break;
            }

            user_id = *((unsigned int*)in_data);
            snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, USERINFO_NAME, user_id);
            retval = save_file(file_name, data, data_len);
            ex_log(LOG_DEBUG, "TYPE_RECEIVE_USER_INFO leave %d,file_name = %s", retval, file_name);
        } break;
        case TYPE_RECEIVE_ENCRY_IMAGE: {
            int finger_id, enroll_index;
            if (in_data == NULL || in_data_len != 8) {
                ex_log(LOG_ERROR, "Please check if the parameters have changed,in_data_len %d",
                       in_data_len);
                retval = FINGERPRINT_RES_INVALID_PARAM;
                break;
            }
            finger_id = ((int*)in_data)[0];
            enroll_index = ((int*)in_data)[1];

            data_len = MAX_ENCRY_IMAGE;
            data = (unsigned char*)malloc(data_len);

            retval = opm_get_data(TYPE_RECEIVE_ENCRY_IMAGE, (unsigned char*)&finger_id, sizeof(int),
                                  data, &data_len);

            if (retval != FINGERPRINT_RES_SUCCESS || data_len <= 0) {
                break;
            }
            snprintf(file_name, MAX_PATH_LEN, "%s/%d", user_path, finger_id);
            if (stat(file_name, &st) == -1) {
                if (mkdir(file_name, 0700) != 0) {
                    ex_log(LOG_ERROR, "mkdir %s fail", file_name);
                    break;
                }
            }

            snprintf(file_name, MAX_PATH_LEN, "%s/%d/%03d", user_path, finger_id, enroll_index);
            retval = save_file(file_name, data, data_len);

            ex_log(LOG_DEBUG, "save encry image %s %d %d %d", file_name, enroll_index, data_len,
                   retval);
        } break;

        default:
            retval = FINGERPRINT_RES_INVALID_PARAM;
            break;
    }

    if (NULL != data) {
        free(data);
        data = NULL;
    }

    ex_log(LOG_INFO, "receive data type %d %d", type, retval);
    return retval;
}

static int _send_file_data(int type, int data_len, const char* path) {
    unsigned char* data = (unsigned char*)malloc(data_len);
    ex_log(LOG_DEBUG, "%s [%d] (%p) %d", __func__, type, data, data_len);

    int retval = get_file(path, data, &data_len);
    ex_log(LOG_DEBUG, "%s [%d] (%s) %d", __func__, type, path, data_len);
    if (FINGERPRINT_RES_SUCCESS != retval) {
        ex_log(LOG_ERROR, "get_file retval = %d", retval);
        goto out;
    }
    retval = opm_set_data(type, data, data_len);
    ex_log(LOG_DEBUG, "opm_set_data retval = %d", retval);
out:
    if (NULL != data) {
        free(data);
        data = NULL;
    }
    return retval;
}

static int _templ_upgrade_fingerid(unsigned int fingerid) {
    ex_log(LOG_DEBUG, "%s enter", __func__);
    char file_name[MAX_PATH_LEN] = {0};
    char user_path[MAX_PATH_LEN] = {0};
    int i, j, retval;
    int data_len = 200 * 200 * 2 + 12 + 100;
    strncpy(user_path, "/data/vendor_de/0/fpdata", MAX_PATH_LEN);

    ex_log(LOG_DEBUG, "%s fingerid %u", __func__, fingerid);

    int num_encryp_images = 15;
    unsigned int data[2];
    data[0] = fingerid;
    data[1] = num_encryp_images;
    ex_log(LOG_DEBUG, "%s data %d", __func__, (int)sizeof(data));
    retval = opm_set_data(TYPE_SEND_ENCRY_IMAGE_START, data, sizeof(data));
    if(retval != FINGERPRINT_RES_SUCCESS){
        ex_log(LOG_ERROR, "%s TYPE_SEND_ENCRY_IMAGE_START fail retval = %d", __func__, retval);
        return retval;
    }
    for (i = 0; i < num_encryp_images + 5; i++) {
        j = i;
        if (j >= num_encryp_images) j -= num_encryp_images;

        snprintf(file_name, MAX_PATH_LEN, "%s/%d/%03d", user_path, fingerid, j);
        ex_log(LOG_DEBUG, "%s file_name %s", __func__, file_name);
        retval = _send_file_data(TYPE_SEND_ENCRY_IMAGE, data_len, file_name);
        if (retval != FINGERPRINT_RES_SUCCESS) {
            ex_log(LOG_ERROR, "Failed to send data, %s", (char *)file_name);
            break;
        }
    }
    retval = opm_set_data(TYPE_SEND_ENCRY_IMAGE_END, NULL, 0);
    return retval;
}

int trustonic_send_data(int type, unsigned char* in_data, int in_data_len) {
    int retval = FINGERPRINT_RES_FAILED;
    unsigned char* data = NULL;
    char file_name[MAX_PATH_LEN] = {0};
    char user_path[MAX_PATH_LEN] = {0};
    int data_len;
#ifdef __TEMLATE_BACKUP__
    char bk_file_name[MAX_PATH_LEN] = {0};
#endif
    strncpy(user_path, g_user_path, MAX_PATH_LEN-1);
    user_path[MAX_PATH_LEN-1] = '\0';
    ex_log(LOG_DEBUG, "trustonic_send_data type  %d ", type);

    switch (type) {
        case TYPE_SEND_SCRATCH:
        case TYPE_SEND_CALIBRATION_DATA:
        case TYPE_SEND_BACKUP_CALIBRATION_DATA:
        case TYPE_SEND_DEBASE: {
            char* pname = NULL;
            if (type == TYPE_SEND_CALIBRATION_DATA) {
                data_len = MAX_CALIBRATION_DATA_SIZE;
                pname = CALIBRATION_NAME;
            } else if (type == TYPE_SEND_BACKUP_CALIBRATION_DATA) {
                data_len = MAX_CALIBRATION_DATA_SIZE;
                pname = BACKUP_CALIBRATION_NAME;
            } else if (type == TYPE_SEND_SCRATCH) {
                data_len = MAX_SCRATCH_SIZE;
                pname = SCRATCH_NAME;
            } else if (type == TYPE_SEND_DEBASE) {
                data_len = MAX_DEBASE_SIZE;
                pname = DEBASE_NAME;
            } else
                break;

            data = (unsigned char*)malloc(data_len);
            retval = get_file(pname, data, &data_len);

            ex_log(LOG_DEBUG, "get_file retval = %d,pname=%s,data_len=%d", retval, pname, data_len);
            if (FINGERPRINT_RES_SUCCESS != retval) {
                if (NULL != data) {
                    free(data);
                    data = NULL;
                }
                return retval;
            }

            if (type == TYPE_SEND_BACKUP_CALIBRATION_DATA) {
                int ret = save_file(CALIBRATION_NAME, data, data_len);
                ex_log(LOG_DEBUG, "save_calibration_data ret = %d ,datalen %d ,%s ", ret, data_len,
                       CALIBRATION_NAME);
            }

            retval = opm_set_data(type, data, data_len);
            ex_log(LOG_DEBUG, "opm_set_data retval = %d", retval);
        } break;
        case TYPE_SEND_BDS: {
            int offset = 0;
            int send_len = RECEIVE_SEND_DATA_SIZE_ONCE;
            DIR* dir;

            data_len = MAX_BDS_SIZE;
            data = (unsigned char*)malloc(data_len);
            if (NULL == data) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            dir = opendir(user_path);
            if (dir == NULL) {
                retval = FINGERPRINT_RES_FAILED;
                break;
            }

            ex_log(LOG_DEBUG, "user path = %s", user_path);

            snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, BDS_NAME);
            offset = 0;
            data_len = MAX_BDS_SIZE;
            retval = get_file(file_name, data, &data_len);

            ex_log(LOG_DEBUG, "filename = %s,ret = %d,data_len = %d", file_name, retval, data_len);
            if (retval == FINGERPRINT_RES_SUCCESS) {
                retval = opm_set_data(TYPE_SEND_BDS_START, (unsigned char*)&data_len,
                                      sizeof(unsigned int));
                do {
                    send_len = (data_len - offset < RECEIVE_SEND_DATA_SIZE_ONCE)
                                   ? data_len - offset
                                   : RECEIVE_SEND_DATA_SIZE_ONCE;

                    retval = opm_set_data(TYPE_SEND_BDS, data + offset, send_len);

                    offset += send_len;

                    ex_log(LOG_DEBUG, "template offset  %d ,send_len %d,data_len=%d，retval=%d",
                           offset, send_len, data_len, retval);
                } while (data_len - offset > 0);
                retval = opm_set_data(TYPE_SEND_BDS_END, NULL, 0);
            }
            closedir(dir);
            ex_log(LOG_DEBUG, "template filename  %s ,ret %d ,%d", file_name, retval, data_len);

        } break;
        case TYPE_SEND_TEMPLATE: {
            int offset = 0;
            int send_len = RECEIVE_SEND_DATA_SIZE_ONCE;
            retval = opm_set_data(TYPE_DELETE_TEMPLATE, NULL, 0);
            if (retval != FINGERPRINT_RES_SUCCESS) {
                break;
            }

            data_len = MAX_TEMPLATE_DATA_STRUCT_SIZE;
            data = (unsigned char*)malloc(data_len);
            if (NULL == data) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            DIR* dir = opendir("/data/vendor_de/0/fpdata");
            struct dirent* ptr = NULL;

            if (dir == NULL) {
                retval = FINGERPRINT_RES_FAILED;
                break;
            }
            ex_log(LOG_DEBUG, "user path = %s", user_path);
            while ((ptr = readdir(dir)) != NULL) {
                if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
                    continue;
                }

                if (ptr->d_type != 8 || strncmp(ptr->d_name, "tp_", strlen("tp_")) != 0) {
                    continue;
                }

                snprintf(file_name, MAX_PATH_LEN, "%s/%s", user_path, ptr->d_name);
#ifdef __TEMLATE_BACKUP__
                snprintf(bk_file_name, MAX_PATH_LEN, "%s/bk_%s", user_path, ptr->d_name);
#endif
                offset = 0;
                data_len = MAX_TEMPLATE_DATA_STRUCT_SIZE;
                retval = get_file(file_name, data, &data_len);

                ex_log(LOG_INFO, "%s : %d : %d", file_name, retval, data_len);

                if (retval == FINGERPRINT_RES_SUCCESS) {
#ifdef __TEMLATE_BACKUP__
                    save_file(bk_file_name, data, data_len);  // put data into bk_file_name
#endif
                    retval = opm_set_data(TYPE_SEND_TEMPLATE_START, (unsigned char*)&data_len,
                                          sizeof(unsigned int));
                    if (retval == EGIS_NOT_SEND_TEMPL) {
                        ex_log(LOG_ERROR, "SEND_TEMPLATE no bkg exist");
                        break;
                    }

                    do {
                        send_len = (data_len - offset < RECEIVE_SEND_DATA_SIZE_ONCE)
                                       ? data_len - offset
                                       : RECEIVE_SEND_DATA_SIZE_ONCE;

                        retval = opm_set_data(TYPE_SEND_TEMPLATE, data + offset, send_len);

                        offset += send_len;

                        ex_log(LOG_DEBUG, "template offset  %d ,send_len %d,data_len=%d，retval=%d",
                               offset, send_len, data_len, retval);
                    } while (data_len - offset > 0);
                    retval = opm_set_data(TYPE_SEND_TEMPLATE_END, NULL, 0);
                    ex_log(LOG_DEBUG, "TYPE_SEND_TEMPLATE_END template filename  %s ,ret %d ,%d",
                           file_name, retval, data_len);
                    if (retval == EGIS_TEMPL_NEED_UPGRADED) {
                        break;
                    } else if (retval != FINGERPRINT_RES_SUCCESS) {
                        ex_log(LOG_ERROR, "template filename  %s ,ret %d ,%d", file_name, retval,
                               data_len);
                    }
                } else {
#ifdef __TEMLATE_BACKUP__
                    ex_log(LOG_INFO, "TYPE_SEND_TEMPLATE_getting_backup");
                    retval = get_file(bk_file_name, data, &data_len);
                    ex_log(LOG_INFO, "TYPE_SEND_TEMPLATE_bk_file_name:%s", bk_file_name);
                    if (retval == FINGERPRINT_RES_SUCCESS) {
                        if (check_template(data, data_len)) {
                            save_file(file_name, data, data_len);
                            ex_log(LOG_INFO, "TYPE_SEND_TEMPLATE_save_file_name:%s", file_name);
                            retval = opm_set_data(TYPE_SEND_TEMPLATE, data,
                                                  data_len - sizeof(unsigned int));
                        }
                    }

#endif
                    if (retval != FINGERPRINT_RES_SUCCESS) {
                        plat_remove_file(file_name);
#ifdef __TEMLATE_BACKUP__
                        plat_remove_file(bk_file_name);
#endif
                        ex_log(LOG_ERROR, "template filename  %s ,ret %d ,%d", file_name, retval,
                               data_len);
                    }
                }
            }
        } break;
        case TYPE_SEND_UPGRADE: {
            ex_log(LOG_DEBUG, "TYPE_SEND_UPGRADE enter,g_cache_info.fingerprint_ids_count = %d",
                   g_cache_info.fingerprint_ids_count);
            for (unsigned int index = 0; index < g_cache_info.fingerprint_ids_count; index++) {
                ex_log(LOG_DEBUG, "TYPE_SEND_UPGRADE fingerid = %u",
                       g_cache_info.fingerprint_ids[index]);
                _templ_upgrade_fingerid(g_cache_info.fingerprint_ids[index]);
            }
        } break;
        case TYPE_SEND_USER_INFO: {
            ex_log(LOG_DEBUG, "TYPE_SEND_USER_INFO enter");
            unsigned int user_id = 0;
            if (in_data == NULL || in_data_len < (int)sizeof(int)) {
                ex_log(LOG_ERROR, "TYPE_SEND_USER_INFO invalid param");
                return FINGERPRINT_RES_INVALID_PARAM;
            }

            user_id = *((unsigned int*)in_data);
            snprintf(file_name, MAX_PATH_LEN, "%s/%s_%u", user_path, USERINFO_NAME, user_id);

            ex_log(LOG_DEBUG, "TYPE_SEND_USER_INFO path : %s", file_name);
            data_len = MAX_GROUP_INFO_SIZE;
            data = (unsigned char*)malloc(data_len);
            if (NULL == data) {
                retval = FINGERPRINT_RES_ALLOC_FAILED;
                break;
            }

            retval = get_file(file_name, data, &data_len);
            if (retval == FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_DEBUG, " TYPE_SEND_USER_INFO send user info to ta");
                retval = opm_set_data(TYPE_SEND_USER_INFO, data, data_len);
            }
            ex_log(LOG_DEBUG, "TYPE_SEND_USER_INFO leave %d", retval);
        } break;
        case TYPE_COPY_CALI_DATA: {
            char* pname = NULL;
            data_len = MAX_CALIBRATION_DATA_SIZE;
            pname = CALIBRATION_NAME;

            data = (unsigned char*)malloc(data_len);
            retval = get_file(pname, data, &data_len);
            if (retval == FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_INFO, " TYPE_COPY_CALI_DATA get file success");
                pname = CALI_DATA_COPY_PATH;
                retval = save_file(pname, data, data_len);
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, " TYPE_COPY_CALI_DATA save file fail");
                }
            }
        } break;
        case TYPE_SEND_USERID: {
            char* pname = NULL;
            data_len = sizeof(int);
            pname = USERID_NAME;

            retval = get_file(pname, &g_user_info_value, &data_len);
            if (retval == FINGERPRINT_RES_SUCCESS) {
                ex_log(LOG_INFO, " TYPE_SEND_USERID get file success");
            } else {
                srand((unsigned)time(NULL));
                data_len = sizeof(int);
                g_user_info_value = rand();
                retval = save_file(pname, &g_user_info_value, data_len);
                if (retval != FINGERPRINT_RES_SUCCESS) {
                    ex_log(LOG_ERROR, " TYPE_SEND_USERID save file fail, %d",retval);
                }
            }
            ex_log(LOG_INFO, " TYPE_SEND_USERID g_user_info_value = %d", g_user_info_value);
        }break;
        default:
            retval = FINGERPRINT_RES_INVALID_PARAM;
            break;
    }

    if (NULL != data) {
        free(data);
        data = NULL;
    }

    ex_log(LOG_INFO, "send data type %d %d", type, retval);
    return retval;
}

static int remove_dir(const char* dir) {
    char cur_dir[] = ".";
    char up_dir[] = "..";
    char dir_name[255];
    DIR* dirp;
    struct dirent* dp;
    struct stat dir_stat;
    if (0 != access(dir, F_OK)) {
        return FINGERPRINT_RES_SUCCESS;
    }

    if (0 != stat(dir, &dir_stat)) {
        return FINGERPRINT_RES_FAILED;
    }

    if (S_ISREG(dir_stat.st_mode)) {
        remove(dir);
        ex_log(LOG_DEBUG, "remove_dir %s", dir);
    } else if (S_ISDIR(dir_stat.st_mode)) {
        dirp = opendir(dir);
        if (dirp != NULL) {
            while ((dp = readdir(dirp)) != NULL) {
                if ((0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name))) {
                    continue;
                }

                sprintf(dir_name, "%s/%s", dir, dp->d_name);
                remove_dir(dir_name);
            }

            closedir(dirp);
        }
        rmdir(dir);
    } else {
        return FINGERPRINT_RES_FAILED;
    }

    return FINGERPRINT_RES_SUCCESS;
}