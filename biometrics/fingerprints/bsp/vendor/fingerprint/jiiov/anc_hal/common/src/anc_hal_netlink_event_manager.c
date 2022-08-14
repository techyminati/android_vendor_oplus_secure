#define LOG_TAG "[ANC_HAL][NetlinkManager]"

#include "anc_hal_netlink_event_manager.h"

#include "anc_log.h"
#include <time.h>

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include "anc_common_type.h"
#include "anc_memory_wrapper.h"
#include "anc_tac_time.h"


#define NETLINK_ANC     30
#define USER_PORT       100
#define MAX_PLOAD       125

static int g_socket_fd = -1;

// match with ANC_NETLINK_EVENT_TYPE
static char* gp_event_type_string[] = {
    "test",
    "irq",
    "screen off",
    "screen on",
    "touch down",
    "touch up",
    "ui ready",
    "exit",
    "invalid",
    "max",
};

static char *ConvertEventTypeToString(ANC_NETLINK_EVENT_TYPE event_type) {
    char *p_string = NULL;
    int event_type_string_array_size = sizeof(gp_event_type_string)/sizeof(gp_event_type_string[0]);
    int event_type_count = ANC_NETLINK_EVENT_MAX + 1;

    if ((event_type < ANC_NETLINK_EVENT_MAX)
       && (event_type_count == event_type_string_array_size)) {
        p_string = gp_event_type_string[event_type];
    } else {
        ANC_LOGE("event type string array size:%d, event type count:%d, eventtype:%d",
                  event_type_string_array_size, event_type_count, event_type);
    }

    return p_string;
}

static void AncCloseNetlinkSocket() {
    int ret = 0;

    if (g_socket_fd != -1) {
        ret = close(g_socket_fd);
        if(-1 == ret) {
            ANC_LOGE("close netlink socket, error:%d", errno);
        } else {
            g_socket_fd = -1;
        }
    }
}

static ANC_RETURN_TYPE AncCreateNetlinkSocket() {
    ANC_RETURN_TYPE ret_val = ANC_OK;
#ifndef ANC_LOCAL_REE_PLATFORM
    struct sockaddr_nl server_address;


    g_socket_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ANC);
    if(-1 == g_socket_fd) {
        ANC_LOGE("create socket error");
        return ANC_FAIL;
    }

    AncMemset(&server_address, 0, sizeof(server_address));
    server_address.nl_family = AF_NETLINK; //AF_NETLINK
    server_address.nl_pid = USER_PORT;     //端口号(port id)
    server_address.nl_groups = 0;
    if (bind(g_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) {
        ANC_LOGE("bind() error");
        AncCloseNetlinkSocket();
        ret_val = ANC_FAIL;
    }
#endif
    return ret_val;
}

static ANC_RETURN_TYPE AncSendMessageToDriver(char send_message) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
#ifdef ANC_LOCAL_REE_PLATFORM
    ANC_UNUSED(send_message);
#else
    ssize_t ret = 0;
    struct sockaddr_nl server_address;
    struct nlmsghdr *p_netlink_message_header = NULL;
    struct iovec io_vector;
    struct msghdr send_message_header;

    AncMemset(&server_address, 0, sizeof(server_address));
    server_address.nl_family = AF_NETLINK;
    server_address.nl_pid = 0; // to kernel
    server_address.nl_groups = 0;

    p_netlink_message_header = (struct nlmsghdr *)AncMalloc(NLMSG_SPACE(MAX_PLOAD));
    AncMemset(p_netlink_message_header, 0, sizeof(struct nlmsghdr));
    p_netlink_message_header->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
    p_netlink_message_header->nlmsg_flags = 0;
    p_netlink_message_header->nlmsg_type = 0;
    p_netlink_message_header->nlmsg_seq = 0;
    p_netlink_message_header->nlmsg_pid = USER_PORT; // self port id
    *(char *)(NLMSG_DATA(p_netlink_message_header)) = send_message;

    io_vector.iov_base = (void *) p_netlink_message_header;
    io_vector.iov_len = p_netlink_message_header->nlmsg_len;
    AncMemset(&send_message_header, 0, sizeof(struct msghdr));
    send_message_header.msg_iov = &io_vector;
    send_message_header.msg_iovlen = 1;
    send_message_header.msg_name = (void *) &server_address;
    send_message_header.msg_namelen = sizeof(struct sockaddr_nl);

    ret = sendmsg(g_socket_fd, &send_message_header, 0);
    if(-1 == ret) {
        ANC_LOGE("send message, error:%d", errno);
        AncCloseNetlinkSocket();
        ret_val = ANC_FAIL;
    }

    AncFree((void *)p_netlink_message_header);
#endif
    return ret_val;
}

static ANC_RETURN_TYPE AncReceiveMessageFromDriver(ANC_NETLINK_EVENT_TYPE *p_type) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    char message = 0;
    ssize_t received_length = 0;
    char receive_buffer[4096] = {0};
    struct iovec iov = {receive_buffer, sizeof(receive_buffer)};
    struct sockaddr_nl source_socket_address;
    struct msghdr receive_message_header;
    struct nlmsghdr *p_netlink_message_header = NULL;

    AncMemset(&receive_message_header, 0, sizeof(struct msghdr));
    receive_message_header.msg_name = (void *)&source_socket_address;
    receive_message_header.msg_namelen = sizeof(source_socket_address);
    receive_message_header.msg_iov = &iov;
    receive_message_header.msg_iovlen = 1;


    received_length = recvmsg(g_socket_fd, &receive_message_header, 0);
    if (received_length <= 0) {
        ANC_LOGE("receive message: length:%d, errno:%d", (int)received_length, errno);
        return ANC_FAIL;
    }

    p_netlink_message_header = (struct nlmsghdr *)receive_buffer;
    if ( !NLMSG_OK(p_netlink_message_header, (unsigned int)received_length)) {
        ANC_LOGE("message length is error!");
        return ANC_FAIL;
    }
    message = *((char *) NLMSG_DATA(p_netlink_message_header));
    *p_type = (ANC_NETLINK_EVENT_TYPE)message;

    ANC_LOGW("receive message from driver: %s, %d", ConvertEventTypeToString(*p_type), *p_type);

    return ret_val;
}

