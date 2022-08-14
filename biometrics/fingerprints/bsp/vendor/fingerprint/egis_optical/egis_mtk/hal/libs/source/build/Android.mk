# =============================================================================
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

BUILD_RBS_RELEASE = true

EXTERNAL_LIB := NONE
BUILD_MERGE_MAP = false
NEED_SDK_SAVE_IMAGE = true
BUILD_OPLUS = true
SUPPORT_SAVE_ENROLL_RAW_DATA = false
SUPPORT_AUTH0 = true
SAVE_BACKUP_CB = true

ifeq ($(SENSOR_TYPE),ET0XX)
	PLATFORM=normal
else
	include $(LOCAL_PATH)/config.mk
endif

ifeq ($(PLATFORM),normal_otg)
PLATFORM=normal
PLATFORM_OTG=true
else ifeq ($(PLATFORM),trustonic)
EXTERNAL_LIB := libMcClient
else ifeq ($(PLATFORM),teei)
EXTERNAL_LIB := libmtee
endif 

$(info "PLATFORM = $(PLATFORM)")
$(info "EXTERNAL_LIB = $(EXTERNAL_LIB)")
$(info "BUILD_RBS_RELEASE = $(BUILD_RBS_RELEASE)")

ifneq ($(EXTERNAL_LIB), NONE)
include $(CLEAR_VARS)
LOCAL_MODULE := $(EXTERNAL_LIB)
LOCAL_SRC_FILES := ../../project/$(PLATFORM)_7xx/CA/Locals/Code/libs/$(TARGET_ARCH_ABI)/$(EXTERNAL_LIB).so
include $(PREBUILT_SHARED_LIBRARY)
endif 


include $(CLEAR_VARS)
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


ifeq ($(PLATFORM),trustonic)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../project/trustonic_7xx/CA/Locals/Code/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../project/trustonic_7xx/CA/Locals/Code
LOCAL_CFLAGS += -DTEMPLATE_UPGRADE_FROM_SAVED_IMAGE
LOCAL_CFLAGS += -D__xSAVE_IN_REE__
else ifeq ($(PLATFORM),normal)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../core/command_handler \
	$(LOCAL_PATH)/../core/inline_handler
endif 

LOCAL_VENDOR_MODULE := true

LOCAL_MODULE	:= libRbsFlow
ifneq ($(EXTERNAL_LIB), NONE)
LOCAL_SHARED_LIBRARIES := $(EXTERNAL_LIB)
endif

ifeq ($(PLATFORM), normal)
LOCAL_SHARED_LIBRARIES := libegis_fp_core
endif 

LOCAL_SHARED_LIBRARIES += \
    libcutils

# === config of Optical Calibration ===
ifeq ($(FP_EGIS_OPTICAL_FA118), y)
LOCAL_CFLAGS += -DFP_EGIS_OPTICAL_FA118
else
BUILD_RBS_RELEASE = false
endif
#	define source file path
LOCAL_SRC_FILES :=	../flow/egis_rbs_api.c
#	==== core ====
LOCAL_SRC_FILES += ../flow/core/captain.c	\
				   ../flow/core/device_int.c \
				   ../flow/core/op_manager.c	\
				   ../flow/core/thread_manager.c \
				   ../flow/plugin/sdk_save_image/src/save_image.c

ifeq ($(BUILD_OPLUS),true)
LOCAL_SRC_FILES += ../flow/core/oplus_event.c
endif

#	==== util ====
ifeq ($(SENSOR_TYPE),ET0XX)	
LOCAL_SRC_FILES += ../flow/util/fps_no_sensor.c
LOCAL_CFLAGS += -D__ET0XX__
ifeq ($(ET0XX_FLOW_ET7XX),true)
LOCAL_CFLAGS += -D__ET7XX__
LOCAL_CFLAGS += -DENABLE_POLL
endif
ifeq ($(BUILD_RBS_EVTOOL),true)
LOCAL_CFLAGS += -DRBS_EVTOOL
endif
else ifeq ($(SENSOR_TYPE),ET7XX)
LOCAL_CFLAGS += -D__ET7XX__ -DHOST_TOUCH_CONTROL 
LOCAL_CFLAGS += -D__INLINE_SAVE_LOG_ENABLE_DEFAULT__  #(Choose __INLINE_SAVE_XXX__ to select which you need)
LOCAL_CFLAGS += -D__INLINE_SAVE_LOG_ENABLE_EXTRA_INFO__
#LOCAL_CFLAGS += -D__INLINE_SAVE_LOG_DISABLE__
LOCAL_CFLAGS += -DSEND_HOST_TEMPERATURE_INFO -DPOWER_CONTRL -DSW_INTERRUPT
LOCAL_CFLAGS += -D__TEMLATE_BACKUP__
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../core/sensor_test/src/7XX_inline_test 
ifeq ($(PLATFORM_OTG),true)
LOCAL_SRC_FILES += ../flow/util/fps_no_sensor.c
else
LOCAL_SRC_FILES += ../flow/util/fps_normal.c
endif
else
LOCAL_SRC_FILES += ../flow/util/fps_normal.c
endif

