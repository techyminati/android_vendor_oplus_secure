/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/fs.h>
#include <string.h>

#include "gf_hal.h"
#include "gf_hal_log.h"
#include "gf_hal_frr_database.h"
#include "gf_error.h"
#include "gf_hal_mem.h"
#include "gf_hal_test_utils.h"


#define LOG_TAG "[GF_HAL][gf_hal_frr_database]"

/*
 * Description: Create FRR database with header information
 * Header data format must be %07d
 * Database Like:
 *
 * PACKAGE_VERSION=XXXXXXX
 * PROTOCOL_VERSION=XXXXXXX
 * CHIP_TYPE=XXXXXXX
 * SCREEN_ON_FAIL_RETRY=XXXXXXX
 * SCREEN_OFF_FAIL_RETRY=XXXXXXX
 * CHIP_SUPPORT_BIO=XXXXXXX
 * IS_BIO_OPEN=XXXXXXX
 * SUCCESS_WITH_BIO=XXXXXXX
 * FAILED_WITH_BIO=XXXXXXX
 * SUCCESS=XXXXXXX
 * FAILED=XXXXXXX
 * BUF_FULL=XXXXXXX
 * UPDATE_POS=XXXXXXX
 * XQXXXAXXX
 * XQXXXAXXX
 * ...........
 *
 */


/**
 * Function: gf_hal_create_frr_database
 * Description: Create database file, and write header data.
 * Input: None
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t gf_hal_create_frr_database()
{
    gf_error_t err = 0;
    FILE *fp = NULL;
    char line_data[64] = { 0 };
    int32_t chip_support_bio = 0;
    FUNC_ENTER();

    do
    {
        int32_t is_bio_enable = 0;
        // Create database file and initialize header data
        fp = fopen(FRR_DATABASE_FILE, "w");

        if (NULL == fp)
        {
            err = GF_ERROR_FILE_OPEN_FAILED;
            LOG_E(LOG_TAG, "[%s] Open file %s failed. err=%s, errno=%d", __func__,
                  FRR_DATABASE_FILE, gf_strerror(err), err);
            break;
        }

        // only Milan A/C support bio
        if ((g_hal_config.chip_type == GF_CHIP_5206)
            || (g_hal_config.chip_type == GF_CHIP_5208))
        {
            chip_support_bio = 1;
        }

        is_bio_enable = g_hal_config.support_bio_assay;
        LOG_D(LOG_TAG, "[%s] chip_support_bio=%d is_bio_enable=%d", __func__,
              chip_support_bio, is_bio_enable);
        // PACKAGE_VERSION
        snprintf(line_data, sizeof(line_data), "PACKAGE_VERSION=%07d\n",
                 atoi(GF_PACKAGE_VERSION_CODE));
        fputs(line_data, fp);
        // PROTOCOL_VERSION
        snprintf(line_data, sizeof(line_data), "PROTOCOL_VERSION=%07d\n",
                 PROTOCOL_VERSION);
        fputs(line_data, fp);
        // CHIP_TYPE
        snprintf(line_data, sizeof(line_data), "CHIP_TYPE=%07d\n",
                 g_hal_config.chip_type);
        fputs(line_data, fp);
        // SCREEN_ON_FAIL_RETRY
        snprintf(line_data, sizeof(line_data), "SCREEN_ON_FAIL_RETRY=%07d\n",
                 g_hal_config.screen_on_authenticate_fail_retry_count);
        fputs(line_data, fp);
        // SCREEN_OFF_FAIL_RETRY
        snprintf(line_data, sizeof(line_data), "SCREEN_OFF_FAIL_RETRY=%07d\n",
                 g_hal_config.screen_off_authenticate_fail_retry_count);
        fputs(line_data, fp);
        // CHIP_SUPPORT_BIO
        snprintf(line_data, sizeof(line_data), "CHIP_SUPPORT_BIO=%07d\n",
                 chip_support_bio);
        fputs(line_data, fp);
        // IS_BIO_OPEN
        snprintf(line_data, sizeof(line_data), "IS_BIO_OPEN=%07d\n", is_bio_enable);
        fputs(line_data, fp);
        // SUCCESS_WITH_BIO
        snprintf(line_data, sizeof(line_data), "SUCCESS_WITH_BIO=%07d\n", 0);
        fputs(line_data, fp);
        // FAILED_WITH_BIO
        snprintf(line_data, sizeof(line_data), "FAILED_WITH_BIO=%07d\n", 0);
        fputs(line_data, fp);
        // SUCCESS
        snprintf(line_data, sizeof(line_data), "SUCCESS=%07d\n", 0);
        fputs(line_data, fp);
        // FAILED
        snprintf(line_data, sizeof(line_data), "FAILED=%07d\n", 0);
        fputs(line_data, fp);
        // BUF_FULL
        snprintf(line_data, sizeof(line_data), "BUF_FULL=%07d\n", 0);
        fputs(line_data, fp);
        // UPDATE_POS
        snprintf(line_data, sizeof(line_data), "UPDATE_POS=%07d\n", 0);
        fputs(line_data, fp);
        fflush(fp);
    }  // do gf_hal_create_frr_database
    while (0);

    if (fp != NULL)
    {
        fclose(fp);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_get_tag_data_pos
 * Description: Get position of tag data.
 * Input: tag_idx
 * Output: None
 * Return: uint32_t
 */
