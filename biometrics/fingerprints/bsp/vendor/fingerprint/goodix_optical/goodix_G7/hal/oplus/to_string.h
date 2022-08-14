/************************************************************************************
 ** File: - to_string.h
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2021-2025, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      constant to string
 **
 ** Version: 1.0
 ** Date created: 11:00,11/01/2021
 ** Author: Zhi.Wang@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Zhi.Wang
 ************************************************************************************/
#ifndef TO_STRING_H
#define TO_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

const char* module_to_str(int module_id);
const char* cmd_to_str(int module_id, int cmd_id);
const char* cali_op_to_str(int cali_op_id);
const char* netlink_to_str(int event_id);
const char* event_to_str(int event_id);
const char* msg_to_str(int msg_id);

#ifdef __cplusplus
}
#endif

#endif  // TO_STRING_H
