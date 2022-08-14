#ifndef __SILEAD_FINGER_CUST_H__
#define __SILEAD_FINGER_CUST_H__

#ifdef __cplusplus
extern "C" {
#endif

int silfp_get_enroll_total_times();
int silfp_pause_enroll();
int silfp_continue_enroll();
int silfp_pause_identify();
int silfp_continue_identify();
int silfp_get_alikey_status();
int silfp_cleanup();
int silfp_set_touch_event_listener();
int silfp_set_screen_state(uint32_t sreenstate);
int silfp_dynamically_config_log(uint32_t on);
int silfp_get_engineering_info(uint32_t type);

#ifdef __cplusplus
}
#endif

#endif // __SILEAD_FINGER_CUST_H__