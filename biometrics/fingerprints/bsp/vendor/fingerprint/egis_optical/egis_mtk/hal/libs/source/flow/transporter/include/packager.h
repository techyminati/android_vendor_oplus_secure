#ifndef __PACKAGER_H__
#define __PACKAGER_H__

#include <stdio.h>
#include <string.h>
#include "type_def.h"

#define OPERATION_TYPE 0

int transfer_data(unsigned int pid, unsigned int cid, unsigned int uid, unsigned int fid,
                  unsigned int in_data_len, unsigned char* in_data, unsigned int* out_data_len,
                  unsigned char* out_data);
#endif
