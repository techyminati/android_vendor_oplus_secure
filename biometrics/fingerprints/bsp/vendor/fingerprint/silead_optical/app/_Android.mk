LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../products/sileadConfig.mk

ifeq ($(strip $(SILEAD_FP_TEST_SUPPORT)),yes)
include $(call all-makefiles-under, $(LOCAL_PATH))
endif