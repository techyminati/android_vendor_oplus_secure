#ifndef __OPLUS_EVENT_H__
#define __OPLUS_EVENT_H__
#include "device_int.h"
#include <pthread.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <response_def.h>
#include <errno.h>
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
#define NETLINK_EGIS 23
#define ACTION_IO 12
#define ACTION_TIMEOUT 3000
#define MAX_LEN 20
#define MAX_NL_MSG_LEN 16
uint8_t egis_netlink_threadloop(void *handle);
#endif


