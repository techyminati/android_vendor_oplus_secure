/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "HalLog.h"
#include "PersistData.h"
#include <stdlib.h>

#define LOG_TAG "[GF_HAL][PersistData]"

#define PERSIST_DATA_PATH "/mnt/vendor/persist/cali_data.so"

namespace goodix {
    PersistData::PersistData() {
    }

    PersistData::~PersistData() {
    }

    gf_error_t PersistData::getPersistData(uint8_t **data, uint32_t *datalen) {
        gf_error_t err = GF_SUCCESS;
        FILE* file = NULL;
        int32_t filesize = 0;
        uint8_t *filedata = NULL;

        FUNC_ENTER();

        do {
            file = fopen(PERSIST_DATA_PATH, "rb");
            if (file == NULL) {
                LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, PERSIST_DATA_PATH);
                err = GF_ERROR_FILE_OPEN_FAILED;
                break;
            }

            if (fseek(file, 0L, SEEK_END) != 0) {
                LOG_E(LOG_TAG, "[%s] Cannot read file", __func__);
                err = GF_ERROR_GENERIC;
                break;
            }

           filesize = ftell(file);
           if (filesize < 0) {
               LOG_E(LOG_TAG, "[%s] Cannot get the file size", __func__);
               err = GF_ERROR_GENERIC;
               break;
           }
           if (filesize == 0) {
               LOG_E(LOG_TAG, "[%s] File size is not correct", __func__);
               err = GF_ERROR_GENERIC;
               break;
           }

           /* Set the file pointer at the beginning of the file */
           if (fseek(file, 0L, SEEK_SET) != 0)
           {
                LOG_E(LOG_TAG, "[%s] Cannot read file", __func__);
                err = GF_ERROR_GENERIC;
                break;
           }

           /* Allocate a buffer for the content */
           filedata = (uint8_t*)malloc(filesize);
           if (filedata == NULL) {
                LOG_E(LOG_TAG, "[%s] Out of memory", __func__);
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
           }

            /* Read data from the file into the buffer */
            if (fread(filedata, (size_t)filesize, sizeof(uint8_t), file) != 1) {
                LOG_E(LOG_TAG, "[%s] Cannot read file", __func__);
                err = GF_ERROR_GENERIC;
                break;
            }

            *data = filedata;
            *datalen = filesize;
        } while (0);

        if (GF_SUCCESS != err && NULL != filedata) {
            free(filedata);
        }

        if (NULL != file) {
            fclose(file);
        }

        FUNC_EXIT(err);
        return err;
}

    gf_error_t PersistData::savePersistData(uint8_t *data, uint32_t datalen) {
        gf_error_t err = GF_SUCCESS;
        FILE *file = NULL;

        FUNC_ENTER();

        do
        {
            file = fopen(PERSIST_DATA_PATH, "wb");
            if (NULL == file)
            {
                LOG_E(LOG_TAG, "[%s] open file (%s) fail", __func__, PERSIST_DATA_PATH);
                err = GF_ERROR_FILE_OPEN_FAILED;
                break;
            }
            int32_t ret = fwrite(data, sizeof(uint8_t), datalen, file);
            if (ret != (int32_t) datalen)
            {
                LOG_E(LOG_TAG, "[%s] Write file (%s) error,  ret = %d", __func__, PERSIST_DATA_PATH, ret);
                err = GF_ERROR_FILE_WRITE_FAILED;
            }
            if (file != NULL)
            {
                fflush(file);
                fclose(file);
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix

