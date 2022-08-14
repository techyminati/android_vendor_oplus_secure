LOCAL_PATH:= $(call my-dir)
SINGLE_MODULE_TAGS := optional
SINGLE_TARGET := 

copy_from := $(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find . -name "*.b*" -o -name "*.mdt"  -and -not -name ".*") \
 )

$(foreach SINGLE_TARGET, $(copy_from), $(eval include $(LOCAL_PATH)/single.mk))
