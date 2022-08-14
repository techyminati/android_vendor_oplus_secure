#ifndef TEST_H
#define TEST_H

typedef enum {
    ANC_CA_FULL_FUNCTION_TEST = 0,
    ANC_CA_CAPTURE_STRESS_TEST,
    ANC_CA_SPI_STRESS_TEST,
    ANC_CA_TEST_CASE_MAX
}ANC_CA_TEST_CASE;

int DoTest(ANC_CA_TEST_CASE test_case, int test_count);

#endif