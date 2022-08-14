/************************************************************************************
 ** File: - hal\main\Device.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      Goodixr fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,24/02/20
 ** Author: Bangxiong.Wu@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <who>            <when>            <what>
 **  Bangxiong.Wu     2020/02/24        new fpThread to handle performance request
 ***********************************************************************************/

#define LOG_TAG "[GF_HAL][Device]"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>

#include <linux/netlink.h>
#include <linux/socket.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sched.h>

#include "Device.h"
#include "HalLog.h"
#include "MsgBus.h"
#include "HalContext.h"
#include "EventCenter.h"
#include "CaEntry.h"
#include "FingerprintCore.h"
#include <cutils/properties.h>

#define GF_IOC_MAGIC    'g'      // define magic number
/* define commands */
#define GF_IOC_INIT             (_IOR(GF_IOC_MAGIC, 0, uint8_t))
#define GF_IOC_EXIT             (_IO(GF_IOC_MAGIC, 1))
#define GF_IOC_RESET            (_IO(GF_IOC_MAGIC, 2))

#define GF_IOC_ENABLE_IRQ       (_IO(GF_IOC_MAGIC, 3))
#define GF_IOC_DISABLE_IRQ      (_IO(GF_IOC_MAGIC, 4))
#define GF_IOC_ENABLE_SPI_CLK   (_IOW(GF_IOC_MAGIC, 5, uint32_t))
#define GF_IOC_DISABLE_SPI_CLK  (_IO(GF_IOC_MAGIC, 6))

#define GF_IOC_ENABLE_POWER     (_IO(GF_IOC_MAGIC, 7))
#define GF_IOC_DISABLE_POWER    (_IO(GF_IOC_MAGIC, 8))

#define GF_IOC_INPUT_KEY_EVENT  (_IOW(GF_IOC_MAGIC, 9, gf_key_event_t))

/* fp chip has change to sleep mode while screen off */
#define GF_IOC_ENTER_SLEEP_MODE (_IO(GF_IOC_MAGIC, 10))
#define GF_IOC_GET_FW_INFO      (_IOR(GF_IOC_MAGIC, 11, uint8_t))
#define GF_IOC_REMOVE           (_IO(GF_IOC_MAGIC, 12))
#define GF_IOC_CHIP_INFO        (_IOW(GF_IOC_MAGIC, 13, gf_ioc_chip_info_t))

#define GF_IOC_NAV_EVENT        (_IOW(GF_IOC_MAGIC, 14, gf_nav_code_t))

#define GF_IOC_WAKELOCK_TIMEOUT_ENABLE        _IO(GF_IOC_MAGIC, 18)
#define GF_IOC_WAKELOCK_TIMEOUT_DISABLE        _IO(GF_IOC_MAGIC, 19)

#define GF_IOC_CLEAN_TOUCH_FLAG (_IO(GF_IOC_MAGIC, 20))
#define GF_IOC_AUTO_SEND_TOUCHDOWN        _IO(GF_IOC_MAGIC, 21 )
#define GF_IOC_AUTO_SEND_TOUCHUP        _IO(GF_IOC_MAGIC, 22 )
#define GF_IOC_STOP_WAIT_INTERRUPT_EVENT        _IO(GF_IOC_MAGIC, 23)
/* netlink feature support */
#define MAX_NL_MSG_LEN 16

#define GF_DEV_NAME "/dev/goodix_fp"
#define UINPUT_DEV "/dev/uinput"
#define FP_UINPUT_DEV_NAME "uinput_nav"
#define UINPUT_DEV_VENDOR 0x1
#define UINPUT_DEV_PRODUCT 0x1
#define UINPUT_DEV_VERSION 1
#define NETLINK_ROUTE_DEFAULT 25

#define FP_ENABLE_TP_PATH "/proc/touchpanel/fp_enable"
#define FP_LCD_TOUCH_DOWN "/sys/kernel/oplus_display/notify_fppress"

#define HARDWARE_PCB_VERSION  "/proc/oplusVersion/pcbVersion"
//add for hypnus
static const char *action_info = "/sys/kernel/hypnus/action_info";
#define ACTION_IO 12
#define ACTION_TIMEOUT 3000
#define MAX_LEN 20

#ifdef FP_HYPNUSD_ENABLE
/* hypnusd feature support */
goodix::FingerprintCore::hypnus_state_t hypnus_state = {
    .hypnusd_request = false,
    .hypnusd_action_type = 0,
    .hypnusd_action_timeout = 0
};
#endif

