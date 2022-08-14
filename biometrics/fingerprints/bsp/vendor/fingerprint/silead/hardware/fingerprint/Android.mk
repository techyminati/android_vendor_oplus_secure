##
# export SEC_TYPE=gp
# export SEC_CFLAGS=-DAAA -DBBB
##

LOCAL_PATH := $(call my-dir)

#------------------------------------------------
# NOSEC(non security) TEE(trustonic) QSEE(qsee) GP(global platform) TRUSTY(trusty) OPTEE(op-tee) BEANPOD(beanpod) WATCH(watch) ALL(all)
ifeq ($(strip $(SEC_TYPE)),)
    SEC_TYPE = ALL
endif

ifneq ($(strip $(SIL_SEC_TYPE)),)
    SEC_TYPE = $(shell echo $(SIL_SEC_TYPE) | tr a-z A-Z)
endif

ifeq ($(strip $(SEC_TYPE)),ALL)
    SEC_TYPE = QSEE TEE NOSEC GP TRUSTY OPTEE BEANPOD WATCH
endif

ifneq ($(strip $(SEC_TYPE)),)
SEC_TYPE := $(shell echo $(SEC_TYPE) | sed 's/TRSTY/TRUSTY/')
endif

#------------------------------------------------
ifneq ($(strip $(SIL_CUST_TYPE)),)
    CUST_TYPE := $(shell echo $(SIL_CUST_TYPE) | tr A-Z a-z)
endif

CUST_BASE_DIR_NAME :=
CUST_DIR_PATH := $(shell find $(LOCAL_PATH)/cust_$(CUST_TYPE) ! -path "." -type d 2>/dev/null)
CUST_DIR_NAME := $(strip $(foreach type, $(CUST_DIR_PATH), $(shell basename $(type) | tr A-Z a-z)))
ifeq ($(CUST_DIR_NAME),cust_$(CUST_TYPE))
    CUST_BASE_DIR_NAME := $(CUST_DIR_NAME)
endif

#------------------------------------------------
build_date := $(shell date +%y%m%d%H%M%S)
LOCAL_CFLAGS_BASE := -DBUILD_DATE="\"$(build_date)\""
LOCAL_CFLAGS_BASE += -DPLATFORM_VERSION="\"$(PLATFORM_VERSION)\""

ifneq ($(strip $(SEC_CFLAGS)),)
    LOCAL_CFLAGS_BASE += $(SEC_CFLAGS)
endif

EXT_INFO_VALUE :=
ifeq ($(strip $(SIL_DUMP_IMAGE)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))i
    LOCAL_CFLAGS_BASE += -DSIL_DUMP_IMAGE
    ifeq ($(strip $(SIL_DUMP_IMAGE_DYNAMIC)),yes)
        EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))d
        LOCAL_CFLAGS_BASE += -DSIL_DUMP_IMAGE_DYNAMIC
        ifneq ($(strip $(SIL_DUMP_IMAGE_SWITCH_PROP)),)
            LOCAL_CFLAGS_BASE += -DSIL_DUMP_IMAGE_SWITCH_PROP="\"$(SIL_DUMP_IMAGE_SWITCH_PROP)\""
        endif
    endif
endif

ifeq ($(strip $(SIL_DEBUG_ALL_LOG)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))l
    LOCAL_CFLAGS_BASE += -DSIL_DEBUG_ALL_LOG
    ifeq ($(strip $(SIL_DEBUG_LOG_DYNAMIC)),yes)
        EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))d
        LOCAL_CFLAGS_BASE += -DSIL_DEBUG_LOG_DYNAMIC
    endif
endif

ifeq ($(strip $(SIL_FP_EXT_CAPTURE_ENABLE)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))c
    LOCAL_CFLAGS_BASE += -DSIL_FP_EXT_CAPTURE_ENABLE
endif

ifeq ($(strip $(SIL_FP_EXT_SKT_SERVER_ENABLE)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))s
    LOCAL_CFLAGS_BASE += -DSIL_FP_EXT_SKT_SERVER_ENABLE
