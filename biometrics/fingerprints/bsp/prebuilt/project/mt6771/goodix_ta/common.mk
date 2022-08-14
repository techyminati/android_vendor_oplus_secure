MTK_PROJECT_NAME := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
#ifneq ($(filter full_oppo6771_19531 full_oppo6771_19151 full_oppo6771_19350, $(TARGET_PRODUCT)),)
ifneq ($(filter oppo6771_19531 oppo6771_19151 oppo6771_19350, $(OPPO_TARGET_DEVICE)),)
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/19531/05060000000000000000000000000000.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05060000000000000000000000000000.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/19531/05070000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05070000000000000000000000000000.drbin
else
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05060000000000000000000000000000.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05060000000000000000000000000000.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05060000000000000000000000000001.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05060000000000000000000000000001.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05070000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05070000000000000000000000000000.drbin
endif