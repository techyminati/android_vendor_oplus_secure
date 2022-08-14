 /************************************************************************************
 ** File: - Device.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service implementation for goodix optical fingerprint (android O)
 **
 ** Version: 1.0
 ** Date : 18:03:11,04/10/2018
 ** Author: oujinrong@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  oujinrong   2018/10/04           create file, and bind to big core
 **  oujinrong   2018/10/24           add for not showing the UI when finger is up
 **  oujinrong   2018/10/31           add debug info for release version
 **  luhongyu    2018/11/07           remove event handler of screen event
 **  Ran.Chen    2019/02/18           modify for coverity 775755 775749 775748
 **  Dongnan.Wu  2019/04/08           add get pcb version interface
 **  Ran.Chen    2019/05/07           add for clear_kernel_touch_flag
 **  Ziqing.Guo  2019/08/21           move hypnus to fingerprint common module
 ************************************************************************************/

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
static const char* action_info = "/sys/kernel/hypnus/action_info";
#define ACTION_IO 12
#define ACTION_TIMEOUT 3000
#define MAX_LEN 20

namespace goodix
{

    enum NETLINK_CMD
    {
        GF_NETLINK_TEST = 0,  //
        GF_NETLINK_IRQ = 1,
        GF_NETLINK_SCREEN_OFF,
        GF_NETLINK_SCREEN_ON,
        GF_NETLINK_TP_TOUCHDOWN,
        GF_NETLINK_TP_TOUCHUP,
        GF_NETLINK_UI_READY,
        GF_NETLINK_MAX,
    };

    Device::Device(HalContext* context) :
            HalBase(context), mFD(-1), mSpiSpeed(GF_SPI_SPEED_LOW), mNetlinkRoute(
                    NETLINK_ROUTE_DEFAULT), mThread(this)
    {
    }

    Device::~Device()
    {
        mThread.requestExit();
        close();
    }

    gf_error_t Device::open()
    {
        gf_error_t err = GF_SUCCESS;
        uint32_t i = 0;
        FUNC_ENTER();
        do
        {
            while (i < 10)
            {
                mFD = ::open(GF_DEV_NAME, O_RDWR);
                if (mFD < 0)
                {
                    LOG_E(LOG_TAG, "[%s] Failed to open device (%s),and reopen device. g_fd=%u, errno =%d", __func__, GF_DEV_NAME,
                            mFD, errno);
                    err = GF_ERROR_OPEN_DEVICE_FAILED;
                    mFD = -1;
                    i++;
                } else {
                    LOG_E(LOG_TAG, "[%s] Success to open device (%s) g_fd=%u", __func__, GF_DEV_NAME,
                            mFD);
                    err = GF_SUCCESS;
                    break;
                }
                usleep(200*1000);
            }
            // Note: The method can't access mCaEntry data, or crash wil happend. Now only ree use the method.
            mContext->mCaEntry->setDeviceHandle(mFD);
        }while (0);
        FUNC_EXIT(err);
        return err;
    }

    void Device::close()
    {
        VOID_FUNC_ENTER();
 
        disable();
        if (mFD >= 0)
        {
            ::close(mFD);
            mFD = -1;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t Device::reset()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_RESET) != 0)
            {
                LOG_E(LOG_TAG, "[%s] ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }

            sendMessage(MsgBus::MSG_HARDWARE_RESET);
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::enable()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_INIT, &mNetlinkRoute) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_INIT ioctl failed", __func__);
                err = GF_ERROR_OPEN_DEVICE_FAILED;
                break;
            }

            LOG_I(LOG_TAG, "[%s] g_netlink_route=%u", __func__, mNetlinkRoute);

