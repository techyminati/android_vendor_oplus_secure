#include "plat_time.h"
#include "plat_log.h"
#include "fps_normal.h"
#include "response_def.h"
#include "device_int.h"

#define POLL_TIME 100

extern int g_hdev;

BOOL wait_trigger(int try_count, int timeout)
{
	BOOL bret = FALSE;
	BOOL bWait = TRUE;
	int icount = try_count;
	int retval = FINGERPRINT_RES_SUCCESS;
#ifdef ENABLE_POLL
	plat_sleep_time(POLL_TIME);
	ex_log(LOG_DEBUG, "poll trigger");
	return TRUE;
#endif
	retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
	retval = fp_device_interrupt_enable(g_hdev, FLAG_INT_INIT);

	if (retval != FINGERPRINT_RES_SUCCESS) return FALSE;

	while (bWait) {
		if (check_cancelable()) goto exit;
		if(check_need_pause()) goto exit;
		retval = fp_device_interrupt_wait(g_hdev, timeout);
		if (check_cancelable()) goto exit;
		if(check_need_pause()) goto exit;

		if (try_count != 0 && --icount <= 0) bWait = FALSE;

		if (FINGERPRINT_RES_SUCCESS == retval) {
			ex_log(LOG_DEBUG, "interrupt trigger");
			bret = TRUE;
			break;
		}
	}
exit:
#ifndef _WINDOWS
	fp_device_interrupt_enable(g_hdev, FLAG_INT_CLOSE);
#endif
	if(check_need_pause()) return FALSE;
	return bret;
}
