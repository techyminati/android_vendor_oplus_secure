/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - fingerprint/AutoSmoking/AutoSmoking/cpp
 **
 ** Description:
 **      AutoSmoking test for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,17/08/2021
 ** Author: zoulian@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  zoulian       2021/08/17        create the file
 ************************************************************************************/
#define LOG_TAG "[FP_HAL][AutoSmoking]"

#include "AutoSmoking.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <vector>


#include "FpCommon.h"
#include "HalLog.h"

namespace android
{
AutoSmoking::AutoSmoking(IAutoSmokingInterface* handle, fp_sensor_type_t type)
{
    mhandle = handle;
    mEnrollFinish = 0;
    mSensorType = type;
    memset(mSmokingFinish, 1, sizeof(mSmokingFinish));
}

AutoSmoking::~AutoSmoking()
{
}

fp_return_type_t  AutoSmoking::readFileList(const char *basePath, std::vector<std::string> &allFiles)
{
    fp_return_type_t err = FP_SUCCESS;
    DIR *dir;
    struct dirent *ptr;
    char vfiles[256];
    int num = 0;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "func:%s fullpath: %s", __func__, basePath);
    if ((dir = opendir(basePath)) == NULL) {
        LOG_E(LOG_TAG, "fullpath: %s", basePath);
        goto fp_out;
    }

    while ((ptr=readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == 8) {
            snprintf(vfiles, 256, "%s/%s", basePath, ptr->d_name);
            allFiles.push_back(vfiles);
        }
    }
    closedir(dir);
    std::sort(allFiles.begin(), allFiles.end());
    for (num = 0; num < allFiles.size(); num++) {
        LOG_D(LOG_TAG, "fullpath[%d]: %s", num, allFiles[num].c_str());
    }
fp_out:
    FUNC_EXIT(err);
    return err;
}




fp_return_type_t AutoSmoking::readDatFile(const char* fullpath, uint8_t** data, uint32_t* data_len) {
    FILE *file_handle;
    uint8_t *file_buf = NULL;
    unsigned int read_len = 0;
    unsigned int file_len = 0;
    fp_return_type_t err = FP_SUCCESS;

    FUNC_ENTER();
    LOG_E(LOG_TAG, "fullpath: %s", fullpath);

    if (fullpath == NULL || data == NULL || data_len == NULL) {
        goto fp_out;
    }

    file_handle = fopen(fullpath, "rb");
    if (file_handle == NULL) {
        err = FP_SMOKING_Files_NOT_EXIST;
        LOG_E(LOG_TAG, "func:%s open file failed path:%s, err:%d, reason:%s", __func__, fullpath, errno, strerror(errno));
        goto fp_out;
    }

    fseek(file_handle, 0, SEEK_END);
    file_len = ftell(file_handle);
    fseek(file_handle, 0, SEEK_SET);
    if (0 == file_len) {
        err = FP_SMOKING_Files_IS_DAMAGE;
        LOG_E(LOG_TAG "[%s] read file failed\n", __func__);
        goto fp_out;
    }

    file_buf = (unsigned char*)malloc(file_len);
    read_len = fread(file_buf, file_len, 1, file_handle);
    fclose(file_handle);
    *data = file_buf;
    *data_len = file_len;
fp_out:
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t AutoSmoking::switchSmokingMode(fp_injection_mode_t config) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();

    fp_inject_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.header.module_id = FP_MODULE_INJECT;
    cmd.header.cmd_id = FP_CMD_INJECT_SWITCH_MODE;
    cmd.config = config;
    err = mhandle->sendTaCommand(&cmd, sizeof(fp_inject_t));

    FUNC_EXIT(err);
    return err;
}

fp_return_type_t AutoSmoking::injectData(const char *filepath, uint8_t* data, uint32_t data_len) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    fp_inject_t *cmd = NULL;
    uint32_t total_size = sizeof(fp_inject_t) + data_len;
    cmd = (fp_inject_t *)malloc(total_size * sizeof(char));
    if (NULL == cmd) {
        LOG_E(LOG_TAG, "fp_inject_t malloc err");
        goto fp_out;
    }

    cmd->header.module_id = FP_MODULE_INJECT;
    if (0 == strncmp(filepath, AST_CALI_PATH, strlen(AST_CALI_PATH))) {
        cmd->header.cmd_id = FP_CMD_INJECT_CALI;
    } else {
        cmd->header.cmd_id = FP_CMD_INJECT_IMAGE;
    }

    memcpy(cmd->image.img_name, filepath, strlen(filepath));
    cmd->image.img_len = data_len;

    memcpy(&cmd->image.img_addr, data, data_len);
    LOG_D(LOG_TAG, "[%s] read file img_addr[0]=%d img_addr[%d]=%d\n", cmd->image.img_name, cmd->image.img_addr,
        data_len, *(&cmd->image.img_addr + data_len - 1));
    err = mhandle->sendTaCommand(cmd, total_size);

