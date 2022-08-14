/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */
#define LOG_TAG "[GF_HAL][FingerListSync]"
#include <binder/IInterface.h>
#include <inttypes.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <utils/String16.h>
#include <utils/Looper.h>
#include <hardware/hardware.h>
#include <utils/Log.h>
#include <string.h>
#include <endian.h>
#include "HalLog.h"
#include "Device.h"
#include "HalContext.h"
#include "HalUtils.h"
#include "FingerprintCore.h"
#include "gf_error.h"
#include "HalBase.h"
#include "gf_event.h"
#include "gf_fpcore_types.h"

namespace goodix
{
    class IFingerService : public android::IInterface
    {
    public:
        DECLARE_META_INTERFACE(FingerService)
        virtual int32_t getEnrolledFingerprints(uint32_t groupId, uint32_t **ppList) = 0;
    };

    const android::String16
    FINGER_DESCRIPTOR("android.hardware.fingerprint.IFingerprintService");

    class BpFingerService : public android::BpInterface<IFingerService>
    {
        enum
        {
            FIRST_CALL_TRANSACTION = 1,
            TRANSACTION_GET_ENROLLED_FINGERPRINTS = (android::IBinder::FIRST_CALL_TRANSACTION + 6),
        };

    public:
        /**
         * Function:BpFingerService
         * Description:BpFingerService constructor
         * Input:
         *    rImpl implement of binder.
         * Output:nothing
         * Return:BpFingerService
         */
        BpFingerService(
            const android::sp<android::IBinder> &rImpl):
            android::BpInterface<IFingerService>(rImpl) {}

        /**
         * Function:getEnrolledFingerprints
         * Description:get enrolled fingerprint list
         * Input:
         *    groupId indicate the group.
         * Output:ppList have enrolled fingerprint list.
         * Return:size of ppList
         */
        int32_t getEnrolledFingerprints(uint32_t groupId, uint32_t **ppFingerList)
        {
            android::Parcel data;
            android::Parcel reply;

            data.writeInterfaceToken(FINGER_DESCRIPTOR);
            data.writeInt32(groupId);
            data.writeString16(android::String16("system"));
            remote()->transact(TRANSACTION_GET_ENROLLED_FINGERPRINTS, data, &reply, 0);
            uint32_t code = reply.readExceptionCode();
            if (0 != code)
            {
                return -1;
            }

            int32_t count = reply.readInt32();
            if (count <= 0)
            {
                return 0;
            }

            *ppFingerList = (uint32_t *)malloc(count * sizeof(uint32_t));
            if (nullptr == *ppFingerList)
            {
                return -1;
            }
            memset(*ppFingerList, 0, count * sizeof(uint32_t));

            for (int32_t i = 0; i < count; i++)
            {
                if (reply.readInt32() == 1)
                {
                    reply.readString16();
                    reply.readInt32();
                    (*ppFingerList)[i] = reply.readInt32();
                    reply.readInt64();
                }
            }

            ALOG(LOG_VERBOSE, LOG_TAG, "[%s] count = %d.", __func__, count);
            return count;
        }
    };
    IMPLEMENT_META_INTERFACE(FingerService, "android.hardware.fingerprint.IFingerprintService")

    static int32_t gIsFirstSync = 0;  // 0 :first sync 1: not first sync
    void syncFingerListBasedOnSettings(HalContext* context, uint32_t groupId)
    {
        uint32_t *pSettingFingerList = nullptr;
        int32_t count = 0;
        uint32_t teeFingerList[MAX_FINGERS_PER_USER] = { 0 };
        int32_t teeFingerCount = MAX_FINGERS_PER_USER;
        gf_error_t err = GF_SUCCESS;

        if (0 != gIsFirstSync)
        {
            return;
        }
        else
        {
            gIsFirstSync = 1;
        }
        // get finger list from setting
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::IBinder> binder = sm->getService(android::String16("fingerprint"));
        android::sp<IFingerService> fingerservice = android::interface_cast<IFingerService>(binder);
        if (fingerservice == nullptr)
        {
            ALOG(LOG_ERROR, LOG_TAG, "[%s] can't get IFingerService", __func__);
            return;
        }

        ALOG(LOG_VERBOSE, LOG_TAG, "[%s] sync tee finger list begin", __func__);
        count = fingerservice->getEnrolledFingerprints(groupId, &pSettingFingerList);
        if (count < 0)
        {
            ALOG(LOG_ERROR, LOG_TAG, "[%s] can't get list from framework", __func__);
            return;
        }

        err = context->mFingerprintCore->getIdList(groupId, teeFingerList, &teeFingerCount);
        if (GF_SUCCESS != err)
        {
            ALOG(LOG_ERROR, LOG_TAG, "get id list from ta error\n");
            return;
        }

        for (int32_t i = 0; i < teeFingerCount; i++)
        {
            ALOG(LOG_DEBUG, LOG_TAG, "[%s] ta list, setting_finger_id[%d]=%u",
                 __func__, i, teeFingerList[i]);
            int8_t consistenceFlag = 0;

            for (int32_t j = 0; j < count; j++)
            {
                ALOG(LOG_VERBOSE, LOG_TAG, "[%s] setting list, tee_finger_id[%d]=%u", __func__, j, pSettingFingerList[j]);
                if (teeFingerList[i] == pSettingFingerList[j])
                {
                    consistenceFlag = 1;
                    break;
                }
            }

            if (0 == consistenceFlag)
            {
                ALOG(LOG_DEBUG, LOG_TAG, "[%s] delete groupid=%d, fingerid=%d.", __func__, groupId, teeFingerList[i]);
                err = context->mFingerprintCore->removeTemplates(groupId, teeFingerList[i]);
                if (err != GF_SUCCESS)
                {
                    ALOG(LOG_ERROR, LOG_TAG, "[%s] something wrong happens.", __func__);
                }
            }
        }  // end for teeFingerCount

        ALOG(LOG_VERBOSE, LOG_TAG, "[%s] sync tee finger list end!", __func__);

        if (NULL != pSettingFingerList)
        {
            free(pSettingFingerList);
            pSettingFingerList = NULL;
        }
        return;
    }

}   // namespace goodix