namespace goodix {
Device::Device(HalContext *context) :
    HalBase(context),
    mFD(-1),
    mSpiSpeed(GF_SPI_SPEED_LOW),
    mNetlinkRoute(NETLINK_ROUTE_DEFAULT),
    mThread(this),
    mAuthScreenState(-1)
#ifdef FP_HYPNUSD_ENABLE
    , fpThread(this)
#endif
{
#ifdef FP_HYPNUSD_ENABLE
    if (!fpThread.isRunning()) {
        fpThread.run("FingerprintPerfThread");
    }
#endif
}

Device::~Device() {
    close();
}

gf_error_t Device::open() {
    gf_error_t err = GF_SUCCESS;
    uint32_t i = 0;
    FUNC_ENTER();

    do {
        while (i < 10) {
            mFD = ::open(GF_DEV_NAME, O_RDWR);

            if (mFD < 0) {
                LOG_E(LOG_TAG,
                      "[%s] Failed to open (%s),and reopen device. g_fd=%u, errno =%d",
                      __func__, GF_DEV_NAME, mFD, errno);
                err = GF_ERROR_OPEN_DEVICE_FAILED;
                mFD = -1;
                i++;
            } else {
                LOG_E(LOG_TAG, "[%s] Success to open (%s) g_fd=%u", __func__,
                      GF_DEV_NAME, mFD);
                err = GF_SUCCESS;
                break;
            }

            usleep(200 * 1000);
        }

        // Note: The method can't access mCaEntry data, or crash wil happend. Now only ree use the method.
        mContext->mCaEntry->setDeviceHandle(mFD);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

void Device::close() {
    VOID_FUNC_ENTER();
    disable();

    if (mFD >= 0) {
        ::close(mFD);
        mFD = -1;
    }

    VOID_FUNC_EXIT();
}

    gf_error_t Device::automatic_send_touchdown()
    {
        gf_error_t err = GF_SUCCESS;
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s], %s null", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            return err;
        }

        if (ioctl(mFD, GF_IOC_AUTO_SEND_TOUCHDOWN) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            return err;
        }
        return err;
    }
    gf_error_t Device::automatic_send_touchup()
    {
        gf_error_t err = GF_SUCCESS;
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s], %s, null", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            return err;
        }

        if (ioctl(mFD, GF_IOC_AUTO_SEND_TOUCHUP) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            return err;
        }
        return err;
    }
gf_error_t Device::reset() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_RESET) != 0) {
            LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }

        sendMessage(MsgBus::MSG_HARDWARE_RESET);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::enable() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_INIT, &mNetlinkRoute) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_INIT ioctl failed", __func__);
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        LOG_I(LOG_TAG, "[%s] g_netlink_route=%u", __func__, mNetlinkRoute);

        if (ioctl(mFD, GF_IOC_ENABLE_IRQ) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        startNetlinkRoutingThread();
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::disable() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mThread.isRunning()) {
            mThread.requestExit();
            if (mPipeFD[1] != -1) {
                int32_t msg = 1;
                ::write(mPipeFD[1], &msg, sizeof(int32_t));
            }
        }

        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_DISABLE_IRQ) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        if (ioctl(mFD, GF_IOC_EXIT) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_EXIT ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::controlSpiClock(uint8_t enable) {
    gf_error_t err = GF_SUCCESS;

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (enable > 0) {
            if (ioctl(mFD, GF_IOC_ENABLE_SPI_CLK, &mSpiSpeed) != 0) {
                LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }

            /*
             //samsung exynos8890 spi clock only  set by linux and spi clock need set after enable spi clock.
             if (ioctl(g_fd, GF_IOC_SET_SPI_SPEED, &g_spi_speed) != 0) {
             LOG_E(LOG_TAG, "[%s] :set spi speed failed, g_spi_speed=%d.", __func__, g_spi_speed);
             err = GF_ERROR_HAL_IOCTL_FAILED;
             break;
             }
             */
        } else {
            if (ioctl(mFD, GF_IOC_DISABLE_SPI_CLK) != 0) {
                LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
    } while (0);

    return err;
}

gf_error_t Device::enablePower() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_ENABLE_POWER) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_POWER ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::disablePower() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_DISABLE_POWER) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_POWER ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::enableIrq() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_ENABLE_IRQ) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::disableIrq() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_DISABLE_IRQ) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_IRQ ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::getFirmwareInfo(uint8_t *buf) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (NULL == buf) {
            LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_GET_FW_INFO, buf) != 0) {
            LOG_E(LOG_TAG, "gf_hal_get_fw_info ioctl faild");
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::remove() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s] no device is opened", __func__);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_REMOVE) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_REMOVE ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

