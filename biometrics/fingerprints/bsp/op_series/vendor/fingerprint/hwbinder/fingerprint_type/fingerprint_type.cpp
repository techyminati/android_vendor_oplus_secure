/************************************************************************************
 ** File: - vendor\fingerprint\hwbinder\fingerprint_type.cpp
 ** OPLUS_FEATURE_FINGERPRINT
 ** Copyright (C), 2008-2020, OPLUS Mobile Comm Corp., Ltd
 **
 ** Description:
 **      HIDL Service entry for fingerprint
 **
 ** Version: 1.0
 ** Date created: 10:58:11,16/08/2020
 ** Author: Chen.ran@Prd.BaseDrv
 ** TAG: BSP.Fingerprint.Basic
 ** --------------------------- Revision History: --------------------------------
 **  <author>           <data>            <desc>
 **  Ran.Chen         2017/11/09        create the file
 **  Ziqing.Guo       2017/12/03        fix the warning message
 **  Ran.Chen         2018/01/29        modify for fp_id, Code refactoring
 **  Hongdao.yu       2018/03/13        add gf_5298
 **  Ran.Chen         2018/03/21        modify for  compile warning
 **  Hongdao.yu       2018/03/26        add for gf_5298(5228+158) glass
 **  Ran.Chen         2018/06/25        add for 1023(1023+2060) glass
 **  Yang.Tan         2018/11/09        add for 18531 fpc1511
 **  Yang.Tan         2018/11/26        add fpc sw23 and sw28 compatible
 **  Ran.Chen         2018/12/06        modify for optical_fingerprint set iconlocaltion
 **  Yang.Tan         2018/12/11        add fingerprint image quality pass  in engineeringinfo
 **  Ran.Chen         2018/01/07        modify for optical_fingerprint set iconlocaltion for 18116
 **  Bangxiong.Wu     2019/02/23        modify 18073 iconlocation and size
 **  Dongnan.Wu       2019/02/23        add goodix optical_fingerprint set iconlocaltion for 18073
 **  Bangxiong.Wu     2019/02/24        add for 18593 silead_optical_fp
 **  Ran.Chen         2019/03/07        add for 18120
 **  Bangxiong.Wu     2019/03/09        Icon property set for 18075 silead optical fp
 **  Dongnan.Wu       2019/03/12        modify icon size for 18073 & 18593
 **  Qing.Guan        2019/04/01        add for egis fp 18041
 **  Ran.Chen         2019/04/30        add for 18117\18118
 **  Qing.Guan        2019/05/08        add for silead 710P
 **  Bangxiong.Wu     2019/05/16        Add icon set for SM7150(MSM_19031 MSM_19331)
 **  Ran.Chen         2019/05/16        add for 19071\19081\19371
 **  Bangxiong.Wu     2019/05/20        Add icon set for SM7150(MSM_19032)
 **  Dongnan.Wu       2019/05/21        add icon set for 19011&19301
 **  Qing.Guan        2019/05/23        add 18081/18181 goodix fp using old icon
 **  Qing.Guan        2019/05/29        modify 19081 19071 19371 G3 icon
 **  Ziqing.Guo       2019/08/16        modify for goodix optical android Q (Euclid)
 **  Ran.Chen         2019/09/16        add for SDM7250 G3/G5
 **  Ran.Chen         2019/10/02        modify for 19125 icon location
 **  Bangxiong.Wu     2019/10/03        Add icon and screen type setting for SM8250(19065/19066)
 **  Bangxiong.Wu     2019/10/12        modify SM8250 lcdtype as it need different positioning circle
 **  Ran.Chen         2019/10/15        add for lcd_type
 **  Ran.Chen         2019/10/25        add for sensorrotation
 **  Bangxiong.Wu     2019/11/08        move G5 algo version property setting to HalContext init
 **  Ran.Chen         2020/02/24        add for SDM7250 G6 (19191)  G3 (19015)
 **  Zemin.Li         2020/02/28        add for 19305 icon
 **  Ran.Chen         2020/03/10        add for G_OPTICAL_G6 (19191)
 **  zhuangzhuang.Zhu 2020/09/19        add for SDM7150 G3 (206B1 206B2 206B3 206B5)
 **  Zemin.Li         2020/10/22        add fp_typp in fp_config_info
 **  Mingzhi.Guo      2020/10/30        add RM SM8350 G3S fp_config_info (20627 20664)
 **  Mingzhi.Guo      2020/11/24        add RM SM8350 G3S fp_config_info (20659 20755)
 ************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <log/log.h>
#include "fingerprint_type.h"
#include <cutils/properties.h>

#define TA_TBASE_PATH_1023     "odm/vendor/app/mcRegistry/04010000000000000000000000000000.tlbin"
#define TA_TBASE_PATH_1022     "odm/vendor/app/mcRegistry/04010000000000000000000000000001.tlbin"
#define TA_TBASE_PATH_GF5298   "odm/vendor/app/mcRegistry/05060000000000000000000000000000.tabin"
#define TA_TBASE_PATH_GF5658   "odm/vendor/app/mcRegistry/05060000000000000000000000000001.tabin"
#define TA_TBASE_PATH_1023_GS  "odm/vendor/app/mcRegistry/04010000000000000000000000000006.tlbin"
#define TA_TBASE_PATH_1511     "odm/vendor/app/mcRegistry/04010000000000000000000000000005.tlbin"
#define TA_TBASE_PATH_1541     "odm/vendor/app/mcRegistry/04010000000000000000000000000007.tlbin"
#define TA_TBASE_PATH_520      "odm/vendor/app/mcRegistry/08080000000000000000000000000000.tlbin"

#define TA_DEFAULT_NAME        "ta_default_name"

#define OPTICAL_ICONLOCATION   "persist.vendor.fingerprint.optical.iconlocation"
#define OPTICAL_ICONSIZE       "persist.vendor.fingerprint.optical.iconsize"
#define OPTICAL_ICONNUMBER     "persist.vendor.fingerprint.optical.iconnumber"
#define OPTICAL_SENSORLOCATION "persist.vendor.fingerprint.optical.sensorlocation"
#define PROPERTY_SCREEN_TYPE   "persist.vendor.fingerprint.optical.lcdtype"
#define OPTICAL_SENSORROTATION "persist.vendor.fingerprint.optical.sensorrotation"
#define FINGEPRINT_FP_ID       "persist.vendor.fingerprint.fp_id"
#define FINGERPRINT_FP_TYPE    "persist.vendor.fingerprint.sensor_type"
/************************************************************************************
 ** circlenumber, used in factorymode-test apk
 0: G5-(19101\19161)
 1: G5-(19065)
 2: G6-(19191\19161)
 3: G6-(19065)
 ************************************************************************************/

