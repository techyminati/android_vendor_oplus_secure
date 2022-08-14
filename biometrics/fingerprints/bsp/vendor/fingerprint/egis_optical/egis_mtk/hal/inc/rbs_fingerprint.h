//
//    Copyright 2017 Egis Technology Inc.
//
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Discretix in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//
#ifndef ETS_FINGERPRINT_HEADER
#define ETS_FINGERPRINT_HEADER
#include "egis_rbs_api.h"
#include "rbs_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fingerprint errors are meant to tell the framework to terminate the current
 * operation and ask
 * for the user to correct the situation. These will almost always result in
 * messaging and user
 * interaction to correct the problem.
 *
 * For example, FINGERPRINT_ERROR_CANCELED should follow any acquisition message
 * that results in
 * a situation where the current operation can't continue without user
 * interaction. For example,
 * if the sensor is dirty during enrollment and no further enrollment progress
 * can be made,
 * send FINGERPRINT_ACQUIRED_IMAGER_DIRTY followed by
 * FINGERPRINT_ERROR_CANCELED.
 */
typedef enum fingerprint_error {
	FINGERPRINT_ERROR_HW_UNAVAILABLE =
	    1, /* The hardware has an error that can't be resolved. */
	FINGERPRINT_ERROR_UNABLE_TO_PROCESS =
	    2, /* Bad data; operation can't continue */
	FINGERPRINT_ERROR_TIMEOUT =
	    3, /* The operation has timed out waiting for user input. */
	FINGERPRINT_ERROR_NO_SPACE =
	    4, /* No space available to store a template */
	FINGERPRINT_ERROR_CANCELED =
	    5, /* The current operation can't proceed. See above. */
	FINGERPRINT_ERROR_UNABLE_TO_REMOVE =
	    6, /* fingerprint with given id can't be removed */
	FINGERPRINT_ERROR_VENDOR_BASE =
	    1000 /* vendor-specific error messages start here */
} fingerprint_error_t;

/*
 * Fingerprint acquisition info is meant as feedback for the current operation.
 * Anything but
 * FINGERPRINT_ACQUIRED_GOOD will be shown to the user as feedback on how to
 * take action on the
 * current operation. For example, FINGERPRINT_ACQUIRED_IMAGER_DIRTY can be used
 * to tell the user
 * to clean the sensor.  If this will cause the current operation to fail, an
 * additional
 * FINGERPRINT_ERROR_CANCELED can be sent to stop the operation in progress
 * (e.g. enrollment).
 * In general, these messages will result in a "Try again" message.
 */
typedef enum fingerprint_acquired_info {
	FINGERPRINT_ACQUIRED_GOOD = 0,
	FINGERPRINT_ACQUIRED_PARTIAL =
	    1, /* sensor needs more data, i.e. longer swipe. */
	FINGERPRINT_ACQUIRED_INSUFFICIENT =
	    2,				       /* image doesn't contain enough detail for recognition*/
	FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3, /* sensor needs to be cleaned */
	FINGERPRINT_ACQUIRED_TOO_SLOW =
	    4, /* mostly swipe-type sensors; not enough data collected */
	FINGERPRINT_ACQUIRED_TOO_FAST =
	    5, /* for swipe and area sensors; tell user to slow down*/
	ACQUIRED_VENDOR_BASE = 1000,
	ACQUIRED_TOO_SIMILAR = 1001,
	/*for the same fingerprint as enrolled*/
	ACQUIRED_ALREADY_ENROLLED = 1002,
	FINGERPRINT_ACQUIRED_VENDOR_BASE =
	    1100, /* vendor-specific acquisition messages start here */
	FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT =
	    FINGERPRINT_ACQUIRED_VENDOR_BASE + 1,
	FINGERPRINT_ACQUIRED_FINGER_DOWN = FINGERPRINT_ACQUIRED_VENDOR_BASE + 2,
	FINGERPRINT_ACQUIRED_FINGER_UP = FINGERPRINT_ACQUIRED_VENDOR_BASE + 3,
	FINGERPRINT_ACQUIRED_INPUT_TOO_LONG =
	    FINGERPRINT_ACQUIRED_VENDOR_BASE + 4,
	FINGERPRINT_ACQUIRED_DUPLICATE_FINGER =
	    FINGERPRINT_ACQUIRED_VENDOR_BASE + 5,
	FINGERPRINT_ACQUIRED_DUPLICATE_AREA =
	    FINGERPRINT_ACQUIRED_VENDOR_BASE + 6,
	FINGERPRINT_ACQUIRED_LOW_COVER = FINGERPRINT_ACQUIRED_VENDOR_BASE + 7,
	FINGERPRINT_ACQUIRED_BAD_IMAGE = FINGERPRINT_ACQUIRED_VENDOR_BASE + 8
} fingerprint_acquired_info_t;


#ifdef __cplusplus
}
#endif

#endif  // ETS_FINGERPRINT_HEADER
