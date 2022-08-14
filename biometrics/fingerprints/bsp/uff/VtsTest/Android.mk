LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := vts_uff_fingerprint
LOCAL_SRC_FILES := \
    VtsClient.cpp

$(warning "uff_fingerprint PLATFORM_VERSION $(PLATFORM_VERSION)")
ifeq ($(PLATFORM_VERSION), 11)
LOCAL_CFLAGS += -DVERSION_ANDROID_R
LOCAL_SHARED_LIBRARIES := \
    libbinder \
    liblog \
    libhidlbase \
    libhidltransport \
    libcutils \
    libion\
    libutils \
    vendor.oplus.hardware.biometrics.fingerprint@2.1
else
LOCAL_CFLAGS += -DVERSION_ANDROID_S
LOCAL_SHARED_LIBRARIES := libbinder liblog libhidlbase libhidltransport \
                          libutils vendor.oplus.hardware.biometrics.fingerprint@2.1 \
                          android.hardware.biometrics.fingerprint@2.3 \
                          android.hardware.biometrics.fingerprint@2.2 \
                          android.hardware.biometrics.fingerprint@2.1
endif

include $(BUILD_EXECUTABLE)