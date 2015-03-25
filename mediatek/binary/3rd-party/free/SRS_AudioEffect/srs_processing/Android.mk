ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(LOCAL_PATH)/CONFIG.mk $(LOCAL_PATH)/license/LICENSE.mk
################################################################################
#### BUILD SETUP - DO NOT EDIT #################################################

VCHECK_EMPTY :=
VCHECK_SPACE := $(VCHECK_EMPTY) $(VCHECK_EMPTY)
VCHECK_DOT := .
VCHECK_NUMS := $(subst $(VCHECK_DOT),$(VCHECK_SPACE),$(PLATFORM_VERSION))

ifeq ($(word 1,$(VCHECK_NUMS)),3)
AND_PLAT_HONEY := true
endif

ifeq ($(word 1,$(VCHECK_NUMS)),4)
ifeq ($(word 2,$(VCHECK_NUMS)),0)
AND_PLAT_ICS := true
endif
ifeq ($(word 2,$(VCHECK_NUMS)),1)
AND_PLAT_JB := true
endif
ifeq ($(word 2,$(VCHECK_NUMS)),2)
AND_PLAT_JB := true
endif
ifeq ($(word 2,$(VCHECK_NUMS)),3)
AND_PLAT_JB := true
endif
ifeq ($(word 2,$(VCHECK_NUMS)),4)
AND_PLAT_JB := true
endif
endif

AND_PLAT_EXT := 
AND_PLAT_INDEX := 7

ifeq ($(AND_PLAT_HONEY),true)
AND_PLAT_INDEX := 8
endif

ifeq ($(AND_PLAT_ICS),true)
AND_PLAT_EXT := _ics
AND_PLAT_INDEX := 9
endif

ifeq ($(AND_PLAT_JB),true)
AND_PLAT_EXT := _jb
AND_PLAT_INDEX := 10
endif

######################################################################################
#### ENVIRONMENT CONFIG - SETUP BASED ON ENVIRONMENT VARIABLES #######################

ifneq (,$(SRS_ENV_PRODUCT))
SRS_PRODUCT := $(SRS_ENV_PRODUCT)
endif

ifneq (,$(SRS_ENV_VARIANT))
SRS_VARIANT := $(SRS_ENV_VARIANT)
endif

ifneq (,$(SRS_ENV_TAGS))
SRS_TAGS := $(SRS_ENV_TAGS)
endif

ifneq (,$(SRS_ENV_FEATURES))
SRS_FEATURES := $(SRS_ENV_FEATURES)
endif

################################################################################
#### AUTO CONFIG - SETUP BASED ON BUILD CONFIG VARIABLES #######################

ifeq ($(SRS_PRODUCT),trumedia_hd)
SRS_TECH_WOWHDX := true
SRS_TECH_CSHP := true
SRS_TECH_TRUEQ := true
SRS_TECH_HLIMIT := true
endif

ifeq ($(SRS_PRODUCT),trumedia)
SRS_TECH_HPF := true
SRS_TECH_WOWHD := true
SRS_TECH_CSHP := true
SRS_TECH_TRUEQ := true
SRS_TECH_HLIMIT := true
endif

ifneq (,$(findstring userpeq,$(SRS_FEATURES)))
SRS_TECH_USERPEQ := true
endif

ifneq (,$(findstring usergeq,$(SRS_FEATURES)))
SRS_TECH_GEQ := true
endif

ifeq ($(SRS_TECH_WOWHDX),true)
SRS_TECHLIB_SRS3D := true
SRS_TECHLIB_SA_TRUBASS := true
SRS_TECHLIB_WIDESUR := true
SRS_TECHLIB_FFT := true
SRS_TECHLIB_DESIGN := true
endif

ifeq ($(SRS_TECH_WOWHD),true)
SRS_TECHLIB_SRS3D := true
SRS_TECHLIB_SA_TRUBASS := true
SRS_TECHLIB_DESIGN := true
endif

ifeq ($(SRS_TECH_CSHP),true)
SRS_TECHLIB_CSDECODE := true
SRS_TECHLIB_DESIGN := true
SRS_TECHLIB_HP360 := true
SRS_TECHLIB_SA_TRUBASS := true
endif

ifeq ($(SRS_TECH_USERPEQ),true)
SRS_TECH_TRUEQ := true
endif

ifeq ($(SRS_TECH_TRUEQ),true)
SRS_TECHLIB_DESIGN := true
endif

################################################################################
#### BUILD STATIC LIBRARY RULES ################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_ARCH),arm)
SRSLIB_PLAT := ARMV5TE_LE
endif

ifeq ($(TARGET_ARCH),x86)
SRSLIB_PLAT := x86
endif

ifeq ($(SRS_TECH_WOWHDX),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_wowhdplus_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECH_WOWHD),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_wowhd_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECH_CSHP),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_cshp_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECH_HLIMIT),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_hardlimiter_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_WIDESUR),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_widesurround_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_SRS3D),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_srs3d_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_CSDECODE),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_csdecoder_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_HP360),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_hp360_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_SA_TRUBASS),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_sa_trubass_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_FFT),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_fft_$(SRSLIB_PLAT)_NDK.a
endif

