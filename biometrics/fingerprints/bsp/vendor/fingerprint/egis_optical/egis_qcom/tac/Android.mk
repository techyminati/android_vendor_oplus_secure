LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=  \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(TOPDIR)vendor/qcom/proprietary/securemsm/QSEEComAPI \
                    $(TARGET_OUT_HEADERS)/common/inc  \
                    $(LOCAL_PATH)/inc
          
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

#LOCAL_MODULE_TARGET_ARCH := arm64
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        libQSEEComAPI \
        libion


LOCAL_MODULE := libets_teeclient_v3
LOCAL_SRC_FILES := src/fp_client_lib.c
LOCAL_MULTILIB      := 64
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
#LOCAL_PRELINK_MODULE := false
#LOCAL_MODULE_OWNER := qcom
LOCAL_LDLIBS := -llog
LOCAL_PROPRIETARY_MODULE := true
LOCAL_LDFLAGS += -nodefaultlibs -lc -lm -ldl
include $(BUILD_SHARED_LIBRARY)



