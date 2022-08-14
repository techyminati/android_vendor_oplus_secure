/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : ./vendor/qcom/proprietary/securemsm/rpmbeng_client/rpmbengclient_test.c
 ** Description : NULL
 ** Date        : 2016-02-04 15:12
 ** Author      : Lycan.Wang
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **     Lycan.Wang     2016/02/04         NULL
 *************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpmbengclient.h"

extern int32_t rpmbeng_get_state();
extern int32_t rpmbeng_enable_rpmb();
extern int32_t rpmbeng_get_enable_state();
int main(int argc, char *argv[]) {
        if (argc == 2) {
                if (!strcmp(argv[1], "--get_rpmb_state")) {
                        if (!rpmbeng_get_state()) {
                                printf("rpmb enabled\n");
                                return 0;
                        } else {
                                printf("rpmb disabled\n");
                        }
                }

                if (!strcmp(argv[1], "--enable_rpmb")) {
                        if (!rpmbeng_enable_rpmb()) {
                                printf("rpmb enable success\n");
                                return 0;
                        } else {
                                printf("rpmb enable failed\n");
                        }
                }

                if (!strcmp(argv[1], "--get_rpmb_enable_state")) {
                        if (!rpmbeng_get_enable_state()) {
                                printf("rpmb enable state true\n");
                                return 0;
                        } else {
                                printf("rpmb enable state false\n");
                        }
                }
        } else {
                printf("rpmbengclient_test [--get_rpmb_state|--enable_rpmb|--get_rpmb_enable_state]\n");
        }
        return -1;
}
