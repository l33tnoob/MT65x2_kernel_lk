ifeq ($(BYPASS_NONNDK_BUILD), no)
NONNDK_C_INCLUDES := $(TARGET_PROJECT_INCLUDES) $(LOCAL_C_INCLUDES)
$(shell if [ ! -d $(LOCAL_PATH)/nonndk ]; then mkdir $(LOCAL_PATH)/nonndk; fi)
NONNDK_OUTPUT_PATH := $(LOCAL_PATH)/nonndk
NONNDK_CHECK := $(shell python $(MTK_PATH_SOURCE)external/nonNDK/nonndk.py "$(LOCAL_SRC_FILES)" "$(NONNDK_C_INCLUDES)" "$(LOCAL_PATH)" 	"$(NONNDK_OUTPUT_PATH)") 
endif
LOCAL_SRC_FILES  += $(subst $(LOCAL_PATH)/, , $(wildcard $(LOCAL_PATH)/nonndk/*.cpp))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/nonndk
$(warning $(NONNDK_CHECK))
 