endif

ifeq ($(strip $(SIL_DEBUG_LOG_DUMP_DYNAMIC)),yes)
    LOG_DUMP_SUPPORT := $(foreach type, $(SEC_TYPE), $(if $(shell [[ "qsee" == "$(shell echo $(type) | tr A-Z a-z)" ]] && find $(LOCAL_PATH)/qsee/extend -type f 2>/dev/null), yes))
    ifeq ($(strip $(LOG_DUMP_SUPPORT)),yes)
        EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE))f
        LOCAL_CFLAGS_BASE += -DSIL_DEBUG_LOG_DUMP_DYNAMIC
    endif
endif

ifeq ($(strip $(SILEAD_TEST_TA)),yes)
    LOCAL_CFLAGS_BASE += -DSIL_DEBUG_ENABLE
endif

#------------------------------------------------
EXCEPT_SRC_LIST := 

#------------------------------------------------
ifeq ($(strip $(SIL_FP_MTEE_SDSP_ENABLE)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE)),dsp
    LOCAL_CFLAGS_BASE += -DSIL_FP_MTEE_SDSP_ENABLE
    LOCAL_INCLUDES_BASE += $(LOCAL_PATH)/gp/mtee/public
endif
ifeq ($(strip $(SIL_FP_GP_CA_LOAD_DRV)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE)),drv
    LOCAL_CFLAGS_BASE += -DSIL_FP_GP_CA_LOAD_DRV
endif
ifeq ($(strip $(SIL_FP_GP_MEM_SIZE_2M)),yes)
    EXT_INFO_VALUE := $(strip $(EXT_INFO_VALUE)),2M
    LOCAL_CFLAGS_BASE += -DSIL_FP_GP_MEM_SIZE_2M
endif

