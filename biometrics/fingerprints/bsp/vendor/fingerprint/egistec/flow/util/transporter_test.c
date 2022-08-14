#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "transporter_test.h"
#include "common_definition.h"
#include "response_def.h"
#include "opt_file.h"
#include "plat_log.h"

#define DEFAULT_TEST_DATA_SIZE 60

static int transporter_test_generate_in_data(
    transporter_test_in_head_t *in_header, int type, int test_size,
    unsigned short **in_data_pointer, int *in_data_size,
    int *expect_out_data_size)
{
	int i;

	if (NULL == in_header || NULL == in_data_pointer ||
	    NULL == in_data_size || NULL == expect_out_data_size ||
	    *in_data_pointer == NULL) {
		ex_log(LOG_ERROR, "invalid parameter");
		return FINGERPRINT_RES_FAILED;
	}

	if (test_size < 1) {
		ex_log(LOG_ERROR, "test_size (%d) must be positive.", test_size);
		return FINGERPRINT_RES_FAILED;
	}

	in_header->type = type;
	in_header->step = 1;
	in_header->in_data_size = test_size;

	for (i = 0; i < test_size / (int)sizeof(uint16_t); i++) {
		(*in_data_pointer)[i] = i % 5;
	}

	if (type == INCREMENTAL_TEST) {
		// Only use one sizeof(uint16_t) as the starting number
		(*in_data_pointer)[0] = test_size;
		*in_data_size = sizeof(uint16_t) + sizeof(transporter_test_in_head_t);
	} else {
		*in_data_size = test_size + sizeof(transporter_test_in_head_t);
	}
	*expect_out_data_size =
	    test_size + sizeof(transporter_test_out_head_t);

	return FINGERPRINT_RES_SUCCESS;
}

static int transporter_test_update_out_data(unsigned char *in_data,
					    int in_data_size,
					    unsigned char *out_data,
					    int *out_data_size)
{
	return opt_receive_data(TYPE_TEST_TRANSPORTER, in_data, in_data_size,
				out_data, out_data_size);
}

int transporter_test_main(int type, int test_case)
{
	int i, in_checksum = 0, out_checksum = 0;
	int retval = FINGERPRINT_RES_FAILED;
	int test_size = test_case;
	if (test_case < 1) {
		test_size = DEFAULT_TEST_DATA_SIZE;
	}

	unsigned char *in_data = (unsigned char *)malloc(
	    test_size + sizeof(transporter_test_in_head_t));
	unsigned char *out_data = (unsigned char *)malloc(
	    test_size + sizeof(transporter_test_out_head_t));
	int in_data_size, expect_out_data_size, out_data_size;
	transporter_test_in_head_t *in_head =
	    (transporter_test_in_head_t *)in_data;
	;
	transporter_test_out_head_t *out_head =
	    (transporter_test_out_head_t *)out_data;
	unsigned short *in_test_data =
	    (unsigned short *)((unsigned char *)in_head +
			       sizeof(transporter_test_in_head_t));
	unsigned short *out_test_data =
	    (unsigned short *)((unsigned char *)out_head +
			       sizeof(transporter_test_out_head_t));

	if (in_data == NULL || out_data == NULL) {
		ex_log(LOG_ERROR, "alloc memory failed");
		goto exit;
	}

	ex_log(LOG_DEBUG,
	       "in_head = %p, in_test_data = %p, out_head = %p, out_test_data "
	       "= %p",
	       in_head, in_test_data, out_head, out_test_data);

	retval = transporter_test_generate_in_data(
	    in_head, type, test_size, &in_test_data, &in_data_size, &expect_out_data_size);
	if (retval != FINGERPRINT_RES_SUCCESS) {
		goto exit;
	}

	out_data_size = expect_out_data_size;
	retval = transporter_test_update_out_data(in_data, in_data_size,
						  out_data, &out_data_size);
	if (retval != FINGERPRINT_RES_SUCCESS) {
		goto exit;
	}

	if (type == INCREMENTAL_TEST) {
		in_checksum = in_test_data[0];
		for (i = 0; i < test_size / (int)sizeof(uint16_t); i++) {
			out_checksum += (in_test_data[0] + i * in_head->step);
		}
	} else {
		for (i = 0; i < test_size / (int)sizeof(uint16_t); i++) {
			in_checksum += in_test_data[i];
		}
	}

	ex_log(LOG_DEBUG,
	       "CORE : type : %d, in_data_checksum : %d, out_data_checksum : "
	       "%d, in_data_size : %d, out_data_size : %d",
	       out_head->type, out_head->in_data_checksum,
	       out_head->out_data_checksum, in_head->in_data_size,
	       out_head->out_data_size);

	ex_log(LOG_DEBUG, "FLOW : in_checksum = %d, out_checksum = %d",
	       in_checksum, out_checksum);

	retval = FINGERPRINT_RES_FAILED;
	if (type == DUPLICATE_TEST) {
		if (out_head->type == in_head->type &&
		    out_head->in_data_checksum == out_head->out_data_checksum &&
		    in_checksum == out_head->out_data_checksum &&
		    expect_out_data_size == out_data_size) {
			retval = FINGERPRINT_RES_SUCCESS;
		}
	} else if (type == INCREMENTAL_TEST) {
		if (out_head->type == in_head->type &&
		    out_head->in_data_checksum == in_checksum &&
		    out_head->out_data_checksum == out_checksum &&
		    expect_out_data_size == out_data_size) {
			retval = FINGERPRINT_RES_SUCCESS;
		}
	} else {
		retval = FINGERPRINT_RES_FAILED;
		ex_log(LOG_ERROR, "not supported test type");
	}

exit:

	if (in_data) {
		free(in_data);
		in_data = NULL;
	}

	if (out_data) {
		free(out_data);
		out_data = NULL;
	}

	return retval;
}