void Device::startNetlinkRoutingThread() {
    if (!mThread.isRunning()) {
        int32_t ret =  ::pipe(mPipeFD);
        if (ret == -1) {
            LOG_E(LOG_TAG, "[%s] create pipe failed. err=%s, erron=%d", __func__,
                  strerror(errno), errno);
            mPipeFD[0] = -1;
            mPipeFD[1] = -1;
        }
        mThread.run("NetlinkHandleThread");
    }
    return;
}

gf_event_type_t Device::mapMsgToEvent(int32_t msg) {
    gf_event_type_t e = EVENT_UNKNOWN;

    if (GF_NETLINK_TEST == msg) {
        // for netlink test use only
        LOG_D(LOG_TAG, "[%s] received GF_NETLINK_TEST command", __func__);
    } else if (GF_NETLINK_IRQ == msg || GF_NETLINK_SCREEN_OFF == msg || GF_NETLINK_SCREEN_ON == msg) {
        if (GF_NETLINK_IRQ == msg) {
            e = EVENT_IRQ;
        } else if (GF_NETLINK_SCREEN_ON == msg) {
            e = EVENT_SCREEN_ON;
        } else if (GF_NETLINK_SCREEN_OFF == msg) {
            e = EVENT_SCREEN_OFF;
        }
    } else {
        LOG_E(LOG_TAG, "[%s] wrong netlink command value=%u", __func__, msg);
    }

    return e;
}

uint32_t Device::getNetlinkMsgDataLen() {
    return MAX_NL_MSG_LEN;
}

void Device::handleNetlinkReceivedData(void *data, uint32_t len) {
    uint8_t value = 0;
    UNUSED_VAR(len);
    if (NULL == data) {
        return;
    }
    value = *((uint8_t *)data);
    LOG_D(LOG_TAG, "[%s] GF netlink thread received msg %u.", __func__, value);
    handleNetlinkMessage(value);
    return;
}

void Device::handleNetlinkMessage(int32_t msg) {
    VOID_FUNC_ENTER();
    gf_event_type_t e = (gf_event_type_t) mapMsgToEvent(msg);

    if (e != EVENT_UNKNOWN && nullptr != mContext && nullptr != mContext->mCenter) {
        if (e == EVENT_FINGER_UP) {
            // notify up
            mContext->mFingerprintCore->notifyTouch(GF_FINGERPRINT_TOUCH_UP);
        }
        mContext->mCenter->postEvent(e);
    }

    VOID_FUNC_EXIT();
    return;
}

Device::NetlinkRoutingThread::NetlinkRoutingThread(Device *device) {
    mDevice = device;
}

Device::NetlinkRoutingThread::~NetlinkRoutingThread() {
    mDevice = NULL;
}

