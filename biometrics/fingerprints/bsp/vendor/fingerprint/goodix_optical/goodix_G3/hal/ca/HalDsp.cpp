/************************************************************************************
 ** File: - HalDsp.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2019, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HalDsp for goodix optical fingerprint (android P)
 **
 ** Version: 1.0
 ** Date : 15:03:11,14/05/2019
 ** Author: Ran.Chen@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>       <data>            <desc>
 **  Ran.Chen   2019/05/14         modify for getlock fail
 ************************************************************************************/

#ifdef SUPPORT_DSP
#define LOG_TAG "[GF_HAL][HalDsp]"

#include <time.h>
#include "HalDsp.h"
#include "CaEntry.h"
#include "HalLog.h"
#include <string.h>

#define TA_DSP_SHARE_BUFFER_SIZE (4 * 1024 * 1024)
#define GF_HXV_PROXY_STUB_LIB "/vendor/lib64/libhvx_proxy_stub.so"
#define CDSP_LOG (0)  // 1 enable, 0 disable cdsp log dumping to hal


extern bool mDspSupport;


namespace goodix {
    void *g_proxy_handle = NULL;

    int (*gf_proxy_dsp_init)(int32_t, int32_t, uint8_t) = NULL;
    int (*gf_proxy_dsp_reinit)(int32_t, int32_t, uint8_t) = NULL;
    int (*gf_proxy_dsp_deinit)(void) = NULL;
    sem_t* (*gf_proxy_get_sem)(void) = NULL;
    int (*gf_proxy_set_high_dsp_freq)(int32_t, int32_t, int32_t) = NULL;
    int (*gf_proxy_set_normal_dsp_freq)(void) = NULL;
    int (*gf_proxy_send_cmd_to_dsp)(dsp_cmd_type) = NULL;
    int (*gf_proxy_get_dsp_result)(void) = NULL;

    HalDsp::HalDsp(CaEntry *caEntry) : mDspTime(0), mDspStatus(DSP_NOT_AVAILABLE)
    {
        mCaEntry = caEntry;
        memset(&mProxyCmd, 0, sizeof(gf_dsp_proxy_cmd_t));
    }

    HalDsp::~HalDsp()
    {
    }

    gf_error_t HalDsp::proxyHandleInit(void)
    {
        gf_error_t err = GF_SUCCESS;

        FUNC_ENTER();

        do
        {
            if (NULL != g_proxy_handle)
            {
                dlclose(g_proxy_handle);
                LOG_E(LOG_TAG, "[%s] handle is not null, close the handle", __func__);
            }

            g_proxy_handle = dlopen(GF_HXV_PROXY_STUB_LIB, RTLD_NOW);

            if (NULL == g_proxy_handle)
            {
                LOG_E(LOG_TAG, "[%s] open %s fail", __func__, GF_HXV_PROXY_STUB_LIB);
                err = GF_ERROR_GENERIC;
                break;
            }

            gf_proxy_dsp_init = (int (*)(int32_t, int32_t, uint8_t))dlsym(g_proxy_handle, "gf_proxy_dsp_init");
            gf_proxy_dsp_reinit = (int (*)(int32_t, int32_t, uint8_t))dlsym(g_proxy_handle, "gf_proxy_dsp_reinit");
            gf_proxy_dsp_deinit = (int (*)(void))dlsym(g_proxy_handle, "gf_proxy_dsp_deinit");
            gf_proxy_get_sem = (sem_t * (*)(void))dlsym(g_proxy_handle, "gf_proxy_get_sem");
            gf_proxy_set_high_dsp_freq = (int (*)(int32_t, int32_t, int32_t))dlsym(g_proxy_handle, "gf_proxy_set_high_dsp_freq");
            gf_proxy_set_normal_dsp_freq = (int (*)(void))dlsym(g_proxy_handle, "gf_proxy_set_normal_dsp_freq");
            gf_proxy_send_cmd_to_dsp = (int (*)(dsp_cmd_type))dlsym(g_proxy_handle, "gf_proxy_send_cmd_to_dsp");
            gf_proxy_get_dsp_result = (int (*)(void))dlsym(g_proxy_handle, "gf_proxy_get_dsp_result");

            if (NULL == gf_proxy_dsp_init || NULL == gf_proxy_dsp_reinit
                || NULL == gf_proxy_dsp_deinit || NULL == gf_proxy_get_sem
                || NULL == gf_proxy_set_high_dsp_freq || NULL == gf_proxy_set_normal_dsp_freq
                || NULL == gf_proxy_send_cmd_to_dsp || NULL == gf_proxy_get_dsp_result)
            {
                LOG_E(LOG_TAG, "[%s] analyze function fail", __func__);
                dlclose(g_proxy_handle);
                g_proxy_handle = NULL;
                err = GF_ERROR_GENERIC;
                break;
            }
        }
        while (0);

        if (NULL == g_proxy_handle)
        {
            gf_proxy_dsp_init = NULL;
            gf_proxy_dsp_reinit = NULL;
            gf_proxy_dsp_deinit = NULL;
            gf_proxy_get_sem = NULL;
            gf_proxy_set_high_dsp_freq = NULL;
            gf_proxy_set_normal_dsp_freq = NULL;
            gf_proxy_send_cmd_to_dsp = NULL;
            gf_proxy_get_dsp_result = NULL;
        }

        FUNC_EXIT(err);
        return err;
    }

