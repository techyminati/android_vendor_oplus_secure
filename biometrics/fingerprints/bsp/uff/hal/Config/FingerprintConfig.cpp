#define LOG_TAG "[FP_HAL][FingerprintConfig]"
#include "FingerprintConfig.h"
#include <cutils/properties.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "HalLog.h"

#define FP_ID_PATH "/proc/fp_id"
#define LCD_PATH "/proc/devinfo/lcd"
#define PROJECT_PROPERTY "ro.separate.soft"
#define FP_MAX_LENGTH 60
#define FIGNERPRINT_CONFIG_PATH "/omd/fingerprintConfig.ini"

// map fwk or apk property
#define P_SENSOR_TYPE "persist.vendor.fingerprint.fp_id"
#define P_OPTICAL_ICONSIZE "persist.vendor.fingerprint.optical.iconsize"
#define P_OPTICAL_ICONLOCATION "persist.vendor.fingerprint.optical.iconlocation"
#define P_OPTICAL_ICONNUMBER "persist.vendor.fingerprint.optical.iconnumber"
#define P_OPTICAL_SENSORLOCATION "persist.vendor.fingerprint.optical.sensorlocation"
#define P_OPTICAL_SENSORROTATION "persist.vendor.fingerprint.optical.sensorrotation"
#define P_OPTICAL_CIRCLENUMBER "persist.vendor.fingerprint.optical.circlenumber"
#define P_FINGERPRINT_FP_TYPE "persist.vendor.fingerprint.sensor_type"
#define P_SYS_LOCK_STATE "ro.boot.vbmeta.device_state"
#define P_FINGERPRINT_VERSION "persist.vendor.fingerprint.version"
#define P_PROJECT_SOFTFEATURE "ro.separate.soft"

