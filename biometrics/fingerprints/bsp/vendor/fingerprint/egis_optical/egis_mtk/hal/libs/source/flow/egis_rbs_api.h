#ifndef __EGIS_RBS_API_H__
#define __EGIS_RBS_API_H__

typedef enum {
	APP_IS_USING_BINDER = 0,
	APP_IS_USING_JNI,
	APP_IS_BIOMETRIC,
} app_instance_type_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
*	the operation events callback
*/
typedef void (*operation_callback_t)(int event_id, int value1, int value2,
				     unsigned char *buffer, int buffer_size);

/*
**	@rbs_initialize
**	initialize the environment of sdk
**	such as create global variables
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]			successfully initialized
**	[ FINGERPRINT_RES_HW_UNAVALABLE ] 	can't find or use hardware
**  [ FINGERPRINT_RES_ALLOC_FAILED ]	alloc memory failed
**  [ FINGERPRINT_RES_ALGORITHM_ERROR ]	algorithm initialize failed
*/
int rbs_initialize(unsigned char *in_data, unsigned int in_data_len);

/*
**	@rbs_uninitialize
**	release the environment of sdk
**	such as recyle the resources
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]			successfully
*uninitialized
*/
int rbs_uninitialize();

/*
**	@rbs_cancel
**	abort the logic loop, interrupt the enroll or authenticate operation
*which is running
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]			successfully canceled
**  [ FINGERPRINT_RES_NOT_IDLE ]		there is another cancel
*operation
*/
int rbs_cancel();

/*
**	@rbs_active_user_group
**	pre-load userdata & template infos with specific user_id
**	it should be called while effective user is switching in android system
**	@params
**	[ user_id ] the index of a user or a group which contains a fingerprint
*set
**	[ data_path ] the path of userdata & template infos, it should be
*available no matter whether it has trustzone or not
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]			successfully active
*specific user
**  [ FINGERPRINT_RES_NOT_IDLE ]		there is another opration,such as
*enroll, authenticate, etc.
**	[ FINGERPRINT_RES_FAILED ]			failed
*/
int rbs_active_user_group(unsigned int user_id, const char *data_path);

/*
**	@rbs_set_data_path
**	set calibration or userdata path before using
**	@params
**	[ data_type ] the type of @data_path which is going to be set
**  [ data_path ] the pointer of the path string
**	[ data_len ] the length of the @data_path
**  @return
**  [ FINGERPRINT_RES_SUCCESS ]			successfully set data path
**  [ FINGERPRINT_RES_NOT_IDLE ]		there is another opration,such as
*enroll, authenticate, etc.
**  [ FINGERPRINT_RES_INVALID_PARAM ]	parameters may be invalid
**  [ FINGERPRINT_RES_FAILED ]			failed
*/
int rbs_set_data_path(unsigned int data_type, const char *data_path,
		      unsigned int path_len);

/*
**	@rbs_chk_secure_id
**	check the android secure_id(SID)
**	We will save the SID when the the first fingerprint enrolled
*successfully.
**	And we will check the input SID with the saved one
**	before each enrollment process for the following fingerprints.
**	With the result unmatched, we will removed all the enrolled
*fingerprints.
**	@params
**	[ user_id ] the index of a fingerprints set. In AOSP, it means User ID
**	[ secure_id ] In AOSP, it's a attribute of user.
**	@return
**	[ FINGERPRINT_RES_SUCEESS ]			matched
**  [ FINGERPRINT_RES_INVALID_PARAM ]	parameters may be invalid
**	[ FINGERPRINT_RES_FAILED ]			not matched
*/
int rbs_chk_secure_id(unsigned int user_id, unsigned long long secure_id);

/*
**	@rbs_pre_enroll
**	operation before enrollment
**	It is necessary to confirm that the finger_id is available.
**	caller should provider a unique id to named this fingerprint
**	caller should provider a user id to divide the registered fingerprint
*in different groups
**	@params
**	[ user_id ] the index of a fingerprint set. If the user_id isnt exsit,
*rbs will create a new set
**	[ fingerprint_id ] the index of the fingerprint. It should be
*unique.SDK will check again.
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]			successfully
**  [ FINGERPRINT_RES_NOT_IDLE ]		there is another opration,such as
*enroll, authenticate, etc.
**	[ FINGERPRINT_RES_DUPLICATE_ID ]	the same finger id already in
*use
**	[ FINGERPRINT_RES_NO_SPACE ]		fingerprint number limited
**	[ FINGERPRINT_RES_FAILED ]			failed
*/
int rbs_pre_enroll(unsigned int user_id, unsigned int fingerprint_id);

