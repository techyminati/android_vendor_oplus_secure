#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "constant_def.h"
#include "response_def.h"
#include "ex_define.h"
#include "type_def.h"
#include "op_manager.h"
#include "plat_log.h"
#include "fps_normal.h"
#include "navi_manager.h"
#include "navi_operator.h"
#include "navi_def.h"
#include "captain.h"
#include "thread_manager.h"
#include "common_definition.h"
#ifndef _WINDOWS
#include <unistd.h>
#else
#include <Windows.h>
#include <io.h>
#define open(file_name, flog, mode) _open(file_name, flog, mode)
#define write(fd, buffer, count) _write(fd, buffer, count)
#define close(fd) _close(fd)
#endif

#define ENABLE_SEND_NAVI_EVENT_TO_DRIVER
#define NAVI_PATH \
	"/sys/devices/platform/egis_input/navigation_event"  // "/data/navii"
#define NAVI_ENABLE_FLAG_PATH \
	"/sys/devices/platform/egis_input/navigation_enable"  // "/data/navii"

int g_navi_x_h_threshold = 7;
int g_navi_x_l_threshold = 3;
int g_navi_y_h_threshold = 7;
int g_navi_y_l_threshold = 3;
static unsigned long g_fast_right_threshold = 150;
static unsigned long g_fast_left_threshold = 70;

int g_navi_swipe_x = 1;
int g_navi_swipe_y = 1;

extern event_callbck_t g_event_callback;
extern int g_hdev;
BOOL g_navi_need_cancel = FALSE;
BOOL g_has_sent_event = FALSE;
int g_only_send_one_event = TRUE;

#ifdef __NAVI_DOUBLE_CLICK__
unsigned long g_last_click_time;
#endif

#ifndef _WINDOWS
static unsigned long GetTickCount(void)
{
	struct timeval tv;
	unsigned long tick;

	gettimeofday(&tv, NULL);
	tick = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

	return tick;
}
#else
#define __unused
#endif

int set_event(const char *file_name, unsigned char *in_buffer,
	      int in_buffer_size)
{
	ex_log(LOG_DEBUG, "set_event enter!");
	if (NULL == in_buffer || 0 == in_buffer_size) {
		return FINGERPRINT_RES_INVALID_PARAM;
	}

	int event, path_len, write_len;
	int retval = FINGERPRINT_RES_SUCCESS;
	int fd = 0;
	char file_path[128] = {0};
	char file_full_name[256];

	event = in_buffer[0];
	path_len = in_buffer[1];

	memcpy(file_path, &in_buffer[2], path_len);
#if !defined(QSEE)
	snprintf(file_full_name, sizeof(file_full_name) - 1, "%s/%s", file_path, file_name);
#endif
	ex_log(LOG_DEBUG, "set_event file_full_name= event= %s,%d",
	       file_full_name, event);

	fd = open(file_full_name, O_CREAT | O_RDWR | O_TRUNC, 0600);
	if (fd <= 0) {
		close(fd);
		ex_log(LOG_ERROR, "set_event open file faild");
		retval = FINGERPRINT_RES_OPEN_FILE_FAILED;
		goto EXIT;
	}

	write_len = write(fd, &event, sizeof(event));
	if ((unsigned long)write_len < sizeof(event)) {
		retval = VENDOR_FILE_WRITE_FAILED;
	}

#ifndef _WINDOWS
	if (0 != fsync(fd)) {
		ex_log(LOG_ERROR, "set_event fsync is fail , errno = %d ",
		       errno);
	}
#endif
	close(fd);
	fd = 0;

EXIT:
	ex_log(LOG_DEBUG, "set_event end! retval = %d", retval);
	return retval;
}

#ifdef ENABLE_SEND_NAVI_EVENT_TO_DRIVER
static unsigned char transfrom_event_number_for_driver(int event)
{
	unsigned char event_code = NAVI_EVENT_UNKNOW;

	switch (event) {
		case EVENT_HOLD:
			event_code = NAVI_EVNT_LCLICK;
			break;
		case EVENT_CLICK:
			event_code = NAVI_EVNT_CLICK;
			break;
		case EVENT_DCLICK:
			event_code = NAVI_EVNT_DCLICK;
			break;
		case EVENT_UP:
			event_code = NAVI_EVENT_UP;
			break;
		case EVENT_DOWN:
			event_code = NAVI_EVENT_DOWN;
			break;
		case EVENT_LEFT:
			event_code = NAVI_EVENT_LEFT;
			break;
		case EVENT_RIGHT:
			event_code = NAVI_EVENT_RIGHT;
			break;
		case EVENT_FINGERON:
			event_code = NAVI_EVENT_ON;
			break;
		case EVENT_FINGEROFF:
			event_code = NAVI_EVENT_OFF;
			break;
		case EVENT_UNKNOWN:
			ex_log(LOG_ERROR, "NAVI_EVENT_UNKNOW");
			break;
		case EVENT_LOST:
			ex_log(LOG_ERROR, "NAVI_EVENT_UNKNOW");
			break;
		default:
			ex_log(LOG_ERROR, "NAVI_EVENT_UNKNOW");
			break;
	}

	return event_code;
}
#endif

int send_navi_event_to_driver(int event)
{
	int ret = FINGERPRINT_RES_SUCCESS;
	ex_log(LOG_DEBUG, "event = %d", event);
#ifdef ENABLE_SEND_NAVI_EVENT_TO_DRIVER
	FILE *fp;
	unsigned char event_code = transfrom_event_number_for_driver(event);

	if (event_code != NAVI_EVENT_UNKNOW) {
		ex_log(LOG_DEBUG, "%s, event_code = %d", __func__, event_code);
		fp = fopen(NAVI_PATH, "wb");
		if (fp == NULL) {
			ex_log(LOG_ERROR, "%s, can't open file", __func__);
			return FINGERPRINT_RES_OPEN_FILE_FAILED;
		}
		ret = fwrite((void *)&event_code, 1, 1, fp);
		fclose(fp);
	}
#endif
	return ret;
}

BOOL navi_is_enable()
{
	unsigned char* buf = NULL;
	int len = 0, ret = FALSE, buf_size = 128;
	FILE *fp = NULL;

	fp = fopen(NAVI_ENABLE_FLAG_PATH, "r");
	if (fp == NULL) {
		ex_log(LOG_ERROR, "%s can't open file", __func__);
		return ret;
	}

	buf = (unsigned char *)malloc(buf_size);
	if(buf == NULL) {
		ex_log(LOG_ERROR, "%s malloc fail", __func__);
		fclose(fp);
		return ret;
	}

	len = fread(buf, 1, buf_size, fp);
	ex_log(LOG_DEBUG, "%s len =%d buf=%s", __func__, len, buf);
	fclose(fp);

	if (len > 0) {
		if (!strncmp((const char *)buf, "enable", strlen("enable"))) {
			ex_log(LOG_DEBUG, "Navi enable");
			ret = TRUE;
		} else if (!strncmp((const char *)buf, "disable", strlen("disable")))
			ex_log(LOG_DEBUG, "Navi disable");
		else
			ex_log(LOG_ERROR, "strcmp not match");
	}

	if (buf != NULL) {
		free(buf);
		buf = NULL;
	}

	return ret;
}

