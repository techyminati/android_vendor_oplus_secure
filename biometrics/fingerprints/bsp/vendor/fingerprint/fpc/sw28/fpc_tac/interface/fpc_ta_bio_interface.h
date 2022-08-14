/*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*/

#ifndef INCLUSION_GUARD_FPC_TA_BIO_INTERFACE
#define INCLUSION_GUARD_FPC_TA_BIO_INTERFACE

#include "fpc_ta_interface.h"
#include <stdbool.h>

#define FPC_TA_BIO_DB_RDONLY 0
#define FPC_TA_BIO_DB_WRONLY 1

// This is an upper limit in the API towards the TA
#define MAX_NR_TEMPLATES_API 25
#define MAX_NR_TEMPLATES 5

typedef struct {
    fpc_ta_cmd_header_t header;
    uint32_t answer;
} fpc_ta_bio_simple_command_t;

//ziqing modify for add the new API to adjust the sensibility
typedef struct {
    fpc_ta_bio_simple_command_t bio;
    bool need_judge_duplicate_finger;
} fpc_ta_enable_duplicate_fp_command_t;

typedef struct {
   /* Progress of the current enroll process in percent */
    uint32_t progress;
    /* Quality for the image*/
    uint32_t quality;
    /* Size of the enrolled template */
    uint32_t enrolled_template_size;
     /* Number of successful enroll attempts so far */
    uint32_t coverage;
    /* Used to indicate that touches are too similar */
    int8_t user_touches_too_immobile;
    /* Number of remaining touches */
    uint32_t remaining_touches;
} fpc_algo_enroll_statistics_t;

typedef struct {
    int32_t coverage;
    int32_t quality;
    int32_t covered_zones;
    uint32_t result;
    uint32_t score;
    uint32_t index;
} fpc_ta_bio_identify_statistics_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    fpc_ta_bio_identify_statistics_t statistics;
} fpc_ta_bio_identify_command_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    fpc_algo_enroll_statistics_t enrol_data;
} fpc_ta_bio_enroll_command_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    uint32_t ids[MAX_NR_TEMPLATES];
} fpc_ta_bio_get_ids_command_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    uint64_t id;
} fpc_ta_bio_get_db_command_t;

typedef struct {
    fpc_ta_bio_simple_command_t bio;
    uint32_t mode;
    uint32_t size;
} fpc_ta_bio_db_open_command_t;

typedef union {
    fpc_ta_cmd_header_t header;
    fpc_ta_bio_simple_command_t bio;
    fpc_ta_byte_array_msg_t store_db;
    fpc_ta_byte_array_msg_t load_db;
    fpc_ta_bio_db_open_command_t db_open;
    fpc_ta_cmd_header_t db_close;
    fpc_ta_byte_array_msg_t db_read;
    fpc_ta_byte_array_msg_t db_write;
    fpc_ta_byte_array_msg_t get_db_size;
    fpc_ta_bio_get_ids_command_t get_ids;
    fpc_ta_bio_identify_command_t identify;
    fpc_ta_bio_get_db_command_t db_id;
    fpc_ta_bio_enroll_command_t enroll;
	//ziqing add for enable duplicate fingerprint detect at enroll
    fpc_ta_enable_duplicate_fp_command_t set_duplicate_fp;
} fpc_ta_bio_command_t;


typedef enum {
  FPC_TA_BIO_INIT_CMD,
  FPC_TA_BIO_BEGIN_ENROL_CMD,
  FPC_TA_BIO_ENROL_CMD,
  FPC_TA_BIO_END_ENROL_CMD,
  FPC_TA_BIO_IDENTIFY_CMD,
  FPC_TA_BIO_UPDATE_TEMPLATE_CMD,
  FPC_TA_BIO_GET_DB_SIZE_CMD,
  FPC_TA_BIO_LOAD_EMPTY_DB_CMD,
  FPC_TA_BIO_GET_FINGER_IDS_CMD,
  FPC_TA_BIO_DELETE_TEMPLATE_CMD,
  FPC_TA_BIO_SET_ACTIVE_FINGERPRINT_SET_CMD,
  FPC_TA_BIO_GET_TEMPLATE_DB_ID_CMD,
  FPC_TA_BIO_LOAD_DB_CMD,
  FPC_TA_BIO_STORE_DB_CMD,
  FPC_TA_BIO_DB_OPEN_CMD,
  FPC_TA_BIO_DB_CLOSE_CMD,
  FPC_TA_BIO_DB_READ_CMD,
  FPC_TA_BIO_DB_WRITE_CMD,
  FPC_TA_BIO_GET_IDENTIFY_STATISTICS_CMD,
  //ziqing add for cancel enrol
  FPC_TA_BIO_CANCEL_ENROL_CMD,
  //ziqing add for enable duplicate finger detect
  FPC_TA_TEE_ENABLE_DUPLICATE_FP_DETECT_CMD,
} fpc_ta_bio_cmd_t;

#endif /* INCLUSION_GUARD_FPC_TA_BIO_INTERFACE */