bool Device::NetlinkRoutingThread::threadLoop() {
    struct nlmsghdr *nlh = NULL;
    int32_t netlinkSockId = 0;
    int32_t pipeFD[2];
    VOID_FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] GF netlink thread started", __func__);

    do {
        struct sockaddr_nl local; /*used to describe local address*/
        struct sockaddr_nl dest;
        struct iovec iov;
        struct msghdr msg;
        fd_set fds;
        uint8_t buf[64];
        int32_t ret = 0;
        uint32_t dataLen = mDevice->getNetlinkMsgDataLen();
        pipeFD[0] = mDevice->mPipeFD[0];
        pipeFD[1] = mDevice->mPipeFD[1];
        /* init socket and bind */
        netlinkSockId = socket(AF_NETLINK, SOCK_RAW, mDevice->mNetlinkRoute);

        if (netlinkSockId < 0) {
            LOG_E(LOG_TAG, "[%s] socket failed. err=%s, errno=%d", __func__,
                  strerror(errno),
                  errno);
            break;
        }

        memset(&local, 0, sizeof(struct sockaddr_nl));
        local.nl_family = AF_NETLINK;
        local.nl_pid = getpid();/*local process id*/
        LOG_D(LOG_TAG, "[%s] native process pid=%d", __func__, local.nl_pid);
        // local.nl_pad = 0;
        local.nl_groups = 0;
        ret = bind(netlinkSockId, (struct sockaddr *) &local,
                   sizeof(struct sockaddr_nl));

        if (ret != 0) {
            LOG_E(LOG_TAG, "[%s] bind failed. err=%s, errno=%d", __func__, strerror(errno),
                  errno);
            break;
        }

        LOG_D(LOG_TAG, "[%s] bind done", __func__);
        /* send init message */
        memset(&dest, 0, sizeof(struct sockaddr_nl));
        dest.nl_family = AF_NETLINK;
        dest.nl_pid = 0; /*destination is kernel so set to 0*/
        dest.nl_groups = 0;
        nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(dataLen));

        if (NULL == nlh) {
            LOG_E(LOG_TAG, "[%s] nlh out of memory", __func__);
            break;
        }

        nlh->nlmsg_len = NLMSG_SPACE(dataLen);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;
        strncpy((char *) NLMSG_DATA(nlh), "GF", strlen("GF") + 1);
        iov.iov_base = (void *) nlh;
        iov.iov_len = nlh->nlmsg_len;
        memset(&msg, 0, sizeof(struct msghdr));
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (void *) &dest;
        msg.msg_namelen = sizeof(struct sockaddr_nl);

        if (sendmsg(netlinkSockId, &msg, 0) < 0) {
            LOG_E(LOG_TAG, "[%s] sendmsg failed. err=%s, erron=%d", __func__,
                  strerror(errno),
                  errno);
            break;
        }

        LOG_D(LOG_TAG, "[%s] send init msg to kernel", __func__);
        /* loop for recv */
        memset(nlh, 0, NLMSG_SPACE(dataLen));
        while (!Thread::exitPending()) {
            int32_t maxfd = netlinkSockId > pipeFD[0] ? netlinkSockId + 1 : pipeFD[0] + 1;
            FD_ZERO(&fds);
            FD_SET(netlinkSockId, &fds);
            if (pipeFD[0] != -1) {
                FD_SET(pipeFD[0], &fds);
            }

            ret = select(maxfd, &fds, NULL, NULL, NULL);
            LOG_D(LOG_TAG, "[%s] select return:%d", __func__, ret);
            if (ret == -1) {
                LOG_E(LOG_TAG, "[%s] select failed. err=%s, erron=%d", __func__,
                      strerror(errno),
                      errno);
                continue;
            } else if (ret == 0) {
                LOG_E(LOG_TAG, "[%s] select time out", __func__);
                continue;
            } else {
                if (FD_ISSET(netlinkSockId, &fds)) {
                    ret = recvmsg(netlinkSockId, &msg, 0);
                    if (ret <= 0) {
                        LOG_E(LOG_TAG, "[%s] recvmsg failed, ret=%d", __func__, ret);
                        continue;
                    }
                } else {
                    if (pipeFD[0] != -1 && FD_ISSET(pipeFD[0], &fds)) {
                        LOG_D(LOG_TAG, "[%s] got exit message", __func__);
                        ret = ::read(pipeFD[0], buf, sizeof(buf));
                        LOG_D(LOG_TAG, "[%s] ret=%d, buf:%d,%d,%d,%d", __func__, ret, buf[0], buf[1], buf[2], buf[3]);
                        break;
                    }
                    continue;
                }
            }

            // thread is blocked by select(), check the thread status
            if (Thread::exitPending()) {
                break;
            }

            mDevice->handleNetlinkReceivedData(NLMSG_DATA(nlh), dataLen);
        }
    } while (0);

    LOG_D(LOG_TAG, "[%s] GF netlink thread finish.", __func__);

    if (nlh != NULL) {
        free(nlh);
    }

    if ((netlinkSockId != -1) && (netlinkSockId != 0)) {
        ::close(netlinkSockId);
        netlinkSockId = 0;
    }

    if (pipeFD[0] != -1) {
        ::close(pipeFD[0]);
    }

    if (pipeFD[1] != -1) {
        ::close(pipeFD[1]);
    }

    VOID_FUNC_EXIT();
    return false;
}

#ifdef FP_HYPNUSD_ENABLE
Device::FpPerfThread::FpPerfThread(Device *device) {
    VOID_FUNC_ENTER();
    mDevice = device;
    pthread_cond_init(&hypnus_state.hypnusd_cond, NULL);
    pthread_mutex_init(&hypnus_state.hypnusd_lock, NULL);
    VOID_FUNC_EXIT();
}

Device::FpPerfThread::~FpPerfThread() {
    VOID_FUNC_ENTER();
    /* exitPending true */
    Thread::requestExit();
    mDevice = NULL;
    VOID_FUNC_EXIT();
}

