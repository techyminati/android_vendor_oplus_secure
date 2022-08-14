#include "save_image.h"
#include "captain.h"
#include "fp_definition.h"
#include "object_def_image.h"
#include "opt_file.h"
#include "plat_file.h"
#include "plat_log.h"
#include "plat_mem.h"
#include "response_def.h"
#include "type_def.h"

#ifdef __SDK_SAVE_IMAGE_V2__
#include "save_image_v2.h"
#endif
#define LOG_TAG "RBS-SAVE-IMAGE"

typedef struct {
    int info_version;
    int is_new_finger_on;
    int try_count;
    int match_score;
    int save_index;
} return_image_info_t;

#if defined(__SHOW_LIVEIMAGE__) || defined(__SUPPORT_SAVE_IMAGE__)
static BOOL set_img_state(transfer_image_type_t type, int img_quality, BOOL force_to_good) {
    if (type == TRANSFER_ENROLL_IMAGE || type == TRANSFER_ENROLL_RAW) {
        return TRUE;
    } else if (type == TRANSFER_VERIFY_IMAGE_V2 || type == TRANSFER_VERIFY_RAW) {
        if (img_quality == FP_LIB_ENROLL_SUCCESS || force_to_good == TRUE) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    ex_log(LOG_ERROR, "invalid parameter, type = %d", type);
    return FALSE;
}
#ifdef __ET7XX__
int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good,
                              int match_result) {
    const int buffer_size = sizeof(rbs_obj_array_request_t) + MULTI_MAX_IMAGE_BUFFER_SIZE;
    int out_size;
    int retval = FINGERPRINT_RES_FAILED, i, j;

    if (type == TRANSFER_LIVE_IMAGE) {
        ex_log(LOG_ERROR, "%s Need to implement getting the last image of IMGTYPE_BIN", __func__);
        return FINGERPRINT_RES_FAILED;
    }

    ex_log(LOG_VERBOSE, "%s (%d) buf_size=%d, match_result=%d", __func__, type, buffer_size,
           match_result);
    ex_log(LOG_VERBOSE, "%s, img_quality=%d, force_to_good=%d", __func__, img_quality,
           force_to_good);
    int is_good_img = set_img_state(type, img_quality, force_to_good);
    ex_log(LOG_VERBOSE, "%s, is_good_img=%d", __func__, is_good_img);
    unsigned char* buffer = malloc(buffer_size);
    if (buffer == NULL) {
        return FINGERPRINT_RES_ALLOC_FAILED;
    }
    rbs_obj_array_request_t array_req;
    rbs_obj_array_t* out_obj_array;
    uint32_t obj_id, obj_total_size;
#if defined(__REMOVE_DETECT_IMG__) && defined(__REMOVE_BKG_IMG__)
    int img_type_request[2] = {IMGTYPE_BIN, IMGTYPE_RAW};
#elif !defined(__REMOVE_DETECT_IMG__) && defined(__REMOVE_BKG_IMG__)
    int img_type_request[3] = {IMGTYPE_BIN, IMGTYPE_RAW, IMGTYPE_DEBUG};
#elif defined(__REMOVE_DETECT_IMG__) && !defined(__REMOVE_BKG_IMG__)
    int img_type_request[3] = {IMGTYPE_BIN, IMGTYPE_RAW, IMGTYPE_BKG};
#else
    int img_type_request[4] = {IMGTYPE_BIN, IMGTYPE_RAW, IMGTYPE_BKG, IMGTYPE_DEBUG};
#endif
#ifdef ONLY_REQUEST_IMGTYPE_BKG  // temporary use define to switch
    int start_request_idx = 2;
#else
    int start_request_idx = 0;
#endif
    for (i = 0;; i++) {
        for (j = start_request_idx; j < (int)(sizeof(img_type_request) / sizeof(*img_type_request));
             j++) {
            out_size = buffer_size;
            array_req.param1 = img_type_request[j];
            RBSOBJ_set_ARRAY_REQ_v1_0((&array_req), i, 1, RBSOBJ_ID_IMAGE);
            retval = opt_receive_data(TYPE_RECEIVE_IMAGE_OBJECT_ARRAY, (unsigned char*)&array_req,
                                      sizeof(array_req), buffer, &out_size);
            out_obj_array = (rbs_obj_array_t*)buffer;
            if (out_obj_array->count == 0) {
                ex_log(LOG_ERROR, "OBJECT_ARRAY count is 0");
                break;
            }
            obj_id = RBSOBJ_get_objarray_obj_id(out_obj_array);
            if (obj_id == RBSOBJ_ID_IMAGE) {
                rbs_obj_image_v1_0_t* img_obj =
                    (rbs_obj_image_v1_0_t*)RBSOBJ_get_payload_pointer(out_obj_array);
                if (!is_good_img) {
                    ex_log(LOG_VERBOSE, "bad image [%d][%d]", i, j);
                    RBSOBJ_set_IMAGE_param(img_obj, is_bad, 1);
                }
                obj_total_size = RBSOBJ_get_obj_total_size(out_obj_array);
                ex_log(LOG_INFO, "OBJECT_ARRAY total_size(%d) image %d:%d, bpp=%d", obj_total_size,
                       img_obj->width, img_obj->height, img_obj->bpp);
                notify(EVENT_RETURN_IMAGEOBJ, 0, 0, buffer, obj_total_size);
#ifdef __SDK_SAVE_IMAGE_V2__
                save_image_info_t save_image_info = {0x0};
                save_image_info.img_type = type;
                save_image_info.rbs_obj_array = out_obj_array;
                save_image_info.rbs_obj_image = img_obj;
                save_image_info.is_image_finger_off_0 = FALSE;
                save_image_info.is_image_finger_off_1 = FALSE;

                if (img_quality == FP_IMAGE_FINGER_OFF_0) {
                    if (out_obj_array->has_more_object) {
                        save_image_info.is_image_finger_off_0 = FALSE;
                    } else {
                        save_image_info.is_image_finger_off_0 = TRUE;
                    }
                } else if (img_quality == FP_IMAGE_FINGER_OFF_1) {
                    if (out_obj_array->has_more_object) {
                        save_image_info.is_image_finger_off_1 = FALSE;
                    } else {
                        save_image_info.is_image_finger_off_1 = TRUE;
                    }
                } else if (img_quality == FP_IMAGE_TOO_FAST) {
                    save_image_info.is_image_too_fast = TRUE;
                } else if (img_quality == FP_IMAGE_PARTIAL_FINGER_OFF_1) {
                    save_image_info.is_image_partial_fingeroff1 = TRUE;
                } else if (img_quality == FP_IMAGE_BAD_IMAGE_FINGER_OFF_1) {
                    save_image_info.is_image_bad_image_fingeroff1 = TRUE;
                } else if (img_quality == FP_IMAGE_BAD_IMAGE) {
                    save_image_info.is_image_bad_image = TRUE;
                } else if (img_quality == FP_IMAGE_PARTIAL) {
                    save_image_info.is_image_partial = TRUE;
                }

                if (img_quality == FP_IMAGE_FAIL_CANCEL) {
                    if (out_obj_array->has_more_object) {
                        save_image_info.is_called_cancel = FALSE;
                    } else {
                        save_image_info.is_called_cancel = TRUE;
                    }
                }

                debug_save_image(save_image_info);
#endif
            } else {
                ex_log(LOG_ERROR, "OBJECT_ARRAY unexpected obj_id %d", obj_id);
            }
        }
        if (!out_obj_array->has_more_object) break;
    }
    opt_send_data(TYPE_SEND_RESET_IMAGE_FRAME_COUNT, NULL, 0);

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    return retval;
}
#else

