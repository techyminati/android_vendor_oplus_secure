MTK_PROJECT_NAME := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/spi_ta/030c0000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/030c0000000000000000000000000000.drbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/spi_ta/030c0000000000000000000000000000.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/030c0000000000000000000000000000.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/spi_ta/09150000000000000000000000000000.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/09150000000000000000000000000000.tlbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/spi_ta/030b0000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/030b0000000000000000000000000000.drbin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/spi_ta/030b0000000000000000000000000000.tlbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/030b0000000000000000000000000000.tlbin