bool Device::FpPerfThread::threadLoop() {
    VOID_FUNC_ENTER();
    while (true) {
        pthread_mutex_lock(&hypnus_state.hypnusd_lock);
        if (!hypnus_state.hypnusd_request) {
            LOG_D(LOG_TAG, "[%s] wait in", __func__);
            pthread_cond_wait(&hypnus_state.hypnusd_cond, &hypnus_state.hypnusd_lock);
            LOG_D(LOG_TAG, "[%s] wait out", __func__);
        }
        pthread_mutex_unlock(&hypnus_state.hypnusd_lock);

        /* FpPerThread wake up, check exitPending or set hypnus */
        if (Thread::exitPending()) {
            break;
        }

        LOG_D(LOG_TAG, "[%s] set_hypnus begin", __func__);
        mDevice->mContext->mFingerprintCore->set_hypnus(hypnus_state.hypnusd_action_type,
                hypnus_state.hypnusd_action_timeout);
        /* request done, reset hypnusd_request flag */
        pthread_mutex_lock(&hypnus_state.hypnusd_lock);
        hypnus_state.hypnusd_request = false;
        pthread_mutex_unlock(&hypnus_state.hypnusd_lock);
    }
    LOG_E(LOG_TAG, "[%s] thread finish", __func__);
    /* destroy something */
    pthread_cond_destroy(&hypnus_state.hypnusd_cond);
    pthread_mutex_destroy(&hypnus_state.hypnusd_lock);
    VOID_FUNC_EXIT();
    return false;
}
#endif

gf_error_t Device::enable_tp(gf_tp_status mode) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    hold_wakelock(GF_HOLD_WAKE_LOCK);
    LOG_E(LOG_TAG, "[%s] mode = %d.", __func__, mode);
    do {
        char buff[2];
        int fd = ::open(FP_ENABLE_TP_PATH, O_WRONLY);
        if (fd < 0) {
            LOG_E(LOG_TAG, "[%s] GF netlink thread finish.", __func__);
            err =  GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }
        snprintf(buff, sizeof(buff), "%d", (int)mode);
        write(fd, buff, 2);
        ::close(fd);
    } while (0);
    hold_wakelock(GF_RELEASE_WAKE_LOCK);
    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::hold_wakelock(gf_wakelock_status mode) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s], no dev=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }
        if (GF_HOLD_WAKE_LOCK == mode) {
            if (ioctl(mFD, GF_IOC_WAKELOCK_TIMEOUT_ENABLE) != 0) {
                LOG_E(LOG_TAG, "[%s] GF_IOC_HOLD_WAKE_LOCK ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        } else {
            if (ioctl(mFD, GF_IOC_WAKELOCK_TIMEOUT_DISABLE) != 0) {
                LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}

gf_error_t Device::enable_lcd(gf_lcd_status mode) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();

    do {
        char buff[50];
        LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
        int fd = ::open(FP_LCD_TOUCH_DOWN, O_WRONLY);
        if (fd < 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
        }
        snprintf(buff, sizeof(buff), "%d", (int)mode);
        write(fd, buff, 50);
        ::close(fd);
    } while (0);

    FUNC_EXIT(err);
    return err;
}

#ifdef __TRUSTONIC
gf_error_t Device::get_pcb_version(uint16_t *oppo_pcb_ver) {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    int fd = 0;
    int size = 0;
    char pcbver[10];

    do {
        memset(pcbver, 0, sizeof(pcbver));

        fd = ::open(HARDWARE_PCB_VERSION, O_RDONLY);
        if (fd == 0) {
            ALOGE("open /proc/oplusVersion/pcbVersion failed!!!");
            err = GF_ERROR_OPEN_DEVICE_FAILED;
            break;
        }

        size = read(fd, (void *)pcbver, sizeof(pcbver));
        if (size < 1) {
            ALOGE("read /proc/oplusVersion/pcbVersion error size:%d", size);
            ::close(fd);
            err = GF_ERROR_BAD_PARAMS;
            break;
        }

        sscanf((char *)pcbver, "%d", oppo_pcb_ver);
        LOG_I(LOG_TAG, "%s: pcbVersion: %d", __func__, *oppo_pcb_ver);
        ::close(fd);
    } while (0);
    FUNC_EXIT(err);
    return err;
}
#endif

gf_error_t Device::clear_kernel_touch_flag() {
    gf_error_t err = GF_SUCCESS;
    FUNC_ENTER();
    LOG_D(LOG_TAG, "[%s] enter GF_IOC_CLEAN_TOUCH_FLAG", __func__);

    do {
        if (mFD < 0) {
            LOG_E(LOG_TAG, "[%s], no dev=%s", __func__, GF_DEV_NAME);
            err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
            break;
        }

        if (ioctl(mFD, GF_IOC_CLEAN_TOUCH_FLAG) != 0) {
            LOG_E(LOG_TAG, "[%s] GF_IOC_CLEAN_TOUCH_FLAG ioctl failed", __func__);
            err = GF_ERROR_HAL_IOCTL_FAILED;
            break;
        }
    } while (0);

    FUNC_EXIT(err);
    return err;
}
}  // namespace goodix
