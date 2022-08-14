#include "plat_time.h"
#include "plat_log.h"
#include "fps_normal.h"
#include "response_def.h"
#include "device_int.h"
#ifndef _WINDOWS
#include <pthread.h>
pthread_t g_thread_on_off, g_thread_ui;
#endif


#if defined(SIMULATE_NO_SENSOR) || defined(RBS_EVTOOL)
#define POLL_TIME 1
#else
#define POLL_TIME 100
#endif

extern int g_hdev;

struct HostTouchEvent {
	BOOL is_enable;
	BOOL is_finger_on;
	BOOL is_low_coverage;
	BOOL is_finger_off;
	BOOL is_ui_ready;
};

func_notify_t g_fn_callback = NULL;

struct HostTouchEvent g_host_touch = {
#ifdef HOST_TOUCH_CONTROL
	.is_enable = TRUE,
#else
	.is_enable = FALSE,
#endif
	.is_finger_on = FALSE,
	.is_finger_off = FALSE,
	.is_low_coverage = FALSE
};
void host_touch_set_enable(BOOL enable) {
	ex_log(LOG_DEBUG, "%s, %d", __func__, enable);
	g_host_touch.is_enable = enable;
	g_host_touch.is_finger_on = FALSE;
	g_host_touch.is_finger_off = FALSE;
	g_host_touch.is_low_coverage = FALSE;
	g_host_touch.is_ui_ready = FALSE;
}

void host_touch_set_finger_on(BOOL on) {
	ex_log(LOG_DEBUG, "%s, %d", __func__, on);
	g_host_touch.is_finger_on = on;
}

void host_touch_set_low_coverage(BOOL flag) {
	ex_log(LOG_DEBUG, "%s, %d", __func__, flag);
	g_host_touch.is_low_coverage = flag;
}

void host_touch_set_finger_off(BOOL off) {
	ex_log(LOG_DEBUG, "%s, %d", __func__, off);
	g_host_touch.is_finger_off = off;
}

void host_touch_set_ui_ready(BOOL ready) {
	ex_log(LOG_DEBUG, "%s, %d", __func__, ready);
	g_host_touch.is_ui_ready = ready;
}

BOOL host_touch_is_enable() {
	return g_host_touch.is_enable;
}

BOOL host_touch_is_finger_on() {
	return g_host_touch.is_finger_on;
}

BOOL host_touch_is_low_coverage() {
	return g_host_touch.is_low_coverage;
}

BOOL host_touch_is_finger_off() {
	return g_host_touch.is_finger_off;
}

BOOL host_touch_is_ui_ready() {
	return g_host_touch.is_ui_ready;
}

BOOL host_touch_is_using_oplus_flow() {
#ifdef __OPLUS__
	extern int g_app_type;
	return (g_app_type == 2)?TRUE:FALSE;
#else
	return FALSE;
#endif
}

oplus_trigger_case_t g_trigger_case = TRIGGER_WAIT_TOUCH_DOWN;

oplus_trigger_case_t ui_trigger_case = TRIGGER_WAIT_UI_READY;

void* wait_oplus_trigger(void* pdata)
{
	unsigned long long time_start_waitfinger = plat_get_time();
	ex_log(LOG_DEBUG, "oplus trigger thread time start = %llu", time_start_waitfinger);
	egis_netlink_threadloop(pdata);

	return NULL;
}

#define TOUCH_DOWN_SET_HBM(x) \
	do{ \
	if(x == FINGERPRINT_RES_SUCCESS) \
		host_touch_set_hbm(TRUE); \
	} while(0)

