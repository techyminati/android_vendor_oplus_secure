#include "FpMonitorTool.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cutils/properties.h>
#include "HalLog.h"
#include <time.h>
#include <sys/time.h>
Device *g_dev;
FILE *g_output_stream = NULL;

#define FP_MONITOR_PROP "persist.vendor.fingerprint.optical.monitor"
#define TOUCH_DOWN_TICK (2)
#define DEFAULT_ERROR_PID (-1)
pthread_t g_pid_down = DEFAULT_ERROR_PID;

static int create_file_handle()
{
    int ret = 0;
    char filename[128] = {'\0'};
    tm *t;
    int len;
    time_t t2 = time(NULL) ;
    t = localtime(&t2);
    len = sprintf(filename, "/data/vendor/fingerprint/fingerprint-rawdata-%d-%d-%d-%d-%d-%d.csv",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    LOG_D(LOG_TAG, "[%s] file name: %s", __func__, filename);
    if (len < 0) {
        return -1;
    }
    if (g_output_stream != NULL) {
        LOG_E(LOG_TAG, "[%s] file already create: %s", __func__, filename);
        return -2;
    }
    g_output_stream = fopen(filename, "w+");
    if (g_output_stream == NULL) {
        LOG_E(LOG_TAG, "[%s] open file error: %s", __func__, filename);
        return -3;
    }
    return ret;
}

static void destory_file_handle()
{
    if (g_output_stream) {
        fclose(g_output_stream);
        g_output_stream = NULL;
    }
}

void save_rawdata(int rawdata, int retry_count, int round, int screen_state)
{
    int ret;
    (void)retry_count;
    (void)round;
    (void)screen_state;
    if (g_output_stream == NULL) {
        LOG_E(LOG_TAG, "[%s] file not create ,exist", __func__);
        return;
    }
    LOG_D(LOG_TAG, "[%s] rawdata: %d", __func__, rawdata);
    ret = fprintf(g_output_stream, "%d,%d,%d\n", retry_count, round, rawdata);
    if (ret  < 0) {
        LOG_E(LOG_TAG, "[%s] write file fail", __func__);
        return;
    }
    ret = fflush(g_output_stream);
    if (ret  < 0) {
        LOG_E(LOG_TAG, "[%s] fflush fail", __func__);
    }
}

bool is_rawdata_mode()
{
    bool ret = false;
    char monitor_prop[PROPERTY_VALUE_MAX] = { '\0' };
    int len = property_get(FP_MONITOR_PROP, monitor_prop, NULL);
    LOG_D(LOG_TAG, "[%s] monitor property value %s", __func__, (char *)monitor_prop);
    if (len <= 0) {
        ret = false;;
    } else {
        if (strcmp((char *)monitor_prop, "1") == 0) {
            ret = true;
        } else {
            ret = false;;
        }
    }
    return ret;
}

void *touch_down_thread(void *args) {
    char monitor_prop[PROPERTY_VALUE_MAX] = { '\0' };
    int len = 0;
    (void)args;
    LOG_I(LOG_TAG, "[%s] monitor thread start", __func__);
    pthread_detach(pthread_self());
    if (g_dev == NULL) {
        LOG_E(LOG_TAG, "[%s] dev is null", __func__);
        goto out;
    }
    while (1) {
        g_dev->automatic_send_touchup();
        sleep(TOUCH_DOWN_TICK);
        if (is_rawdata_mode()) {
            g_dev->automatic_send_touchdown();
        } else {
            break;
        }
        sleep(TOUCH_DOWN_TICK);
    }
out:
    g_pid_down = DEFAULT_ERROR_PID;
    destory_file_handle();
    return NULL;
}

void fp_monitor_init(Device *dev)
{
    int ret = -1;
    LOG_I(LOG_TAG, "[%s] enter", __func__);

    if (dev == NULL) {
        LOG_E(LOG_TAG, "[%s] para error", __func__);
        return;
    }
    // 1. set g_dev communicate with kernel driver
    g_dev = dev;

    // 2. create touch down thread
    if (g_pid_down == DEFAULT_ERROR_PID) {
        LOG_I(LOG_TAG, "[%s] monitor thread init", __func__);
        ret = pthread_create(&g_pid_down, NULL, touch_down_thread, NULL);
        if (ret != 0) {
            LOG_E(LOG_TAG, "%s, Create thread error!\n", __func__);
            return;
        }
    }
    // 3. create file handle
    if (g_output_stream == NULL) {
        ret = create_file_handle();
        if (ret) {
            LOG_E(LOG_TAG, "[%s] create file error", __func__);
            return;
        }
    }
    LOG_I(LOG_TAG, "[%s] exist", __func__);
}
