#ifndef FINGERPRINT_CONFIG_H
#define FINGERPRINT_CONFIG_H

typedef enum lcdType {
    LCD_BOE,
    LCD_SAMSUNG,
    LCD_LG,
    LCD_TM,
    LCD_TM2,
    LCD_DEFAULT
} lcd_type;

enum error_code {
    ERROR_OK,
    ERROR_PARA,
    ERROR_INVALID_CMD
};

typedef enum FpCmd {
    FP_CMD_GET_BRIGHTNESS,
    FP_CMD_GET_LCD_TYPE,
    FP_CMD_GET_HBM_DELAY,
    FP_CMD_GET_BRIGHT_DELAY,
    FP_CMD_GET_TEST,
    FP_CMD_MAX
} fp_cmd_t;

typedef struct FpTransforInfo {
    fp_cmd_t cmd;
    int response;
    void *s_addr;
    unsigned int source_size;
    void *r_addr;
    unsigned int response_size;
} fpTransforInfo;

#endif // FINGERPRINT_CONFIG_h