/************************************************************************************
 * fp_type value:
 * 0 unknown
 * 1 side
 * 2 front
 * 3 back
 * 4 optical
 ************************************************************************************/
#define OPTICAL_CIRCLENUMBER   "persist.vendor.fingerprint.optical.circlenumber"

fp_config_info_t fp_config_info_init;

fp_config_info_t fp_config_info[] = {
    /*sensor id-------fp_id_string-----ta_qseename---ta_tbasepath-------fp_ft_type------ic_type---times--score-image_size--threshold/count--up_irq--fp_type */
    {E_F_1140,          "F_1140",        "a_fp",     TA_DEFAULT_NAME,      FP_FPC,             1140,    20,   20,  192*56,      {200, 0},   false,  2},
    {E_F_1260,          "F_1260",        "b_fp",     TA_DEFAULT_NAME,      FP_FPC,             1260,    20,   20,  176*64,      {200, 0},   true,   2},
    {E_F_1022,          "F_1022",        "a_fp",     TA_TBASE_PATH_1022,   FP_FPC,             1022,    12,   20,  112*88,      {60, 0},    true,   3},
    {E_F_1023,          "F_1023",        "a_fp",     TA_TBASE_PATH_1023,   FP_FPC,             1023,    16,   20,  64*80,       {32, 1},    true,   3},
    {E_F_1023_GLASS,    "F_1023_GLASS",  "a_fp",     TA_TBASE_PATH_1023_GS, FP_FPC,            1023,    16,   20,  64*80,       {32, 1},    true,   3},
    {E_F_1511,          "F_1511",        "f_1511",   TA_TBASE_PATH_1511,   FP_FPC,             1511,    16,   20,  64*80,       {32, 1},    true,   3},
    {E_F_1027,          "F_1027",        "b_fp",     TA_DEFAULT_NAME,      FP_FPC,             1027,    8,    20,  128*112,     {200, 0},   true,   3},
    {E_G_3268,          "G_3268",        "a_goodix", TA_DEFAULT_NAME,      FP_GOODIX,          3268,    12,   25,  88*108,      {0, 0},     true,   3},
    {E_G_5288,          "G_5288",        "b_goodix", TA_DEFAULT_NAME,      FP_GOODIX,          5288,    12,   25,  88*108,      {0, 0},     true,   3},
    {E_G_5298,          "G_5298",        "b_goodix", TA_TBASE_PATH_GF5298, FP_GOODIX,          5298,    12,   25,  64*80,       {0, 0},     true,   3},
    {E_G_5298_GLASS,    "G_5298_GLASS",  "b_goodix", TA_TBASE_PATH_GF5298, FP_GOODIX,          5298,    12,   25,  64*80,       {0, 0},     true,   3},
    {E_G_5658,          "G_5658",        "b_goodix", TA_TBASE_PATH_GF5658, FP_GOODIX,          5658,    12,   25,  64*80,       {0, 0},     true,   3},
    {E_G_5228,          "G_5228",        "b_goodix", TA_DEFAULT_NAME,      FP_GOODIX,          5228,    12,   25,  64*80,       {0, 0},     true,   3},
    {E_G_OPTICAL_G2,    "G_OPTICAL_G2",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_G2,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_G_OPTICAL_G3,    "G_OPTICAL_G3",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_G3,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_G_OPTICAL_G3S,   "G_OPTICAL_G3S", "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_G3S,  9500,   12,   20,  64*80,    {0, 0},     true,   4},
    {E_G_OPTICAL_G5,    "G_OPTICAL_G5",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_G5,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_G_OPTICAL_G6,    "G_OPTICAL_G6",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_G6,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_G_OPTICAL_G7,    "G_OPTICAL_G7",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_G7,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_S_OPTICAL_70,    "S_OPTICAL_70",  "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_S_Truly0,        "S_Truly0",      "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_S_Ofilm0,        "S_Ofilm0",      "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_S_Fingerchip0,   "S_Fingerchip0", "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_S_Truly1,        "S_Truly1",      "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_S_Ofilm1,        "S_Ofilm1",      "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_S_Fingerchip1,   "S_Fingerchip1", "sileadta", TA_DEFAULT_NAME,      FP_OPTICAL_SILEAD,  9500,    17,   45,  160*160,     {0, 0},     true,   4},
    {E_E_OPTICAL_ET713, "E_OPTICAL_ET713", "egis",   TA_DEFAULT_NAME,      FP_OPTICAL_EGIS,    713,     17,   10,  200*200,     {0, 0},     true,   4},
    {E_FP_GOODIX_3626,  "FP_GOODIX_3626", "b_goodix", TA_TBASE_PATH_GF5658, FP_GOODIX,         5658,    12,   25,  64*80,       {0, 0},     true,   1},
    {E_G_3626,          "G_3626",        "goodixfp", TA_DEFAULT_NAME,      FP_GOODIX,          3626,    20,   45,  64*80,       {0, 0},     true,   1},
    {E_F_1541,          "F_1541",        "f_1541",   TA_TBASE_PATH_1541,   FP_FPC,             1541,    20,   20,  160*35,      {200, 0},   true,   1},
    {E_E_520,           "E_520",         "egista",   TA_TBASE_PATH_520,    FP_EGIS,            520,     20,   20,  57*46,       {0, 0},     false,  4},
    {E_19821_G5,   "G_OPTICAL_19821_G5",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_19821_G5,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_19821_G6,   "G_OPTICAL_19821_G6",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_19821_G6,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_19805_G6_3, "G_OPTICAL_19805_G6_3",  "goodixfp", TA_DEFAULT_NAME,    FP_OPTICAL_GOODIX_19805_G6_3,  9500,  12,   20,  64*80,    {0, 0},     true,   4},
    {E_19805_G6_7, "G_OPTICAL_19805_G6_7",  "goodixfp", TA_DEFAULT_NAME,    FP_OPTICAL_GOODIX_19805_G6_7, 9500,   12,   20,  64*80,    {0, 0},     true,   4},
    {E_19811_G5,   "G_OPTICAL_19811_G5",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_19811_G5,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_19811_G6,   "G_OPTICAL_19811_G6",  "goodixfp", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_19811_G6,  9500,    12,   20,  64*80,    {0, 0},     true,   4},
    {E_20838_G6_3, "G_OPTICAL_G6_3.0",  "goodixfp", TA_DEFAULT_NAME,       FP_OPTICAL_GOODIX_G6_3, 9500,   12,   20,  64*80,    {0, 0},     true,   4},
    {E_20828_G6_7, "G_OPTICAL_G6_7.0",  "goodixfp", TA_DEFAULT_NAME,       FP_OPTICAL_GOODIX_G6_7, 9500,   12,   20,  64*80,    {0, 0},     true,   4},
    {E_G_OPTICAL_20801_G3, "G_OPTICAL_20801_G3",  "goodix9558", TA_DEFAULT_NAME,      FP_OPTICAL_GOODIX_20801_G3,  9500, 12,
     20, 64*80,    {0, 0},     true,   4},
};

char* read_proc_string_data(const char *path, char* buf) {
    int fd = -1;
    char sys_buffer_data[128] = {0};

    if ((fd = open(path, O_RDONLY)) >= 0) {
        if (read(fd, (void *)&sys_buffer_data, sizeof(sys_buffer_data)) < 0) {
            ALOGE(" fpc %s read %s error !", __func__, path);
            close(fd);
            return NULL;
        }
        close(fd);
    }
    else {
        ALOGE(" read_proc_string_data %s open %s error!!", __func__, sys_buffer_data);
        return NULL;
    }
    memcpy(buf, sys_buffer_data, strlen(sys_buffer_data));
    return buf;
}

uint32_t oplus_fp_type_init() {
    char buffer_fp_id[FP_ID_MAX_LENGTH] = {0};
    uint32_t i = 0;
    if (read_proc_string_data(FP_ID_PATH, buffer_fp_id) == NULL) {
        goto out;
    }
    if (strlen(buffer_fp_id) <= FPC_SENSOR_TYPE_PREFIX_LENGTH || strlen(buffer_fp_id) > FP_ID_MAX_LENGTH) {
        goto out;
    }
    for (i = 0; i< sizeof (fp_config_info)/sizeof (fp_config_info[0]); i++) {
        // according the second argument (fp_id_string) to set fp_config_info_init
        if (0 == strcmp(buffer_fp_id, fp_config_info[i].fp_id_string)) {
            if (0 == strcmp(buffer_fp_id, "G_OPTICAL_19805_G6_7")) {
                property_set(FINGEPRINT_FP_ID, "G_OPTICAL_G6_7.0");
            } else if ((0 == strcmp(buffer_fp_id, "G_OPTICAL_19805_G6_3")) || (0 == strcmp(buffer_fp_id, "G_OPTICAL_G6_3.0"))) {
                property_set(FINGEPRINT_FP_ID, "G_OPTICAL_G6");
            } else if ((0 == strcmp(buffer_fp_id, "G_OPTICAL_19821_G5")) || (0 == strcmp(buffer_fp_id, "G_OPTICAL_19811_G5"))) {
                property_set(FINGEPRINT_FP_ID, "G_OPTICAL_G5");
            } else if ((0 == strcmp(buffer_fp_id, "G_OPTICAL_19821_G6")) || (0 == strcmp(buffer_fp_id, "G_OPTICAL_19811_G6"))) {
                property_set(FINGEPRINT_FP_ID, "G_OPTICAL_G6");
            } else {
                property_set(FINGEPRINT_FP_ID, buffer_fp_id);
            }
            memcpy(&fp_config_info_init, &fp_config_info[i], sizeof(fp_config_info[i]));
            ALOGE("fp read fp_id_string = %s", fp_config_info[i].fp_id_string);
            ALOGD("fp read ta_qsee_name = %s", fp_config_info[i].ta_qsee_name);
            ALOGD("fp read ta_tbase_path = %s", fp_config_info[i].ta_tbase_path);
            ALOGD("fp read fp_factory_type = %d", fp_config_info[i].fp_factory_type);
            ALOGD("fp read fp_ic_type = %d", fp_config_info[i].fp_ic_type);
            ALOGD("fp read total_enroll_times = %d", fp_config_info[i].total_enroll_times);
            ALOGD("fp read fp_image_size = %d", fp_config_info[i].fp_image_size);
            ALOGD("fp read fp_threshold = %d, fp_count= %d, ", fp_config_info[i].fp_threshold_count[0], fp_config_info[i].fp_threshold_count[1]);
            ALOGD("fp read fp_up_irq = %d", fp_config_info[i].fp_up_irq);
            ALOGD("fp read fp_type = %d", fp_config_info[i].fp_type);
            //set persist.vendor.fingerprint.sensor_type
            set_sensor_type_feature(fp_config_info[i].fp_type);
            return 0;
        }
    }
out:
    ALOGE("%s fp read err", __func__);
    return -1;
}

void set_sensor_type_feature(uint32_t sensor_type) {
    switch (sensor_type) {
        case 1:
            property_set(FINGERPRINT_FP_TYPE, "side");
            break;
        case 2:
            property_set(FINGERPRINT_FP_TYPE, "front");
            break;
        case 3:
            property_set(FINGERPRINT_FP_TYPE, "back");
            break;
        case 4:
            property_set(FINGERPRINT_FP_TYPE, "optical");
            break;
        default:
            property_set(FINGERPRINT_FP_TYPE, "unknown");
            break;
    }
}

bool check_fp_type(char* fp_type_string) {
    char buffer_fp_type[FP_ID_MAX_LENGTH] = {0};
    if (read_proc_string_data(FP_ID_PATH, buffer_fp_type) == NULL) {
        return false;
    }
    ALOGD("check_fp_type read buffer_fp_type = %s, fp_type_string = %s", buffer_fp_type, fp_type_string);

    if (strstr(buffer_fp_type, fp_type_string)) {
        return true;
    } else {
        return false;
    }
}

bool check_lcd_type(char* lcd_type_string) {
    char buffer_lcd_type[FP_ID_MAX_LENGTH] = {0};
    if (read_proc_string_data(LCD_TYPE_PATH, buffer_lcd_type) == NULL) {
        return false;
    }
    ALOGD("check_lcd_type read buffer_lcd_type = %s, lcd_type_string = %s", buffer_lcd_type, lcd_type_string);

    if (strstr(buffer_lcd_type, lcd_type_string)) {
        return true;
    } else {
        return false;
    }
}

void set_goodix_icon_property() {
    char separate_soft[PROPERTY_VALUE_MAX] = {0};
    property_get(OPTICAL_SEPARATE_SOFTFEATURE, separate_soft, "0");

    if ((!strncmp(separate_soft, "18115", sizeof("18115")))
                || (!strncmp(separate_soft, "18501", sizeof("18501")))
                ||(!strncmp(separate_soft, "18503", sizeof("18503")))
                ||(!strncmp(separate_soft, "18117", sizeof("18117")))
                ||(!strncmp(separate_soft, "18118", sizeof("18118")))
                ||(!strncmp(separate_soft, "18119", sizeof("18119")))
                ||(!strncmp(separate_soft, "18120", sizeof("18120")))
                ||(!strncmp(separate_soft, "19061", sizeof("19061")))
                ||(!strncmp(separate_soft, "19062", sizeof("19062")))
                ||(!strncmp(separate_soft, "19068", sizeof("19068")))
                ||(!strncmp(separate_soft, "19363", sizeof("19363")))
                ||(!strncmp(separate_soft, "18116", sizeof("18116")))) {
        ALOGI("SDM855 set goodix iconlocation to 305 (pixel:540 2035) (all: 1080 2340), iconsize to 184 ");
        property_set(OPTICAL_ICONLOCATION, "305");
        property_set(OPTICAL_ICONSIZE, "184");
        property_set(OPTICAL_ICONNUMBER, "1");
    } else  if ((!strncmp(separate_soft, "18097", sizeof("18097")))
                || (!strncmp(separate_soft, "18098", sizeof("18098")))
                || (!strncmp(separate_soft, "18397", sizeof("18397")))
                || (!strncmp(separate_soft, "18037", sizeof("18037")))) {
        ALOGI("set goodix iconlocation to 306 (pixel:540 2035) (all: 1080 2340), iconsize to 196 ");
        property_set(OPTICAL_ICONLOCATION, "306");
        property_set(OPTICAL_ICONSIZE, "196");
        property_set(OPTICAL_ICONNUMBER, "1");
    } else  if ((!strncmp(separate_soft, "18041", sizeof("18041")))
                || (!strncmp(separate_soft, "18038", sizeof("18038")))
                || (!strncmp(separate_soft, "18539", sizeof("18539")))) {
        ALOGI("SDM710P set goodix iconlocation to 273 (pixel:540 2035) (all: 1080 2340), iconsize to 196 ");
        property_set(OPTICAL_ICONLOCATION, "273");
        property_set(OPTICAL_ICONSIZE, "196");
        property_set(OPTICAL_ICONNUMBER, "1");
    } else  if (!strncmp(separate_soft, "18383", sizeof("18383"))) {
        ALOGI("SDM710P set goodix iconlocation to 278 iconsize to 196 ");
        property_set(OPTICAL_ICONLOCATION, "278");
        property_set(OPTICAL_ICONSIZE, "196");
        property_set(OPTICAL_ICONNUMBER, "1");
    } else  if ((!strncmp(separate_soft, "18081", sizeof("18081")))
                || (!strncmp(separate_soft, "18085", sizeof("18085")))
                || (!strncmp(separate_soft, "18086", sizeof("18086")))
                || (!strncmp(separate_soft, "18181", sizeof("18181")))
                || (!strncmp(separate_soft, "18381", sizeof("18381")))
                || (!strncmp(separate_soft, "18385", sizeof("18385")))) {
        ALOGI("SDM710P set goodix iconlocation to 306 (pixel:540 2035) (all: 1080 2340), iconsize to 196 ");
        property_set(OPTICAL_ICONLOCATION, "306");
        property_set(OPTICAL_ICONSIZE, "196");
        property_set(OPTICAL_ICONNUMBER, "0");
    } else if ((!strncmp(separate_soft, "18073", sizeof("18073")))
                || (!strncmp(separate_soft, "18593", sizeof("18593")))
                || (!strncmp(separate_soft, "18075", sizeof("18075")))) {
        ALOGI("MT6779 set goodix iconlocation to 278 (pixel:540 2035) (all: 1080 2340), iconsize to 184, iconnumber to 1 ");
        property_set(OPTICAL_ICONLOCATION, "278");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "1");
    } else if ((!strncmp(separate_soft, "19031", sizeof("19031")))
                || (!strncmp(separate_soft, "19331", sizeof("19331")))
                || (!strncmp(separate_soft, "19032", sizeof("19032")))) {
        ALOGI("SM7150 set goodix iconlocation to 235 (pixel:540 2035) (all: 1080 2340), iconsize to 190, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "235");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if (!strncmp(separate_soft, "20801", sizeof("20801"))
            || !strncmp(separate_soft, "20804", sizeof("20804"))) {
        ALOGI("SM7250 set goodix iconlocation to 234 (pixel:540 2166) (all: 1080 2400), iconsize to 204, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "234");
        property_set(OPTICAL_ICONSIZE, "204");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19111", sizeof("19111")))
                || (!strncmp(separate_soft, "19112", sizeof("19112")))
                || (!strncmp(separate_soft, "19513", sizeof("19513")))) {
        ALOGI("SM7150(19111/19112/19513) set goodix iconlocation to 278 (pixel:540 2035) (all: 1080 2340), iconsize to 190, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "278");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19011", sizeof("19011")))
                || (!strncmp(separate_soft, "19301", sizeof("19301")))
                || (!strncmp(separate_soft, "19305", sizeof("19305")))
                || (!strncmp(separate_soft, "19531", sizeof("19531")))
                || (!strncmp(separate_soft, "19532", sizeof("19532")))
                || (!strncmp(separate_soft, "19533", sizeof("19533")))) {
        ALOGI("MT6779(p90M) set goodix iconlocation to 273 (pixel:540 2035) (all: 1080 2340), iconsize to 186, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "273");
        property_set(OPTICAL_ICONSIZE, "186");
        property_set(OPTICAL_ICONNUMBER, "4");
    }  else if ((!strncmp(separate_soft, "19551", sizeof("19551"))) || (!strncmp(separate_soft, "19550", sizeof("19550"))) ||
        (!strncmp(separate_soft, "19553", sizeof("19553"))) || (!strncmp(separate_soft, "19556", sizeof("19556")))) {
        ALOGI("MT6779(p90Q) set goodix iconlocation to 253 (pixel:540 2035) (all: 1080 2340), iconsize to 193, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "253");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19357", sizeof("19357"))) || (!strncmp(separate_soft, "19358", sizeof("19358"))) ||
        (!strncmp(separate_soft, "19359", sizeof("19359"))) || (!strncmp(separate_soft, "19354", sizeof("19354"))) ||
        (!strncmp(separate_soft, "19536", sizeof("19536"))) || (!strncmp(separate_soft, "19537", sizeof("19537"))) ||
        (!strncmp(separate_soft, "19538", sizeof("19538"))) || (!strncmp(separate_soft, "19539", sizeof("19539"))) ||
        (!strncmp(separate_soft, "19375", sizeof("19375"))) || (!strncmp(separate_soft, "19376", sizeof("19376"))) ||
        (!strncmp(separate_soft, "19541", sizeof("19541")))) {
        ALOGI("MT6779(p90Q) set goodix iconlocation to 215 (pixel:540 2035) (all: 1080 2340), iconsize to 193, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "215");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19081", sizeof("19081")))
                ||(!strncmp(separate_soft, "19387", sizeof("19387")))
                ||(!strncmp(separate_soft, "19781", sizeof("19781")))
                ||(!strncmp(separate_soft, "19688", sizeof("19688")))) {
        ALOGI("SDM855 19081 set goodix iconlocation to 244 (pixel:540 2035) (all: 1080 2340), iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "244");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19101", sizeof("19101")))
                ||(!strncmp(separate_soft, "19102", sizeof("19102")))
                ||(!strncmp(separate_soft, "19501", sizeof("19501")))
                ||(!strncmp(separate_soft, "19040", sizeof("19040")))
                ||(!strncmp(separate_soft, "19335", sizeof("19335")))) {
        ALOGI("SDM7250 G5 set goodix iconlocation to 506 (pixel:540 1894) (all: 1080 2400), iconsize to 237");
        property_set(OPTICAL_ICONLOCATION, "506");
        property_set(OPTICAL_ICONSIZE, "237");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "546::1893");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        if (check_lcd_type("boe")) {
            property_set(PROPERTY_SCREEN_TYPE, "AD097_BOE");
        } else {
            property_set(PROPERTY_SCREEN_TYPE, "AD097_SAMSUNG");
        }
        property_set(OPTICAL_CIRCLENUMBER, "0");
    } else if ((!strncmp(separate_soft, "19191", sizeof("19191")))
                ||(!strncmp(separate_soft, "19192", sizeof("19192")))
                ||(!strncmp(separate_soft, "19591", sizeof("19591")))) {
        ALOGI("SDM7250 G6 set goodix iconlocation to 506 (pixel:540 1894) (all: 1080 2400), iconsize to 13.5mm");
        property_set(OPTICAL_ICONLOCATION, "506");
        property_set(OPTICAL_ICONSIZE, "215");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "546::1893");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        if (check_lcd_type("boe")) {
            property_set(PROPERTY_SCREEN_TYPE, "AE009_BOE");
        } else {
            property_set(PROPERTY_SCREEN_TYPE, "AE009_SAMSUNG");
        }
        property_set(OPTICAL_CIRCLENUMBER, "2");
    } else if ((!strncmp(separate_soft, "19567", sizeof("19567")))
                ||(!strncmp(separate_soft, "19568", sizeof("19568")))
                ||(!strncmp(separate_soft, "19569", sizeof("19569")))) {
        ALOGI("SDM7125 G6 set goodix iconlocation to 506 (pixel:540 1894) (all: 1080 2400), iconsize to 13.5mm");
#ifdef SM7125_TMP_CALIBRATION_PARA
        property_set(OPTICAL_ICONLOCATION, "381");
        property_set(OPTICAL_ICONSIZE, "214");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "546::1750");
        property_set(OPTICAL_SENSORROTATION, "83.5");
