#ifndef __NAVI_OPERATOR_H__
#define __NAVI_OPERATOR_H__

#include "struct_def.h"
#include "type_def.h"

typedef enum nav_state {
	NVSTATE_SUSPEND = 0,
	NVSTATE_INIT,
	NVSTATE_SET_MODE,
	NVSTATE_WAIT_INT,
	NVSTATE_NAV,
	NVSTATE_FINGER_ON
} nav_state_t;

#define EMU_STATE_ON 0x10
#define EMU_STATE_ON_EDGE 0x12
#define EMU_STATE_OFF 0x20

int set_event(const char* file_name, unsigned char* in_buffer,
	      int in_buffer_size);

int set_navi_parameter(int* navi_parameters);
int send_navi_event_to_driver(int event);
BOOL navi_is_enable();

#endif
