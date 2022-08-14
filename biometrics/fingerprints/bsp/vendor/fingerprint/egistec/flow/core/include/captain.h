#ifndef __CAPTAIN_H__
#define __CAPTAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "struct_def.h"

typedef void (*event_callbck_t)(int event_id, int first_param, int second_param,
				unsigned char *data, int data_size);

int cpt_initialize(unsigned char *in_data, unsigned int in_data_len);
int cpt_uninitialize();
int cpt_cancel();
int cpt_pause(void);
int cpt_continue(void);
int cpt_clean_up(void);
int cpt_set_locker_mode(void);

int cpt_set_active_group(unsigned int user_id, const char *data_path);
int cpt_set_data_path(unsigned int data_type, const char *data_path,
		      unsigned int path_len);

int cpt_chk_secure_id(unsigned int user_id, unsigned long long secure_id);
int cpt_pre_enroll(fingerprint_enroll_info_t enroll_info);
int cpt_enroll();
int cpt_post_enroll();
int cpt_chk_auth_token(unsigned char *token, unsigned int len);

int cpt_get_authenticator_id(unsigned long long *id);
int cpt_authenticate(fingerprint_verify_info_t verify_info);
int cpt_remove_fingerprint(fingerprint_remove_info_t remove_info);
int cpt_get_fingerprint_ids(unsigned int user_id, fingerprint_ids_t *fps);

void cpt_set_event_callback(event_callbck_t on_event_callback);
int cpt_navigation();
int cpt_extra_api(int type, unsigned char *in_buffer, int in_buffer_size,
		  unsigned char *out_buffer, int *out_buffer_size);
int cpt_get_screen_state(int * screen_state);
int cpt_keymode_enable(unsigned int enable);

#ifdef __cplusplus
}
#endif

#endif
