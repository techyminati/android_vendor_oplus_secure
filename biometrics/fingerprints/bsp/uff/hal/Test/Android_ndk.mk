LOCAL_PATH := $(call my-dir)

gtest_dir := ${PREBUILT_ROOT}/ree/gtestlib

include $(CLEAR_VARS)
LOCAL_MODULE := gtest
LOCAL_SRC_FILES := \
    $(gtest_dir)/lib64/libgtest.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gtest_main
LOCAL_SRC_FILES := \
    $(gtest_dir)/lib64/libgtest_main.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fpta
LOCAL_SRC_FILES := \
    $(LOCAL_PATH)/../Ca/ree/lib/libfpta.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := fptest
#LOCAL_CPP_FEATURES := rtti-lcrystax_static -lgnustl_static -lsupc++
LOCAL_LDLIBS += -llog -stdlib=libc++ -lstdc++
LOCAL_STATIC_LIBRARIES := gtest gtest_main
LOCAL_SHARED_LIBRARIES := fpta

LOCAL_SRC_FILES := $(shell find src -maxdepth 1 -name "*.cpp")

LOCAL_SRC_FILES += \
    $(LOCAL_PATH)/../Ca/CaFactory.cpp \
    $(LOCAL_PATH)/../Ca/FpCa.cpp \
    $(LOCAL_PATH)/../Ca/ree/ReeCa.cpp

$(info SRC:$(LOCAL_SRC_FILES))

LOCAL_C_INCLUDES += $(gtest_dir)/include
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../Ca/include \
    $(LOCAL_PATH)/../Ca/ree/lib

include $(BUILD_EXECUTABLE)
