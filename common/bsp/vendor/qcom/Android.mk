LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
ifeq ($(call is-vendor-board-platform,QCOM),true)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