#------------------------------------------------
SRC_FILES_SUFFIX := %.cpp %.c
BASE_FILES_PATH := $(LOCAL_PATH) $(LOCAL_PATH)/log
BASE_ALL_FILES  := $(foreach src_path, $(BASE_FILES_PATH), $(wildcard $(src_path)/*))
BASE_SRC_LIST   := $(filter $(SRC_FILES_SUFFIX), $(BASE_ALL_FILES))
BASE_SRC_LIST   := $(BASE_SRC_LIST:$(LOCAL_PATH)/%=%)

CA_FILES_PATH := $(foreach type, $(SEC_TYPE), $(LOCAL_PATH)/$(shell echo $(type) | tr A-Z a-z))
CA_ALL_FILES  := $(foreach src_path, $(CA_FILES_PATH), $(shell find $(src_path) -type f 2>/dev/null))
CA_SRC_LIST   := $(filter $(SRC_FILES_SUFFIX), $(CA_ALL_FILES))
BASE_SRC_LIST += $(CA_SRC_LIST:$(LOCAL_PATH)/%=%)

ifneq ($(strip $(CUST_BASE_DIR_NAME)),)
    CUST_FILES_PATH := $(foreach type, $(CUST_TYPE), $(LOCAL_PATH)/$(shell echo cust_$(type) | tr A-Z a-z))
    CUST_ALL_FILES  := $(foreach src_path, $(CUST_FILES_PATH), $(shell find $(src_path) -type f 2>/dev/null))
    CUST_SRC_LIST   := $(filter $(SRC_FILES_SUFFIX), $(CUST_ALL_FILES))
    BASE_SRC_LIST   += $(CUST_SRC_LIST:$(LOCAL_PATH)/%=%)
endif

BASE_SRC_LIST := $(filter-out $(EXCEPT_SRC_LIST), $(BASE_SRC_LIST))

CA_CFLAGS := $(foreach type, $(SEC_TYPE), $(if $(shell find $(LOCAL_PATH)/$(shell echo $(type) | tr A-Z a-z) -type f 2>/dev/null), -DSECURITY_TYPE_$(shell echo $(type) | tr a-z A-Z)))
CA_CFLAGS += $(foreach type, $(SEC_TYPE), $(if $(shell [[ "gp" == "$(shell echo $(type) | tr A-Z a-z)" ]] && find $(LOCAL_PATH)/$(shell echo $(type) | tr A-Z a-z) -type f 2>/dev/null), -DTBASE_API_LEVEL=3))
LOCAL_CFLAGS_BASE += $(CA_CFLAGS)

SEC_TYPE_VALUE := $(foreach type, $(SEC_TYPE), $(if $(shell find $(LOCAL_PATH)/$(shell echo $(type) | tr A-Z a-z) -type f 2>/dev/null), $(type)))
SEC_TYPE_VALUE := $(strip $(shell echo $(SEC_TYPE_VALUE) | tr A-Z a-z))
ifneq ($(strip $(SEC_TYPE_VALUE)),)
COMMA:= |
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
SEC_TYPE_VALUE := $(subst $(SPACE),$(COMMA),$(SEC_TYPE_VALUE))
LOCAL_CFLAGS_BASE += -DSEC_TYPE_VALUE="\"$(SEC_TYPE_VALUE)\""
endif

ifneq ($(strip $(EXT_INFO_VALUE)),)
LOCAL_CFLAGS_BASE += -DEXT_INFO_VALUE="\"$(EXT_INFO_VALUE)\""
endif

ifneq ($(strip $(CUST_BASE_DIR_NAME)),)
    CUST_CFLAGS := $(strip $(foreach type, $(CUST_TYPE), $(if $(shell find $(LOCAL_PATH)/$(shell echo cust_$(type) | tr A-Z a-z) -type f 2>/dev/null), $(shell echo $(type) | tr a-z A-Z))))
    ifneq ($(strip $(CUST_CFLAGS)),)
        LOCAL_CFLAGS_BASE += -DBUILD_CUST_INFO="\"$(CUST_CFLAGS)\""
    endif
endif

LOCAL_CFLAGS_BASE := $(strip $(LOCAL_CFLAGS_BASE))
$(warning $(LOCAL_CFLAGS_BASE))

#------------------------------------------------
include $(CLEAR_VARS)
# if $(PLATFORM_SDK_VERSION) is android8.0 (26) or higher, enable vendor img
local_silead_support_vendor := $(shell if [ $(PLATFORM_SDK_VERSION) -ge 26 ]; then echo yes; else echo no; fi)
ifeq ($(strip $(local_silead_support_vendor)),yes)
    LOCAL_VENDOR_MODULE := true
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libsl_fp_impl
LOCAL_MODULE_OWNER := silead

LOCAL_SRC_FILES := $(BASE_SRC_LIST)
LOCAL_CFLAGS := $(LOCAL_CFLAGS_BASE)
LOCAL_CFLAGS += -DSIL_CODE_COMPATIBLE

LOCAL_C_INCLUDES := $(LOCAL_INCLUDES_BASE)
LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(LOCAL_PATH)/include \
                    $(LOCAL_PATH)/../../../hwbinder \
                    $(LOCAL_PATH)/../../../hwbinder/fingerprint_type/include


#------------------------------------------------
#select hypnusd method
#------------------------------------------------
ifeq ($(FP_CONFIG_HYPNUSD_ENABLE),1)
LOCAL_CFLAGS += -DFP_HYPNUSD_ENABLE
endif

ifeq ($(FP_SUPPORT_SIDE),y)
LOCAL_CFLAGS += -DSILEAD_CHECK_BROKEN
endif

LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libutils libion

ifeq ($(strip $(SIL_DEBUG_MEM_LEAK)),yes)
LOCAL_LDFLAGS := $(foreach f, $(strip malloc free realloc calloc), -Wl,--wrap=$(f))
LOCAL_SHARED_LIBRARIES += libunwind
LOCAL_STATIC_LIBRARIES += libheaptracker
LOCAL_CFLAGS += -DSIL_DEBUG_MEM_LEAK
endif

include $(BUILD_SHARED_LIBRARY)

