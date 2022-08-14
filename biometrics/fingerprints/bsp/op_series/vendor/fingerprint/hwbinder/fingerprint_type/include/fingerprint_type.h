#ifndef FINGERPRINT_TYPE_H
#define FINGERPRINT_TYPE_H

#if defined(__cplusplus)
extern "C" {
#endif
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define FP_ID_MAX_LENGTH                60
#define FP_TA_PATH_MAX_LENGTH           100

#define FPC_SENSOR_TYPE_PREFIX_LENGTH   3
#define PROPERTY_GDX_FP                 "oplus.goodix.fp"
#define FP_ID_PATH                      "/proc/fp_id"
#define FP_TA_PATH                      "/odm/vendor/firmware"
#define LCD_TYPE_PATH                   "/proc/devinfo/lcd"
#define OPTICAL_SEPARATE_SOFTFEATURE  "ro.separate.soft"
#define OPLUS_TA_LOAD_RETRY_TIMES        (5)

typedef enum {
    FP_GOODIX,
    FP_FPC,
    FP_OPTICAL_GOODIX_G2,
    FP_OPTICAL_GOODIX_G3,
    FP_OPTICAL_GOODIX_G3S,
    FP_OPTICAL_GOODIX_G5,
    FP_OPTICAL_GOODIX_G6,
    FP_OPTICAL_GOODIX_G7,
    FP_OPTICAL_GOODIX_19821_G5,
    FP_OPTICAL_GOODIX_19821_G6,
    FP_OPTICAL_GOODIX_19805_G6_3,
    FP_OPTICAL_GOODIX_19805_G6_7,
    FP_OPTICAL_GOODIX_19811_G5,
    FP_OPTICAL_GOODIX_19811_G6,
    FP_OPTICAL_GOODIX_20801_G3,
    FP_OPTICAL_GOODIX_G6_7,
    FP_OPTICAL_GOODIX_G6_3,
    FP_OPTICAL_SILEAD,
    FP_EGIS,
    FP_OPTICAL_EGIS,
    FP_UNKNOWN,
} fp_facetory_type_t;

enum sensor_id_t {
    E_F_1140,
    E_F_1260,
    E_F_1022,
    E_F_1023,
    E_F_1023_GLASS,
    E_F_1511,
    E_F_1027,
    E_G_3268,
    E_G_5288,
    E_G_5298,
    E_G_5298_GLASS,
    E_G_5658,
    E_G_5228,
    E_G_OPTICAL_G2,
    E_G_OPTICAL_G3,
    E_G_OPTICAL_G3S,
    E_G_OPTICAL_G5,
    E_G_OPTICAL_G6,
    E_S_OPTICAL_70,
    E_S_Truly0,
    E_S_Ofilm0,
    E_S_Fingerchip0,
    E_S_Truly1,
    E_S_Ofilm1,
    E_S_Fingerchip1,
    E_E_OPTICAL_ET713,
    E_FP_GOODIX_3626,
    E_G_3626,
    E_F_1541,
    E_E_520,
    E_G_OPTICAL_G7, //30
    E_19821_G5,
    E_19821_G6,
    E_19805_G6_3,
    E_19805_G6_7,
    E_19811_G5,
    E_19811_G6,
    E_20828_G6_7,
    E_20838_G6_3,
    E_G_OPTICAL_20801_G3,
    E_SENSOR_ID_MAX,
};


typedef struct fp_config_info {
    uint32_t sensor_id;
    char fp_id_string[FP_ID_MAX_LENGTH];
    char ta_qsee_name[FP_ID_MAX_LENGTH];
    char ta_tbase_path[FP_TA_PATH_MAX_LENGTH];
    fp_facetory_type_t fp_factory_type;
    uint32_t fp_ic_type;
    uint32_t total_enroll_times ;
//need by fpc
    uint32_t fp_image_quality_pass_score;
    uint32_t fp_image_size;
    uint32_t fp_threshold_count[2];
    bool fp_up_irq;
    uint32_t fp_type;
}fp_config_info_t;

extern fp_config_info_t fp_config_info_init;
extern fp_config_info_t fp_config_info[];

uint32_t oplus_fp_type_init();
char* read_proc_string_data(const char *path, char* buf);

void set_goodix_icon_property();
void set_silead_icon_property();
void set_egis_icon_property();
void set_sensor_type_feature(uint32_t sensor_type);

#if defined(__cplusplus)
}  // extern "C"
#endif
#endif  // FPC_HIDL_H
