/************************************************************************************
 ** Copyright (C), 2008-2021, OPLUS Mobile Comm Corp., Ltd
 **
 ** File: - fingerprint/AutoSmoking/AutoSmoking/cpp
 **
 ** Description:
 **      AutoSmoking test for fingerprint
 **
 ** Version: 1.0
 ** Date created: 18:03:11,17/08/2021
 ** Author: zoulian@BSP.Fingerprint.Basic
 ** TAG: BSP.Fingerprint.Basic
 ** ---------------------------Revision History--------------------------------
 **  <author>      <data>            <desc>
 **  zoulian       2021/08/17        create the file
 ************************************************************************************/
#include <hidl/HidlSupport.h>
#include <hidl/LegacySupport.h>
#include <hidl/Status.h>
#include <stdio.h>
#include <utils/misc.h>
#include <cstdlib>
#include <unistd.h>
#include "VtsClient.h"
#include <iostream>


using namespace std;

android::sp<IBiometricsFingerprint> gp_service = nullptr;
sp<BiometricsFingerprintClientCallback> gp_client_call_back = nullptr;
const char *gp_exe_case_name = nullptr;
int g_exe_case_time = 0;

void usage() {
    printf("./test_oplus_fingerprint <Pressure loop 1-1000000> <Case number> <Parameters...>\n");
    printf("The parameters in [] are optional parameters,\n"
        "When the parameter in the [] is not empty,\n"
        "the path of transferring the image reads the picture from the file.\n");
}

static void check_parameter_size(int argc, int need_parameter_size) {
    if (need_parameter_size > (argc - 1)) {
        usage();
        exit(-1);
    }
}

void sigint_handler(int sig) {
    switch (sig) {
        case SIGINT:
        case SIGTERM:
        printf("SIGINT or SIGTERM\n");
        if (gp_client_call_back != nullptr) {
            if (gp_service != nullptr) {
                printf("fingerprint cancel!\n");
                gp_service->cancel();
                usleep(200 * 1000);
            }
        }
    break;
    case SIGALRM:
        printf("Error:exe %s timeout %ds!\n", gp_exe_case_name, g_exe_case_time);
        break;
    default:
        printf("Unkonw signal!\n");
        break;
    }
    exit(-1);
}

void enroll_and_authicate(android::sp<IBiometricsFingerprint> service, sp<BiometricsFingerprintClientCallback> client_call_back, int cmd) {
    uint32_t gid = 0;
    printf("enroll_and_authicate entry");
    hidl_vec<int8_t> inBuf;
    service->sendFingerprintCmd(cmd, inBuf);
    usleep(10*1000);  // 1ms
    printf("enroll_and_authicate end\n");
}

void vtsTestCase(android::sp<IBiometricsFingerprint> service, sp<BiometricsFingerprintClientCallback> client_call_back, int cmd) {
    uint32_t gid = 0;
    printf("enroll_and_authicate entry");
    hidl_vec<int8_t> inBuf;
    service->sendFingerprintCmd(cmd, inBuf);
    usleep(10*1000);  // 1ms
    printf("enroll_and_authicate end\n");
}

int main(int argc, char **argv) {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    signal(SIGALRM, sigint_handler);
    int count = 0;
    int test_case = 0;
    printf("IBiometricsFingerprint::getService\n");
    gp_service = IBiometricsFingerprint::getService();
    if (gp_service == nullptr) {
        printf("fail to get gp_service\n");
        return -1;
    }
    gp_client_call_back = new BiometricsFingerprintClientCallback();
    printf("gp_service->setNotify\n");
    uint64_t device_id = gp_service->setNotify(gp_client_call_back);
    printf("device id : %lu %d\n", device_id, test_case);
    enroll_and_authicate(gp_service, gp_client_call_back, 5000);
    while (1) {
        cout << "please input count and test case, ex 1 5002" << endl;
        scanf("%d %d", &count, &test_case);
        for (int i = 0; i < count; i++) {
            if (test_case == 100) {
                enroll_and_authicate(gp_service, gp_client_call_back, 5001);//start test
                for (int i = 5002; i <= 5015; i++) {
                    enroll_and_authicate(gp_service, gp_client_call_back, i);
                }
                enroll_and_authicate(gp_service, gp_client_call_back, 5016);//end test
            } else {
                enroll_and_authicate(gp_service, gp_client_call_back, 5001);
                enroll_and_authicate(gp_service, gp_client_call_back, test_case);
                enroll_and_authicate(gp_service, gp_client_call_back, 5016);
            }
        }
    }
    return 0;
}


