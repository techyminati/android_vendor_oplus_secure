/*
 * Copyright (C) 2013-2019, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef _CUSTOMIZEDHALCONFIG_H_
#define _CUSTOMIZEDHALCONFIG_H_

#define PROPERTY_SCREEN_TYPE "persist.vendor.fingerprint.optical.lcdtype"
#define SCREEN_TYPE_AD097_SAMSUNG "AD097_SAMSUNG"
#define SCREEN_TYPE_AD097_BOE "AD097_BOE"
#define SCREEN_TYPE_BD187_SAMSUNG "BD187_SAMSUNG"
#define SCREEN_TYPE_CC151_SAMSUNG "CC151_SAMSUNG"
#define SCREEN_TYPE_CC161_SAMSUNG "CC161_SAMSUNG"
#define SCREEN_TYPE_DD306_SAMSUNG "DD306_SAMSUNG"
#define SCREEN_TYPE_AD119_SAMSUNG "AD119_SAMSUNG"
#define SCREEN_TYPE_AE009_SAMSUNG "AE009_SAMSUNG"
#define SCREEN_TYPE_AE009_BOE "AE009_BOE"
#define SCREEN_TYPE_RA352_SAMSUNG "RA352_SAMSUNG"
// G7
#define SCREEN_TYPE_AA200_SAMSUNG "AA200_SDC"  // SM8450
#define SCREEN_TYPE_AA202_BOE "AA202_BOE"          // SM8350
#define SCREEN_TYPE_AA262_SAMSUNG "AA262_SDC"       // MT6983
#define SCREEN_TYPE_FERRIRA_SAMSUNG "FERRIRA_SDC"   // SM8450
#define SCREEN_TYPE_AA437_SAMSUNG "AA437_SDC"       // SM8450
#define SCREEN_TYPE_AA439_SAMSUNG "AA439_SDC"       // SM8350

#define PROPERTY_DEVICE_STATE "ro.boot.flash.locked"


namespace goodix {
    const char* getDumpRootDir(void);
    const char* getDumpControlProperty(void);
    bool checkScreenType(char *type);
    bool isTerminalUnlocked(void);
}  // namespace goodix

#endif /* _CUSTOMIZEDHALCONFIG_H_ */
