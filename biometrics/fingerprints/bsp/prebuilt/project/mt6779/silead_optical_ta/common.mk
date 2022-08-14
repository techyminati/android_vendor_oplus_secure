MTK_PROJECT_NAME := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/silead_optical_ta/511ead0a000000000000000000000000.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/511ead0a000000000000000000000000.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/silead_optical_ta/030f0000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/030f0000000000000000000000000000.drbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/silead_optical_ta/030f0000000000000000000000000000.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/030f0000000000000000000000000000.tlbin