            if (ioctl(mFD, GF_IOC_ENABLE_IRQ) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
                err = GF_ERROR_OPEN_DEVICE_FAILED;
                break;
            }
            startNetlinkRoutingThread();
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::disable()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_DISABLE_IRQ) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_IRQ ioctl failed", __func__);
                err = GF_ERROR_OPEN_DEVICE_FAILED;
                break;
            }

            if (ioctl(mFD, GF_IOC_EXIT) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_EXIT ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::controlSpiClock(uint8_t enable)
    {
        gf_error_t err = GF_SUCCESS;

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (enable > 0)
            {
                if (ioctl(mFD, GF_IOC_ENABLE_SPI_CLK, &mSpiSpeed) != 0)
                {
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
            }
            else
            {
                if (ioctl(mFD, GF_IOC_DISABLE_SPI_CLK) != 0)
                {
                    LOG_E(LOG_TAG, "[%s] ioctl failed enable=%u", __func__, enable);
                    err = GF_ERROR_HAL_IOCTL_FAILED;
                    break;
                }
            }
        }
        while (0);

        return err;
    }

    gf_error_t Device::enablePower()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_ENABLE_POWER) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_POWER ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::disablePower()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_DISABLE_POWER) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_POWER ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::enableIrq()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_ENABLE_IRQ) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_ENABLE_IRQ ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::disableIrq()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_DISABLE_IRQ) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_DISABLE_IRQ ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::getFirmwareInfo(uint8_t* buf)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (NULL == buf)
            {
                LOG_E(LOG_TAG, "[%s] invalid parameters", __func__);
                err = GF_ERROR_BAD_PARAMS;
                break;
            }

            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_GET_FW_INFO, buf) != 0)
            {
                LOG_E(LOG_TAG, "gf_hal_get_fw_info ioctl faild");
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::remove()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_REMOVE) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_REMOVE ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    void Device::startNetlinkRoutingThread()
    {
        if (!mThread.isRunning())
        {
            mThread.run("NetlinkHandleThread");
        }
        return;
    }

    void Device::handleNetlinkMessage(int32_t msg)
    {
        VOID_FUNC_ENTER();
        if (GF_NETLINK_TEST == msg)
        {
            // for netlink test use only
            LOG_D(LOG_TAG, "[%s] received GF_NETLINK_TEST command", __func__);
        }
        else if (GF_NETLINK_IRQ == msg || GF_NETLINK_SCREEN_OFF == msg
                || GF_NETLINK_SCREEN_ON == msg || GF_NETLINK_TP_TOUCHDOWN == msg
                || GF_NETLINK_TP_TOUCHUP == msg || GF_NETLINK_UI_READY == msg)
        {
            gf_event_type_t e = EVENT_UNKNOWN;
            if (GF_NETLINK_IRQ == msg)
            {
                e = EVENT_IRQ;
            }
            else if (GF_NETLINK_TP_TOUCHDOWN == msg)
            {
                e = EVENT_FINGER_DOWN;
            }
            else if (GF_NETLINK_TP_TOUCHUP == msg)
            {
                e = EVENT_FINGER_UP;
                mContext->mFingerprintCore->notifyTouch(FINGERPRINT_TOUCH_UP);
            }
            else if (GF_NETLINK_UI_READY == msg)
            {
                e = EVENT_UI_READY;
            }
            if (nullptr != mContext && nullptr != mContext->mCenter)
            {
                gf_error_t err = mContext->mCenter->postEvent(e);
                LOG_D(LOG_TAG, "[%s] post event:%d, return:%d", __func__, e, err);
                if (EVENT_FINGER_DOWN == e) {
                    #ifdef FP_HYPNUSD_ENABLE
                    mContext->mFingerprintCore->set_hypnus(ACTION_TYPE, ACTION_TIMEOUT_3000);
                    #endif
                }
            }
        }
        else
        {
            LOG_E(LOG_TAG, "[%s] wrong netlink command value=%u", __func__, msg);
        }
        VOID_FUNC_EXIT();
        return;
    }

    Device::NetlinkRoutingThread::NetlinkRoutingThread(Device* device)
    {
        mDevice = device;
    }

    Device::NetlinkRoutingThread::~NetlinkRoutingThread()
    {
        mDevice = NULL;
    }

    bool Device::NetlinkRoutingThread::threadLoop()
    {
        struct nlmsghdr *nlh = NULL;
        int32_t netlinkSockId = 0;
        VOID_FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] GF netlink thread started", __func__);

        do
        {
            struct sockaddr_nl local; /*used to describe local address*/
            struct sockaddr_nl dest;
            struct iovec iov;
            struct msghdr msg;
            int32_t ret = 0;
            uint8_t value = 0;
            /* init socket and bind */
            netlinkSockId = socket(AF_NETLINK, SOCK_RAW, mDevice->mNetlinkRoute);

            if (netlinkSockId < 0)
            {
                LOG_E(LOG_TAG, "[%s] socket failed. err=%s, errno=%d", __func__, strerror(errno),
                        errno);
                break;
            }

            memset(&local, 0, sizeof(struct sockaddr_nl));
            local.nl_family = AF_NETLINK;
            local.nl_pid = getpid();/*local process id*/
            LOG_D(LOG_TAG, "[%s] native process pid=%d", __func__, local.nl_pid);
            // local.nl_pad = 0;
            local.nl_groups = 0;
            ret = bind(netlinkSockId, (struct sockaddr *) &local, sizeof(struct sockaddr_nl));

            if (ret != 0)
            {
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
            nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_NL_MSG_LEN));

            if (NULL == nlh)
            {
                LOG_E(LOG_TAG, "[%s] nlh out of memory", __func__);
                break;
            }

            nlh->nlmsg_len = NLMSG_SPACE(MAX_NL_MSG_LEN);
            nlh->nlmsg_pid = getpid();
            nlh->nlmsg_flags = 0;
            strncpy((char*) NLMSG_DATA(nlh), "GF", strlen("GF") + 1);
            iov.iov_base = (void *) nlh;
            iov.iov_len = nlh->nlmsg_len;
            memset(&msg, 0, sizeof(struct msghdr));
            msg.msg_iov = &iov;
            msg.msg_iovlen = 1;
            msg.msg_name = (void *) &dest;
            msg.msg_namelen = sizeof(struct sockaddr_nl);

            if (sendmsg(netlinkSockId, &msg, 0) < 0)
            {
                LOG_E(LOG_TAG, "[%s] sendmsg failed. err=%s, erron=%d", __func__, strerror(errno),
                        errno);
                break;
            }

            LOG_D(LOG_TAG, "[%s] send init msg to kernel", __func__);
            /* loop for recv */
            memset(nlh, 0, NLMSG_SPACE(MAX_NL_MSG_LEN));

            while (!Thread::exitPending())
            {
                ret = recvmsg(netlinkSockId, &msg, 0);

                if (ret < 0)
                {
                    LOG_E(LOG_TAG, "[%s] recvmsg failed, ret=%d", __func__, ret);
                    continue;
                }

                if (0 == ret)
                {
                    LOG_E(LOG_TAG, "[%s] recvmsg failed, ret=%d", __func__, ret);
                    continue;
                }
                // recvmsg is block, so need check the thread status
                if (Thread::exitPending())
                {
                    break;
                }
                value = *((char *) NLMSG_DATA(nlh));
                mDevice->handleNetlinkMessage(value);
            }
        }
        while (0);

        LOG_D(LOG_TAG, "[%s] GF netlink thread finish.", __func__);

        if (nlh != NULL)
        {
            free(nlh);
        }

        if ((netlinkSockId != -1) && (netlinkSockId != 0))
        {
            ::close(netlinkSockId);
            netlinkSockId = 0;
        }

        VOID_FUNC_EXIT();
        return false;
    }

    gf_error_t Device::enable_tp(gf_tp_status mode)
    {

        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        hold_wakelock(GF_HOLD_WAKE_LOCK);
        LOG_E(LOG_TAG, "[%s] mode = %d.", __func__, mode);
        do
        {
            char buff[2];
            int fd = ::open(FP_ENABLE_TP_PATH, O_WRONLY);
            if(fd < 0) {
                LOG_E(LOG_TAG, "[%s] GF netlink thread finish.", __func__);
                err =  GF_ERROR_OPEN_DEVICE_FAILED;
                break;
            }
            snprintf(buff, sizeof(buff), "%d", (int)mode);
            write(fd, buff, 2);
            ::close(fd);
        }
        while (0);
        hold_wakelock(GF_RELEASE_WAKE_LOCK);
        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::hold_wakelock(gf_wakelock_status mode)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }
            if (GF_HOLD_WAKE_LOCK == mode) {
                if (ioctl(mFD, GF_IOC_WAKELOCK_TIMEOUT_ENABLE) != 0)
                {
                    LOG_E(LOG_TAG, "[%s] GF_IOC_HOLD_WAKE_LOCK ioctl failed", __func__);
                    err = GF_ERROR_HAL_IOCTL_FAILED;
                    break;
                }
            } else {
                if (ioctl(mFD, GF_IOC_WAKELOCK_TIMEOUT_DISABLE) != 0)
                {
                    LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
                    err = GF_ERROR_HAL_IOCTL_FAILED;
                    break;
                }
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t Device::enable_lcd(gf_lcd_status mode)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();

        do
        {
            char buff[50];
            LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
            int fd = ::open(FP_LCD_TOUCH_DOWN,O_WRONLY);
            if(fd<0) {
                LOG_E(LOG_TAG, "[%s] GF_IOC_RELEASE_WAKE_LOCK ioctl failed", __func__);
            }
            snprintf(buff, sizeof(buff), "%d",(int)mode);
            write(fd,buff,50);
            ::close(fd);
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }

#ifdef __TRUSTONIC
    gf_error_t Device::get_pcb_version(uint16_t* oplus_pcb_ver)
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        int fd = 0;
        int size = 0;
        char pcbver[10];

        do
        {
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

            sscanf((char *)pcbver, "%d", oplus_pcb_ver);
            LOG_I(LOG_TAG, "%s: pcbVersion: %d", __func__, *oplus_pcb_ver);
            ::close(fd);
        }
        while (0);
        FUNC_EXIT(err);
        return err;
    }
#endif

    gf_error_t Device::clear_kernel_touch_flag()
    {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();
        LOG_D(LOG_TAG, "[%s] enter", __func__);

        do
        {
            if (mFD < 0)
            {
                LOG_E(LOG_TAG, "[%s], no device=%s", __func__, GF_DEV_NAME);
                err = GF_ERROR_HAL_FILE_DESCRIPTION_NULL;
                break;
            }

            if (ioctl(mFD, GF_IOC_CLEAN_TOUCH_FLAG) != 0)
            {
                LOG_E(LOG_TAG, "[%s] GF_IOC_CLEAN_TOUCH_FLAG ioctl failed", __func__);
                err = GF_ERROR_HAL_IOCTL_FAILED;
                break;
            }
        }
        while (0);

        FUNC_EXIT(err);
        return err;
    }
}   // namespace goodix