uint32_t gf_hal_get_tag_data_pos(gf_frr_tag_t tag_idx)
{
    uint32_t pos = 0;
    uint32_t i = 0;
    char *tags[] = {
        "PACKAGE_VERSION=", "PROTOCOL_VERSION=", "CHIP_TYPE=", "SCREEN_ON_FAIL_RETRY=",
        "SCREEN_OFF_FAIL_RETRY=", "CHIP_SUPPORT_BIO=", "IS_BIO_OPEN=", "SUCCESS_WITH_BIO=",
        "FAILED_WITH_BIO=", "SUCCESS=", "FAILED=", "BUF_FULL=", "UPDATE_POS="
    };

    for (i = 0; i < tag_idx; i++)
    {
        // every line have '\n' in the end of line
        pos += strlen(tags[i]) + strlen("XXXXXXX") + 1;
    }

    pos += strlen(tags[i]);
    return pos;
}

/**
 * Function: gf_hal_get_tag_data
 * Description: Read tag data by data position.
 * Input: fp, pos
 * Output: None
 * Return: uint32_t
 */
int32_t gf_hal_get_tag_data(FILE *fp, uint32_t pos)
{
    int32_t ret = 0;
    uint32_t data = 0;
    char buf_read[16] = { 0 };

    if (NULL == fp)
    {
        LOG_E(LOG_TAG, "[%s] fp is null", __func__);
        return -1;
    }

    ret = fseek(fp, pos, SEEK_SET);
    if (0 != ret)
    {
        LOG_E(LOG_TAG, "[%s] fseek failed. ret=%d", __func__, ret);
    }
    fgets(buf_read, sizeof(buf_read), fp);
    data = atoi(buf_read);
    return data;
}

/**
 * Function: gf_hal_set_tag_data
 * Description: Write tag data in the position.
 * Input: fp, pos, data
 * Output: None
 * Return: uint32_t
 */
static void gf_hal_set_tag_data(FILE *fp, uint32_t pos, int32_t data)
{
    int32_t ret = 0;
    char buf_write[16] = { 0 };

    ret = fseek(fp, pos, SEEK_SET);
    if (0 != ret)
    {
        return;
    }
    snprintf(buf_write, sizeof(buf_write), "%07d", data);
    fprintf(fp, "%s\n", buf_write);
    return;
}

/**
 * Function: gf_hal_get_data_start_pos
 * Description: Get data in start position.
 * Input: None
 * Output: None
 * Return: uint32_t
 */
static uint32_t gf_hal_get_data_start_pos()
{
    uint32_t pos = 0;
    pos = gf_hal_get_tag_data_pos(TAG_UPDATE_POS) + strlen("XXXXXXX") + 1;
    return pos;
}


/**
 * Function: gf_hal_set_metadata
 * Description: Write metadata.
 * Input: fp, is_auth_sucess, update_pos_data, chip_support_bio, enable_bio,
 *        image_quality, image_area
 * Output: None
 * Return: None
 * 
 * metadata format like:
 * 0Q043A098
 * 1Q066A031
 * -Q059A076
 * xQ059A076
 * first char means bio flag:
 * -:chip do not support bio
 * x:chip support but not enable
 * 0:chip support bio and enable authenticate failed by GF_ERROR_BIO_ASSAY_FAIL
 * 1:chip support bio and enable authenticate failed by GF_ERROR_NOT_MATCH
 */