namespace android {

char* FingerprintTaConfig::readProcNode(const char *path, char* buf)
{
    int fd = -1;
    char sys_buffer_data[128] = {0};

    if ((fd = open(path, O_RDONLY)) >= 0) {
        if (read(fd, (void *)&sys_buffer_data, sizeof(sys_buffer_data)) < 0) {
            close(fd);
            return NULL;
        }
        close(fd);
    } else {
        return NULL;
    }
    memcpy(buf, sys_buffer_data, strlen(sys_buffer_data));
    return buf;
}

fp_return_type_t FingerprintTaConfig::initTaConfig()
{
    fp_return_type_t ret = FP_SUCCESS;
    fp_ta_config_cmd_t data;
    memset(&data, 0, sizeof(fp_ta_config_cmd_t));
    data.header.module_id = FP_MODULE_FPCORE;
    data.header.cmd_id = FP_CMD_FPCORE_INIT_TA_CONFIG;
    memcpy(data.config.project_name, mProjectName, sizeof(mProjectName));
    memcpy(data.config.lcd_name, mLcdType, sizeof(mLcdType));
    memcpy(data.config.sensor_id, mFpId, sizeof(mFpId));
    memcpy(data.config.lock_state, mLockState, sizeof(mLockState));

    ret = HalContext::getInstance()->mCaEntry->sendCommand(&data, sizeof(fp_ta_config_cmd_t));
    if (ret) {
        LOG_E(LOG_TAG, "sendFingerprintCmdToTA, err:%d", ret);
        return FP_ERR_SEND_CMD_TO_TA;
    }
    return ret;
}

fp_return_type_t FingerprintTaConfig::getTaConfigById(int cmd_id, char *buf, unsigned int len)
{
    fp_return_type_t ret = FP_SUCCESS;
    fp_ta_config_rqst_t data;
    memset(&data, 0, sizeof(fp_ta_config_rqst_t));
    data.header.module_id = FP_MODULE_FPCORE;
    data.header.cmd_id = FP_CMD_FPCORE_GET_TA_CONFIG;
    data.request.cmd_id = cmd_id;
    data.request.in_len = len;
    ret = HalContext::getInstance()->mCaEntry->sendCommand(&data, sizeof(fp_ta_config_rqst_t));
    if (ret) {
        LOG_E(LOG_TAG, "sendFingerprintCmdToTA, err:%d", ret);
        return FP_ERR_SEND_CMD_TO_TA;
    }
    memcpy(buf, data.request.in_buf, len);
    return ret;
}

fp_return_type_t FingerprintTaConfig::readLcdType(char *buffer, unsigned int buffer_size)
{
    char lcdInfo[128] = {'\0'};
    string lcdType = "SDC";
    (void)buffer_size;
    if (readProcNode(LCD_PATH, lcdInfo) == NULL) {
        LOG_E(LOG_TAG, "read lcd type fail, use default");
        goto fp_out;
    }

    if (strstr(lcdInfo, "BOE") || strstr(lcdInfo, "boe")) {
        lcdType = "BOE";
    } else if (strstr(lcdInfo, "samsung") || strstr(lcdInfo, "SAMSUNG")) {
        lcdType = "SDC";
    } else if (strstr(lcdInfo, "TM") || strstr(lcdInfo, "tm")) {
        lcdType = "TM";
    } else {
        lcdType = "SDC";
    }
fp_out:
    memcpy(buffer, lcdType.c_str(), strlen(lcdType.c_str()));
    return FP_SUCCESS;
}

fp_return_type_t FingerprintTaConfig::getLockState(char* buffer)
{
    fp_return_type_t ret = FP_SUCCESS;

    ret = (fp_return_type_t)property_get(P_SYS_LOCK_STATE, buffer, "NULL");
    if (ret < 0) {
        LOG_E(LOG_TAG, "get ro.boot.vbmeta.device_state fail, ret =%d", ret);
    }
    return FP_SUCCESS;
}

void FingerprintTaConfig::setSpecialConfig()
{
    char lcdInfo[128] = {'\0'};
    char separate_soft[PROPERTY_VALUE_MAX] = {0};
    property_get("ro.separate.soft", separate_soft, "0");
    if ((!strncmp(separate_soft, "21311", sizeof("21311")))
        || (!strncmp(separate_soft, "21312", sizeof("21312")))
        || (!strncmp(separate_soft, "21313", sizeof("21313")))) {
        readProcNode(LCD_PATH, lcdInfo);
        if ((strstr(lcdInfo, "samsung") || strstr(lcdInfo, "SAMSUNG")) && strstr(lcdInfo, "S6E3FC3")) {
            memcpy(mIconLocation, "208", sizeof("208"));
        }
    }
}

fp_return_type_t FingerprintTaConfig::init()
{
    fp_return_type_t ret = FP_SUCCESS;

    if (property_get(PROJECT_PROPERTY, mProjectName, "0") <= 0) {
        ret = FP_ERR_CONFIG_NODE_PROJECT;
        LOG_E(LOG_TAG, "read project name fail");
        goto fp_out;
    }

    if (readLcdType(mLcdType, TA_STR_LEN) != FP_SUCCESS) {
        ret = FP_ERR_CONFIG_NODE_LCD;
        LOG_E(LOG_TAG, "read lcd type fail");
        goto fp_out;
    }
    LOG_I(LOG_TAG, "lcd type is %s", mLcdType);

    if (getLockState(mLockState) != FP_SUCCESS) {
        ret = FP_ERR_CONFIG_NODE_LOCKSTATE;
        LOG_E(LOG_TAG, "read lock state fail");
        goto fp_out;
    }
    LOG_I(LOG_TAG, "lcd LockState is %s", mLockState);

    if (initTaConfig()) {
        goto fp_out;
    }
    // calibration
    (void)getTaConfigById(E_CALI_MID_BRIGHTNESS, (char *)&mCaliMidBrightness, sizeof(mCaliMidBrightness));
    (void)getTaConfigById(E_CALIBRAION_FLOW, (char *)mCaliflow, sizeof(mCaliflow));
    LOG_D(LOG_TAG, "cali bright:%d", mCaliMidBrightness);
    // delay
    (void)getTaConfigById(E_CALI_BRIGHTNESS_DELAY, (char *)&mCaliBrightnessDelay, sizeof(mCaliBrightnessDelay));
    (void)getTaConfigById(E_CALI_HBM_DELAY, (char *)&mCaliHbmDelay, sizeof(mCaliHbmDelay));
    (void)getTaConfigById(E_AUTH_UIREADY_DELAY, (char *)&mAuthUireadyDelay, sizeof(mAuthUireadyDelay));
    LOG_D(LOG_TAG, "delay:%d, %d, %d", mCaliBrightnessDelay, mCaliHbmDelay, mAuthUireadyDelay);
    // icon settings
    (void)getTaConfigById(E_ICON_SIZE, mIconSize, sizeof(mIconSize));
    (void)getTaConfigById(E_ICON_LOCATION, mIconLocation, sizeof(mIconLocation));
    (void)getTaConfigById(E_ICON_NUMBER, mIconNumber, sizeof(mIconNumber));
    (void)getTaConfigById(E_SENSOR_LOCATION, mSensorLocation, sizeof(mSensorLocation));
    (void)getTaConfigById(E_SENSOR_ROTATION, mSensorRotation, sizeof(mSensorRotation));
    (void)getTaConfigById(E_FP_TYPE, mFpType, sizeof(mFpType));
    (void)getTaConfigById(E_FP_TYPE, mCircleNumber, sizeof(mCircleNumber));
    LOG_D(LOG_TAG, "delay:%s, %s, %s, %s", mIconSize, mIconLocation, mIconNumber, mFpType);

    // func enable
    (void)getTaConfigById(E_TOKEN_ENABLE, (char *)&mCheckTokenEnable, sizeof(mCheckTokenEnable));
    (void)getTaConfigById(E_UI_FIRST_ENABLE, (char *)&mUiFirstEnble, sizeof(mUiFirstEnble));
    (void)getTaConfigById(E_TEE_BINDCORE_ENABLE, (char *)&mTeeBindcoreEnble, sizeof(mTeeBindcoreEnble));
    (void)getTaConfigById(E_RAWDATA_TEST_ENABLE, (char *)&mRawdaTaTestEnable, sizeof(mRawdaTaTestEnable));
    (void)getTaConfigById(E_DUMP_ENABLE, (char *)&mDumpEnable, sizeof(mDumpEnable));
    LOG_D(LOG_TAG, "func:%d, %d, %d, %d, dump:%d", mCheckTokenEnable, mUiFirstEnble, mTeeBindcoreEnble, mRawdaTaTestEnable, mDumpEnable);
    Dump::setDumpSupport(mDumpEnable);

    // system config
    (void)getTaConfigById(E_BIGCORE_VALUES, mBigCores, sizeof(mBigCores));
    (void)getTaConfigById(E_UIFIRST_VALUES, mUiFirstValue, sizeof(mUiFirstValue));
    LOG_D(LOG_TAG, "system:%s, %s", mBigCores, mUiFirstValue);
    setSpecialConfig();
fp_out:
    LOG_D(LOG_TAG, "%s, %s, %s", mProjectName, mFpId, mLcdType);
    return ret;
}

FingerprintTaConfig::FingerprintTaConfig(/* args */)
{
}

FingerprintTaConfig::~FingerprintTaConfig()
{
}


FingerprintConfig::FingerprintConfig(HalContext *context)
{
    (void)context;
    mTaConfig = new FingerprintTaConfig;
}


fp_return_type_t FingerprintConfig::init()
{
    fp_return_type_t ret = FP_SUCCESS;
    //The first number： major version,
    //TThe second number：new feature upgrade version,
    //TThe third number： bug fix version,
    //Tthe fourth number：provisional version.
    const char *halVersion = "UFF V1.0.0.0";
    setProperty(P_FINGERPRINT_VERSION, (char *)halVersion);
    setProperty((const char *)"vendor.fingerprint.meminfo", (char *)"1");

    ret = mTaConfig->init();
    return ret;
}

fp_return_type_t FingerprintConfig::getBaseInfo()
{
    fp_return_type_t ret = FP_SUCCESS;
    //get PROJECT_PROPERTY
    //FP_ID_PATH
    //FP_ID_PATH
    return ret;
}

void FingerprintConfig::setProperty(string name, char *value)
{
    LOG_I(LOG_TAG, "prop:%s, value:%s", name.c_str(), value);
    property_set((const char *)name.c_str(), value);
}

void FingerprintConfig::setIconType()
{
    setProperty(P_SENSOR_TYPE, (char *)mTaConfig->mSensorId);
    setProperty(P_OPTICAL_ICONSIZE, (char *)mTaConfig->mIconSize);
    setProperty(P_OPTICAL_ICONLOCATION, (char *)mTaConfig->mIconLocation);
    setProperty(P_OPTICAL_ICONNUMBER, (char *)mTaConfig->mIconNumber);
    setProperty(P_OPTICAL_SENSORLOCATION, (char *)mTaConfig->mSensorLocation);
    setProperty(P_OPTICAL_SENSORROTATION, (char *)mTaConfig->mSensorRotation);
    setProperty(P_OPTICAL_CIRCLENUMBER, (char *)mTaConfig->mCircleNumber);
    setProperty(P_FINGERPRINT_FP_TYPE, (char *)mTaConfig->mFpType);
}

void FingerprintConfig::setFactoryCaliState(int state)
{
    const char *name = "vendor.fingerprint.cali";
    if (state == 1) {
        setProperty(name, (char *)"1");
    } else {
        setProperty(name, (char *)"0");
    }
}
void FingerprintConfig::setFactoryAlgoVersion(char *buffer)
{
    const char *name = "oplus.fingerprint.gf.package.version";
    char factory_version[128] = {'\0'};
    sprintf(factory_version, "%s_%s", buffer, mTaConfig->mLcdType);
    setProperty(name, factory_version);
}
void FingerprintConfig::setFactoryQrcode(char *buffer)
{
    // oplus.fingerprint.qrcode.support
    const char *name = "oplus.fingerprint.qrcode.value";
    setProperty("oplus.fingerprint.qrcode.support", (char *)"1");
    setProperty(name, buffer);
}

FingerprintConfig::~FingerprintConfig()
{
    if (mTaConfig) {
        delete mTaConfig;
    }
}
}
