#ifndef __FP_SYS_H__
#define __FP_SYS_H__


int32_t fp_set_tpirq_enable(uint32_t mode);
int32_t notify_finger_ready();
int32_t clear_finger_ready_flag();
int32_t is_finger_ready();
int32_t wait_for_finger_ready();
int32_t fp_read_aod_mode(void);
int32_t fp_set_dim_layer(uint32_t value);
#endif