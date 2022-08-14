#ifndef _AUTOSMOKING_H_
#define _AUTOSMOKING_H_

#include "FpType.h"
#include "HalLog.h"
#include <stdlib.h>
#include <utils/Vector.h>
#include <stddef.h>
#include <stdint.h>
#include "fingerprint.h"
#include <utils/Mutex.h>

#define MAX_FILE_NAME_LENTH 256
#define MAX_IMAGE_DATA_LENTH (256 * 1024)
#define MAX_FILE_DATA_LENTH (600 * 1024)
#define MAX_CASE_LEN 100

#define AST_ENROLL_PATH_1 "/data/vendor/fingerprint/image_test/enroll/1"
#define AST_ENROLL_PATH_2 "/data/vendor/fingerprint/image_test/enroll/2"
#define AST_ENROLL_PATH_3 "/data/vendor/fingerprint/image_test/enroll/3"
#define AST_ENROLL_PATH_4 "/data/vendor/fingerprint/image_test/enroll/4"
#define AST_ENROLL_PATH_5 "/data/vendor/fingerprint/image_test/enroll/5"
#define AST_ENROLL_PATH_6 "/data/vendor/fingerprint/image_test/enroll/6"

#define AST_AUTH_PATH     "/data/vendor/fingerprint/image_test/auth"

#define AST_ENROLL_PATH_REPEAT_PROCESS  "/data/vendor/fingerprint/image_test/enroll/repeat_process"
#define AST_ENROLL_PATH_LOW_QUALITY     "/data/vendor/fingerprint/image_test/enroll/low_quality"
#define AST_ENROLL_PATH_SMALL_AREA      "/data/vendor/fingerprint/image_test/enroll/small_area"


#define AST_CALI_PATH                   "/data/vendor/fingerprint/image_test/cali"
#define AST_ENROLL_PATH                   "/data/vendor/fingerprint/image_test/enroll"

#define AST_AUTH_TOKEN_SIZE 69
#define AST_TEMPLATE_DIR                "/data/vendor_de/0/fpdata/"

typedef enum
{
    DOWN_EVNT = 0,
    UI_EVNT = 1,
    UP_EVNT = 2,
} SIMULATED_EVENT_T;

typedef enum
{
    CMD_TEST_ENROLL_AUTH_SUCCESS,
    CMD_TEST_ENROLL_AUTH_FAIL,
    CMD_TEST_ENROLL_DEPLICATE,
    CMD_TEST_ENROLL_AUTH_SUCCESS_MUL,
    CMD_TEST_ENROLL_AUTH_FAIL_MUL,
    CMD_TEST_ENROLL_AUTH_SUCCESS_AND_FAIL,
    CMD_TEST_ENROLL_EXCEED_UPPER_LIMIT,
} ENROLL_AND_AUTH_TYPE;

namespace android
{
class IAutoSmokingInterface
{
public:
    virtual ~IAutoSmokingInterface() {}
    virtual uint64_t preEnroll() = 0;
    virtual fp_return_type_t enroll(const void *hat, uint32_t gid, uint32_t timeoutSec) = 0;
    virtual fp_return_type_t postEnroll() = 0;
    virtual fp_return_type_t authenticate(uint64_t operationId, uint32_t gid) = 0;
    virtual uint64_t getAuthenticatorId() = 0;
    virtual fp_return_type_t enumerate() = 0;
    virtual fp_return_type_t remove(uint32_t gid, uint32_t fid) = 0;
    virtual fp_return_type_t cancel() = 0;
    virtual fp_return_type_t setActiveGroup(uint32_t gid, const char* path) = 0;
    virtual fp_return_type_t sendTaCommand(void *cmd, uint32_t size) = 0;
    virtual fp_return_type_t simulatedEvent(SIMULATED_EVENT_T event) = 0;
};

class AutoSmoking
{
public:
    AutoSmoking(IAutoSmokingInterface* handle, fp_sensor_type_t type);
    ~AutoSmoking();
    fp_return_type_t autoSmokingCase(int32_t cmd_id, uint32_t gid);
    fp_return_type_t setEnrollFinish(uint32_t isFinish);
    fp_return_type_t setCurrentFingerlist(fp_enumerate_t cmd, uint32_t current_gid);

private:
    fp_return_type_t switchSmokingMode(fp_injection_mode_t config);
    fp_return_type_t readDatFile(const char* fullpath, uint8_t** data, uint32_t* data_len);
    fp_return_type_t injectData(const char *filepath, uint8_t* data, uint32_t data_len);
    fp_return_type_t loadCaliFile(const char *filepath);
    fp_return_type_t loadImage(const char *image_path, uint8_t file_count);
    fp_return_type_t doAuthenticate(const char *auth_image_path, int auth_totoal_times, uint32_t gid);
    fp_return_type_t doEnroll(const char *enroll_image_path, int enroll_totoal_times, uint32_t gid, int leave_halfway, uint8_t should_timeout);
    fp_return_type_t removeAll();
    fp_return_type_t loadCaliWithSensor();
    fp_return_type_t readFileList(const char *basePath, std::vector<std::string> &allFiles);
    int32_t judgeSmokingFinsh(int32_t cmdid);

public:
    IAutoSmokingInterface* mhandle;
private:
    uint32_t mEnrollFinish;
    Mutex mSmokeLock;
    Mutex mEnrollLock;
    fingerprint_enumerated_t mfingerlist;
    fp_sensor_type_t mSensorType;
    uint32_t mSmokingFinish[MAX_CASE_LEN];
};
}  // namespace goodix
#endif  // _AUTOFUNCTESTMANAGER_H_

