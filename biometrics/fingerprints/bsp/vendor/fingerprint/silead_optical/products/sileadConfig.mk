
SILEAD_FP_SUPPORT := yes
ifeq ($(strip $(SILEAD_FP_SUPPORT)),yes)
    SILEAD_FP_TEE_TYPE := gp
    SILEAD_FP_TEST_SUPPORT := yes
    SILEAD_FP_HAL_SRC := yes
    #SILEAD_FP_HAL_LIB_NAME := fingerprint.silead.default
    #SILEAD_FP_HAL_MODULE_ID := fingerprint.silead

    # hal open source, flags define
    ifeq ($(strip $(SILEAD_FP_HAL_SRC)),yes)
        SIL_SEC_TYPE := $(SILEAD_FP_TEE_TYPE)
        SIL_CUST_TYPE := dd235

        SIL_DUMP_IMAGE := no
        SIL_DEBUG_ALL_LOG := no
        SIL_DEBUG_LOG_DYNAMIC := no

        SIL_FP_EXT_CAPTURE_ENABLE := yes
        SIL_FP_EXT_SKT_SERVER_ENABLE := yes
    endif

endif
