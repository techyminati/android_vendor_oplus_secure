/************************************************************************************
 ** File: - record.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2021-2025, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      record data
 **
 ** Version: 1.0
 ** Date created: 11:00,11/19/2021
 ** Author: Zhi.Wang@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Zhi.Wang
 ************************************************************************************/
#ifndef RECORD_H
#define RECORD_H

namespace goodix {

#define PATH_LEN 256

enum RECORD_TYPE {
    RECORD_INIT = 0,
    RECORD_ENROLL = 1,
    RECORD_ENROLL_FINISH,
    RECORD_AUTH,
    RECORD_NONE,
};

class Record {
public:
    Record();
    ~Record();

public:
    /** record the data as csv file
     * data: k=v,k1=v1,k2=v2...
     * record_type: RECORD_INIT/RECORD_ENROLL/RECORD_AUTH
     */
    void recordCsvData(char* data, int record_type);

    void setRecordFileSize(int size) {
        mMaxRecordFileSize = size;
    }
private:
    int setFilename(int record_type, char* filename);
    int parseKV(char* data, char* key, char* val);
    int saveCsvFile(char* path, char* key, char* val);

private:
    int mMaxRecordFileSize;
    char mRecRootPath[PATH_LEN];
};
} // namespace goodix

#endif // RECORD_H