ifeq ($(SRS_TECHLIB_DESIGN),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_designer_$(SRSLIB_PLAT)_NDK.a
endif
	
ifeq ($(SRS_TECH_GEQ),true)
LOCAL_PREBUILT_LIBS += srs_libs/srs_graphiceq_$(SRSLIB_PLAT)_NDK.a
endif

LOCAL_PREBUILT_LIBS += license/srs_spool_$(SRS_PROTECT_TYPE)_$(SRSLIB_PLAT)_NDK.a

LOCAL_PREBUILT_LIBS += srs_libs/srs_common_$(SRSLIB_PLAT)_NDK.a
	
include $(BUILD_MULTI_PREBUILT)

################################################################################
#### BUILD LIBSRSPROCESSING.SO RULES ###########################################

include $(CLEAR_VARS)

LOCAL_MODULE:= libsrsprocessing
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	srs_techs/srs_tech_tools.cpp \
	srs_processing.cpp \
	srs_workspace.cpp \
	srs_params.cpp
	
LOCAL_SRC_FILES += srs_subs/srs_routing$(AND_PLAT_EXT).cpp
	
ifneq (,$(findstring trumedia,$(SRS_PRODUCT)))
LOCAL_SRC_FILES += srs_trumedia.cpp
LOCAL_CFLAGS += -D_SRSCFG_TRUMEDIA
endif

ifneq (,$(findstring trumedia_hd,$(SRS_PRODUCT)))
LOCAL_CFLAGS += -D_SRSCFG_TRUMEDIA_HD
endif

ifeq ($(SRS_TECH_HPF),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_hpf.cpp
endif

ifeq ($(SRS_TECH_WOWHDX),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_wowhdx.cpp
LOCAL_STATIC_LIBRARIES += srs_wowhdplus_$(SRSLIB_PLAT)_NDK
LOCAL_CFLAGS += -D_SRSCFG_WOWHDX
endif

ifeq ($(SRS_TECH_WOWHD),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_wowhd.cpp
LOCAL_STATIC_LIBRARIES += srs_wowhd_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECH_CSHP),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_cshp.cpp
LOCAL_STATIC_LIBRARIES += srs_cshp_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECH_USERPEQ),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_userpeq.cpp
LOCAL_CFLAGS += -D_SRSCFG_USERPEQ
endif

LOCAL_CFLAGS += -fPIC

ifeq ($(SRS_TECH_TRUEQ),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_trueq.cpp
endif

ifeq ($(SRS_TECH_HLIMIT),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_hlimit.cpp
LOCAL_STATIC_LIBRARIES += srs_hardlimiter_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_WIDESUR),true)
LOCAL_STATIC_LIBRARIES += srs_widesurround_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_CSDECODE),true)
LOCAL_STATIC_LIBRARIES += srs_csdecoder_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_HP360),true)
LOCAL_STATIC_LIBRARIES += srs_hp360_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_SRS3D),true)
LOCAL_STATIC_LIBRARIES += srs_srs3d_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_SA_TRUBASS),true)
LOCAL_STATIC_LIBRARIES += srs_sa_trubass_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_FFT),true)
LOCAL_STATIC_LIBRARIES += srs_fft_$(SRSLIB_PLAT)_NDK
endif

ifeq ($(SRS_TECHLIB_DESIGN),true)
LOCAL_STATIC_LIBRARIES += srs_designer_$(SRSLIB_PLAT)_NDK
endif
	
ifeq ($(SRS_TECH_GEQ),true)
LOCAL_SRC_FILES += srs_techs/srs_tech_geq.cpp
LOCAL_STATIC_LIBRARIES += srs_graphiceq_$(SRSLIB_PLAT)_NDK
LOCAL_CFLAGS += -D_SRSCFG_MOBILE_EQ

ifneq (,$(findstring extendgeq,$(SRS_FEATURES)))
LOCAL_CFLAGS += -D_SRSCFG_MOBILE_EQ_EXTENDED
else
LOCAL_CFLAGS += -D_SRSCFG_MOBILE_EQ_BASIC
endif

endif

ifneq (,$(DSPOFFLOAD_PATH))
include $(LOCAL_PATH)/$(DSPOFFLOAD_PATH)
endif

LOCAL_STATIC_LIBRARIES += srs_spool_$(SRS_PROTECT_TYPE)_$(SRSLIB_PLAT)_NDK

LOCAL_STATIC_LIBRARIES += srs_common_$(SRSLIB_PLAT)_NDK

ifeq ($(AND_PLAT_ICS),true)
LOCAL_STATIC_LIBRARIES += libmedia_helper
endif

ifeq ($(AND_PLAT_JB),true)
LOCAL_STATIC_LIBRARIES += libmedia_helper
endif
    
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia
    
