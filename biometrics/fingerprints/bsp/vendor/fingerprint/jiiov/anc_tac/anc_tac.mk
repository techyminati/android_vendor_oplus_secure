
LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        liblog


# --------------------------dcs--------------------------------------------
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/../../hwbinder/dcs/include/
# --------------------------common--------------------------------------------
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/common/common/inc/
LOCAL_SRC_FILES  += $(LOCAL_ANC_TAC_DIR)/common/common/src/anc_lib.c
LOCAL_SRC_FILES  += $(LOCAL_ANC_TAC_DIR)/common/common/src/anc_log_string.c
LOCAL_SRC_FILES  += $(LOCAL_ANC_TAC_DIR)/common/common/src/anc_ta_version.c

# --------------------------message-------------------------------------------
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/common/message/inc/


# --------------------------tac-----------------------------------------------
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/common/proprietary/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/platform/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/sensor/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/algorithm/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/auth_token/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/read_image/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/extension_command/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/auxiliary_command/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/dcs_service/inc/
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/factory_test_v2/inc/

LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/algorithm/src/anc_algorithm.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/algorithm/src/anc_algorithm_image.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/sensor/src/anc_tac_sensor.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/auth_token/src/anc_token.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/extension_command/src/extension_command.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/auxiliary_command/src/anc_auxiliary_command.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/proprietary/src/anc_tac_file.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/proprietary/src/anc_tac_time.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/proprietary/src/anc_tac_utils.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/common/src/anc_memory_debug.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/common/src/anc_memory_wrapper.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/proprietary/src/anc_memory.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/common/proprietary/src/anc_log.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/dcs_service/src/anc_tac_dcs.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/dcs_service/src/anc_tac_dcs_file.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/factory_test_v2/src/factory_test_ca.c

ifeq ($(ANC_CONFIG_USE_VIRTUAL_SENSOR), TRUE)
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/sensor/src/anc_tac_virtual_sensor.c
endif

ifeq ($(ANC_CONFIG_GET_IMAGE_FROM_TA),TRUE)
LOCAL_CFLAGS += -DANC_GET_IMAGE_FROM_TA
ifneq ($(ANC_CONFIG_EXT_TA_IMAGE_DEFAULT_PATH),)
LOCAL_CFLAGS += -DANC_EXT_TA_IMAGE_DEFAULT_PATH='"$(ANC_CONFIG_EXT_TA_IMAGE_DEFAULT_PATH)"'
endif
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_DIR)/read_image/src/anc_ca_image.c
endif

ifneq ($(ANC_CONFIG_ANC_DATA_ROOT),)
LOCAL_CFLAGS += -DANC_DATA_ROOT='"$(ANC_CONFIG_ANC_DATA_ROOT)"'
else
LOCAL_CFLAGS += -DANC_DATA_ROOT='"/data/vendor/fingerprint/"'
endif

ifeq ($(ANC_CONFIG_LOCAL_TEE_PLATFORM), QSEE)
LOCAL_ANC_TAC_PLATFORM_PATH := $(LOCAL_ANC_TAC_DIR)/platform/qsee_$(ANC_CONFIG_LOCAL_QSEE_VERSION)
include $(LOCAL_ANC_TAC_PATH)/platform/qsee_$(ANC_CONFIG_LOCAL_QSEE_VERSION)/qsee.mk

else ifeq ($(ANC_CONFIG_LOCAL_TEE_PLATFORM), TRUSTONIC)
LOCAL_ANC_TAC_PLATFORM_PATH := $(LOCAL_ANC_TAC_DIR)/platform/trustonic
include $(LOCAL_ANC_TAC_PATH)/platform/trustonic/trustonic.mk

else ifeq ($(ANC_CONFIG_LOCAL_TEE_PLATFORM), REE)
LOCAL_ANC_TAC_PLATFORM_PATH := $(LOCAL_ANC_TAC_DIR)/platform/ree
include $(LOCAL_ANC_TAC_PATH)/platform/ree/ree.mk
else
$(error "Unknown LOCAL_TEE_PLATFORM=$(ANC_CONFIG_LOCAL_TEE_PLATFORM)")
endif

ifeq ($(ANC_CONFIG_SAVE_FILE),TRUE)
LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/background_worker/inc/
LOCAL_SRC_FILES  += $(LOCAL_ANC_TAC_DIR)/background_worker/src/anc_background_worker.c
endif

ifneq ($(ANC_SAVE_AUTH_DEFAULT),)
LOCAL_CFLAGS += -DANC_SAVE_AUTH_DEFAULT=$(ANC_SAVE_AUTH_DEFAULT)
endif

ifneq ($(ANC_NEED_ENCRYTER_DEFAULT),)
LOCAL_CFLAGS += -DANC_NEED_ENCRYTER_DEFAULT=$(ANC_NEED_ENCRYTER_DEFAULT)
endif

# --------------------------target out header--------------------------------------------
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc



