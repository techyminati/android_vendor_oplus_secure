/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][Tracer]"
#include <errno.h>
#include "Tracer.h"
#include "HalUtils.h"
#include "HalLog.h"

#define TRACE_PATH                 "/gf_trace/"
#define TRACE_FILE_NAE          "trace.info"
#define LOG_FILE_NAME            "talog"
#define COLON_SPLITER            " : "
#define NEWLINE_SPLITER        " \n "
#define COMMA_SPLITER           ", "
#define LOG_MAX_BUFFER       (1024)
#define LOG_FILE_MAX_SIZE    (1024*1024*10)
#define LOG_FILE_COUNT_MAX (5)
namespace goodix
{
    static  void renameLogFile(const char* current_log_file)
    {
        char originpath[GF_MAX_FILE_NAME_LEN] = { 0 };
        char renamepath[GF_MAX_FILE_NAME_LEN] = { 0 };
        int32_t i = LOG_FILE_COUNT_MAX - 1;
        for (; i > 0; i--)
        {
            FILE* tmpFile = NULL;
            memset(originpath, 0, sizeof(originpath));
            snprintf(originpath, sizeof(originpath), "%s%d", current_log_file, i);
            tmpFile =  fopen(originpath, "r");
            if (tmpFile == NULL)
            {
                // file is not exist
                continue;
            }
            fclose(tmpFile);
            memset(renamepath, 0, sizeof(originpath));
            snprintf(renamepath, sizeof(renamepath), "%s%d", current_log_file, i + 1);
            if (rename(originpath, renamepath) < 0)
            {
                LOG_E(LOG_TAG, "[%s] Rename [%s] to [%s] error : %s", __func__, originpath, renamepath,  strerror(errno));
            }
        }
        if (i == 0)
        {
            memset(renamepath, 0, sizeof(originpath));
            snprintf(renamepath, sizeof(renamepath), "%s%d", current_log_file, i + 1);
            if (rename(current_log_file, renamepath) < 0)
            {
                LOG_E(LOG_TAG, "[%s] Rename [%s] to [%s] error : %s.", __func__, current_log_file, renamepath, strerror(errno));
            }
            if (remove(current_log_file) < 0)
            {
                LOG_E(LOG_TAG, "[%s] Remove [%s] error : %s", __func__, current_log_file, strerror(errno));
            }
        }
    }

    Tracer* Tracer::mTracer = nullptr;

    Tracer::~Tracer()
    {
        mTracer = nullptr;
    }

    Tracer* Tracer::getInstance(HalContext* context)
    {
        if (nullptr == mTracer)
        {
            mTracer = new Tracer(context);
        }
        return mTracer;
    }

    Tracer::Tracer(HalContext* context) :
            HalBase(context), mAsyncQueue("Tracer")
    {
    }

    void Tracer::start()
    {
        FUNC_ENTER();
        char filepath[GF_MAX_FILE_NAME_LEN] = { 0 };
        snprintf(filepath, sizeof(filepath), "%s%s", GF_DUMP_DATA_ROOT_PATH, TRACE_PATH);
        mEncoder.init(true, filepath);
        mEncoder.setEncryptDir(".");

        if (!isRunning())
        {
            run("Tracer");
            mAsyncQueue.start();
            mContext->mMsgBus.addMsgListener(this);
        }
        VOID_FUNC_EXIT();
    }

    void Tracer::stop()
    {
        FUNC_ENTER();
        if (isRunning())
        {
            requestExit();
            mAsyncQueue.stop();
        }
        VOID_FUNC_EXIT();
    }

    gf_error_t Tracer::onMessage(const MsgBus::Message& msg)
    {
        gf_error_t err = GF_SUCCESS;
        switch (msg.msg)
        {
            case MsgBus::MSG_AUTHENTICATE_REQUESTED:
            case MsgBus::MSG_ENROLL_REQUESTED:
            case MsgBus::MSG_AUTHENTICATE_END:
            case MsgBus::MSG_ENROLL_END:
            {
                gf_error_t err = GF_SUCCESS;
                gf_data_info_t data_cmd;
                memset(&data_cmd, 0, sizeof(gf_data_info_t));
                data_cmd.header.target = GF_TARGET_TRACE;
                uint8_t* data = nullptr;
                uint32_t data_len = 0;
                err = getTraceDataFromTa(&data_cmd, &data, &data_len);
                if (err == GF_SUCCESS)
                {
                    AsyncMessage* entity = new AsyncMessage();
                    HalUtils::genTimestamp(entity->timestamp, sizeof(entity->timestamp));
                    entity->data = data;
                    entity->dataLen = data_len;
                    entity->mpHandler = this;
                    mAsyncQueue.postAsyncMessage(entity);
                }
            }
                break;
            default:
                break;
        }
        return err;
    }

