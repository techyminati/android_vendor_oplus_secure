#ifndef __PACKAGER_H__
#define __PACKAGER_H__

#include <string.h>
#include <stdio.h>
#include "type_def.h"

#define OPERATION_TYPE 0
#define NAVIGATION_TYPE 4

typedef enum navi_cmd {
	NAVI_CMD_NAVIGATION = 0,
	NAVI_CMD_SET_WORK_MODE,
	NAVI_CMD_NAVIGATION_INIT,
	NAVI_CMD_CHECK_FINGER,
	NAVI_CMD_GET_FINGER_STATE_FROM_ZONE2
} navi_cmd_t;

int transfer_data(unsigned int pid, unsigned int cid, unsigned int uid,
		  unsigned int fid, unsigned int in_data_len,
		  unsigned char *in_data, unsigned int *out_data_len,
		  unsigned char *out_data);
#endif
