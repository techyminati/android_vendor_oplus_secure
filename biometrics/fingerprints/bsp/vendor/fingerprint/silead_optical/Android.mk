#ifneq ($(filter full_oppo6779_18073 full_oppo6779_18593 full_oppo6779_19011 full_oppo6779_19301, $(TARGET_PRODUCT)),)

ifeq ($(TARGET_PRODUCT),full_oppo6779)
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/products/sileadConfig.mk

ifeq ($(strip $(SILEAD_FP_SUPPORT)),yes)
include $(call all-makefiles-under, $(LOCAL_PATH))
endif

endif