int send_navi_event(__unused enum navi_event event) { return 0; }
void send_swipe_event(int *x, int *y, int x_threshold, int y_threshold)
{
	ex_log(LOG_DEBUG,
	       "send_swipe_event x = %d , y = %d , x_threshold = %d , "
	       "y_threshold = %d ",
	       *x, *y, x_threshold, y_threshold);

	BOOL temp_condition = FALSE;
	enum navi_event sendevent = NAVI_EVENT_CANCEL;

	if (g_navi_swipe_x && g_navi_swipe_y) {
		// to do
	} else if (g_navi_swipe_x) {
		if ((abs(*y) > NAVI_Y_BREAK_THRESHOLD) ||
		    (abs(*x) - abs(*y) < NAVI_XY_DIFF)) {
			*x = 0;
			return;
		}
	} else if (g_navi_swipe_y) {
		// to do
	}

	if (g_navi_swipe_x) {
		temp_condition = (abs(*x) >= x_threshold);
	}

	if (g_navi_swipe_y) {
		temp_condition = (abs(*y) >= y_threshold);
	}

	if (g_navi_swipe_x && g_navi_swipe_y) {
		temp_condition =
		    (abs(*x) >= x_threshold) || (abs(*y) >= y_threshold);
	}

	if (temp_condition) {
		if (abs(*x) >= abs(*y)) {
			if (*x > 0) {
				sendevent = NAVI_EVENT_LEFT;
			} else {
				sendevent = NAVI_EVENT_RIGHT;
			}
		} else {
			if (*y > 0) {
				sendevent = NAVI_EVENT_UP;
			} else {
				sendevent = NAVI_EVENT_DOWN;
			}
		}

		*x = 0;
		*y = 0;

		if (TRUE == g_only_send_one_event) {
			if (NULL == g_event_callback) {
				ex_log(LOG_ERROR, "g_event_callback == NULL");
				goto SEND_EVENT;
			}

			if (NAVI_EVENT_RIGHT == sendevent) {
				ex_log(LOG_DEBUG, "[event] NAVI_EVENT_RIGHT ");
				g_event_callback(EVENT_NAVIGATION,
						 NAVI_EVENT_RIGHT, 0, NULL, 0);

			} else if (NAVI_EVENT_LEFT == sendevent) {
				ex_log(LOG_DEBUG, "[event] NAVI_EVENT_LEFT ");
				g_event_callback(EVENT_NAVIGATION,
						 NAVI_EVENT_LEFT, 0, NULL, 0);

			} else if (NAVI_EVENT_UP == sendevent) {
				ex_log(LOG_DEBUG, "[event] NAVI_EVENT_UP ");
				g_event_callback(EVENT_NAVIGATION,
						 NAVI_EVENT_UP, 0, NULL, 0);

			} else if (NAVI_EVENT_DOWN == sendevent) {
				ex_log(LOG_DEBUG, "[event] NAVI_EVENT_DOWN ");
				g_event_callback(EVENT_NAVIGATION,
						 NAVI_EVENT_DOWN, 0, NULL, 0);
			}
		SEND_EVENT:
			send_navi_event(sendevent);

			g_only_send_one_event = FALSE;
		}
	}
}

#define DOUBLE_CLICK_DURATION 700  // ms
int send_navi_event_512(int navi_event, int value2)
{
#ifdef __NAVI_DOUBLE_CLICK__
	unsigned long currentTime;
#endif

	if (g_has_sent_event) {
		return FINGERPRINT_RES_FAILED;
	}

	g_has_sent_event = TRUE;

	ex_log(LOG_DEBUG, "send_navi_event %d, %d", navi_event, value2);
	switch (navi_event) {
		case NAVI_EVNT_CLICK: {
#ifdef __NAVI_DOUBLE_CLICK__
			ex_log(LOG_DEBUG, "NAVI_DOUBLE_CLICK");

			currentTime = GetTickCount();

			if (0 == g_last_click_time ||
			    currentTime - g_last_click_time >
				DOUBLE_CLICK_DURATION) {
				g_last_click_time = currentTime;
				ex_log(LOG_DEBUG, "g_last_click_time =%d",
				       currentTime);
				return FINGERPRINT_RES_SUCCESS;
			} else {
				navi_event = NAVI_EVNT_DCLICK;
				g_last_click_time = 0;
			}
#else
			ex_log(LOG_DEBUG, "NAVI_EVNT_CLICK");

#endif
		} break;

		case NAVI_EVNT_DCLICK: {
			ex_log(LOG_DEBUG, "fcDblClick");
		} break;

		case NAVI_EVNT_LCLICK: {
			ex_log(LOG_DEBUG, "fcLongTouch");
		} break;

		case NAVI_EVENT_LEFT: {
			ex_log(LOG_DEBUG, "fcNaviLeft");
		} break;

		case NAVI_EVENT_RIGHT: {
			ex_log(LOG_DEBUG, "fcNaviRight");
		} break;

		case NAVI_EVENT_FASTLEFT: {
			ex_log(LOG_DEBUG, "fcNaviFastLeft");
		} break;

		case NAVI_EVENT_FASTRIGHT: {
			ex_log(LOG_DEBUG, "fcNaviFastRight");
		} break;

		case NAVI_EVENT_DOWN: {
			ex_log(LOG_DEBUG, "fcSwipeDown");
		} break;

		case NAVI_EVENT_UP: {
			ex_log(LOG_DEBUG, "fcSwipeUp");
		} break;

		default:
			break;
	}

	if (NULL != g_event_callback) {
		g_event_callback(EVENT_NAVIGATION, navi_event, 0, NULL, 0);
	}

	return FINGERPRINT_RES_SUCCESS;
}

