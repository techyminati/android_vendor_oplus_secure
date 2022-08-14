LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := sileadManager
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := silead

LOCAL_STATIC_JAVA_LIBRARIES := com.silead.manager
LOCAL_REQUIRED_MODULES := com.silead.manager

# if $(PLATFORM_SDK_VERSION) is android8.0 (26) or higher, use hidl
local_silead_build_hidl := $(shell if [ $(PLATFORM_SDK_VERSION) -ge 26 ]; then echo yes; else echo no; fi)
local_silead_build_below_9 := $(shell if [ $(PLATFORM_SDK_VERSION) -lt 28 ]; then echo yes; else echo no; fi)

ifeq ($(strip $(local_silead_build_hidl)),yes)
$(warning use hidl interface)
local_silead_except_src_list := $(call find-other-java-files, src/com/silead/fingerprint/server-normal)
ifeq ($(strip $(local_silead_build_below_9)),yes) #android8
LOCAL_STATIC_JAVA_LIBRARIES += vendor.silead.hardware.fingerprintext-V1.0-java-static
LOCAL_REQUIRED_MODULES += vendor.silead.hardware.fingerprintext-V1.0-java-static
else #android9
LOCAL_STATIC_JAVA_LIBRARIES += vendor.silead.hardware.fingerprintext-V1.0-java
LOCAL_REQUIRED_MODULES += vendor.silead.hardware.fingerprintext-V1.0-java
endif
else #android6/7
local_silead_except_src_list := $(call find-other-java-files, src/com/silead/fingerprint/server-hidl)
endif

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES := $(filter-out $(local_silead_except_src_list), $(LOCAL_SRC_FILES))

LOCAL_DEX_PREOPT := false

include $(BUILD_PACKAGE)