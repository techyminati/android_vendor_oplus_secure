# =============================================================================
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

#Qing.Guan@BSP.Fingerprint.Secure for save image if true

NEED_SDK_SAVE_IMAGE =  false
NEED_INLINE_SAVE_IMAGE = false
EGIS_LOG_DBG = false

include $(CLEAR_VARS)

EXTERNAL_LIB := NONE

include $(LOCAL_PATH)/config.mk

$(info "PLATFORM = $(PLATFORM)")
$(info "EXTERNAL_LIB = $(EXTERNAL_LIB)")
$(info "LOCAL_PATH = $(LOCAL_PATH)")

#	define head file path
LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/../flow/ \
$(LOCAL_PATH)/../flow/external_include 	\
$(LOCAL_PATH)/../flow/def 			\
$(LOCAL_PATH)/../flow/core/include		\
$(LOCAL_PATH)/../flow/util/include 		\
$(LOCAL_PATH)/../flow/plugin/sdk_save_image/inc 	\
$(LOCAL_PATH)/../flow/transporter/include \
$(LOCAL_PATH)/../common/platform/inc \
$(LOCAL_PATH)/../common/definition  \
$(LOCAL_PATH)/../common/utility/inih \
$(LOCAL_PATH)/../common/utility/config/inc \
$(LOCAL_PATH)/../common/utility/hash_map \
$(LOCAL_PATH)/../common/utility/finger_image_db/inc \
$(LOCAL_PATH)/../core/sensor_test/src/et7xx \
$(LOCAL_PATH)/../oplus_customer

LOCAL_VENDOR_MODULE := true

LOCAL_MODULE	:= libRbsFlow

#	define source file path
LOCAL_SRC_FILES :=  ../flow/egis_rbs_api.c
#	==== core ====
LOCAL_SRC_FILES +=  ../flow/core/captain.c  \
                    ../flow/core/device_int.c  \
                    ../flow/core/op_manager.c  \
                    ../flow/core/thread_manager.c

#	==== util ====
LOCAL_SRC_FILES += ../flow/util/fps_normal.c
LOCAL_CFLAGS += -D__ET7XX__ -DHOST_TOUCH_CONTROL 
LOCAL_CFLAGS += -DSEND_HOST_TEMPERATURE_INFO -DPOWER_CONTRL -DSW_INTERRUPT
LOCAL_CFLAGS += -DG3PLUS_MATCHER
LOCAL_SRC_FILES += ../flow/util/opt_file.c  \
                   ../flow/util/op_sensortest.c	\
                   ../flow/util/transporter_test.c \
                   ../common/platform/src/linux/plat_mem_linux.c  \
                   ../common/platform/src/linux/plat_time_linux.c \
                   ../common/platform/src/linux/plat_file_linux.c \
                   ../common/platform/src/linux/plat_thread_linux.c \
                   ../common/platform/src/linux/plat_log_linux.c  \
                   ../common/platform/src/linux/plat_std_linux.c 
# ==== sdk save image ====
ifeq ($(NEED_SDK_SAVE_IMAGE),true)
LOCAL_CFLAGS += -D__SDK_SAVE_IMAGE_V2__
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/save_image_v2.c 
endif 

# ==== inline save image ====
ifeq ($(NEED_INLINE_SAVE_IMAGE),true)
LOCAL_CFLAGS += -D__INLINE_SAVE_LOG__
endif 

#	==== transporter ====
LOCAL_SRC_FILES += ../flow/transporter/packager.c \
				   ../flow/transporter/transporter_$(PLATFORM).c

#	==== oplus cust ====
LOCAL_SRC_FILES += ../oplus_customer/oplus_event.c
#   ==== ini ====
LOCAL_SRC_FILES += ../common/utility/config/src/core_config.c \
                   ../common/utility/inih/ini.c \
                   ../common/utility/inih/ini_parser.c \
                   ../common/utility/hash_map/hashmap.c \
                   ../common/platform/src/egis_sprintf.c

				   
LOCAL_CFLAGS += -D__LINUX__ -Wall -Wunused-function -Wsometimes-uninitialized -Wunused-parameter -Wsign-compare
LOCAL_CFLAGS +=  -pie -fPIE 
LOCAL_CFLAGS += -D__SINGLE_UPPER_LIMITS__=5
LOCAL_CFLAGS += -D__LEVEL_TRIGGER__
LOCAL_CFLAGS += -DINI_HASH_MAP

# ==== inline save image ====
ifeq ($(EGIS_LOG_DBG),true)
LOCAL_CFLAGS += -DEGIS_DBG
endif 

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS += -fPIC
else ifeq ($(TARGET_ARCH_ABI),x86_64)
LOCAL_CFLAGS += -fPIC
endif

ifeq ($(BUILD_MERGE_MAP),true)
LOCAL_CFLAGS += -D_INLINE_MERGE_MAP
endif

#ifeq ($(SUPPORT_BINDER_SERVER),true)
#LOCAL_CFLAGS += -D__BINDER_SERVICE__
#endif
ifeq ($(BUILD_RBS_RELEASE),true)
else
ifeq ($(SUPPORT_SAVE_IMAGE),true)
LOCAL_CFLAGS += -D__SUPPORT_SAVE_IMAGE__ \
				-DUSE_CORE_CONFIG_INI
endif
endif
LOCAL_CFLAGS += -D__OPLUS__

LOCAL_LDFLAGS += -shared -nodefaultlibs -lc -lm -ldl

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)

ifeq ($(PLATFORM),normal)
include $(LOCAL_PATH)/../core/Android_core.mk
endif
