LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)
ifeq ($(proj), box_tv)
LOCAL_STATIC_LIBRARIES += libdrvb_boxtv
#PRODUCT_COPY_FILES += $(LOCAL_PATH)/../libdrvb_boxtv.a:obj/STATIC_LIBRARIES/libdrvb_intermediates/libdrvb_boxtv.a
LOCAL_CFLAGS +=-D__ANDROID_BOX_TV__
else
LOCAL_STATIC_LIBRARIES += libapedec_mtk
LOCAL_STATIC_LIBRARIES += libdrvb
#PRODUCT_COPY_FILES += $(LOCAL_PATH)/../libdrvb.a:obj/STATIC_LIBRARIES/libdrvb_intermediates/libdrvb.a
LOCAL_CFLAGS +=-D__ANDROID_SP_TABLET__
endif

LOCAL_SRC_FILES := \
    ../demo/demac.c\
    ../demo/cue_parser.c\
    ../demo/libprofiler.c\
    ../demo/parser.c\
    ../demo/wavwrite.c
    
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../inc\
                    $(LOCAL_PATH)/../demo                    
                    
LOCAL_CFLAGS += -D__arm__ -D__ANDROID__ -D'__inline=inline' -D'__int64=long long' -mcpu=arm1136j-s -O3 -D'STATIC_DECLARE=static' -DSTATIC_ENHANCE -D'EXTERN=static'
LOCAL_MODULE_TAGS:=user
LOCAL_MODULE := $(sp2_swip)
LOCAL_PRELINK_MODULE:=false
LOCAL_ARM_MODE:=arm


	
include $(BUILD_EXECUTABLE)
