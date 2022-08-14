/*************************************************************
 ** Copyright (C), 2008-2012, OPLUS Mobile Comm Corp., Ltd
 ** OPLUS_FEATURE_SECURITY_COMMON
 ** File        : ./vendor/qcom/proprietary/securemsm/rpmbeng_client/rpmbengclient_test.c
 ** Description : NULL
 ** Date        : 2018-12-04 14:39
 ** Author      : LONG.liu
 **
 ** ------------------ Revision History: ---------------------
 **      <author>        <date>          <desc>
 **     Long.Liu        2018/12/04      create file
 *************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpmbengclient.h"

extern int32_t rpmbeng_get_state();
extern int32_t rpmbeng_enable_rpmb();
extern int32_t rpmbeng_get_enable_state();
int main(int argc, char *argv[]) 
{
	int res = -1;
	if (argc == 2)
	{
		if (!strcmp(argv[1], "--get_rpmb_state")) 
		{
                        if (!rpmbeng_get_state()) {
                                printf("rpmb enabled\n");
                                return 0;
                        } else {
                                printf("rpmb disabled\n");
                        }
                }

        if (!strcmp(argv[1], "--enable_rpmb"))
		{
			res = rpmbeng_enable_rpmb();
			if (0 == res)
			{
				printf("rpmb enable success\n");
				return 0;
			}
			else
			{
				printf("rpmb enable failed\n");
				return -1;
			}
		}

		/*if (!strcmp(argv[1], "--get_rpmb_enable_state")) 
		{
			if (!rpmbeng_get_enable_state()) 
			{
				printf("rpmb enable state true\n");
                return 0;
            }
			else 
				printf("rpmb enable state false\n");
		}*/
	} 
	else 
	{
		printf("rpmbengclient_test [--get_rpmb_state|--enable_rpmb|--get_rpmb_enable_state]\n");
		return -1;
	}
}
