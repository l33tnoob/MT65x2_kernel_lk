###################################################################
# flag
###################################################################

MOD_SRC  = 
MOD_OBJ  = $(MTK_ROOT_OUT)/PRELOADER_OBJ/obj

ifeq ("$(MTK_SEC_VIDEO_PATH_SUPPORT)","yes")
C_OPTION+=DMTK_SEC_VIDEO_PATH_SUPPORT
endif

ifeq ("$(TRUSTONIC_TEE_SUPPORT)","yes")
C_OPTION+=DTRUSTONIC_TEE_SUPPORT
endif

###################################################################
# source files
# - add your file here !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
###################################################################

MOD_SRC  := \
trustzone.c \
tz_utils.c \
tz_mem.c \
tz_emi_mpu.c \
tz_boot_share_page.c

###################################################################
# target
###################################################################

include $(COMMON_FILE_MK)
