ifeq ($(shell echo $(TARGET_PRODUCT) | sed -e 's/_1.*//g'),full_oppo6873)
PRODUCT_COPY_FILES += \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05060000000000000000000000000001.tabin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05060000000000000000000000000001.tabin \
    vendor/oppo_app/securebsp/Fingerprint/prebuilt/project/$(MTK_PROJECT_NAME)/goodix_ta/05070000000000000000000000000000.drbin:$(TARGET_COPY_OUT_VENDOR)/app/mcRegistry/05070000000000000000000000000000.drbin
endif