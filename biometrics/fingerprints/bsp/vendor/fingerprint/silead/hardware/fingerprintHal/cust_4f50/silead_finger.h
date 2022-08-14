#ifndef __SILEAD_FINGER_H__
#define __SILEAD_FINGER_H__

#ifdef __cplusplus
extern "C" {
#endif

uint64_t silfp_finger_pre_enroll();
int silfp_finger_enroll(const hw_auth_token_t *hat, uint32_t gid, uint32_t timeout_sec);
int silfp_finger_post_enroll();
int silfp_finger_authenticate(uint64_t operation_id, uint32_t gid);
int silfp_finger_authenticate_ext(uint64_t operation_id, uint32_t gid, uint32_t is_pay);
uint64_t silfp_finger_get_auth_id();
int silfp_finger_cancel();
int silfp_finger_remove(uint32_t gid, uint32_t fid);
int silfp_finger_enumerate6(fingerprint_finger_id_t *results, uint32_t *max_size);
int silfp_finger_enumerate();
int silfp_finger_set_active_group(uint32_t gid, const char *store_path);
int silfp_finger_set_notify_callback(fingerprint_notify_t notify);

int silfp_finger_init();
int silfp_finger_deinit();

void silfp_finger_set_dump_path(const char *path);
void silfp_finger_set_cal_path(const char *path);
void silfp_finger_set_ta_name(const char *taname);
void silfp_finger_capture_disable(void);
void silfp_finger_set_key_mode(uint32_t enable);
void silfp_finger_opt_notify(int32_t cmd_id);
#ifdef __cplusplus
}
#endif

#endif // __SILEAD_FINGER_H__