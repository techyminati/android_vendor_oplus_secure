#define LOG_TAG "[ANC_CA_TEST]"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "anc_log.h"
#include "test.h"

void usage() {
    printf("./anc_ca_test\n");
    printf("      ca function full test\n");
    printf("./anc_ca_test <case number> <Pressure loop>\n");
    printf("      case number:\n");
    printf("      capture stress test : 1\n");
    printf("      spi stress test : 2\n");
    printf("      Pressure loop: 1-1000000\n");
}


int main(int argc, char *argv[])
{
    ANC_CA_TEST_CASE test_case = ANC_CA_FULL_FUNCTION_TEST;
    int test_count = 1;
    int ret_val = 0;

    ANC_LOGD("start anc tac test\n");

    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (argc > 2) {
            test_count = atoi(argv[2]);
        }
        ANC_LOGD("test_case = %d, test_count = %d\n", test_case, test_count);
        printf("test_case = %d, test_count = %d\n", test_case, test_count);

        if ((test_case >= ANC_CA_FULL_FUNCTION_TEST) && (test_case < ANC_CA_TEST_CASE_MAX)) {
            ret_val = DoTest(test_case, test_count);
        } else {
            printf("test_case %d not found!\n", test_case);
            usage();
        }
    } else {
        ret_val = DoTest(test_case, test_count);
    }

    return ret_val;
}