    gf_error_t Tracer::getTraceDataFromTa(gf_data_info_t* cmd, uint8_t** data, uint32_t* data_len)
    {
        gf_error_t err = GF_SUCCESS;
        uint8_t* tmpBuf = NULL;
        uint8_t* buf = NULL;
        do
        {
            if (nullptr == cmd || nullptr == data || nullptr == data_len)
            {
                err = GF_ERROR_BAD_PARAMS;
                break;
            }
            // first fetch data
            cmd->i_first_fetch_data = 1;
            err = invokeCommand(cmd, sizeof(gf_data_info_t));
            GF_ERROR_BREAK(err);
            buf = new uint8_t[cmd->o_total_len];
            if (NULL == buf)
            {
                err = GF_ERROR_OUT_OF_MEMORY;
                break;
            }
            memset(buf, 0, cmd->o_total_len);

            tmpBuf = buf;
            *data_len = cmd->o_total_len;
            if (tmpBuf +  cmd->o_actual_len <= buf + cmd->o_total_len)
            {
                memcpy(tmpBuf, cmd->o_buffer, cmd->o_actual_len);
            }
            else
            {
                err = GF_ERROR_NO_SPACE;
                break;
            }


            while (cmd->o_remaining_len != 0)
            {
                tmpBuf += cmd->o_actual_len;
                memset(cmd->o_buffer, 0, sizeof(cmd->o_buffer));
                cmd->i_first_fetch_data = 0;
                err = invokeCommand(cmd, sizeof(gf_data_info_t));
                GF_ERROR_BREAK(err);
                if (tmpBuf +  cmd->o_actual_len <= buf + cmd->o_total_len)
                {
                    memcpy(tmpBuf, cmd->o_buffer, cmd->o_actual_len);
                }
                else
                {
                    err = GF_ERROR_NO_SPACE;
                    break;
                }
            }
            GF_ERROR_BREAK(err);
        }
        while (0);

        if (err != GF_SUCCESS)
        {
            delete[] buf;
            buf = nullptr;
        }
        else
        {
            *data = buf;
        }
        return err;
    }

    static gf_error_t formatTraceInfo(Vector<int8_t>& str_buffer, gf_trace_data_t* entity)
    {
        gf_error_t err = GF_SUCCESS;
        /**
         * tag:value [func_name, line]
         */
        str_buffer.appendArray((int8_t*) entity->tag, strlen((char*) entity->tag));
        str_buffer.appendArray((int8_t*) COLON_SPLITER, strlen(COLON_SPLITER));
        str_buffer.appendArray((int8_t*) entity->data, strlen((char*) entity->data));
        str_buffer.appendArray((int8_t*) " [", strlen(" ["));
        str_buffer.appendArray((int8_t*) entity->fun_name, strlen((char*) entity->fun_name));
        str_buffer.appendArray((int8_t*) COMMA_SPLITER, strlen(COMMA_SPLITER));
        str_buffer.appendArray((int8_t*) entity->line, strlen((char*) entity->line));
        str_buffer.appendArray((int8_t*) "]", strlen("]"));
        str_buffer.appendArray((int8_t *) NEWLINE_SPLITER, strlen(NEWLINE_SPLITER));
        return err;
    }

