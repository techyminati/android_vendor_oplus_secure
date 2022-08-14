#include "packager.h"
#include "egis_definition.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "plat_time.h"
#include "response_def.h"
#include "stdlib.h"
#include "thread_manager.h"
#include "transporter.h"

typedef struct transfer_package {
    unsigned int process;
    unsigned int command;
    unsigned int uid;
    unsigned int fid;
    unsigned int in_data_len;
    unsigned char in_data[0];
} transfer_package_t;

#define LOG_TAG "RBS"
#define SIZEOF_INT sizeof(unsigned int)
#define BASE_TRANSFER_SIZE 1024

#define SUPPORT_EGIS_NEED_TO_RESET
#define EGIS_NEED_TO_RESET_RETRY_COUNT 15

#ifdef SUPPORT_EGIS_NEED_TO_RESET
#include "fps_normal.h"
extern int g_hdev;
#endif
/*
**	@transfer_data
**	Transmission information channel
**	@params
**	[ pid cid uid fid in_data_len ] transfer_header_t
**	[ in_data ] the pointer of the msg string
**	[ out_data_len ] the pointer of the @out_data length
**	[ out_data ] the pointer of the out info string
**	@other_explain
**	[ BASE_TRANSFER_SIZE ] align up the message length with a certain size
*/
int transfer_data(unsigned int pid, unsigned int cid, unsigned int uid, unsigned int fid,
                  unsigned int in_data_len, unsigned char* in_data, unsigned int* out_data_len,
                  unsigned char* out_data) {
    ex_log(LOG_INFO, "-- **packager p1 %d p2 %d p3 %d p4 %d", pid, cid, uid, fid);
    int retval;
    unsigned int msg_data_len, rsp_data_len;
    transfer_package_t* msg_data = NULL;
    int retry_reset_count = 0;

    if (0 != trylock_operation(RBS_MUTEX_TRANSFER))  // To Do : use lock instead of trylock
        return FINGERPRINT_RES_NOT_IDLE;

    msg_data_len = SIZEOF_INT * 5 + in_data_len;

    if (msg_data_len % BASE_TRANSFER_SIZE != 0) {
        msg_data_len = (msg_data_len / BASE_TRANSFER_SIZE + 1) * BASE_TRANSFER_SIZE;
    }

    msg_data = malloc(msg_data_len);
    if (NULL == msg_data) {
        retval = FINGERPRINT_RES_ALLOC_FAILED;
        goto EXIT;
    }

    for (;;) {
        /*can not remove this 'mem_set' calling because a lots of input strings do not have a '\0'
         * ending such as most input paths*/
        /*or using mem_set(msg_data->in_data, 0, in_data_len + 1) as an alternative*/
        mem_set(msg_data, 0, msg_data_len);
        msg_data->process = pid;
        msg_data->command = cid;
        msg_data->uid = uid;
        msg_data->fid = fid;
        msg_data->in_data_len = 0;

        ex_log(LOG_DEBUG, "-- packager, process: %d, command: %d, uid: %d, fid:%d",
               msg_data->process, msg_data->command, msg_data->uid, msg_data->fid);

        if (in_data_len > 0 && NULL != in_data) {
            msg_data->in_data_len = in_data_len;
            mem_move(msg_data->in_data, in_data, in_data_len);
        }
        if (NULL != out_data_len) {
            rsp_data_len = *out_data_len;
        }

        retval = transporter((unsigned char*)msg_data, msg_data_len, out_data, out_data_len);

#ifdef SUPPORT_EGIS_NEED_TO_RESET
        if (retval != EGIS_ESD_NEED_RESET)
            break;
        else if (retry_reset_count > EGIS_NEED_TO_RESET_RETRY_COUNT) {
            ex_log(LOG_ERROR, "-- packager, transporter Failed RESET (already retry %d)",
                   retry_reset_count);
            break;
        }

        retry_reset_count++;
        ex_log(LOG_DEBUG, "-- packager, transporter (%d)", retry_reset_count);

#if defined(POWER_CONTRL) && defined(ESD_WORKAROUND_POWER_OFF)
        fp_device_power_control(g_hdev, FALSE);
        plat_sleep_time(1000);  // ms
        fp_device_power_control(g_hdev, TRUE);
        plat_sleep_time(50);
#endif
        retval = fp_device_reset(g_hdev);
        plat_wait_time(20);
        ex_log(LOG_DEBUG, "-- packager, transporter done RESET. retval=%d", retval);
        if (NULL != out_data_len) *out_data_len = rsp_data_len;
#else
        break;
#endif
    }

    if (NULL != msg_data) {
        free(msg_data);
        msg_data = NULL;
    }

EXIT:
    unlock_operation(RBS_MUTEX_TRANSFER);

    return retval;
}
