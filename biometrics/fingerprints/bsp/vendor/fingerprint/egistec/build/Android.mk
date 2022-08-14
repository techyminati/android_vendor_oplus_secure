# =============================================================================
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

EXTERNAL_LIB := NONE
NEED_SDK_SAVE_IMAGE = false
NEED_SDK_SAVE_FINGER_OFF_IMAGE = false
SENSOR_TYPE=ET5XX

ifeq ($(call is-vendor-board-platform,QCOM),true)
PLATFORM=qsee
LOCAL_CFLAGS += -DAUTHTOKEN_HMAC
LOCAL_CFLAGS += -DQSEE
else
PLATFORM=trustonic
endif

SUPPORT_SAVE_IMAGE=true

ifeq ($(PLATFORM),trustonic)
EXTERNAL_LIB := libMcClient
else ifeq ($(PLATFORM),teei)
EXTERNAL_LIB := libmtee
endif

$(warning "PLATFORM = $(PLATFORM)")
$(warning "EXTERNAL_LIB = $(EXTERNAL_LIB)")

ifneq ($(EXTERNAL_LIB), NONE)
include $(CLEAR_VARS)
LOCAL_MODULE := $(EXTERNAL_LIB)
LOCAL_SRC_FILES := ../flow/libs/$(TARGET_ARCH_ABI)/$(EXTERNAL_LIB).so
include $(PREBUILT_SHARED_LIBRARY)
endif


include $(CLEAR_VARS)
# define head file path
LOCAL_C_INCLUDES := \
$(LOCAL_PATH)/../flow/ \
$(LOCAL_PATH)/../flow/navi/include	\
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

ifeq ($(PLATFORM),trustonic)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../project/trustonic/CA/Locals/Code/include
else ifeq ($(PLATFORM),normal)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../core/command_handler \
    $(LOCAL_PATH)/../../core/inline_handler
endif

ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../egis_biometrics
endif

LOCAL_VENDOR_MODULE := true

LOCAL_MODULE	:= libRbsFlow_cap
ifneq ($(EXTERNAL_LIB), NONE)
LOCAL_SHARED_LIBRARIES := $(EXTERNAL_LIB)
endif

ifeq ($(PLATFORM), normal)
LOCAL_SHARED_LIBRARIES := libegis_fp_core
endif 

#	define source file path
LOCAL_SRC_FILES :=	../flow/egis_rbs_api.c
#	==== core ====
LOCAL_SRC_FILES += ../flow/core/captain.c	\
				   ../flow/core/device_int.c \
				   ../flow/core/op_manager.c	\
				   ../flow/core/thread_manager.c \

#	==== util ====
ifeq ($(SENSOR_TYPE),ET0XX)	
	LOCAL_SRC_FILES += ../flow/util/fps_no_sensor.c
	
	LOCAL_CFLAGS += -D__ET0XX__
else
	LOCAL_SRC_FILES += ../flow/util/fps_normal.c
endif
LOCAL_SRC_FILES += ../flow/util/opt_file.c \
				   ../flow/util/op_sensortest.c	\
				   ../flow/util/transporter_test.c \
				   ../common/platform/src/linux/plat_log_linux.c  \
				   ../common/platform/src/linux/plat_mem_linux.c  \
				   ../common/platform/src/linux/plat_time_linux.c \
				   ../common/platform/src/linux/plat_file_linux.c \
				   ../common/platform/src/linux/plat_thread_linux.c \
				   ../common/platform/src/linux/plat_std_linux.c \
				   ../common/platform/src/egis_sprintf.c
# ==== sdk save image ====
ifeq ($(NEED_SDK_SAVE_IMAGE),true)
LOCAL_CFLAGS += -D__SDK_SAVE_IMAGE__
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/save_image.c 
endif

# ==== sdk save finger off image ====
ifeq ($(NEED_SDK_SAVE_FINGER_OFF_IMAGE),true)
LOCAL_CFLAGS += -D__SDK_SAVE_FINGER_OFF_IMAGE__
ifeq ($(NEED_SDK_SAVE_IMAGE),false)
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/save_image.c
endif
endif

ifeq ($(PLATFORM),trustonic)
LOCAL_SRC_FILES += ../flow/util/opt_file_trustonic.c 
LOCAL_CFLAGS += -D__TRUSTONIC__
LOCAL_CFLAGS += -D__TEMLATE_BACKUP__
LOCAL_CFLAGS += -DDEVICE_DRIVER_NEED_SPI_ENALBE_DISABLE
LOCAL_CFLAGS += -DSUPPORT_EGIS_NEED_TO_RESET
else ifeq ($(PLATFORM),normal)
LOCAL_CFLAGS += -DDEVICE_DRIVER_HAS_SPI
else ifeq ($(PLATFORM),qsee)
LOCAL_CFLAGS += -DSUPPORT_EGIS_NEED_TO_RESET
LOCAL_CFLAGS += -D__ENABLE_POWER_CONTROL__
endif 
#	==== transporter ====
LOCAL_SRC_FILES += ../flow/transporter/packager.c \
				   ../flow/transporter/transporter_$(PLATFORM).c

#	==== navigation ====
LOCAL_SRC_FILES += ../flow/navi/navi_manager.c \
				   ../flow/navi/navi_operator.c

#   ==== ini ====
LOCAL_SRC_FILES += ../common/utility/config/src/core_config.c \
				   ../common/utility/inih/ini.c \
				   ../common/utility/inih/ini_parser.c \
				   ../common/utility/hash_map/hashmap.c \
				   
LOCAL_CFLAGS += -D__LINUX__ -Werror -Wall -Wextra
LOCAL_CFLAGS +=  -pie -fPIE 
LOCAL_CFLAGS += -D__SINGLE_UPPER_LIMITS__=5
LOCAL_CFLAGS += -D__LEVEL_TRIGGER__
LOCAL_CFLAGS += -DINI_HASH_MAP
ifeq ($(BUILD_RBS_RELEASE),false)
# LOCAL_CFLAGS += -D__ENABLE_NAVIGATION__
LOCAL_CFLAGS += -DEGIS_DBG
endif
LOCAL_CFLAGS += -D__USE_EGIS_DEMOTOOL__

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS += -fPIC
endif

ifeq ($(BUILD_MERGE_MAP),true)
LOCAL_CFLAGS += -D_INLINE_MERGE_MAP
endif

#ifeq ($(SUPPORT_BINDER_SERVER),true)
#LOCAL_CFLAGS += -D__BINDER_SERVICE__
#endif

ifeq ($(SUPPORT_SAVE_IMAGE),true)
ifndef EGIS_DBG
LOCAL_CFLAGS += -D__SUPPORT_SAVE_IMAGE__ \
				-DUSE_CORE_CONFIG_INI
endif
endif

LOCAL_LDFLAGS += -shared -nodefaultlibs -lc -lm -ldl

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)

ifeq ($(PLATFORM),normal)
include $(LOCAL_PATH)/../core/Android_core.mk
endif