/*
 * Copyright (C) 2020, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][DataSynchronizer]"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "DataSynchronizer.h"
#include "CaEntry.h"
#include "HalLog.h"
#include "HalContext.h"

#define  SYNC_OP_SAVE 1

#ifndef PERSIST_DATA_BACKUP_DIR
#define PERSIST_DATA_BACKUP_DIR "data/media/0"
#endif  // PERSIST_DATA_BACKUP_DIR

#ifndef MT_CALI_DATA_PATH
#define MT_CALI_DATA_PATH "/data/mt_cali/"
#endif  // MT_CALI_DATA_PATH

#ifndef DEVICE_CALI_DATA_PATH
#define DEVICE_CALI_DATA_PATH "/data/device_cali/"
#endif  // DEVICE_CALI_DATA_PATH

namespace goodix {

    DataSynchronizer::DataSynchronizer(HalContext *context) : DataSynchronizer(context, (char *)PERSIST_DATA_BACKUP_DIR) {  // NOLINT(432)
    }

    DataSynchronizer::DataSynchronizer(HalContext *context, char* savePath) :
            HalBase(context),
            mDataRoot(savePath),
            mAsyncQueue("DataSyncThread"),
            mIsMtCaliSave(false) {
        LOG_D(LOG_TAG, "[%s] mDataRoot=%s", __func__, mDataRoot);
    }

    DataSynchronizer::~DataSynchronizer() {
        mContext->mMsgBus.removeMsgListener(this);
        mAsyncQueue.stop();
    }

    gf_error_t DataSynchronizer::init() {
        gf_error_t err = GF_SUCCESS;
        uint64_t t0 = 0;
        uint64_t t1 = 0;
        FUNC_ENTER();

        do {
            mDataRoot = (char *)PERSIST_DATA_BACKUP_DIR;
            t0 = HalUtils::getCurrentTimeMicrosecond();
            err = listFilesAndInject();
            t1 = HalUtils::getCurrentTimeMicrosecond();
            LOG_D(LOG_TAG, "[%s] inject persist data to tee time = %dms", __func__, (int32_t)((t1 - t0)/ 1000));
            GF_ERROR_BREAK(err);
            mDataRoot = (char *)MT_CALI_DATA_PATH;
            err = listFilesAndInject();
            GF_ERROR_BREAK(err);
            mContext->mMsgBus.addMsgListener(this);
            mAsyncQueue.start();
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DataSynchronizer::injectFile(char *file_name) {
        gf_error_t err = GF_SUCCESS;
        int32_t fileSize = 0;
        int32_t totalSize = 0;
        int fd = -1;
        uint8_t *readBuf = nullptr;
        uint8_t *tmpBuf = nullptr;
        char abs_path[MAX_FILE_ROOT_PATH_LEN] = { 0 };
        gf_data_sync_info_t cmd = {{ 0 }};

        FUNC_ENTER();
        do {
            if (nullptr == file_name) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            snprintf(abs_path, sizeof(abs_path), "%s/%s", mDataRoot, file_name);
            LOG_D(LOG_TAG, "%s will inject to tee", abs_path);

            fd = open(abs_path, O_RDONLY);
            if (fd < 0) {
                LOG_E(LOG_TAG, "[%s] Read file: %s failed.", __func__, abs_path);
                break;
            }
            fileSize = lseek(fd, 0, SEEK_END);
            if (fileSize < 0) {
                LOG_E(LOG_TAG, "[%s] Seek file: %s failed.", __func__, abs_path);
                break;
            }
            totalSize = fileSize + MAX_FILE_ROOT_PATH_LEN;
            readBuf = new uint8_t[totalSize];
            if (nullptr == readBuf) {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memset(readBuf, 0, fileSize);

            memcpy(readBuf, file_name, strlen(file_name) + 1);
            err = HalUtils::readFile(mDataRoot, file_name, readBuf + MAX_FILE_ROOT_PATH_LEN, fileSize);
            GF_ERROR_BREAK(err);

            cmd.header.target = GF_TARGET_DATA_SYNC;
            if (0 == strncmp(mDataRoot, MT_CALI_DATA_PATH, strlen(MT_CALI_DATA_PATH))) {
                cmd.header.cmd_id = GF_CMD_DATA_SYNC_MTCALI_TO_TEE;
                LOG_E(LOG_TAG, "[%s] path : %s", __func__, MT_CALI_DATA_PATH);
            } else {
                cmd.header.cmd_id = GF_CMD_DATA_SYNC_INIT_INJECT;
                LOG_E(LOG_TAG, "[%s] path : %s", __func__, PERSIST_DATA_BACKUP_DIR);
            }
            cmd.total_len = totalSize;
            cmd.remaining_len = cmd.total_len;
            tmpBuf = readBuf;
            // first inject data
            cmd.first_trans = 1;

            do {
                tmpBuf += cmd.actual_len;
                memset(cmd.buffer, 0, sizeof(cmd.buffer));
                if (cmd.remaining_len <= CA_TO_TA_MAX_BUFFER) {
                    cmd.actual_len = cmd.remaining_len;
                    cmd.remaining_len = 0;
                } else {
                    cmd.actual_len = CA_TO_TA_MAX_BUFFER;
                    cmd.remaining_len -= CA_TO_TA_MAX_BUFFER;
                }
                memcpy(cmd.buffer, tmpBuf, cmd.actual_len);
                err = invokeCommand(&cmd, sizeof(gf_data_sync_info_t));
                GF_ERROR_BREAK(err);
                cmd.first_trans = 0;
            } while (cmd.remaining_len != 0);
        } while (0);

        if (fd >= 0) {
            close(fd);
        }
        if (nullptr != readBuf) {
            delete []readBuf;
        }
        FUNC_EXIT(err);

        return err;
    }

    gf_error_t DataSynchronizer::listFilesAndInject() {
        gf_error_t err = GF_SUCCESS;
        DIR *dp = nullptr;
        struct dirent *entry;
        struct stat st;
        char path[MAX_FILE_ROOT_PATH_LEN] = { 0 };
        char *name = nullptr;

        FUNC_ENTER();
        do {
            if ((dp = opendir(mDataRoot)) == nullptr) {
                LOG_D(LOG_TAG, "[%s] Directory %s is not existed", __func__, mDataRoot);
                break;
            }
            while ((entry = readdir(dp)) != nullptr) {
                name = entry->d_name;
                snprintf(path, sizeof(path), "%s/%s", mDataRoot, name);
                if (0 != lstat(path, &st)) {
                    LOG_E(LOG_TAG, "[%s] %s stat wrong.", __func__, path);
                    break;
                }
                if (!S_ISDIR(st.st_mode)) {
                    err = injectFile(name);
                    if (err != GF_SUCCESS) {
                        LOG_E(LOG_TAG, "[%s] injectFile %s fail", __func__, path);
                    }
                }
            }
        } while (0);

        if (dp != nullptr) {
            closedir(dp);
        }
        FUNC_EXIT(err);

        return err;
    }

    gf_error_t DataSynchronizer::postDataSyncMessage(AsyncMessage *e) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (nullptr == e) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            if (nullptr == e->mpHandler) {
                e->mpHandler = this;
            }

            mAsyncQueue.postAsyncMessage(e);
            LOG_D(LOG_TAG, "[%s] enqueue data sync message<%d>", __func__, e->params1);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DataSynchronizer::getDDIC(int8_t *buf, int32_t buf_size, int32_t *len) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        UNUSED_VAR(buf);
        UNUSED_VAR(buf_size);
        UNUSED_VAR(len);
        do {
            if (nullptr == buf) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DataSynchronizer::doWork(AsyncMessage *e) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            if (nullptr == e) {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            switch (e->params1) {
                case SYNC_OP_SAVE: {
                    int8_t ddic_buf[MAX_FILE_ROOT_PATH_LEN] = { 0 };
                    int8_t temp[MAX_FILE_ROOT_PATH_LEN] = { 0 };
                    int32_t ddic_len = 0;
                    char *pos;
                    getDDIC(ddic_buf, MAX_FILE_ROOT_PATH_LEN, &ddic_len);
                    if (0 == ddic_len) {
                        err = HalUtils::writeFile(mDataRoot, (char*)e->data,
                            (uint8_t *)e->data + MAX_FILE_ROOT_PATH_LEN, (e->dataLen - MAX_FILE_ROOT_PATH_LEN));
                    } else {
                        memcpy(temp, (const void *)e->data, MAX_FILE_ROOT_PATH_LEN);
                        pos = strchr((char *)temp, '_');
                        if (nullptr != pos) {
                            memcpy(ddic_buf + ddic_len, pos, strlen(pos)+1);
                        }
                        err = HalUtils::writeFile(mDataRoot, (char*)ddic_buf,
                            (uint8_t *)e->data + MAX_FILE_ROOT_PATH_LEN, (e->dataLen - MAX_FILE_ROOT_PATH_LEN));
                    }
                    if (GF_SUCCESS != err) {
                        LOG_D(LOG_TAG, "[%s] save %s fail.", __func__, e->data);
                    }
                    if (nullptr != e->data) {
                        delete [](e->data);
                    }
                    break;
                }

                default: {
                    break;
                }
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DataSynchronizer::onMessage(const MsgBus::Message &message) {
        gf_error_t err = GF_SUCCESS;
        uint8_t data = 0;
        uint64_t t0 = 0;
        uint64_t t1 = 0;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] message.msg=%d",  __func__, message.msg);
        switch (message.msg) {
            case MsgBus::MSG_PERSIST_DATA_CHANGE: {
                data = *(uint8_t *)(message.data);
                mDataRoot = (char *)PERSIST_DATA_BACKUP_DIR;
                if (GF_DATA_SYNC_MT_CALI == data) {
                    mDataRoot = (char *)DEVICE_CALI_DATA_PATH;
                    mIsMtCaliSave = true;
                }
                if (GF_DATA_SYNC_MT_CALI != data) {
                    t0 = HalUtils::getCurrentTimeMicrosecond();
                }
                err = onPersistDataChange();
                if (GF_DATA_SYNC_MT_CALI != data) {
                    t1 = HalUtils::getCurrentTimeMicrosecond();
                    LOG_D(LOG_TAG, "[%s] sync persist data from tee time = %dms", __func__, (int32_t)((t1 - t0)/ 1000));
                }

                break;
            }

            default:
                break;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t DataSynchronizer::onPersistDataChange() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        uint8_t *tmpBuf = nullptr;
        uint8_t *buf = nullptr;
        uint32_t remainingFileCount = 1;
        gf_data_sync_info_t cmd = {{ 0 }};
        AsyncMessage *entity = nullptr;
        do {
            while (remainingFileCount > 0) {
                buf = nullptr;
                entity = new AsyncMessage();
                memset(entity, 0, sizeof(AsyncMessage));
                memset(&cmd, 0, sizeof(gf_data_sync_info_t));
                cmd.header.target = GF_TARGET_DATA_SYNC;
                cmd.header.cmd_id = GF_CMD_DATA_SYNC_FROM_TEE;
                if (mIsMtCaliSave) {
                    cmd.header.cmd_id = GF_CMD_DATA_SYNC_CALI_FROM_TEE;
                }
                // first fetch data
                cmd.first_trans = 1;
                err = invokeCommand(&cmd, sizeof(gf_data_sync_info_t));
                GF_ERROR_BREAK(err);
                tmpBuf = new uint8_t[cmd.total_len];
                remainingFileCount = cmd.remaining_file_count;

                if (nullptr == tmpBuf) {
                    err = GF_ERROR_OUT_OF_MEMORY;
                    break;
                }

                memset(tmpBuf, 0, cmd.total_len);
                buf = tmpBuf;
                memcpy(tmpBuf, cmd.buffer, cmd.actual_len);

                while (cmd.remaining_len != 0) {
                    tmpBuf += cmd.actual_len;
                    LOG_D(LOG_TAG, "[%s] cmd.actual_len = %d", __func__, cmd.actual_len);
                    memset(cmd.buffer, 0, sizeof(cmd.buffer));
                    cmd.first_trans = 0;
                    err = invokeCommand(&cmd, sizeof(gf_data_sync_info_t));
                    GF_ERROR_BREAK(err);
                    memcpy(tmpBuf, cmd.buffer, cmd.actual_len);
                }
                GF_ERROR_BREAK(err);
                entity->mpHandler = this;
                entity->data = buf;
                entity->dataLen = cmd.total_len;
                entity->params1 = SYNC_OP_SAVE;
                postDataSyncMessage(entity);
            }
        } while (0);

        if (err != GF_SUCCESS) {
            if (nullptr != buf) {
                delete[] buf;
                buf = nullptr;
            }
            if (nullptr != entity) {
                delete entity;
                entity = nullptr;
            }
        }

        FUNC_EXIT(err);
        return err;
    }
}  // namespace goodix