BOOL wait_trigger(int retry_count, int timeout_for_one_try, int total_timeout)
{
	BOOL bret = FALSE;
	BOOL bWait = TRUE;
	int icount = retry_count;
	int retval = FINGERPRINT_RES_SUCCESS;
	unsigned long long time_start_waitfinger = plat_get_time();
    ex_log(LOG_DEBUG, "time start = %llu", time_start_waitfinger);

	if (host_touch_is_enable()) {
		while (bWait) {
			//plat_sleep_time(POLL_TIME);
            if (total_timeout >= 0){
                if ((int)(plat_get_diff_time(time_start_waitfinger)/1000) > total_timeout){
    				ex_log(LOG_INFO, "!!! enroll timeout !!! =%d",total_timeout);
					bret = FINGERPRINT_RES_SECURE_ENROLL_TIMEOUT;
                    break;
				}
			}
			if (check_cancelable()) break;
			retval = host_touch_is_finger_on() ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_TIMEOUT;
			if (check_cancelable()) break;

			/** OPLUS FLOW **/
			if (host_touch_is_using_oplus_flow())
			{
				if (g_trigger_case == TRIGGER_WAIT_TOUCH_DOWN)
				{
					if (check_cancelable()) break;
						retval = (host_touch_is_ui_ready() && host_touch_is_finger_on())
					 ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_TIMEOUT;
#ifndef __OPLUS__
						TOUCH_DOWN_SET_HBM(retval);
#endif
					if (check_cancelable()) break;
				}else if (g_trigger_case == TRIGGER_WAIT_TOUCH_UP)
				{
					if (check_cancelable()) break;
						retval = host_touch_is_finger_off() ? FINGERPRINT_RES_SUCCESS : FINGERPRINT_RES_TIMEOUT;
					if (check_cancelable()) break;
				}
			}
			/** OPLUS FLOW  END**/

			if (check_cancelable()) break;

			if (retry_count != 0 && icount-- <= 0)
				bWait = FALSE;

			if (FINGERPRINT_RES_SUCCESS == retval) {
				ex_log(LOG_DEBUG, "poll touch event trigger");
				bret = TRUE;

				/** OPLUS FLOW DISABLED **/
				if (!host_touch_is_using_oplus_flow())
				{
					host_touch_set_finger_on(FALSE);
					host_touch_set_finger_off(FALSE);
				}
				/** OPLUS FLOW DISABLED END **/
				break;
			}
		}
		return bret;
	}

#ifdef ENABLE_POLL
	//plat_sleep_time(POLL_TIME);
	ex_log(LOG_DEBUG, "poll trigger");
	return TRUE;
#endif
	retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
	retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_INIT);

	if (retval != FINGERPRINT_RES_SUCCESS) return FALSE;

	while (bWait) {
        if (total_timeout >= 0)
            if ((int)(plat_get_diff_time(time_start_waitfinger)/1000) > total_timeout)
                break;

		if (check_cancelable()) goto exit;
		retval = fp_device_interrupt_wait(g_hdev, timeout_for_one_try);
		if (check_cancelable()) goto exit;

		if (retry_count != 0 && --icount <= 0) bWait = FALSE;

		if (FINGERPRINT_RES_SUCCESS == retval) {
			ex_log(LOG_DEBUG, "interrupt trigger");
			bret = TRUE;
#ifdef __DEVICE_DRIVER_MEIZU_1712__
			// MEIZU_1712 workaround, need abort
			fp_device_interrupt_enable(g_hdev, FLAG_INT_ABORT);
			return bret;
#else
			break;
#endif
		}
	}
exit:
#ifndef _WINDOWS
	fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
#endif
	return bret;
}

poll_revents event_poll_wait(event_check_fd *check_group, uint32_t fd_count, int timeout)
{
	const int sleep_time = 30;
	int count;
	unsigned int i;
	event_check_fd *check;
	poll_revents got_event = REVENT_TIMEOUT;

	if (NULL == check_group) {
		ex_log(LOG_ERROR, "event_poll_wait NULL check_group");
		return -1;
	}

	count = timeout / sleep_time;
	do {
		for (i = 0; i < fd_count; i++) {
			check = check_group + i;
			if (NULL == check->checkerFunc) return -1;
			check->revent = check->checkerFunc();
			if (check->revent) {
				got_event = REVENT_EVENT_POLLIN;
				//ex_log(LOG_DEBUG, "check, %d, %d",i, check->events);
			}
		}
		ex_log(LOG_DEBUG, "spi_poll_wait [%d] ", count);
		if (got_event != REVENT_TIMEOUT) break;

		if (count > 0) {
			plat_sleep_time(sleep_time);
		}
	} while (count-- > 0);

	return got_event;
}

void host_touch_set_callback(func_notify_t callback)
{
	g_fn_callback = callback;	
}

int init_oplus_event_listener(uint8_t fd)
{
#ifndef _WINDOWS
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,1);
	pthread_create(&g_thread_ui, &attr, wait_oplus_trigger, (void *)(unsigned long)(fd));
	
#endif
	return 0;
}

void host_touch_set_hbm_evalute_always(void)
{
	extern int g_app_type;
	if (g_app_type == 2)
		return;
	#ifndef __OPLUS__
	host_touch_set_hbm(1);
	#endif
}

void host_touch_set_hbm_system(BOOL on_off)
{

/*	extern int g_app_is_using_jni;
	if (g_app_is_using_jni != 2)
		return;
	host_touch_set_hbm(on_off);
	plat_sleep_time(50);
*/
}

void host_touch_set_hbm(BOOL on_off)
{
	FILE *fp;
	int ret = FINGERPRINT_RES_SUCCESS;
#ifdef __OPLUS__
#define HBM_PATH "/sys/kernel/oplus_display/hbm"
	ex_log(LOG_DEBUG, "%s [%d] ",__func__, on_off);
	fp = fopen(HBM_PATH, "wb");
	if (fp == NULL) {
		ex_log(LOG_ERROR, "%s, can't open file", __func__);
		fclose(fp);
	}
	ret = fwrite(on_off?"1":"0", 1, 1, fp);
	fclose(fp);
#endif
}

