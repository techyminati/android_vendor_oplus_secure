//
//    Copyright 2017 Egis Technology Inc.
//
//    This software is protected by copyright, international
//    treaties and various patents. Any copy, reproduction or otherwise use of
//    this software must be authorized by Egis in a license agreement and
//    include this Copyright Notice and any other notices specified
//    in the license agreement. Any redistribution in binary form must be
//    authorized in the license agreement and include this Copyright Notice
//    and any other notices specified in the license agreement and/or in
//    materials provided with the binary distribution.
//

#include <stdint.h>
#include <vector>
#include <cutils/log.h>
#include "rbs_hidl.h"
#include "egis_fingerprint.h"
#include "plat_log.h"

#define LOG_TAG "FingerprintHal"

sp<IBiometricsFingerprintClientCallback> mCallback;

void trans_callback(int event_id, int value1, int value2, uint8_t* buffer, int buffer_size)
{
	egislog_d("buffer_size = %d", buffer_size);
	// const android::hardware::hidl_vec<uint8_t> hv1 = std::vector<uint8_t>(buffer, buffer + buffer_size);
	// mCallback->ipc_callback(event_id, value1, value2, hv1, buffer_size);
}

Return<void> ets_extra_hidl::extra_api(int32_t pid, const hidl_vec<uint8_t>& in_buffer, extra_api_cb _hidl_cb)
{
	int ret = -1;
	egislog_d("ets_extra_hidl.extra_api pid = %d", pid);
	std::vector<uint8_t> out_buffer(256, 0);
	egislog_d("in_buffer.size() = %d", in_buffer.size());
	ret = do_extra_api_in(pid, static_cast<const uint8_t*>(in_buffer.data()), in_buffer.size(), NULL, NULL);
	egislog_d("extra_api ret = %d", ret);
	out_buffer[0] = ret >> 24 & 0xFF;
	out_buffer[1] = ret >> 16 & 0xFF;
	out_buffer[2] = ret >> 8 & 0xFF;
	out_buffer[3] = ret & 0xFF;
	out_buffer.resize(4);

	if (_hidl_cb != NULL) {
		_hidl_cb(out_buffer);
	}
	return Void();
}

Return<int32_t> ets_extra_hidl::set_on_callback_proc(
    const ::android::sp<IBiometricsFingerprintClientCallback>& clientCallback)
{
	mCallback = clientCallback;
	egislog_d("#set_on_callback_proc");
	return extra_set_on_callback_proc(trans_callback);
}