#else
        property_set(OPTICAL_ICONLOCATION, "506");
        property_set(OPTICAL_ICONSIZE, "214");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "546::1894");
        property_set(OPTICAL_SENSORROTATION, "83.5");
#endif
        if (check_lcd_type("boe")) {
            property_set(PROPERTY_SCREEN_TYPE, "AE009_BOE");
        } else {
            property_set(PROPERTY_SCREEN_TYPE, "AD119_SAMSUNG");
        }
        property_set(OPTICAL_CIRCLENUMBER, "2");
    } else if ((!strncmp(separate_soft, "19365", sizeof("19365")))
                ||(!strncmp(separate_soft, "19367", sizeof("19367")))
                ||(!strncmp(separate_soft, "19368", sizeof("19368")))) {
        ALOGI("SDM7125 G3 set goodix iconlocation to 212 (pixel:540 *) (all: 1080 2400)), iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "215");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19125", sizeof("19125")))
                ||(!strncmp(separate_soft, "19126", sizeof("19126")))
                ||(!strncmp(separate_soft, "19127", sizeof("19127")))
                ||(!strncmp(separate_soft, "19128", sizeof("19128")))
                ||(!strncmp(separate_soft, "19521", sizeof("19521")))
                ||(!strncmp(separate_soft, "19015", sizeof("19015")))
                ||(!strncmp(separate_soft, "19016", sizeof("19016")))
                ||(!strncmp(separate_soft, "19525", sizeof("19525")))
                ||(!strncmp(separate_soft, "19528", sizeof("19528")))) {
        ALOGI("SDM7250 G3 set goodix iconlocation to 212 (pixel:540 *) (all: 1080 2400)), iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "215");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "20111", sizeof("20111")))
                ||(!strncmp(separate_soft, "20113", sizeof("20113")))
                ||(!strncmp(separate_soft, "20251", sizeof("20251")))) {
        ALOGI("SDM7250R G3S set goodix iconlocation to 224 (pixel:540 *) (all: 1080 2400)), iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "224");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19065", sizeof("19065")))
                ||(!strncmp(separate_soft, "19063", sizeof("19063")))
                ||(!strncmp(separate_soft, "19066", sizeof("19066")))
                ||(!strncmp(separate_soft, "19067", sizeof("19067")))
                ||(!strncmp(separate_soft, "19361", sizeof("19361")))
                ||(!strncmp(separate_soft, "19362", sizeof("19362")))) {
        ALOGI("SM8250 G5/G6 set goodix iconlocation to 624 (pixel:1440/2=720 3168-624=2544) (all: 1440 3168),"
                "iconsize to 303(15mm/0.0495), set screen type to CC161_SAMSUNG");
        property_set(OPTICAL_ICONLOCATION, "624");
        property_set(OPTICAL_ICONSIZE, "303");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "728::2543");
        property_set(PROPERTY_SCREEN_TYPE, "CC161_SAMSUNG");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        if (check_fp_type("G_OPTICAL_G5")) {
            property_set(OPTICAL_CIRCLENUMBER, "1");
        } else {
            property_set(OPTICAL_CIRCLENUMBER, "3");
        }
    } else if ((!strncmp(separate_soft, "19821", sizeof("19821")))
                ||(!strncmp(separate_soft, "19823", sizeof("19823")))
                ||(!strncmp(separate_soft, "19839", sizeof("19839")))
                ||(!strncmp(separate_soft, "19841", sizeof("19841")))
                ||(!strncmp(separate_soft, "19855", sizeof("19855")))
                ||(!strncmp(separate_soft, "19867", sizeof("19867")))) {
        ALOGI("SDM8250(19821) G5/G6 set goodix iconlocation to 506 (pixel:540 1894) (all: 1080 2400), iconsize to 237, set screen type to DD306_SAMSUNG");
        property_set(OPTICAL_ICONLOCATION, "490");
        property_set(OPTICAL_ICONSIZE, "237");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "546::1909");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        property_set(PROPERTY_SCREEN_TYPE, "DD306_SAMSUNG");
        if (check_fp_type("G_OPTICAL_19821_G5")) {
            property_set(OPTICAL_CIRCLENUMBER, "0");
        } else {
            property_set(OPTICAL_CIRCLENUMBER, "3");
        }
        //property_set(OPTICAL_CIRCLENUMBER, "0");
    } else if ((!strncmp(separate_soft, "19805", sizeof("19805")))
                ||(!strncmp(separate_soft, "20806", sizeof("20806")))
                ||(!strncmp(separate_soft, "20808", sizeof("20808")))
                ||(!strncmp(separate_soft, "20807", sizeof("20807")))
                ||(!strncmp(separate_soft, "20809", sizeof("20809")))
                ||(!strncmp(separate_soft, "2080E", sizeof("2080E")))
                ||(!strncmp(separate_soft, "20819", sizeof("20819")))) {
        ALOGI("SDM8250(19805) G6 3.0/7.0 set goodix iconlocation to 503 (pixel:540 1894) (all: 1080 2400), iconsize to 190, set screen type to DD306_SAMSUNG");
        property_set(OPTICAL_ICONLOCATION, "503");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "540::1897");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        property_set(PROPERTY_SCREEN_TYPE, "DD306_SAMSUNG");
        if (check_fp_type("G_OPTICAL_19805_G6_7")) {
            property_set(OPTICAL_CIRCLENUMBER, "2");
        } else {
            property_set(OPTICAL_CIRCLENUMBER, "3");
        }
    } else if ((!strncmp(separate_soft, "19811", sizeof("19811")))
                ||(!strncmp(separate_soft, "19813", sizeof("19813")))
                ||(!strncmp(separate_soft, "19835", sizeof("19835")))
                ||(!strncmp(separate_soft, "19837", sizeof("19837")))) {
        ALOGI("SDM8250(19811) G5/G6 set goodix iconlocation to 618(pixel:720 2550) (all: 1400 3168), iconsize to 303, set screen type to DD306_SAMSUNG");
        property_set(OPTICAL_ICONLOCATION, "618");
        property_set(OPTICAL_ICONSIZE, "303");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "726::2549");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        property_set(PROPERTY_SCREEN_TYPE, "SAMSUNG");
        if (check_fp_type("G_OPTICAL_19811_G5")) {
            property_set(OPTICAL_CIRCLENUMBER, "0");
        } else {
            property_set(OPTICAL_CIRCLENUMBER, "3");
        }
        //property_set(OPTICAL_CIRCLENUMBER, "0");
    } else if ((!strncmp(separate_soft, "20828", sizeof("20828")))
                ||(!strncmp(separate_soft, "20838", sizeof("20838")))) {
        ALOGI("20828/20838 G6 7.0/G6 3.0, set goodix iconlocation to 503 (pixel:540 1894) (all: 1080 2400), iconsize to 190, set screen type to DD306_SAMSUNG");
        property_set(OPTICAL_ICONLOCATION, "503");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "540::1897");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        property_set(PROPERTY_SCREEN_TYPE, "DD306_SAMSUNG");
        property_set(OPTICAL_CIRCLENUMBER, "2");
    } else if ((!strncmp(separate_soft, "19161", sizeof("19161")))
                ||(!strncmp(separate_soft, "19162", sizeof("19162")))) {
        ALOGI("SDM8250(19161/19162) G5/G6 set goodix iconlocation to 506 (pixel:540 1894) (all: 1080 2400), iconsize to 237, set screen type to DD306_SAMSUNG");
        property_set(OPTICAL_ICONLOCATION, "506");
        property_set(OPTICAL_ICONSIZE, "237");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_SENSORLOCATION, "546::1893");
        property_set(OPTICAL_SENSORROTATION, "83.5");
        property_set(PROPERTY_SCREEN_TYPE, "DD306_SAMSUNG");
        if (check_fp_type("G_OPTICAL_G5")) {
            property_set(OPTICAL_CIRCLENUMBER, "0");
        } else {
            property_set(OPTICAL_CIRCLENUMBER, "2");
        }
    } else if ((!strncmp(separate_soft, "19166", sizeof("19166")))
                ||(!strncmp(separate_soft, "19165", sizeof("19165")))) {
        ALOGI("MT6885 G3 set goodix iconlocation to 215 (pixel:215 *) (all: * *), iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "215");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "19795", sizeof("19795")))
            ||(!strncmp(separate_soft, "20607", sizeof("20607")))
            ||(!strncmp(separate_soft, "19797", sizeof("19797")))
            ||(!strncmp(separate_soft, "19705", sizeof("19705")))
            ||(!strncmp(separate_soft, "19706", sizeof("19706")))) {
        ALOGI("sm8250 19795 set goodix iconlocation to 234 , iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "234");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
        } else if ((!strncmp(separate_soft, "20135", sizeof("20135")))
                    ||(!strncmp(separate_soft, "20235", sizeof("20235")))
                    ||(!strncmp(separate_soft, "20351", sizeof("20351")))
                    ||(!strncmp(separate_soft, "20352", sizeof("20352")))
                    ||(!strncmp(separate_soft, "20161", sizeof("20161")))
                    ||(!strncmp(separate_soft, "20163", sizeof("20163")))
                    ||(!strncmp(separate_soft, "20139", sizeof("20139")))) {
        ALOGI("sm8250 20135 20235 20139 set goodix iconlocation to 193 , iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "193");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "7");
        } else if ((!strncmp(separate_soft, "20261", sizeof("20261")))
                    ||(!strncmp(separate_soft, "20262", sizeof("20262")))
                    ||(!strncmp(separate_soft, "20263", sizeof("20263")))) {
        ALOGI("SDM7125 G3S set goodix iconlocation to 225 (pixel:540 *) (all: 1080 2400)), iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "225");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "20131", sizeof("20131")))
                    ||(!strncmp(separate_soft, "20133", sizeof("20133")))
                    ||(!strncmp(separate_soft, "20255", sizeof("20255")))
                    ||(!strncmp(separate_soft, "20257", sizeof("20257")))) {
        ALOGI("MT6889 G3S set goodix iconlocation to 193 (pixel:540 *) (all: 1080 2400)), iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "193");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "7");
    } else if ((!strncmp(separate_soft, "20171", sizeof("20171")))
                    ||(!strncmp(separate_soft, "20172", sizeof("20172")))
                    ||(!strncmp(separate_soft, "20353", sizeof("20353")))) {
        ALOGI("MT6891 G3S set goodix iconlocation to 193 (pixel:540 *) (all: 1080 2400)), iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "193");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "7");
    } else if ((!strncmp(separate_soft, "20817", sizeof("20817")))
                    ||(!strncmp(separate_soft, "20831", sizeof("20831")))
                    ||(!strncmp(separate_soft, "20827", sizeof("20827")))) {
        ALOGI("MT6893 G3S set goodix iconlocation to 224 (pixel:540 *) (all: 1080 2400)), iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "224");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "20031", sizeof("20031")))
                    ||(!strncmp(separate_soft, "20311", sizeof("20311")))
                    ||(!strncmp(separate_soft, "20061", sizeof("20061")))
                    ||(!strncmp(separate_soft, "20062", sizeof("20062")))
                    ||(!strncmp(separate_soft, "20857", sizeof("20857")))
                    ||(!strncmp(separate_soft, "20858", sizeof("20858")))
                    ||(!strncmp(separate_soft, "20859", sizeof("20859")))
                    ||(!strncmp(separate_soft, "2085A", sizeof("2085A")))
                    ||(!strncmp(separate_soft, "19815", sizeof("19815")))) {
        ALOGI("G3S set goodix iconlocation to 272 (all: 3216 1440)), iconsize to 248 ");
        property_set(OPTICAL_ICONLOCATION, "272");
        property_set(OPTICAL_ICONSIZE, "248");
        property_set(OPTICAL_ICONNUMBER, "7");
    } else if ((!strncmp(separate_soft, "19825", sizeof("19825")))
                    ||(!strncmp(separate_soft, "20851", sizeof("20851")))
                    ||(!strncmp(separate_soft, "20852", sizeof("20852")))
                    ||(!strncmp(separate_soft, "20853", sizeof("20853")))
                    ||(!strncmp(separate_soft, "20854", sizeof("20854")))) {
        ALOGI("G3S set goodix iconlocation to 194 (all: 2206 540)), iconsize to 192 ");
        property_set(OPTICAL_ICONLOCATION, "194");
        property_set(OPTICAL_ICONSIZE, "192");
        property_set(OPTICAL_ICONNUMBER, "7");
    } else if ((!strncmp(separate_soft, "20291", sizeof("20291")))
            ||(!strncmp(separate_soft, "20292", sizeof("20292")))
            ||(!strncmp(separate_soft, "20293", sizeof("20293")))
            ||(!strncmp(separate_soft, "20294", sizeof("20294")))
            ||(!strncmp(separate_soft, "20295", sizeof("20295")))) {
        ALOGI("MT6779 G3s set goodix iconlocation to 210 (pixel:210 *) (all: * *), iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "210");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "20075", sizeof("20075")))
                ||(!strncmp(separate_soft, "20076", sizeof("20076")))) {
        ALOGI("MT6853 G3 set goodix iconlocation to 210 (pixel:210 *) (all: * *), iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "210");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "206B1", sizeof("206B1")))
            ||(!strncmp(separate_soft, "206B2", sizeof("206B2")))
            ||(!strncmp(separate_soft, "206B3", sizeof("206B3")))
            ||(!strncmp(separate_soft, "206B5", sizeof("206B5")))) {
            ALOGI("sm7150 %s set goodix iconlocation to 215 , iconsize to 194", separate_soft);
            property_set(OPTICAL_ICONLOCATION, "215");
            property_set(OPTICAL_ICONSIZE, "194");
            property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "20627", sizeof("20627")))
            ||(!strncmp(separate_soft, "20664", sizeof("20664")))) {
        ALOGI("SM8350 20627 20664 G3S set goodix iconlocation to 225 , iconsize to 193 ");
        property_set(OPTICAL_ICONLOCATION, "225");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else if ((!strncmp(separate_soft, "20659", sizeof("20659")))
            ||(!strncmp(separate_soft, "20755", sizeof("20755")))) {
        ALOGI("sm8350 20659 20755 set goodix iconlocation to 193 , iconsize to 190 ");
        property_set(OPTICAL_ICONLOCATION, "193");
        property_set(OPTICAL_ICONSIZE, "190");
        property_set(OPTICAL_ICONNUMBER, "7");
    } else if ((!strncmp(separate_soft, "21005", sizeof("21005")))
            ||(!strncmp(separate_soft, "21205", sizeof("21205")))) {
        ALOGI("sm8350s G7 set goodix iconlocation to 579 (all: 2400 1080)), iconsize to 174 ");
        property_set(OPTICAL_ICONLOCATION, "579");
        property_set(OPTICAL_ICONSIZE, "174");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_CIRCLENUMBER, "5");
    } else if ((!strncmp(separate_soft, "21001", sizeof("21001")))
            || (!strncmp(separate_soft, "21201", sizeof("21201")))
            || (!strncmp(separate_soft, "20846", sizeof("20846")))
            || (!strncmp(separate_soft, "20847", sizeof("20847")))
            || (!strncmp(separate_soft, "2084A", sizeof("2084A")))
            || (!strncmp(separate_soft, "21631", sizeof("21631")))
            || (!strncmp(separate_soft, "216AC", sizeof("216AC")))) {
        ALOGI("sm8450 G7 21001 21201 20846 20847 2084A 21631 216AC set goodix iconlocation to 753 (all: 3216*1440) , iconsize to 228 ");
        property_set(OPTICAL_ICONLOCATION, "753");
        property_set(OPTICAL_ICONSIZE, "228");
        property_set(OPTICAL_ICONNUMBER, "7");
        property_set(OPTICAL_CIRCLENUMBER, "6");
    } else {
        ALOGI("do nothing");
    }
}

void set_silead_icon_property() {
    char separate_soft[PROPERTY_VALUE_MAX] = {0};
    property_get(OPTICAL_SEPARATE_SOFTFEATURE, separate_soft, "0");

    if ((!strncmp(separate_soft, "18115", sizeof("18115"))) || (!strncmp(separate_soft, "18501", sizeof("18501"))) ||
        (!strncmp(separate_soft, "18119", sizeof("18119"))) ||(!strncmp(separate_soft, "18503", sizeof("18503")))) {
        ALOGI("SDM855 set silead iconlocation to 305 (pixel:540 2035) (all: 1080 2340), iconsize to 184 ");
        property_set(OPTICAL_ICONLOCATION, "305");
        property_set(OPTICAL_ICONSIZE, "184");
    } else if ((!strncmp(separate_soft, "18073", sizeof("18073")))
                || (!strncmp(separate_soft, "18593", sizeof("18593")))
                || (!strncmp(separate_soft, "18075", sizeof("18075")))) {
        ALOGI("MT6779 set silead iconlocation to 278 (pixel:540 2035) (all: 1080 2340), iconsize to 174, iconnumber to 0 ");
        property_set(OPTICAL_ICONLOCATION, "278");
        property_set(OPTICAL_ICONSIZE, "174");
        property_set(OPTICAL_ICONNUMBER, "2");
    } else if ((!strncmp(separate_soft, "18081", sizeof("18081"))) || (!strncmp(separate_soft, "18085", sizeof("18085"))) ||
        (!strncmp(separate_soft, "18086", sizeof("18086")))  || (!strncmp(separate_soft, "18181", sizeof("18181"))) ||
        (!strncmp(separate_soft, "18381", sizeof("18381"))) || (!strncmp(separate_soft, "18385", sizeof("18385")))) {
        ALOGI("18081/18081/18085/18086/18381/18385 P set silead iconlocation to 278 iconsieze 175 iconlocation iconnumber to 0 ");
        property_set(OPTICAL_ICONLOCATION, "278");
        property_set(OPTICAL_ICONSIZE, "175");
        property_set(OPTICAL_ICONNUMBER, "0");
    } else if ((!strncmp(separate_soft, "19011", sizeof("19011")))
                || (!strncmp(separate_soft, "19301", sizeof("19301")))
                || (!strncmp(separate_soft, "19305", sizeof("19305")))
                || (!strncmp(separate_soft, "19531", sizeof("19531")))
                || (!strncmp(separate_soft, "19532", sizeof("19532")))
                || (!strncmp(separate_soft, "19533", sizeof("19533")))) {
        ALOGI("MT6779(p90M) set silead iconlocation to 273 (pixel:540 2035) (all: 1080 2340), iconsize to 186, iconnumber to 4 ");
        property_set(OPTICAL_ICONLOCATION, "273");
        property_set(OPTICAL_ICONSIZE, "186");
        property_set(OPTICAL_ICONNUMBER, "4");
    } else {
        ALOGI("do nothing");
    }
}

void set_egis_icon_property() {
    char separate_soft[PROPERTY_VALUE_MAX] = {0};
    property_get(OPTICAL_SEPARATE_SOFTFEATURE, separate_soft, "0");

    if  ((!strncmp(separate_soft, "19537", sizeof("19537")))
        || (!strncmp(separate_soft, "19538", sizeof("19538")))
        || (!strncmp(separate_soft, "19539", sizeof("19539")))
        || (!strncmp(separate_soft, "19541", sizeof("19541")))
        || (!strncmp(separate_soft, "19536", sizeof("19536")))) {
        ALOGI("P90 set egis iconlocation to 305 (pixel:540 2035) (all: 1080 2340), iconsize to 193");
        property_set(OPTICAL_ICONLOCATION, "215");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "6");
    } else if ((!strncmp(separate_soft, "19151", sizeof("19151")))
        || (!strncmp(separate_soft, "19350", sizeof("19350")))) {
        ALOGI("P70 set egis iconlocation to 305 (pixel:540 2035) (all: 1080 2340), iconsize to 193");
        property_set(OPTICAL_ICONLOCATION, "215");
        property_set(OPTICAL_ICONSIZE, "193");
        property_set(OPTICAL_ICONNUMBER, "8");
    } else {
        ALOGI("do nothing");
    }
}


