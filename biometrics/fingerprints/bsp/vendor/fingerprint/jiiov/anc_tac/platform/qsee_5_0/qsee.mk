

LOCAL_C_INCLUDES += $(LOCAL_ANC_TAC_PATH)/$(LOCAL_ANC_TAC_PLATFORM_PATH)/inc/

LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_PLATFORM_PATH)/src/anc_ca.c
LOCAL_SRC_FILES += $(LOCAL_ANC_TAC_PLATFORM_PATH)/src/anc_tee_hw_auth.c


LOCAL_C_INCLUDES += hardware/libhardware/include

LOCAL_CFLAGS += $(QSEECOM_CFLAGS)
LOCAL_CFLAGS += -DANC_EXPLICIT_CONVERSION
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifeq ($(INDEPENDENT_QSEE_5_x_SDK), TRUE)
LOCAL_C_INCLUDES += platform/qsee_5_x/include/QSEEComAPI
LOCAL_LDFLAGS += platform/qsee_5_x/libs/$(APP_ABI)/libQSEEComAPI.so

ifeq ($(TARGET_ION_ABI_VERSION_2), TRUE)
LOCAL_C_INCLUDES += platform/qsee_5_x/include/libion_2
else ifeq ($(TARGET_ION_ABI_VERSION_3), TRUE)
LOCAL_C_INCLUDES += platform/qsee_5_x/include/libion_3
endif
LOCAL_LDFLAGS += platform/qsee_5_x/libs/$(APP_ABI)/libion.so
else
LOCAL_C_INCLUDES += vendor/qcom/proprietary/securemsm/QSEEComAPI
LOCAL_SHARED_LIBRARIES += libQSEEComAPI
# only for sm7150
#ifeq ($(call is-board-platform-in-list,msmnile sdmshrike $(MSMSTEPPE) $(TRINKET)),true)
LOCAL_C_INCLUDES += system/core/libion/include \
		system/core/libion/kernel-headers

LOCAL_SHARED_LIBRARIES += libion
#endif
# only for sm8350
ifeq ($(call is-board-platform-in-list,msmnile sdmshrike kona $(MSMSTEPPE) lito atoll bengal $(TRINKET) lahaina sdm660 holi),true)
include $(LIBION_HEADER_PATH_WRAPPER)
LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
LOCAL_SHARED_LIBRARIES += libion
endif
endif



LOCAL_CFLAGS += -DANC_TA_DEFAULT_PATH='"$(ANC_CONFIG_TA_DEFAULT_PATH)"'
LOCAL_CFLAGS += -DANC_TA_DEFAULT_NAME='"$(ANC_CONFIG_TA_DEFAULT_NAME)"'
LOCAL_CFLAGS += -DANC_KEYMASTER_TA_PATH='"$(ANC_CONFIG_KEYMASTER_TA_PATH)"'
LOCAL_CFLAGS += -DANC_KEYMASTER_TA_NAME='"$(ANC_CONFIG_KEYMASTER_TA_NAME)"'
