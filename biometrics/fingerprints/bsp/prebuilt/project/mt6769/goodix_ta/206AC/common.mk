MTK_PROJECT_NAME := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
ifneq ($(filter full_oppo6785_20682 full_oppo6785, $(TARGET_PRODUCT)),)
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/20682/05060000000000000000000000000000.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05060000000000000000000000000000.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/20682/05070000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05070000000000000000000000000000.drbin
else
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05060000000000000000000000000000.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05060000000000000000000000000000.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05070000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05070000000000000000000000000000.drbin
endif
