LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/products/sileadConfig.mk

ifeq ($(strip $(FP_SILEAD_SUPPORT)),y)
include $(call all-makefiles-under, $(LOCAL_PATH))
endif

ifeq ($(call is-vendor-board-platform,QCOM),true)
SILEAD_TEE_RUNTIME=QSEE
else
SILEAD_TEE_RUNTIME=TBASE
endif

