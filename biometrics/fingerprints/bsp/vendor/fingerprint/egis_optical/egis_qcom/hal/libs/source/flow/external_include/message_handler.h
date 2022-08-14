#ifndef __MESSAGE_HANDLER_H__
#define __MESSAGE_HANDLER_H__

#define OPERATION_TYPE 0
#define NAVIGATION_TYPE 4

int message_handler(unsigned char* buffer, int buffersize,
		    unsigned char* outdata, int* outdata_len);

#endif