int transfer_frames_to_client(transfer_image_type_t type, int img_quality, BOOL force_to_good,
                              int match_result) {
    const int buffer_size = sizeof(receive_images_out_t) + MULTI_MAX_IMAGE_BUFFER_SIZE;
    int out_size;
    int retval = FINGERPRINT_RES_FAILED, width, height, bpp, i, img_size;
    int type_id;
    return_image_info_t return_image_info = {0};
    BOOL is_good_img = FALSE;
    BOOL is_new_finger_on = TRUE;

    receive_images_in_t in_data;
    receive_images_out_t* out_buffer = NULL;

#ifndef EGIS_DBG
    return FINGERPRINT_RES_FAILED;
#endif

    ex_log(LOG_DEBUG, "%s (buffer size=%d) match_result=%d", __func__, buffer_size, match_result);
    unsigned char* buffer = (unsigned char*)malloc(buffer_size);
    if (buffer == NULL) {
        return retval;
    }

    out_buffer = (receive_images_out_t*)buffer;
    unsigned char* image_buf = buffer + sizeof(receive_images_out_t);

    mem_set(&in_data, 0, sizeof(receive_images_in_t));
    in_data.image_count_request = EGIS_TRANSFER_FRAMES_PER_TIME;
    in_data.image_type = type;
    in_data.reset_mode = FRAMES_RESET_AUTO;

    return_image_info.info_version = 0x01;

    if (type == TRANSFER_LIVE_IMAGE) {
        in_data.reset_mode = FRAMES_RESET_NEVER;
        type_id = TYPE_RECEIVE_LIVE_IMAGE;
    } else {
        type_id = TYPE_RECEIVE_MULTIPLE_IMAGE;
    }

    do {
        out_size = buffer_size;
        retval =
            opt_receive_data(type_id, (unsigned char*)&in_data, sizeof(in_data), buffer, &out_size);
        if (retval == FINGERPRINT_RES_SUCCESS) {
            width = out_buffer->format.width;
            height = out_buffer->format.height;
            bpp = out_buffer->format.bpp;
            img_size = width * height * bpp / 8;

            // call back images
            if (type == TRANSFER_LIVE_IMAGE) {
                notify(EVENT_RETURN_LIVE_IMAGE, width, height, image_buf, width * height);
                break;
            } else if (type == TRANSFER_VERIFY_IMAGE_V2) {
                is_good_img = set_img_state(type, img_quality, force_to_good);

                return_image_info.is_new_finger_on = is_new_finger_on;
                return_image_info.try_count = out_buffer->identify_info.try_count;
                return_image_info.match_score = out_buffer->identify_info.match_score;
                return_image_info.save_index = out_buffer->identify_info.save_index;

                notify(EVENT_RETURN_IMAGE_INFO, out_buffer->image_count_included, is_good_img,
                       (unsigned char*)&return_image_info, sizeof(return_image_info));
                notify(EVENT_RETURN_IMAGE, width, height, image_buf,
                       img_size * out_buffer->image_count_included);
            } else {
                if (out_buffer->image_count_included <= 0) {
                    ex_log(LOG_ERROR, "no output image !");
                }
                for (i = 0; i < out_buffer->image_count_included; i++) {
                    is_good_img = set_img_state(type, img_quality, force_to_good);

                    return_image_info.is_new_finger_on = is_new_finger_on;
                    notify(EVENT_RETURN_IMAGE_INFO, 1, is_good_img,
                           (unsigned char*)&return_image_info, sizeof(return_image_info));
                    notify(EVENT_RETURN_IMAGE, width, height, image_buf + i * img_size, img_size);
                }
            }
            is_new_finger_on = FALSE;

            in_data.image_index_start = out_buffer->image_index_end;
            if (out_buffer->has_more_image == TRANSFER_MORE_NONE) {
                break;
            } else if (out_buffer->has_more_image == TRANSFER_MORE_NEXT_RAW) {
                ex_log(LOG_DEBUG, "to continue to get RAW image");
                in_data.image_index_start = 0;
                if (type == TRANSFER_VERIFY_IMAGE_V2) {
                    in_data.image_type = TRANSFER_VERIFY_RAW;
                } else if (type == TRANSFER_ENROLL_IMAGE) {
                    in_data.image_type = TRANSFER_ENROLL_RAW;
                } else {
                    ex_log(LOG_ERROR, "unexpected type %d", type);
                    break;
                }
            }
        } else {
            ex_log(LOG_ERROR, "receive image failed! retval : %d", retval);
            break;
        }
    } while (1);

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    return retval;
}
#endif  // __ET7XX__
#endif