    void HalDsp::proxyHandleDeinit(void)
    {
        FUNC_ENTER();

        if (g_proxy_handle != NULL)
        {
            dlclose(g_proxy_handle);
            g_proxy_handle = NULL;
        }

        VOID_FUNC_EXIT();
    }

    gf_error_t HalDsp::init()
    {
        gf_error_t err = GF_SUCCESS;
        uint32_t ret = 0;
        int32_t fd = -1;
        int32_t len;

        FUNC_ENTER();

        do
        {
            mDspStatus = DSP_NOT_AVAILABLE;
            err = proxyHandleInit();

            if (GF_SUCCESS != err)
            {
                LOG_E(LOG_TAG, "[%s] hal proxy handle init failed, ret:%d", __func__, ret);
                break;
            }

            usleep(500 * 1000);
            // run thread
            if (!isRunning())
            {
                if (!run("dsp_handle_thread"))
                {
                    err = GF_ERROR_DSP_NOT_AVAILABLE;
                }
            }
            else
            {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
            }

            if (GF_SUCCESS != err) {
                LOG_E(LOG_TAG, "[%s] pthread create failed", __func__);
                break;
            }

            mCaEntry->getCarveoutFdInfo(&fd, &len);
            ret = gf_proxy_dsp_init(fd, len, CDSP_LOG);

            if (0 != ret)
            {
                LOG_E(LOG_TAG, "[%s] proxy dsp init fail, reinit dsp. ret:%d", __func__, ret);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            // wait map addr complete
            err = waitDspNotify(0, 200 * 1000);
            if (GF_SUCCESS != err)
            {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            mDspStatus = DSP_AVAILABLE;

            /* set freq to high */
            sendCmdToDsp(DSP_CMD_SET_HIGH_SPEED);
            sendCmdToDsp(DSP_CMD_GET_VERSION);
            /* set freq to normal */
            sendCmdToDsp(DSP_CMD_SET_NORMAL_SPEED);
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::reinit()
    {
        gf_error_t err = GF_SUCCESS;
        uint32_t ret = 0;
        int32_t fd = -1;
        int32_t len;
        uint32_t retry_count = 5;

        FUNC_ENTER();

        do
        {
            usleep(500 * 1000);
            mDspStatus = DSP_INITIALIZING;
            ret = proxyHandleInit();
            if (ret)
            {
                LOG_E(LOG_TAG, "[%s] hal proxy handle init failed, ret:%d", __func__, ret);
                usleep(200 * 1000);
                continue;
            }

            mCaEntry->getCarveoutFdInfo(&fd, &len);

            ret = gf_proxy_dsp_reinit(fd, len, CDSP_LOG);
            if (ret)
            {
                LOG_E(LOG_TAG, "[%s] proxy dsp reinit fail, ret:%d", __func__, ret);
                usleep(200 * 1000);
                continue;
            }

            // wait map addr complete
            ret = waitDspNotify(0, 200 * 1000);
            if (ret)
            {
                usleep(200 * 1000);
                continue;
            }

            // fixed reinit lead to power consumption issue
            ret = setDspHighFreq();
            if (ret)
            {
                usleep(200 * 1000);
                continue;
            }

            usleep(20 * 1000);
            ret = setDspNormalFreq();
            if (ret)
            {
                usleep(200 * 1000);
                continue;
            }
        } while(retry_count-- > 0 && ret);

        mDspStatus = DSP_AVAILABLE;
        LOG_D(LOG_TAG, "[%s] reinit success", __func__);

        if (ret)
        {
            mDspStatus= DSP_NOT_AVAILABLE;
            err = GF_ERROR_DSP_NOT_AVAILABLE;
            LOG_E(LOG_TAG, "[%s] reinit fail", __func__);
        }

        FUNC_EXIT(err);
        return err;
    }
    gf_error_t HalDsp::deinit()
    {
        FUNC_ENTER();
        gf_error_t err = GF_SUCCESS;

        {
            Mutex::Autolock _l(mDspMutex);
            gf_proxy_dsp_deinit();
            proxyHandleDeinit();
        }

        mProxyCmd.type = DSP_PROXY_EXIT;
        requestExit();
        mCond.signal();

        FUNC_EXIT(err);
        return err;
    }

    sem_t* HalDsp::getDspSem()
    {

        if (gf_proxy_get_sem != NULL)
        {
            return gf_proxy_get_sem();
        }
        else
        {
            return NULL;
        }
    }

    gf_error_t HalDsp::waitDspNotify(uint32_t wait_time_sec, uint32_t wait_time_us) {
        gf_error_t err = GF_SUCCESS;
        int ret_sem = -1;
        struct timespec abs_time;
        uint32_t wait_time_nsec = wait_time_us * 1000;

        FUNC_ENTER();

        do
        {
            if (clock_gettime(CLOCK_REALTIME, &abs_time) < 0)
            {
                LOG_E(LOG_TAG, "[%s] clock gettime fail", __func__);
                err = GF_ERROR_BASE;
                break;
            }

            // set wait time, TODO: when system time change,timeout??
            abs_time.tv_sec += wait_time_sec;
            abs_time.tv_nsec += wait_time_nsec;
            abs_time.tv_sec += abs_time.tv_nsec / 1000000000;   // Nanoseconds [0 ..999999999], to avoid overflow
            abs_time.tv_nsec = abs_time.tv_nsec % 1000000000;

            sem_t* sem_temp = getDspSem();
            if (NULL == sem_temp)
            {
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            LOG_D(LOG_TAG, "[%s] hal begin wait sem", __func__);
            ret_sem = sem_timedwait(sem_temp, &abs_time);
            LOG_D(LOG_TAG, "[%s] hal end wait sem, ret_sem:%d", __func__, ret_sem);

            if (0 != ret_sem)
            {
                err = GF_ERROR_DSP_WAIT_TIMEOUT;
                break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::waitDspNotify() {
        gf_error_t err = GF_SUCCESS;
        int32_t lockerr = 0;
        uint32_t i = 0;

        FUNC_ENTER();

        do {
            if (mDspStatus != DSP_AVAILABLE) {
                LOG_D(LOG_TAG, "[%s] dsp not available", __func__);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            do {
                lockerr = mDspMutex.tryLock();
                if (lockerr) {
                    LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available,lockerr =%d", __func__,lockerr);
                    err = GF_ERROR_DSP_NOT_AVAILABLE;
                    usleep(100);
                } else {
                    break;
                }
                i++;
            } while (i < 5);

            if (i == 5 && err == GF_ERROR_DSP_NOT_AVAILABLE) {
                LOG_E(LOG_TAG, "[%s] trylock %d fail, dsp not available", __func__, i);
                break;
            }

            err = waitDspNotify(0, 500 * 1000);
            if (GF_SUCCESS == err) {
                uint32_t dsp_result = 0;

                dsp_result = gf_proxy_get_dsp_result();
                if (dsp_result)
                {
                    LOG_E(LOG_TAG, "[%s] dsp get feature fail, result:0x%x", __func__, dsp_result);
                    err = GF_ERROR_DSP_GET_FEATURE_FAIL;
                }
            }

            mDspMutex.unlock();
        } while (0);

        if (GF_SUCCESS != err) {
            sendCmdToDsp(DSP_CMD_SET_NORMAL_SPEED);
            mDspStatus = DSP_NOT_AVAILABLE;
        }

        FUNC_EXIT(err);
        return err;
    }

    gf_dsp_status_t HalDsp::checkDspValid() {
        LOG_D(LOG_TAG, "[%s] dsp status:%d", __func__, mDspStatus);

        if (mDspMutex.tryLock()) {
            LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
            return DSP_NOT_AVAILABLE;
        }

        mDspMutex.unlock();

        return mDspStatus;
    }

    void HalDsp::clearDspSem() {
        gf_error_t err = GF_SUCCESS;

        VOID_FUNC_ENTER();

        do {
            if (mDspStatus != DSP_AVAILABLE) {
                LOG_D(LOG_TAG, "[%s] dsp not available", __func__);
                break;
            }

            if (mDspMutex.tryLock()) {
                LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
                break;
            }

            while (1) {
                int count = 0;
                sem_t* sem_temp = getDspSem();
                if (NULL == sem_temp)
                {
                    err = GF_ERROR_DSP_NOT_AVAILABLE;
                    break;
                }

                sem_getvalue(sem_temp, &count);
                if (count > 0) {
                    err = waitDspNotify(0, 1);  //wait 1US
                    if (GF_SUCCESS != err) {
                        LOG_E(LOG_TAG, "[%s] wait notify fail", __func__);
                        break;
                    } else {
                        LOG_D(LOG_TAG, "[%s] there is dsp sem and decrease it", __func__);
                    }
                } else {
                    LOG_D(LOG_TAG, "[%s] there is no dsp sem", __func__);
                    break;
                }
            }

            mDspMutex.unlock();

        } while (0);

        VOID_FUNC_EXIT();
    }

    gf_error_t HalDsp::dspProxySendCmd(gf_dsp_proxy_cmd_t *proxy_cmd) {
        gf_error_t err = GF_SUCCESS;
        uint32_t retry_count = 1000;

        FUNC_ENTER();

        do
        {
            if (mDspStatus != DSP_AVAILABLE)
            {
                LOG_E(LOG_TAG, "[%s] dsp not available", __func__);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            if (mDspMutex.tryLock())
            {
                LOG_E(LOG_TAG, "[%s] trylock fail, dsp not available", __func__);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            memcpy(&mProxyCmd, proxy_cmd, sizeof(gf_dsp_proxy_cmd_t));

            mDspMutex.unlock();
            mCond.signal();

            while (retry_count-- > 0 && SEND_CMD_DOING == mProxyCmd.status)
            {
                usleep(200);
            }

            if (mProxyCmd.status != SEND_CMD_FINISH)
            {
                LOG_E(LOG_TAG, "[%s] send cmd fail, type:%d, status:%d", __func__,
                      mProxyCmd.type, mProxyCmd.status);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
            }

        } while(0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::sendCmdToDsp(gf_dsp_cmd_t cmd) {
        gf_error_t err = GF_SUCCESS;
        gf_dsp_proxy_cmd_t proxy_cmd;

        FUNC_ENTER();

        do {
            LOG_D(LOG_TAG, "[%s] cmd type:%d", __func__, cmd);

            if (mDspStatus != DSP_AVAILABLE) {
                LOG_D(LOG_TAG, "[%s] dsp not available", __func__);
                err = GF_ERROR_DSP_NOT_AVAILABLE;
                break;
            }

            memset(&proxy_cmd, 0, sizeof(gf_dsp_proxy_cmd_t));
            switch (cmd) {
                case DSP_CMD_GET_VERSION: {
                    proxy_cmd.type = DSP_PROXY_SEND_CMD;
                    proxy_cmd.cmd_type = SEC_FP_HVX_GET_VERSION;
                    err = dspProxySendCmd(&proxy_cmd);
                    break;
                }

                case DSP_CMD_GET_FEATURE_TWO: {
                    clearDspSem();

                    proxy_cmd.type = DSP_PROXY_SEND_CMD;
                    proxy_cmd.cmd_type = SEC_FP_HVX_GET_FEATURE_TWO;
                    err = dspProxySendCmd(&proxy_cmd);
                    break;
                }

                case DSP_CMD_GET_FEATURE_FOUR: {
                    clearDspSem();

                    proxy_cmd.type = DSP_PROXY_SEND_CMD;
                    proxy_cmd.cmd_type = SEC_FP_HVX_GET_FEATURE_FORE;
                    err = dspProxySendCmd(&proxy_cmd);
                    break;
                }

                case DSP_CMD_AGING_TEST: {
                    clearDspSem();

                    proxy_cmd.type = DSP_PROXY_SEND_CMD;
                    proxy_cmd.cmd_type = SEC_FP_HVX_AGING_TEST;
                    err = dspProxySendCmd(&proxy_cmd);
                    break;
                }

                case DSP_CMD_SET_HIGH_SPEED: {
                    // power_level : the level of dsp frequency, 0~5(0 is the highest frequency 1190.4M, 5 is about 400M)
                    // latency :     the time of wake up dsp when it sleep, units is ms;
                    // dcvs_enable : enable detector whether dsp is running, when it doesn't run,
                    //               it will decrease the frequency
                    //               0: disable      1: enable
                    proxy_cmd.type = DSP_PROXY_SET_HIGH_FREQ;
                    proxy_cmd.high_freq.power_level = 0;
                    proxy_cmd.high_freq.latency = 1000;
                    proxy_cmd.high_freq.dcvs_enable = 0;
                    err = dspProxySendCmd(&proxy_cmd);
                    break;
                }

                case DSP_CMD_SET_NORMAL_SPEED: {
                    // set the frequency of dsp to normal (288M)
                    proxy_cmd.type = DSP_PROXY_SET_NORMAL_FREQ;
                    err = dspProxySendCmd(&proxy_cmd);
                    break;
                }

                default:
                    LOG_E(LOG_TAG, "[%s] not support", __func__);
                    break;
            }
        } while (0);

        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::setDspHighFreq() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();


        if (mDspSupport) {
            int ret = 0;
            ret = gf_proxy_set_high_dsp_freq(0, 1000, 0);

            if (0 != ret) {
                LOG_E(LOG_TAG, "[%s] hal set high dsp freq fail", __func__);
                err = GF_ERROR_BASE;
            }
        }


        FUNC_EXIT(err);
        return err;
    }

    gf_error_t HalDsp::setDspNormalFreq() {
        gf_error_t err = GF_SUCCESS;
        FUNC_ENTER();


        if (mDspSupport) {
            int ret = 0;
            ret = gf_proxy_set_normal_dsp_freq();

            if (0 != ret)
            {
                LOG_E(LOG_TAG, "[%s] hal set normal dsp frequency fail", __func__);
                err = GF_ERROR_BASE;
            }
        }


        FUNC_EXIT(err);
        return err;
    }

    bool HalDsp::threadLoop() {
        while (!Thread::exitPending()) {
            LOG_D(LOG_TAG, "[%s] thread runing", __func__);
            Mutex::Autolock _l(mDspMutex);

            while (1) {
                int ret = 0;
                uint8_t reinit_flag = 0;

                mCond.wait(mDspMutex);
                // wait for comdition
                switch (mProxyCmd.type) {
                    case DSP_PROXY_SEND_CMD: {
                        ret = gf_proxy_send_cmd_to_dsp(mProxyCmd.cmd_type);
                        if (ret) {
                            reinit_flag = 1;
                            mProxyCmd.status = SEND_CMD_ERROR;
                            break;
                        }
                        mProxyCmd.status = SEND_CMD_FINISH;
                        break;
                    }

                    case DSP_PROXY_SET_HIGH_FREQ: {
                        ret = gf_proxy_set_high_dsp_freq(mProxyCmd.high_freq.power_level,
                                                         mProxyCmd.high_freq.latency,
                                                         mProxyCmd.high_freq.dcvs_enable);
                        if (ret) {
                            reinit_flag = 1;
                            mProxyCmd.status = SEND_CMD_ERROR;
                            break;
                        }
                        mProxyCmd.status = SEND_CMD_FINISH;
                        break;
                    }

                    case DSP_PROXY_SET_NORMAL_FREQ: {
                        ret = gf_proxy_set_normal_dsp_freq();
                        if (ret) {
                            reinit_flag = 1;
                            mProxyCmd.status = SEND_CMD_ERROR;
                            break;
                        }
                        mProxyCmd.status = SEND_CMD_FINISH;
                        break;
                    }

                    default:
                        break;
                }

                if (reinit_flag) {
                    mDspStatus= DSP_NOT_AVAILABLE;
                    reinit();
                }

                if (DSP_PROXY_EXIT == mProxyCmd.type) {
                    LOG_D(LOG_TAG, "[%s] exit thread", __func__);
                    break;
                }
            }
        }

        return false;
    }
}  // namespace goodix
#endif   //SUPPORT_DSP