static ANC_RETURN_TYPE AncHandleNetlinkMessage(AncFingerprintManager *p_manager, ANC_NETLINK_EVENT_TYPE event_type) {
    ANC_RETURN_TYPE ret_val = ANC_OK;

    switch (event_type) {
        case ANC_NETLINK_EVENT_TEST:
            break;
        case ANC_NETLINK_EVENT_IRQ:
            break;
        case ANC_NETLINK_EVENT_SCR_OFF:
            ret_val = p_manager->p_tp_event_manager->ScreenOff(p_manager);
            break;
        case ANC_NETLINK_EVENT_SCR_ON:
            ret_val = p_manager->p_tp_event_manager->ScreenOn(p_manager);
            break;
        case ANC_NETLINK_EVENT_TOUCH_DOWN:
            ret_val = p_manager->p_tp_event_manager->TouchDown(p_manager);
            break;
        case ANC_NETLINK_EVENT_TOUCH_UP:
            ret_val = p_manager->p_tp_event_manager->TouchUp(p_manager);
            break;
        case ANC_NETLINK_EVENT_UI_READY:
            ret_val = p_manager->p_hbm_event_manager->HBMReady(p_manager);
            break;
        default:
            break;
    }

    return ret_val;
}

static void *DoTask(void *p_arg) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncFingerprintManager *p_manager = (AncFingerprintManager *)p_arg;
    AncNetlinkEventManager *p_netlink_event_manager = p_manager->p_netlink_event_manager;
    ANC_NETLINK_EVENT_TYPE netlink_event_type = ANC_NETLINK_EVENT_INVALID;

    ANC_LOGD("start netlink event manager thread");
    ret_val = AncCreateNetlinkSocket();
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to create netlink socket");
        return NULL;
    }

    ret_val = AncSendMessageToDriver(ANC_NETLINK_EVENT_TEST);
    if (ANC_OK != ret_val) {
        ANC_LOGE("fail to send message to driver");
        return NULL;
    }

    while (ANC_TRUE) {

        if (p_netlink_event_manager->status == ANC_NETLINK_EVENT_STATUS_EXIT) {
            ANC_LOGE("netlink event manager status :%d, exit", p_netlink_event_manager->status);
            break;
        }

        ret_val = AncReceiveMessageFromDriver(&netlink_event_type);
        if (ANC_OK == ret_val) {
            if (ANC_NETLINK_EVENT_EXIT == netlink_event_type) {
                ANC_LOGD("exit netlink event manager thread");
                break;
            } else if (ANC_NETLINK_EVENT_INVALID != netlink_event_type) {
                AncHandleNetlinkMessage(p_manager, netlink_event_type);
            }
        } else {
            ANC_LOGE("find some errors, then exit, ret:%d", ret_val);
            break;
        }
    }

    AncCloseNetlinkSocket();

    return NULL;
}

static ANC_RETURN_TYPE AncNetlinkInit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncNetlinkEventManager *p_netlink_event_manager = p_manager->p_netlink_event_manager;

    pthread_mutex_init(&p_netlink_event_manager->mutex, NULL);

    p_netlink_event_manager->status = ANC_NETLINK_EVENT_STATUS_INVALID;

    int status = pthread_create(&p_netlink_event_manager->thread, NULL, DoTask, p_manager);
    if (0 != status) {
        ANC_LOGE("fail to create netlink event manager pthread, status:%d", status);
        ret_val = ANC_FAIL;
    } else {
        p_netlink_event_manager->status = ANC_NETLINK_EVENT_STATUS_INIT;
    }

    return ret_val;
}

static ANC_RETURN_TYPE AncNetlinkDeinit(AncFingerprintManager *p_manager) {
    ANC_RETURN_TYPE ret_val = ANC_OK;
    AncNetlinkEventManager *p_netlink_event_manager = p_manager->p_netlink_event_manager;

    pthread_mutex_lock(&p_netlink_event_manager->mutex);
    p_netlink_event_manager->status = ANC_NETLINK_EVENT_STATUS_EXIT;
    pthread_mutex_unlock(&p_netlink_event_manager->mutex);

    ret_val = AncSendMessageToDriver(ANC_NETLINK_EVENT_EXIT);

    if (ANC_NETLINK_EVENT_STATUS_INVALID != p_netlink_event_manager->status) {
        pthread_join(p_netlink_event_manager->thread, NULL);
    }

    pthread_mutex_destroy(&p_netlink_event_manager->mutex);

    return ret_val;
}

static AncNetlinkEventManager g_netlink_event_manager = {
    .Init = AncNetlinkInit,
    .Deinit = AncNetlinkDeinit,
};

ANC_RETURN_TYPE InitNetlinkEventManager(AncFingerprintManager *p_manager) {
    p_manager->p_netlink_event_manager = &g_netlink_event_manager;
    return g_netlink_event_manager.Init(p_manager);
}

ANC_RETURN_TYPE DeinitNetlinkEventManager(AncFingerprintManager *p_manager) {
    return g_netlink_event_manager.Deinit(p_manager);
}