LOCAL_C_INCLUDES += frameworks/base/libs/audioflinger

LOCAL_CFLAGS += -DSRS_BASECFG_READPATH=$(SRS_BASECFG_READPATH)
LOCAL_CFLAGS += -DSRS_USERCFG_PATH=$(SRS_USERCFG_PATH)

LOCAL_CFLAGS += -D_SRSCFG_LIBVARIANT=$(SRS_VARIANT)

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -D_SRSCFG_ARCH_ARM
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS += -D_SRSCFG_ARCH_X86
endif

ifneq (,$(POSTPRO_PROPGATE))
LOCAL_CFLAGS += -DPOSTPRO_PROPGATE
endif

ifeq ($(SRS_USERCFG_ALLOW),true)
LOCAL_CFLAGS += -DSRS_USERCFG_ALLOW
endif

ifeq ($(SRS_USERCFG_UNLOCKED),true)
LOCAL_CFLAGS += -DSRS_USERCFG_UNLOCKED
endif

ifneq (,$(findstring logging,$(SRS_FEATURES)))
LOCAL_CFLAGS += -DSRS_VERBOSE
endif

ifneq (,$(findstring audiolog,$(SRS_FEATURES)))
LOCAL_CFLAGS += -DSRS_AUDIOLOG
LOCAL_CFLAGS += -DSRS_AUDIOLOG_PREPATH=$(SRS_AUDIOLOG_PREPATH)
LOCAL_CFLAGS += -DSRS_AUDIOLOG_POSTPATH=$(SRS_AUDIOLOG_POSTPATH)

ifneq (,$(findstring audiosesslog,$(SRS_FEATURES)))
LOCAL_CFLAGS += -DSRS_AUDIOLOG_MARKERS
endif

endif

ifneq (,$(findstring forcesilence,$(SRS_FEATURES)))
LOCAL_CFLAGS += -DSRS_FORCE_SILENCE
endif

LOCAL_CFLAGS += -DSRS_AND_PLAT_INDEX=$(AND_PLAT_INDEX)

STAGS_EMPTY :=
STAGS_SPACE := $(STAGS_EMPTY) $(STAGS_EMPTY)
STAGS_COMMA := ,
STAGS_SEP := ^
SRS_TAGS_NOSPACE := $(subst $(STAGS_SPACE),$(STAGS_SEP),$(SRS_TAGS))
SRS_TAGS_OUT := $(subst $(STAGS_COMMA),$(STAGS_SEP),$(SRS_TAGS_NOSPACE))

LOCAL_CFLAGS += -DSRS_TAGS=$(SRS_TAGS_OUT)

ifeq ($(SRS_PROTECT_TYPE),license)
LOCAL_CFLAGS += -DSRS_PROTECT_PATH=$(SRS_PROTECT_PATH)
LOCAL_CFLAGS += -DSRS_PROTECT_AUTHID=$(SRS_PROTECT_AUTHID)
endif

ifneq (,$(findstring perftrack,$(SRS_FEATURES)))
LOCAL_CFLAGS += -D_SRSCFG_PERFTRACK
endif

ifeq ($(SRS_PARAMWRITE_CFG),false)
LOCAL_CFLAGS += -DSRS_PARAMWRITE_CFG_BLOCK
endif

ifeq ($(SRS_PARAMWRITE_PREF),false)
LOCAL_CFLAGS += -DSRS_PARAMWRITE_PREF_BLOCK
endif

ifeq ($(SRS_PARAMREAD_INFO),false)
LOCAL_CFLAGS += -DSRS_PARAMREAD_INFO_BLOCK
endif

ifeq ($(SRS_PARAMREAD_DEBUG),false)
LOCAL_CFLAGS += -DSRS_PARAMREAD_DEBUG_BLOCK
endif

ifeq ($(SRS_PARAMREAD_CFG),false)
LOCAL_CFLAGS += -DSRS_PARAMREAD_CFG_BLOCK
endif

ifeq ($(SRS_PARAMREAD_PREF),false)
LOCAL_CFLAGS += -DSRS_PARAMREAD_PREF_BLOCK
endif

ifeq ($(TARGET_SIMULATOR),true)
 LOCAL_LDLIBS += -ldl
else
 LOCAL_SHARED_LIBRARIES += libdl
endif

ifeq ($(TARGET_SIMULATOR),true)
    ifeq ($(HOST_OS),linux)
        LOCAL_LDLIBS += -lrt -lpthread
    endif
endif

LOCAL_LDFLAGS := -Wl,--no-fatal-warnings

ifeq ($(TARGET_ARCH),arm)
	ifeq ($(FORCE_COMPAT),true)
	LOCAL_CFLAGS += -fno-exceptions \
		-march=armv6 \
		-mthumb-interwork \
		-mfloat-abi=softfp \
		-g
	endif
endif

include $(BUILD_SHARED_LIBRARY)
endif
