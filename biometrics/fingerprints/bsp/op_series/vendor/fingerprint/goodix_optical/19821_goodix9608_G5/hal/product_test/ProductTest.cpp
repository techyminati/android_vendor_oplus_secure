/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#define LOG_TAG "[GF_HAL][ProductTest]"

#include "Mutex.h"
#include "ProductTest.h"
#include "HalLog.h"
#include "ProductTestDefine.h"
#include "TestUtils.hpp"
#include "HalContext.h"
#include "gf_sensor_types.h"
#include "GoodixFingerprint.h"

namespace goodix {

    ProductTest::ProductTest(HalContext *context)
        : HalBase(context) {
        // NOLINT(432)
        mContext->mMsgBus.addMsgListener(this);
    }

    ProductTest::~ProductTest() {
        mContext->mMsgBus.removeMsgListener(this);
    }

    gf_error_t ProductTest::onCommand(int32_t cmdId, const int8_t *in,
                                      uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        if (isNeedLock(cmdId)) {
            mContext->mHalLock.lock();
        }

        err = executeCommand(cmdId, in, inLen, out, outLen);

        if (isNeedLock(cmdId)) {
            mContext->mHalLock.unlock();
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t ProductTest::executeCommand(int32_t cmdId, const int8_t *in,
                                           uint32_t inLen, int8_t **out, uint32_t *outLen) {
        gf_error_t err = GF_SUCCESS;
        UNUSED_VAR(in);
        UNUSED_VAR(inLen);

        do {
            switch (cmdId) {
                case CMD_TEST_GET_CONFIG: {
                    err = getConfig(out, outLen);
                    break;
                }

                default: {
                    err = GF_ERROR_UNKNOWN_CMD;
                    break;
                }
            }
        } while (0);

        return err;
    }

    gf_error_t ProductTest::onMessage(const MsgBus::Message &msg) {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do {
            switch (msg.msg) {
                case MsgBus::MSG_AUTHENTICATE_ALGO_END:
                case MsgBus::MSG_ENROLL_END: {
                    handleEnrollAuthEndMessage();
                    break;
                }

                case MsgBus::MSG_WAIT_FOR_FINGER_INPUT: {
                    handleWaitForFingerInputMessage();
                    break;
                }

                case MsgBus::MSG_CANCELED: {
                    handleCanceledMessage();
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

    void ProductTest::handleEnrollAuthEndMessage() {
        return;
    }

    void ProductTest::handleWaitForFingerInputMessage() {
        return;
    }

    void ProductTest::handleCanceledMessage() {
        return;
    }

    void ProductTest::notifyTestCmd(int64_t devId, int32_t cmdId,
                                    const int8_t *result, int32_t resultLen) {
        VOID_FUNC_ENTER();

        if (mpNotify != NULL) {
            mpNotify(devId, GF_FINGERPRINT_TEST_CMD, cmdId, result, resultLen);
        }

        VOID_FUNC_EXIT();
    }

    void ProductTest::initializePreviewWindowSize() {
        return;
    }


}  // namespace goodix
