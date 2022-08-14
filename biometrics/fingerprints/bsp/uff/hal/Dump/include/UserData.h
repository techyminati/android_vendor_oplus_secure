/************************************************************************************
 ** Copyright (C), 2021-2029, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - vendor/fingerprint/Dump/include/UserData.h
 **
 ** Description:
 **      UserData.h
 **
 ** Version: 1.0
 ** Date created: 18:03:11,13/08/2021
 ** Author: wangtao12
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  WangTao    2021/08/13          UserData.h
 ************************************************************************************/
#ifndef _USER_DATA_H_
#define _USER_DATA_H_

#include "IData.h"
#include "FpType.h"
#include <string>
using namespace std;

namespace android {

class UserData : public IData {
    public:
        UserData();
        virtual ~UserData();
        virtual void setParameters(void *buf, int len);
        virtual void getParameters(void *buf, int len);
        virtual fp_return_type_t saveDataTaToHal(void);
        virtual fp_return_type_t sendDataHalToTa(void);

    private:
        int save();
        void setUserDataDirName();
        bool checkBmpFileName();
        fp_mode_t mCurrentMode;
        char mCurModeName[MAX_MODULE_NAME_LEN]   = {0};
        char mFingerIdName[MAX_MODULE_NAME_LEN]      = {0};
        char mEnrollRenamePath[MAX_FILE_NAME_LENGTH];
        char mTimestamp[MAX_MODULE_NAME_LEN];
        char mGidName[MAX_MODULE_NAME_LEN];
        string mFilePath;
        string mBaseDir;
        unsigned int mResult;
        unsigned int mFingerId;
        unsigned int mGid;
        int mFormat;
        const char *mData;
        int mDataLen;
};

}

#endif //_USER_DATA_H_
