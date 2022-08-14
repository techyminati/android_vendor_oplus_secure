MTK_PROJECT_NAME := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000002.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000002.tlbin \
	vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000005.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000005.tlbin
