LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

define all-dirs-under-contain-name
$(call find-subdir-files,$(2) -type d -name "*$(1)*")
endef

define all-subdir-under-contain-name
$(call all-dirs-under-contain-name,$(1),.)
endef

ifeq ($(TARGET_PRODUCT),sdm710)
include $(call all-makefiles-under,$(LOCAL_PATH)/18097_18397/)
endif

ifeq ($(TARGET_PRODUCT),sm6150)
include $(call all-makefiles-under,$(LOCAL_PATH)/19031_19331_19111_19513/)
endif

ifeq ($(TARGET_PRODUCT),msmnile)
include $(call all-makefiles-under,$(LOCAL_PATH)/18115_18501_18503_18119_19061_19361/)
include $(call all-makefiles-under,$(LOCAL_PATH)/19071_19081_19371/)
endif

ifeq ($(TARGET_PRODUCT),sdm660)
include $(call all-makefiles-under,$(LOCAL_PATH)/17081_17085/)
endif

