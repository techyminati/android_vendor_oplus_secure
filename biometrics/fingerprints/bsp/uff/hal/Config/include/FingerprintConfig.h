#ifndef FINGERPRINT_CONFIG_H
#define FINGERPRINT_CONFIG_H
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "FpType.h"
#include "HalContext.h"

using namespace std;
#define TA_STR_LEN (32)
#define REQUST_LEN (512)

typedef struct fp_ta_init_config {
    char project_name[TA_STR_LEN];
    char lcd_name[TA_STR_LEN];
    char sensor_id[TA_STR_LEN];
    char lock_state[TA_STR_LEN];
} fp_ta_init_config_t;

typedef struct {
    fp_ta_cmd_header_t       header;
    struct fp_ta_init_config config;
} fp_ta_config_cmd_t;

typedef struct fp_ta_config_requst {
    int  cmd_id;
    int  sub_id;
    char in_buf[REQUST_LEN];
    unsigned int in_len;
} fp_ta_config_requst_t;

typedef struct {
    fp_ta_cmd_header_t         header;
    struct fp_ta_config_requst request;
} fp_ta_config_rqst_t;

enum requst_id_t {
    E_PROJECT_NAME = 0,
    E_LCD_NAME     = 1,
    // calibration
    E_CALI_LOW_BRIGHTNESS    = 2,
    E_CALI_MID_BRIGHTNESS    = 3,
    E_CALI_HIGH_BRIGHTNESS   = 4,
    E_CALI_CUSTOM_BRIGHTNESS = 5,
    // icon settings
    E_SENSOR_ID       = 10,
    E_ICON_SIZE       = 11,
    E_ICON_LOCATION   = 12,
    E_ICON_NUMBER     = 13,
    E_SENSOR_LOCATION = 14,
    E_SENSOR_ROTATION = 15,
    E_FP_TYPE         = 16,
    // function enable
    E_TOKEN_ENABLE        = 20,
    E_UI_FIRST_ENABLE     = 21,
    E_TEE_BINDCORE_ENABLE = 22,
    E_RAWDATA_TEST_ENABLE = 23,
    E_DUMP_ENABLE         = 24,

    // machine config
    E_BIGCORE_VALUES = 31,
    E_UIFIRST_VALUES = 32,
    // delay config
    E_CALI_BRIGHTNESS_DELAY = 40,
    E_CALI_HBM_DELAY        = 41,
    E_AUTH_UIREADY_DELAY    = 42,
    E_ENROLL_UIREADY_DELAY  = 43,
    E_CALIBRAION_FLOW       = 44,
};
namespace android {
class FingerprintTaConfig {
   public:
    // calibrightness
    int mCaliMidBrightness;
    // delay
    int mCaliBrightnessDelay;
    int mCaliHbmDelay;
    int mAuthUireadyDelay;
    // spot config
    char mSensorId[TA_STR_LEN];
    char mIconSize[TA_STR_LEN];
    char mIconLocation[TA_STR_LEN];
    char mIconNumber[TA_STR_LEN];
    char mSensorLocation[TA_STR_LEN];
    char mSensorRotation[TA_STR_LEN];
    char mCircleNumber[TA_STR_LEN];
    char mFpType[TA_STR_LEN];
    // func switch
    int mCheckTokenEnable;
    int mUiFirstEnble;
    int mTeeBindcoreEnble;
    int mRawdaTaTestEnable;
    char mDumpEnable;
    // perf
    char mBigCores[TA_STR_LEN];
    char mUiFirstValue[TA_STR_LEN];
    // kernel
    char mProjectName[TA_STR_LEN];
    char mFpId[TA_STR_LEN];
    char mLcdType[TA_STR_LEN];
    char mCaliflow[256];
    char mLockState[TA_STR_LEN];

   public:
    fp_return_type_t init(/* args */);
    fp_return_type_t setIconType();
    FingerprintTaConfig(/* args */);
    ~FingerprintTaConfig();

   private:
    char *           readProcNode(const char *path, char *buf);
    fp_return_type_t initTaConfig();
    fp_return_type_t getTaConfigById(int cmd_id, char *buf, unsigned int len);
    fp_return_type_t readLcdType(char *buffer, unsigned int buffer_size);
    fp_return_type_t getLockState(char *lock_state);
    void setSpecialConfig();
};
class FingerprintConfig {
   private:
   public:
    FingerprintTaConfig *mTaConfig;
    fp_return_type_t     init();
    fp_return_type_t     getBaseInfo();
// feature for systemUi
    void                 setIconType();
// feature for factory mode
    void setFactoryCaliState(int state);
    void setFactoryAlgoVersion(char *buffer);
    void setFactoryQrcode(char *buffer);
    void                 setProperty(string name, char *value);
    FingerprintConfig(HalContext *context);
    ~FingerprintConfig();
};
}  // namespace android
#endif  //FINGERPRINT_CONFIG_H