static void gf_hal_set_metadata(FILE *fp, gf_error_t is_auth_sucess,
                                int32_t update_pos_data,
                                int32_t chip_support_bio, int32_t enable_bio, int32_t image_quality,
                                int32_t image_area)
{
    int32_t ret = 0;
    char buf_write[16] = { 0 };
    uint32_t pos = 0;
    uint32_t step = 0;
    char bio = '?';

    if (chip_support_bio != 1)
    {
        bio = '-';
    }

    if ((chip_support_bio == 1) && (enable_bio != 1))
    {
        bio = 'x';
    }

    if ((chip_support_bio == 1) && (enable_bio == 1))
    {
        if (is_auth_sucess == GF_ERROR_BIO_ASSAY_FAIL)
        {
            bio = '0';
        }
        else
        {
            bio = '1';
        }
    }

    snprintf(buf_write, sizeof(buf_write), "%cQ%03dA%03d\n", bio, image_quality,
             image_area);
    step = strlen("XQXXXAXXX") + 1;
    pos = gf_hal_get_data_start_pos();
    pos += step * update_pos_data;
    ret = fseek(fp, pos, SEEK_SET);
    if (0 != ret)
    {
        return;
    }

    fprintf(fp, "%s", buf_write);
    LOG_I(LOG_TAG, "[%s] Metadata in %cQ%03dA%03d ", __func__, bio, image_quality,
          image_area);
    return;
}

/**
 * Function: gf_hal_update
 * Description: Update authenticat result, or add/update metadata if authenticat failed.
 * Input: fp, is_auth_sucess, image_quality, image_area
 * Output: None
 * Return: None
 */
static void gf_hal_update(FILE *fp, gf_error_t is_auth_sucess,
                          int32_t image_quality,
                          int32_t image_area)
{
    uint32_t offset = 0;
    uint32_t chip_support_bio_offset = 0;
    uint32_t is_open_bio_offset = 0;
    int32_t data = 0;
    int32_t chip_support_bio_data = 0;
    int32_t is_open_bio_data = 0;
    gf_frr_tag_t tag = TAG_PACKAGE_VERSION;
    VOID_FUNC_ENTER();
    // Step1: get 'chip_support_bio' TAG
    chip_support_bio_offset = gf_hal_get_tag_data_pos(TAG_CHIP_SUPPORT_BIO);
    chip_support_bio_data = gf_hal_get_tag_data(fp, chip_support_bio_offset);
    // Step2: get 'is_open_bio' TAG
    is_open_bio_offset = gf_hal_get_tag_data_pos(TAG_IS_BIO_ENABLE);
    is_open_bio_data = gf_hal_get_tag_data(fp, is_open_bio_offset);

    if ((chip_support_bio_data == 1)
        && (is_open_bio_data != g_hal_config.support_bio_assay))
    {
        LOG_I(LOG_TAG,
              "[%s] Chip support Bio and custom have change bio from [%d] to [%d]",
              __func__, is_open_bio_data, g_hal_config.support_bio_assay);
        is_open_bio_data = g_hal_config.support_bio_assay;
        gf_hal_set_tag_data(fp, is_open_bio_offset, is_open_bio_data);
    }

    if (is_auth_sucess == GF_SUCCESS)
    {
        if ((chip_support_bio_data == 1) && (is_open_bio_data == 1))
        {
            tag = TAG_AUTHENTICATED_WITH_BIO_SUCCESS_COUNT;
        }
        else
        {
            tag = TAG_AUTHENTICATED_SUCCESS_COUNT;
        }
    }
    else
    {
        if ((chip_support_bio_data == 1) && (is_open_bio_data == 1))
        {
            tag = TAG_AUTHENTICATED_WITH_BIO_FAILED_COUNT;
        }
        else
        {
            tag = TAG_AUTHENTICATED_FAILED_COUNT;
        }
    }

    // Step3: update authenticat result (SUCCESS/FAILED zone)
    offset = gf_hal_get_tag_data_pos(tag);
    data = gf_hal_get_tag_data(fp, offset);
    data++;
    gf_hal_set_tag_data(fp, offset, data);
    LOG_I(LOG_TAG, "[%s] 'SUCCESS/FAILED' zone update. Current value=%d", __func__,
          data);

    // Step4: add/update metadata if authenticat failed
    if ((tag == TAG_AUTHENTICATED_FAILED_COUNT)
        || (tag == TAG_AUTHENTICATED_WITH_BIO_FAILED_COUNT))
    {
        uint32_t update_pos_offset = 0;
        int32_t update_pos_data = 0;
        // get UPDATE_POS
        update_pos_offset = gf_hal_get_tag_data_pos(TAG_UPDATE_POS);
        update_pos_data = gf_hal_get_tag_data(fp, update_pos_offset);

        // update the metadata. if buf is full, loop covery the oldest one
        if (update_pos_data >= FRR_DATABASE_METADATA_MAX)
        {
            uint32_t buf_full_offset = 0;
            int32_t buf_full_data = 0;
            update_pos_data = 0;
            // get BUF_FULL and update it if necessary
            buf_full_offset = gf_hal_get_tag_data_pos(TAG_BUF_FULL);
            buf_full_data = gf_hal_get_tag_data(fp, buf_full_offset);

            if (buf_full_data == 0)
            {
                buf_full_data = 1;
                gf_hal_set_tag_data(fp, buf_full_offset, buf_full_data);
                LOG_I(LOG_TAG, "[%s] BUF Full ", __func__);
            }
        }

        gf_hal_set_metadata(fp, is_auth_sucess, update_pos_data, chip_support_bio_data,
                            is_open_bio_data, image_quality, image_area);
        update_pos_data++;
        LOG_I(LOG_TAG, "[%s] UPDATE_POS in value=%d", __func__, update_pos_data);
        gf_hal_set_tag_data(fp, update_pos_offset, update_pos_data);
    }  // end if Step4

    VOID_FUNC_EXIT();
    return;
}