    gf_error_t Tracer::doWork(AsyncMessage* e)
    {
        gf_error_t err = GF_SUCCESS;
        uint8_t* data = e->data;
        uint32_t data_len = e->dataLen;
        do
        {
            if (nullptr == data || 0 == data_len)
            {
                break;
            }
            uint8_t* tmp_buf = data;
            uint32_t total_item = *((uint32_t*) tmp_buf);
            tmp_buf += sizeof(uint32_t);

            Vector < int8_t > str_buffer;
            for (uint32_t i = 0; i < total_item; i++)
            {
                gf_trace_data_t* entity = (gf_trace_data_t*) tmp_buf;
                if (entity->type == 0)
                {
                    formatTraceInfo(str_buffer, entity);
                }
                else if (entity->type == 1)
                {
                    // TODO write to bin file
                }
                tmp_buf += (sizeof(gf_trace_data_t) + entity->data_size);
            }
            if (!str_buffer.isEmpty())
            {
                char filepath[256] = { 0 };
                //mEncoder.dumpBegin(false, NULL);
                snprintf(filepath, sizeof(filepath), "%s%s%s", GF_DUMP_DATA_ROOT_PATH, TRACE_PATH,
                        TRACE_FILE_NAE);
                mEncoder.dumpDataToFileDirectly(filepath, (uint8_t *) str_buffer.array(), str_buffer.size());
                mEncoder.dumpEnd();
            }
        }
        while (0);

        if (e->data != nullptr)
        {
            delete[] e->data;
        }
        return err;
    }

    bool Tracer::threadLoop()
    {
        FUNC_ENTER();
#ifdef __QSEE
        mLogManager.startCatchQseeLog();
#endif  // __QSEE
        VOID_FUNC_EXIT();
        return false;
    }

    gf_error_t startTracer(HalContext* context)
    {
        gf_error_t err = GF_SUCCESS;
        Tracer* tracer = Tracer::getInstance(context);
        tracer->start();
        return err;
    }

    void stopTracer(HalContext* context)
    {
        Tracer* tracer = Tracer::getInstance(context);
        tracer->stop();
        delete tracer;
    }

    Tracer::LogManager::LogManager() : mLogFile(nullptr), mCurrentLogFileBytes(0)
    {
        snprintf(mLogFileName, sizeof(mLogFileName), "%s%s%s", GF_DUMP_DATA_ROOT_PATH, TRACE_PATH,
                LOG_FILE_NAME);
        HalUtils::makedirsByFilePath(mLogFileName);
    }

    void Tracer::LogManager::startCatchQseeLog()
    {
        char log[LOG_MAX_BUFFER] = { 0 };
        const char* qsee_log_name = "/d/tzdbg/qsee_log";
        FILE *read_file = nullptr;
        FILE* write_file = nullptr;
        uint32_t file_len = 0;
        do
        {
            read_file = fopen(qsee_log_name, "r");
            if (nullptr == read_file)
            {
                LOG_E(LOG_TAG, "[%s] fopen (%s) error. ", __func__, qsee_log_name);
                break;
            }
            write_file = fopen(mLogFileName, "a");
            if (nullptr == write_file)
            {
                LOG_E(LOG_TAG, "[%s] fopen (%s) error. ", __func__, mLogFileName);
                break;
            }
            mLogFile = write_file;
            while (1)
            {
                file_len = fread(log, sizeof(uint8_t), sizeof(log), read_file);
                if (file_len > 0)
                {
                    bool ret = appendLog(log, file_len);
                    if (!ret)
                    {
                        break;
                    }
                }
            }
        }
        while (0);
        if (read_file != nullptr)
        {
            fclose(read_file);
        }
        if (mLogFile != nullptr)
        {
            fclose(mLogFile);
        }
    }

    bool Tracer::LogManager::appendLog(const char* log, uint32_t log_len)
    {
        if (mLogFile == nullptr)
        {
            return false;
        }

        if (log_len != fwrite(log, sizeof(uint8_t), log_len, mLogFile))
        {
            LOG_E(LOG_TAG, "[%s] write log fail", __func__);
            return false;
        }
        mCurrentLogFileBytes += log_len;
        fflush(mLogFile);
        if (mCurrentLogFileBytes > LOG_FILE_MAX_SIZE)
        {
            if (mLogFile != nullptr)
            {
                fclose(mLogFile);
                mLogFile = nullptr;
            }
            renameLogFile(mLogFileName);
            mCurrentLogFileBytes = 0;
            mLogFile = fopen(mLogFileName, "a");
            if (mLogFile == nullptr)
            {
                LOG_E(LOG_TAG, "[%s] fopen (%s) error : %s ", __func__, mLogFileName,
                        strerror(errno));
                return false;
            }
        }
        return true;
    }
}  // namespace goodix
