/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : ./vendor/qcom/proprietary/securemsm/rpmbeng_client/rpmbengclient.h
 ** Description : NULL
 ** Date        : 2018-12-04 14:39
 ** Author      : LONG.liu
 **
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **     Long.Liu        2018/12/04      create file
 *************************************************************/

#ifndef __RPMBENGCLIENT_H__
#define __RPMBENGCLIENT_H__

#define OPLUS_RESERVE1_EMMC_ENABLE_RPMB_FLAG_OFFSET            (9068 * 512)
#define OPLUS_RESERVE1_UFS_ENABLE_RPMB_FLAG_OFFSET            (1176 * 4096)

#define UFS_DEVICE_BLOCK_PATH     "/proc/devinfo/ufs"
#define RPMB_ENABLE_MAGIC_STRING "0x9621"
#define RPMB_KEY_PROVISIONED_STRING "0x594553"
#define RPMB_KEY_NOT_PROVISIONED_STRING "0x4e4f"
#define OPLUSRESERVE1_RPMB_FLAG_LENGTH 20

#define OPLUS_RESERVE1_PATH   "/dev/block/by-name/oplusreserve1"
// get rpmb state, return 0 for enable, else for disabled or error
int32_t rpmbeng_get_state();

// enable rpmb, return 0 for success, else for failed
int32_t rpmbeng_enable_rpmb();

// get rpmb enable state for factory, return 0 for have enabled in factory
int32_t rpmbeng_get_enable_state();

// get rpmb enable state for oplusreseve1
bool set_enable_rpmb_flag();

typedef struct
{
    unsigned char rpmb_enable[OPLUSRESERVE1_RPMB_FLAG_LENGTH];
    unsigned char rpmb_keyprovision[OPLUSRESERVE1_RPMB_FLAG_LENGTH];
} Rpmb_flag_Config;

enum status_rpmb_key_provisioned
{
    STATUS_RPMB_KEY_NOT_PROVISIONED = 0,
    STATUS_RPMB_KEY_PROVISIONED = 1,
    STATUS_RPMB_KEY_PROVISIONED_ERROR = 2
};

enum status_rpmb_key_provisioned get_rpmb_key_provisioned_flag(void);
#endif /*__RPMBENGCLIENT_H__*/
