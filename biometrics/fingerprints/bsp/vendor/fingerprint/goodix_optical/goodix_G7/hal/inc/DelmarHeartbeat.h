/*
 * Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */


#ifndef _DELMARHEARTBEATRATE_H_
#define _DELMARHEARTBEATRATE_H_

#include "Heartbeat.h"
#include "gf_delmar_heartbeat_rate_types.h"

namespace goodix {

    class DelmarHeartbeatRate : public HeartbeatRate, public MsgBus::IMsgListener {
    public:
        explicit DelmarHeartbeatRate(HalContext *context);
        virtual ~DelmarHeartbeatRate();
        virtual gf_error_t onMessage(const MsgBus::Message& msg);
        virtual bool isNeedLock(int32_t cmdId);
        gf_error_t startHeartBeatDetect();

    protected:
        virtual gf_error_t executeCommand(int32_t cmdId, const int8_t *in, uint32_t inLen, int8_t **out, uint32_t *outLen);

    private:
        uint32_t mWorkState;
        void handleCanceledMessage();
        void handleWaitForFingerInputMessage();
        void notifySensorDisplayControl(int64_t devId, int32_t cmdId,
                                            const int8_t *sensorData, int32_t dataLen);
        gf_error_t preHeartBeatDetect(const int8_t *in, uint32_t inLen);
        gf_error_t postHeartBeatDetect(const int8_t *in, uint32_t inLen);
        gf_error_t heartBeatParseTestArgs(const int8_t *in, uint32_t inLen);
        gf_error_t stopHeartBeatDetect();
        void notifyHeartBeatRateResult(gf_error_t err, gf_delmar_heart_beat_rate_cmd_t *result);
        bool isNeedCtlSensor(uint32_t cmdId);
        void notifyHeartbeatRateCmd(int64_t devId, int32_t cmdId, const int8_t *result,
                                   int32_t resultLen);
        gf_error_t heartBeatParseResultArgs(const int8_t *in, uint32_t inLen);
        gf_error_t handleEvent(gf_event_type_t e);
    };

}  // namespace goodix



#endif /* _DELMARHEARTBEATRATE_H_ */
