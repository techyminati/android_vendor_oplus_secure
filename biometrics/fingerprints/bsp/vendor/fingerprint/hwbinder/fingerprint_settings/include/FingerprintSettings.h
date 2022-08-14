#ifndef FINGERPRINT_SETTINGS_H
#define FINGERPRINT_SETTINGS_H
#include "FingerprintConfig.h"

#define NAME_LENGTH (32)

#define SAMSUNG_KEYWORD "samsung"
#define BOE_KEYWORD "boe"
#define LG_KEYWORD "lg"
#define TM_KEYWORD "tm"
#define TM2_KEYWORD "tm2"

struct lcdInfo {
    lcd_type type;
    int brightness;
};

struct fpConfigInfo {
    char projectNames[128];
    lcdInfo lcd;
    unsigned int hbm_delay;
    unsigned int bright_delay;
};

void fpSettingsInit();
int fpGetDataById(void *para);
#endif // FINGERPRINT_SETTINGS_H
