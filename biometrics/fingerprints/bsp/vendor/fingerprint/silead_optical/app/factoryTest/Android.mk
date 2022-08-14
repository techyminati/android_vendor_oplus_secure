LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := factoryTest
LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := silead

LOCAL_STATIC_JAVA_LIBRARIES := com.silead.manager
LOCAL_REQUIRED_MODULES := com.silead.manager

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_DEX_PREOPT := false

include $(BUILD_PACKAGE)