#ifndef __OPLUS_EVENT_H__
#define __OPLUS_EVENT_H__
#include "device_int.h"
#include <pthread.h>
#include <sys/socket.h>
#include <linux/netlink.h>

enum EGIS_NETLINK_CMD {
    EGIS_NETLINK_TEST = 0,
    EGIS_NETLINK_IRQ,
    EGIS_NETLINK_SCR_OFF,
    EGIS_NETLINK_SCR_ON,
    EGIS_NETLINK_TP_TOUCHDOWN,
    EGIS_NETLINK_TP_TOUCHUP,
    EGIS_NETLINK_TP_UI_READY,
    EGIS_NETLINK_MAX,
};
#endif


