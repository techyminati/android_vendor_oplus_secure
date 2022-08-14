/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : ./vendor/qcom/proprietary/securemsm/rpmbeng_client/rpmbengclient.h
 ** Description : NULL
 ** Date        : 2016-02-06 09:37
 ** Author      : Lycan.Wang
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **     Lycan.Wang     2016/02/06         NULL
 *************************************************************/

#ifndef __RPMBENGCLIENT_H__
#define __RPMBENGCLIENT_H__
// get rpmb state, return 0 for enable, else for disabled or error
int32_t rpmbeng_get_state();

// enable rpmb, return 0 for success, else for failed
int32_t rpmbeng_enable_rpmb();

// get rpmb enable state for factory, return 0 for have enabled in factory
int32_t rpmbeng_get_enable_state();
#endif /*__RPMBENGCLIENT_H__*/
