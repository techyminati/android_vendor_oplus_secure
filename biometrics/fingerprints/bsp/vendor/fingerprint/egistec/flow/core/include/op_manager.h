#ifndef __OPMANAGER_H__
#define __OPMANAGER_H__

#include "struct_def.h"
#include "common_definition.h"

int opm_initialize_sdk(unsigned char* in_data, unsigned int in_data_size);
int opm_initialize_algo();
int opm_initialize_sensor();
int opm_uninitialize_sdk();
int opm_uninitialize_algo();
int opm_uninitialize_sensor();

int opm_set_active_group(unsigned int user_id, const char* data_path);
int opm_set_data_path(unsigned int data_type, const char* data_path,
		      unsigned int path_len);
int opm_set_work_mode(unsigned int mode);

int opm_calibration(int status, int type, unsigned char* calibration_data,
		    int data_size);

int opm_get_fingerprint_ids(unsigned int user_id, fingerprint_ids_t* fps);
int opm_remove_fingerprint(fingerprint_remove_info_t remove_info);

int opm_open_spi();
int opm_close_spi();
int opm_get_image(int opt_type, unsigned int* quality);
int opm_check_finger_lost(int timeout, unsigned int* status);

int opm_get_enrolled_count(unsigned int* count);
int opm_enroll_initialize();
int opm_do_enroll(cmd_enrollresult_t * enroll_result, int enroll_option, int is_identify_count);
int opm_save_enrolled_fingerprint(fingerprint_enroll_info_t enroll_info);
int opm_enroll_uninitialize();

int opm_chk_secure_id(unsigned int user_id, unsigned long long secure_id);
int opm_chk_auth_token(unsigned char* token, unsigned int len);
int opm_get_authenticator_id(unsigned long long* id);
int opm_identify_start(unsigned int need_liveness_authentication);
int opm_identify(fingerprint_verify_info_t* verify_info, unsigned int* match_id,
		 unsigned int* status, unsigned char* out_token,
		 unsigned int* out_token_size);
int opm_identify_template_update(unsigned char* is_update);
int opm_identify_template_save();
int opm_identify_finish();
int opm_get_navi_event(unsigned char* out_data, int* out_data_size);

int opm_set_data(int file_type, unsigned char* data, int data_size);
int opm_get_data(int file_type, unsigned char* in_data, int in_data_size,
		 unsigned char* out_data, int* out_data_size);

int opm_extra_command(int type, unsigned char* in_data, int in_data_size,
		      unsigned char* out_buffer, int* out_buffer_size);
int opm_general_command(int type, int cmd, unsigned char* in_data,
			int in_data_size, unsigned char* out_buffer,
			int* out_buffer_size);
int opm_navi_control(int type, unsigned char *in_data, int in_data_size,
	unsigned char *out_buffer, int *out_buffer_size);

#endif
