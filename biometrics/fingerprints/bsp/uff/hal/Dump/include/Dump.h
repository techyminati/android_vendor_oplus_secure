/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/Dump.h
 **
 ** Description:
 **      Dump for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,26/03/2021
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 **
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  Ran.Chen   2021/03/26        create the file
 **  WangTao    2021/05/25        Dump.h
 ************************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "FpType.h"
#include <string>

#ifndef _DUMP_H_
#define _DUMP_H_

namespace android {

using namespace std;

#define MAX_INFO_LEN            128
#define TIME_STAMP_LEN          60
#define CA_TA_BUF_MAX_LEN       400*1024

typedef struct {
    char *dir;
    char *fileName;
    char *buf;
    int32_t len;
    unsigned char appendFlag;
} file_info_t;

typedef struct {
    int32_t index;
    char filePathName[MAX_FILE_NAME_LENGTH];
} dir_record_info_t;

class Dump {
public:
        Dump();
        ~Dump();
        static int dumpFileManager(const char* dir, const char* path);
        static fp_return_type_t getTaFileCount(int *cnt);
        static fp_return_type_t getTaDataSize(int fileIndex, long long *dataAddr, char *file_path, int *dataSize);
        static fp_return_type_t dumpFreeData();
        static fp_return_type_t getDataFromTa(fp_dump_t *cmd, int size,
            int readSize, int file_index, int offset, long long dataAddr);
        static string getDataType(fp_mode_t mode);
        static void getEngineeringType();
        static void *generateParameters(char *file_path, char *buf, int len, int format, unsigned int gid,
            unsigned int result, unsigned int fingerId, fp_mode_t mode);
        static int getGenerateParametersLength(fp_mode_t mode);

        static int dumpProcess(fp_mode_t fpMode, uint32_t gid, unsigned int fingerId, int result);
        static void dumpRename(unsigned int finger_id);
        static void setDumpSupport(char dumpSupport);
        static void setCurrentMode(fp_mode_t mode);
        static fp_mode_t getCurrentMode();
        static void setPreversionFlag(unsigned char flag);
        static unsigned char getPreversionFlag();
};
}  // namespace android

#endif  // _DUMP_H_

