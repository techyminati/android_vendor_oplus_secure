#include "egis_rbs_api.h"
#include <string.h>
#include "captain.h"
#include "common_definition.h"
#include "constant_def.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "response_def.h"
#include "struct_def.h"
#include "version.h"

#define COMMANDER_VERSION "1.0.1"
#define MAX_PATH_LEN 256

int rbs_initialize(unsigned char* in_data, unsigned int in_data_len) {
    ex_log(LOG_INFO, "#*rbs_initialize#*#*[enter]#*");
    ex_log(LOG_INFO, "GIT_SHA1 = OPLUS_0927_1-16-726-10_R_dbe8592");

    extern app_instance_type_t g_app_type;
    ex_log(LOG_DEBUG, "#*rbs_app_instance type %d #*", g_app_type);
    int retval = cpt_initialize(in_data, in_data_len);

    ex_log(LOG_INFO, "#*rbs_initialize#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_uninitialize() {
    ex_log(LOG_INFO, "#*rbs_uninitialize#*#*[enter]#*");

    int retval = cpt_uninitialize();

    ex_log(LOG_INFO, "#*rbs_uninitialize#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_cancel() {
    ex_log(LOG_INFO, "#*rbs_cancel#*#*[enter]#*");

    int retval = cpt_cancel();

    ex_log(LOG_INFO, "#*rbs_cancel#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_active_user_group(unsigned int user_id, const char* data_path) {
    ex_log(LOG_INFO, "#*rbs_active_user_group#*#*[enter]#*");

    if (NULL == data_path) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    int retval = cpt_set_active_group(user_id, data_path);

    ex_log(LOG_INFO, "#*rbs_active_user_group#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_set_data_path(unsigned int data_type, const char* data_path, unsigned int path_len) {
    ex_log(LOG_INFO, "#*rbs_set_data_path#*#*[enter]#*");

    if (NULL == data_path || path_len == 0 || path_len > MAX_PATH_LEN) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    int retval = cpt_set_data_path(data_type, data_path, path_len);

    ex_log(LOG_INFO, "#*rbs_set_data_path#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_chk_secure_id(unsigned int user_id, unsigned long long sid) {
    ex_log(LOG_INFO, "#*rbs_chk_secure_id#*#*[enter]#*");

    int retval = cpt_chk_secure_id(user_id, sid);

    ex_log(LOG_INFO, "#*rbs_chk_secure_id#*#*[leave] %d , %d %llu #*", retval, user_id, sid);
    return retval;
}

int rbs_chk_auth_token(unsigned char* token, unsigned int len) {
    ex_log(LOG_INFO, "#*rbs_chk_auth_token#*#*[enter]#*");

    if (NULL == token) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    int retval = cpt_chk_auth_token(token, len);

    ex_log(LOG_INFO, "#*rbs_chk_auth_token#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_pre_enroll(unsigned int user_id, unsigned int finger_id) {
    ex_log(LOG_INFO, "#*rbs_pre_enroll#*#*[enter]#*");
    ex_log(LOG_INFO, "commander_version: %s", COMMANDER_VERSION);
    ex_log(LOG_INFO, "GIT_SHA1 = OPLUS_0927_1-16-726-10_R_dbe8592");

    fingerprint_enroll_info_t enroll_info;
    enroll_info.fingerprint_info.user_id = user_id;
    enroll_info.fingerprint_info.fingerprint_id = finger_id;

    int retval = cpt_pre_enroll(enroll_info);

    ex_log(LOG_INFO, "#*rbs_pre_enroll#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_enroll() {
    ex_log(LOG_INFO, "#*rbs_enroll#*#*[enter]#*");

    int retval = cpt_enroll();

    ex_log(LOG_INFO, "#*rbs_enroll#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_post_enroll() {
    ex_log(LOG_INFO, "#*rbs_post_enroll#*#*[enter]#*");

    int retval = cpt_post_enroll();

    ex_log(LOG_INFO, "#*rbs_post_enroll#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_get_authenticator_id(unsigned long long* id) {
    ex_log(LOG_INFO, "#*rbs_get_authenticator_id#*#*[enter]#*");

    if (NULL == id) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    int retval = cpt_get_authenticator_id(id);

    ex_log(LOG_INFO, "#*rbs_get_authenticator_id#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_authenticator(unsigned int user_id, unsigned int* finger_ids, unsigned int finger_count,
                      unsigned long long challenge) {
    ex_log(LOG_INFO, "#*rbs_authenticator#*#*[enter]#*");
    ex_log(LOG_INFO, "GIT_SHA1 = OPLUS_0927_1-16-726-10_R_dbe8592");

    fingerprint_verify_info_t verify_info;

    if (finger_count > __SINGLE_UPPER_LIMITS__) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    verify_info.user_id = user_id;
    verify_info.fingerprints.fingerprint_ids_count = 0;
    verify_info.accuracy_level = LEVEL_1;
    verify_info.challenge = challenge;

    if (finger_ids) {
        mem_move(verify_info.fingerprints.fingerprint_ids, finger_ids,
                 sizeof(unsigned int) * finger_count);
        verify_info.fingerprints.fingerprint_ids_count = finger_count;
    }

    int retval = cpt_authenticate(verify_info);

    ex_log(LOG_INFO, "#*rbs_authenticator#*#*[leave] %d#*", retval);

    return retval;
}

int rbs_remove_fingerprint(unsigned int user_id, unsigned int finger_id) {
    ex_log(LOG_INFO, "#*rbs_remove_fingerprint#*#*[enter]#*");

    fingerprint_remove_info_t remove_info;
    remove_info.fingerprint_info.user_id = user_id;
    remove_info.fingerprint_info.fingerprint_id = finger_id;

    int retval = cpt_remove_fingerprint(remove_info);

    ex_log(LOG_INFO, "#*rbs_remove_fingerprint#*#*[leave] %d#*", retval);
    return retval;
}

int rbs_get_fingerprint_ids(unsigned int user_id, int* finger_ids, int* finger_count) {
    ex_log(LOG_INFO, "#*rbs_get_fingerprint_ids#*#*[enter]#*");
    fingerprint_ids_t fps;

    if (NULL == finger_ids || NULL == finger_count || *finger_count > __SINGLE_UPPER_LIMITS__) {
        return FINGERPRINT_RES_INVALID_PARAM;
    }

    fps.fingerprint_ids_count = *finger_count;
    mem_move(fps.fingerprint_ids, finger_ids, sizeof(int) * fps.fingerprint_ids_count);

    int retval = cpt_get_fingerprint_ids(user_id, &fps);

    if (FINGERPRINT_RES_SUCCESS == retval) {
        *finger_count = fps.fingerprint_ids_count;
        mem_move(finger_ids, fps.fingerprint_ids, fps.fingerprint_ids_count * sizeof(unsigned int));
    }

    ex_log(LOG_INFO, "#*rbs_get_fingerprint_ids#*#*[leave] %d#*", retval);
    return retval;
}

void rbs_set_on_callback_proc(operation_callback_t operation_callback) {
    ex_log(LOG_INFO, "#*rbs_set_on_callback_proc#*#*[enter]#*\n");

    if (NULL == operation_callback) {
        return;
    }

    cpt_set_event_callback(operation_callback);

    ex_log(LOG_INFO, "#*rbs_set_on_callback_proc#*#*[leave] #*\n");
}

int rbs_extra_api(int type, unsigned char* in_buffer, int in_buffer_size, unsigned char* out_buffer,
                  int* out_buffer_size) {
    ex_log(LOG_INFO, "#*rbs_extra_api#*#*[enter]#*");

    int retval = cpt_extra_api(type, in_buffer, in_buffer_size, out_buffer, out_buffer_size);

    if (type == 10) {
         engineer_info_t info[3] = {0};
         //void notify(int event_id, int first_param, int second_param, unsigned char* data, int data_size)
         notify(EVENT_SENSOR_OPTICAL_SELF_TEST, retval, 0, (unsigned char*)info, 3*sizeof(engineer_info_t));
         ex_log(LOG_DEBUG, "selftest   END_CALL_API");
     }

    ex_log(LOG_INFO, "#*rbs_extra_api#*#*[leave] %d#*", retval);

    return retval;
}

int rbs_pause(void) {
    ex_log(LOG_INFO, "#*rbs_pause#*#*[enter]#*");

    int retval = cpt_pause();

    ex_log(LOG_INFO, "#*rbs_pause#*#*[leave] %d#*", retval);

    return retval;
}

int rbs_continue(void) {
    ex_log(LOG_INFO, "#*rbs_continue#*#*[enter]#*");

    int retval = cpt_continue();

    ex_log(LOG_INFO, "#*rbs_continue#*#*[leave] %d#*", retval);

    return retval;
}

int rbs_set_app_type(unsigned int type) {
    extern app_instance_type_t g_app_type;
    g_app_type = type;
    return 0;
}
