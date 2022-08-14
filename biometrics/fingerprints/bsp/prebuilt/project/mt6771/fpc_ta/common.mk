MTK_PROJECT_NAME := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000000.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000000.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000001.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000001.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000002.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000002.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000003.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000003.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000004.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000004.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000005.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000005.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/fpc_ta/04010000000000000000000000000006.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/04010000000000000000000000000006.tlbin
