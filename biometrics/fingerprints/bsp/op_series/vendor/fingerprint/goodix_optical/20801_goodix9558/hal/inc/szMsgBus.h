#ifndef _SZMSGBUS_H_
#define _SZMSGBUS_H_
#include "MsgBus.h"

typedef  enum GF_BIG_DATA_MSG {
    MSG_BIG_DATA_UI_READY = goodix::MsgBus::MSG_MAX + 1000,
    MSG_BIG_DATA_GET_FEAURE_TIME,
    MSG_BIG_DATA_ENROLL_AUTH_END,
    MSG_BIG_DATA_FINGER_DOWN,
    MSG_BIG_DATA_FINGER_UP
}gf_big_data_msg_t;
#endif  // szMsgBus.h
