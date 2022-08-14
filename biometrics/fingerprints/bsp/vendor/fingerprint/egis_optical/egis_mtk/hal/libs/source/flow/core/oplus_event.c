#include <stdlib.h>
#include <linux/netlink.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include "device_int.h"
#include "plat_log.h"
#include "plat_time.h"
#include "common_definition.h"

#include "oplus_event.h"

extern int g_oparate_type;
extern int g_verify_flow_flag;
int g_verify_touch_up_flag = 0;
#define LOG_TAG "egis_fp_oplus_event"
int oplus_interrupt() {
    int ret = 0;
    
    return ret;
}

static const char* action_info = "/sys/kernel/hypnus/action_info";
#define ACTION_TYPE 12
#define ACTION_TIME_OUT_6000 6000
#define ACTION_TIME_OUT_1500 1500

void egis_set_action(uint32_t timeout)
{
    g_fn_callback(EVENT_SET_ACTION, ACTION_TYPE, timeout, NULL, 0);
}

#define SKIP_QUICK_TOUCH_DOWN_TOUCH_INTERVAL 100
void egis_handle_netlink_msg(int32_t msg)
{
    static unsigned long previous_touch_up = 0;
    static BOOL need_skip_quick_touch = FALSE;

    ex_log(LOG_DEBUG,"[%s] start\n", __func__);
    if (msg == EGIS_NETLINK_TEST) {
        ex_log(LOG_DEBUG,"[%s] egis_netlink_test\n", __func__);
    }
    if(g_oparate_type == DO_OTHER) {
        ex_log(LOG_DEBUG,"[%s] do nothing when no enroll/verfiy/inline running, g_oparate_type = %d\n", __func__, g_oparate_type);
        return;
    }

    if (EGIS_NETLINK_IRQ ==msg || EGIS_NETLINK_SCR_OFF == msg 
    || EGIS_NETLINK_SCR_ON == msg || EGIS_NETLINK_TP_TOUCHDOWN == msg 
    || EGIS_NETLINK_TP_TOUCHUP == msg || EGIS_NETLINK_TP_UI_READY == msg) {
        ex_log(LOG_DEBUG, "[%s] msg =%d,g_verify_flow_flag=%d,g_verify_touch_up_flag=%d\n", __func__,
               msg, g_verify_flow_flag, g_verify_touch_up_flag);
        if (g_verify_flow_flag) {
            if (msg == EGIS_NETLINK_TP_TOUCHUP) {
                g_verify_touch_up_flag = 1;
            } else if ((msg == EGIS_NETLINK_TP_TOUCHDOWN) && (g_verify_touch_up_flag == 1)) {
                g_verify_touch_up_flag = 0;
            }
        }

        switch (msg) {
            case EGIS_NETLINK_IRQ:

                break;
            case EGIS_NETLINK_TP_TOUCHDOWN:
                host_touch_set_ui_ready(FALSE);
                host_touch_set_finger_on(TRUE);
			    host_touch_set_finger_off(FALSE);
			    ex_log(LOG_DEBUG,"[%s] TOUCH DOWN\n", __func__);
			    if (plat_get_diff_time(previous_touch_up) < SKIP_QUICK_TOUCH_DOWN_TOUCH_INTERVAL)
			    {
			        ex_log(LOG_INFO, "[%s] skip quick touch Down", __func__);
			        need_skip_quick_touch = TRUE;
			        break;
			    }
			    g_fn_callback(EVENT_FINGER_TOUCH_DOWN, 0, 0, NULL, 0);
                egis_set_action(ACTION_TIME_OUT_1500);
                break;
            case EGIS_NETLINK_TP_TOUCHUP:
                host_touch_set_hbm_system(FALSE);
			    host_touch_set_finger_on(FALSE);
			    host_touch_set_finger_off(TRUE);
			    ex_log(LOG_DEBUG,"[%s] TOUCH UP\n", __func__);
			    if (need_skip_quick_touch)
			    {
			        ex_log(LOG_INFO, "[%s] skip quick touch up", __func__);
			        need_skip_quick_touch = FALSE;
			    }
			    g_fn_callback(EVENT_FINGER_TOUCH_UP, 0, 0, NULL, 0);
			    previous_touch_up = plat_get_time();
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

