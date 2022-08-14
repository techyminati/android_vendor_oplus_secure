#ifndef __DEVICE_INT_H__
#define __DEVICE_INT_H__

#include "type_def.h"

#define TIMEOUT_WAIT_FOREVER -1

void host_touch_set_enable(BOOL enable);

void host_touch_set_finger_reset();

void host_touch_set_finger_on(BOOL on);

void host_touch_set_low_coverage(BOOL flag);

void host_touch_set_finger_off(BOOL off);

BOOL host_touch_is_enable();

BOOL host_touch_is_finger_on();
BOOL host_touch_is_finger_on_last();

BOOL host_touch_is_low_coverage();

BOOL host_touch_is_finger_off();
BOOL host_touch_is_finger_off_last();

BOOL wait_trigger(int retry_count, int timeout_for_one_try, int total_timeout);

// implementation is on captain.c
BOOL check_cancelable();

typedef BOOL (*eventCheckerFunc)();

typedef struct _event_check_fd {
    eventCheckerFunc checkerFunc;
    BOOL revent;
} event_check_fd;

typedef enum {
    REVENT_TIMEOUT = 0,
    REVENT_EVENT_POLLIN,
} poll_revents;

poll_revents event_poll_wait(event_check_fd* check_group, uint32_t fd_count, int timeout);

typedef void (*func_notify_t)(int event_id, int first_param, int second_param, unsigned char* data,
                              int data_size);

typedef enum _oplus_wait_trigger_case_ {
    TRIGGER_WAIT_TOUCH_DOWN,
    TRIGGER_WAIT_TOUCH_UP,
    TRIGGER_WAIT_UI_READY,
} oplus_trigger_case_t;

extern func_notify_t g_fn_callback;

BOOL host_touch_is_using_oplus_flow();
void host_touch_set_ui_ready(BOOL ready);
void host_touch_set_callback(func_notify_t callback);
void host_touch_set_hbm_system(BOOL on_off);
void host_touch_set_hbm_evalute_always(void);
void host_touch_set_hbm(BOOL on_off);
int init_oplus_event_listener(uint8_t fd);
void get_image_check_start();
int get_image_stable_expo(unsigned long long start_wait_expo);
BOOL get_image_is_too_fast(uint32_t duration);
BOOL is_verify_finger_on_stable(BOOL isretry, int stable_time);
#endif