fp_out:
    if (NULL != cmd) {
        free(cmd);
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t AutoSmoking::loadCaliWithSensor()
{
    fp_return_type_t err = FP_SUCCESS;
    uint32_t num = 0;
    std::string file_type = {".cali.gx_g3s"};
    std::vector<std::string> caliFiles;
    FUNC_ENTER();
    LOG_V(LOG_TAG, "mSensorType = %d", mSensorType);
    readFileList(AST_CALI_PATH, caliFiles);

    if (FP_SENSOR_TYPE_G_G3 == mSensorType) {
        for (num = 0; num < caliFiles.size(); num++) {
            if (std::string::npos == caliFiles[num].find(file_type)) {
                LOG_E(LOG_TAG, "err cali type = %s", caliFiles[num].c_str());
                err = FP_SMOKING_Files_IS_DAMAGE;
                goto fp_out;
            }
            err = loadCaliFile(caliFiles[num].c_str());
            CHECK_RESULT_SUCCESS(err);
        }
    }
fp_out:
    caliFiles.clear();
    FUNC_EXIT(err);
    return err;
}


fp_return_type_t AutoSmoking::loadCaliFile(const char *filepath)
{
    fp_return_type_t err = FP_SUCCESS;
    uint8_t *data = NULL;
    uint32_t data_len = 0;
    FUNC_ENTER();

    LOG_E(LOG_TAG, "%s", filepath);
    err = readDatFile(filepath,  &data, &data_len);
    CHECK_RESULT_SUCCESS(err);

    err = injectData(filepath, data, data_len);
    CHECK_RESULT_SUCCESS(err);
fp_out:
    if (data != NULL) {
        free(data);
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t AutoSmoking::loadImage(const char *image_path, uint8_t file_count)
{
    fp_return_type_t err = FP_SUCCESS;
    uint8_t *data = NULL;
    uint32_t data_len = 0;
    UNUSE(file_count);
    FUNC_ENTER();
    LOG_E(LOG_TAG, "inject filepath:%s", image_path);

    err = readDatFile(image_path, &data, &data_len);
    CHECK_RESULT_SUCCESS(err);
    err = injectData(image_path, data, data_len);
    CHECK_RESULT_SUCCESS(err);
fp_out:
    if (data != NULL) {
        free(data);
    }
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t AutoSmoking::setEnrollFinish(uint32_t isFinish)
{
    LOG_E(LOG_TAG, "setEnrollFinish isFinish");
    Mutex::Autolock _l(mEnrollLock);
    mEnrollFinish = isFinish;
    return FP_SUCCESS;
}

fp_return_type_t AutoSmoking::doEnroll(const char *enroll_image_path, int enroll_totoal_times, uint32_t gid, int leave_halfway, uint8_t should_timeout) {
    fp_return_type_t err = FP_SUCCESS;
    uint8_t hat_array[AST_AUTH_TOKEN_SIZE];
    int enroll_count = 0;
    memset(hat_array, 0, AST_AUTH_TOKEN_SIZE);
    uint32_t timeoutSec = 10 * 60 * 1000 * 1000;
    uint64_t challenge = 0;
    std::vector<std::string> enrollFiles;

    FUNC_ENTER();
    setEnrollFinish(0);

    challenge = mhandle->preEnroll();
    LOG_D(LOG_TAG, "challenge : %llu\n", (unsigned long long)challenge);
    if (should_timeout) {
        usleep(timeoutSec + 1000);
        LOG_D(LOG_TAG, "enroll timeout exit");
    }

    hat_array[0] = 0;
    memcpy(&hat_array[1], &challenge, 8);
    err = mhandle->enroll(hat_array, gid, timeoutSec);
    CHECK_RESULT_SUCCESS(err);
    if (should_timeout) {
        LOG_D(LOG_TAG, "doEnroll timeout exit");
        goto fp_out;
    }
    err = readFileList(enroll_image_path, enrollFiles);
    LOG_D(LOG_TAG, "enroll enroll_count = %lu", enrollFiles.size());

    if (enroll_totoal_times > enrollFiles.size()) {
        LOG_E(LOG_TAG, "Please prepare enough image data, current %lu < enroll_totoal_times %d", enrollFiles.size(), enroll_totoal_times);
        err = FP_SMOKING_Files_IS_DAMAGE;
        goto fp_out;
    }
    if (20 == enroll_totoal_times) {
        enroll_totoal_times = enrollFiles.size();
    }
    for (enroll_count = 0; enroll_count < enroll_totoal_times; enroll_count++) {
        LOG_V(LOG_TAG, "enroll enroll_count = %d %s", enroll_count, enrollFiles[enroll_count].c_str());

        {
            Mutex::Autolock _l(mEnrollLock);
            if (mEnrollFinish) {
                LOG_E(LOG_TAG, "enroll already finish, enroll_count = %d", enroll_count);
                break;
            }
        }
        if (enroll_count == leave_halfway) {
            LOG_E(LOG_TAG, "enroll leave halfway, enroll_count = %d", enroll_count);
            break;
        }
        err = loadImage(enrollFiles[enroll_count].c_str(), enroll_count + 1);
        CHECK_RESULT_SUCCESS(err);
        mhandle->simulatedEvent(DOWN_EVNT);
        //usleep(200*1000);
        mhandle->simulatedEvent(UP_EVNT);
    }

fp_out:
    mhandle->postEnroll();
    mhandle->cancel();
    enrollFiles.clear();
    FUNC_EXIT(err);
    return err;
}

fp_return_type_t AutoSmoking::doAuthenticate(const char *auth_image_path, int auth_totoal_times, uint32_t gid)
{
    fp_return_type_t err = FP_SUCCESS;
    int auth_count = 0;
    int auth_total = 0;
    uint64_t operationId = 16524854;
    std::vector<std::string> authFiles;
    FUNC_ENTER();
    err = readFileList(auth_image_path, authFiles);
    LOG_D(LOG_TAG, "enroll enroll_count = %lu", authFiles.size());
    if (auth_totoal_times > authFiles.size()) {
        LOG_E(LOG_TAG, "Please prepare enough image data, current %lu < auth_totoal_times %d", authFiles.size(), auth_totoal_times);
        err = FP_SMOKING_Files_IS_DAMAGE;
        goto fp_out;
    }
    auth_total = auth_totoal_times < authFiles.size() ? auth_totoal_times : authFiles.size();

    for (auth_count = 0; auth_count < auth_total; auth_count++) {
        err = mhandle->authenticate(operationId, gid);
        CHECK_RESULT_SUCCESS(err);
        LOG_V(LOG_TAG, "auth_count = %d", auth_count);
        err = loadImage(authFiles[auth_count].c_str(), auth_count);
        CHECK_RESULT_SUCCESS(err);
        mhandle->simulatedEvent(DOWN_EVNT);
        //usleep(200*1000);
        mhandle->simulatedEvent(UP_EVNT);
    }
fp_out:
    mhandle->cancel();
    FUNC_EXIT(err);
    return err;
}


fp_return_type_t AutoSmoking::setCurrentFingerlist(fp_enumerate_t cmd, uint32_t current_gid) {
    LOG_E(LOG_TAG, "setCurrentFingerlist");
    fingerprint_enumerated_t flist;
    memset(&flist, 0, sizeof(fingerprint_enumerated_t));
    flist.gid = current_gid;
    for (int i = 0; i < cmd.result.finger_count; i++) {
        flist.fingers[i].gid = current_gid;
        flist.fingers[i].fid = cmd.result.finger_id[i];
        LOG_D(LOG_TAG, "[%s] group_id[%d]=%u, finger_id[%d]=%u, remains=%u", __func__,
                            i, current_gid, i, cmd.result.finger_id[i], cmd.result.finger_count);
    }
    mfingerlist = flist;
    return FP_SUCCESS;
}

fp_return_type_t AutoSmoking::removeAll() {
    fp_return_type_t err = FP_SUCCESS;
    fingerprint_enumerated_t tmp = mfingerlist;
    for (int i = 0; i < tmp.remaining_templates; i++) {
        LOG_V(LOG_TAG, "finger list = %d", tmp.fingers[i].fid);
        err = mhandle->remove(tmp.fingers[i].gid, tmp.fingers[i].fid);
        CHECK_RESULT_SUCCESS(err);
    }
fp_out:
    FUNC_EXIT(err);
    return err;
}

int32_t AutoSmoking::judgeSmokingFinsh(int32_t cmdid) {
    int32_t slot = cmdid - FINGERPRINT_SMOKING_SUPPORT;
    int32_t num = 0;
    int32_t is_finish = 1;
    if (slot >= MAX_CASE_LEN || slot <= 0) {
        LOG_E(LOG_TAG, "cmdid = %d, slot = %d", cmdid, slot);
        is_finish = 0;
        goto fp_out;
    }
    LOG_E(LOG_TAG, "slot = %d", slot);
    mSmokingFinish[slot] = 0;
    for (num = 2; num < FINGERPRINT_SMOKING_END - FINGERPRINT_SMOKING_SUPPORT; num++) {
        if (mSmokingFinish[num] > 0) {
            is_finish = 0;
            LOG_E(LOG_TAG, "case [%d] is not finish", num);
            break;
        }
    }
fp_out:
    LOG_E(LOG_TAG, "all case is_finish = %d", is_finish);
    return is_finish;
}


fp_return_type_t AutoSmoking::autoSmokingCase(int32_t cmd_id, uint32_t gid) {
    fp_return_type_t err = FP_SUCCESS;
    FUNC_ENTER();
    LOG_E(LOG_TAG, "enrollAndAuth = %d %d", cmd_id, gid);
    Mutex::Autolock _l(mSmokeLock);
    switch (cmd_id) {
        case FINGERPRINT_SMOKING_SUPPORT:
            LOG_E(LOG_TAG, "enrollAndAuth = %d %d, FINGERPRINT_SMOKING_SUPPORT", cmd_id, gid);
            err = mhandle->setActiveGroup(gid, AST_TEMPLATE_DIR);
            CHECK_RESULT_SUCCESS(err);
            err = mhandle->enumerate();
            CHECK_RESULT_SUCCESS(err);
            err = removeAll();
            CHECK_RESULT_SUCCESS(err);
            break;
        case FINGERPRINT_SMOKING_START:
            LOG_E(LOG_TAG, "enrollAndAuth = %d %d, FINGERPRINT_SMOKING_START", cmd_id, gid);
            err = switchSmokingMode(FP_INJECTION_ON);
            CHECK_RESULT_SUCCESS(err);
            err = loadCaliWithSensor();
            CHECK_RESULT_SUCCESS(err);
            memset(mSmokingFinish, 1, sizeof(mSmokingFinish));
            break;
        case FINGERPRINT_SMOKING_AUTH_SUCCESS:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 1, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_AUTH_FAIL:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 1, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_DEPLICATE:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_1, 2, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_AUTH_SUCCESS_MUL:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_2, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_3, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_4, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_5, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 5, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_AUTH_FAIL_MUL:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_2, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_3, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_4, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_5, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 5, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_AUTH_SUCCESS_AND_FAIL:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 2, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_UPPER_LIMIT:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_2, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_3, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_4, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_5, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            //err = doEnroll(AST_ENROLL_PATH_6, 2, gid, -1, 0);
            //CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_CANCEL:
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, 5, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 1, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_REPEAT:
            err = doEnroll(AST_ENROLL_PATH_REPEAT_PROCESS, 6, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_LOW_QUALITY:
            err = doEnroll(AST_ENROLL_PATH_LOW_QUALITY, 5, gid, 5, 0);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_SAMLL_AREA:
            err = doEnroll(AST_ENROLL_PATH_SMALL_AREA, 2, gid, 5, 0);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_TIMEOUT:
            err = doEnroll(AST_ENROLL_PATH_1, 1, gid, -1, 1);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_2, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 2, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_ENROLL_MANAY_ERR:
            err = doEnroll(AST_ENROLL_PATH_LOW_QUALITY, 11, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doEnroll(AST_ENROLL_PATH_1, 20, gid, -1, 0);
            CHECK_RESULT_SUCCESS(err);
            err = doAuthenticate(AST_AUTH_PATH, 2, gid);
            CHECK_RESULT_SUCCESS(err);
            removeAll();
            break;
        case FINGERPRINT_SMOKING_AUTH_LOW_QUALITY:
            LOG_E(LOG_TAG, "enrollAndAuth unknown = %d", cmd_id);
            break;
        case FINGERPRINT_SMOKING_END:
            LOG_E(LOG_TAG, "enrollAndAuth %d, FINGERPRINT_SMOKING_END", cmd_id);
            mhandle->enumerate();
            removeAll();
            err = switchSmokingMode(FP_INJECTION_OFF);
            break;
        default:
            LOG_E(LOG_TAG, "enrollAndAuth unknown = %d", cmd_id);
            break;
    }
fp_out:
    if (judgeSmokingFinsh(cmd_id)) {
        switchSmokingMode(FP_INJECTION_OFF);
        memset(mSmokingFinish, 1, sizeof(mSmokingFinish));
    }
    return err;
}
}   // namespace goodix

