SL_FP_ROOT := vendor/oplus_app/securebsp/Fingerprint/vendor/fingerprint/silead

include $(SL_FP_ROOT)/products/sileadConfig.mk

ifeq ($(strip $(FP_SILEAD_SUPPORT)),y)
    BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/common

    ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),qsee)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/qsee
    else ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),tee)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/tee
    else ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),gp)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/tee
    else ifeq ($(strip $(SILEAD_FP_TEE_TYPE)),trusty)
        BOARD_SEPOLICY_DIRS += $(SL_FP_ROOT)/sepolicy/android8/vendor/trusty
    endif

    ifeq ($(strip $(SILEAD_FP_TEST_SUPPORT)),yes)
        BOARD_PLAT_PUBLIC_SEPOLICY_DIR += $(SL_FP_ROOT)/sepolicy/android8/public
        BOARD_PLAT_PRIVATE_SEPOLICY_DIR += $(SL_FP_ROOT)/sepolicy/android8/private
    endif
endif
