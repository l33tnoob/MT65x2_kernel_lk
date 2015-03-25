########################    
# SRS Processing

# DEFINE SRS_LIB_TARGET elsewhere to redirect dependancies; for example between libsrsprocessing and libsrsstub.
ifeq ($(SRS_LIB_TARGET),)
SRS_LIB_TARGET := libsrsprocessing
endif

# DEFINE SRS_LIB_PATH elsewhere to account for placing the srs library projects in another location.
ifeq ($(SRS_LIB_PATH),)
#SRS_LIB_PATH := frameworks/base/services/srs_processing
SRS_LIB_PATH := mediatek/binary/3rd-party/free/SRS_AudioEffect/srs_processing
endif

# DEFINE SRS_LIB_SUPRESS to prevent AudioFlinger from depending on any srs libraries (for use with NULL stubs)
ifeq ($(SRS_LIB_SUPRESS),)
LOCAL_SHARED_LIBRARIES += $(SRS_LIB_TARGET)
endif

LOCAL_C_INCLUDES += $(SRS_LIB_PATH)

ifneq (,$(POSTPRO_PROPGATE))
LOCAL_CFLAGS += -DPOSTPRO_PROPGATE
endif

# SRS Processing
########################
