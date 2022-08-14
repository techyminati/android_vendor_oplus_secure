#include "plat_log.h"

#include "oplus_event.h"

#define LOG_TAG "egis_fp_oplus_event"
int oplus_interrupt() {
    int ret = 0;
    
    return ret;
}

static const char* action_info = "/sys/kernel/hypnus/action_info";
void egis_set_action(uint32_t timeout)
{
    char buf[MAX_LEN];
    int fd = open(action_info, O_WRONLY);
    if (fd < 0) {
        egislog_e("[%s] egis set action open file failed\n", __func__);
        return;
    }
    snprintf(buf, sizeof(buf), "%d %d", ACTION_IO, timeout);
    write(fd, buf, MAX_LEN);
    close(fd);
    egislog_d("[%s] set action for egis\n", __func__);
    return;
}

void egis_handle_netlink_msg(int32_t msg)
{
    egislog_d("[%s] start\n", __func__);
    if (msg == EGIS_NETLINK_TEST) {
        egislog_d("[%s] egis_netlink_test\n", __func__);
    }

    if (EGIS_NETLINK_IRQ ==msg || EGIS_NETLINK_SCR_OFF == msg 
    || EGIS_NETLINK_SCR_ON == msg || EGIS_NETLINK_TP_TOUCHDOWN == msg 
    || EGIS_NETLINK_TP_TOUCHUP == msg || EGIS_NETLINK_TP_UI_READY == msg) {

        switch (msg) {
            case EGIS_NETLINK_IRQ:
                
                break;
            case EGIS_NETLINK_TP_TOUCHDOWN:
                host_touch_set_finger_on(TRUE);
			    host_touch_set_finger_off(FALSE);
			    egislog_d("[%s] TOUCH DOWN\n", __func__);
			    g_fn_callback(EVENT_FINGER_TOUCH_DOWN, 0, 0, NULL, 0);
                egis_set_action(600);
                break;
            case EGIS_NETLINK_TP_TOUCHUP:
                host_touch_set_hbm_system(FALSE);
			    host_touch_set_finger_on(FALSE);
			    host_touch_set_finger_off(TRUE);
			    egislog_d("[%s] TOUCH UP\n", __func__);
			    g_fn_callback(EVENT_FINGER_TOUCH_UP, 0, 0, NULL, 0);
			    host_touch_set_ui_ready(FALSE);
			    host_touch_set_finger_off(TRUE);
			    host_touch_set_finger_on(FALSE);
                break;
            case EGIS_NETLINK_TP_UI_READY:
                egislog_d("[%s] UI READY\n", __func__);
			    host_touch_set_ui_ready(TRUE);
                usleep(14*1000);
                break;
            default:
                break;
        }
    } else {
        egislog_d("[%s] msg invalid\n", __func__);
    }
    return;
}


uint8_t egis_netlink_threadloop(void *handle)
{
    struct nlmsghdr * nlh = NULL;
    int32_t netlinkSockId = 0;
    egislog_d("[%s] start\n", __func__);

    do {
        struct sockaddr_nl local;
        struct sockaddr_nl dest;
        struct iovec iov;
        struct msghdr msg;
        int32_t ret = 0;
        uint8_t type = 0;
        uint8_t value = 0;
        netlinkSockId = socket(AF_NETLINK, SOCK_RAW, NETLINK_EGIS);

        if(netlinkSockId < 0) {
            egislog_d("[%s] failed to get netlinkSockId ret = %d\n", __func__, netlinkSockId);
            break;
        }
        memset(&local, 0, sizeof(struct sockaddr_nl));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid();
        ret = bind(netlinkSockId, (struct sockaddr *)&local, sizeof(struct sockaddr_nl));

        if(ret != 0) {
            egislog_d("[%s] failed to bind egis bind ret = %d errno = %d\n", __func__, ret, errno);
            break;
        }
        memset(&dest, 0, sizeof(struct sockaddr_nl));
        dest.nl_family = AF_NETLINK;
        dest.nl_pid = 0;
        dest.nl_groups = 0;
		nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_NL_MSG_LEN));
		if (NULL == nlh) {
                egislog_d("[%s] failed to malloc NL_MSG\n", __func__);
                break;
        }
		nlh->nlmsg_len = NLMSG_SPACE(MAX_NL_MSG_LEN);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;
        strncpy((char*) NLMSG_DATA(nlh), "EG", strlen("EG") + 1);
        iov.iov_base = (void *) nlh;
        iov.iov_len = nlh->nlmsg_len;
        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (void *) &dest;
        msg.msg_namelen = sizeof(struct sockaddr_nl);
		if (sendmsg(netlinkSockId, &msg, 0) < 0) {
			egislog_d("[%s] failed to send msg errno = %d\n", __func__, errno);
		}
		memset(nlh, 0, NLMSG_SPACE(MAX_NL_MSG_LEN));
		while(1) {
                ret = recvmsg(netlinkSockId, &msg, 0);

                if (ret < 0)
                {  
                    continue;
                }

                if (0 == ret)
                {
                    continue;
                }
                value = *((char *) NLMSG_DATA(nlh));
                egis_handle_netlink_msg(value);
        }
    } while(0);
    if (nlh != NULL)
    {
		free(nlh);
    }

    if ((netlinkSockId != -1) && (netlinkSockId != 0))
    {
        close(netlinkSockId);
        netlinkSockId = 0;
    }
	return false;
}


//TODO other function


