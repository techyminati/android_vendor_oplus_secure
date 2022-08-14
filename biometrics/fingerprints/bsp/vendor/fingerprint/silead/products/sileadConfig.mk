
ifeq ($(strip $(FP_SILEAD_SUPPORT)),y)

ifeq ($(FPC_TEE_RUNTIME), QSEE)
    SILEAD_FP_TEE_TYPE := qsee
else
    SILEAD_FP_TEE_TYPE := gp
endif
    SILEAD_RA := yes
    SILEAD_FP_TEST_SUPPORT := no
    SILEAD_FP_HAL_SRC := yes
    #SILEAD_FP_HAL_LIB_NAME := fingerprint.silead.default
    #SILEAD_FP_HAL_MODULE_ID := fingerprint.silead

    # hal open source, flags define
    ifeq ($(strip $(SILEAD_FP_HAL_SRC)),yes)
        SIL_SEC_TYPE := $(SILEAD_FP_TEE_TYPE)
        SIL_CUST_TYPE := 4f50

ifeq ($(OBSOLETE_KEEP_ADB_SECURE),1)
        #SIL_DUMP_IMAGE := yes
else
        SIL_DUMP_IMAGE := yes
endif
        #SIL_DUMP_IMAGE_DYNAMIC := yes
        #SIL_DUMP_IMAGE_SWITCH_PROP := log.tag.XXX.XXX
        SIL_DEBUG_ALL_LOG := yes
        #SIL_DEBUG_LOG_DYNAMIC := yes

        SIL_FP_EXT_CAPTURE_ENABLE := yes
        SIL_FP_EXT_SKT_SERVER_ENABLE := yes
        #SIL_DEBUG_MEM_LEAK := yes

        SIL_DEBUG_LOG_DUMP_DYNAMIC := yes
        SIL_FP_MTEE_SDSP_ENABLE := no
    endif

endif