#ifndef __PLAT_RESET__
#define __PLAT_RESET__

int eg_ta_sensor_reset_high(void);
int eg_ta_sensor_reset_low(void);
int eg_ta_sensor_cs1_low(void);
int eg_ta_sensor_cs1_high(void);
int eg_ta_sensor_cs2_high(void);
int eg_ta_sensor_cs2_low(void);
int eg_ta_sensor_cs1_switch_to_spi(void);

#endif