/**
 * Function: gf_hal_update_database
 * Description: Update authenticat result or metadata to database file.
 * Input: is_auth_sucess, image_quality, image_area
 * Output: None
 * Return: gf_error_t
 */
static gf_error_t gf_hal_update_database(gf_error_t is_auth_sucess,
                                         int32_t image_quality,
                                         int32_t image_area)
{
    gf_error_t err = GF_SUCCESS;
    FILE *fp = NULL;
    FUNC_ENTER();

    do
    {
        fp = fopen(FRR_DATABASE_FILE, "r+");

        if (NULL == fp)
        {
            err = GF_ERROR_FILE_OPEN_FAILED;
            LOG_E(LOG_TAG, "[%s] Open file %s failed. err=%s, errno=%d", __func__,
                  FRR_DATABASE_FILE, gf_strerror(err), err);
            break;
        }

        gf_hal_update(fp, is_auth_sucess, image_quality, image_area);
    }
    while (0);

    if (fp != NULL)
    {
        fclose(fp);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_handle_frr_database
 * Description: Interface for update database. if database file not exsit, create it.
 * Input: is_auth_sucess, image_quality, image_area
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_handle_frr_database(gf_error_t is_auth_sucess,
                                      int32_t image_quality,
                                      int32_t image_area)
{
    gf_error_t err = GF_SUCCESS;
    int32_t ret = 0;
    FUNC_ENTER();

    do
    {
        // database file not exsit create it
        ret = access(FRR_DATABASE_FILE, F_OK);
        if (0 != ret)
        {
            ret = access(FRR_DATABASE_DIR, F_OK);
            if (0 != ret)
            {
                LOG_I(LOG_TAG, "[%s] frr database file and dir not exist ", __func__);
                ret = fs_mkdirs(FRR_DATABASE_DIR, 0755);

                if (ret < 0)
                {
                    LOG_E(LOG_TAG, "[%s] mkdir %s failed. ret=%d", __func__, FRR_DATABASE_DIR, ret);
                    err = GF_ERROR_MKDIR_FAILED;
                    break;
                }

                LOG_I(LOG_TAG, "[%s] mkdir %s success ", __func__, FRR_DATABASE_DIR);
            }

            err = gf_hal_create_frr_database();

            if (err != GF_SUCCESS)
            {
                break;
            }

            LOG_I(LOG_TAG, "[%s] create file %s success ", __func__, FRR_DATABASE_DIR);
        }

        err = gf_hal_update_database(is_auth_sucess, image_quality, image_area);
    }  // do gf_hal_handle_frr_database
    while (0);

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_handle_frr_database
 * Description: If chip type is incompatible with database, delete database file.
 * Input: chip_type, chip_series
 * Output: None
 * Return: gf_error_t
 */
gf_error_t gf_hal_judge_delete_frr_database(gf_chip_type_t chip_type,
                                            gf_chip_series_t chip_series)
{
    gf_error_t err = GF_SUCCESS;
    int32_t ret = 0;
    FILE *fp = NULL;
    uint32_t chip_type_offset = 0;
    uint32_t pre_chip_type = 0;
    FUNC_ENTER();
    UNUSED_VAR(chip_series);

    do
    {
        ret = access(FRR_DATABASE_FILE, F_OK);
        if (0 == ret)
        {
            fp = fopen(FRR_DATABASE_FILE, "r");

            if (NULL == fp)
            {
                err = GF_ERROR_FILE_OPEN_FAILED;
                LOG_E(LOG_TAG, "[%s] Open file %s failed. err=%s, errno=%d", __func__,
                      FRR_DATABASE_FILE, gf_strerror(err), err);
                break;
            }

            // get 'chip_type' TAG
            chip_type_offset = gf_hal_get_tag_data_pos(TAG_CHIP_TYPE);
            pre_chip_type = gf_hal_get_tag_data(fp, chip_type_offset);
            LOG_D(LOG_TAG, "[%s] chip_type_offset=%u; pre_chip_type=%d; chip_type=%d ",
                  __func__,
                  chip_type_offset, pre_chip_type, chip_type);

            if (pre_chip_type != chip_type)
            {
                fclose(fp);
                fp = NULL;
                err = remove(FRR_DATABASE_FILE);

                if (err != 0)
                {
                    LOG_E(LOG_TAG, "[%s] delete file %s failed. err=%d", __func__,
                          FRR_DATABASE_FILE, err);
                    err = GF_ERROR_FILE_DELETE_FAILED;
                    break;
                }

                LOG_D(LOG_TAG, "[%s] delete file %s success ", __func__, FRR_DATABASE_FILE);
            }
        }  // end if: access(FRR_DATABASE_FILE, F_OK)
    }  // do gf_hal_judge_delete_frr_database
    while (0);

    if (fp != NULL)
    {
        fclose(fp);
    }

    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_get_template_length
 * Description: get template length
 * Input: group_id, finger_id
 * Output: template_len
 * Return: gf_error_t
 */
static gf_error_t hal_get_template_length(uint32_t group_id, uint32_t finger_id, uint32_t *template_len)
{
    gf_error_t err = GF_SUCCESS;

    gf_dump_template_t *template = NULL;
    uint32_t template_size = sizeof(gf_dump_template_t);

    do
    {
        if (NULL == template_len)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        template = (gf_dump_template_t *) GF_MEM_MALLOC(template_size);
        if (NULL == template)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, template", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        template->finger_id = finger_id;
        template->group_id = group_id;
        err = gf_hal_common_dump_invoke_command(GF_CMD_DUMP_TEMPLATE, template, template_size);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] dump template info error", __func__);
            err = GF_ERROR_GENERIC;
            break;
        }
        LOG_D(LOG_TAG, "[%s] dump template info len:%u", __func__, template->template_len);
        *template_len = template->template_len;
    }  // end do
    while (0);
    if (template != NULL)
    {
        GF_MEM_FREE(template);
    }

    return err;
}

/**
 * Function: gf_hal_write_auth_record
 * Description: write auth data to file, include finger id, authenticate time,
 *              authenticate result, tenplate size and match score
 * Input: authenticate data in buf, length of buf
 * Output: buf
 * Return: gf_error_t
 */
static gf_error_t gf_hal_write_auth_record(uint8_t*buf, uint32_t len)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    FILE* file = NULL;
    do
    {
        gf_stat_t file_data;
        if (NULL == buf || 0 == len)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (access(AUTH_RATIO_FILENAME, F_OK) == 0)
        {
            if (stat(AUTH_RATIO_FILENAME, &file_data) != 0)
            {
                err = GF_ERROR_FILE_NOT_EXIST;
                LOG_E(LOG_TAG, "[%s]read %s file size error", __func__,
                        AUTH_RATIO_FILENAME);
                break;
            }
            if (file_data.st_size >= AUTH_RATIO_FILE_MAX)
            {
                if (remove(AUTH_RATIO_FILENAME_BAK) != 0)
                {
                    LOG_E(LOG_TAG, "[%s]remove %s file not exist", __func__,
                        AUTH_RATIO_FILENAME_BAK);
                }
                if (rename(AUTH_RATIO_FILENAME, AUTH_RATIO_FILENAME_BAK) != 0)
                {
                    err = GF_ERROR_FILE_NOT_EXIST;
                    LOG_E(LOG_TAG, "[%s]rename %s file error", __func__,
                        AUTH_RATIO_FILENAME);
                    break;
                }
            }  // end if
        }  // end if

        file = fopen(AUTH_RATIO_FILENAME, "a+");
        if (file == NULL)
        {
            err = GF_ERROR_FILE_OPEN_FAILED;
            LOG_E(LOG_TAG, "[%s] %s can't open file. err=%s, errno=%d", __func__,
                AUTH_RATIO_FILENAME, gf_strerror(err), err);
            break;
        }
        fwrite(buf, 1, len, file);
    }  // end do
    while (0);
    if (file != NULL)
    {
        fflush(file);
        fclose(file);
        file = NULL;
    }
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: read_data_from_file
 * Description: read data from file name
 * Input: file_name, count
 * Output: buf, count
 * Return: gf_error_t
 */
static gf_error_t read_data_from_file(char* file_name, uint8_t* buf, uint32_t* count)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    FILE* file = NULL;
    do
    {
        uint32_t read_size = 0;
        uint8_t temp_buf[AUTH_RATIO_FILE_MAX + AUTH_RATIO_RECORD_LEN];
        if (buf == NULL || count == NULL)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }
        file = fopen(file_name, "r");
        if (file == NULL)
        {
            err = GF_ERROR_FILE_OPEN_FAILED;
            LOG_E(LOG_TAG, "[%s] %s can't open file. err=%s, errno=%d", __func__,
                file_name, gf_strerror(err), err);
            *count = 0;
            break;
        }
        read_size = fread(temp_buf, 1, AUTH_RATIO_FILE_MAX + AUTH_RATIO_RECORD_LEN, file);
        LOG_D(LOG_TAG, "[%s] read size:%u", __func__, read_size);
        if (read_size <= (*count) * AUTH_RATIO_RECORD_LEN)
        {
            *count = read_size / AUTH_RATIO_RECORD_LEN;
            memcpy(buf, temp_buf, read_size);
        }
        else
        {
            memcpy(buf, temp_buf + read_size - (*count) * AUTH_RATIO_RECORD_LEN, (*count) * AUTH_RATIO_RECORD_LEN);
        }
    }  // end do
    while (0);
    if (file != NULL)
    {
        fclose(file);
    }
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_read_auth_record
 * Description: read authenticate data from file,include finger id, authenticate time,
 *              authenticate result, tenplate size and match score
 * Input: buf, len
 * Output: authenticate data in buf
 * Return: gf_error_t
 */
static gf_error_t gf_hal_read_auth_record(uint8_t* buf, uint32_t* len)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    uint8_t* temp_buf = NULL;
    uint8_t* temp_buf_bak = NULL;
    uint32_t tag = 0;
    uint32_t total_count = 0;
    uint32_t record_buf_len = 0;
    uint32_t current = 0;
    do
    {
        uint32_t count = AUTH_RATIO_READ_COUNT;
        uint32_t count_bak = 0;
        temp_buf = (uint8_t *) GF_MEM_MALLOC(AUTH_RATIO_BUF_LEN);
        if (NULL == temp_buf)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, temp_buf", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        err = read_data_from_file(AUTH_RATIO_FILENAME, temp_buf, &count);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] %s read file error. err=%s, errno=%d", __func__,
                AUTH_RATIO_FILENAME, gf_strerror(err), err);
            break;
        }
        LOG_D(LOG_TAG, "[%s] count:%u", __func__, count);
        if (count < AUTH_RATIO_READ_COUNT)
        {
            temp_buf_bak = (uint8_t *) GF_MEM_MALLOC(AUTH_RATIO_BUF_LEN);
            if (NULL == temp_buf_bak)
            {
                LOG_E(LOG_TAG, "[%s] out of memory, temp_buf_bak", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            count_bak = AUTH_RATIO_READ_COUNT - count;
            read_data_from_file(AUTH_RATIO_FILENAME_BAK, temp_buf_bak, &count_bak);
            LOG_D(LOG_TAG, "[%s] count_bak:%u", __func__, count_bak);
        }
        tag = TEST_TOKEN_AUTH_COUNT;
        memcpy(buf, &tag, sizeof(uint32_t));
        current += sizeof(uint32_t);
        total_count = count + count_bak;  // the count of authentciate fingers be readed
        memcpy(buf + current, &total_count, sizeof(uint32_t));
        current += sizeof(uint32_t);

        tag = TEST_TOKEN_AUTH_DATA;
        memcpy(buf + current, &tag, sizeof(uint32_t));
        current += sizeof(uint32_t);
        record_buf_len = total_count * AUTH_RATIO_RECORD_LEN;
        memcpy(buf + current, &record_buf_len, sizeof(uint32_t));
        current += sizeof(uint32_t);

        if (temp_buf_bak != NULL && count_bak != 0)
        {
            memcpy(buf + current, temp_buf_bak, count_bak * AUTH_RATIO_RECORD_LEN);
            current += count_bak * AUTH_RATIO_RECORD_LEN;
            LOG_D(LOG_TAG, "[%s] bak current:%u", __func__, current);
        }
        memcpy(buf + current, temp_buf, count * AUTH_RATIO_RECORD_LEN);
        current += count * AUTH_RATIO_RECORD_LEN;
        *len = current;
        LOG_D(LOG_TAG, "[%s] current:%u", __func__, current);
    }  // end do
    while (0);
    if (temp_buf != NULL)
    {
        GF_MEM_FREE(temp_buf);
    }
    if (temp_buf_bak != NULL)
    {
        GF_MEM_FREE(temp_buf_bak);
    }
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_save_auth_ratio_record
 * Description: get authenticate ratio database, include finger id, authenticate time,
 *              authenticate result, tenplate size and match score
 * Input: cmd
 * Output: None
 * Return: None
 */
void gf_hal_save_auth_ratio_record(gf_irq_t* cmd)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;
    uint32_t result = 0;
    uint32_t template_size = 0;
    uint32_t match_score = 0;
    uint32_t finger_id = 0;
    int64_t timestamp;
    uint8_t* buf = NULL;

    do
    {
        uint8_t* current = NULL;
        if (NULL == cmd)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            break;
        }
        finger_id = cmd->finger_id;
        match_score = cmd->dump_performance.match_score;
        timestamp = gf_hal_current_time_microsecond();
        if (finger_id != 0)
        {
            err = hal_get_template_length(cmd->group_id, cmd->finger_id, &template_size);
            if (err != GF_SUCCESS)
            {
                LOG_E(LOG_TAG, "[%s] get template length error", __func__);
                break;
            }
        }
        if (finger_id == 0)  //  fail
        {
            result = 0;
        }
        else if (cmd->update_stitch_flag == 1)  //  update
        {
            result = 2;
        }
        else  //  sucess
        {
            result = 1;
        }
        buf = (uint8_t *) GF_MEM_MALLOC(AUTH_RATIO_RECORD_LEN);
        if (NULL == buf)
        {
            LOG_E(LOG_TAG, "[%s] out of memory, buf", __func__);
            err = GF_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(buf, 0, AUTH_RATIO_RECORD_LEN);
        current = buf;
        memcpy(current, &timestamp, sizeof(uint64_t));
        current += sizeof(uint64_t);
        memcpy(current, &result, sizeof(uint32_t));
        current += sizeof(uint32_t);
        memcpy(current, &match_score, sizeof(uint32_t));
        current += sizeof(uint32_t);
        memcpy(current, &finger_id, sizeof(uint32_t));
        current += sizeof(uint32_t);
        memcpy(current, &template_size, sizeof(uint32_t));

        err = gf_hal_write_auth_record(buf, AUTH_RATIO_RECORD_LEN);
    }  // end do
    while (0);

    if (buf != NULL)
    {
        GF_MEM_FREE(buf);
    }
    FUNC_EXIT(err);
}

/**
 * Function: gf_hal_get_auth_ratio_record
 * Description: get authenticate ratio database
 * Input: buff
 * Output: buff, len
 * Return: gf_error_t
 */
gf_error_t gf_hal_get_auth_ratio_record(uint8_t* buff, uint32_t* len)
{
    FUNC_ENTER();
    gf_error_t err = GF_SUCCESS;

    do
    {
        if (NULL == buff || NULL == len || *len < AUTH_RATIO_BUF_LEN + sizeof(uint32_t) * 4)
        {
            LOG_E(LOG_TAG, "[%s] bad parameter", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        err = gf_hal_read_auth_record(buff, len);
        if (err != GF_SUCCESS)
        {
            LOG_E(LOG_TAG, "[%s] read authenticate ratio data error", __func__);
            break;
        }
    }
    while (0);
    FUNC_EXIT(err);
    return err;
}