LOCAL_CFLAGS += -DFD_CAPTURE_IMAGE_FIRST_ONLY_GET_RAW
#	==== Speed log ====
LOCAL_CFLAGS += -DEGIS_SPEED_DBG
LOCAL_CFLAGS += -DEGIS_SAVE_BMP

LOCAL_SRC_FILES += ../flow/util/opt_file.c \
				   ../flow/util/op_sensortest.c	\
				   ../flow/util/transporter_test.c \
				   ../common/platform/src/linux/plat_log_linux.c  \
				   ../common/platform/src/linux/plat_mem_linux.c  \
				   ../common/platform/src/linux/plat_time_linux.c \
				   ../common/platform/src/linux/plat_file_linux.c \
				   ../common/platform/src/linux/plat_thread_linux.c \
				   ../common/platform/src/linux/plat_std_linux.c 


# ==== sdk save image ====
ifeq ($(NEED_SDK_SAVE_IMAGE),true)
ifeq ($(SENSOR_TYPE),ET7XX)
LOCAL_CFLAGS += -D__SDK_SAVE_IMAGE_V2__
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/save_image_v2.c
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/egis_save_to_bmp.c
else ifeq ($(ET0XX_FLOW_ET7XX),true)
LOCAL_CFLAGS += -D__SDK_SAVE_IMAGE_V2__
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/save_image_v2.c
LOCAL_SRC_FILES += ../flow/plugin/sdk_save_image/src/egis_save_to_bmp.c
endif
endif

ifeq ($(BUILD_ALGORITHM_VERSION),4)
LOCAL_CFLAGS += -DALGO_GEN_4
LOCAL_CFLAGS += -DALGO_OUT_BPP_32
endif

ifeq ($(BUILD_ALGORITHM_VERSION),5)
LOCAL_CFLAGS += -DALGO_GEN_5
endif

ifeq ($(PLATFORM),trustonic)
LOCAL_SRC_FILES += ../flow/util/opt_file_trustonic.c 
LOCAL_CFLAGS += -D__TRUSTONIC__
LOCAL_CFLAGS += -DDEVICE_DRIVER_NEED_SPI_ENALBE_DISABLE
else ifeq ($(PLATFORM),normal)
LOCAL_CFLAGS += -D__PLATFORM_NORMAL__
LOCAL_CFLAGS += -D__OPLUS_ON_A50__
ifeq ($(PLATFORM_OTG),true)
LOCAL_CFLAGS += -D__OTG_SENSOR__
else
LOCAL_CFLAGS += -DDEVICE_DRIVER_HAS_SPI
endif
else ifeq ($(PLATFORM),teei)
LOCAL_CFLAGS += -DDEVICE_DRIVER_NEED_SPI_ENALBE_DISABLE
LOCAL_CFLAGS += -D__DEVICE_DRIVER_MEIZU_1712__  #MEIZU_1712 workaround, need interrupt abort.
endif 
#	==== transporter ====
LOCAL_SRC_FILES += ../flow/transporter/packager.c \
				   ../flow/transporter/transporter_$(PLATFORM).c



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

ifeq ($(BUILD_RBS_RELEASE),true)
else
LOCAL_CFLAGS += -DEGIS_DBG
endif

ifeq ($(BUILD_RBS_RELEASE),true)
else
ifeq ($(OPEN_DEBUG_MEMORY),true)
$(info debug build open EGIS_DEBUG_MEMORY)
LOCAL_CFLAGS += -DEGIS_DEBUG_MEMORY
LOCAL_SRC_FILES += ../common/platform/src/egis_mem_debug.c
endif
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS += -fPIC
else ifeq ($(TARGET_ARCH_ABI),x86_64)
LOCAL_CFLAGS += -fPIC
endif

ifeq ($(BUILD_MERGE_MAP),true)
LOCAL_CFLAGS += -D_INLINE_MERGE_MAP
endif

ifeq ($(BUILD_RBS_RELEASE),true)
else
ifeq ($(SUPPORT_SAVE_IMAGE),true)
LOCAL_CFLAGS += -D__SUPPORT_SAVE_IMAGE__ \
				-DUSE_CORE_CONFIG_INI
endif
endif
LOCAL_CFLAGS += -D__OPLUS__
ifeq ($(SUPPORT_SAVE_ENROLL_RAW_DATA),true)
LOCAL_CFLAGS += -D__SUPPORT_SAVE_ENROLL_RAW__
endif

ifeq ($(SUPPORT_AUTH0),true)
LOCAL_CFLAGS += -D__AUTH0__
endif

ifeq ($(SAVE_BACKUP_CB),true)
LOCAL_CFLAGS += -DSAVE_BACKUP_CB
endif

LOCAL_LDFLAGS += -shared -nodefaultlibs -lc -lm -ldl

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)

ifeq ($(PLATFORM),normal)
include $(LOCAL_PATH)/../core/Android_core.mk
endif
