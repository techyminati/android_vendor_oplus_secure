#include "FingerprintSettings.h"
#include <stdio.h>
#include <string.h>
#include <log/log.h>
#include <cutils/properties.h>
#include "fingerprint_type.h"

#include <algorithm>
#include <cctype>

#define PROPERTY_SCREEN_TYPE   "persist.vendor.fingerprint.optical.lcdtype"

struct fpConfigDevice {
    char projectName[PROPERTY_VALUE_MAX];
    lcd_type type;
    int fpConfigChooseIndex;
};

fpConfigDevice gFpConfigDevice;

fpConfigInfo gfpConfigInfos[] = {
    {"20111 20251 20113 20261 20262 20263 20817 20827 20831 21881 21882", {LCD_SAMSUNG, 1630}}, // 0
    {"20135 20235 20139 20161 20351 20352 20163", {LCD_SAMSUNG, 1915}},       // 1
    {"19015 19016 19525 19125 19126 19127 19128 19521 19365 19367 19368 19567 19568 19569", {LCD_DEFAULT, 580}},       // SM7250R G3 (old)
    {"20131 20133 20255 20257", {LCD_SAMSUNG, 1915}}, // 3
    {"20131 20133 20255 20257", {LCD_BOE, 3125}},     // 4
    {"20131 20133 20255 20257", {LCD_TM, 1915}},      // 5
    {"20075 20076 19165", {LCD_DEFAULT, 1600}},                   // 6
    {"20061 20062", {LCD_SAMSUNG, 1700}},
    {"20031 20311 19815 20857 20858 20859 2085A", {LCD_SAMSUNG, 1700}},  // SM8350R G3S
    {"19825 20851 20852 20853 20854", {LCD_SAMSUNG, 1760}},        //SM8350R G3S
    {"20627 20664", {LCD_SAMSUNG, 1850}},  //SM8350R G3S
    {"20659 20755", {LCD_SAMSUNG, 1915}},  //SM8350R G3S
    {"20171 20172 20353", {LCD_BOE, 3125}},
    {"20171 20172 20353", {LCD_TM, 1830}},
    {"21005 21205", {LCD_BOE, 0x0EC90D0C}}, //SM8350S G7, 50%HBM = 0xFFFF & 0x0EC90D0C, 75%HBM = 0xFFFF & (0x0EC90D0C >> 16)
    {"21001 21201 21631 216AC 20846 20847 2084A", {LCD_SAMSUNG, 0x09B00681}}, //SM8450 G7
    {"default", {LCD_DEFAULT, 1630}},           // 7
};

void fpSettingsInit()
{
    char lcdName[128] = {'\0'};

    memset(&gFpConfigDevice, 0, sizeof(fpConfigDevice));

    // 1. read project name
    property_get(OPTICAL_SEPARATE_SOFTFEATURE, gFpConfigDevice.projectName, "0");
    ALOGI("%s project name:%s", __func__, gFpConfigDevice.projectName);

    // 2. confirm lcd type
    if (read_proc_string_data(LCD_TYPE_PATH, lcdName) == NULL) {
        gFpConfigDevice.type = LCD_DEFAULT;
    } else {
        std::transform(lcdName, lcdName+127, lcdName, std::tolower);
        if (strstr(lcdName, SAMSUNG_KEYWORD)) {
            gFpConfigDevice.type = LCD_SAMSUNG;
        }else if (strstr(lcdName, BOE_KEYWORD)) {
            gFpConfigDevice.type = LCD_BOE;
        } else if (strstr(lcdName, LG_KEYWORD)) {
            gFpConfigDevice.type = LCD_LG;
        } else if (strstr(lcdName, TM_KEYWORD)) {
            gFpConfigDevice.type = LCD_TM;
        } else {
            gFpConfigDevice.type = LCD_DEFAULT;
        }
    }
    ALOGI("%s lcd type:%d", __func__, (int)gFpConfigDevice.type);

    // 3. for avoid list frequently, select the index
    int projectNumbers = sizeof(gfpConfigInfos) / sizeof(fpConfigInfo);
    int configIndex = 0;
    for (configIndex = 0; configIndex < projectNumbers; configIndex++) {
        fpConfigInfo info = gfpConfigInfos[configIndex];
        // 3.1 makesure projectNames contain current project
        if (!strstr(info.projectNames, gFpConfigDevice.projectName)) {
            continue;
        }
        // 3.2 choose lcd type. config "LCD_DEFAULT" means all the lcd use the same brightness
        if (info.lcd.type == LCD_DEFAULT) {
            break;
        }
        // 3.3 if not use the LCD_DEFAULT, we need to choose the correct lcd type
        if (info.lcd.type == gFpConfigDevice.type) {
            break;
        }
    }

    // 3.4 if config not contain the current project, chose the "default" 1630
    if (configIndex == projectNumbers) {
        gFpConfigDevice.fpConfigChooseIndex = projectNumbers - 1;
    } else {
        gFpConfigDevice.fpConfigChooseIndex = configIndex;
    }

    //4. set the screen type property
    int lcd_type = (int)gfpConfigInfos[gFpConfigDevice.fpConfigChooseIndex].lcd.type;
    switch (lcd_type)
    {
        case LCD_BOE:
                property_set(PROPERTY_SCREEN_TYPE, "BOE");
                break;
        case LCD_SAMSUNG:
                property_set(PROPERTY_SCREEN_TYPE, "SDC");
                break;
        case LCD_LG:
                property_set(PROPERTY_SCREEN_TYPE, "LG");
                break;
        case LCD_TM:
                property_set(PROPERTY_SCREEN_TYPE, "TM");
                break;
        default:
                ALOGI("%s unknown lcd type:%d", __func__,lcd_type);
                break;
    }

    ALOGI("%s select config index is :%d", __func__, gFpConfigDevice.fpConfigChooseIndex);
}

static int fpGetBrightness()
{
    return gfpConfigInfos[gFpConfigDevice.fpConfigChooseIndex].lcd.brightness;
}

static int fpGetLcdType()
{
    return (int)gfpConfigInfos[gFpConfigDevice.fpConfigChooseIndex].lcd.type;
}

int fpGetDataById(void *para)
{
    int status = ERROR_OK;
    if (para == NULL) {
        status = -ERROR_PARA;
        goto out;
    }
    fpTransforInfo data;
    memset(&data, 0, sizeof(fpTransforInfo));
    memcpy(&data, para, sizeof(fpTransforInfo));
    ALOGI("%s cmd:%d\n", __func__, data.cmd);
    switch (data.cmd) {
    case FP_CMD_GET_BRIGHTNESS:
        data.response = fpGetBrightness();
        break;
    case FP_CMD_GET_LCD_TYPE:
        data.response = fpGetLcdType();
        break;
    default:
        status =-ERROR_INVALID_CMD;
    }
    memcpy(para, &data, sizeof(fpTransforInfo));
out:
    return status;

}