/*
**	@rbs_enroll
**	start a thread to do enrollment operation
**	It will save related template immediately upon finished
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]			enrollment thread was
*successfully started
**  [ FINGERPRINT_RES_NOT_IDLE ]		there is another opration,such as
*enroll, authenticate, etc.
**	[ FINGERPRINT_RES_FAILED ]			failed
*/
int rbs_enroll();

/*
**	@rbs_post_enroll
**	do operation after enrollment
**	It will save the template the SDK got last time
**	No call, no save
**	@return
**	[ FINGERPRINT_RES_SUCCESS]
**	[ FINGERPRINT_RES_OPEN_FILE_FAILED ]
**	[ FINGERPRINT_RES_WRITE_FILE_FAILED ]
**	[ FINGERPRINT_RES_FAILED ]
*/
int rbs_post_enroll();

/*
**	@rbs_chk_auth_token
**	Check the validity of hw_auth_token_t provided by GateKeeper.
**	@param
**	[ token ] sptr of hw_auth_token_t, the buffer should be provided by the
*caller
**	[ len ] the length of the token buffer
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]
**	[ FINGERPRINT_RES_FAILED ]
*/
int rbs_chk_auth_token(unsigned char *token, unsigned int len);

/*
**	@rbs_get_authenticator_id
**	get authenticator id
**	@params
**	[ id ]
**	@return
**	[ FINGERPRINT_RES_SUCCESS]
**  [ FINGERPRINT_RES_INVALID_PARAM ]	parameters may be invalid
**	[ FINGERPRINT_RES_FAILED ]
*/
int rbs_get_authenticator_id(unsigned long long *id);

/*
**	@rbs_authenticator
**	verify fingerprints
**	caller should provider a exsited user id to select a fingerprint group
*in all fingerprints groups
**	caller should provider a sets of finger ids for identified
**	@params
**	[ user_id ]
**	[ fingerprint_ids ]
**	[ fingerprint_count ]
**	[ wait_finger_off ]
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]
**	[ FINGERPRINT_RES_INVALID_PARAM ]
**	[ FINGERPRINT_RES_USER_NOT_EXSIT ]
**	[ FINGERPRINT_RES_FINGERPRINT_NOT_EXSIT]
**	[ FINGERPRINT_RES_FAILED ]
*/
int rbs_authenticator(unsigned int user_id, unsigned int *fingerprint_ids,
		      unsigned int fingerprint_count,
		      unsigned long long challenge);

/*
**	@rbs_remove_finger
**	remove registered fingerprint
**	use user_id to select a fingerprint group
**	use	finger_id to identify a registered fingerprint
**  @params
**	[ user_id ]
**	[ fingerprint_id ]
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]
**	[ FINGERPRINT_RES_USER_NOT_EXSIT ]
**	[ FINGERPRINT_RES_FINGERPRINT_NOT_EXSIT]
**	[ FINGERPRINT_RES_FAILED ]
*/
int rbs_remove_fingerprint(unsigned int user_id, unsigned int fingerprint_id);

/*
**	@rbs_get_fingerprint_ids
**	get all the finger ids in a fingerprint group which devided with user_id
**	caller should provider a exsited user id to select a fingerprint group
*in all fingerprints groups
**	caller should prepare a array(@size MAX_FINGERPRINTS) as finger_ids, to
*contain finger ids
**	@params
**	[ user_id ]
**	[ fingerprint_ids ]
**	[ fingerprint_count ]
**	@return
**	[ FINGERPRINT_RES_SUCCESS ]
**	[ FINGERPRINT_RES_INVALID_PARAM ]
**	[ FINGERPRINT_RES_USER_NOT_EXSIT ]
**	[ FINGERPRINT_RES_FAILED ]
*/
int rbs_get_fingerprint_ids(unsigned int user_id, int *fingerprint_ids,
			    int *fingerprint_count);

/*
**	set the callback function pointer
*/
void rbs_set_on_callback_proc(operation_callback_t operation_callback);

int rbs_extra_api(int type, unsigned char *in_buffer, int in_buffer_size,
		  unsigned char *out_buffer, int *out_buffer_size);

/*
**	for default functional commands
*/

int rbs_pause(void);
int rbs_continue(void);

#ifdef __cplusplus
}
#endif

#endif /* for __EGIS_RBS_API_H__ */
