#ifndef __NAVI_DEF_H__
#define __NAVI_DEF_H__

#include "type_def.h"

enum navi_event {
	NAVI_EVENT_CANCEL,     // 0
	NAVI_EVENT_ON,	 // 1
	NAVI_EVENT_OFF,	// 2
	NAVI_EVENT_SWIPE,      // 3
	NAVI_EVENT_UP,	 // 4
	NAVI_EVENT_DOWN,       // 5
	NAVI_EVENT_RIGHT,      // 6
	NAVI_EVENT_LEFT,       // 7
	NAVI_EVNT_CLICK,       // 8
	NAVI_EVNT_LCLICK,      // 9
	NAVI_EVNT_DCLICK,      // 10
	NAVI_EVENT_FASTRIGHT,  // 11
	NAVI_EVENT_FASTLEFT,   // 12
	NAVI_EVENT_UNKNOW = 0xFF
};

typedef enum {
	NAVI_OP_START = 0,
	NAVI_OP_GETPOS,
	NAVI_OP_GETTOUCH,
	NAVI_OP_DONE,
} navi_op_t;

enum navi_finger_status {
	DETECT_FINGER_ON,  // FINGER_OFF,
	SEND_FINGER_ON,    // FINGER_ON
	DETECT_SWIPE,
	SEND_FINGER_OFF,
	ON_EDGE
};

typedef struct navigation_info {
	unsigned char type;
} navigation_info_t;

typedef struct _navi_movement_t {
	int x;
	int y;
} navi_movement_t;

typedef struct _navi_para_t {
	navi_op_t op;
	navi_movement_t mov;
	BOOL touch;
	int mov_threshold_x;
	int mov_threshold_y;
	int time_out;
	int swipe_x;
	int swipe_y;
} navi_ctx_t;

#define FRAME_COUNTS 5

#define NAVI_TZ_TIMEOUT (FRAME_COUNTS - 1)

#define NAVI_X_SPEED_THRESHOLD 3

#define NAVI_Y_BREAK_THRESHOLD \
	8  // if (dy>NAVI_Y_BREAK_THRESHOLD), DO NOT send swipe event.
#define NAVI_XY_DIFF 2  // if (dx-dy < NAVI_XY_DIFF), DO NOT send swipe event.

#define NAV_TYPE_SWIPE ((unsigned char)0x01)
#define NAV_TYPE_CLICK ((unsigned char)0x02)
#define NAV_TYPE_LONG_CLICK ((unsigned char)0x04)
#define NAV_TYPE_DOUBLE_CLICK ((unsigned char)0x08)

#define NAV_ORIENTATION_VERTICAL ((unsigned char)0x01)
#define NAV_ORIENTATION_HORIZONTAL ((unsigned char)0x02)

#endif
