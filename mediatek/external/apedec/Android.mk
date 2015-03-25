LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
ifeq ($(proj), box_tv)
LOCAL_CFLAGS +=-D__ANDROID_BOX_TV__
else
LOCAL_CFLAGS +=-D__ANDROID_SP_TABLET__
endif

LOCAL_SRC_FILES := \
    src/apedec_all.c \
    src/gnu_asm/ARMv6_aligned_dot_and_add_64.s\
    src/gnu_asm/ARMv6_aligned_dot_and_sub_64.s\
    src/gnu_asm/ARMv6_unaligned_dot_and_add_64.s\
    src/gnu_asm/ARMv6_unaligned_dot_and_sub_64.s\
    src/gnu_asm/ARMv6_dot_product.s\
    src/gnu_asm/ARMv6_predictor.s

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_CFLAGS += -D__arm__ -D__ANDROID__ -D'__inline=inline' -D'__int64=long long' -mcpu=arm1136j-s -O3 -D'STATIC_DECLARE=static' -DSTATIC_ENHANCE -D'EXTERN=static'
ifeq ($(symbol), scramble)
LOCAL_CFLAGS +=-D__SCRAMBLE_SYMBOL__
endif
LOCAL_MODULE:= libapedec_mtk
LOCAL_PRELINK_MODULE:=false
LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:=\
	libnativehelper \
	libcutils \
	libutils
	
include $(BUILD_STATIC_LIBRARY)