#define abs(x) ((x) >= 0 ? (x) : -(x))
BOOL analyze_speed_send_swipe_event(int move_count, int accum_dx,
				    unsigned long begin_touch_time,
				    int speed_threshold)
{
	int speed_factor;
	int sendevent = NAVI_EVENT_UNKNOW;
	int avg_speed;
	if (0 == move_count) {
		return FALSE;
	}

	unsigned long now_time = GetTickCount();

	unsigned long duration = now_time - begin_touch_time;

	avg_speed = accum_dx / move_count;

	ex_log(LOG_DEBUG,
	       "move_count=%d accum_dx=%d,avg_speed=%d , speed_threshold=%d",
	       move_count, accum_dx, avg_speed, speed_threshold);

	if (abs(avg_speed) > speed_threshold) {
		if (avg_speed > 0) {
			speed_factor = avg_speed - speed_threshold;
			sendevent = duration > g_fast_right_threshold
					? NAVI_EVENT_RIGHT
					: NAVI_EVENT_FASTRIGHT;
		} else {
			speed_factor = avg_speed + speed_threshold;
			sendevent = duration > g_fast_left_threshold
					? NAVI_EVENT_LEFT
					: NAVI_EVENT_FASTLEFT;
		}
	} else {
		ex_log(LOG_DEBUG, "abs(avg_speed) < 3");
	}

	if (sendevent != NAVI_EVENT_UNKNOW) {
		send_navi_event_512(sendevent, speed_factor);
		return TRUE;
	}

	return FALSE;
}

void new_send_swipe_event(int *x, __unused int *y, int x_threshold,
			  __unused int y_threshold, int long_touch_duration,
			  unsigned long begin_touch_time, BOOL *is_swipe,
			  BOOL *is_long_touch)
{
	int navi_event = NAVI_EVENT_UNKNOW;
	static int total_touch_time = 0;

	if (*is_long_touch) {
		*x = 0;
		return;
	}

	if (!*is_swipe) {
		total_touch_time = GetTickCount() - begin_touch_time;
	} else {
		total_touch_time = 0;
	}

	if (total_touch_time <= long_touch_duration) {
		*x = 0;
		return;
	}

	if (abs(*x) >= x_threshold) {
		*is_swipe = TRUE;
		return;
	} else {
		if (total_touch_time > long_touch_duration) {
			*is_long_touch = TRUE;
			navi_event = NAVI_EVNT_LCLICK;
			send_navi_event_512(navi_event, 0);
			*x = 0;
			g_has_sent_event = FALSE;
			return;
		}
	}
}


int set_navi_parameter(int *navi_parameters)
{
	ex_log(LOG_DEBUG, "set_navi_parameter enter!");

	g_navi_swipe_x = navi_parameters[0];
	g_navi_swipe_y = navi_parameters[1];

	g_navi_x_h_threshold = navi_parameters[2];
	g_navi_x_l_threshold = navi_parameters[3];
	g_navi_y_h_threshold = navi_parameters[4];
	g_navi_y_l_threshold = navi_parameters[5];

	return 0;
}
