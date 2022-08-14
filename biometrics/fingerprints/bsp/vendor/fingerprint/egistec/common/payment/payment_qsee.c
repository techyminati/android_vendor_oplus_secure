#include <string.h>
#include "biometric_result.h"
#include "payment.h"
#include "plat_file.h"
#include "qsee_fs.h"
#include "qsee_log.h"

#ifndef QSEE_LOG
#define QSEE_LOG qsee_log
#endif

#define MAX_FINGERS_PER_USER 5

void payment_update_last_identify_result(struct LastIdentifyResult* identify_result)
{
    QSEE_LOG(QSEE_LOG_MSG_HIGH, "payment_set_identified_result");

    BIO_ERROR_CODE result;
    bio_result bio_res;

    bio_res.method = BIO_FINGERPRINT_MATCHING;
    bio_res.result = false;
    bio_res.user_id = BIO_NA;
    bio_res.user_entity_id = 0;

    if (identify_result != NULL && identify_result->matched_fingerprint_id > 0) {
        bio_res.result = true;
        bio_res.user_entity_id = identify_result->matched_fingerprint_id;
        bio_res.transaction_id = identify_result->operation_id;
        QSEE_LOG(QSEE_LOG_MSG_HIGH, "%s, matched fid = %d, operation_id = %d", __func__, bio_res.user_entity_id, bio_res.transaction_id);
    } else {
        QSEE_LOG(QSEE_LOG_MSG_HIGH, "%s to reset", __func__);
    }

    result = bio_set_auth_result(&bio_res, NULL);
    if(result != BIO_NO_ERROR)
    {
        QSEE_LOG(QSEE_LOG_MSG_FATAL, "%s bio_set_auth_result fail , result = %d" , __func__, result);
    }
}

unsigned int payment_setAuthenticatorVersion(unsigned int version, char* path)
{
    QSEE_LOG(QSEE_LOG_MSG_HIGH, "payment_setAuthenticatorVersion, version = %d", version);
    uint32_t retval = IFAA_ERR_SUCCESS;
    int32_t write_result;
    int32_t fd;

    QSEE_LOG(QSEE_LOG_MSG_DEBUG, "payment_setAuthenticatorVersion, path=%s", path);
    fd = open((const char*)path, O_RDWR | O_CREAT | O_TRUNC);
    if(fd < 0)
    {
        QSEE_LOG(QSEE_LOG_MSG_FATAL, "payment_setAuthenticatorVersion open file fail");
        return IFAA_ERR_UNKNOWN;
    }

    write_result = write(fd, (const void*)&version, sizeof(int32_t));
    if (write_result != sizeof(int32_t))
    {
        QSEE_LOG(QSEE_LOG_MSG_FATAL, "payment_setAuthenticatorVersion write file fail");
          retval = IFAA_ERR_WRITE;
    }

    close(fd);

    return retval;
}

unsigned int payment_update_fingerprint_ids(unsigned int* ids, unsigned int count, char* path)
{
    QSEE_LOG(QSEE_LOG_MSG_HIGH, "payment_update_fingerprint_ids");
    uint32_t retval = IFAA_ERR_SUCCESS;
    int32_t fd;
    int32_t write_result;
    int32_t trans_pkg[MAX_FINGERS_PER_USER + 1] = {0};

    if (ids == NULL || count > MAX_FINGERS_PER_USER)
    {
        QSEE_LOG(QSEE_LOG_MSG_FATAL, "ERROR! payment_update_fingerprint_ids INVALID_PARAMETER");
        return IFAA_ERR_BAD_PARAM;
    }

    trans_pkg[0] = count;
    memcpy((void*)&trans_pkg[1], (void*)ids, count * sizeof(int32_t));

    unsigned int ret_size = 0;
    const unsigned int file_size = (MAX_FINGERS_PER_USER + 1) * sizeof(int32_t); //fid_count and 5 fid
    QSEE_LOG(QSEE_LOG_MSG_DEBUG, "payment_update_fingerprint_ids, path=%s", path);

    fd = open((const char*)path, O_RDWR | O_CREAT | O_TRUNC);
    if(fd < 0)
    {
        QSEE_LOG(QSEE_LOG_MSG_FATAL, "payment_update_fingerprint_ids open file fail");
        return IFAA_ERR_UNKNOWN;
    }


    ret_size = write(fd, (unsigned char*)trans_pkg, file_size);
    if (ret_size != file_size){
        QSEE_LOG(QSEE_LOG_MSG_FATAL, "ERROR! payment_update_fingerprint_ids save file fail");
        retval = IFAA_ERR_WRITE;
    }

    close(fd);
    return retval;
}
